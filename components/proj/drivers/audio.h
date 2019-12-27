/********************************************************************************************************
 * @file     audio.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#ifndef audio_H
#define audio_H

/************************************************************
*	@param		m:		audio input mode, '1' diff; '0' single end
*				b:		battery check mode bit[7], battery input channel bit[5:0]
*
*/
#define AUDIOSE_SYSCLK48M_96KADC_16KSDM(m,b)			Audio_Init(m,b,AMIC,61,16,R6)

#define AUDIOSE_SYSCLK32M_96KADC_16KSDM(m,b)			Audio_Init(m,b,AMIC,50,8,R6)

#define AUDIOSE_SYSCLK24M_96KADC_16KSDM(m,b)			Audio_Init(m,b,AMIC,38,6,R6)

#define AUDIOSE_SYSCLK16M_48KADC_16KSDM(m,b)			Audio_Init(m,b,AMIC,35,12,R3)


#define DMIC_CFG_GPIO_PA0_PA1()    do{\
								       *(volatile unsigned char  *)0x800586 &= 0xfc;\
									   *(volatile unsigned char  *)0x8005b0 |= 0x01;\
								    }while(0)

//down sample rate
enum AUDIODSR{
	R1,R2,R3,R4,R5,R6,
	R7,R8,R16,R32,R64,R128,
};

//input channel select
enum AUDIOINPUTCH{
	AMIC,
	DMIC,
};


/***************************************************************
*	@brief	audio init function, call the adc init function, configure ADC, PGA and filter parameters used for audio sample
*			and process
*
*	@param	mFlag - audio input mode flag, '1' differ mode; '0' single end mode.
*			bcm_inputCh - battery check mode and input channel selection byte, the largest bit indicates mode, the lower
*						7 bits indicates input channel
*			audio_channel - enum variable, indicates the audio input source Analog MIC or Digital MIC
*			adc_max_m - Misc period set parameter, T_Misc = 2 * adc_max_m
*			adc_max_l - Left channel period set, T_Left = 16*adc_max_l
*			d_samp - decimation filter down sample rate
*
*	@return	None
*/
extern void Audio_Init(unsigned char mFlag,unsigned char checkM,enum AUDIOINPUTCH audio_channel,unsigned short adc_max_m, unsigned char adc_max_l,enum AUDIODSR d_samp);


/************************************************************************************
*
*	@brief	audio input set function, select analog audio input channel, start the filters
*
*	@param	adc_ch:	if audio input as signle end mode, should identify an analog audio signal input channel, enum variable of ADCINPUTCH
*
*	@return	none
*/
extern void Audio_InputSet(unsigned char adc_ch);


/************************************************************************************
*
*	@brief	sdm set function, enabl or disable the sdm output, configure SDM output paramaters
*
*	@param	audio_out_en:		audio output enable or disable set, '1' enable audio output; '0' disable output
*			sdm_setp:		SDM clk divider
*			sdm_clk:			SDM clk, default to be 8Mhz
*
*	@return	none
*/
extern void Audio_SDMOutputSet(unsigned char audio_out_en,unsigned short sdm_step,unsigned char sdm_clk);


/*******************************************************************************************
*	@brief	set audio volume level
*
*	@param	input_output_select:	select the tune channel, '1' tune ALC volume; '0' tune SDM output volume
*			volume_set_value:		volume level
*
*	@return	none
*/
extern void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value);


/********************************************************
*
*	@brief		get the battery value
*
*	@param		None
*
*	@return		unsigned long - return the sampled value, 7 bits resolution
*/
extern unsigned short Audio_BatteryValueGet(void);


/********************************************************
*
*	@brief		reg0x30[1:0] 2 bits for fine tuning, divider for slow down sample rate
*
*	@param		unsigned char fine_tune,
*
*	@return		void
*/
extern void Audio_FineTuneSampleRate(unsigned char fine_tune);


#endif
