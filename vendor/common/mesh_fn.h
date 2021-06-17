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
#include "vendor/mesh/app.h"
#include "vendor/mesh_lpn/app.h"
#include "vendor/mesh_provision/app.h"
#include "vendor/mesh_switch/app.h"

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
void friend_cmd_send_offer(u8 lpn_idx);
void friend_cmd_send_update(u8 lpn_idx, u8 md);
void friend_cmd_send_clear(u16 adr_dst, u8 *par, u32 len);
void friend_cmd_send_clear_conf(u16 adr_dst, u8 *par, u32 len);
void friend_cmd_send_subsc_conf(u16 adr_dst, u8 transNo);
int mesh_tx_cmd_layer_upper_ctl_FN(u8 op, u8 *par, u32 len_par, u16 adr_dst);
void mesh_iv_update_start_poll_fn(u8 iv_update_by_sno, u8 beacon_iv_update_pkt_flag);
u16 mesh_group_match_friend(u16 adr);
int fn_cache_get_extend_adv_short_unseg(u32 ctl, u8 len_ut_max_single_seg_rx);


extern u8 g_max_lpn_num;
extern u8 use_mesh_adv_fifo_fn2lpn;

