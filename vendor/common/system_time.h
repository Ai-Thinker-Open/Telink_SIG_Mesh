/********************************************************************************************************
 * @file     system_time.h 
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


extern u32 system_time_ms;
extern u32 system_time_100ms;
extern u32 system_time_s;

static inline u32 clock_time_ms(){
    return system_time_ms;
}

static inline u32 clock_time_100ms(){
    return system_time_100ms;
}

static inline u32 clock_time_s(){
    return system_time_s;
}

void system_time_run();
u32 clock_time_exceed_ms(u32 ref, u32 span_ms);
u32 clock_time_exceed_100ms(u32 ref, u32 span_100ms);
u32 clock_time_exceed_s(u32 ref, u32 span_s);
