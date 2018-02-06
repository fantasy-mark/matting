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
unsigned int i_thresh = 0x0044;
unsigned int o_thresh = 0x0006;

#define ALPHA_CALC 1
static void __attribute__ ((noinline)) fastMatting(void *pFC/*r0*/, void *pBC/*r1*/, void *pFY/*r2*/, void *pBY/*r3*/)
{
	asm volatile(
// load global variable
		"ldr	r4, =i_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q15, r5\n\t"	// q15 v_LowThr

		"ldr	r4, =o_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q14, r5\n\t"	// q14 

// define constant value
		"vmov.U16	q13, #0x0100\n\t"
		"vmov.U16	q12, #0x00FF\n\t"

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
		"mov	r7, #1920\n\t"	// reset width r7 = 1920
		"add	r4, r2, #1920\n\t"	// pFY + 1920
		"add	r5, r3, #1920\n\t"	// pBY + 1920

		".wLoop:\n\t"
//---------------------------matting start---------------------------	
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
		"vshr.U16	q3, #2\n\t" //  ((q3+q6+q5+q8)/4 + (q4+q7)*3)/4

//q1, right; q2, right down; q3, down; q4, current V; q9, current U

// q4, q9
// green Spillover  b > r && g> b
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
		"vcgt.U16	q6, q5, q6\n\t" //  g >  b

		//b > r && g> b
		"vand.U16	q8, q6, q8\n\t"

		"vqsub.U16	q5, q4, q8\n\t"
		"vqsub.U16	q6, q9, q8\n\t"

		"vmov.U16	q7, #0x0080\n\t"
		"vand.U16	q7, q8, q7\n\t"

		"vorr.U16	q8, q5, q7\n\t"  // V
		"vorr.U16	q9, q6, q7\n\t"  // U

// 0, calculate UV alpha & blend q4
// alpha blend, q4,q5 fgalpha, q6,q7 bg alpha
#if ALPHA_CALC
		"vqsub.U16	q4, q4, q15\n\t" // v - v_low
		"vmul.U16	q4, q4, q14\n\t"
		"vmin.U16	q4, q4, q13\n\t" // fg alpha
		"vqsub.U8	q6, q13, q4\n\t" // bg alpha

		"vqsub.U16	q1, q1, q15\n\t" // v - v_low
		"vmul.U16	q1, q1, q14\n\t"
		"vmin.U16	q5, q1, q13\n\t" // fg alpha
		"vsub.U16	q7, q13, q5\n\t"
#else
		"vmov.U16	q4, #0x0080\n\t"
		"vmov.U16	q5, #0x0080\n\t"
		"vsub.U16	q6, q13, q4\n\t"
		"vsub.U16	q7, q13, q5\n\t"
#endif
// uv blend
		"vmul.U16	q8, q8, q4\n\t"
		"vmul.U16	q9, q9, q4\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		"vadd.U16	q10, q8, q9\n\t"

		"vld1.8 {d18-d19}, [r1]\n\t"

		"vshl.U16	q8, q9, #8\n\t"
		"vshr.U16	q8, q8, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t"

		"vmul.U16	q8, q8, q6\n\t"
		"vmul.U16	q9, q9, q6\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		//"vmin.U16	q9, q13\n\t"  // vmax = shr + shl
		"vadd.U16	q9, q8, q9\n\t"

		"vadd.U16	q9, q9, q10\n\t"
		"vst1.8 {d18-d19}, [r1]!\n\t"

// y0
		"vld1.8 {d18-d19}, [r2]!\n\t"

		"vshl.U16	q8, q9, #8\n\t"
		"vshr.U16	q8, q8, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t"

		"vmul.U16	q8, q8, q4\n\t"
		"vmul.U16	q9, q9, q5\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		"vadd.U16	q10, q8, q9\n\t"

		"vld1.8 {d18-d19}, [r3]\n\t"

		"vshl.U16	q8, q9, #8\n\t"
		"vshr.U16	q8, q8, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t"

		"vmul.U16	q8, q8, q6\n\t"
		"vmul.U16	q9, q9, q7\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		"vadd.U16	q9, q8, q9\n\t"

		"vadd.U16	q9, q9, q10\n\t"
		"vst1.8 {d18-d19}, [r3]!\n\t"

// y1
#if ALPHA_CALC
		"vqsub.U16	q4, q3, q15\n\t" // v - v_low
		"vmul.U16	q4, q4, q14\n\t"
		"vmin.U16	q4, q4, q13\n\t" // fg alpha
		"vqsub.U8	q6, q13, q4\n\t" // bg alpha

		"vqsub.U16	q1, q2, q15\n\t" // v - v_low
		"vmul.U16	q1, q1, q14\n\t"
		"vmin.U16	q5, q1, q13\n\t" // fg alpha
		"vsub.U16	q7, q13, q5\n\t"
#endif	

		"vld1.8 {d18-d19}, [r4]!\n\t"

		"vshl.U16	q8, q9, #8\n\t"
		"vshr.U16	q8, q8, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t"

		"vmul.U16	q8, q8, q4\n\t"
		"vmul.U16	q9, q9, q5\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		"vadd.U16	q10, q8, q9\n\t"

		"vld1.8 {d18-d19}, [r5]\n\t"

		"vshl.U16	q8, q9, #8\n\t"
		"vshr.U16	q8, q8, #8\n\t"
		"vshr.U16	q9, q9, #8\n\t"

		"vmul.U16	q8, q8, q6\n\t"
		"vmul.U16	q9, q9, q7\n\t"

		"vshr.U16	q8, #8\n\t"
		"vshr.U16	q9, #8\n\t"
		"vshl.U16	q9, #8\n\t"
		"vadd.U16	q9, q8, q9\n\t"

		"vadd.U16	q9, q9, q10\n\t"
		"vst1.8 {d18-d19}, [r5]!\n\t"

//---------------------------matting  end ---------------------------

		"sub	r7, #16\n\t"  // r7(width) -= 14
		"cmp	r7, #0\n\t"   // r7 == 0
		"bne	.wLoop\n\t"   // not eq, goto wLoop 
//---------------------------for(w=1920; w>0; w++) end---------------------------
		"sub	r6, #1\n\t"   // r6(height) -= 1
		"cmp	r6, #2\n\t"   // r6 == 0

		"add	r2, r2, #1920\n\t" //skip to next line
		"add	r3, r3, #1920\n\t" //skip to next line

		"bne	.hLoop\n\t"   // not eq, goto hLoop
//---------------------------for(h=540; h>0; h++) end---------------------------

// todo last line
		".theEnd:\n\t"
		: 
		: "r"(pBY), "r"(pFY), "r"(pBC), "r"(pFC)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "memory"
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
	int i;

	for(i=1920; i<1920+16; i++)
	{
		pFC[i] = 1920+255 - i*16;
	}
	pFC[1920] = 0xFF;
	pFC[1922] = 0xe0;
	pFC[1924] = 0xc0;
	pFC[1926] = 0xa0;
	pFC[1928] = 0x80;
	pFC[1930] = 0x40;
	pFC[1932] = 0x20;
	pFC[1934] = 0x00;
	#if 0
	// V
	pFC[0]  = 0x00;
	pFC[2]  = 0x02;
	pFC[4]  = 0x04;
	pFC[6]  = 0x06;

	pFC[8]  = 0x08;
	pFC[10] = 0x0a;
	pFC[12] = 0x0c;
	pFC[14] = 0x0e;

	// U
	pFC[1]  = 0x01;
	pFC[3]  = 0x03;
	pFC[5]  = 0x05;
	pFC[7]  = 0x07;

	pFC[9]  = 0x09;
	pFC[11] = 0x0b;
	pFC[13] = 0x0d;
	pFC[15] = 0x0f;
	
	pFC[16] = 0x10;
	pFC[17] = 0x11;
	
	pFC[1918] = 0x18;
	pFC[1919] = 0x19;
	pFC[1920] = 0x20;
	pFC[1921] = 0x21;
	pFC[1922] = 0x22;
	pFC[1923] = 0x23;
	pFC[1924] = 0x24;
	pFC[1925] = 0x25;
	
	pFC[3838] = 0x28;
	pFC[3839] = 0x29;
	pFC[3840] = 0x30;
	pFC[3841] = 0x31;
	pFC[3842] = 0x32;
	pFC[3843] = 0x33;
	pFC[3844] = 0x34;
	pFC[3845] = 0x35;

	#endif
	pFY[0] = 0xFF;
	pFY[1] = 0xFF;
#endif

	fastMatting(pFC, pBC, pFY, pBY);

#if NEON_DEBUG
	
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
#if 0
	usleep(100000);
	
	HI_U32 u32PhyAddr_LAB = 0;
	HI_U8 *pLab = 0;
	HI_U8 *pL = 0;
	HI_U8 *pA = 0;
	HI_U8 *pB = 0;
	
	HI_U32 u32PhyAddr_HSV = 0;
	HI_U8 *pHsv = 0;
	HI_U8 *pH = 0;
	HI_U8 *pS = 0;
	HI_U8 *pV = 0;
	
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr_LAB, (void **)&pLab, "", "", 1920*1080*3);
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr_HSV, (void **)&pHsv, "", "", 1920*1080*3);

	s32Ret = HI_MPI_VPSS_GetChnFrame(5, 0, &stFrame_fg, 1000);
	if(HI_SUCCESS == s32Ret)
	{
		img2Lab(&stFrame_fg, u32PhyAddr_LAB);
		pL = pLab;
		pA = pLab+1920*1080;
		pB = pLab+1920*1080*2;
		
		img2Hsv(&stFrame_fg, u32PhyAddr_HSV);
		pH = pHsv;
		pS = pHsv+1920*1080;
		pV = pHsv+1920*1080*2;

		HI_U8 *pFY = (HI_U8 *)HI_MPI_SYS_MmapCache(stFrame_fg.stVFrame.u32PhyAddr[0], 3110400);
		HI_U8 *pFC = pFY + 1920*1080;

		HI_U8 *pBY = (HI_U8 *)HI_MPI_SYS_MmapCache(stFrame_fg.stVFrame.u32PhyAddr[0], 3110400);
		HI_U8 *pBC = pBY + 1920*1080;
#if 0
		int i;

		printf("\n");

		for(i=960*270; i<960*271; i++)
		{
			printf("%3d\t%3d\n", pFC[2*i], pFC[2*i+1]);
		}

		printf("\n");
#endif
#if 0
		int i, j;
		
		printf("\n");

		for(i=1920*540; i<1920*541; i++)
		{
			//if(pH[i]>85 && pH[i]<106)
			{
				//printf("%d\t%02d\t%02d\t%02d\n", i%1920, pH[i]*360/256, pS[i]*100/256, pV[i]*100/256);
				printf("%d\t%d\t%d\t%d\n", i%1920, pH[i]*360/256, pS[i]*100/256,	pA[i]);
			}
		}
#endif
#if 0
		int i;
		int t;

		for(i=0; i<960*540; i++)
		{
			//pBC[2*i+1] = (pBC[2*i] * 159 + 5400)/201;
			pBC[2*i+1] >>= 1;
		}
#endif

		matting(&stFrame_fg, &stFrame_bg);

		HI_MPI_SYS_MflushCache(stFrame_fg.stVFrame.u32PhyAddr[0], pBY, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
		HI_MPI_SYS_Munmap(pFY, 3110400);
		HI_MPI_SYS_Munmap(pBY, 3110400);
		
		HI_MPI_VPSS_SendFrame(12, &stFrame_fg, 1000);
	}

	return NULL;
#endif
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

