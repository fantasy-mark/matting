#ifndef __GV7704_BASE_H__
#define __GV7704_BASE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#include "gv7704.h"

typedef struct _sdi_info_s{
	int mode;  // VI_SCAN_INTERLACED  = 0, VI_SCAN_PROGRESSIVE = 1,
	int width;
	int height;
	int frame_rate;
}sdi_info_s;

int gv7704_read(int sc, sdi_info_s *info);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

