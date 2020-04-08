/********************************************************************************************************
 * @file     aes_cbc.c 
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
#ifndef _AES_H_
#define _AES_H_

#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 0
#endif

#ifndef CTR
  #define CTR 0
#endif

#define AES128 1
#define AES192 0
#define AES256 0

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN 32
    #define AES_keyExpSize 240
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN 24
    #define AES_keyExpSize 208
#else
    #define AES_KEYLEN 16   // Key length in bytes
    #define AES_keyExpSize 176
	#define AES_128_HW_MODE 1 // use hardware aes128
#endif

#ifndef AES_128_HW_MODE
#define AES_128_HW_MODE		0
#endif

typedef struct
{
  uint8_t RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
  uint8_t Iv[AES_BLOCKLEN];
#endif
}AES_ctx;

extern AES_ctx ctx;

void AES_init_ctx(AES_ctx* ctx, const uint8_t* key);

//note:padding_buf must AES_BLOCKLEN(16) larger than src_buf.
void AES128_pkcs7_padding(u8 *src_buf, u32 len, u8 *padding_buf);
void tn_aes_128(unsigned char *key, unsigned char *plaintext, unsigned char *result);
int aes_decrypt(unsigned char *Key, unsigned char *Data, unsigned char *Result);
void aes_ll_decryption(unsigned char *Key, unsigned char *Data, unsigned char *Result);

#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(ECB) && (ECB == 1)
void AES_ECB_encrypt(const AES_ctx* ctx, uint8_t* buf);
void AES_ECB_decrypt(const AES_ctx* ctx, uint8_t* buf);
#endif // #if defined(ECB) && (ECB == !)

#if defined(CBC) && (CBC == 1)
void AES_CBC_encrypt_buffer(AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_decrypt_buffer(AES_ctx* ctx, uint8_t* buf, uint32_t length);
void aes_cbc_encrypt(uint8_t *buf, uint32_t length, AES_ctx* ctx, uint8_t *key, const u8 *iv);
void aes_cbc_decrypt(uint8_t *buf, uint32_t length, AES_ctx* ctx, uint8_t *key, const u8 *iv);
#endif

#if defined(CTR) && (CTR == 1)
void AES_CTR_xcrypt_buffer(AES_ctx* ctx, uint8_t* buf, uint32_t length);
#endif


#endif
