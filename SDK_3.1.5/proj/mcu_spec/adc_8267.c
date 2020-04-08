/********************************************************************************************************
 * @file     adc_8267.c 
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

#include "../tl_common.h"

#include "adc_8267.h"
#define batt_1per3_vol 1
#if(__TL_LIB_8267__ || (MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8261__ || (MCU_CORE_TYPE == MCU_CORE_8261) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE == MCU_CORE_8269)	)

/**************************************************************************************************
  Filename:       	adc.c
  Author:			junjun.xu@telink-semi.com
  Created Date:	2016/06/05

  Description:    This file contains the adc driver functions for the Telink 8267. It provided some sample applications like battery check and temperature check

**************************************************************************************************/
//#include "adc.h"


#define		EN_ADCCLK			(*(volatile unsigned char  *)0x80006b |= 0x80)

//Select ADC mannul mode
#define		EN_MANUALM			write_reg8(0x800033,0x00)


//Start sampling and conversion process for mannual mode
#define		STARTSAMPLING		write_reg8(0x800035,0x80)

//Read sampling data
#define		READOUTPUTDATA		read_reg16(0x800038)

/********************************************************
*
*	@brief		set ADC reference voltage for the Misc and L channel
*
*	@param		adcCha - enum variable adc channel.
*				adcRF - enum variable of adc reference voltage.
*
*	@return		None
*/
void adc_RefVoltageSet(enum ADCRFV adcRF){
	unsigned char st;

	st = (unsigned char)adcRF;
	*(volatile unsigned char  *)0x80002b &= 0xFC;

	*(volatile unsigned char  *)0x80002b |= st;
}


/********************************************************
*
*	@brief		set ADC resolution for channel Misc
*
*	@param		adcRes - enum variable adc resolution.
*
*	@return		None
*/
void adc_ResSet(enum ADCRESOLUTION adcRes){
	unsigned char resN;
	resN = (unsigned char )adcRes;
	*(volatile unsigned char  *)0x80003c &= 0xC7;
	*(volatile unsigned char  *)0x80003c |= (resN<<3);
}


/********************************************************
*
*	@brief		set ADC sample time(the number of adc clocks for each sample)
*
*	@param		adcCha - enum variable adc channel.
*				adcST - enum variable of adc sample time.
*
*	@return		None
*/

void adc_SampleTimeSet( enum ADCST adcST){

	unsigned char st;
	st = (unsigned char)adcST;

	*(volatile unsigned char  *)(0x80003c) &= 0xF8;

	*(volatile unsigned char  *)(0x80003c) |= st;
}


/********************************************************
*
*	@brief		set ADC analog input channel
*
*	@param		adcCha - enum variable adc channel.
*				adcInCha - enum variable of adc input channel.
*
*	@return		None
*/
void adc_AnaChSet(enum ADCINPUTCH adcInCha){
	unsigned char cnI;

	cnI = (unsigned char)adcInCha;

	*(volatile unsigned char  *)(0x80002c) &= 0xE0;
	*(volatile unsigned char  *)(0x80002c) |= cnI;
}

/***************************************************************************
*
*	@brief	set IO power supply for the 1/3 voltage division detection, there are two input sources of the
*			IO input battery voltage, one through the VDDH and the other through the  ANA_B<7> pin
*
*	@param	IOp - input power source '1' is the VDDH; '2' is the ANA_B<7>.
*
*	@return	'1' setting success; '0' set error
*/
#if batt_1per3_vol
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
#endif

/********************************************************
*
*	@brief		set ADC input channel mode - signle-end or differential mode
*
*	@param		adcCha - enum variable adc channel.
*				inM - enum variable of ADCINPUTMODE.
*
*	@return		None
*/
void adc_AnaModeSet( enum ADCINPUTMODE inM){
	unsigned char cnM;

	cnM = (unsigned char)inM;
	*(volatile unsigned char  *)(0x80002c) &= 0x1F;
	*(volatile unsigned char  *)(0x80002c) |= (cnM<<5);
}


/**********************************************************************
*	@brief	ADC initiate function, set the ADC clock details (4MHz) and start the ADC clock.
*			ADC clock relys on PLL, if the FHS isn't selected to 192M PLL (probably modified
*			by other parts codes), adc initiation function will returns error.
*
*	@param	None
*
*	@return	setResult - '1' set success; '0' set error
*/
unsigned char adc_Init(void ){

	unsigned char fhsBL,fhsBH;

	/******set adc clk as 4MHz******/
	write_reg8(0x800069,0x04); // adc clk step as 4
	write_reg8(0x80006a,0xc0); // adc clk mode as 192

	fhsBL = read_reg8(0x800070)&0x01;//0x70[0]
	fhsBH = read_reg8(0x800066)&0x80;//0x66[7]
	fhsBL = fhsBL|fhsBH;
	if(fhsBL){//FHS not default set to 192MHz
		return 0;
	}
	write_reg8(0x800070,0x00);// sel adc clk source as 192M pll
	write_reg8(0x80006b,0x80); // adc clk enable
	WriteAnalogReg(0x88,0x0f);// select 192M clk output
	WriteAnalogReg(0x05,0x60);// power on pll
	WriteAnalogReg(0x06,0xfe);// power on sar
	write_reg16(0x800030,(0xE2<<2));//set M channel period as (0xE2<<2), L channel's period is 0x06*16 defaultly,so sample frequency is sysclk/(0xE2<<2+0x06*16)
	EN_ADCCLK;//Enable adc CLK
	EN_MANUALM;
	return 1;
}
/********************************************************
*
*	@brief		Initiate function for the battery check function
*				AVSS and B7 can use interval 1/3 voltage
*
*	@param		checkM - Battery check mode, '0' for battery dircetly connected to chip,
*				'1' for battery connected to chip via boost DCDC
*
*	@return		None
*/
void adc_BatteryCheckInit(unsigned char checkM){
	/***1.set adc mode and input***/
	//a.use avss or B7 as input, can use interval 1/3 voltage division
#if (ADC_CHNM_ANA_INPUT == AVSS || ADC_CHNM_ANA_INPUT == B7)
#if batt_1per3_vol    ///if 1,internal 1/3 voltage division open.
	write_reg8(0x80002c,0x12);       //select "1/3 voltage division detection" as single-end input

	/***2.set battery check mode***/
	
	#if (ADC_CHNM_ANA_INPUT == AVSS)
		adc_IOPowerSupplySet(1);
	#elif (ADC_CHNM_ANA_INPUT == B7)
		adc_IOPowerSupplySet(2);
	#endif
#else
	write_reg8(0x80002c,0x0c);
#endif
#else
	//b.use other io as input
	adc_AnaChSet(ADC_CHNM_ANA_INPUT);
	adc_AnaModeSet(SINGLEEND);
#endif

	/***3.set adc reference voltage***/
	adc_RefVoltageSet(RV_1P428);     //Set reference voltage (V_REF)as  1.428V

	/***4.set adc resultion***/
	adc_ResSet(RES14);               //Set adc resolution to 14 bits, bit[14] to bit bit[1]

	/***5.set adc sample time***/
	adc_SampleTimeSet(S_3);          //set sample time

	/***6.enable manual mode***/
	EN_MANUALM;
}

/********************************************************

********************************************************/
void adc_drv_init(){
	adc_Init();
	adc_BatteryCheckInit(0);
}

/********************************************************
*
*	@brief		get the battery value
*
*	@param		None
*
*	@return		unsigned long - return the sampling value
*/
unsigned short adc_BatteryValueGet(void){

	unsigned short sampledValue;

	STARTSAMPLING;

	while(CHECKADCSTATUS);

	sampledValue = READOUTPUTDATA&0x3FFF;

	return sampledValue;
}
/********************************************************
*
*	@brief		Initiate function for the temparture sensor
*
*	@param		None
*
*	@return		None
*/

void adc_TemSensorInit(void){
	/***1.set adc mode and input***/
	write_reg8(0x80002c,0x0f);  //select TEMSENSORN as single-end input

	/***2. set adc reference voltage***/
	adc_RefVoltageSet(RV_AVDD);

	/***3.set adc resultion***/
	adc_ResSet(RES14);

	/***4.set adc sample time***/
	adc_SampleTimeSet(S_3);

	/***5.enable manual mode***/
	EN_MANUALM;
}

/********************************************************
*
*	@brief		get the temperature sensor sampled value
*
*	@param		None
*
*	@return		unsigned short - return the adc sampled value 14bits significants
*/

unsigned short adc_TemValueGet(void){
	unsigned short sampledValue;
	STARTSAMPLING;
	while(CHECKADCSTATUS);
	sampledValue = (unsigned short)(READOUTPUTDATA & 0x3FFF);
	STARTSAMPLING;
	while(CHECKADCSTATUS);
	sampledValue = sampledValue - (unsigned short)(READOUTPUTDATA & 0x3FFF);
	return sampledValue;
}

/*************************************************************************
*
*	@brief	get adc sampled value
*
*	@param	none
*
*	@return	sampled_value:	raw data
*/
unsigned short adc_SampleValueGet(void){
	//unsigned short sampledValue;
	STARTSAMPLING;
	while(CHECKADCSTATUS);
	return READOUTPUTDATA;
}

#endif
