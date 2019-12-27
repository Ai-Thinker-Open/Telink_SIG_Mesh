/********************************************************************************************************
 * @file     sha512.h
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

#pragma once

#include "../../../tl_common.h"
#include "../../sha/sha512.h"

#include "BaseTypes.h"
#define SHA512_DIGEST_LENGTH    64
#define SHA512_CBLOCK           128     /* SHA-512 treats input data as a
                                         * contiguous array of 64 bit
                                         * wide big-endian values. */
#define SHA_LONG64 U64


#define	SHA512_CTX			sha512_context
#define SHA512_Init			sha512_init_start
#define SHA512_Update		sha512_update
#define SHA512_Final(a,b)	sha512_finish(b,a)
