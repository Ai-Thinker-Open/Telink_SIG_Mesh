/********************************************************************************************************
 * @file     mesh_fn.c 
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
#include "proj/tl_common.h"
#if !WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "proj_lib/pm.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "lighting_model.h"
#include "mesh/sensors_model.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "mesh_common.h"

u8 g_max_lpn_num = MAX_LPN_NUM;

#if 1 // only friend node use
STATIC_ASSERT(MAX_LPN_NUM <= 16);	// because F2L is 16bit

// mesh_friend_seg_cache_t friend_seg_cache;
u8 mesh_fri_cache_fifo_b[MAX_LPN_NUM][sizeof(mesh_cmd_bear_unseg_t) * (1 << FN_CACHE_SIZE_LOG)]={{0}};
my_fifo_t mesh_fri_cache_fifo[MAX_LPN_NUM] = {};//{,,0,0, name##_b};

mesh_ctl_fri_poll_t fn_poll[MAX_LPN_NUM] = {};
mesh_ctl_fri_update_t fn_update[MAX_LPN_NUM] = {};
mesh_ctl_fri_req_t fn_req[MAX_LPN_NUM] = {};
mesh_ctl_fri_offer_t fn_offer[MAX_LPN_NUM] = {};
mesh_fri_ship_other_t fn_other_par[MAX_LPN_NUM] = {};
fn_ctl_rsp_delay_t fn_ctl_rsp_delay[MAX_LPN_NUM] = {};

mesh_fri_ship_proc_fn_t fri_ship_proc_fn[MAX_LPN_NUM] = {};
#endif

void mesh_feature_set_fn(){
    #if FRI_SAMPLE_EN
	friend_ship_sample_message_test();
    #else
	#if (FEATURE_FRIEND_EN)
	foreach(i, MAX_LPN_NUM){
	    fn_other_par[i].FriAdr = ele_adr_primary;

	    mesh_ctl_fri_update_t *p_update = fn_update+i;
	    memcpy(p_update->IVIndex, iv_idx_st.tx, sizeof(p_update->IVIndex));

	    mesh_ctl_fri_offer_t *p_offer = (fn_offer+i);
	    #if 0
	    if(ele_adr_primary == ADR_FND2){
	        p_offer->RecWin = 0xfa;
	        p_offer->CacheSize = 0x02;
	        p_offer->SubsListSize = 0x05;
	        p_offer->RSSI = -90;    // 0xa6
	        p_offer->FriCounter = 0x000a - 1;  // increase counter later
	    }else
	    #endif
	    {
	        p_offer->RecWin = FRI_REC_WIN_MS; // 200;// 
	        p_offer->CacheSize = 1 << FN_CACHE_SIZE_LOG;
	        p_offer->SubsListSize = SUB_LIST_MAX_LPN;
	        p_offer->RSSI = FN_RSSI_INVALID; // -70;    // 0xba
	        p_offer->FriCounter = 0x072f - 1;  // increase counter later
	    }
	}
	#endif
    #endif
}

#if (FEATURE_FRIEND_EN)
#if 0
void fn_update_RecWin(u8 RecWin)
{
#if 0   // no stop friend ship should be better.
    if(!is_fn_support_and_en){
        return;
    }
    
	foreach(i, MAX_LPN_NUM){
	    mesh_ctl_fri_offer_t *p_offer = (fn_offer+i);
	    if(p_offer->RecWin != RecWin){
            p_offer->RecWin = RecWin;
            if(fn_other_par[i].link_ok){
                mesh_friend_ship_proc_init_fn(i);    // stop friend ship
            }
        }
    }
#endif
}

u32 get_RecWin_connected()
{
    return ((get_blt_conn_interval_us()+999)/1000 + 10);
}
#endif

u8 mesh_get_fn_cache_size_log_cnt()
{
    return (1<<FN_CACHE_SIZE_LOG);
}

/**
* friend node: when friend ship establish ok, this function would be called.
* @params: lpn_idx: array index of fn_other_par[].
*/
void friend_ship_establish_ok_cb_fn(u8 lpn_idx)
{

}

/**
* friend node: when friend ship disconnect, this function would be called.
* @params: lpn_idx: array index of fn_other_par[].
* @params: type: disconnect type.for example FS_DISCONNECT_TYPE_POLL_TIMEOUT.
*/
void friend_ship_disconnect_cb_fn(u8 lpn_idx, int type)
{

}

#endif

void mesh_global_var_init_fn_buf()
{
	foreach(i,MAX_LPN_NUM){
		mesh_fri_cache_fifo[i].size = sizeof(mesh_cmd_bear_unseg_t);
		mesh_fri_cache_fifo[i].num = (1 << FN_CACHE_SIZE_LOG);
		mesh_fri_cache_fifo[i].wptr = 0;
		mesh_fri_cache_fifo[i].rptr = 0;
		mesh_fri_cache_fifo[i].p = mesh_fri_cache_fifo_b[i];
	}
}

