/********************************************************************************************************
 * @file     nl_common.c 
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

#include "nl_common.h"
#include "nl_model_level.h"
#include "nl_model_schedules.h"
#include "../user_config.h"

#if NL_API_ENABLE
void register_nl_callback(){
	//nl_model_level
	nl_register_level_server_state_changed_callback(nl_level_server_state_changed);
	
	//nl_model_schedule
	nl_register_scene_server_state_recalled_callback(nl_scene_server_state_recalled);
	nl_register_scene_server_get_vendor_scene_data_callback(nl_scene_server_get_vendor_scene_data);
}
#else
#define register_nl_callback() 
#endif