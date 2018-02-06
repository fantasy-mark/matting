#pragma once

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define HI3531A

#define DEV_NAME    "gv7704"

#ifdef HI3531A
#define CHIP_NUM                                        0x2
#endif
#define STREAM_ON_CHIP                                  0x4

/* Important use bit operation to select gv7704 chip, even only 1 on board */
#define GV7704                                          'f'

#define TEST_SPI                                        _IOWR(GV7704, 1, int)
#define SELECT_CHIP                                     _IOWR(GV7704, 2, int)
#define GET_STREAM_ID                                   _IOWR(GV7704, 3, int)

#define F720P23                                         0x00
#define F720P24                                         0x01
#define F720P25                                         0x02
#define F720P29                                         0x03
#define F720P30                                         0x04
#define F720P50                                         0x05
#define F720P59                                         0x06
#define F720P60                                         0x07
#define F1080P23                                        0x08
#define F1080P24                                        0x09
#define F1080P25                                        0x0a
#define F1080P29                                        0x0b
#define F1080P30                                        0x0c
#define F1080I50                                        0x0d
#define F1080I59                                        0x0e
#define F1080I60                                        0x0f
#define F1080P50                                        0x10
#define F1080P59                                        0x11
#define F1080P60                                        0x12
#define INVALID_FORMAT                                  0x13


/******************************************************************************
 *
 *      Adv7441a main struct
 *      Important : driver & app must sync struct gv7704_info number order
 *
 *****************************************************************************/
typedef struct _gv7704_info {
    int sc;                                             // select chip0/chip1
    int sf[CHIP_NUM * STREAM_ON_CHIP];                  // stream fomat index
}gv7704_info;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


