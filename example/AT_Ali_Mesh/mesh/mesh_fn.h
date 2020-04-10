/********************************************************************************************************
 * @file     mesh_fn.h 
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
#include "app/app.h"
// #include "mesh_lpn/app.h"
// #include "mesh_provision/app.h"
// #include "mesh_switch/app.h"

enum{
    FS_DISCONNECT_TYPE_POLL_TIMEOUT = 1,
    FS_DISCONNECT_TYPE_CLEAR        = 2,
    FS_DISCONNECT_TYPE_RX_REQUEST   = 3, // receive request when being keeping friend ship.
};

void mesh_feature_set_fn();
void fn_update_RecWin(u8 RecWin);
u32 get_RecWin_connected();
void friend_ship_establish_ok_cb_fn(u8 lpn_idx);
void friend_ship_disconnect_cb_fn(u8 lpn_idx, int type);
u8 mesh_get_fn_cache_size_log_cnt();


extern u8 g_max_lpn_num;
