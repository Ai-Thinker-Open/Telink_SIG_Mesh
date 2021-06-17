/********************************************************************************************************
 * @file     nl_common.h 
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
#include "../user_config.h"
#include "nl_model_level.h"
#include "nl_model_schedules.h"

#if NL_API_ENABLE

#define CB_NL_PAR_NUM_1(cb, a)	do{if(cb){cb(a);}}while(0)
#define CB_NL_PAR_NUM_2(cb, a, b)	do{if(cb){cb(a, b);}}while(0)
#define CB_NL_PAR_NUM_3(cb, a, b, c)	do{if(cb){cb(a, b, c);}}while(0)

void register_nl_callback();

#else

#define CB_NL_PAR_NUM_1(cb, a)
#define CB_NL_PAR_NUM_2(cb, a, b)	
#define CB_NL_PAR_NUM_3(cb, a, b, c)	

#define register_nl_callback() 
#endif
