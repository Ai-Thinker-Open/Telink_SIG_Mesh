/********************************************************************************************************
 * @file     user_app_config.h 
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

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

/*
    Note: only can use "#define", "#if .. #elif..#endif", etc. here. Don't use "enum" "typedef".
*/



// ------- function Macro ---------
// #define CB_USER_FACTORY_RESET_ADDITIONAL()   cb_user_factory_reset_additional()
// #define CB_USER_PROC_LED_ONOFF_DRIVER(on)    cb_user_proc_led_onoff_driver(on)


#include "user_app_default.h"   // must at the end of this file

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
