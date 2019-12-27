/********************************************************************************************************
 * @file     t_sha.h
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

#include "../sha/sha512.h"

// use sha512  instead of sha1
typedef sha512_context		SHA_CTX;
typedef sha512_context		SHACTX;
#define SHAInit 			sha512_init_start
#define SHA1Init 			sha512_init
#define SHA1Update 			sha512_update
#define SHAUpdate 			sha512_update
#define SHA1Final(o,c)		sha512_finish(c,o)
#define SHAFinal(o,c)		sha512_finish(c,o)
// #define SHA_DIGESTSIZE 	20
// #define SHA_BlockSize 		128

