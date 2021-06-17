/********************************************************************************************************
 * @file     nl_model_schedules.h 
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
#ifndef WIN32
#include <stdbool.h>
#include <stdint.h>
#endif
#include "../user_config.h"

#if NL_API_ENABLE
#define NL_VENDOR_SCENE_DATA_LENGTH 64 //bytes

/******************************
 ***** SCENE SERVER MODEL *****
 ******************************/

/**
 * Callback function
 * @fn        void nl_scene_server_state_recalled(uint8_t scene_number, uint8_t* vendor_data)
 * @brief     Callback gets called whenever a scene is recalled
 * @param[in] scene_number is the index of scene, in the range 0-15
 * @param[in] vendor_data  ptr to vendor data binary data. Length of data is NL_VENDOR_SCENE_DATA_LENGTH bytes
 *
 * Registering function
 * @brief     Registers callback for the event when a state is recalled
 * @param[in] nl_scene_server_state_recalled callback to be called whenever any new device is discovered
 */
typedef void (*nl_scene_server_state_recalled_t)(uint8_t scene_number, uint8_t* vendor_data);
extern 	nl_scene_server_state_recalled_t 	p_nl_scene_server_state_recalled;
void nl_register_scene_server_state_recalled_callback(void *p);
//void nl_register_scene_server_state_recalled_callback(void (*nl_scene_server_state_recalled)(uint8_t scene_number, uint8_t* vendor_data));

/**
 * Callback function
 * @fn         void nl_get_vendor_scene_data(, uint8_t* vendor_data)
 * @brief      Callback gets called when Telink needs Nanoleaf vendor data. Length is NL_VENDOR_SCENE_DATA_LENGTH
 * @param[in]  scene_number is the index of scene, in the range 0-15
 * @param[out] vendor_data  ptr to vendor data binary data. Vendor fills this inside callback. Length of data is NL_VENDOR_SCENE_DATA_LENGTH bytes
 *
 * Registering function
 * @brief     Registers callback for the event when a state is recalled
 * @param[in] device_discovered_callback callback to be called whenever any new device is discovered
 */
//void nl_register_scene_server_get_vendor_scene_data_callback(void (*nl_get_vendor_scene_data)(uint8_t scene_number, uint8_t* vendor_data));
typedef void (*nl_get_vendor_scene_data_t)(uint8_t scene_number, uint8_t* vendor_data);
extern 	nl_get_vendor_scene_data_t 	p_nl_get_vendor_scene_data;
void nl_register_scene_server_get_vendor_scene_data_callback(void *p);

void nl_scene_server_get_vendor_scene_data(uint8_t scene_number, uint8_t* vendor_data);
void nl_scene_server_state_recalled(uint8_t scene_number, uint8_t* vendor_data);
#else
#define nl_scene_server_get_vendor_scene_data(scene_number, vendor_data) 
#define nl_scene_server_state_recalled(scene_number, vendor_data)
#endif
