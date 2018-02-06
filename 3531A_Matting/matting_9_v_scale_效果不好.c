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
unsigned int i_thresh = 0x003c;
unsigned int o_thresh = 0x0006;

static void __attribute__ ((noinline)) fastMatting(void *pFC/*r0*/, void *pBC/*r1*/, void *pFY/*r2*/, void *pBY/*r3*/)
{
	asm volatile(
// load global variable
		"ldr	r4, =i_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q15, r5\n\t"	// q15 v_thresh

		"ldr	r4, =o_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q14, r5\n\t"	// q14 u_thresh

// define constant value
		"vmov.U16 q13, #0x0100\n\t"	//
		
//q0 for mul factor d0 [211, 179, 21, 232], d1 [4091, 32498]		
		"ldr	r5, =211\n\t"
		"vmov.U16	d0[0], r5\n\t"
		"ldr	r5, =179\n\t"
		"vmov.U16	d0[1], r5\n\t"
		"ldr	r5, =21\n\t"
		"vmov.U16	d0[2], r5\n\t"
		"ldr	r5, =232\n\t"
		"vmov.U16	d0[3], r5\n\t"
		"ldr	r5, =4091\n\t"
		"vmov.U16	d1[0], r5\n\t"
		"ldr	r5, =32498\n\t"
		"vmov.U16	d1[1], r5\n\t"
		"ldr	r5, =4\n\t"
		"vmov.U16	d1[2], r5\n\t"

		"mov	r8, #2\n\t"
		"mov	r9, #1920\n\t"
		"sub	r9, #4\n\t" 

// todo first line
		//"sub	r0, #1920\n\t" // start pos, upline
		//"add	r0, #1920\n\t"		
		//"sub	r0, #2\n\t"    // current line right 2 pixel,  for next line upline
		"sub	r0, #1\n\t"

		"add	r2, #1920\n\t"
		"add	r3, #1920\n\t"

//---------------------------for(h=540; h>0; h++) start ---------------------------
		"mov	r6, #540\n\t"	// reset height r6 = 540
		".hLoop:\n\t"

//---------------------------for(w=1920; w>0; w++) start---------------------------		
		"add	r4, r2, #1920\n\t"	// pFY + 1920
		"add	r5, r3, #1920\n\t"	// pBY + 1920
		"mov	r7, #1920\n\t"	// reset width r7 = 1920		
		".wLoop:\n\t"
// load data for scale V && blend UV
		"vld1.8 {d2-d3}, [r0], r8\n\t" // q1,	 up line r0 += 2
		"vshr.U16	q1, #8\n\t"

		"vld1.8 {d4-d5}, [r0], r9\n\t" // q2, up line right r0 += 1916
		"vshr.U16	q2, #8\n\t"

		"vld1.8 {d6-d7}, [r0], r8\n\t" // q3, current line left	
		"vshr.U16	q3, #8\n\t"

		"vld1.8 {d8-d9}, [r0], r8\n\t" // q4, current line v
		"vshr.U16	q4, #8\n\t"

		"vld1.8 {d18-d19}, [r0], r9\n\t" // q5, current line right
		"vshr.U16	q5, q9, #8\n\t"
		"vshl.U16	q9, q9, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t" // q9, current u

		"vld1.8 {d12-d13}, [r0], r8\n\t" // q6, up line 	3838+2
		"vshr.U16	q6, #8\n\t"

		"vld1.8 {d14-d15}, [r0], r8\n\t" // q7, up line 	3840+2
		"vshr.U16	q7, #8\n\t"

		"vld1.8 {d16-d17}, [r0]\n\t" // q8, up line 
		"vshr.U16	q8, #8\n\t"

		"sub	r0, #3840\n\t"
		"add	r0, #14\n\t"

// scale V
	// right //  ((q1+q2+q7+q8)/4 + (q4+q5)*3) /4
		"vadd.u16	q10, q1, q2\n\t"
		"vadd.u16	q11, q7, q8\n\t"
		"vadd.u16	q10, q10, q11\n\t"
		"vshr.U16	q10, #2\n\t"  // (q1+q2+q7+q8)/4

		"vadd.u16	q12, q4, q5\n\t"
		"vadd.u16	q11, q12, q12\n\t"
		"vadd.u16	q11, q11, q12\n\t" // (q4+q5)*3
		"vadd.u16	q10, q11, q10\n\t" // (q1+q2+q7+q8)/4 + (q4+q5)*3
		"vshr.U16	q1, q10, #2\n\t" //  ((q1+q2+q7+q8)/4 + (q4+q5)*3) /4
		//q1, right, q2, NULL

	// right down	(q4+q5+q7+q8)/4
		"vadd.u16	q10, q4, q5\n\t"
		"vadd.u16	q11, q7, q8\n\t"
		"vadd.u16	q2, q11, q10\n\t" 
		"vshr.U16	q2, #2\n\t"
		//q1, right, q2, right down

	// down (q3+q6+q5+q8)/16 + (q4+q7)*3/4
		"vadd.u16	q10, q3, q6\n\t"
		"vadd.u16	q11, q5, q8\n\t"
		"vadd.u16	q10, q10, q11\n\t"
		"vshr.U16	q10, #2\n\t"  //  (q3+q6+q5+q8)/4

		"vadd.u16	q12, q4, q7\n\t"
		"vadd.u16	q11, q12, q12\n\t"
		"vadd.u16	q11, q11, q12\n\t" // (q4+q7)*3
		"vadd.u16	q3, q11, q10\n\t" // (q3+q6+q5+q8)/4 + (q4+q7)*3
		"vshr.U16	q3, #2\n\t" //	((q3+q6+q5+q8)/4 + (q4+q7)*3)/4

//q1, right; q2, right down; q3, down; q4, current V; q9, current U
// q4, q9
#if 0
// green Spillover	b > r && g> b
// [b >  r] -> [ 211 * U > 179 * V + 4091 ]
		"vmul.U16	q8, q9, d0[0]\n\t" // 211 * U

		"vdup.U16	q7, d1[0]\n\t" // 4091
		"vmla.U16	q7, q4, d0[1]\n\t" // 4091 + 179 * V

		// vcgt q8 > q7
		"vcgt.U16	q8, q8, q7\n\t" // q2 b > r

// [g >  b] -> [ 32498 > 21 * U + 232 * V ]
		"vmul.U16	q6, q9, d0[2]\n\t" // 21 * U
		"vmla.U16	q6, q4, d0[3]\n\t" // 232 * V + 21 * U

		"vdup.U16	q5, d1[1]\n\t" // 32498
		"vcgt.U16	q6, q5, q6\n\t" //	g >  b

		//b > r && g> b
		"vand.U16	q8, q6, q8\n\t"

		"vqsub.U16	q5, q4, q8\n\t"
		"vqsub.U16	q6, q9, q8\n\t"

		"vmov.U16	q7, #0x0080\n\t"
		"vand.U16	q7, q8, q7\n\t"

		"vorr.U16	q8, q5, q7\n\t"  // V
		"vorr.U16	q9, q6, q7\n\t"  // U
#endif

// q4 alpha
		"vqsub.U16	q6, q4, q15\n\t"
		"vmul.U16	q6, q6, q14\n\t"
		"vcge.U16	q6, q6, q13\n\t"

		"vshl.U16	q7, q6, #8\n\t"
		"vshr.U16	q6, q7, #8\n\t"

		// UV blend q4, q9
		
		"vld1.8 {d16-d17}, [r1]\n\t"
		
		"vsub.U16	q5, q13, q6\n\t"  // q6 fg, q5 bg

		"vshl.U16	q10, q8, #8\n\t"
		"vshr.U16	q10, q10, #8\n\t" // V

		"vshr.U16	q8, q8, #8\n\t" // U
	
		"vmul.U16	q4, q4, q6\n\t"
		"vmul.U16	q9, q9, q6\n\t"

		"vmla.U16	q4, q10, q5\n\t"
		"vmla.U16	q9, q8, q5\n\t"
		
		"vshr.U16	q4, #8\n\t" // V
		"vshr.U16	q9, #8\n\t" // U
		"vshl.U16	q9, #8\n\t" // U
		"vorr.U16	q8, q4, q9\n\t"
		
		"vst1.8 {d16-d17}, [r1]!\n\t"

		// UV end

// q1 alpha
#if 1
		"vqsub.U16	q7, q1, q15\n\t"
		"vmul.U16	q7, q7, q14\n\t"
		"vcge.U16	q7, q7, q13\n\t"

		"vshl.U16	q7, q7, #8\n\t"
		//"vshr.U16	q7, q7, #8\n\t"
#endif


#if 1
		"vorr.U16	q6, q6, q7\n\t"

		"vmovl.U8	q5, d12\n\t"
		"vmovl.U8	q6, d13\n\t"
#endif

// alpha blend
		// q5 -> d0, q6 -> d1 foreground alpha value
		// q11 -> d2, q12 -> d3 background alpha value
		"vsub.U16 q11, q13, q5\n\t"
		"vsub.U16 q12, q13, q6\n\t"

//---------------------------matting start---------------------------

// Y0 line 2*i
		"vld1.8 {d14-d15}, [r2]!\n\t"
		"vld1.8 {d16-d17}, [r3]\n\t"

		"vmovl.U8	q9, d14\n\t"
		"vmovl.U8	q10, d16\n\t"
		"vmul.U16	q9, q9, q5\n\t"
		"vmla.U16	q9, q10, q11\n\t"
		"vshr.U16	q9, #8\n\t"
		"vmovn.U16	d16, q9\n\t"

		"vmovl.U8	q9, d15\n\t"
		"vmovl.U8	q10, d17\n\t"		
		"vmul.U16	q9, q9, q6\n\t"
		"vmla.U16	q9, q10, q12\n\t"
		"vshr.U16	q9, #8\n\t"
		"vmovn.U16	d17, q9\n\t"

		"vst1.8 {d16-d17}, [r3]!\n\t"

// Y1 line 2*i + 1

		"vqsub.U16	q6, q3, q15\n\t"
		"vmul.U16	q6, q6, q14\n\t"
		"vcge.U16	q6, q6, q13\n\t"

		"vshl.U16	q7, q6, #8\n\t"
		"vshr.U16	q6, q7, #8\n\t"
	// q1 alpha
#if 1
		"vqsub.U16	q7, q2, q15\n\t"
		"vmul.U16	q7, q7, q14\n\t"
		"vcge.U16	q7, q7, q13\n\t"

		"vshl.U16	q7, q7, #8\n\t"
		//"vshr.U16	q7, q7, #8\n\t"
#endif


#if 1
		"vorr.U16	q6, q6, q7\n\t"

		"vmovl.U8	q5, d12\n\t"
		"vmovl.U8	q6, d13\n\t"
#endif

		"vsub.U16 q11, q13, q5\n\t"
		"vsub.U16 q12, q13, q6\n\t"	


		"vld1.8 {d14-d15}, [r4]!\n\t"
		"vld1.8 {d16-d17}, [r5]\n\t"

		"vmovl.U8	q9, d14\n\t"
		"vmovl.U8	q10, d16\n\t"
		"vmul.U16	q9, q9, q5\n\t"
		"vmla.U16	q9, q10, q11\n\t"
		"vshr.U16	q9, #8\n\t"
		"vmovn.U16	d16, q9\n\t"

		"vmovl.U8	q9, d15\n\t"
		"vmovl.U8	q10, d17\n\t"
		"vmul.U16	q9, q9, q6\n\t"
		"vmla.U16	q9, q10, q12\n\t"
		"vshr.U16	q9, #8\n\t"
		"vmovn.U16	d17, q9\n\t"

		"vst1.8 {d16-d17}, [r5]!\n\t"

//---------------------------matting  end ---------------------------

		"sub	r7, #16\n\t"  // r7(width) -= 16
		"cmp	r7, #0\n\t"   // r7 == 0		
		"bne	.wLoop\n\t"   // not eq, goto wLoop	
//---------------------------for(w=1920; w>0; w++) end---------------------------

		"sub	r6, #1\n\t"   // r6(height) -= 1
		"cmp	r6, #0\n\t"   // r6 == 0
		
		"add	r2, r2, #1920\n\t" //skip to next line
		"add	r3, r3, #1920\n\t" //skip to next line

		"bne	.hLoop\n\t"   // not eq, goto hLoop
		".theEnd:\n\t"
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		: "r"(pBY), "r"(pFY), "r"(pBC), "r"(pFC)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "memory"
	);
}

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_fg, VIDEO_FRAME_INFO_S *frame_bg)
{
	HI_U8 *pFY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFC = pFY + 1920*1080;
	
	HI_U8 *pBY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBC = pBY + 1920*1080;
	
#if 0
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

	fastMatting(pFC, pBC, pFY, pBY);
#if 0
	int i;
	char info[12][10]={
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"a",
		"b",
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

