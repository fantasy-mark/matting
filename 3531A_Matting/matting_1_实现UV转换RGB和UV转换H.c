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

// bg, normal camera; fg green camera;
// Y[8D~A9]
// U[78~82]
// V[32~5a]
// VUVU
// __builtin_prefetch Êý¾ÝÔ¤È¡

unsigned int y_thresh = 0x008D;
unsigned int u_thresh = 0x0082;
unsigned int v_thresh = 0x0044;

// TODO alpha blend 9ms
// r0~r7 usable

#define NEON_DEBUG 0

static void __attribute__ ((noinline)) fastMatting(void *pBY/*r0*/, void *pFY/*r1*/, void *pBUV/*r2*/, void *pFUV/*r3*/)
{
	// r0~r3 , pAY, pBY, pAUV, pBUV
	asm volatile(
// load global variable
		"ldr	r4, =v_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q11, r5\n\t"	// q11 v_thresh
		
		//"vst1.8 {d22-d23}, [r3]\n\t"  // test v_thresh

		"ldr	r4, =u_thresh\n\t"
		"ldr	r5, [r4]\n\t"
		"vdup.16	q10, r5\n\t"	// q10 u_thresh

		//"vst1.8 {d20-d21}, [r3]\n\t"  // test u_thresh

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
		"mov	r7, #1920\n\t"  // reset width r7 = 1920		
		".wLoop:\n\t"

// separate & duplicate UV
		"vld1.8 {d0-d1}, [r2]!\n\t"

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
		"vcgt.U16   q14, q14, q9\n\n"
		"vcgt.U16   q15, q15, q9\n\n"

		"vbic.I16	q14, q14, q8\n\t"
		"vbic.I16	q15, q15, q8\n\t"

// green Spillover ( V = 20-(20*U)/128 && U = 43-(43*V)/128)
	
		"vbic.I16	q1, q0, q8\n\t" // q1 V 0x00xx
		"vbic.I16	q2, q0, q9\n\t"
		"vshr.U16	q2, q2, #8\n\t" // q2 U 0x00xx

		

#if 0
		"vbic.I16	q1, q0, q8\n\t" // q1 V 0x00xx
		"vbic.I16	q2, q0, q9\n\t"
		"vshr.U16	q2, q2, #8\n\t" // q2 U 0x00xx

		//"vst1.8 {d2-d3}, [r3]!\n\t" // test q1
		//"vst1.8 {d4-d5}, [r3]!\n\t" // test q2

		"vmov.I16	q13, #0x0080\n\t"

		// U < 128
		"vmov.I16	q12, #0x0014\n\t"
		"vmul.I16	q3, q2, q12\n\t" // * 20
		"vshr.U16	q3, #7\n\t" //	/ 128
		"vqsub.U16	q3, q12, q3\n\t" // 20 -
		"vadd.U16	q3, q3, q13\n\t" // + 128  // q4 v_thresh for Spillover when  U < 128

		"vcgt.U16	q6, q13, q2\n\t" // if 0x80 > U
		"vand.U16	q3, q3, q6\n\t" // clear 0x94, when U < 128
		"vand.U16	q4, q1, q6\n\t" // clear 0x94, when U < 128

		//"vst1.8 {d6-d7}, [r3]!\n\t"  // test q3 v_thr
		//"vst1.8 {d8-d9}, [r3]!\n\t"  // test q4 v_org
		
		"vcgt.U16	q3, q3, q4\n\t"
		//"vst1.8 {d6-d7}, [r3]!\n\t"  // test q3 v_thr

	// U >= 128
		"vmov.I16	q12, #0x002b\n\t"
		"vmul.I16	q4, q1, q12\n\t" // * 43
		"vshr.U16	q4, #7\n\t" // / 128
		"vqsub.U16	q4, q12, q4\n\t" // 43 -
		"vadd.U16	q4, q4, q13\n\t"

		"vcge.U16	q6, q2, q13\n\t" // if U >= 0x80
		"vand.U16	q4, q4, q6\n\t" // clear 0x94, when U >= 128
		"vand.U16	q5, q1, q6\n\t" // clear 0x94, when U >= 128

		//"vst1.8	{d8-d9}, [r3]!\n\t" // test q3 u_thr
		//"vst1.8	{d10-d11}, [r3]!\n\t" // test q4 u_org
		
		"vcgt.U16	q5, q4, q5\n\t"	
		//"vst1.8	{d10-d11}, [r3]!\n\t" // test q4 u_org

		"vorr.U16	q5, q5, q3\n\t"
		//"vst1.8	{d10-d11}, [r3]!\n\t" // test q4 u_org

		"vmov.I8	q12, #0x7f\n\t"
		"vand.U16	q12, q12, q5\n\t"

		//"vst1.8	{d0-d1}, [r3]!\n\t" // test q0 UV_ORG
		"vqsub.U8	q0, q0, q5\n\t" // 43 -
		//"vst1.8	{d0-d1}, [r3]!\n\t" // test q0 UV_ORG -0xFF

		"vadd.U16	q0, q0, q12\n\t"
		//"vst1.8	{d0-d1}, [r3]!\n\t" // test q0 UV_RET
		
		//"b	.theEnd\n\t"
#endif
// alpha blend
		// q14 -> d0, q15 -> d1 foreground alpha value
		// q12 -> d2, q13 -> d3 background alpha value
		"vsub.U16 q12, q7, q14\n\t"
		"vsub.U16 q13, q7, q15\n\t"

//---------------------------matting start---------------------------
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
		".theEnd:"
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		: "r"(pBUV), "r"(pFUV), "r"(pBY), "r"(pFY)
		: "r4", "r5","r6","r7", "memory"
	);
}

#if 0
static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_bg, VIDEO_FRAME_INFO_S *frame_fg)
{
	HI_U8 *pB   = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBY  = pB;
	HI_U8 *pBUV = pB + 1920*1080;

	HI_U8 *pF   = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFY  = pF;
	HI_U8 *pFUV = pF + 1920*1080;
	
#if NEON_DEBUG
	// V
	pFUV[0]  = 0x00;
	pFUV[2]  = 0x7F;
	pFUV[4]  = 0x90;
	pFUV[6]  = 0xFE;

	pFUV[8]  = 0x00;
	pFUV[10] = 0x7F;
	pFUV[12] = 0x90;
	pFUV[14] = 0xFE;

	// U
	pFUV[1]  = 0x00;
	pFUV[3]  = 0x10;
	pFUV[5]  = 0x40;
	pFUV[7]  = 0x7F;

	pFUV[9]  = 0x80;
	pFUV[11] = 0xA0;
	pFUV[13] = 0xC0;
	pFUV[15] = 0xFE;
#endif

	fastMatting(pBY, pFY, pBUV, pFUV);

#if NEON_DEBUG

	int i;
	char info[12][10]={
		"v",
		"u",
		"v_thr",
		"v_cmp",
		"v_ret",
		"u_thr",
		"u_cmp",
		"u_ret",
		};
	for(i=0; i<16*12; i++)
	{		
		if(i%16 == 0)
			printf("\n%s:\t", info[i/16]);
		
		printf("%02x ", pBUV[i]); 
	}

	printf("\nend\n");
#endif

	HI_MPI_SYS_MflushCache(frame_bg->stVFrame.u32PhyAddr[0], pB, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_MflushCache(frame_fg->stVFrame.u32PhyAddr[0], pF, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_Munmap(pB, 3110400);
	HI_MPI_SYS_Munmap(pF, 3110400);
	return frame_fg;
}
#endif

static int img2Hsv(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	IVE_HANDLE IveHandle;
	HI_BOOL bFinish;
	IVE_CSC_CTRL_S ctrl = {0};
	ctrl.enMode=IVE_CSC_MODE_PIC_BT709_YUV2HSV;
	
	IVE_IMAGE_S src = {0};
	IVE_IMAGE_S dst = {0};
	
	src.enType = IVE_IMAGE_TYPE_YUV420SP;
	src.u16Width = 1920;
	src.u16Height = frame->stVFrame.u32Height;
	src.u16Stride[0] = frame->stVFrame.u32Stride[0];
	src.u32PhyAddr[0] = frame->stVFrame.u32PhyAddr[0];
	src.u16Stride[1] = frame->stVFrame.u32Stride[1];
	src.u32PhyAddr[1] = frame->stVFrame.u32PhyAddr[1];

	dst.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = 1920;
	dst.u16Stride[1] = 1920;
	dst.u16Stride[2] = 1920;
	dst.u32PhyAddr[0] = u32PhyAddr;
	dst.u32PhyAddr[1] = u32PhyAddr+1920*1080;
	dst.u32PhyAddr[2] = u32PhyAddr+1920*1080*2;
	
	s32Ret = HI_MPI_IVE_CSC(&IveHandle, &src, &dst, &ctrl, HI_FALSE);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"HI_MPI_IVE_CSC Error 0x%x\n", s32Ret);
	   //printf("HI_MPI_IVE_CSC failed with %#x!\n", s32Ret);
	   return s32Ret; 
	}
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

static int img2Lab(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	IVE_HANDLE IveHandle;
	HI_BOOL bFinish;
	IVE_CSC_CTRL_S ctrl = {0};
	ctrl.enMode=IVE_CSC_MODE_PIC_BT709_YUV2LAB;
	
	IVE_IMAGE_S src = {0};
	IVE_IMAGE_S dst = {0};
	
	src.enType = IVE_IMAGE_TYPE_YUV420SP;
	src.u16Width = 1920;
	src.u16Height = frame->stVFrame.u32Height;
	src.u16Stride[0] = frame->stVFrame.u32Stride[0];
	src.u32PhyAddr[0] = frame->stVFrame.u32PhyAddr[0];
	src.u16Stride[1] = frame->stVFrame.u32Stride[1];
	src.u32PhyAddr[1] = frame->stVFrame.u32PhyAddr[1];

	dst.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = 1920;
	dst.u16Stride[1] = 1920;
	dst.u16Stride[2] = 1920;
	dst.u32PhyAddr[0] = u32PhyAddr;
	dst.u32PhyAddr[1] = u32PhyAddr+1920*1080;
	dst.u32PhyAddr[2] = u32PhyAddr+1920*1080*2;
	
	s32Ret = HI_MPI_IVE_CSC(&IveHandle, &src, &dst, &ctrl, HI_FALSE);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"HI_MPI_IVE_CSC Error 0x%x\n", s32Ret);
	   //printf("HI_MPI_IVE_CSC failed with %#x!\n", s32Ret);
	   return s32Ret; 
	}
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

static int img2Rgb(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	IVE_HANDLE IveHandle;
	HI_BOOL bFinish;
	IVE_CSC_CTRL_S ctrl = {0};
	ctrl.enMode=IVE_CSC_MODE_PIC_BT709_YUV2RGB;
	
	IVE_IMAGE_S src = {0};
	IVE_IMAGE_S dst = {0};
	
	src.enType = IVE_IMAGE_TYPE_YUV420SP;
	src.u16Width = 1920;
	src.u16Height = frame->stVFrame.u32Height;
	src.u16Stride[0] = frame->stVFrame.u32Stride[0];
	src.u32PhyAddr[0] = frame->stVFrame.u32PhyAddr[0];
	src.u16Stride[1] = frame->stVFrame.u32Stride[1];
	src.u32PhyAddr[1] = frame->stVFrame.u32PhyAddr[1];

	dst.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = 1920;
	dst.u16Stride[1] = 1920;
	dst.u16Stride[2] = 1920;
	dst.u32PhyAddr[0] = u32PhyAddr;
	dst.u32PhyAddr[1] = u32PhyAddr+1920*1080;
	dst.u32PhyAddr[2] = u32PhyAddr+1920*1080*2;
	
	s32Ret = HI_MPI_IVE_CSC(&IveHandle, &src, &dst, &ctrl, HI_FALSE);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"HI_MPI_IVE_CSC Error 0x%x\n", s32Ret);
	   //printf("HI_MPI_IVE_CSC failed with %#x!\n", s32Ret);
	   return s32Ret; 
	}
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

HI_U32 u32PhyAddr_HSV = 0;
HI_U8 *pHsv = 0;

HI_U32 u32PhyAddr_LAB = 0;
HI_U8 *pLab = 0;

HI_U32 u32PhyAddr_RGB = 0;
HI_U8 *pRgb = 0;

//H = [1.79274107 * (V-128) - 2.11240179 * (U-128)] / [2.3256504 * (V-128) + 0.21324861 * (U-128)]
static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_bg, VIDEO_FRAME_INFO_S *frame_fg)
{
	HI_U8 *pB   = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pBY  = pB;
	HI_U8 *pBUV = pB + 1920*1080;

	HI_U8 *pF   = (HI_U8 *)HI_MPI_SYS_MmapCache(frame_fg->stVFrame.u32PhyAddr[0], 3110400);
	HI_U8 *pFY  = pF;
	HI_U8 *pFUV = pF + 1920*1080;

#if 1
	HI_U8 *pH = pHsv;
	HI_U8 *B = pRgb;
	HI_U8 *G = pRgb+1920*1080;
	HI_U8 *R = pRgb+1920*1080*2;
	
	HI_U8 Y = 174;
	HI_U8 U = 118;
	HI_U8 V = 76;

	pFY[0]  = Y;
	pFUV[0] = V;
	pFUV[1] = U;
	HI_MPI_SYS_MflushCache(frame_fg->stVFrame.u32PhyAddr[0], pF, 3110400);

	img2Rgb(frame_fg, u32PhyAddr_RGB);
	img2Hsv(frame_fg, u32PhyAddr_HSV);

	printf("\nYUV:\t%d, %d, %d\n", pFY[0], pFUV[1], pFUV[0]);
	printf("RGB:\t%d, %d, %d\n", R[0], G[0], B[0]);
	printf("H:\t%d\n", pH[0]*360/256);
	
	printf("\n");
	printf("RGB->H:\t%d\n", (B[0]-R[0])*60/(G[0]-R[0])+120);

	int r = 1.16438356 * (Y - 16) + 1.79274107 * (V-128);
	int g = 1.16438356 * (Y - 16) - 0.21324861 * (U-128) - 0.53290933 * (V-128);
	int b = 1.16438356 * (Y - 16) + 2.11240179 * (U-128);

	printf("YUV->rgb:\t%d, %d, %d\n", r, g, b);	
	printf("rgb->H:\t%d\n", 60*(b-r)/(g-r));

	printf("YUV->H:\t%f\n",
		60 * ((1.16438356 * (Y - 16) + 2.11240179 * (U-128))-(1.16438356 * (Y - 16) + 1.79274107 * (V-128)))/
		((1.16438356 * (Y - 16) - 0.21324861 * (U-128) - 0.53290933 * (V-128))-(1.16438356 * (Y - 16) + 1.79274107 * (V-128)))
		);

	printf("YUV->H:\t%f\n", 60 * (( 2.11240179 * (U-128))-(1.79274107 * (V-128)))/(- 0.21324861 * (U-128) - 2.3256504 * (V-128)));

	printf("YUV->H:\t%f\n", 60 * ( 2.11240179 * U - 1.79274107 * V - 40.91657216)/(324.97907328 - 0.21324861 * U - 2.3256504 * V) );
	
	printf("YUV->H:\t%d\n", 60 * ( 211 * U - 179 * V - 4091)/(32498 - 21 * U - 232 * V) );
	
	printf("\n\n");
	
	// b > r && g> b
	// 211 * U - 179 * V - 4091 > 0  // b >  r
	// 32498 - 21 * U - 232 * V > 0  // g >  b

	
/*
1.16438356 0 1.79274107
1.16438356 -0.21324861 -0.53290933
1.16438356 2.11240179 0
*/
#endif

#if 0

	img2Hsv(frame_fg, u32PhyAddr_HSV);

	int i, j;
	HI_U8 *pH = pHsv;
	for(i=0; i<540; i++)
	{
		for(j=0; j<960; j++)
		{
			if(pH[2*i*1920+2*j] > 80 && pH[2*i*1920+2*j] < 128 )
			{
				pFUV[2*i*960 + 2*j] = 0x80;
				pFUV[2*i*960 + 2*j + 1] = 0x80;
			}
		}
	}
#endif
	//HI_MPI_SYS_MflushCache(frame_bg->stVFrame.u32PhyAddr[0], pB, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_MflushCache(frame_fg->stVFrame.u32PhyAddr[0], pF, 3110400); // yuv422sp 3.8ms; yuv420sp 2.85ms
	HI_MPI_SYS_Munmap(pB, 3110400);
	HI_MPI_SYS_Munmap(pF, 3110400);
	return frame_fg;
}



void *mattingProcessThread(void *argv)
{
	HI_S32 s32Ret = HI_SUCCESS;
	struct	timezone   tz;
	struct	timeval    tv;
	struct	timeval    tv2;

	VIDEO_FRAME_INFO_S stFrame_img;
	VIDEO_FRAME_INFO_S stFrame_fg;
	VIDEO_FRAME_INFO_S stFrame_bg;
	VIDEO_FRAME_INFO_S *pstFrame = NULL;

	initBackgroundFrame(&stFrame_img, "/mnt/disk/green.yuv");

	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr_HSV, (void **)&pHsv, "", "", 1920*1080*3);
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr_LAB, (void **)&pLab, "", "", 1920*1080*3);
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr_RGB, (void **)&pRgb, "", "", 1920*1080*3);
	
	img2Hsv(&stFrame_img, u32PhyAddr_HSV);
	img2Lab(&stFrame_img, u32PhyAddr_LAB);
	img2Rgb(&stFrame_img, u32PhyAddr_RGB);

//	while(g_thread_run)
	{
		s32Ret = HI_MPI_VPSS_GetChnFrame(0, 0, &stFrame_bg, 1000);
		if(HI_SUCCESS == s32Ret)
		{
			do{
				s32Ret = HI_MPI_VPSS_GetChnFrame(5, 0, &stFrame_fg, 1000);
			}while(s32Ret != HI_SUCCESS);

			gettimeofday(&tv,&tz);

			// TODO matting
			
			pstFrame = matting(&stFrame_bg, &stFrame_fg);
			if(NULL != pstFrame)
			{
				gettimeofday(&tv2,&tz);
				setInfo(tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);
				
				//fprintf(stderr, "%ld\n", tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);
				
				HI_MPI_VPSS_SendFrame(12, pstFrame, 1000);
			}

			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_fg);
			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_bg);
		}
	}
	
	return NULL;
}
