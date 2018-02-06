#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>

#include "gv7704_base.h"

static int         gv7704_fd        = -1;
static gv7704_info gv7704_data      = {0};
static time_t      gv7704_last_read = 0;

int gv7704_read(int sc, sdi_info_s *info)
{
	int ret = 0;
	time_t time_now;
	
	if(NULL == info)
		return -1;

	if(gv7704_fd < 0)
	{
		gv7704_fd = open("/dev/" DEV_NAME, O_RDWR);
		if(gv7704_fd<0)
			return -1;
	}
	
	time_now = time(NULL);
	if(time_now - gv7704_last_read > 1)
	{
		memset(&gv7704_data, 0, sizeof(gv7704_info));
		ret = ioctl(gv7704_fd, GET_STREAM_ID, &gv7704_data);
		gv7704_last_read = time_now;
		
		if(ret!=0)
			return ret;
	}

	memset(info, 0, sizeof(sdi_info_s));
	
	info->mode = 1;
	
	switch(gv7704_data.sf[sc])
	{
	case F720P23:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 24;  // 23.976
		break;
	case F720P24:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 25;  // 24.97
		break;
	case F720P25:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 25;
		break;
	case F720P29:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 30;  // 29.97
		break;
	case F720P30:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 30;
		break;
	case F720P50:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 50;
		break;
	case F720P59:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 60;  // 59.94
		break;
	case F720P60:
		info->width      = 1280;
		info->height     = 720;
		info->frame_rate = 60;
		break;
	case F1080P23:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 24;  // 23.976
		break;
	case F1080P24:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 25;  // 24.97
		break;
	case F1080P25:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 25;
		break;
	case F1080P29:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 30;  // 29.97
		break;
	case F1080P30:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 30;
		break;
	case F1080I50: // 1080 i50
		info->mode       = 0;
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 25;
		break;
	case F1080I59: // 1080 i59.94
		info->mode       = 0;
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 30;  // 59.94
		break;
	case F1080I60: // 1080 i60
		info->mode       = 0;
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 30;
		break;
	case F1080P50:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 50;  // 50
		break;
	case F1080P59:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 60;  // 59.94
		break;
	case F1080P60:
		info->width      = 1920;
		info->height     = 1080;
		info->frame_rate = 60;
		break;
	}
	return 0;
}

