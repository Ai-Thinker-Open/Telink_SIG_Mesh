/********************************************************************************************************
 * @file     battery.c 
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
#include "battery.h"
#include "adc.h"

#if(MODULE_BATT_ENABLE)

STATIC_ASSERT(ADC_CHN0_REF_SRC == ADC_REF_SRC_INTERNAL);	// if fail, pls change the core voltage 1300, //  we have  only  max 14 bits resolution
#define batt_conv_adc_mv(adc) ((adc - 188) >> 2)		//  通过回归分析得到
#define batt_conv_mv_adc(mv)  ((mv << 2)  + 188)		//  通过回归分析得到

static u8 avdd_val[]={0x81, 0x8f, 0x9c, 0xaa, 0xb7, 0xc4};
void batt_trim_rdiv_3v(u8 vol_value){		//  to adjust  core voltage
	int i;
	char ana_data;
	static char ana_data_last = 0;
	for(i = 0;(i < 6) && (vol_value > avdd_val[i]); ++i);
	
	ana_data = 6 - i;
	if(ana_data != ana_data_last){
		analog_write(0x08,(ana_data & 0x01)? (analog_read(0x08) | 0x80) : (analog_read(0x08) & (~0x80)));
		analog_write(0x09, analog_read(0x09) & (~0x03) | (ana_data>>1));
		ana_data_last = ana_data;
	}
}

int batt_get_value(void){
	static int batt_val = 0;
	int val = adc_get_value();
	batt_val = val + ((val - batt_val) >> 4);
	return batt_val;
}

batt_stat_t batt_stat;
// return  1 if status changes, return 0 if no changes
int batt_check(void){
	if(gpio_read(GPIO_USB_DET)){
		batt_stat.batt_volt_stat = BATT_STAT_CHARGING;
	}else{
        int batt_adc = batt_get_value();
		if(batt_adc > batt_conv_mv_adc(BATT_FULL_VOLT)){
		    batt_stat.batt_volt_stat = BATT_STAT_FULL;
		}
        else if(batt_adc > batt_conv_mv_adc(BATT_LOW_VOLT)){
		    batt_stat.batt_volt_stat = BATT_STAT_NORMAL;
        }else if(batt_adc > batt_conv_mv_adc(BATT_NO_PWR_VOLT)){
		    batt_stat.batt_volt_stat = BATT_STAT_LOW;
        }else{
		    batt_stat.batt_volt_stat = BATT_STAT_NO_PWR;
        }

		batt_trim_rdiv_3v((batt_adc >> 6));
    }
	
	if(batt_stat.batt_last_volt_stat != batt_stat.batt_volt_stat){
		batt_stat.batt_last_volt_stat = batt_stat.batt_volt_stat;
		return 1;
	}
	return 0;
}

void batt_init(void){}

#endif

