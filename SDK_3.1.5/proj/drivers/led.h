/********************************************************************************************************
 * @file     led.h 
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

#pragma once

#include "proj/tl_common.h"
#include "proj/os/ev.h"
#include "proj/os/et.h"

// if pattern changed,  the led will change accordingly within LED_MIN_RESP_TIME in ms
#define LED_MIN_RESP_TIME	50

typedef struct {
	u32 gpio;
	int *pattern_time;
	et_t et;
	int duration;
	u8  pattern;
}led_ctrl_t;

typedef void (*led_count_init_function)(void);
extern led_count_init_function led_count_init_cb;

int led_handler(int led);
void led_set_pattern(int led, int pat);
void led_set_next_pattern(int led);
void led_init(void);


