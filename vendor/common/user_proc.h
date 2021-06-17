/********************************************************************************************************
 * @file     user_proc.h 
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

#ifndef __USER_PROC_H
#define __USER_PROC_H
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/user_config.h"
#include "user_ali.h"
#include "nl_api/nl_common.h"

u8 ais_pri_data_set(u8 *p);
void user_sha256_data_proc();
void user_node_oob_set();
u8   user_mac_proc();
void user_power_on_proc();
void user_mesh_cps_init();
void user_set_def_sub_adr();
void user_system_time_proc();
void mesh_provision_para_init(u8 *p_random);
void user_prov_multi_oob();
void user_prov_multi_device_uuid();
int user_node_rc_link_open_callback();

#endif
