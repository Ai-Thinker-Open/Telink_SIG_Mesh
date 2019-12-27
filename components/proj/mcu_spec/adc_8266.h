/********************************************************************************************************
 * @file     adc_8266.h 
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

#ifndef ADC_8266_H_
#define ADC_8266_H_


#if(__TL_LIB_8266__ || (MCU_CORE_TYPE == MCU_CORE_8266))


#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

/** @addtogroup  HAL_ADC_Module ADC Module
 *  @{
 *  @details	ADC Module can used to sample battery voltage,temperature sensor,mono audio signals.
 */

#include "../common/types.h"
//#include "../mcu_spec/anareg_8886.h"

/** @addtogroup  HAL_ADC_REF ADC Reference
 *  @{
 */
#define ADC_REF_1_3V             0x00		//!< ADC Reference:1.3v
#define ADC_REF_AVDD             0x01		//!< ADC Reference:AVDD
/** @} end of group HAL_ADC_REF */


/** @addtogroup  HAL_ADC_DONE_SIGNAL ADC Done Signal
 *  @{
 */
enum{
	ADC_DONE_SIGNAL_COUNTER,
	ADC_DONE_SIGNAL_RISING,
	ADC_DONE_SIGNAL_FALLING,
};
/** @} end of group HAL_ADC_DONE_SIGNAL */


/** @addtogroup  HAL_ADC_AUDIO_MODE ADC Audio Mode
 *  @{
 */
enum{
	ADC_AUDIO_MODE_NONE,		//!< ADC Audio Mode Uselessly
	ADC_AUDIO_MODE_MONO,		//!< ADC Audio Mono Mode
	ADC_AUDIO_MODE_STEREO,		//!< ADC Audio Stereo Mode,8266 not supported this mode.
};
/** @} end of group HAL_ADC_AUDIO_MODE */


//enum{
//	FLD_ADC_CHN_D0=1,
//	FLD_ADC_CHN_D1
//	FLD_ADC_CHN_D2,
//	FLD_ADC_CHN_D3,
//	FLD_ADC_CHN_D4,
//	FLD_ADC_CHN_D5,
//	FLD_ADC_CHN_C2,
//	FLD_ADC_CHN_C3,
//	FLD_ADC_CHN_C4,
//	FLD_ADC_CHN_C5,
//	FLD_ADC_CHN_C6,
//	FLD_ADC_CHN_C7,
//
//	FLD_ADC_CHN_GND = 0x12,
//};


/** @addtogroup  HAL_ADC_SAMP_CYC ADC Sample Cycle
 *  @{
 *  The cycle on adc clock for complete a Sampling
 */
enum{
	ADC_SAMPLING_CYCLE_3 = 0,		//!<Adc Sampling Cycle:3
	ADC_SAMPLING_CYCLE_6 = 1,		//!<Adc Sampling Cycle:6
	ADC_SAMPLING_CYCLE_9 = 2,		//!<Adc Sampling Cycle:9
	ADC_SAMPLING_CYCLE_12 = 3,		//!<Adc Sampling Cycle:12
	ADC_SAMPLING_CYCLE_18 = 4,		//!<Adc Sampling Cycle:18
	ADC_SAMPLING_CYCLE_24 = 5,		//!<Adc Sampling Cycle:24
	ADC_SAMPLING_CYCLE_48 = 6,		//!<Adc Sampling Cycle:48
	ADC_SAMPLING_CYCLE_144 = 7,		//!<Adc Sampling Cycle:144
};
/** @} end of group HAL_ADC_SAMP_CYC */

/** @addtogroup  HAL_ADC_RES ADC Resolution
 *  @{
 *  The ADC Resolution is 1/(2^adc_res_bits - 1)
 */
enum{
	ADC_SAMPLING_RES_7BIT = 0,		//!<ADC Sample Resolution Bits:adc_res_bits = 7
	ADC_SAMPLING_RES_9BIT = 1,		//!<ADC Sample Resolution Bits:adc_res_bits = 9
	ADC_SAMPLING_RES_10BIT = 2,		//!<ADC Sample Resolution Bits:adc_res_bits = 10
	ADC_SAMPLING_RES_11BIT = 3,		//!<ADC Sample Resolution Bits:adc_res_bits = 11
	ADC_SAMPLING_RES_12BIT = 4,		//!<ADC Sample Resolution Bits:adc_res_bits = 12
	ADC_SAMPLING_RES_13BIT = 5,		//!<ADC Sample Resolution Bits:adc_res_bits = 13
	ADC_SAMPLING_RES_14BIT = 7,		//!<ADC Sample Resolution Bits:adc_res_bits = 14
};
/** @} end of group HAL_ADC_RES */

/**
 * @brief  		ADC Initialization
 *
 * @return  None
 * @details	  - adc_init has already set some parameter:
 *				  -# channel misc sampling cycle:6
 *				  -# channel misc resolution: 14 bits
 *				  -# channel left and right sample cycle:9
 *				  -# adc clk:4Mhz
 *				  -# adc module state:power on
 *
 */
void adc_init();

/**
 * @brief  		Start ADC and get the channel misc converter result
 *
 * @param		None
 *
 * @return  	ADC Result
 */
u16 adc_get(void);

void adc_power_down(void);

void adc_power_down_start(void);
void adc_power_down_end(void);
void adc_setting_recover(void);

/** @} end of group HAL_ADC_Module */

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif



#endif

#endif /* ADC_8266_H_ */
