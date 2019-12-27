/********************************************************************************************************
 * @file     aes.h 
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
#ifndef _AES_H
#define _AES_H

#include <stdlib.h>

/*  This include is used to find 8 & 32 bit unsigned integer types  */
#include "brg_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define AES_128     /* define if AES with 128 bit keys is needed    */
#define AES_192     /* define if AES with 192 bit keys is needed    */
#define AES_256     /* define if AES with 256 bit keys is needed    */
#define AES_VAR     /* define if a variable key size is needed      */
#define AES_MODES   /* define if support is needed for modes        */

/* The following must also be set in assembler files if being used  */

#define AES_ENCRYPT /* if support for encryption is needed          */
#define AES_DECRYPT /* if support for decryption is needed          */
#define AES_ERR_CHK /* for parameter checks & error return codes    */
#define AES_REV_DKS /* define to reverse decryption key schedule    */

#define AES_BLOCK_SIZE  16  /* the AES block size in bytes          */
#define N_COLS           4  /* the number of columns in the state   */

/* The key schedule length is 11, 13 or 15 16-byte blocks for 128,  */
/* 192 or 256-bit keys respectively. That is 176, 208 or 240 bytes  */
/* or 44, 52 or 60 32-bit words.                                    */

#if defined( AES_VAR ) || defined( AES_256 )
#define KS_LENGTH       60
#elif defined( AES_192 )
#define KS_LENGTH       52
#else
#define KS_LENGTH       44
#endif

#if defined( AES_ERR_CHK )
#define AES_RETURN     INT_RETURN
#else
#define AES_RETURN     VOID_RETURN
#endif

#if defined( LONG_MESSAGES )
  typedef u64 length_t;
#else
  typedef u32 length_t;
#endif

/* the character array 'inf' in the following structures is used    */
/* to hold AES context information. This AES code uses cx->inf.b[0] */
/* to hold the number of rounds multiplied by 16. The other three   */
/* elements can be used by code that implements additional modes    */

typedef union
{   u32 l;
    u8 b[4];
} aes_inf;

typedef struct
{   u32 ks[KS_LENGTH];
    aes_inf inf;
} aes_encrypt_ctx;

typedef struct
{   u32 ks[KS_LENGTH];
    aes_inf inf;
} aes_decrypt_ctx;

/* This routine must be called before first use if non-static       */
/* tables are being used                                            */

AES_RETURN aes_init(void);

/* Key lengths in the range 16 <= key_len <= 32 are given in bytes, */
/* those in the range 128 <= key_len <= 256 are given in bits       */

#if defined( AES_ENCRYPT )

#if defined(AES_128) || defined(AES_VAR)
AES_RETURN aes_encrypt_key128(const unsigned char *key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_192) || defined(AES_VAR)
AES_RETURN aes_encrypt_key192(const unsigned char *key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_256) || defined(AES_VAR)
AES_RETURN aes_encrypt_key256(const unsigned char *key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_VAR)
AES_RETURN aes_encrypt_key(const unsigned char *key, int key_len, aes_encrypt_ctx cx[1]);
#endif

AES_RETURN aes_encrypt(const unsigned char *in, unsigned char *out, const aes_encrypt_ctx cx[1]);

#endif

#if defined( AES_DECRYPT )

#if defined(AES_128) || defined(AES_VAR)
AES_RETURN aes_decrypt_key128(const unsigned char *key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_192) || defined(AES_VAR)
AES_RETURN aes_decrypt_key192(const unsigned char *key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_256) || defined(AES_VAR)
AES_RETURN aes_decrypt_key256(const unsigned char *key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_VAR)
AES_RETURN aes_decrypt_key(const unsigned char *key, int key_len, aes_decrypt_ctx cx[1]);
#endif

AES_RETURN aes_decrypt(const unsigned char *in, unsigned char *out, const aes_decrypt_ctx cx[1]);

#endif

#if defined(AES_MODES)

/* Multiple calls to the following subroutines for multiple block   */
/* ECB, CBC, CFB, OFB and CTR mode encryption can be used to handle */
/* long messages incremantally provided that the context AND the iv */
/* are preserved between all such calls.  For the ECB and CBC modes */
/* each individual call within a series of incremental calls must   */
/* process only full blocks (i.e. len must be a multiple of 16) but */
/* the CFB, OFB and CTR mode calls can handle multiple incremental  */
/* calls of any length. Each mode is reset when a new AES key is    */
/* set but ECB and CBC operations can be reset without setting a    */
/* new key by setting a new IV value.  To reset CFB, OFB and CTR    */
/* without setting the key, aes_mode_reset() must be called and the */
/* IV must be set.  NOTE: All these calls update the IV on exit so  */
/* this has to be reset if a new operation with the same IV as the  */
/* previous one is required (or decryption follows encryption with  */
/* the same IV array).                                              */

AES_RETURN aes_test_alignment_detection(unsigned int n);

AES_RETURN aes_ecb_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, const aes_encrypt_ctx cx[1]);

AES_RETURN aes_ecb_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, const aes_decrypt_ctx cx[1]);

AES_RETURN aes_cbc_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, const aes_encrypt_ctx cx[1]);

AES_RETURN aes_cbc_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, const aes_decrypt_ctx cx[1]);

AES_RETURN aes_mode_reset(aes_encrypt_ctx cx[1]);

AES_RETURN aes_cfb_encrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx cx[1]);

AES_RETURN aes_cfb_decrypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx cx[1]);

#define aes_ofb_encrypt aes_ofb_crypt
#define aes_ofb_decrypt aes_ofb_crypt

AES_RETURN aes_ofb_crypt(const unsigned char *ibuf, unsigned char *obuf,
                    int len, unsigned char *iv, aes_encrypt_ctx cx[1]);

typedef void cbuf_inc(unsigned char *cbuf);

#define aes_ctr_encrypt aes_ctr_crypt
#define aes_ctr_decrypt aes_ctr_crypt

AES_RETURN aes_ctr_crypt(const unsigned char *ibuf, unsigned char *obuf,
            int len, unsigned char *cbuf, cbuf_inc ctr_inc, aes_encrypt_ctx cx[1]);

#endif

#if defined(__cplusplus)
}
#endif

#endif
