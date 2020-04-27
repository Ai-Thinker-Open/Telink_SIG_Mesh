/********************************************************************************************************
 * @file     mesh_test_cmd.h 
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

#define SEC_MES_DEBUG_EN    0
#define FRIEND_MSG_TEST_EN  0

#define FRI_SAMPLE_EN       0
#define	SEG_DEBUG_LED(on)	//do{ light_onoff_all(on);}while(0)


void mesh_message6();
void friend_cmd_send_sample_message(u8 op);
void friend_ship_sample_message_test();
void mesh_message_fri_msg_ctl_seg();
int mesh_tx_cmd_layer_upper_ctl_primary(u8 op, u8 *par, u32 len_par, u16 adr_dst);
void test_cmd_wakeup_lpn();
void test_cmd_tdebug();


