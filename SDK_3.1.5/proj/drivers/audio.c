/********************************************************************************************************
 * @file     audio.c 
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
#include "proj/tl_common.h"
#include "audio.h"
#include "pga.h"
//set period for Misc
#define		SET_PFM(v)			write_reg16(0x800030,(v<<2)&0x0FFF)
//set period for L
#define		SET_PFL(v)			write_reg8(0x800032,v)


unsigned char audio_mode;
static unsigned char adc_IOPowerSupplySet(unsigned char IOp){
	unsigned char vv1;
	if(IOp>2||IOp<1){

		return 0;
	}
	else{
		vv1 = ReadAnalogReg(0x02);
		vv1 = vv1 & 0xcf;
		vv1 = vv1 | (IOp<<4);
		WriteAnalogReg(0x02,vv1);
		return 1;
	}
}

/**************************************************
*
*	@brief	battery check init function used when enabling audio, use 7 bits resolution, the adc refernce voltage
*			set to 1.428V.
*
*	@param	bcm_inputCh:		battery check mode and input channel selection, bit[7] indicates the battery check mode, if bit[7]
*							equals to 1, the lower 5 bits assign an adc sample input channel (enum ADCINPUTCH).
*	@return	none
*/
void Audio_BatteryCheckInit(unsigned char checkM){//add to audio_Init
	/***1.set adc mode and input***/
	write_reg8(0x80002e,0x12);       //select "1/3 voltage division detection" as single-end input
    write_reg8(0x80002c,0x12);
	/***2.set battery check mode***/
	if(!checkM)
		adc_IOPowerSupplySet(1);
	else
		adc_IOPowerSupplySet(2);

	//SETB(0x800033, 0x02);// Enable Misc channel to sample battery's voltage
	SETB(0x800033, 0x08);// Enable Misc channel to sample battery's voltage
	//adc_RefVoltageSet(RV_1P428);//Set reference voltage (V_REF)as  RV_AVDD
	//adc_RefVoltageSet(RV_AVDD);//Set reference voltage (V_REF)as  RV_AVDD
	*(volatile unsigned char  *)0x80002b &= 0xCF;
    *(volatile unsigned char  *)0x80002b &= 0xFC;

	//adc_ResSet(RES7);//Set adc resolution to 7 bits, bit[14] to bit bit[8]
	*(volatile unsigned char  *)0x80003C &= 0xC7;
    *(volatile unsigned char  *)0x80002f &= 0xC7;

	//adc_SampleTimeSet(MISC,S_3);//set sample time
	//*(volatile unsigned char  *)(0x80003c) &= 0xF8;

}


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
#if 0
void Audio_Init(unsigned char mFlag,unsigned char checkM,enum AUDIOINPUTCH audio_channel,unsigned short adc_max_m, unsigned char adc_max_l,enum AUDIODSR d_samp){
	unsigned char tem;
	/******config FHS as 192M PLL******/
	write_reg8(0x800070,0x00);// sel fhs clk source as 192M pll
	WriteAnalogReg(0x88,0x0f);// select 192M clk output
	WriteAnalogReg(0x05,0x60);// power on pll
	WriteAnalogReg(0x06,0xfe);// power on sar
	write_reg8(0x800065,read_reg8(0x800065)|0x04);//Enable dfifo CLK

	if(audio_channel == AMIC){
		/*******config adc clk as 4MHz*****/
		write_reg8(0x800069,0x04); // adc clk step as 4
		write_reg8(0x80006a,0xc0); // adc clk mode as 192
		*(volatile unsigned char  *)0x80006b |= 0x80;//Enable adc CLK

		/****************************ADC setting for analog audio sample*************************************/
		/****Set reference voltage for sampling AVDD****/
		*(volatile unsigned char  *)0x80002b &= 0xF3;//Set reference voltage (V_REF)as  RV_AVDD
		// *(volatile unsigned char  *)0x80002b |= 0x04;
		/****signed adc data****/
		*(volatile unsigned char  *)0x80002d |= 0x80;
		/****set adc resolution to 14 bits****/
		*(volatile unsigned char  *)0x80002f &= 0xF8;
		*(volatile unsigned char  *)0x80002f |= 0x07;
		/****set adc sample time to 3 cycles****/
		*(volatile unsigned char  *)0x80003d &= 0xF8;

		if(mFlag){//diff mode
			//L channel pga-vom input
			*(volatile unsigned char  *)(0x80002d) &= 0xE0;
			*(volatile unsigned char  *)(0x80002d) |= 0x0D;
			//adc_AnaChSet(LCHANNEL,PGAVOM);
			//adc_AnaModeSet(LCHANNEL,PGAVOPM);
			*(volatile unsigned char  *)(0x80002d) &= 0x9F;
			*(volatile unsigned char  *)(0x80002d) |= 0x60;
			audio_mode = 1;//start PGA
		}
		else{//single end mode
			audio_mode = 0;
			//set L channel single end mode
			*(volatile unsigned char  *)(0x80002d) &= 0x1F;
		}

		write_reg8(0x800b03,0x32);//audio input select AMIC, enable dfifo, enable wptr
		SET_PFM(adc_max_m);//set Misc channel period
		SET_PFL(adc_max_l);//set L channel period
		//write_reg8(0x800033,0x15);//mono mode; L channel enable; audio adc output enable
	    write_reg8(0x800033,0x2f);//mono mode; L channel enable; audio adc output enable
	}
	else if(audio_channel == DMIC){
		/*******config DMIC clk as 1MHz*****/
		write_reg8(0x80006c,0x01); // dmic clk step as 1
		write_reg8(0x80006d,0xc0); // dmic clk mode as 192
		*(volatile unsigned char  *)0x80006c |= 0x80;//Enable dmic CLK
		write_reg8(0x800b03,0x30);//audio input select DMIC, enable dfifo, enable wptr
		write_reg8(0x800b06,0x40);//enable DMIC input volume auto control
	}

	write_reg8(0x800b04,d_samp);//setting down sample rate

	/********************************************Filter setting*****************************/
#if (BLE_DMIC_ENABLE)
	tem = read_reg8(0x800b04);//low pass filter setting
	tem = tem & 0x8F;
	tem = tem | 0x10;
	write_reg8(0x800b04,tem);
#else
    write_reg8(0x800b04,0x21);// reg0xb04[6:4] cic filter output select 1 , cic[22:5] , reg0xb04[3:0] down scale by 2,
#endif

	tem = read_reg8(0x800b05);
	tem = tem & 0xF0;
	tem = tem | 0x0B;
	write_reg8(0x800b05,tem);//hpf 11
	//write_reg8(0x800b06,0x1c);//ALC Volume setting
	write_reg8(0x800b06,0x24);//ALC Volume setting

	Audio_BatteryCheckInit(checkM);
}
#endif

void Audio_Init(unsigned char mFlag,unsigned char checkM,enum AUDIOINPUTCH audio_channel,unsigned short adc_max_m, unsigned char adc_max_l,enum AUDIODSR d_samp){
	unsigned char tem;
	/******config FHS as 192M PLL******/
	write_reg8(0x800070,0x00);// sel fhs clk source as 192M pll
	WriteAnalogReg(0x88,0x0f);// select 192M clk output
	WriteAnalogReg(0x05,0x62);// power on pll
	WriteAnalogReg(0x06,0xfe);// power on sar
	write_reg8(0x800065,read_reg8(0x800065)|0x04);//Enable dfifo CLK

	if(audio_channel == AMIC){
		/*******config adc clk as 4MHz*****/
		write_reg8(0x800069,0x04); // adc clk step as 4
		write_reg8(0x80006a,0xc0); // adc clk mode as 192
		*(volatile unsigned char  *)0x80006b |= 0x80;//Enable adc CLK

		/****************************ADC setting for analog audio sample*************************************/
		/****Set reference voltage for sampling AVDD****/
		*(volatile unsigned char  *)0x80002b &= 0xF3;//Set reference voltage (V_REF)as  RV_AVDD 1.428v
		*(volatile unsigned char  *)0x80002b |= 0x04;///reference voltage:AVDD  L_channel
		/****signed adc data****/
		*(volatile unsigned char  *)0x80002d |= 0x80;
		/****set adc resolution to 14 bits****/
		*(volatile unsigned char  *)0x80002f &= 0xF8;
		*(volatile unsigned char  *)0x80002f |= 0x07;
		/****set adc sample time to 3 cycles****/
		*(volatile unsigned char  *)0x80003d &= 0xF8;

		if(mFlag){//diff mode
			//L channel pga-vom input
			*(volatile unsigned char  *)(0x80002d) &= 0xE0;
			*(volatile unsigned char  *)(0x80002d) |= 0x0D;
			//adc_AnaChSet(LCHANNEL,PGAVOM);
			//adc_AnaModeSet(LCHANNEL,PGAVOPM);
			*(volatile unsigned char  *)(0x80002d) &= 0x9F;
			*(volatile unsigned char  *)(0x80002d) |= 0x60;
			audio_mode = 1;//start PGA
		}
		else{//single end mode
			audio_mode = 0;
			//set L channel single end mode
			*(volatile unsigned char  *)(0x80002d) &= 0x1F;
		}

		write_reg8(0x800b03,0x32);//audio input select AMIC, enable dfifo, enable wptr
		SET_PFM(adc_max_m);//set Misc channel period
		SET_PFL(adc_max_l);//set L channel period
		write_reg8(0x800033,0x15);//mono mode; L channel enable; audio adc output enable
	}
	else if(audio_channel == DMIC){
		/*******config DMIC clk as 1MHz*****/
		write_reg8(0x80006c,0x01); // dmic clk step as 1
		write_reg8(0x80006d,0xc0); // dmic clk mode as 192
		*(volatile unsigned char  *)0x80006c |= 0x80;//Enable dmic CLK
		write_reg8(0x800b03,0x30);//audio input select DMIC, enable dfifo, enable wptr
		write_reg8(0x800b06,0x40);//enable DMIC input volume auto control
	}

	write_reg8(0x800b04,d_samp);//setting down sample rate

	/********************************************Filter setting*****************************/
#if TL_MIC_32K_FIR_16K
	tem = read_reg8(0x800b04);//low pass filter setting
	tem = tem & 0x8F;
	tem = tem | 0x20;////0x10
	write_reg8(0x800b04,tem);
#else
	tem = read_reg8(0x800b04);//low pass filter setting
	tem = tem & 0x8F;
	tem = tem | 0x40;
	write_reg8(0x800b04,tem);
#endif


	tem = read_reg8(0x800b05);
	tem = tem & 0xF0;/////0xF0
#if TL_MIC_32K_FIR_16K
	tem = tem | 0x09;
#else
	tem = tem | 0x0B;/////0x0B
#endif

	write_reg8(0x800b05,tem);//hpf 11


#if TL_MIC_32K_FIR_16K
	write_reg8(0x800b06,0x24);//ALC Volume setting  0x24 by Q.W
#else
	write_reg8(0x800b06,0x1c);//ALC Volume setting  0x1c by Q.W
#endif
}
void Audio_FineTuneSampleRate(unsigned char fine_tune){
    //if(fine_tune>3) return;
    unsigned char tmp = read_reg8(0x800030);
    tmp |= (fine_tune&0x03); ////reg0x30[1:0] 2 bits for fine tuning

    write_reg8(0x800030,tmp);
}



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

void Audio_SDMOutputSet(unsigned char audio_out_en,unsigned short sdm_step,unsigned char sdm_clk){
	unsigned char tem;
	if(audio_out_en){
		/***1.Set SDM output clock to 8 Mhz***/
		write_reg8(0x800067,(0x80|sdm_clk));//i2s clock, 8M Hz
		write_reg8(0x800068,0xc0);//mod_i = 192MHz
		write_reg16(0x800564,sdm_step);

		/***2.Enable PN generator as dither control, clear bit 2, 3, 6***/
		CLRB(0x800560,0x4C);
		SETB(0x800560,0x30);//set bit 4 bit 5
		CLRB(0x800560,0x03);//Close audio output
		write_reg8(0x800562, 0x08);//PN generator 1 bits ussed
		write_reg8(0x800563, 0x08);//PN generator 2 bits ussed

        /***3.Enable SDM pins(sdm_n, sdm_p)***/
		tem = read_reg8(0x8005a6);
		tem = tem &0xFC;
		write_reg8(0x8005a6,tem);//pe[1:0] gpio off
		tem = read_reg8(0x8005b4);
		tem = tem &0xFC;
		write_reg8(0x8005b4,tem);//pe[1:0] as sdm_n, sdm_p

		/***4.enable audio, enable sdm player***/
		SETB(0x800560,0x03);
	}
	else
		CLRB(0x800560,0x02);

}



/************************************************************************************
*
*	@brief	audio input set function, select analog audio input channel, start the filters
*
*	@param	adc_ch:	if audio input as signle end mode, should identify an analog audio signal input channel, enum variable of ADCINPUTCH
*
*	@return	none
*/
void Audio_InputSet(unsigned char adc_ch){
	unsigned char tem;
	if(audio_mode){
		//PGA Setting
		pgaInit();
		preGainAdjust(DB20);//set pre pga gain to 0  ////DB20 by sihui
		postGainAdjust(DB9);//set post pga gain to 0 ////DB9 by sihui
	}
	else{
		//L channel's input as C[0]
		//adc_AnaChSet(LCHANNEL,adc_ch);
		*(volatile unsigned char  *)(0x80002d) &= 0xE0;
		*(volatile unsigned char  *)(0x80002d) |= adc_ch;
		}
	//open lpf, hpf, alc
	tem = read_reg8(0x800b05);
	tem = tem & 0x8F;
	tem = tem | 0x40;////open hpf,alc;close lpf
	write_reg8(0x800b05,tem);
}


/*******************************************************************************************
*	@brief	set audio volume level
*
*	@param	input_output_select:	select the tune channel, '1' tune ALC volume; '0' tune SDM output volume
*			volume_set_value:		volume level
*
*	@return	none
*/
void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value){

	if(input_output_select)
		write_reg8(0x800b06,volume_set_value);
	else
		write_reg8(0x800561,volume_set_value);
}


/*************************************************************
*
*	@brief	automatically gradual change volume
*
*	@param	vol_step - volume change step, the high part is decrease step while the low part is increase step
*			gradual_interval - volume increase interval
*
*	@return	none
*/
void Audio_VolumeStepChange(unsigned char vol_step,unsigned short gradual_interval){
	// unsigned char low_part,high_part;
	// low_part = (unsigned char)gradual_interval;
	// high_part = (unsigned char)(gradual_interval>>8);
	write_reg8(0x800b0b,vol_step);
	write_reg16(0x800b0c,gradual_interval);
	// write_reg8(0x800b0d,high_part);
}


/********************************************************
*
*	@brief		get the battery value
*
*	@param		None
*
*	@return		unsigned long - return the sampled value, 7 bits resolution
*/
	//Check adc status, busy return 1
#define		CHECKADCSTATUS		(((*(volatile unsigned char  *)0x80003a) & 0x01) ? 1:0)
#if 1 // diff mode on off, audio on
unsigned short Audio_BatteryValueGet(void){
	unsigned short sampledValue;
	write_reg8(0x800035,0x80);
	while(CHECKADCSTATUS);
	sampledValue = read_reg16(0x800038);
	sampledValue = (sampledValue&0x3F80);
    sampledValue = (sampledValue -128)*64*1428/63/16384;
	return sampledValue;
}


#else //for diff mode on
unsigned short Audio_BatteryValueGet(void){
	unsigned short sampledValue;
	while(CHECKADCSTATUS);
	sampledValue = read_reg16(0x800038);
	sampledValue = sampledValue&0x3F80;
	return (sampledValue>>7);
}
#endif

