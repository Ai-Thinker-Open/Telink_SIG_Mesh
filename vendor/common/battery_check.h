/********************************************************************************************************
 * @file     battery_check.h
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
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

#ifndef BATTERY_CHECK_H_
#define BATTERY_CHECK_H_

#define MANNUAL_MODE_GET_ADC_SAMPLE_RESULT  0
#define VBAT_LEAKAGE_PROTECT_EN				0
#define VBAT_ALRAM_THRES_MV				(2000)   // 2000 mV low battery alarm

#define LOG_BATTERY_CHECK_DEBUG(pbuf,len,format,...)    //LOG_MSG_LIB(LOG_GET_LEVEL_MODULE(TL_LOG_LEVEL_LIB,TL_LOG_NODE_SDK),pbuf,len,format,__VA_ARGS__)


void battery_set_detect_enable (int en);
int  battery_get_detect_enable (void);

int app_battery_power_check(u16 alram_vol_mv, int loop_flag);
void app_battery_power_check_and_sleep_handle(int loop_flag);
void battery_power_low_handle(int loop_flag);


#endif /* APP_BATTDET_H_ */
