/********************************************************************************************************
 * @file     audio.c
 *
 * @brief    This is the Audio driver file for TLSR8278
 *
 * @author	 Driver Group
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/

#include "audio.h"
#include "pga.h"
#include "adc.h"
#include "irq.h"
#include "adc.h"
#include "register.h"


unsigned char AMIC_ADC_SampleLength[3] = {0xf0/*96K*/,0xac/*132K*/,0x75/*192K*/};


unsigned char CODEC_CIC_Rate[RATE_SIZE] = {	0xc9/*8k*/,
		                                    0xd5/*16k*/,
		                                    0xd9/*32k*/,
		                                    0xc1/*48k*/};

/*  matrix used under condition: I2S = 2 MHz  */
unsigned long ASDM_Rate_Matching[RATE_SIZE] = {	0x00832001/*8k */,
												0x01063001/*16k*/,
												0x020C5001/*32k*/};

/*  matrix used under condition: I2S = 2 MHz  */
unsigned long DSDM_Rate_Matching[RATE_SIZE] = {	0x00820001/*8 k*/,
												0x01058001/*16k*/,
											    0x020AF001/*32k*/};

enum {
	AUD_DMIC,
	AUD_AMIC,
};

#define SET_PFM(v)     do{\
                           BM_CLR(reg_adc_period_chn0,FLD_ADC_CHNM_PERIOD);\
	              	   	   reg_adc_period_chn0 |= MASK_VAL(FLD_ADC_CHNM_PERIOD,v);\
                       }while(0)
#define SET_PFL(v)     do{\
                           BM_CLR(reg_adc_period_chn12,FLD_ADC_CHNLR_PERIOD);\
                           reg_adc_period_chn12 |= MASK_VAL(FLD_ADC_CHNLR_PERIOD,v);\
                       }while(0)

/**
 * 	@brief     audio amic initial function. configure ADC corresponding parameters. set hpf,lpf and decimation ratio.
 * 	@param[in] Audio_Rate - audio rate value
 * 	@return    none
 */
void audio_amic_init(AudioRate_Typedef Audio_Rate)
{
	if((analog_read(pga_audio_enable) && 0x04) == 0x04)
	{
		analog_write(pga_audio_enable,analog_read(pga_audio_enable) & 0xFB);
	}

	set_pga_input_vol();

	//enable fifo auto mode,operate 0xb2c:BIT(0) to select auto/manual mode.1--manual mode,0--auto mode.
	reg_dfifo_manual_mode = FLD_AUU_FIFO_AUTO_MODE;

	reg_dfifo_mode = FLD_AUD_DFIFO0_IN;

	//ain0_sel sel i2s as input
	reg_dfifo_ain = 	  MASK_VAL( FLD_AUD_SAMPLE_TIME_CONFIG,      	0x01,\
			                        FLD_AUD_FIFO0_INPUT_SELECT, 		AUDIO_FIFO0_INPUT_SELECT_16BIT, \
			                        FLD_AUD_FIFO1_INPUT_SELECT, 		AUDIO_FIFO1_INPUT_SELECT_USB, \
			                        FLD_AUD_MIC_LEFT_CHN_SELECT, 	1, \
			                        FLD_AUD_MIC_RIGHT_CHN_SELECT,	1 );

    //open for user to config the mic vol.
	reg_mic_ctrl =    MASK_VAL( FLD_AUD_MIC_VOL_CONTROL,      	MIC_VOL_CONTROL_m16DB,\
			                    FLD_AUD_MIC_MONO_EN, 	        1, \
			                    FLD_AUD_AMIC_DMIC_SELECT,    	0 );

	if(Audio_Rate==AUDIO_48K)
	{
		reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_48K];
	}
	else
	{
		if(Audio_Rate==AUDIO_32K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_32K];//32k
		}
		else if(Audio_Rate==AUDIO_16K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_16K];//16k
		}
		else if(Audio_Rate==AUDIO_8K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_8K];//16k
		}
	}

	//enable codec
	reg_codec_dec_en = FLD_AUD_CODEC_DEC_EN;

	//set codec clk;is fixd=24MHz  codec_clk=48M*codec_step[6:0]/codec_mod[7:0]
	audio_set_codec_clk(0x81,0x02);

	//this value is fixed
	reg_set_filter_para = 0x07;// [0]:hpf_en, codec digital clock switch
}

/**
 * @brief     This function servers to close PGA input and CODEC clock.After calling the Audio module,
 *            you need to call this API to close Audio to get the current back to the call.
 *            when use this API to close Audio module,you should turn on PGA_Audio_Enable to open Audio module again!(example: turn ana_0x34<2>:1 to ana_0x34<2>:0)
 * @param[in] none.
 * @return    none.
 */
void audio_codec_and_pga_disable(void)
{
	analog_write(codec_ana_cfg3,analog_read(codec_ana_cfg3) | 0x1f);
	reg_codec_clk_step = 0x00;
}

/**
 * @brief     This function servers to receive data from buffer.
 * @param[in] buf - the buffer in which the data need to write
 * @param[in] len - the length of the buffer.
 * @return    none.
 */
void audio_rx_data_from_buff(signed char* buf,unsigned int len)
{
	signed short data;

	for (int i=0; i<len; i+=2)
	{
		if(i%4)
		{
			data = buf[i] & 0xff;
			data |= buf[i+1] << 8;
			reg_usb_mic_dat0 = data;
		}
		else
		{
			data = buf[i] & 0xff;
			data |= buf[i+1] << 8;
			reg_usb_mic_dat1 = data;
		}
	}
}

/**
 * @brief     audio DMIC init function, config the speed of DMIC and downsample audio data to required speed.
 *            actually audio data is dmic_speed/d_samp.
 * @param[in] Audio_Rate  - audio rate.
 * @return    none.
 */
void audio_dmic_init(AudioRate_Typedef Audio_Rate)
{

    //enable audio input of FIFO0
	reg_dfifo_mode |= FLD_AUD_DFIFO0_H_INT|FLD_AUD_DFIFO0_L_INT|FLD_AUD_DFIFO0_OUT|FLD_AUD_DFIFO0_IN;

	//ain0_sel sel i2s as input
	reg_dfifo_ain = 	  MASK_VAL( FLD_AUD_SAMPLE_TIME_CONFIG,      	0x01,\
			                        FLD_AUD_FIFO0_INPUT_SELECT, 		AUDIO_FIFO0_INPUT_SELECT_16BIT, \
			                        FLD_AUD_FIFO1_INPUT_SELECT, 		AUDIO_FIFO1_INPUT_SELECT_USB, \
			                        FLD_AUD_MIC_LEFT_CHN_SELECT, 	1, \
			                        FLD_AUD_MIC_RIGHT_CHN_SELECT,	1 );

    //open for user , [5:0] mic vol control. default:MIC_VOL_CONTROL_0DB
	reg_mic_ctrl =    MASK_VAL( FLD_AUD_MIC_VOL_CONTROL,      	MIC_VOL_CONTROL_m6DB,\
			                    FLD_AUD_MIC_MONO_EN, 	        1, \
			                    FLD_AUD_AMIC_DMIC_SELECT,    	1 );

	if(Audio_Rate==AUDIO_48K)
	{
		reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_48K];
	}
	else
	{
		if(Audio_Rate==AUDIO_32K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_32K];//32k
		}
		else if(Audio_Rate==AUDIO_16K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_16K];//16k
		}
		else if(Audio_Rate==AUDIO_8K)
		{
			reg_dfifo_dec_ratio = CODEC_CIC_Rate[AUDIO_8K];//16k
		}
	}

	reg_codec_dec_en = FLD_AUD_CODEC_DEC_EN;


	//set codec clk;is fixd=24MHz  codec_clk=48M*codec_step[6:0]/codec_mod[7:0]
	audio_set_codec_clk(0x81,0x02);

	//this value is fixed
	reg_set_filter_para = 0x07;// [0]:hpf_en, codec digital clock switch

}


/**
 * @brief     audio USB init function, config the speed of DMIC and downsample audio data to required speed.
 *            actually audio data is dmic_speed/d_samp.
 * @param[in] Audio_Rate  - audio rate.
 * @return    none.
 */
void audio_usb_init(AudioRate_Typedef Audio_Rate)
{
	//1. Dfifo setting
	reg_clk_en2 |= FLD_CLK2_DFIFO_EN; //enable dfifo clock, this will be initialed in cpu_wakeup_int()
	reg_dfifo_mode = FLD_AUD_DFIFO0_IN | FLD_AUD_DFIFO0_L_INT;

	//ain0_sel sel i2s as input
	reg_dfifo_ain = 	  MASK_VAL( FLD_AUD_SAMPLE_TIME_CONFIG,      	0x01,\
			                        FLD_AUD_FIFO0_INPUT_SELECT, 		AUDIO_FIFO0_INPUT_SELECT_USB, \
			                        FLD_AUD_FIFO1_INPUT_SELECT, 		AUDIO_FIFO1_INPUT_SELECT_USB, \
			                        FLD_AUD_MIC_LEFT_CHN_SELECT, 	1, \
			                        FLD_AUD_MIC_RIGHT_CHN_SELECT,	1 );

	/*******1.Dmic setting for audio input**************************/
	reg_audio_ctrl = AUDIO_OUTPUT_OFF;

	//enable codec
	reg_codec_dec_en = FLD_AUD_CODEC_DEC_EN;

	//set codec clk;is fixd=24MHz  codec_clk=48M*codec_step[6:0]/codec_mod[7:0]
	audio_set_codec_clk(0x81,0x02);

	//this value is fixed
	reg_set_filter_para = 0x07;// [0]:hpf_en, codec digital clock switch

}


/**
 * @brief     audio buff init function, config the speed of DMIC and downsample audio data to required speed.
 *            actually audio data is dmic_speed/d_samp.
 * @param[in] Audio_Rate  - audio rate.
 * @return    none.
 */
void audio_buff_init(AudioRate_Typedef Audio_Rate)
{
	//1. Dfifo setting
	reg_clk_en2 |= FLD_CLK2_DFIFO_EN; //enable dfifo clock, this will be initialed in cpu_wakeup_int()
	reg_dfifo_mode = FLD_AUD_DFIFO0_IN | FLD_AUD_DFIFO0_L_INT;

	//ain0_sel sel i2s as input
	reg_dfifo_ain = 	  MASK_VAL( FLD_AUD_SAMPLE_TIME_CONFIG,      	0x01,\
			                        FLD_AUD_FIFO0_INPUT_SELECT, 		AUDIO_FIFO0_INPUT_SELECT_I2S, \
			                        FLD_AUD_FIFO1_INPUT_SELECT, 		AUDIO_FIFO1_INPUT_SELECT_USB, \
			                        FLD_AUD_MIC_LEFT_CHN_SELECT, 	1, \
			                        FLD_AUD_MIC_RIGHT_CHN_SELECT,	1 );

	/*******1.Dmic setting for audio input**************************/
	reg_audio_ctrl = AUDIO_OUTPUT_OFF;

//	audio_set_dmic_clk(0x81,0x18);     //16K*2*64=2.048M, since now DMIC clock is not get!

}

 /**
 *	@brief	   sdm setting function, enable or disable the sdm output, configure SDM output paramaters.
 *	@param[in]	InType -	  SDM input type, such as AMIC,DMIC,I2S_IN,USB_IN.
 *	@param[in]	Audio_Rate - audio sampling rate, such as 16K,32k etc.
 *	@param[in]	audio_out_en - audio output enable or disable set, '1' enable audio output; '0' disable output.
 *	@return	none
 */
void audio_set_sdm_output(AudioInput_Typedef InType,AudioRate_Typedef Audio_Rate,unsigned char audio_out_en)
{
	if(audio_out_en)
	{
		//SDM0  EVB(C1T139A30_V1.2) not used
//		gpio_set_func(GPIO_PB4, AS_SDM);
//		gpio_set_func(GPIO_PB5, AS_SDM);
		//SDM1
		gpio_set_func(GPIO_PB6, AS_SDM);
		gpio_set_func(GPIO_PB7, AS_SDM);

		reg_pwm_ctrl = MASK_VAL( 	FLD_PWM_MULTIPLY2,			0,\
									FLD_PWM_ENABLE,				0,\
									FLD_LINER_INTERPOLATE_EN,	1,\
									FLD_LEFT_SHAPING_EN,		0,\
									FLD_RIGTH_SHAPING_EN,		0);

		reg_pn2_right &= SDM_LEFT_CHN_CONST_EN;		//enable pn

		audio_set_i2s_clk(1,24);

		reg_ascl_tune = ASDM_Rate_Matching[Audio_Rate];


		reg_pn1_left =	MASK_VAL( 	PN1_LEFT_CHN_BITS,		6,\
									PN2_LEFT_CHN_EN, 		0,\
									PN1_LEFT_CHN_EN, 		0);
		reg_pn2_left =	MASK_VAL( 	PN2_LEFT_CHN_BITS,		6,\
									PN2_RIGHT_CHN_EN, 		0,\
									PN1_RIGHT_CHN_EN, 		0);
		reg_pn1_right =	MASK_VAL( 	PN1_RIGHT_CHN_BITS,		6,\
									CLK2A_AUDIO_CLK_EN, 	0,\
									EXCHANGE_SDM_DATA_EN,	0);
		reg_pn2_right = MASK_VAL( 	PN2_RIGHT_CHN_BITS,		6,\
									SDM_LEFT_CHN_CONST_EN, 	0,\
									SDM_RIGHT_CHN_CONST_EN, 0);

		reg_dfifo_mode |= FLD_AUD_DFIFO0_OUT;

		//config player mode
		if(InType == I2S_IN)
		{
			reg_audio_ctrl = (FLD_AUDIO_MONO_MODE|FLD_AUDIO_SDM_PLAYER_EN|FLD_AUDIO_I2S_RECORDER_EN|FLD_AUDIO_I2S_INTERFACE_EN);
		}
		else if(InType == USB_IN)
		{
			reg_audio_ctrl = (FLD_AUDIO_SDM_PLAYER_EN|FLD_AUDIO_HPF_EN);
		}
		else if(InType==BUF_IN)
		{
			reg_audio_ctrl = (FLD_AUDIO_MONO_MODE|FLD_AUDIO_SDM_PLAYER_EN);
		}
		else
		{
			reg_audio_ctrl = (FLD_AUDIO_SDM_PLAYER_EN);
		}
	}
	else
	{
		reg_audio_ctrl = AUDIO_OUTPUT_OFF;
	}
}

/*
*	The software control interface may be operated using either a 3-wire (SPI-compatible) or 2-wire MPU
*	interface. Selection of interface format is achieved by setting the state of the MODE pin.
*		MODE	0	2 wire		default(hardware)
*				1	3 wire
*	In 2-wire mode, the state of CSB pin allows the user to select one of two addresses.
*		CSB		0	0011010		default(hardware)
*				1   0011011
*
*	note:
*		The WM8731 is not a standard I2C interface, so the Kite standard I2C driver is not used.
*/

#define		WM8731_LEFT_HEADPHONE_OUT			0x04		//Left Headphone Out
#define		WM8731_RIGHT_HEADPHONE_OUT			0x06		//Right Headphone Out

#define		WM8731_ANA_AUDIO_PATH_CTRL			0x08		//Analogue Audio Path Control
#define		WM8731_DIG_AUDIO_PATH_CTRL			0x0a		//Digital Audio Path Control
#define		WM8731_POWER_DOWN_CTRL				0x0c		//Power Down Control
#define		WM8731_SEL_LINE_INPUT				0x08		//Set line input
#define	    WM8731_ST_LINE_VOL                  0x00        //Set linmute volume
#define	    WM8731_ST_RINE_VOL                  0x02        //Set rinmute volume
#define		WM8731_DIG_AUDIO_INTERFACE_FORMAT	0x0e		//Digital Audio Interface Format
#define		WM8731_SAMPLING_CTRL 				0x10		//Sampling Control
#define		WM8731_ACTIVE_CTRL 					0x12		//Active Control
#define		WM8731_RESET_CTRL 					0x1e		//Reset Register


unsigned char LineIn_To_I2S_CMD_TAB[9][2]={	{WM8731_RESET_CTRL, 				0x00},
		                                    {WM8731_POWER_DOWN_CTRL,			0x7a},
		                                    {WM8731_SEL_LINE_INPUT,             0x09},
											{WM8731_ST_LINE_VOL,         		0x1b},
											{WM8731_ST_RINE_VOL,	        	0x1b},
											{WM8731_DIG_AUDIO_INTERFACE_FORMAT,	0x02},
											{WM8731_SAMPLING_CTRL,				0x19},
											{WM8731_ACTIVE_CTRL,				0x01},
};

unsigned char I2S_To_HPout_CMD_TAB[9][2] ={	{WM8731_RESET_CTRL, 				0x00},
		                                    {WM8731_POWER_DOWN_CTRL,			0x27},
											{WM8731_ANA_AUDIO_PATH_CTRL,		0x10},
											{WM8731_LEFT_HEADPHONE_OUT,	        0x3f},
											{WM8731_RIGHT_HEADPHONE_OUT,	    0x3f},
											{WM8731_DIG_AUDIO_PATH_CTRL,		0x00},
											{WM8731_DIG_AUDIO_INTERFACE_FORMAT,	0x02},
											{WM8731_SAMPLING_CTRL,				0x19},
											{WM8731_ACTIVE_CTRL,				0x01},
};

/**
 * @brief     This function servers to config I2S input.
 * @param[in] i2c_pin_group - select the pin for I2S.
 * @param[in] CodecMode - select I2S mode.
 * @param[in] sysclk - system clock.
 * @return    none.
 */
void audio_set_codec(I2C_GPIO_SdaTypeDef sda_pin,I2C_GPIO_SclTypeDef scl_pin, CodecMode_Typedef CodecMode,unsigned sysclk)
{

	unsigned char i = 0;
	//I2C pin set
	i2c_gpio_set(sda_pin,scl_pin);//SDA/CK : A3/A4
	i2c_master_init(0x34, (unsigned char)(sysclk/(4*200000)) );		//i2c clock 200K, only master need set i2c clock

	if(CodecMode == CODEC_MODE_LINE_TO_HEADPHONE_LINEOUT_I2S)
	{
		for(i=0;i<9;i++)
		{
			do
			{
				write_reg8(0x04,LineIn_To_I2S_CMD_TAB[i][0]);
				write_reg8(0x05,LineIn_To_I2S_CMD_TAB[i][1]);
				write_reg8(0x07,0x37);
				while(read_reg8(0x02)&0x01);
			}
			while(read_reg8(0x02)&0x04);
		}
	}
	else if(CodecMode == CODEC_MODE_I2S_TO_HEADPHONE_LINEOUT)
	{
		for(i=0;i<9;i++)
		{
			do
			{
				write_reg8(0x848000,0x50);
				write_reg8(0x04,I2S_To_HPout_CMD_TAB[i][0]);
				write_reg8(0x05,I2S_To_HPout_CMD_TAB[i][1]);
				write_reg8(0x07,0x37);
				while(read_reg8(0x02)&0x01);
			}
			while(read_reg8(0x02)&0x04);
			write_reg8(0x848001,0x51);
		}
	}
	else
	{

	}
}


/**
 * @brief     audio I2S init in function, config the speed of i2s and MCLK to required speed.
 * @param[in] none.
 * @return    none.
 */
void audio_i2s_init(void)
{
	/*******1.I2S setting for audio input**************************/
	reg_audio_ctrl = AUDIO_OUTPUT_OFF;

	//if system clock=24M_Crystal. PWM0 2 frequency division output, for 12Mhz to offer the MCLK of CORDEC. select pd5 as PWM0 output.
	sub_wr(0x5af, 0x0, 3, 2); //PD5=0
	sub_wr(0x59e, 0x0, 5, 5); //PD5=0
	write_reg16(0x796,0x02);//TMAX0 continue mode
	write_reg16(0x794,0x01);//TCMP0
	write_reg8(0x781,0x01);//pwm0_en  781[bit0]

	//config i2s clock for 1Mhz to offer the clk of CORDEC
	gpio_set_func(GPIO_PD7, AS_I2S);		//I2S_BCK
	gpio_set_func(GPIO_PD2, AS_I2S);		//I2S_ADC_LRC  I2S_DAC_LRC
	gpio_set_func(GPIO_PD3, AS_I2S);		//I2S_ADC_DAT
	gpio_set_func(GPIO_PD4, AS_I2S);		//I2S_DAC_DAT
	gpio_set_input_en(GPIO_PD3, 1);

	//set I2S clock
	audio_set_i2s_clk(0x81,0x18);

	reg_clk_en2 |= FLD_CLK2_DFIFO_EN; //enable dfifo clock, this will be initialed in cpu_wakeup_int()

	//ain0_sel sel i2s as input
	reg_dfifo_ain = 	  MASK_VAL( FLD_AUD_SAMPLE_TIME_CONFIG,      	0x01,\
			                        FLD_AUD_FIFO0_INPUT_SELECT, 		AUDIO_FIFO0_INPUT_SELECT_I2S, \
			                        FLD_AUD_FIFO1_INPUT_SELECT, 		AUDIO_FIFO1_INPUT_SELECT_USB, \
			                        FLD_AUD_MIC_LEFT_CHN_SELECT, 	0, \
			                        FLD_AUD_MIC_RIGHT_CHN_SELECT,	0 );

	reg_audio_ctrl |= FLD_AUDIO_I2S_INTERFACE_EN|FLD_AUDIO_I2S_RECORDER_EN;

	reg_dfifo_mode = FLD_AUD_DFIFO0_IN;
}





/**
 *
 * @brief	   	i2s setting function, enable or disable the i2s output, configure i2s output paramaters
 * @param[in] 	InType		- select audio input type including amic ,dmic ,i2s and usb
 * @param[in] 	Audio_Rate 	- select audio rate, which will decide on which adc sampling rate and relative decimation configuration will be chosen.
 * @return	  	none
 */
void audio_set_i2s_output(AudioInput_Typedef InType,AudioRate_Typedef Audio_Rate)
{

//	volatile unsigned int i;
	//if system clock=24M_Crystal. PWM0 2 frequency division output, for 12Mhz to offer the MCLK of CORDEC. select pd5 as PWM0 output.
	sub_wr(0x5af, 0x0, 3, 2); //PD5=0
	sub_wr(0x59e, 0x0, 5, 5); //PD5=0
	write_reg16(0x796,0x02);//TMAX0 continue mode
	write_reg16(0x794,0x01);//TCMP0
	write_reg8(0x781,0x01);//pwm0_en  781[bit0]

	//config i2s clock for 1Mhz to offer the clk of CORDEC
	gpio_set_func(GPIO_PD7, AS_I2S);		//I2S_BCK
	gpio_set_func(GPIO_PD2, AS_I2S);		//I2S_ADC_LRC  I2S_DAC_LRC
	gpio_set_func(GPIO_PD3, AS_I2S);		//I2S_ADC_DAT
	gpio_set_func(GPIO_PD4, AS_I2S);		//I2S_DAC_DAT
	gpio_set_input_en(GPIO_PD3, 1);

	audio_set_i2s_clk(1,0x18);

	reg_pwm_ctrl = MASK_VAL( 	FLD_PWM_MULTIPLY2,			0,\
								FLD_PWM_ENABLE,				0,\
								FLD_LINER_INTERPOLATE_EN,	1,\
								FLD_LEFT_SHAPING_EN,		0,\
								FLD_RIGTH_SHAPING_EN,		0);


	if(InType==AMIC)
	{
		reg_ascl_tune = 0x80000000;
	}
	else
	{
		reg_ascl_tune = DSDM_Rate_Matching[Audio_Rate];
	}

	//config player mode
	if(InType == I2S_IN)
	{
		reg_audio_ctrl = (FLD_AUDIO_MONO_MODE|FLD_AUDIO_SDM_PLAYER_EN|FLD_AUDIO_I2S_RECORDER_EN|FLD_AUDIO_I2S_INTERFACE_EN);
	}
	else if(InType == USB_IN)
	{
		reg_audio_ctrl = (FLD_AUDIO_MONO_MODE|FLD_AUDIO_SDM_PLAYER_EN|FLD_AUDIO_ISO_PLAYER_EN);
	}
	else
	{
		reg_audio_ctrl = (FLD_AUDIO_I2S_PLAYER_EN|FLD_AUDIO_I2S_RECORDER_EN|FLD_AUDIO_I2S_INTERFACE_EN);
	}

	reg_dfifo_mode |= FLD_AUD_DFIFO0_OUT;
}

/**
 * @brief     This function servers to set USB input/output.
 * @param[in] none.
 * @return    none.
 */
void audio_set_usb_output(void)
{
	write_reg8(0xb10,0xf9);
}



