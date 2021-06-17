/********************************************************************************************************
 * @file     app.h 
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


typedef struct{
	u8 kb_pressed;
	u8 adv_send_enable;
	u8 rc_deep_flag;
	u8 rc_sending_flag;
	u32 adv_send_tick;
	u32 adv_timeout_def_ms;
	u32 rc_start_tick;
}rc_para_mag;

extern rc_para_mag rc_mag;


void set_rc_flag_when_send_pkt(int is_sending);
void mesh_proc_keyboard ();
void global_reset_new_key_wakeup();


