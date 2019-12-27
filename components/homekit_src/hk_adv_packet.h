/********************************************************************************************************
 * @file     hk_adv_packet.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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

#ifndef HK_ADV_PACKET_H_
#define HK_ADV_PACKET_H_

#pragma once

/* ------------------------------------
    Header Files
   ------------------------------------ */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "homekit_inc.h"
#include "homekit_def.h"

extern rf_packet_scan_rsp_t	pkt_scan_rsp;				// defined in ll_adv.c kite sdk v3.3 2019/4/30

extern u16 blt_adv_global_state;
extern u8 blt_adv_global_state_modify_once;
extern u16 blt_broadcast_global_state;

void blt_adv_update_pkt(u8 field, const u8 *pData);
void blt_broadcast_update_pkt(u8 field, const u8 *pData);
void blt_adv_set_pair_flag (int paired);
void blt_adv_set_global_state ();
void blt_broadcast_set_global_state();

void blt_set_normal_adv_pkt(u16 cat, const char * localname);
void blt_set_encryption_broadcast_pkt(u16 enc_iid, u8 * charval);

void blt_set_scan_rsp_pkt(u8 *p_mac, const char * localname, u8 *p_rsp);
////////////////clean broadcast param	////////////////////
void blt_clean_broadcast_param(void);

#endif /* HK_ADV_PACKET_H_ */
