/********************************************************************************************************
 * @file     socket_test.h 
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


#ifdef WIN32

#include<stdio.h>
#include<winsock2.h>

#include "../common/types.h"
#include "../common/assert.h"

#define RECEIVE_MAX_LENGTH  128
#define SEND_MAX_LENGTH     128
#define SOCKET_HOST         "127.0.0.1"
#define SOCKET_PORT_SERVER         7001
#define SOCKET_PORT_CLIENT         7002

#endif

