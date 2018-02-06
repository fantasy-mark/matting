#pragma once

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_comm_hdmi.h"
#include "hi_defines.h"
#include "hi_type.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_hdmi.h"

#include "log.h"

#define LOG_FILE_PATH     "/mnt/disk/log/director.log"
#define LOG_DIR_PATH      "/mnt/disk/log"
#define LOG_LEVEL         LOG_DEBUG
#define LOG_OUTPUT        TERMINAL_MODE
#define LOG_ROTATE        5
#define LOG_MINSIZE       3

#define IMAGE_1080P_SP420_SIZE 3159360  // 1920*1080 sp420,  VENC requiry size
#define IMAGE_1080P_SP422_SIZE 1920*1080*2
#define VO_DEV_DHD0 0

#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

HI_S32 SYS_init(HI_VOID);
HI_VOID SYS_Exit(HI_VOID);
HI_S32 VO_Start(HI_VOID);
HI_S32 VI_Start(VI_DEV viDev, VI_CHN viChn, SIZE_S *viSize, HI_U32 viFps, VI_SCAN_MODE_E enScanMode);
HI_S32 MPI_VO_BindVi(VO_LAYER VoLayer, VO_CHN VoChn, VI_CHN ViChn);
HI_S32 MPI_VI_BindVpss(VI_CHN ViChn, VPSS_GRP VpssGrp);
HI_S32 MPI_VO_BindVpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);
HI_S32 MPI_VPSS_Start(VPSS_GRP VpssGrp, HI_S32 ViFps, HI_S32 VpssFps);
HI_S32 MPI_HIFB_Init(int fb, int width, int height);

int framePoolInit(VIDEO_FRAME_INFO_S* stFrameBase, VB_POOL pool);
int frameCacheInit(VIDEO_FRAME_INFO_S* pstFrameBase);
int initBackgroundFrame(VIDEO_FRAME_INFO_S *pFrame, char *filename);
int readBmp(void *pRgb, char *filename);


int img2Hsv(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr);
int img2Lab(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr);
int img2Rgb(VIDEO_FRAME_INFO_S *frame, HI_U32 u32PhyAddr);
int img2Yuv(HI_U32 u32PhyAddr_rgb, VIDEO_FRAME_INFO_S *yuv);


