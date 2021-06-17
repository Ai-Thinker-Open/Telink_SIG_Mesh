/********************************************************************************************************
 * @file     nl_model_level.h 
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
/******************************
 ***** LEVEL SERVER MODEL *****
 ******************************/

/**
 * Callback function
 * @fn void level_state_changed(uint8_t element_index, int16_t old_level, int16_t new_level)
 * @param[in] element_idx is an index to the element in this device on which the level changed
 * @param[in] old_level is the previous value of the generic level state
 * @param[in] new_level is the new value of the generic level state
 *
 * Registering function
 * @brief     Registers callback that gets called whenever there is a level change in the model
 * @param[in] level_state_changed ptr to function callback
 *           
 */
//void nl_register_level_server_state_changed_callback(void (*level_state_changed)(uint8_t element_idx, int16_t present_level, int16_t target_level));
typedef void (*nl_level_state_changed_t)(uint8_t element_idx, int16_t present_level, int16_t target_level);
extern nl_level_state_changed_t	p_nl_level_state_changed;
void nl_register_level_server_state_changed_callback(void *p);

void nl_level_server_state_changed(uint8_t element_idx, int16_t pre_level, int16_t tar_level);


/**
 * gets the current state from the level server
 * @param[in] element_idx index to the element in this device on which the level changed. 0-based index
 * @return the current level state in the level server
 */
int16_t nl_get_current_level_in_level_server(uint16_t element_idx);

#define CB_NL_LEVEL_STATE_CHANGE(p, element_idx, present_level, target_level)	
#else
#define nl_level_server_state_changed(element_idx, present_level, target_level) 
#endif
