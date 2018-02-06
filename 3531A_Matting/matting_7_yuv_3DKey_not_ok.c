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

#define ALPHA_TEST 1
static void __attribute__ ((noinline)) fastMatting(void *pFC/*r0*/, void *pBC/*r1*/, void *pFY/*r2*/, void *pBY/*r3*/)
{
	asm volatile(
// load global variable
		"ldr	r4, =y_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q15, r5\n\t"	// q15 y_thresh

		"ldr	r4, =u_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q14, r5\n\t"	// q14 u_thresh

		"ldr	r4, =v_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q13, r5\n\t"	// q13 v_thresh

		"ldr	r4, =i_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q12, r5\n\t"	// q12 i_thresh

		"ldr	r4, =o_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q11, r5\n\t"	// q11 o_thresh

// define constant value
		"vmov.U16	q10, #0x0100\n\t"

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
		"vld1.8 {d2-d3}, [r0]!\n\t" // q1 F VU

		"vshl.U16	q0, q1, #8\n\t" // q0 V 0xHH00
		"vshr.U16	q0, q0, #8\n\t" // q0 V 0x00HH

		"vshr.U16	q1, q1, #8\n\t" // q1 U 0x00HH

// q8 = V^2 + U^2 // for calculate alpha
		"vabd.U16	q2, q0, q13\n\t" // v - v_thresh
		"vabd.U16	q3, q1, q14\n\t" // u - u_thresh

		"vmul.U16	q2, q2, q2\n\t"
		"vmul.U16	q3, q3, q3\n\t"

		"vqadd.U16	q8, q3, q2\n\t"

// green Spillover  b > r && g> b
		// 211 * U - 179 * V - 4091 > 0  // b >  r
		"vmov.U16	q6, #0x00D3\n\t"
		"vmul.U16	q2, q1, q6\n\t" // 211 * U

		// 4091
		"vmov.U16	q5, #0x0FFF\n\t"  // 4095
		"vmov.U16	q6, #0x0004\n\t"
		"vsub.U16	q3, q5, q6\n\t"  // 4095 - 4

		"vmov.U16	q6, #0x00B3\n\t"
		"vmla.U16	q3, q0, q6\n\t" // 4091 + 179 * V

		// vcgt q2 > q3
		"vcgt.U16	q2, q2, q3\n\t" // q2 b > r

		// 32498 - 21 * U - 232 * V > 0  // g >  b
		"vmov.U16	q6, #0x0015\n\t"
		"vmul.U16	q3, q1, q6\n\t" // 21 * U

		"vmov.U16	q6, #0x00E8\n\t"
		"vmla.U16	q3, q0, q6\n\t" // 232 * V + 21 * U

		"vmov.U16	q5, #0x7E00\n\t"
		"vmov.U16	q6, #0x00F2\n\t"
		"vadd.U16	q6, q5, q6\n\t"

		"vcgt.U16	q3, q6, q3\n\t" // q3  g >  b

		"vand.U16	q2, q2, q3\n\t"

		"vmov.U16	q6, #0x8080\n\t"
		"vand.U16	q6, q2, q6\n\t"

		"vqsub.U8	q0, q0, q2\n\t"
		"vqsub.U8	q1, q1, q2\n\t"

		"vorr.U16	q0, q0, q6\n\t"
		"vorr.U16	q1, q1, q6\n\t"

// calculate alpha & blend
// (q0 v; q1 u)degreened, q8 = v^2+u^2
// y0
		"vld1.8 {d6-d7}, [r2]!\n\t" // q3 F Y line 0	

		"vshl.U16	q2, q3, #8\n\t"
		"vshr.U16	q2, q2, #8\n\t" // q2 F Y00
		"vshr.U16	q3, q3, #8\n\t" // q3 F Y01
#if ALPHA_TEST
		"vabd.U8	q4, q2, q15\n\t" // y00 - y_thresh
		//"vabd.U8	q5, q3, q15\n\t" // y01 - y_thresh

// process y0 & vu
// calculate alpha q2 -> q4

		"vmul.U16	q4, q4, q4\n\t" // F Y00^2
		"vqadd.U16	q4, q8, q4\n\t" // F Y00^2 + V^2 + V^2

		"vmovl.U16	q6, d8\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t"
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d8, q6\n\t"

		"vmovl.U16	q6, d9\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t" 	
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d9, q6\n\t"

		"vcgt.U16	q4, q4, q11\n\t" // o_thresh

		"vcgt.U16	q4, q12, q4\n\t" // i_thresh
		"vand.U16	q4, q4, q10\n\t" // fg alpha
		"vsub.U16	q6, q10, q4\n\t" // bg alpha
		
#if 0
		"vmov.U16	q2, #0x0000\n\t"	
		"vst1.8 {d4-d5}, [r1]!\n\t"
		"vst1.8 {d4-d5}, [r1]!\n\t"
		"vst1.8 {d4-d5}, [r1]!\n\t"
		"vst1.8 {d4-d5}, [r1]!\n\t"
		"vst1.8 {d4-d5}, [r1]!\n\t"
		"vst1.8 {d4-d5}, [r1]!\n\t"

		"b .theEnd\n\t"	
#endif

#else
		"vmov.U16	q4, #0x00F0\n\t"
		"vmov.U16	q6, #0x0010\n\t"
#endif

// uv blend
		"vmul.U16	q0, q0, q6\n\t"
		"vmul.U16	q1, q1, q6\n\t"
		"vshr.U16	q0, #8\n\t"
		"vshr.U16	q1, #8\n\t"

		"vshl.U16	q1, #8\n\t" 	// *256
		"vadd.U16	q7, q0, q1\n\t"

		"vld1.8 {d2-d3}, [r1]\n\t" // q5 B C
		"vshl.U16	q0, q1, #8\n\t"
		"vshr.U16	q0, q0, #8\n\t" // q4 B V
		"vshr.U16	q1, q1, #8\n\t" // q5 B U

		"vmul.U16	q0, q0, q4\n\t"
		"vmul.U16	q1, q1, q4\n\t"
		"vshr.U16	q0, #8\n\t"
		"vshr.U16	q1, #8\n\t"

		"vshl.U16	q1, #8\n\t" 	// *256
		"vadd.U16	q0, q0, q1\n\t"

		"vadd.U16	q0, q0, q7\n\t"

		"vst1.8 {d0-d1}, [r1]!\n\t"

// y00 blend
		"vld1.8 {d2-d3}, [r3]\n\t" // q1 B Y line 0

		"vshl.U16	q0, q1, #8\n\t"
		"vshr.U16	q0, q0, #8\n\t" // q0 B Y00

		"vshr.U16	q1, q1, #8\n\t" // q1 B Y01

		// y00 alpha blend FY00 * fg alpha + BY00 * bg alpha
		"vmul.U16	q2, q2, q6\n\t"
		"vmul.U16	q0, q0, q4\n\t"
		"vadd.U16	q0, q2, q0\n\t"
		"vshr.U16	q0, #8\n\t"

// y01  q3 -> q5{d10, d11}
#if ALPHA_TEST
		"vabd.U8	q4, q3, q15\n\t" // y00 - y_thresh

// process y0 & vu
// calculate alpha q2 -> q4

		"vmul.U16	q4, q4, q4\n\t" // F Y00^2
		"vqadd.U16	q4, q8, q4\n\t" // F Y00^2 + V^2 + V^2

		"vmovl.U16	q6, d8\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t"
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d8, q6\n\t"

		"vmovl.U16	q6, d9\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t"
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d9, q6\n\t"
	
	"vcgt.U16	q4, q4, q11\n\t" // o_thresh

		"vcgt.U16	q4, q12, q4\n\t" // i_thresh
		"vand.U16	q4, q4, q10\n\t" // fg alpha
		"vsub.U16	q6, q10, q4\n\t" // bg alpha
#else
		"vmov.U16	q4, #0x00F0\n\t"
		"vmov.U16	q6, #0x0010\n\t"
#endif

		// y01 alpha blend FY01 * fg alpha + BY01 * bg alpha

		"vmul.U16	q3, q3, q6\n\t"
		"vmul.U16	q1, q1, q4\n\t"
		"vadd.U16	q1, q3, q1\n\t"
		"vshr.U16	q1, #8\n\t"

		// y00 + y01
		"vshl.U16	q1, #8\n\t" 	// *256 	
		"vadd.U16	q0, q0, q1\n\t"

		// store y0
		"vst1.8 {d0-d1}, [r3]!\n\t"

// y1 calculate alpha
		"vld1.8 {d6-d7}, [r4]!\n\t" // q3 F Y line 0
		"vshl.U16	q2, q3, #8\n\t"
		"vshr.U16	q2, q2, #8\n\t" // q2 F Y00
		"vshr.U16	q3, q3, #8\n\t" // q3 F Y01

#if ALPHA_TEST
		"vabd.U8	q4, q2, q15\n\t" // y00 - y_thresh

// process y0 & vu
// calculate alpha q2 -> q4

		"vmul.U16	q4, q4, q4\n\t" // F Y00^2
		"vqadd.U16	q4, q8, q4\n\t" // F Y00^2 + V^2 + V^2

		"vmovl.U16	q6, d8\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t" 	
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d8, q6\n\t"

		"vmovl.U16	q6, d9\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t" 	
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d9, q6\n\t"
	
	"vcgt.U16	q4, q4, q11\n\t" // o_thresh

		"vcgt.U16	q4, q12, q4\n\t" // i_thresh
		"vand.U16	q4, q4, q10\n\t" // fg alpha
		"vsub.U16	q6, q10, q4\n\t" // bg alpha
#else
		"vmov.U16	q4, #0x00F0\n\t"
		"vmov.U16	q6, #0x0010\n\t"
#endif


		"vld1.8 {d2-d3}, [r5]\n\t" // q1 B Y line 0

		"vshl.U16	q0, q1, #8\n\t"
		"vshr.U16	q0, q0, #8\n\t" // q0 B Y00

		"vshr.U16	q1, q1, #8\n\t" // q1 B Y01

		// y00 alpha blend FY00 * fg alpha + BY00 * bg alpha
		"vmul.U16	q2, q2, q6\n\t"
		"vmul.U16	q0, q0, q4\n\t"
		"vadd.U16	q0, q2, q0\n\t"
		"vshr.U16	q0, #8\n\t"
		
#if ALPHA_TEST
		"vabd.U8	q4, q3, q15\n\t" // y00 - y_thresh

// process y0 & vu
// calculate alpha q2 -> q4

		"vmul.U16	q4, q4, q4\n\t" // F Y00^2
		"vqadd.U16	q4, q8, q4\n\t" // F Y00^2 + V^2 + V^2

		"vmovl.U16	q6, d8\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t" 	
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d8, q6\n\t"

		"vmovl.U16	q6, d9\n\t"
		"vcvt.F32.U32	q6, q6\n\t"
		"vrsqrte.F32	q6, q6\n\t"
		"vrecpe.F32 q6, q6\n\t" 	
		"vcvt.U32.F32	q6, q6\n\t"
		"vmovn.U32	d9, q6\n\t"
	
		"vcgt.U16	q4, q4, q11\n\t" // o_thresh
		"vcgt.U16	q4, q12, q4\n\t" // i_thresh
		"vand.U16	q4, q4, q10\n\t" // fg alpha
		"vsub.U16	q6, q10, q4\n\t" // bg alpha
#else
		"vmov.U16	q4, #0x00F0\n\t"
		"vmov.U16	q6, #0x0010\n\t"
#endif

		// y01 alpha blend FY01 * fg alpha + BY01 * bg alpha

		"vmul.U16	q3, q3, q4\n\t"
		"vmul.U16	q1, q1, q6\n\t"
		"vadd.U16	q1, q3, q1\n\t"
		"vshr.U16	q1, #8\n\t"

		// y00 + y01
		"vshl.U16	q1, #8\n\t" 	// *256 	
		"vadd.U16	q0, q0, q1\n\t"

		// store y0
		"vst1.8 {d0-d1}, [r5]!\n\t"

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
		: "r4", "r5","r6","r7", "memory"
	);
}

#define NEON_DEBUG 0

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_fg, VIDEO_FRAME_INFO_S *frame_bg)
{
	HI_U8 *pFY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFC = pFY + 1920*1080;
	
	HI_U8 *pBY = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBC = pBY + 1920*1080;
	
//#if NEON_DEBUG
#if 0
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

	//fastMatting(pFC, pBC, pFY, pBY);

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

	int x;
	for(i=0; i<10; i++)
	{
		memcpy(&x, pBC+i*16, 4);
		printf("\n%d", x);
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

