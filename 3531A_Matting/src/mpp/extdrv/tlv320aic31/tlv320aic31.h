/*
 * extdrv/peripheral/ada/tlv320.h for Linux .
 *
 * History: 
 *      10-April-2006 create this file
 */ 

#ifndef _INC_TLV320AIC31
#define  _INC_TLV320AIC31    

#define     tlv320aic31 't'

#define     IN2LR_2_LEFT_ADC_CTRL       _IOWR(tlv320aic31, 0x00, unsigned long)
#define     IN2LR_2_RIGTH_ADC_CTRL      _IOWR(tlv320aic31, 0x01, unsigned long)
#define     IN1L_2_LEFT_ADC_CTRL        _IOWR(tlv320aic31, 0x02, unsigned long)
#define     IN1R_2_RIGHT_ADC_CTRL       _IOWR(tlv320aic31, 0x03, unsigned long)
#define     PGAL_2_HPLOUT_VOL_CTRL      _IOWR(tlv320aic31, 0x04, unsigned long)
#define     PGAR_2_HPLOUT_VOL_CTRL      _IOWR(tlv320aic31, 0x05, unsigned long)
#define     DACL1_2_HPLOUT_VOL_CTRL     _IOWR(tlv320aic31, 0x06, unsigned long)
#define     DACR1_2_HPLOUT_VOL_CTRL     _IOWR(tlv320aic31, 0x07, unsigned long)
#define     HPLOUT_OUTPUT_LEVEL_CTRL    _IOWR(tlv320aic31, 0x08, unsigned long) 
#define     PGAL_2_HPLCOM_VOL_CTRL      _IOWR(tlv320aic31, 0x09, unsigned long)
#define     PGAR_2_HPLCOM_VOL_CTRL      _IOWR(tlv320aic31, 0x0a, unsigned long)
#define     DACL1_2_HPLCOM_VOL_CTRL     _IOWR(tlv320aic31, 0x0b, unsigned long)
#define     DACR1_2_HPLCOM_VOL_CTRL     _IOWR(tlv320aic31, 0x0c, unsigned long)
#define     HPLCOM_OUTPUT_LEVEL_CTRL    _IOWR(tlv320aic31, 0x0d, unsigned long)
#define     PGAR_2_HPROUT_VOL_CTRL      _IOWR(tlv320aic31, 0x0f, unsigned long)
#define     DACR1_2_HPROUT_VOL_CTRL     _IOWR(tlv320aic31, 0x10, unsigned long)
#define     HPROUT_OUTPUT_LEVEL_CTRL    _IOWR(tlv320aic31, 0x11, unsigned long)
#define     PGAR_2_HPRCOM_VOL_CTRL      _IOWR(tlv320aic31, 0x12, unsigned long)
#define     DACR1_2_HPRCOM_VOL_CTRL     _IOWR(tlv320aic31, 0X13, unsigned long)
#define     HPRCOM_OUTPUT_LEVEL_CTRL    _IOWR(tlv320aic31, 0x14, unsigned long)
#define     PGAL_2_LEFT_LOP_VOL_CTRL    _IOWR(tlv320aic31, 0x15, unsigned long)
#define     DACL1_2_LEFT_LOP_VOL_CTRL   _IOWR(tlv320aic31, 0x16, unsigned long)
#define     LEFT_LOP_OUTPUT_LEVEL_CTRL  _IOWR(tlv320aic31, 0x17, unsigned long)
#define     PGAR_2_RIGHT_LOP_VOL_CTRL   _IOWR(tlv320aic31, 0x18, unsigned long)
#define     DACR1_2_RIGHT_LOP_VOL_CTRL  _IOWR(tlv320aic31, 0x19, unsigned long)
#define     RIGHT_LOP_OUTPUT_LEVEL_CTRL _IOWR(tlv320aic31, 0x20, unsigned long)
#define     SET_ADC_SAMPLE              _IOWR(tlv320aic31, 0x21, unsigned long)
#define     SET_DAC_SAMPLE              _IOWR(tlv320aic31, 0x22, unsigned long)
#define     SET_DATA_LENGTH             _IOWR(tlv320aic31, 0x23, unsigned long)
#define     SET_CTRL_MODE               _IOWR(tlv320aic31, 0x24, unsigned long)
#define     LEFT_DAC_VOL_CTRL           _IOWR(tlv320aic31, 0x25, unsigned long)
#define     RIGHT_DAC_VOL_CTRL          _IOWR(tlv320aic31, 0x26, unsigned long)
#define     LEFT_DAC_POWER_SETUP        _IOWR(tlv320aic31, 0x27, unsigned long)
#define     RIGHT_DAC_POWER_SETUP       _IOWR(tlv320aic31, 0x28, unsigned long)
#define     DAC_OUT_SWITCH_CTRL         _IOWR(tlv320aic31, 0x29, unsigned long)
#define     LEFT_ADC_PGA_CTRL           _IOWR(tlv320aic31, 0x30, unsigned long)
#define     RIGHT_ADC_PGA_CTRL          _IOWR(tlv320aic31, 0x31, unsigned long)
#define     TLV320AIC31_REG_DUMP        _IOWR(tlv320aic31, 0x32, unsigned long)
#define     SOFT_RESET                  _IOWR(tlv320aic31, 0x33, unsigned long)
#define     SET_TRANSFER_MODE           _IOWR(tlv320aic31, 0x34, unsigned long)
#define     SET_SERIAL_DATA_OFFSET      _IOWR(tlv320aic31, 0X35, unsigned long)
#define     LEFT_DAC_GET_VOLUME         _IOWR(tlv320aic31, 0X36, unsigned long)
#define     RIGHT_DAC_GET_VOLUME        _IOWR(tlv320aic31, 0X37, unsigned long)

/*
0: ADC Fs = Fsref/1     48        44
1: ADC Fs = Fsref/1.5    32    
2: ADC Fs = Fsref/2        24        22
3: ADC Fs = Fsref/2.5    20
4: ADC Fs = Fsref/3        16
5: ADC Fs = Fsref/3.5    13.7
6: ADC Fs = Fsref/4        12        11
7: ADC Fs = Fsref/4.5    10.6
8: ADC Fs = Fsref/5        9.6
9: ADC Fs = Fsref/5.5
a: ADC Fs = Fsref / 6    8
*/
#define     AC31_SET_8K_SAMPLERATE      0xa
#define     AC31_SET_12K_SAMPLERATE     0x6
#define     AC31_SET_16K_SAMPLERATE                    0x4
#define     AC31_SET_24K_SAMPLERATE                    0x2
#define     AC31_SET_32K_SAMPLERATE                    0x1
#define     AC31_SET_48K_SAMPLERATE                    0x0

#define     AC31_SET_11_025K_SAMPLERATE                    0x6
#define     AC31_SET_22_05K_SAMPLERATE                    0x2
#define     AC31_SET_44_1K_SAMPLERATE                    0x0

#define     AC31_SET_SLAVE_MODE                      0
#define     AC31_SET_MASTER_MODE                     1

#define     AC31_SET_16BIT_WIDTH                      0
#define     AC31_SET_20BIT_WIDTH                      1
#define     AC31_SET_24BIT_WIDTH                      2
#define     AC31_SET_32BIT_WIDTH                      3        




typedef enum Audio_In_
{
    IN1L =0,
    IN1R =1,
    IN2L =2,
    IN2R =3,
}Audio_In;
typedef enum Audio_Out_
{
    LINE_OUT_R=0,
    LINE_OUT_L,
    HPL,
    HPR,
}Audio_Out;
typedef struct 
{
    unsigned int chip_num:3;
    unsigned int audio_in_out:2;
    unsigned int if_mute_route:1;
    unsigned int if_powerup:1;
    unsigned int input_level:7;
    unsigned int sample:4;
    unsigned int sampleRate:4;
    unsigned int if_44100hz_series:1;
    unsigned int data_length:2;
    unsigned int ctrl_mode:1;
    unsigned int dac_path:2;
    unsigned int trans_mode:2;
    unsigned int reserved :6;
    unsigned int data_offset:8;
    unsigned int left_adc_volume:7;
    unsigned int right_adc_volume:7;
}Audio_Ctrl;



typedef union{
    struct
    {
        unsigned char reserved2 :4;
        unsigned char bit_work_dri_ctrl:1;
        unsigned char reserved1:1;
        unsigned char work_clock_dic_ctrl:1;
        unsigned char bit_clock_dic_ctrl:1;
    }bit;
    unsigned char b8;
}Ctrl_Mode;

typedef union{
    struct
    {
        unsigned char input_vol_level_ctrl:7;
        unsigned char if_mute_route:1;
    }bit;
    unsigned char b8;
}Adc_Pga_Dac_Gain_Ctrl;
    
typedef union{
    struct 
    {   
        unsigned char in2r_adc_input_level_sample:4;
        unsigned char in2l_adc_input_level_sample:4;
    }bit;
    unsigned char b8;
}In2_Adc_Ctrl_Sample;

typedef union{
    struct
    {
        unsigned adc_pga_step_ctrl:2;
        unsigned adc_ch_power_ctrl:1;
        unsigned char in1_adc_input_level:4;
        unsigned char mode:1;
    }bit;
    unsigned char b8;
}In1_Adc_Ctrl;

typedef union{
    struct
    {
        unsigned char reserved:4;
        unsigned char data_length:2;  
        unsigned char transfer_mode:2;                      
    }bit;
    unsigned char b8;
}Serial_Int_Ctrl;

typedef union{
    struct
    {
        unsigned char power_status:1;
        unsigned char vol_ctrl_status:1;
        unsigned char power_down_ctrl:1;
        unsigned char if_mute:1;
        unsigned char output_level:4;
    }bit;
    unsigned char b8;
}Line_Hpcom_Out_Ctrl;
typedef union{
    struct
    {
        unsigned char reserved1:1;
        unsigned char right_dac_datapath_ctrl:2;
        unsigned char left_dac_datapath_ctrl:2;
        unsigned char reserved2:3;
    }bit;
    unsigned char b8;
}Codec_Datapath_Setup_Ctrl;
typedef union{
    struct
    {
        unsigned char reserved:6;
        unsigned char right_dac_power_ctrl:1;
        unsigned char left_dac_power_ctrl:1;
    }bit;
    unsigned char b8;
}DAC_POWER_CTRL;
typedef union{
    struct
    {
        unsigned char reserved:4;
        unsigned char right_dac_swi_ctrl:2;
        unsigned char left_dac_swi_ctrl:2;
    }bit;
    unsigned char b8;
}DAC_OUTPUT_SWIT_CTRL;

typedef union{
    struct
    {
        unsigned char serial_data_offset:8;
    }bit;
    unsigned char b8;
}Serial_Data_Offset_Ctrl;
#endif
