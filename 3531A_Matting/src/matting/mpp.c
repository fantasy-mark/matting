#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "mpi_ive.h"
#include "hi_ive.h"
#include "hi_comm_ive.h"

#include "hifb.h"
#include "mpp.h"

HI_S32 MPI_SYS_MemConfig(HI_VOID)
{
	HI_S32 i = 0;
	HI_S32 s32Ret = HI_SUCCESS;

	HI_CHAR * pcMmzName = NULL;
	MPP_CHN_S stMppChnVO;
	MPP_CHN_S stMppChnVPSS;
	MPP_CHN_S stMppChnVENC;
	MPP_CHN_S stMppChnVDEC;

	/* vdec chn config to mmz 'null' */
	for(i = 0; i < VDEC_MAX_CHN_NUM; i++)
	{
		stMppChnVDEC.enModId = HI_ID_VDEC;
		stMppChnVDEC.s32DevId = 0;
		stMppChnVDEC.s32ChnId = i;

		/*vdec*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVDEC,pcMmzName);
		if (s32Ret)
		{
			LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_SetMemConf failed!");
			return HI_FAILURE;
		}
	}

	/* vpss group config to mmz 'null' */
	for(i = 0; i < VPSS_MAX_GRP_NUM; i++)
	{
		stMppChnVPSS.enModId  = HI_ID_VPSS;
		stMppChnVPSS.s32DevId = i;
		stMppChnVPSS.s32ChnId = 0;

		/*vpss*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVPSS, pcMmzName);
		if (s32Ret)
		{
			LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_SetMemConf failed!");
			return HI_FAILURE;
		}
	}

	/* venc chn config to mmz 'null' */
	for (i = 0; i < VENC_MAX_CHN_NUM; i++)
	{
		stMppChnVENC.enModId = HI_ID_VENC;
		stMppChnVENC.s32DevId = 0;
		stMppChnVENC.s32ChnId = i;

		/*venc*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVENC, pcMmzName);
		if (s32Ret)
		{
			LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_SetMemConf failed!");
			return HI_FAILURE;
		}
	}

	/* vo config to mmz 'null' */
	stMppChnVO.enModId  = HI_ID_VOU;
	stMppChnVO.s32DevId = 0;
	stMppChnVO.s32ChnId = 0;
	s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVO, pcMmzName);
	if (s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_SetMemConf failed!");
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 MPI_SYS_Init(VB_CONF_S *pstVbConf)
{
	MPP_SYS_CONF_S stSysConf = {0};
	HI_S32 s32Ret = HI_FAILURE;

	if (NULL == pstVbConf)
	{
		LOG_MESSAGE(LOG_ERR, "param error!");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VB_SetConf(pstVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_SetConf failed!");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VB_Init();
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_Init failed!");
		return HI_FAILURE;
	}

	stSysConf.u32AlignWidth = 64;
	s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_SetConf failed!");
		return HI_FAILURE;
	}

	//vdec config
	{
		HI_S32 PicSize;
		VB_CONF_S stModVbConf;
		memset(&stModVbConf, 0, sizeof(VB_CONF_S));
		stModVbConf.u32MaxPoolCnt = 2;
		VB_PIC_BLK_SIZE(1920, 1080, PT_JPEG, PicSize);
		stModVbConf.astCommPool[0].u32BlkSize = PicSize;
		stModVbConf.astCommPool[0].u32BlkCnt  = 16;
		HI_MPI_VB_SetModPoolConf(VB_UID_VDEC, &stModVbConf);
	    HI_MPI_VB_InitModCommPool(VB_UID_VDEC);
	}

	s32Ret = HI_MPI_SYS_Init();
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_Init failed!");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_VOID MPI_SYS_Exit(void)
{
	HI_S32 i;

	HI_MPI_SYS_Exit();
	for(i=0;i<VB_MAX_USER;i++)
	{
		HI_MPI_VB_ExitModCommPool((VB_UID_E)i);
	}
	for(i=0; i<VB_MAX_POOLS; i++)
	{
		HI_MPI_VB_DestroyPool(i);
	}
	HI_MPI_VB_Exit();
}

static HI_VOID MPI_VI_SetMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	switch (ViDev % 2)
	{
		case 0:
			pstDevAttr->au32CompMask[0] = 0x00FF0000;
			if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[0] = 0xFF000000;
				pstDevAttr->au32CompMask[1] = 0xFF000000>>8;
			}
			else
				pstDevAttr->au32CompMask[1] = 0x0;
			break;
		case 1:
			pstDevAttr->au32CompMask[0] = 0xFF0000<<8;
			pstDevAttr->au32CompMask[1] = 0x0;
			break;
		default:
			HI_ASSERT(0);
	}
}

HI_S32 MPI_VI_StartDev(VI_DEV ViDev, SIZE_S *pstSrcSize, HI_U32 u32FrameRate)
{
	HI_S32 s32Ret;

	VI_DEV_ATTR_S DEV_ATTR_SDI_1080P =
	{
		VI_MODE_BT1120_INTERLEAVED,   //Interface mode
		VI_WORK_MODE_1Multiplex,      //1-, 2-, or 4-channel multiplexed mode
		{0x00ff0000, 0},              //Component mask
		VI_CLK_EDGE_SINGLE_UP,        //Clock edge mode
		{-1, -1, -1, -1},                //Its value range is [–1, +3]. Typically, the default value –1 is recommended.
		VI_INPUT_DATA_UVUV,           //Input data sequence (only the YUV format is supported)
		{
			/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg */
			VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,
			VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
			{ 0,           0,        0,   //hsync_hfb    hsync_act    hsync_hhb
				0,            0,        0,   //vsync0_vhb vsync0_act vsync0_hhb
				0,            0,        0}   //vsync1_vhb vsync1_act vsync1_hhb
		},
		VI_PATH_BYPASS,               //This member is invalid for the Hi3536/Hi3521A/Hi3531A.
		VI_DATA_TYPE_YUV              //Input data type. Typically, the input data from thesensor is RGB data,
		//and the input data from the ADC is YUV data.
	};

	VI_DEV_ATTR_S DEV_ATTR_7441_1080P =
	{
		VI_MODE_BT1120_STANDARD,
		VI_WORK_MODE_1Multiplex,
		{0xFF000000, 0xFF0000},
		VI_CLK_EDGE_SINGLE_UP,
		{-1, -1, -1, -1},
		VI_INPUT_DATA_UVUV,
		{
			VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,
			VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
			{0,           1920,        0,
				0,           1080,        0,
				0,            0,          0}
		},
		VI_PATH_BYPASS,
		VI_DATA_TYPE_YUV
	};

	VI_DEV_ATTR_S stDevAttr;
	memset(&stDevAttr, 0, sizeof(VI_DEV_ATTR_S));

	if (ViDev < 6)
	{
		memcpy(&stDevAttr, &DEV_ATTR_SDI_1080P, sizeof(VI_DEV_ATTR_S));
		if(1920 == pstSrcSize->u32Width &&  1080 == pstSrcSize->u32Height && u32FrameRate > 30)
			stDevAttr.enClkEdge = VI_CLK_EDGE_DOUBLE;
		MPI_VI_SetMask(ViDev, &stDevAttr);
	}
	else
	{
		memcpy(&stDevAttr, &DEV_ATTR_7441_1080P, sizeof(VI_DEV_ATTR_S));
		MPI_VI_SetMask(ViDev, &stDevAttr);
	}

	s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stDevAttr);
	if (HI_SUCCESS != s32Ret )
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VI_SetDevAttr failed with %#x! ViDev=%d", s32Ret, ViDev);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_EnableDev(ViDev);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VI_EnableDev failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 MPI_VI_StartChn(VI_CHN ViChn,  SIZE_S *pstDestSize, VI_SCAN_MODE_E enScanMode)
{
	HI_S32 s32Ret;
	VI_CHN_ATTR_S stChnAttr;

	LOG_MESSAGE(LOG_DEBUG, "VI stDestSize: ViChn=%d, Width=%d, Height=%d",
			ViChn, pstDestSize->u32Width, pstDestSize->u32Height);

	memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	stChnAttr.stCapRect.s32X = 0;
	stChnAttr.stCapRect.s32Y = 0;
	stChnAttr.stCapRect.u32Width = pstDestSize->u32Width;
	stChnAttr.stCapRect.u32Height = pstDestSize->u32Height;
	stChnAttr.stDestSize.u32Width = pstDestSize->u32Width;
	stChnAttr.stDestSize.u32Height = pstDestSize->u32Height;
	stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stChnAttr.bMirror = HI_FALSE;
	stChnAttr.bFlip = HI_FALSE;
	stChnAttr.s32SrcFrameRate = -1;
	stChnAttr.s32DstFrameRate = -1;

	stChnAttr.enScanMode = enScanMode;

	s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VI_SetChnAttr failed with %#x!, ViChn=%d", s32Ret, ViChn);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_EnableChn(ViChn);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VI_EnableChn failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	if (ViChn==24)
	{
		HI_MPI_VI_SetFrameDepth(ViChn, 1);
	}

	return HI_SUCCESS;
}

HI_S32 MPI_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_SetPubAttr failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_Enable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_Enable failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 MPI_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_SetVideoLayerAttr failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_EnableVideoLayer failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 MPI_VO_StartChn(VO_LAYER VoLayer, VO_CHN VoChn, RECT_S *pRect)
{
	HI_S32 s32Ret = HI_SUCCESS;
	VO_CHN_ATTR_S stChnAttr;

	stChnAttr.stRect.s32X       = ALIGN_BACK(pRect->s32X, 2);
	stChnAttr.stRect.s32Y       = ALIGN_BACK(pRect->s32Y, 2);
	stChnAttr.stRect.u32Width   = ALIGN_BACK(pRect->u32Width, 2);
	stChnAttr.stRect.u32Height  = ALIGN_BACK(pRect->u32Height, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;

	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_SetChnAttr failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_EnableChn(VoLayer, VoChn);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_EnableChn failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 MPI_VO_HdmiStart(VO_INTF_SYNC_E enIntfSync)
{
	HI_HDMI_INIT_PARA_S stHdmiPara = {0};
	HI_HDMI_ATTR_S      stAttr = {0};
	HI_HDMI_VIDEO_FMT_E enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;

	stHdmiPara.enForceMode = HI_HDMI_FORCE_HDMI;
	stHdmiPara.pCallBackArgs = NULL;
	stHdmiPara.pfnHdmiEventCallback = NULL;
	HI_MPI_HDMI_Init(&stHdmiPara);

	HI_MPI_HDMI_Open(HI_HDMI_ID_0);

	HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

	stAttr.bEnableHdmi = HI_TRUE;

	stAttr.bEnableVideo = HI_TRUE;
	stAttr.enVideoFmt = enVideoFmt;

	stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
	stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
	stAttr.bxvYCCMode = HI_FALSE;

	stAttr.bEnableAudio = HI_TRUE;
	stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
	stAttr.bIsMultiChannel = HI_FALSE;
	stAttr.enSampleRate = HI_HDMI_SAMPLE_RATE_44K;
	stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

	stAttr.bEnableAviInfoFrame = HI_TRUE;
	stAttr.bEnableAudInfoFrame = HI_TRUE;
	stAttr.bEnableSpdInfoFrame = HI_FALSE;
	stAttr.bEnableMpegInfoFrame = HI_FALSE;

	stAttr.bDebugFlag = HI_FALSE;
	stAttr.bHDCPEnable = HI_FALSE;
	stAttr.b3DEnable = HI_FALSE;

	HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);

	HI_MPI_HDMI_Start(HI_HDMI_ID_0);
	return HI_SUCCESS;
}

HI_S32 MPI_VO_Stop(VO_DEV VoDev)
{
	HI_S32 i, s32Ret = HI_SUCCESS;

	for (i=0; i < VO_MAX_CHN_NUM; i++)
	{
		s32Ret = HI_MPI_VO_DisableChn(VoDev, i);
		if (s32Ret != HI_SUCCESS)
		{
			LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_DisableChn failed with %#x!", s32Ret);
			return HI_FAILURE;
		}
	}

	s32Ret = HI_MPI_VO_DisableVideoLayer(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VO_DisableVideoLayer failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_Disable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "VoDev failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;

}

HI_S32 SYS_init(HI_VOID)
{
	HI_S32 s32Ret = HI_SUCCESS;
	VB_CONF_S stVbConf;

	/* clear original resource */
	SYS_Exit();

	/* Init vb & sys */
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = VB_MAX_POOLS;
	stVbConf.astCommPool[0].u32BlkSize = 3133440;
	stVbConf.astCommPool[0].u32BlkCnt = 128;
	memset(stVbConf.astCommPool[0].acMmzName, 0, sizeof(stVbConf.astCommPool[0].acMmzName));

	
	s32Ret = MPI_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_SYS_Init failed!");
		return HI_FAILURE;
	}

	/* Config to mmz 'null' */
	s32Ret = MPI_SYS_MemConfig();
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_SYS_MemConfig failed!");
		return HI_FAILURE;
	}

	/* Initializes the HiMPP PTS. */
	s32Ret = HI_MPI_SYS_InitPtsBase(1);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_InitPtsBase failed with %#x", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_VOID SYS_Exit(HI_VOID)
{
	MPI_SYS_Exit();
}

HI_S32 VO_Stop(HI_VOID)
{
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = MPI_VO_Stop(VO_DEV_DHD0);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_VO_Stop failed!");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 VO_HDMI_Start(VO_DEV VoDev)
{
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = MPI_VO_HdmiStart(VO_OUTPUT_1080P60);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_VO_HdmiStart failed!");
		return HI_FAILURE;
	}
	LOG_MESSAGE(LOG_INFO, "MPI_VO_HdmiStart success!");

	return HI_SUCCESS;
}

HI_S32 VO_Start(HI_VOID)
{
	HI_S32  s32Ret = HI_SUCCESS;
	VO_PUB_ATTR_S stPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VO_Stop();

	/* start VGA & HDMI Dev */
	memset(&stPubAttr, 0, sizeof(VO_PUB_ATTR_S));
	stPubAttr.u32BgColor = 0x00000000;
	stPubAttr.enIntfType = VO_INTF_VGA;
	stPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	s32Ret = MPI_VO_StartDev(VO_DEV_DHD0, &stPubAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_VO_StartDev HDMI failed!", s32Ret);
		return HI_FAILURE;
	}

	/* start VGA & HDMI Layer */
	memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
	stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stLayerAttr.u32DispFrmRt = 60;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = 1920;
	stLayerAttr.stDispRect.u32Height  = 1080;
	stLayerAttr.stImageSize.u32Width  = 1920;
	stLayerAttr.stImageSize.u32Height  = 1080;

	s32Ret = MPI_VO_StartLayer(VO_DEV_DHD0, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_VO_StartLayer VGA failed!", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = MPI_VO_StartChn(VO_DEV_DHD0, 0, &stLayerAttr.stDispRect);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "MPI_VO_StartChn failed!");
		return HI_FAILURE;
	}

	if(stPubAttr.enIntfType & VO_INTF_HDMI)
	{
		s32Ret = VO_HDMI_Start(VO_DEV_DHD0);
		if (s32Ret != HI_SUCCESS)
		{
			LOG_MESSAGE(LOG_ERR, "VO_VGA_Start failed!");
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;

}

HI_S32 VI_Start(VI_DEV viDev, VI_CHN viChn, SIZE_S *viSize, HI_U32 viFps, VI_SCAN_MODE_E enScanMode)
{
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = MPI_VI_StartDev(viDev, viSize, viFps);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "MC_MPI_VI_StartDev failed!");
		return HI_FAILURE;
	}
	s32Ret = MPI_VI_StartChn(viChn, viSize, enScanMode);
	if (HI_SUCCESS != s32Ret)
	{
		LOG_MESSAGE(LOG_ERR, "MC_MPI_VI_StartChn failed!");
		return HI_FAILURE;
	}

	return HI_SUCCESS;

}

HI_S32 MPI_VO_BindVi(VO_LAYER VoLayer, VO_CHN VoChn, VI_CHN ViChn)
{
	MPP_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId    = HI_ID_VIU;
	stSrcChn.s32DevId   = 0;
	stSrcChn.s32ChnId   = ViChn;

	stDestChn.enModId   = HI_ID_VOU;
	stDestChn.s32ChnId  = VoChn;
	stDestChn.s32DevId  = VoLayer;

	return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

HI_S32 MPI_VI_BindVpss(VI_CHN ViChn, VPSS_GRP VpssGrp)
{
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = ViChn;

	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;

	return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

HI_S32 MPI_VO_BindVpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoLayer;
	stDestChn.s32ChnId = VoChn;

	return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

HI_S32 MPI_VPSS_SetChnMode(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, int w, int h, VPSS_CHN_MODE_E mode)
{
	HI_S32 s32Ret = HI_FAILURE;
	VPSS_CHN_MODE_S stChnMode;

	s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VpssChn, &stChnMode);
	if (s32Ret != HI_SUCCESS){
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_GetChnMode failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	stChnMode.enChnMode = mode;
	stChnMode.u32Width  = w;
	stChnMode.u32Height = h;
	stChnMode.stFrameRate.s32DstFrmRate = -1;
	stChnMode.stFrameRate.s32SrcFrmRate = -1;
	stChnMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stChnMode.enCompressMode = COMPRESS_MODE_NONE;

	s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stChnMode);
	if (s32Ret != HI_SUCCESS){
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetChnMode failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	if (VPSS_CHN_MODE_USER == mode) {
		s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, 2);
		if (s32Ret != HI_SUCCESS){
			LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetDepth failed with %#x!", s32Ret);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

HI_S32 MPI_VPSS_Start(VPSS_GRP VpssGrp, HI_S32 ViFps, HI_S32 VpssFps)
{
	HI_S32 s32Ret;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_GRP_PARAM_S stVpssParam;
	VPSS_FRAME_RATE_S stVpssFrameRate;

	memset(&stGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = 1920;
	stGrpAttr.u32MaxH = 1080;
	stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stGrpAttr.bIeEn = HI_FALSE;
	stGrpAttr.bNrEn = HI_FALSE;
	stGrpAttr.bDciEn = HI_FALSE;
	stGrpAttr.bHistEn = HI_FALSE;
	stGrpAttr.bEsEn = HI_FALSE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;

	/* create vpss group */
	s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_CreateGrp failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	/* set vpss param */
	s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	stVpssParam.u32IeStrength = 0;
	s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "failed with %#x!", s32Ret);
		return HI_FAILURE;
	}

	/* set vpss framerate*/
	stVpssFrameRate.s32SrcFrmRate = ViFps;
	stVpssFrameRate.s32DstFrmRate = VpssFps;
	s32Ret = HI_MPI_VPSS_SetGrpFrameRate(VpssGrp, &stVpssFrameRate);
	if(s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetGrpFrameRate failed! VpssGrp = %d, s32Ret=%#x", VpssGrp, s32Ret);
	}

	VpssChn = VPSS_CHN0;
	stChnAttr.bSpEn = HI_FALSE;
	stChnAttr.bUVInvert = HI_FALSE;
	stChnAttr.bBorderEn = HI_FALSE;

	s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetChnAttr failed with %#x", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetChnAttr failed with %#x", s32Ret);
		return HI_FAILURE;
	}
	
	MPI_VPSS_SetChnMode(VpssGrp, VpssChn, 1920, 1080, VPSS_CHN_MODE_USER);
	
	/* start vpss group */
	s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_VPSS_SetChnAttr failed with %#x", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 MPI_HIFB_Init(int fb, int width, int height)
{
	HI_BOOL bShow = HI_TRUE;
	int FrameBufferFd = -1;
	struct fb_var_screeninfo DefaultVinfo = {0};
	struct fb_var_screeninfo vinfo = {0};
	struct fb_fix_screeninfo fix;
	unsigned char *pShowScreen = NULL;
	HIFB_ALPHA_S stAlpha = {0};

	static struct fb_bitfield g_a32 = {24, 8, 0};
	static struct fb_bitfield g_r32 = {16, 8, 0};
	static struct fb_bitfield g_g32 = {8, 8, 0};
	static struct fb_bitfield g_b32 = {0, 8, 0};

	char FrameBufferDevFile[16];
	memset(FrameBufferDevFile, 0, sizeof(FrameBufferDevFile));
	sprintf(FrameBufferDevFile, "/dev/fb%d", fb);

	FrameBufferFd = open(FrameBufferDevFile, O_RDWR, 0);
	if(FrameBufferFd < 0)
	{
		LOG_MESSAGE(LOG_ERR, "open FrameBufferDevFile(%s) failed!", FrameBufferDevFile);
		return HI_FAILURE;
	}

	HIFB_CAPABILITY_S pstCap;
	if(ioctl(FrameBufferFd, FBIOGET_CAPABILITY_HIFB, &pstCap)<0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOGET_CAPABILITY_HIFB failed!");
		goto end;
	}

	if (ioctl(FrameBufferFd, FBIOGET_VSCREENINFO, &DefaultVinfo) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOGET_VSCREENINFO failed!");
		goto end;
	}

	DefaultVinfo.bits_per_pixel = 4 * 8;
	DefaultVinfo.transp= g_a32;
	DefaultVinfo.red = g_r32;
	DefaultVinfo.green = g_g32;
	DefaultVinfo.blue = g_b32;
	DefaultVinfo.activate = FB_ACTIVATE_FORCE;
	DefaultVinfo.xres = DefaultVinfo.xres_virtual = width;
	DefaultVinfo.yres = height;
	DefaultVinfo.yres_virtual =height * 2;

	if(ioctl(FrameBufferFd, FBIOPUT_VSCREENINFO, &DefaultVinfo) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOPUT_VSCREENINFO failed!");
		goto end;
	}

	memset(&fix, 0, sizeof(fix));
	if (ioctl(FrameBufferFd, FBIOGET_FSCREENINFO, &fix) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "InitOsd FBIOGET_FSCREENINFO failed!");
		goto end;
	}

	pShowScreen = (unsigned char *)mmap(NULL, fix.smem_len, PROT_READ|PROT_WRITE,MAP_SHARED, FrameBufferFd, 0);
	if(MAP_FAILED == pShowScreen)
	{
		LOG_MESSAGE(LOG_ERR, "mmap framebuffer failed!");
		goto end;
	}

	if (ioctl(FrameBufferFd, FBIOGET_VSCREENINFO, &vinfo) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOGET_VSCREENINFO failed!");
		goto end;
	}

	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_FALSE;
	stAlpha.u8Alpha0 = 0;  // FILLBOX_TRANSPARENT_COLOR == 0
	stAlpha.u8Alpha1 = 0xff;
	stAlpha.u8GlobalAlpha = 0xff;
	if (ioctl(FrameBufferFd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOPUT_ALPHA_HIFB failed!");
		goto end;
	}

	bShow = HI_TRUE;
	if (ioctl(FrameBufferFd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
	{
		LOG_MESSAGE(LOG_ERR, "FBIOPUT_SHOW_HIFB failed!");
		goto end;
	}

	close(FrameBufferFd);
	return HI_SUCCESS;

end:
	close(FrameBufferFd);
	return HI_FAILURE;
}

int framePoolInit(VIDEO_FRAME_INFO_S* stFrameBase, VB_POOL pool)
{
	HI_S32 s32Ret = 0;
    HI_U32 u32PhyAddr;
    HI_U8 *pVirAddr;
    HI_U32 u32Size = 3110400;
	VB_BLK blk;
    VIDEO_FRAME_INFO_S* pstVFrameInfo = stFrameBase;
    blk = HI_MPI_VB_GetBlock(pool, u32Size, NULL);
    if (VB_INVALID_HANDLE == blk)
    {
    	LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_GetBlock err! size:%d\n", u32Size);
        return -1;
    }
	
    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(blk);
    if (0 == u32PhyAddr)
    {
    	LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_Handle2PhysAddr err!\n");
        return -1;
    }
	
    pstVFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(blk);
    if (VB_INVALID_POOLID == pstVFrameInfo->u32PoolId)
    {
    	LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_Handle2PoolId err!\n");
        return -1;
    }

	pstVFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
    pstVFrameInfo->stVFrame.u32PhyAddr[1] = pstVFrameInfo->stVFrame.u32PhyAddr[0] + (1920*1080);

	pstVFrameInfo->stVFrame.u32Width  = 1920;
    pstVFrameInfo->stVFrame.u32Height = 1080;
    pstVFrameInfo->stVFrame.u32Stride[0] = 1920;
    pstVFrameInfo->stVFrame.u32Stride[1] = 1920;
    pstVFrameInfo->stVFrame.u32Stride[2] = 0;
    pstVFrameInfo->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    pstVFrameInfo->stVFrame.u32Field = VIDEO_FIELD_FRAME;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */

	pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
    if (NULL == pVirAddr)
    {
    	LOG_MESSAGE(LOG_ERR, "HI_MPI_VB_Handle2PoolId err! size:%d", u32Size);
        return -1;
    }

	pstVFrameInfo->stVFrame.pVirAddr[0] = pVirAddr;
    pstVFrameInfo->stVFrame.pVirAddr[1] = (HI_VOID *)((char *)pstVFrameInfo->stVFrame.pVirAddr[0] + (1920*1080));

    memset(pVirAddr, 0x10,1080*1920);//set Y 
	memset(pVirAddr+1080*1920, 0x80, 1080*1920/2); //set U V	

    HI_MPI_SYS_Munmap(pVirAddr, u32Size);

	return s32Ret;	 
}

int frameCacheInit(VIDEO_FRAME_INFO_S* pstFrameBase)

{
	HI_S32 s32Ret = HI_SUCCESS;

	memset(pstFrameBase, 0, sizeof(VIDEO_FRAME_INFO_S));
	
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstFrameBase->stVFrame.u32PhyAddr[0], 
		(void **)&pstFrameBase->stVFrame.pVirAddr[0], "", "", 3110400);
	
	if(s32Ret != HI_SUCCESS)
	{
    	LOG_MESSAGE(LOG_ERR, "HI_MPI_SYS_MmzAlloc_Cached failed with %#x", s32Ret);
		return s32Ret;
	}

	pstFrameBase->stVFrame.u32PhyAddr[1] = pstFrameBase->stVFrame.u32PhyAddr[0] + (1920*1080);	
	pstFrameBase->stVFrame.pVirAddr[1] = (HI_VOID *)((char *)pstFrameBase->stVFrame.pVirAddr[0] + (1920*1080));

	pstFrameBase->stVFrame.u32Width  = 1920;
	pstFrameBase->stVFrame.u32Height = 1080;
	pstFrameBase->stVFrame.u32Stride[0] = 1920;
	pstFrameBase->stVFrame.u32Stride[1] = 1920;
	pstFrameBase->stVFrame.u32Stride[2] = 0;
	pstFrameBase->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	pstFrameBase->stVFrame.u32Field = VIDEO_FIELD_FRAME;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */
	pstFrameBase->u32PoolId = -1;
	
	return s32Ret;	 
}

int initBackgroundFrame(VIDEO_FRAME_INFO_S *pFrame, char *filename)
{
	framePoolInit(pFrame, -1);

    HI_U8 *pVirAddr = NULL;
	FILE *fp = NULL;
	fp = fopen(filename, "r");

	if( NULL != fp )
	{
		pVirAddr = HI_MPI_SYS_Mmap(pFrame->stVFrame.u32PhyAddr[0], 3110400);
		fread(pVirAddr, 3110400, 1, fp);
		fclose(fp);
		HI_MPI_SYS_Munmap(pVirAddr, 3110400);
	}
	return 0;
}

int readBmp(void *pRgb, char *filename)
{
	FILE *fp = NULL;
	fp = fopen(filename, "r");
	if( NULL != fp )
	{
		fseek(fp, 54, SEEK_CUR);
		if(1==fread(pRgb, 1920*1080*3, 1, fp))
		{
    		LOG_MESSAGE(LOG_INFO, "read %s successful!\n", filename);
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}

int img2Hsv(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
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
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_CSC failed with 0x%x!", s32Ret);
		return s32Ret; 
	}
	
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

int img2Lab(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
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
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_CSC failed with 0x%x!", s32Ret);
		return s32Ret; 
	}
	
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

int img2Rgb(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr)
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
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_CSC failed with 0x%x!", s32Ret);
		return s32Ret; 
	}
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 

	return s32Ret; 
}

int img2Yuv(HI_U32 u32PhyAddr_rgb, VIDEO_FRAME_INFO_S *yuv)
{
	HI_S32 s32Ret = HI_SUCCESS;
	IVE_HANDLE IveHandle;
	HI_BOOL bFinish;
	IVE_CSC_CTRL_S ctrl ={0};
	ctrl.enMode = IVE_CSC_MODE_PIC_BT709_RGB2YUV;
	
	IVE_IMAGE_S src = {0};
	IVE_IMAGE_S dst = {0};

	src.enType     = IVE_IMAGE_TYPE_U8C3_PACKAGE;
	src.u16Width   = 1920;
	src.u16Height  = 1080;
	src.u16Stride[0]  = src.u16Width;
	src.u32PhyAddr[0] = u32PhyAddr_rgb;

	dst.enType = IVE_IMAGE_TYPE_YUV420SP;
	dst.u16Width = 1920;
	dst.u16Height = 1080;
	dst.u16Stride[0] = dst.u16Width;
	dst.u32PhyAddr[0] = yuv->stVFrame.u32PhyAddr[0];
	dst.u16Stride[1] = dst.u16Width;
	dst.u32PhyAddr[1] = yuv->stVFrame.u32PhyAddr[1];

	s32Ret = HI_MPI_IVE_CSC(&IveHandle, &src, &dst, &ctrl, HI_FALSE);
	if (s32Ret != HI_SUCCESS)
	{
		LOG_MESSAGE(LOG_ERR, "HI_MPI_IVE_CSC failed with 0x%x!", s32Ret);
		return s32Ret; 
	}
	
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, HI_TRUE); 
	return s32Ret;
}

