/********************************************************************************************************
 * @file     usbkb.h 
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

#include "../common/types.h"
#include "../common/static_assert.h"
#include "../usbstd/HIDReportData.h"
#include "../usbstd/HIDClassCommon.h"


/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#define KEYBOARD_REPORT_KEY_MAX     6
typedef struct {
    u8 Modifier; /**< Keyboard modifier byte, indicating pressed modifier keys (a combination of
                       *   \c HID_KEYBOARD_MODIFER_* masks).
                       */
    u8 Reserved; /**< Reserved for OEM use, always set to 0. */
    u8 KeyCode[KEYBOARD_REPORT_KEY_MAX]; /**< Key codes of the currently pressed keys. */
} usbkb_hid_report_t;

int usbkb_hid_report_normal(u8 ctrl_key, u8 *keycode);


/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

