#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "arm_neon.h"
#include "mpp.h"
#include "matting.h"

// bg, normal camera; fg green camera;
unsigned int y_thresh = 0x00a8;
unsigned int u_thresh = 0x0078;
unsigned int v_thresh = 0x003c;
unsigned int i_thresh = 0x0014;
unsigned int o_thresh = 0x0030;

static void __attribute__ ((noinline)) fastMatting(void *pFC/*r0*/, void *pBC/*r1*/, void *pFY/*r2*/, void *pBY/*r3*/)
{
	asm volatile(
// load global variable

//---------------------------for(h=540; h>0; h++) start ---------------------------
		"mov	r6, #540\n\t"	// reset height r6 = 540
		".hLoop:\n\t"

//---------------------------for(w=1920; w>0; w++) start---------------------------
		"add	r4, r2, #1920\n\t"  // pFY + 1920
		"add	r5, r3, #1920\n\t"  // pBY + 1920
		"mov	r7, #1920\n\t"	// reset width r7 = 1920		
		".wLoop:\n\t"
//---------------------------matting start---------------------------

// load VU
		"vld1.8 {d0-d1}, [r0]!\n\t" // q0 F VU

		"vshl.U16	q1, q0, #8\n\t" // q1 V 0xHH00
		"vshr.U16	q1, q1, #8\n\t" // q1 V 0x00HH

		"vshr.U16	q2, q0, #8\n\t" // q2 U 0x00HH

// green Spillover ( V = 20-(20*U)/128 && U = 43-(43*V)/128)    b > r && g> b
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

// calculate alpha & blend

// V^2 + U^2
		"vmov.U16	q15, #0x003c\n\t"
		"vabd.U16	q1, q1, q15\n\t"
		//"vst1.8 {d2-d3}, [r1]!\n\t"

		"vmov.U16	q15, #0x0078\n\t"
		"vabd.U16	q2, q2, q15\n\t"
		//"vst1.8 {d4-d5}, [r1]!\n\t"

		"vmul.U16	q1, q1, q1\n\t"
		//"vst1.8 {d2-d3}, [r1]!\n\t"
		"vmul.U16	q2, q2, q2\n\t"
		//"vst1.8 {d4-d5}, [r1]!\n\t"

		"vqadd.U16	q1, q1, q2\n\t"
		//"vst1.8 {d2-d3}, [r1]!\n\t"

// 1
		"vld1.8 {d6-d7}, [r2]!\n\t" // q3 F Y line 0
		//"vst1.8 {d6-d7}, [r1]!\n\t"

		"vmov.I8	q15, #0xA8\n\t"
		"vabd.U8	q4, q3, q15\n\t"
		//"vst1.8 {d8-d9}, [r1]!\n\t"

		"vshl.U16	q5, q4, #8\n\t"
		"vshr.U16	q5, q5, #8\n\t"
		//"vst1.8 {d10-d11}, [r1]!\n\t"

		"vshr.U16	q4, q4, #8\n\t"
		//"vst1.8 {d8-d9}, [r1]!\n\t"
		
		"vmul.U16	q4, q4, q4\n\t"	
		"vqadd.U16	q4, q1, q4\n\t"
		//"vst1.8 {d8-d9}, [r1]!\n\t"	

		// calculate alpha
		// uv alpha blend
		// y00 alpha blend

		"vmul.U16	q5, q5, q5\n\t"	
		"vqadd.U16	q5, q1, q5\n\t"
		//"vst1.8 {d10-d11}, [r1]!\n\t"

		// calculate alpha
		// y01 alpha blend

		"vld1.8 {d4-d5}, [r4]!\n\t" // F Y line 1

	
		// calculate alpha
		// uv alpha blend
		// y10 alpha blend

		// calculate alpha
		// y11 alpha blend

#if 0
		"vmov.I16 q15, #0x0000\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"vst1.8 {d30-d31}, [r1]!\n\t"
		"b	.theEnd\n\t"
#endif

//---------------------------matting  end ---------------------------

		"sub	r7, #16\n\t"  // r7(width) -= 16
		"cmp	r7, #0\n\t"   // r7 == 0		
		"bne	.wLoop\n\t"   // not eq, goto wLoop	
//---------------------------for(w=1920; w>0; w++) end---------------------------

		"sub	r6, #1\n\t"   // r6(height) -= 1
		"cmp	r6, #0\n\t"   // r6 == 0
		
		"add	r2, r2, #1920\n\t"
		"add	r3, r3, #1920\n\t"

		"bne	.hLoop\n\t"   // not eq, goto hLoop
		".theEnd:"
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		: "r"(pBY), "r"(pFY), "r"(pBC), "r"(pFC)
		: "r4", "r5","r6","r7", "memory"
	);
}

#define NEON_DEBUG 1

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_fg, VIDEO_FRAME_INFO_S *frame_bg)
{
	HI_U8 *pFY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFC = pFY + 1920*1080;
	
	HI_U8 *pBY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBC = pBY + 1920*1080;
	
#if NEON_DEBUG
		// V
		pFC[0]  = 0xFF;
		pFC[2]  = 0x7F;
		pFC[4]  = 0x90;
		pFC[6]  = 0xFE;
	
		pFC[8]  = 0x01;
		pFC[10] = 0x7F;
		pFC[12] = 0x90;
		pFC[14] = 0xFE;
	
		// U
		pFC[1]  = 0xFF;
		pFC[3]  = 0x10;
		pFC[5]  = 0x40;
		pFC[7]  = 0x7F;
	
		pFC[9]  = 0x80;
		pFC[11] = 0xA0;
		pFC[13] = 0xC0;
		pFC[15] = 0xFE;
		pFY[0] = 0xFF;
		pFY[1] = 0xFF;
#endif

	fastMatting(pFC, pBC, pFY, pBY);

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
	HI_MPI_SYS_Munmap(pFY, 3110400);
	HI_MPI_SYS_Munmap(pBY, 3110400);
	return frame_bg;
}

void *mattingProcessThread(void *argv)
{
	HI_S32 s32Ret = HI_SUCCESS;
	struct	timezone   tz;
	struct	timeval    tv;
	struct	timeval    tv2;

	VIDEO_FRAME_INFO_S *pstFrame = NULL;

	VIDEO_FRAME_INFO_S stFrame_bg;
	VIDEO_FRAME_INFO_S stFrame_fg;

	while(g_thread_run)
	{
		s32Ret = HI_MPI_VPSS_GetChnFrame(5, 0, &stFrame_fg, 1000);
		if(HI_SUCCESS == s32Ret)
		{
			do{
				s32Ret = HI_MPI_VPSS_GetChnFrame(0, 0, &stFrame_bg, 1000);
			}while(s32Ret != HI_SUCCESS);

			gettimeofday(&tv,&tz);

			pstFrame = matting(&stFrame_fg, &stFrame_bg);
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

