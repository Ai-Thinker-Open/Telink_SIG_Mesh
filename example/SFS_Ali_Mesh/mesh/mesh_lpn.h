/********************************************************************************************************
 * @file     mesh_lpn.h 
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

#if LPN_DEBUG_PIN_EN
void lpn_debug_set_current_pin(u8 level);
void lpn_debug_set_debug_pin(u8 level);
void lpn_debug_set_irq_pin(u8 level);
void lpn_debug_set_event_handle_pin(u8 level);
void lpn_debug_alter_debug_pin(int reset);
#else
#define lpn_debug_set_current_pin(level)  
#define lpn_debug_alter_debug_pin(reset)  
#define lpn_debug_set_irq_pin(level) 
#define lpn_debug_set_event_handle_pin(level)   
#endif

void friend_cmd_send_poll();
void mesh_friend_ship_start_poll();
void mesh_feature_set_lpn();
void suspend_quick_check();
void lpn_quick_tx_and_suspend(int suspend, u8 op);
int lpn_rx_offer_handle(u8 *bear);
void lpn_no_offer_handle();
void subsc_add_rmv_test(int add);
void pts_test_case_lpn();
void lpn_subsc_list_update_by_sub_set_cmd(u16 op, u16 sub_adr);
void mesh_main_loop_LPN();
void friend_send_current_subsc_list();
void suspend_handle_next_poll_interval(int handle_type);
void suspend_handle_wakeup_rx(int handle_type);
void mesh_friend_ship_retry();
void send_gatt_adv_right_now();
u8 lpn_get_poll_retry_max();

enum{
    HANDLE_NONE                     = 0,
    HANDLE_RETENTION_DEEP_PRE       = BIT(0),
    HANDLE_RETENTION_DEEP_AFTER     = BIT(1),
    HANDLE_RETENTION_DEEP_ADV_PRE   = BIT(2),
    HANDLE_SUSPEND_NORMAL           = HANDLE_RETENTION_DEEP_PRE | HANDLE_RETENTION_DEEP_AFTER,
};

enum{
    LPN_SUSPEND_EVENT_NONE          = 0,
    LPN_SUSPEND_EVENT_NEXT_POLL_INV = 1,
    LPN_SUSPEND_EVENT_WAKEUP_RX     = 2,
};

typedef struct {
	u8 type;
	u8 event;
	u8 adv_cnt;
}lpn_deep_handle_t;

extern lpn_deep_handle_t lpn_deep_handle;
extern u8 mesh_lpn_rx_master_key;
extern u8 send_gatt_adv_now_flag;
extern u8 fri_request_retry_max;
extern u8 lpn_deep_retention_en;
extern u16 lpn_establish_win_ms;
extern u32 lpn_wakeup_tick;