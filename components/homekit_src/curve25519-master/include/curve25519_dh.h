/********************************************************************************************************
 * @file     curve25519_dh.h
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
#ifndef __curve25519_dh_key_exchange_h__
#define __curve25519_dh_key_exchange_h__

#ifdef __cplusplus
extern "C" {
#endif

/* Return public key associated with sk */
/* sk will be trimmed on return */
void curve25519_dh_CalculatePublicKey(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

/* Faster alternative for curve25519_dh_CalculatePublicKey */
/* sk will be trimmed on return */
void curve25519_dh_CalculatePublicKey_fast(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

/* sk will be trimmed on return */
void curve25519_dh_CreateSharedKey(
    unsigned char *shared,      /* [32-bytes] OUT: Created shared key */
    const unsigned char *pk,    /* [32-bytes] IN: Other side's public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

#ifdef __cplusplus
}
#endif
#endif  /* __curve25519_dh_key_exchange_h__ */