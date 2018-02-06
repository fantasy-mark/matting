#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "arm_neon.h"
#include "mpp.h"
#include "matting.h"

// bg, normal camera; fg green camera;
unsigned int y_thresh = 0x00a8;
unsigned int u_thresh = 0x0078;
unsigned int v_thresh = 0x003c;
unsigned int i_thresh = 0x0044;
unsigned int o_thresh = 0x0006;

static void __attribute__ ((noinline)) fastMatting(void *pBY/*r0*/, void *pFY/*r1*/, void *pFC/*r2*/)
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

		"mov	r4, r2\n\t"
		"ldr	r5, =2073600\n\t"
		"add	r2, r0, r5\n\t"
		"add	r3, r1, r5\n\t"
		"sub	r4, #1\n\t"
		"mov	r5, r2\n\t"

//---------------------------for(h=540; h>0; h++) start ---------------------------	
		"mov	r6, #540\n\t"	// reset height r6 = 540
		".hLoop:\n\t"

//---------------------------for(w=1920; w>0; w++) start---------------------------
		"mov	r7, #1920\n\t"	// reset width r7 = 1920
		".wLoop:\n\t"

//---------------------------matting start---------------------------
	
// Y0
		"vld1.8 {d2-d3}, [r4]!\n\t"
		"vld1.8 {d4-d5}, [r4]!\n\t"
		"vld1.8 {d6-d7}, [r1]!\n\t" // fg
		"vld1.8 {d8-d9}, [r0]\n\t"

#if 1
		"vshr.U16	q1, #8\n\t"
		"vshr.U16	q2, #8\n\t"

		"vqsub.U16	q1, q1, q15\n\t"
		"vqsub.U16	q2, q2, q15\n\t"

		"vmul.I16	q1, q1, q14\n\t"
		"vmul.I16	q2, q2, q14\n\t"

		"vcge.U16	q1, q1, q13\n\t"
		"vcge.U16	q2, q2, q13\n\t"

		"vshl.U16	q1, q1, #8\n\t"
		"vshl.U16	q2, q2, #8\n\t"

		"vshr.U16	q1, q1, #8\n\t"
		"vshr.U16	q2, q2, #8\n\t"

		"vsub.U16 q11, q13, q1\n\t"
		"vsub.U16 q12, q13, q2\n\t"

		"vmovl.U8	q6, d6\n\t"
		"vmovl.U8	q8, d8\n\t"
		"vmul.U16	q6, q6, q1\n\t"
		"vmla.U16	q6, q8, q11\n\t"
		"vshr.U16	q6, #8\n\t"
		"vmovn.U16	d8, q6\n\t"

		"vmovl.U8	q7, d7\n\t"
		"vmovl.U8	q9, d9\n\t" 	
		"vmul.U16	q7, q7, q2\n\t"
		"vmla.U16	q7, q9, q12\n\t"
		"vshr.U16	q7, #8\n\t"
		"vmovn.U16	d9, q7\n\t"
#endif
		"vst1.8 {d8-d9}, [r0]!\n\t"
// Y1
		"vld1.8 {d2-d3}, [r4]!\n\t"
		"vld1.8 {d4-d5}, [r4]!\n\t"
		"vld1.8 {d6-d7}, [r1]!\n\t" // fg
		"vld1.8 {d8-d9}, [r0]\n\t"
#if 1
		"vshr.U16	q1, #8\n\t"
		"vshr.U16	q2, #8\n\t"

		"vqsub.U16	q1, q1, q15\n\t"
		"vqsub.U16	q2, q2, q15\n\t"

		"vmul.I16	q1, q1, q14\n\t"
		"vmul.I16	q2, q2, q14\n\t"

		"vcge.U16	q1, q1, q13\n\t"
		"vcge.U16	q2, q2, q13\n\t"

		"vshl.U16	q1, q1, #8\n\t"
		"vshl.U16	q2, q2, #8\n\t"

		"vshr.U16	q1, q1, #8\n\t"
		"vshr.U16	q2, q2, #8\n\t"
		
		"vsub.U16 q11, q13, q1\n\t"
		"vsub.U16 q12, q13, q2\n\t"

		"vmovl.U8	q6, d6\n\t"
		"vmovl.U8	q8, d8\n\t"
		"vmul.U16	q6, q6, q1\n\t"
		"vmla.U16	q6, q8, q11\n\t"
		"vshr.U16	q6, #8\n\t"
		"vmovn.U16	d8, q6\n\t"
		
		"vmovl.U8	q7, d7\n\t"
		"vmovl.U8	q9, d9\n\t" 	
		"vmul.U16	q7, q7, q2\n\t"
		"vmla.U16	q7, q9, q12\n\t"
		"vshr.U16	q7, #8\n\t"
		"vmovn.U16	d9, q7\n\t"
#endif
		"vst1.8 {d8-d9}, [r0]!\n\t"
// UV
		"vld1.8 {d6-d7}, [r3]!\n\t" // fg
		"vld1.8 {d8-d9}, [r2]\n\t"
#if 1
		"vshl.U16	q1, q3, #8\n\t"
		"vshr.U16	q1, q1, #8\n\t"

		"vqsub.U16	q1, q1, q15\n\t"
		"vmul.I16	q1, q1, q14\n\t"
		"vcge.U16	q1, q1, q13\n\t"
		"vshl.U16	q1, q1, #8\n\t"
		"vshr.U16	q1, q1, #8\n\t"
		"vshl.U16	q2, q1, #8\n\t"
		"vorr.U16	q7, q2, q1\n\t"
	
		"vmovl.U8	q1, d14\n\t"
		"vmovl.U8	q2, d15\n\t"
		
		"vsub.U16 q11, q13, q1\n\t"
		"vsub.U16 q12, q13, q2\n\t"

		"vmovl.U8	q6, d6\n\t"
		"vmovl.U8	q8, d8\n\t"
		"vmul.U16	q6, q6, q1\n\t"
		"vmla.U16	q6, q8, q11\n\t"
		"vshr.U16	q6, #8\n\t"
		"vmovn.U16	d8, q6\n\t"
		
		"vmovl.U8	q7, d7\n\t"
		"vmovl.U8	q9, d9\n\t" 	
		"vmul.U16	q7, q7, q2\n\t"
		"vmla.U16	q7, q9, q12\n\t"
		"vshr.U16	q7, #8\n\t"
		"vmovn.U16	d9, q7\n\t"
#endif
		"vst1.8 {d8-d9}, [r2]!\n\t"

//---------------------------matting  end ---------------------------

		"sub	r7, #16\n\t"  // r7(width) -= 16
		"cmp	r7, #0\n\t"   // r7 == 0		
		"bne	.wLoop\n\t"   // not eq, goto wLoop	
//---------------------------for(w=1920; w>0; w++) end---------------------------

		"sub	r6, #1\n\t"   // r6(height) -= 1
		"cmp	r6, #0\n\t"   // r6 == 0

		"bne	.hLoop\n\t"   // not eq, goto hLoop
		".theEnd:\n\t"
//---------------------------for(h=540; h>0; h++) end---------------------------
		: 
		: "r"(pFC), "r"(pFY), "r"(pBY)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "memory"
	);
}

#include "mpi_ive.h"
#include "hi_ive.h"
#include "hi_comm_ive.h"

static int imgSub(IVE_HANDLE *pIveHandle, HI_U32 u32PhyAddr_A, HI_U32 u32PhyAddr_B, HI_U32 u32PhyAddr_C)
{
	HI_S32 s32Ret = HI_SUCCESS;

	IVE_SUB_CTRL_S ctrl;// = {0};
	IVE_IMAGE_S src1;// = {0};
	IVE_IMAGE_S src2;// = {0};
	IVE_IMAGE_S dst;// = {0};

	ctrl.enMode = IVE_SUB_MODE_ABS;

	src1.enType = IVE_IMAGE_TYPE_U8C1;
	src1.u16Width = 1920;
	src1.u16Height = 1080;
	src1.u16Stride[0] = 1920;
	src1.u32PhyAddr[0] = u32PhyAddr_A;

	src2.enType = IVE_IMAGE_TYPE_U8C1;
	src2.u16Width = 1920;
	src2.u16Height = 1080;
	src2.u16Stride[0] = 1920;
	src2.u32PhyAddr[0] = u32PhyAddr_B;

	dst.enType = IVE_IMAGE_TYPE_U8C1;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = 1920;
	dst.u32PhyAddr[0] = u32PhyAddr_C;

	s32Ret = HI_MPI_IVE_Sub(pIveHandle, &src1, &src2, &dst, &ctrl, HI_FALSE);
	if(HI_SUCCESS != s32Ret)
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_Thresh failed with 0x%x!", s32Ret);
#if 0
	src1.u16Height = 540;
	src1.u32PhyAddr[0] = u32PhyAddr_A+1920*1080;

	src2.u16Height = 540;
	src2.u32PhyAddr[0] = u32PhyAddr_B+1920*1080;

	dst.u16Height = 540;
	dst.u32PhyAddr[0] = u32PhyAddr_C+1920*1080;

	s32Ret = HI_MPI_IVE_Sub(pIveHandle, &src1, &src2, &dst, &ctrl, HI_FALSE);
	if(HI_SUCCESS != s32Ret)
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_Thresh failed with 0x%x!", s32Ret);

#endif

	return s32Ret;
}

static int imgThr(IVE_HANDLE *pIveHandle, HI_U32 u32PhyAddr_in, HI_U32 u32PhyAddr_out, int value)
{
	HI_S32 s32Ret = HI_SUCCESS;
	IVE_THRESH_CTRL_S ctrl;// = {0};
	IVE_IMAGE_S src;// = {0};
	IVE_IMAGE_S dst;// = {0};

	ctrl.enMode = IVE_THRESH_MODE_BINARY;
	ctrl.u8LowThr = value;
	ctrl.u8MinVal = 0;
	ctrl.u8MaxVal = 255;

	src.enType = IVE_IMAGE_TYPE_U8C1;
	src.u16Width = 1920;
	src.u16Height = 1080;
	src.u16Stride[0] = 1920;
	src.u32PhyAddr[0] = u32PhyAddr_in;

	dst.enType = IVE_IMAGE_TYPE_U8C1;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = 1920;
	dst.u32PhyAddr[0] = u32PhyAddr_out;

	s32Ret = HI_MPI_IVE_Thresh(pIveHandle, &src, &dst, &ctrl, HI_FALSE);
	if(HI_SUCCESS != s32Ret)
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_Thresh failed with 0x%x!", s32Ret);
	
	return s32Ret;
}

static int imgFilter(IVE_HANDLE *pIveHandle, HI_U32 u32PhyAddr)
{
	HI_S32 s32Ret = HI_SUCCESS;	
	IVE_FILTER_CTRL_S ctrl;

#if 0
	ctrl.u8Norm = 7;

    HI_S8 as8Mask[25] = 
		{1,2,3,2,1,
	    2,5,6,5,2,
	    3,6,8,6,3,
	    2,5,6,5,2,
	    1,2,3,2,1};
#else
	ctrl.u8Norm = 4;

    HI_S8 as8Mask[25] = 
		{
		0,0,0,0,0,
	    0,1,2,1,0,
	    0,2,4,2,0,
	    0,1,2,1,0,
	    0,0,0,0,0
	    };
#endif

	memcpy(ctrl.as8Mask, as8Mask, sizeof(as8Mask));

	IVE_IMAGE_S src1;// = {0};
	IVE_IMAGE_S dst1;// = {0};
		
	IVE_IMAGE_S src2;// = {0};
	IVE_IMAGE_S dst2;// = {0};
	
	src1.enType = IVE_IMAGE_TYPE_U8C1;
	src1.u16Width = 1920;
	src1.u16Height = 540;
	src1.u16Stride[0] = 1920;
	src1.u32PhyAddr[0] = u32PhyAddr;
	
	dst1.enType = IVE_IMAGE_TYPE_U8C1;
	dst1.u16Width = 1920;
	dst1.u16Height = 540;
	dst1.u16Stride[0] = 1920;
	dst1.u32PhyAddr[0] = u32PhyAddr;

	s32Ret = HI_MPI_IVE_Filter(pIveHandle, &src1, &dst1, &ctrl, HI_FALSE);
		
	if(HI_SUCCESS != s32Ret)
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_And failed with 0x%x!", s32Ret);

	return s32Ret;
}

#define WRITE_YUV 0

static VIDEO_FRAME_INFO_S *matting(VIDEO_FRAME_INFO_S *frame_bg, VIDEO_FRAME_INFO_S *frame_fg, VIDEO_FRAME_INFO_S *frame_sg)
{
	IVE_HANDLE IveHandle;
	HI_BOOL bFinish = HI_TRUE;
	FILE *fp = NULL;
	HI_U8 *pVirAddr = NULL;

	pVirAddr = HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	memset(pVirAddr, 0, 3110400);
	HI_MPI_SYS_MflushCache(frame_bg->stVFrame.u32PhyAddr[0], pVirAddr, 3110400);
	HI_MPI_SYS_Munmap(pVirAddr, 3110400);

	imgSub(&IveHandle, frame_fg->stVFrame.u32PhyAddr[0], frame_sg->stVFrame.u32PhyAddr[0], frame_bg->stVFrame.u32PhyAddr[0]);

#if WRITE_YUV
	HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE);
	fp = fopen("//mnt//disk//0.sub", "w");
	pVirAddr = HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	fwrite(pVirAddr, 3110400, 1, fp);
	HI_MPI_SYS_Munmap(pVirAddr, 3110400);
	fclose(fp);
#endif

	imgFilter(&IveHandle, frame_bg->stVFrame.u32PhyAddr[0]);

#if WRITE_YUV
	HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE);
	fp = fopen("//mnt//disk//1.sub", "w");
	pVirAddr = HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	fwrite(pVirAddr, 3110400, 1, fp);
	HI_MPI_SYS_Munmap(pVirAddr, 3110400);
	fclose(fp);
#endif

	//imgThr(&IveHandle, frame_bg->stVFrame.u32PhyAddr[0], frame_bg->stVFrame.u32PhyAddr[0], 0x16);

#if WRITE_YUV
	HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE);
	fp = fopen("//mnt//disk//2.sub", "w");
	pVirAddr = HI_MPI_SYS_MmapCache(frame_bg->stVFrame.u32PhyAddr[0], 3110400);
	fwrite(pVirAddr, 3110400, 1, fp);
	HI_MPI_SYS_Munmap(pVirAddr, 3110400);
	fclose(fp);
#endif

	
	HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE);

	return frame_bg;
}

void *mattingProcessThread(void *argv)
{
	HI_S32 count, s32Ret = HI_SUCCESS;
	struct	timezone   tz;
	struct	timeval    tv;
	struct	timeval    tv2;

	VIDEO_FRAME_INFO_S *pstFrame = NULL;

	VIDEO_FRAME_INFO_S stFrame_bg;
	VIDEO_FRAME_INFO_S stFrame_fg;
	VIDEO_FRAME_INFO_S stFrame_sg;

	framePoolInit(&stFrame_sg, -1);
	framePoolInit(&stFrame_fg, -1);

	FILE *fp = NULL;
    HI_U8 *pVirAddr = NULL;

	fp = fopen("//mnt//disk//1.nv21", "r");  // frame total 2688: 0~27, pure background; 28~~, human move

	if(NULL == fp){
		fprintf(stderr, "fopen errno = %d\n", errno);
		exit(0);
	}

	s32Ret = fseek(fp, 3110400*27, SEEK_SET);

	if(s32Ret){
		fprintf(stderr, "fseek errno = %d\n", errno);
		exit(0);
	}

	pVirAddr = HI_MPI_SYS_MmapCache(stFrame_sg.stVFrame.u32PhyAddr[0], 3110400);
	fread(pVirAddr, 3110400, 1, fp);
	HI_MPI_SYS_MflushCache(stFrame_sg.stVFrame.u32PhyAddr[0], pVirAddr, 3110400);
	HI_MPI_SYS_Munmap(pVirAddr, 3110400);

	s32Ret = fseek(fp, 3110400*100, SEEK_SET);

	count = 0;
	fprintf(stderr, "%s:%d\n", __FUNCTION__, __LINE__);
	//while(g_thread_run)
	{
		s32Ret = HI_MPI_VPSS_GetChnFrame(6, 0, &stFrame_bg, 1000);
		if(HI_SUCCESS == s32Ret)
		{

			pVirAddr = HI_MPI_SYS_MmapCache(stFrame_fg.stVFrame.u32PhyAddr[0], 3110400);
			s32Ret = fread(pVirAddr, 3110400, 1, fp); count ++;
			//fseek(fp, 3110400 , SEEK_CUR); count ++;  // 30fps or 60 fps
			HI_MPI_SYS_MflushCache(stFrame_fg.stVFrame.u32PhyAddr[0], pVirAddr, 3110400);
			HI_MPI_SYS_Munmap(pVirAddr, 3110400);

			if(count == 2600)
			{
				count = 0;
				fseek(fp, 3110400*30, SEEK_SET);
			}

			stFrame_fg.stVFrame.u64pts = stFrame_bg.stVFrame.u64pts;
			stFrame_fg.stVFrame.u32TimeRef += 2;
			//HI_MPI_VPSS_SendFrame(12, &stFrame_fg, 1000);

			gettimeofday(&tv,&tz);

			pstFrame = matting(&stFrame_bg, &stFrame_fg, &stFrame_sg);
			if(NULL != pstFrame)
			{
				gettimeofday(&tv2,&tz);
				setInfo(tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);

				fprintf(stderr, "%ld\n", tv2.tv_sec*1000000+tv2.tv_usec - tv.tv_sec*1000000 - tv.tv_usec);

				HI_MPI_VPSS_SendFrame(12, pstFrame, 1000);
			}

            printf("test matting thread\n");

			HI_MPI_VPSS_ReleaseChnFrame(0 ,0, &stFrame_bg);

#if 0
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
#endif
		}
	}

	fclose(fp);

	return NULL;
}

