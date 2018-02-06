#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "matting.h"
#include "mpp.h"
#include "mpi_ive.h"
#include "hi_ive.h"
#include "hi_comm_ive.h"
#include "arm_neon.h"

// A cat; B obama;
// Y[8D~A9]
// U[78~82]
// V[32~5a]
// VUVU
// __builtin_prefetch Êý¾ÝÔ¤È¡

unsigned char y_mid = 0x0;
unsigned char u_mid = 0x46;
unsigned char v_mid = 0x7D;

unsigned char y_thresh = 0;
unsigned char u_thresh = 0x7D;  // mid, +-5
unsigned char v_thresh = 0x46;  // mid, +-20

// TODO alpha blend 9ms
// r0~r11 usable
static void fastMatting(void *pAY/*r0*/, void *pBY/*r1*/, void *pAUV/*r2*/, void *pBUV/*r3*/)
{
	// r0~r3 , pAY, pBY, pAUV, pBUV
	asm volatile(
		// define constant value
		"vmov.I16 q4, #0x0006\n\t"  // for V sub & mul V = 20-(20*U)/128
		"vmov.I16 q5, #0x002b\n\t"  // for U sub & mul U = 43-(43*V)/128
		"vmov.I16 q6, #0x0080\n\t"  //
		"vmov.I16 q7, #0x0044\n\t"	// sub (V-0x5A ) 
		"vmov.I16 q8, #0xFF00\n\t"	//
		"vmov.I16 q9, #0x00FF\n\t"	//

//---------------------------for(h=540; h>0; h++) start ---------------------------
		"mov	r6, #540\n\t"	// reset height r6 = 540
		".hLoop:\n\t"

//---------------------------for(w=1920; w>0; w++) start---------------------------
		"add	r4, r0, #1920\n\t"
		"add	r5, r1, #1920\n\t"
		"mov	r7, #1920\n\t"  // reset width r7 = 1920		
		".wLoop:\n\t"

// calculate alpha value && process green Spillover 	// V = 20-(20*U)/128
		"vld1.8 {d0-d1}, [r2]!\n\t"

		// for V
		"vbic.I16	q1, q0, q8\n\t"
		"vshl.U16	q2, q1, #8\n\t"
		"vorr.U16	q1, q2, q1\n\t"  // q1 VVVVVVVVVVVVVVVV

		// U8 -> U16
		"vmovl.U8	q2, d2\n\t"
		"vmovl.U8	q3, d3\n\t"

		// V-q7
		"vqsub.U16	q2, q2, q7\n\t"
		"vqsub.U16	q3, q3, q7\n\t"

		// V*20
		"vmul.I16	q14, q2, q4\n\t" // 20*
		"vmul.I16	q15, q3, q4\n\t" // 20*

		// 
		"vcgt.U16   q14, q14, q9\n\n"
		"vcgt.U16   q15, q15, q9\n\n"
		
		"vbic.I16	q14, q14, q8\n\t"		
		"vbic.I16	q15, q15, q8\n\t"


#if 0
		// for U
		"vbic.I16	q2, q0, q9\n\t"
		"vshr.U16	q3, q2, #8\n\t"
		"vorr.U16	q2, q2, q3\n\t"  // q2 UUUUUUUUUUUUUUUU
#endif
		// for test UV
		//"vst1.8 {d2-d3}, [r3]!\n\t"
		//"vst1.8 {d4-d5}, [r3]!\n\t"

#if 0
		//green Spillover

		"vbic.I16	q0, 0xFF00\n\t"
		"vqsub.U16	q0, q0, q6\n\t" // V-128
		"vmul.I16	q0, q0, q6\n\t" // 20*
		"vshr.U16	q0, #8\n\t"     // >>8
		"vqsub.U16	q0, q6, q0\n\t" // 20 -
		
		//"vst1.8 {d0-d1}, [r3]!\n\t"
	
		"vbic.I16	q0, 0x00FF\n\t"
		"vshr.U16	q0, #8\n\t"
		"vqsub.U16	q0, q0, q6\n\t" // V-128
		"vmul.I16	q0, q0, q4\n\t" // 20*
		"vshr.U16	q0, #8\n\t" 	// >>8
		"vqsub.U16	q0, q4, q0\n\t" // 20 -

		//"vst1.8 {d0-d1}, [r3]\n\t"
#endif
// alpha blend
		// q14 -> d0, q15 -> d1 foreground alpha value
		// q12 -> d2, q13 -> d3 background alpha value
		"vmov.I16 q13, #0x0100\n\t"

		"vsub.U16 q12, q13, q14\n\t"
		"vsub.U16 q13, q13, q15\n\t"

//---------------------------matting start---------------------------
#if 1
// UV
		//"vld1.8 {d0-d1}, [r2]!\n\t"
		"vld1.8 {d2-d3}, [r3]\n\t"

		"vmovl.U8	q2, d0\n\t"
		"vmovl.U8	q3, d2\n\t"
		"vmul.I16	q2, q2, q14\n\t"
		"vmla.I16	q2, q3, q12\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d2, q2\n\t"

		"vmovl.U8	q2, d1\n\t"
		"vmovl.U8	q3, d3\n\t"
		"vmul.I16	q2, q2, q15\n\t"
		"vmla.I16	q2, q3, q13\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d3, q2\n\t"

		"vst1.8 {d2-d3}, [r3]!\n\t"
#endif
// Y0 line 2*i
		"vld1.8	{d0-d1}, [r0]!\n\t"
		"vld1.8	{d2-d3}, [r1]\n\t"

		"vmovl.U8	q2, d0\n\t"
		"vmovl.U8	q3, d2\n\t"
		"vmul.I16	q2, q2, q14\n\t"
		"vmla.I16	q2, q3, q12\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d2, q2\n\t"

		"vmovl.U8	q2, d1\n\t"
		"vmovl.U8	q3, d3\n\t"		
		"vmul.I16	q2, q2, q15\n\t"
		"vmla.I16	q2, q3, q13\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d3, q2\n\t"

		"vst1.8 {d2-d3}, [r1]!\n\t"

// Y1 line 2*i + 1
		"vld1.8 {d0-d1}, [r4]!\n\t"
		"vld1.8 {d2-d3}, [r5]\n\t"

		"vmovl.U8	q2, d0\n\t"
		"vmovl.U8	q3, d2\n\t"
		"vmul.I16	q2, q2, q14\n\t"
		"vmla.I16	q2, q3, q12\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d2, q2\n\t"

		"vmovl.U8	q2, d1\n\t"
		"vmovl.U8	q3, d3\n\t"
		"vmul.I16	q2, q2, q15\n\t"
		"vmla.I16	q2, q3, q13\n\t"
		"vshr.U16	q2, #8\n\t"
		"vmovn.I16	d3, q2\n\t"

		"vst1.8 {d2-d3}, [r5]!\n\t"

//---------------------------matting  end ---------------------------

		"sub	r7, #16\n\t"  // r7(width) -= 16
		"cmp	r7, #0\n\t"   // r7 == 0		
		"bne	.wLoop\n\t"   // not eq, goto wLoop	
//---------------------------for(w=1920; w>0; w++) end---------------------------

		"sub	r6, #1\n\t"   // r6(height) -= 1
		"cmp	r6, #0\n\t"   // r6 == 0
		
		"add	r0, r0, #1920\n\t"
		"add	r1, r1, #1920\n\t"

		"bne	.hLoop\n\t"   // not eq, goto hLoop
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		:
		: "r4", "r5","r6","r7", "memory"
	);
}

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *aFrame, VIDEO_FRAME_INFO_S *bFrame)
{
	HI_U8 *pA   = (HI_U8 *)HI_MPI_SYS_MmapCache(aFrame->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pAY  = pA;
	HI_U8 *pAUV = pA + 1920*1080;

	HI_U8 *pB   = (HI_U8 *)HI_MPI_SYS_MmapCache(bFrame->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBY  = pB;
	HI_U8 *pBUV = pB + 1920*1080;
	
	fastMatting(pAY, pBY, pAUV, pBUV);
#if 1
	printf("Y:%02x %02x %02x %02x %02x %02x %02x %02x UV:%02x %02x %02x %02x %02x %02x %02x %02x\n", 
		pBY[0], pBY[1], pBY[2], pBY[3], pBY[4], pBY[5], pBY[6], pBY[7],
		pBUV[0], pBUV[1], pBUV[2], pBUV[3], pBUV[4], pBUV[5], pBUV[6], pBUV[7]);
#endif
	HI_MPI_SYS_MflushCache(bFrame->stVFrame.u32PhyAddr[0], pB, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_Munmap(pA, 3110400);
	HI_MPI_SYS_Munmap(pB, 3110400);
	return bFrame;
}

void *mattingProcessThread(void *argv)
{
	HI_S32 s32Ret = HI_SUCCESS;
	VIDEO_FRAME_INFO_S stFrame_fg;
	VIDEO_FRAME_INFO_S stFrame_bg;
	VIDEO_FRAME_INFO_S *pstFrame = NULL;

	while(g_thread_run)
	{
		s32Ret = HI_MPI_VPSS_GetChnFrame(0, 0, &stFrame_fg, 1000);
		if(HI_SUCCESS == s32Ret)
		{
			do{
				s32Ret = HI_MPI_VPSS_GetChnFrame(6, 0, &stFrame_bg, 1000);
			}while(s32Ret != HI_SUCCESS);
			
			struct	timezone   tz;
			struct	timeval    tv;
			struct	timeval    tv2;
			gettimeofday(&tv,&tz);

			// TODO matting
			pstFrame = matting(&stFrame_fg, &stFrame_bg);
			if(NULL != pstFrame)
			{
				gettimeofday(&tv2,&tz);
				setInfo(tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);
				
				HI_MPI_VPSS_SendFrame(12, pstFrame, 1000);
			}

			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_fg);
			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_bg);
		}
	}
	
	return NULL;
}
