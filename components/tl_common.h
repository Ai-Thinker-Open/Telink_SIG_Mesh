/********************************************************************************************************
 * @file     tl_common.h
 *
 * @brief    This is the header file for TLSR8258
 *
 * @author	 author@telink-semi.com;
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#pragma once
#include "proj/tl_common.h"

#define SHOW_FUNC_IN(msg, arg...) printf("> %s(%d): " msg "\n", __FUNCTION__,__LINE__, ##arg)
#define SHOW_FUNC_OUT(msg, arg...) printf("< %s(%d): " msg "\n\n", __FUNCTION__,__LINE__, ##arg)
#define SHOW_DBG(msg, arg...) printf("%s:%s(%d): " msg "\n", __FILE__, __FUNCTION__,__LINE__, ##arg)
