/********************************************************************************************************
 * @file     dual_mode_adapt.h 
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

#define FW_RAMCODE_SIZE_MAX         (0x4000)    // no limit for bootloader.

#if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define DUAL_MODE_ZB_FW_SIZE_MAX_K  (256)
#define DUAL_MODE_FW_ADDR_SIGMESH   (0x80000)   // if modify, must modify __FW_OFFSET to the same,
#define DUAL_MODE_FW_ADDR_ZIGBEE    (0xC0000)
#define FLASH_ADR_UPDATE_NEW_FW     (0x40000)
#else
#define DUAL_MODE_FW_ADDR_SIGMESH   (0x00000)
#define DUAL_MODE_FW_ADDR_ZIGBEE    (0x40000)
#endif

enum{
	RF_MODE_BLE			= 0,
	RF_MODE_ZIGBEE,
};

#if (0 == FW_START_BY_BOOTLOADER_EN)
enum{
    DUAL_MODE_SAVE_ENABLE       = 0x5A,    // dual mode state should be define both 73000 and 76080
    DUAL_MODE_SAVE_DISABLE      = 0x00,
    // all other is disable exclude 0xff
};
#endif

enum{
    DUAL_MODE_NOT_SUPPORT       = 0x00,
    DUAL_MODE_SUPPORT_ENABLE    = 0x01, // must 0xff
    DUAL_MODE_SUPPORT_DISABLE   = 0x02,
};

enum{
    DUAL_MODE_CALI_VAL_FLAG         = 0x5A,
};

extern u8 rf_mode;
extern u8 dual_mode_state;
int is_ble_found();
int is_zigbee_found();
u8 dual_mode_proc();
void dual_mode_en_init();
void dual_mode_select();
void dual_mode_disable();
void irq_zigbee_sdk_handler(void);

// telink mesh
void dual_mode_restore_TLK_4K();
int UI_resotre_TLK_4K_with_check();


