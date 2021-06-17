/********************************************************************************************************
 * @file     config_model.h 
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
#include "proj_lib/sig_mesh/app_mesh.h"

// to increase efficiency
STATIC_ASSERT(sizeof(model_common_t) % 4 == 0);
STATIC_ASSERT(sizeof(model_g_light_s_t) % 4 == 0);

static inline void mesh_model_store_cfg_s()
{
	mesh_model_store(1, SIG_MD_CFG_SERVER);
}

u8 mesh_get_network_transmit();
u8 mesh_get_relay_retransmit();
u8 mesh_get_ttl();
u8 mesh_get_hb_pub_ttl();
u8 mesh_get_gatt_proxy();
u8 mesh_get_friend();
u8 mesh_get_relay();

u8 mesh_cmd_sig_cfg_model_sub_set2(u16 op, u16 sub_adr, u8 *uuid, model_common_t *p_model, u32 model_id, bool4 sig_model);

