/********************************************************************************************************
 * @file     hk_indicate_queue.h
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

#ifndef HK_INDICATE_QUEUE_H_
#define HK_INDICATE_QUEUE_H_


#pragma once

/* ------------------------------------
    Header Files
   ------------------------------------ */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "homekit_inc.h"
#include "homekit_def.h"

#define MAX_INDICATE_HANDLE_QUEUE_NUM   24

extern u8 ccc_indicate_handle_queue[MAX_INDICATE_HANDLE_QUEUE_NUM];
extern u8 pkt_ll_buf[36];

extern int indicate_handle_no_reg(unsigned char handle_no);
extern int indicate_handle_no_remove(unsigned char handle_no);
extern int indicate_handle_no_send_once_after_checkit(unsigned char handle_no);
extern int indicate_handle_no_sendout_all(void);
#endif /* HK_INDICATE_QUEUE_H_ */
