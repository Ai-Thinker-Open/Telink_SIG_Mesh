/********************************************************************************************************
 * @file     aescpp.h 
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

#ifndef _AESCPP_H
#define _AESCPP_H

#include "aes.h"

#if defined( AES_ENCRYPT )

class AESencrypt
{
public:
    aes_encrypt_ctx cx[1];
    AESencrypt(void) { aes_init(); };
#ifdef  AES_128
    AESencrypt(const unsigned char key[])
        {   aes_encrypt_key128(key, cx); }
    AES_RETURN key128(const unsigned char key[])
        {   return aes_encrypt_key128(key, cx); }
#endif
#ifdef  AES_192
    AES_RETURN key192(const unsigned char key[])
        {   return aes_encrypt_key192(key, cx); }
#endif
#ifdef  AES_256
    AES_RETURN key256(const unsigned char key[])
        {   return aes_encrypt_key256(key, cx); }
#endif
#ifdef  AES_VAR
    AES_RETURN key(const unsigned char key[], int key_len)
        {   return aes_encrypt_key(key, key_len, cx); }
#endif
    AES_RETURN encrypt(const unsigned char in[], unsigned char out[]) const
        {   return aes_encrypt(in, out, cx);  }
#ifndef AES_MODES
    AES_RETURN ecb_encrypt(const unsigned char in[], unsigned char out[], int nb) const
        {   while(nb--)
            {   aes_encrypt(in, out, cx), in += AES_BLOCK_SIZE, out += AES_BLOCK_SIZE; }
        }
#endif
#ifdef AES_MODES
    AES_RETURN mode_reset(void)   { return aes_mode_reset(cx); }

    AES_RETURN ecb_encrypt(const unsigned char in[], unsigned char out[], int nb) const
        {   return aes_ecb_encrypt(in, out, nb, cx);  }

    AES_RETURN cbc_encrypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[]) const
        {   return aes_cbc_encrypt(in, out, nb, iv, cx);  }

    AES_RETURN cfb_encrypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[])
        {   return aes_cfb_encrypt(in, out, nb, iv, cx);  }

    AES_RETURN cfb_decrypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[])
        {   return aes_cfb_decrypt(in, out, nb, iv, cx);  }

    AES_RETURN ofb_crypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[])
        {   return aes_ofb_crypt(in, out, nb, iv, cx);  }

    typedef void ctr_fn(unsigned char ctr[]);

    AES_RETURN ctr_crypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[], ctr_fn cf)
        {   return aes_ctr_crypt(in, out, nb, iv, cf, cx);  }

#endif

};

#endif

#if defined( AES_DECRYPT )

class AESdecrypt
{
public:
    aes_decrypt_ctx cx[1];
    AESdecrypt(void) { aes_init(); };
#ifdef  AES_128
    AESdecrypt(const unsigned char key[])
            { aes_decrypt_key128(key, cx); }
    AES_RETURN key128(const unsigned char key[])
            { return aes_decrypt_key128(key, cx); }
#endif
#ifdef  AES_192
    AES_RETURN key192(const unsigned char key[])
            { return aes_decrypt_key192(key, cx); }
#endif
#ifdef  AES_256
    AES_RETURN key256(const unsigned char key[])
            { return aes_decrypt_key256(key, cx); }
#endif
#ifdef  AES_VAR
    AES_RETURN key(const unsigned char key[], int key_len)
            { return aes_decrypt_key(key, key_len, cx); }
#endif
    AES_RETURN decrypt(const unsigned char in[], unsigned char out[]) const
        {   return aes_decrypt(in, out, cx);  }
#ifndef AES_MODES
    AES_RETURN ecb_decrypt(const unsigned char in[], unsigned char out[], int nb) const
        {   while(nb--)
            {   aes_decrypt(in, out, cx), in += AES_BLOCK_SIZE, out += AES_BLOCK_SIZE; }
        }
#endif
#ifdef AES_MODES

    AES_RETURN ecb_decrypt(const unsigned char in[], unsigned char out[], int nb) const
        {   return aes_ecb_decrypt(in, out, nb, cx);  }

    AES_RETURN cbc_decrypt(const unsigned char in[], unsigned char out[], int nb,
                                    unsigned char iv[]) const
        {   return aes_cbc_decrypt(in, out, nb, iv, cx);  }
#endif
};

#endif

#endif
