/********************************************************************************************************
 * @file     ed25519-hash.h
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
#include "../Configuration.h"
#include "../sha/sha512.h"

//typedef SHA512_CTX SHACTX;
typedef sha512_context SHACTX;

typedef SHACTX ed25519_hash_context;

static void
ed25519_hash_init(ed25519_hash_context *ctx) {
	//SHAInit(ctx);
	sha512_init_start (ctx);
}

static void
ed25519_hash_update(ed25519_hash_context *ctx, const uint8_t *in, size_t inlen) {
	//SHAUpdate(ctx, in, inlen);
	sha512_update(ctx, in, inlen);
}

static void
ed25519_hash_final(ed25519_hash_context *ctx, uint8_t *hash) {
	//SHAFinal(hash, ctx);
	sha512_finish(ctx, hash);
}

static void
ed25519_hash(uint8_t *hash, const uint8_t *in, size_t inlen) {
	//SHA512(in, inlen, hash);
	sha512(in, inlen, hash, 0);
}
