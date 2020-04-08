/********************************************************************************************************
 * @file     hk_l2cap_handle.c
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


#ifndef HK_L2CAP_HANDLE_H_
#define HK_L2CAP_HANDLE_H_


#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "homekit_inc.h"
#include "homekit_def.h"

extern int 	hk_l2cap_packet_receive (u16 connHandle, u8 * p);

#endif /* HK_L2CAP_HANDLE_H_ */
