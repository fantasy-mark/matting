#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "arm_neon.h"
#include "mpp.h"
#include "matting.h"

// bg, normal camera; fg green camera;
unsigned int y_thresh = 0x008D;
unsigned int u_thresh = 0x0082;
unsigned int v_thresh = 0x0033;

/*
int r = 1.16438356 * (Y - 16) + 1.79274107 * (V-128);
int g = 1.16438356 * (Y - 16) - 0.21324861 * (U-128) - 0.53290933 * (V-128);
int b = 1.16438356 * (Y - 16) + 2.11240179 * (U-128);
*/
	
#define NEON_DEBUG 0

static void __attribute__ ((noinline)) fastMatting(void *pBY/*r0*/, void *pFY/*r1*/, void *pBC/*r2*/, void *pFC/*r3*/)
{
#if 0
	int x = 0;
	x += 32498;
#else
	// r0~r3 , pBY, pFY, pBC, pFC
	asm volatile(
// load global variable
		"ldr	r4, =v_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q11, r5\n\t"	// q11 v_thresh
		
		//"vst1.8 {d22-d23}, [r3]\n\t"	// test v_thresh

		"ldr	r4, =u_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q10, r5\n\t"	// q10 u_thresh

		//"vst1.8 {d20-d21}, [r3]\n\t"	// test u_thresh

// define constant value
		"vmov.I16 q9, #0x00FF\n\t"	//
		"vmov.I16 q8, #0xFF00\n\t"	//
		"vmov.I16 q7, #0x0100\n\t"	//

//---------------------------for(h=540; h>0; h++) start ---------------------------
		"mov	r6, #540\n\t"	// reset height r6 = 540
		".hLoop:\n\t"

//---------------------------for(w=1920; w>0; w++) start---------------------------
		"add	r4, r0, #1920\n\t"
		"add	r5, r1, #1920\n\t"
		"mov	r7, #1920\n\t"	// reset width r7 = 1920		
		".wLoop:\n\t"

// separate & duplicate UV
		"vld1.8 {d0-d1}, [r3]!\n\t"

		// for V
		"vbic.I16	q1, q0, q8\n\t"
		"vshl.U16	q2, q1, #8\n\t"
		"vorr.U16	q1, q2, q1\n\t"  // q1 VVVVVVVVVVVVVVVV

		//"vst1.8 {d2-d3}, [r3]\n\t"  // test v

		// for U
		"vbic.I16	q2, q0, q9\n\t"
		"vshr.U16	q3, q2, #8\n\t"
		"vorr.U16	q2, q2, q3\n\t"  // q2 UUUUUUUUUUUUUUUU

		//"vst1.8 {d4-d5}, [r3]\n\t"  // test u

// calculate alpha
		// U8 -> U16
		"vmovl.U8	q3, d2\n\t"
		"vmovl.U8	q4, d3\n\t"

		// V - v_thresh
		"vqsub.U16	q3, q3, q11\n\t"
		"vqsub.U16	q4, q4, q11\n\t"

		// V * 
		"vmov.I16	q5, #0x0006\n\t"
		"vmul.I16	q14, q3, q5\n\t"
		"vmul.I16	q15, q4, q5\n\t"

		// 
		"vcgt.U16	q14, q14, q9\n\t"
		"vcgt.U16	q15, q15, q9\n\t"

		"vbic.I16	q14, q14, q8\n\t"
		"vbic.I16	q15, q15, q8\n\t"
		
// green Spillover ( V = 20-(20*U)/128 && U = 43-(43*V)/128)
		"vbic.I16	q1, q0, q8\n\t" // q1 V 0x00xx
		"vshr.U16	q2, q0, #8\n\t" // q2 U 0x00xx
		
		//"vst1.8 {d2-d3}, [r2]!\n\t"
		//"vst1.8 {d4-d5}, [r2]!\n\t"

// b > r && g> b
		// 211 * U - 179 * V - 4091 > 0  // b >  r
		"vmov.U16	q6, #0x00D3\n\t"
		"vmul.U16	q3, q2, q6\n\t"

		// 4091
		"vmov.U16	q5, #0x0FFF\n\t"
		"vmov.U16	q6, #0x0004\n\t"
		"vsub.U16	q5, q5, q6\n\t"

		"vmov.U16	q6, #0x00B3\n\t"
		"vmla.U16	q5, q1, q6\n\t"

		// vcgt q3 > q5
		"vcgt.U16	q4, q3, q5\n\t" // q4 b > r
		
		//"vst1.8 {d6-d7}, [r2]!\n\t"
		//"vst1.8 {d10-d11}, [r2]!\n\t"
		
		// 32498 - 21 * U - 232 * V > 0  // g >  b
		"vmov.U16	q6, #0x0015\n\t"
		"vmul.U16	q3, q2, q6\n\t"
		
		//"vst1.8 {d6-d7}, [r2]!\n\t"
		
		"vmov.U16	q6, #0x00E8\n\t"
		"vmla.U16	q3, q1, q6\n\t"
		
		//"vst1.8 {d6-d7}, [r2]!\n\t"
	
		"vmov.U16	q5, #0x7E00\n\t"
		"vmov.U16	q6, #0x00F2\n\t"
		"vadd.U16	q6, q5, q6\n\t"
		
		//"vst1.8 {d12-d13}, [r2]!\n\t"
		
		"vcgt.U16	q5, q6, q3\n\t" // q3  g >  b
		//"vst1.8 {d8-d9}, [r2]!\n\t"
		//"vst1.8 {d10-d11}, [r2]!\n\t"

		"vand.U16	q6, q5, q4\n\t"
		//"vst1.8 {d12-d13}, [r2]!\n\t"

		"vmov.U16	q5, #0x8080\n\t"	
		"vand.U16	q5, q6, q5\n\t"

		"vqsub.U16	q0, q0, q6\n\t"
		//"vst1.8	{d0-d1}, [r2]!\n\t"

		"vorr.U16	q0, q0, q5\n\t"
		
		//"vst1.8	{d0-d1}, [r2]!\n\t"		
		//"b	.theEnd\n\t"
		
// alpha blend
		// q14 -> d0, q15 -> d1 foreground alpha value
		// q12 -> d2, q13 -> d3 background alpha value
		"vsub.U16 q12, q7, q14\n\t"
		"vsub.U16 q13, q7, q15\n\t"

//---------------------------matting start---------------------------
// UV
		//"vld1.8 {d0-d1}, [r3]!\n\t"
		"vld1.8 {d2-d3}, [r2]\n\t"

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

		"vst1.8 {d2-d3}, [r2]!\n\t"

// Y0 line 2*i
		"vld1.8	{d0-d1}, [r1]!\n\t"
		"vld1.8	{d2-d3}, [r0]\n\t"

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

		"vst1.8 {d2-d3}, [r0]!\n\t"

// Y1 line 2*i + 1
		"vld1.8 {d0-d1}, [r5]!\n\t"
		"vld1.8 {d2-d3}, [r4]\n\t"

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

		"vst1.8 {d2-d3}, [r4]!\n\t"

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
		".theEnd:"
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		: "r"(pFC), "r"(pBC), "r"(pFY), "r"(pBY)
		: "r4", "r5","r6","r7", "memory"
	);
#endif
}

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_bg, VIDEO_FRAME_INFO_S *frame_fg)
{
	HI_U8 *pBY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBC = pBY + 1920*1080;

	HI_U8 *pFY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFC = pFY + 1920*1080;
	
#if NEON_DEBUG
		// V
		pFC[0]  = 0x30;
		pFC[2]  = 0x7F;
		pFC[4]  = 0x90;
		pFC[6]  = 0xFE;
	
		pFC[8]  = 0x01;
		pFC[10] = 0x7F;
		pFC[12] = 0x90;
		pFC[14] = 0xFE;
	
		// U
		pFC[1]  = 0x90;
		pFC[3]  = 0x10;
		pFC[5]  = 0x40;
		pFC[7]  = 0x7F;
	
		pFC[9]  = 0x80;
		pFC[11] = 0xA0;
		pFC[13] = 0xC0;
		pFC[15] = 0xFE;
#endif

	fastMatting(pBY, pFY, pBC, pFC);

#if NEON_DEBUG
	
		int i;
		char info[12][10]={
			"v",
			"u",
			"b-r",
			"g-b",
			"",
			"",
			"",
			"",
			};
		for(i=0; i<16*12; i++)
		{		
			if(i%16 == 0)
				printf("\n%s:\t", info[i/16]);
			//if(i%16<2)
				printf("%02x ", pBC[i]); 
		}
	
		printf("\nend\n");
#endif

	HI_MPI_SYS_MflushCache(frame_bg->stVFrame.u32PhyAddr[0], pBY, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_Munmap(pBY, 3110400);
	HI_MPI_SYS_Munmap(pFY, 3110400);
	return frame_bg;
}

void *mattingProcessThread(void *argv)
{
	HI_S32 s32Ret = HI_SUCCESS;
	struct	timezone   tz;
	struct	timeval    tv;
	struct	timeval    tv2;

	VIDEO_FRAME_INFO_S stFrame_fg;
	VIDEO_FRAME_INFO_S stFrame_bg;
	VIDEO_FRAME_INFO_S *pstFrame = NULL;
	
	while(g_thread_run)
	{
		s32Ret = HI_MPI_VPSS_GetChnFrame(0, 0, &stFrame_bg, 1000);
		if(HI_SUCCESS == s32Ret)
		{
			do{
				s32Ret = HI_MPI_VPSS_GetChnFrame(5, 0, &stFrame_fg, 1000);
			}while(s32Ret != HI_SUCCESS);

			gettimeofday(&tv,&tz);

			pstFrame = matting(&stFrame_bg, &stFrame_fg);
			if(NULL != pstFrame)
			{
				gettimeofday(&tv2,&tz);
				setInfo(tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);
				
				fprintf(stderr, "%ld\n", tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);
				
				HI_MPI_VPSS_SendFrame(12, pstFrame, 1000);
			}

			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_fg);
			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_bg);
		}
	}
	
	return NULL;
}

