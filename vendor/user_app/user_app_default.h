/********************************************************************************************************
 * @file     user_app_default.h 
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

// ----- user can define in user_app_config.h ------ //
#ifndef LIGHTNESS_DATA_REFORMAT_EN          
#define LIGHTNESS_DATA_REFORMAT_EN          0
#endif

#ifndef LIGHT_RES_HW_USER_EN          
#define LIGHT_RES_HW_USER_EN                0
#endif

#ifndef LIGHT_PAR_USER_EN          
#define LIGHT_PAR_USER_EN                   0
#endif

#ifndef USER_MESH_CMD_VD_ARRAY          
#define USER_MESH_CMD_VD_ARRAY              // NULL
#endif

#ifndef USER_REDEFINE_SCAN_RSP_EN          
#define USER_REDEFINE_SCAN_RSP_EN           0
#endif

// PWM_FREQ
// HCI_LOG_FW_EN
// TRANSITION_TIME_DEFAULT_VAL
// GPIO_LED

// ------- function Macro ---------
#ifndef CB_USER_FACTORY_RESET_ADDITIONAL
#define CB_USER_FACTORY_RESET_ADDITIONAL()          
#endif

#ifndef CB_USER_LIGHT_INIT_ON_CONDITION
#define CB_USER_LIGHT_INIT_ON_CONDITION()   (1)
#endif

#ifndef CB_USER_BLE_CONNECT
#define CB_USER_BLE_CONNECT(e, p, n)                      
#endif

#ifndef CB_USER_INIT
#define CB_USER_INIT()                      
#endif

#ifndef CB_USER_MAIN_LOOP
#define CB_USER_MAIN_LOOP()                      
#endif

// CB_USER_PROC_LED_ONOFF_DRIVER
// CFG_LED_EVENT_SET_MESH_INFO
// CFG_LED_EVENT_SET_SUBSCRIPTION
// CB_USER_IS_CMD_WITH_TID_VENDOR

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
