/********************************************************************************************************
 * @file     compiler.h 
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

#ifdef __GNUC__
#if IOS_APP_ENABLE || ANDROID_APP_ENABLE 
#define inline 
#define _attribute_packed_
#define _attribute_aligned_(s)	__declspec(align(s))
#define _attribute_session_(s)
#define _attribute_ram_code_
#define _attribute_custom_code_
#define _attribute_no_inline_   __declspec(noinline)
#else
#define _inline_ 				inline				//   C99 meaning
#define _attribute_packed_		__attribute__((packed))
#define _attribute_aligned_(s)	__attribute__((aligned(s)))
#define _attribute_session_(s)	__attribute__((section(s)))
#define _attribute_ram_code_  	_attribute_session_(".ram_code") __attribute__((noinline))
#define _attribute_custom_code_  	_attribute_session_(".custom") volatile
#define _attribute_no_inline_   __attribute__((noinline)) 
// #define _inline_ 				extern __attribute__ ((gnu_inline)) inline

#include "config.h"

#if ((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
#define	BLC_PM_DEEP_RETENTION_MODE_EN       1
#else
#define	BLC_PM_DEEP_RETENTION_MODE_EN       0   // must 0
#endif

#if (BLC_PM_DEEP_RETENTION_MODE_EN)
//#define _attribute_data_retention_   __attribute__((section(".retention_data")))
// #define _attribute_bss_retention_    __attribute__((section(".retention_bss")))          // not use now, always take a space in firmware for private section.
#if (!__PROJECT_BOOTLOADER__) // if bootloader use this section, it need to fix AT(no_ret_data) and copy position from flash in cstartup.
#define _attribute_no_retention_data_   __attribute__((section(".no_ret_data")))
#endif
#define _attribute_no_retention_bss_   __attribute__((section(".no_ret_bss")))
#endif
#endif
#else
#define _inline_ 				inline				//   C99 meaning
#define _attribute_packed_
#define _attribute_aligned_(s)	__declspec(align(s))
#define _attribute_session_(s)
#define _attribute_ram_code_
#define _attribute_custom_code_
#define _attribute_no_inline_   __declspec(noinline) 

#endif

#ifndef _attribute_data_retention_
#define _attribute_data_retention_
#endif
#ifndef _attribute_bss_retention_
#define _attribute_bss_retention_       //
#endif
#ifndef _attribute_no_retention_data_
#define _attribute_no_retention_data_
#endif
#ifndef _attribute_no_retention_bss_
#define _attribute_no_retention_bss_    //
#endif

