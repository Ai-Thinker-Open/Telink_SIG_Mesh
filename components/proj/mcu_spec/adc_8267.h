/********************************************************************************************************
 * @file     adc_8267.h 
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



#if(__TL_LIB_8267__ || (MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8261__ || (MCU_CORE_TYPE == MCU_CORE_8261) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE == MCU_CORE_8269)	)

#ifndef 	adc_new_H
#define 	adc_new_H

#include "../mcu/register.h"
#include "../common/compatibility.h"
#include "../common/tutility.h"


#define battery2audio() (*(volatile unsigned char*)(0x800033)=0x15)
#define audio2battery() (*(volatile unsigned char*)(0x800033)=0x00)

//ADC reference voltage
enum ADCRFV{
	RV_1P428,
	RV_AVDD,
	RV_1P224,
};
//ADC resolution
enum ADCRESOLUTION{
	RES7,
	RES9,
	RES10,
	RES11,
	RES12,
	RES13,
	RES14,
};

//ADC Sampling time
enum ADCST{
	S_3,
	S_6,
	S_9,
	S_12,
	S_18,
	S_24,
	S_48,
	S_144,
};

//ADC analog input channel selection enum
enum ADCINPUTCH{
	NOINPUT,
	C0,
	C1,
	C6,
	C7,
	B0,
	B1,
	B2,
	B3,
	B4,
	B5,
	B6,
	B7,
	PGAVOM,
	PGAVOP,
	TEMSENSORN,
	TEMSENSORP,
	AVSS,
	OTVDD,//1/3 voltage division detection
};
//ADC channel input mode
enum ADCINPUTMODE{
	SINGLEEND,
	INVERTB_1,
	INVERTB_3,
	PGAVOPM,
};


//set period for Misc
#define		SET_P(v)			write_reg16(0x800030,(v<<2)&0x0FFF)


//Check adc status, busy return 1
#define		CHECKADCSTATUS		(((*(volatile unsigned char  *)0x80003a) & 0x01) ? 1:0)


/********************************************************
*
*	@brief		set ADC resolution for channel Misc
*
*	@param		adcRes - enum variable adc resolution.
*
*	@return		None
*/
extern void adc_ResSet(enum ADCRESOLUTION adcRes);


/********************************************************
*
*	@brief		set ADC input channel
*
*	@param		adcCha - enum variable adc channel.
*				adcInCha - enum variable of adc input channel.
*
*	@return		None
*/
extern void adc_AnaChSet( enum ADCINPUTCH adcInCha);



/***************************************************************************
*
*	@brief	This function must be called when the input channel selected to 1/3 volatage division.
*			Set IO power supply for the 1/3 voltage division detection, there are two input sources of the
*			IO input battery voltage, one through the VDDH and the other through the  ANA_B<7> pin.
*
*	@param	IOp - input power source '1' is the VDDH; '2' is the ANA_B<7>.
*
*	@return	'1' set success; '0' set error
*/
// extern unsigned char adc_IOPowerSupplySet(unsigned char IOp);

/********************************************************
*
*	@brief		set ADC input channel mode - signle-end or differential mode
*
*	@param		adcCha - enum variable adc channel.
*				inM - enum variable of ADCINPUTMODE.
*
*	@return		None
*/
extern void adc_AnaModeSet( enum ADCINPUTMODE inM);

/********************************************************
*
*	@brief		set ADC sample time(the number of adc clocks for each sample)
*
*	@param		adcCha - enum variable adc channel.
*				adcST - enum variable of adc sample time.
*
*	@return		None
*/
extern void adc_SampleTimeSet(enum ADCST adcST);

/********************************************************
*
*	@brief		set ADC reference voltage for the Misc and L channel
*
*	@param		adcCha - enum variable adc channel.
*				adcRF - enum variable of adc reference voltage.
*
*	@return		None
*/
extern void adc_RefVoltageSet(enum ADCRFV adcRF);


/**********************************************************************
*	@brief	ADC initiate function, set the ADC clock details (3MHz) and start the ADC clock.
*			ADC clock relys on PLL, if the FHS isn't selected to 192M PLL (probably modified
*			by other parts codes), adc initiation function will returns error.
*
*	@param	None
*
*	@return	setResult - '1' set success; '0' set error
*/
extern unsigned char adc_Init(void );
/********************************************************
*
*	@brief		Initiate function for the battery check function
*
*	@param		checkM - Battery check mode, '0' for battery dircetly connected to chip,
*				'1' for battery connected to chip via boost DCDC
*
*	@return		None
*/
extern void adc_BatteryCheckInit(unsigned char checkM);
/********************************************************
*
*	@brief		get the battery value
*
*	@param		None
*
*	@return		unsigned long - return the sampling value multiplex 3
*/
extern unsigned short adc_BatteryValueGet(void);

/********************************************************
*
*	@brief		Initiate function for the temparture sensor
*
*	@param		None
*
*	@return		None
*/
extern void adc_TemSensorInit(void);
/********************************************************
*
*	@brief		get the temperature sensor sampled value
*
*	@param		None
*
*	@return		unsigned short - return the adc sampled value 14bits significants
*/
extern unsigned short adc_TemValueGet(void);

/*************************************************************************
*
*	@brief	get adc sampled value
*
*	@param	adc_ch:	adc channel select, MISC or the LCHANNEL, enum variable
*			sample_mode:	adc sample mode, '1' manual mode; '0' auto sample mode
*
*	@return	sampled_value:	raw data
*/
extern unsigned short adc_SampleValueGet(void);

#endif

#endif

