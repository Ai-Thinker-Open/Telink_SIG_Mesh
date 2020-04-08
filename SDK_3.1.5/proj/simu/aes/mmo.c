/********************************************************************************************************
 * @file     mmo.c 
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

#include <stdio.h>
#include "../../common/types.h"
#include "inc/mmo.h"
#include "inc/mode_hdr.h"
#include "inc/debug.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define BLOCK_SIZE      AES_BLOCK_SIZE      /* block length                */
#define BLK_ADR_MASK    (BLOCK_SIZE - 1)    /* mask for 'in block' address */
#define INIT_STATE      10
#define AUTH_STATE      11
#define MSG_STATE       12

#define MAX_NONCE_LEN   12

#define ctr_len(x)      (((*(u8*)(x)) & 0x07) + 1)

#define clr_ctr(x,l)    memset((u8*)(x) + BLOCK_SIZE - (l), 0, (l))

#define set_ctr(x,v)                                \
  {                                                 \
    u8 *_p = (u8*)(x) + BLOCK_SIZE;       \
    length_t _t = (v), _l = ctr_len(x);             \
    do                                              \
    {                                               \
      *--_p = (u8)_t; _t >>= 8;                \
    } while (--_l);                                 \
  }

#if 0
#define inc_ctr(x)                                  \
  {                                                 \
    u8 *_p = (u8*)(x) + BLOCK_SIZE;       \
    length_t _l = ctr_len(x);                       \
    while (_l-- && ++(*--_p) == 0) ;                \
  }
#else
#define inc_ctr(x, m)                               \
  do {                                              \
      u8 *_p = (u8*)(x) + BLOCK_SIZE;     \
      if ((++(*--_p)) == ctx->mod)                  \
            *_p = 0;                                \
  }while(0);
#endif

#ifdef LONG_MESSAGES

#  define mlen_len(x)                     \
    (((x) & 0xff00000000000000ll) ? 7 :     \
     ((x) & 0xffff000000000000ll) ? 6 :     \
     ((x) & 0xffffff0000000000ll) ? 5 :     \
     ((x) & 0xffffffff00000000ll) ? 4 :     \
     ((x) & 0xffffffffff000000ll) ? 3 :     \
     ((x) & 0xffffffffffff0000ll) ? 2 : 1)

#else
#  define mlen_len(x) (((x) & 0xff000000) ? 3 : ((x) & 0xffff0000) ? 2 : 1)
#endif
extern int j;



/*****************************************************************************/

u32 mmo_init_and_key(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            mmo_ctx ctx[1])                /* the mode context            */
{
    ctx->state = RETURN_GOOD;
    if ((key_len != 16)) {
        ctx->state = MMO_BAD_KEY;
    }

#ifdef KEYED_HASH_FUNCTION_FOR_MSG_AUTH
    /* init the ipad and opad */
    memset(UI8_PTR(ctx->ipad), 0x36, 16);
    memset(UI8_PTR(ctx->opad), 0x5c, 16);

    xor_block_aligned(ctx->key1, key, ctx->ipad);
    xor_block_aligned(ctx->key2, key, ctx->opad);

#else
    memcpy(UI8_PTR(ctx->hash_val), key, BLOCK_SIZE);
#endif
    return RETURN_GOOD;
}


/*****************************************************************************/

u32 mmo_hash(
            unsigned char data[],           /* the message buffer        */
            length_t data_len,              /* and its length in bytes   */
            mmo_ctx ctx[1])                 /* the mode context          */
{
    u32 count = 0;
    u32  cnt = 0;
    u32  b_pos = 0;
    u32  bit_len = 0;

    /* hash0 */
    //memset(UI8_PTR(ctx->hash_val), 0, 16);
    memset(UI8_PTR(ctx->mmo_buf), 0, 16);

    while (cnt + BLOCK_SIZE <= data_len) {
        /* g() function */
        aes_encrypt_key(UI8_PTR(ctx->hash_val), BLOCK_SIZE, ctx->aes);

        memcpy(UI8_PTR(ctx->mmo_buf), data+cnt, BLOCK_SIZE);

        /* E(hash0, mi) */
        aes_encrypt(UI8_PTR(ctx->mmo_buf), UI8_PTR(ctx->hash_val), ctx->aes);

        /* E(hash0, mi) XOR mi */
        xor_block_aligned(ctx->hash_val, ctx->hash_val, ctx->mmo_buf);

        printf("HASH(%d) :\n", count++);
        for (j=0;j<16;j++) {
            printf("%x ", UI8_PTR(ctx->hash_val)[j]);
        }
        printf("\n");

        cnt += BLOCK_SIZE;
    }

    /* g(hash(n-1)) function */
    aes_encrypt_key(UI8_PTR(ctx->hash_val), BLOCK_SIZE, ctx->aes);

    memset(UI8_PTR(ctx->mmo_buf), 0, BLOCK_SIZE);
    memcpy(UI8_PTR(ctx->mmo_buf), data+cnt, data_len-cnt);
    UI8_PTR(ctx->mmo_buf)[data_len-cnt] = 0x80;
    if (data_len - cnt < BLOCK_SIZE - 2) {
        bit_len = data_len * 8;
        UI8_PTR(ctx->mmo_buf)[BLOCK_SIZE-2] = (bit_len & 0xff00) >> 8;
        UI8_PTR(ctx->mmo_buf)[BLOCK_SIZE-1] = bit_len & 0x00ff;


        /* calc the hash(n) */
        aes_encrypt(UI8_PTR(ctx->mmo_buf), UI8_PTR(ctx->hash_val), ctx->aes);
        xor_block_aligned(ctx->hash_val, ctx->hash_val, ctx->mmo_buf);

        printf("HASH(%d) :\n", count++);
        for (j=0;j<16;j++) {
            printf("%x ", UI8_PTR(ctx->hash_val)[j]);
        }
        printf("\n");
    }
    else {
        /* case in BLOCK_SIZE - 1, or BLOCK_SIZE - 2 bytes */

        /* calc the hash(n-1) */
        aes_encrypt(UI8_PTR(ctx->mmo_buf), UI8_PTR(ctx->hash_val), ctx->aes);
        xor_block_aligned(ctx->hash_val, ctx->hash_val, ctx->mmo_buf);

        memset(UI8_PTR(ctx->mmo_buf), 0, BLOCK_SIZE);
        bit_len = data_len * 8;
        UI8_PTR(ctx->mmo_buf)[BLOCK_SIZE-2] = (bit_len & 0xff00) >> 8;
        UI8_PTR(ctx->mmo_buf)[BLOCK_SIZE-1] = bit_len & 0x00ff;

        aes_encrypt_key(UI8_PTR(ctx->hash_val), BLOCK_SIZE, ctx->aes);

        aes_encrypt(UI8_PTR(ctx->mmo_buf), UI8_PTR(ctx->hash_val), ctx->aes);
        xor_block_aligned(ctx->hash_val, ctx->hash_val, ctx->mmo_buf);

        printf("HASH(%d) :\n", count++);
        for (j=0;j<16;j++) {
            printf("%x ", UI8_PTR(ctx->hash_val)[j]);
        }
        printf("\n");

    }


    return RETURN_GOOD;
}

/*****************************************************************************/

u32 mmo_encrypt_message(              /* encrypt an entire message */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            mmo_ctx ctx[1],
            u8 result[])                /* the mode context          */
{
#ifdef KEYED_HASH_FUNCTION_FOR_MSG_AUTH
    ret_type retVal = RETURN_GOOD;

    do {

        if (msg_len == 0) {
            retVal = MMO_MSG_LENGTH_ERROR;
            break;
        }

        ctx->cnt = 0;
        ctx->txt_acnt = 0;
        ctx->msg_len = msg_len;

        if (msg_len <= BLOCK_SIZE) {
            /* at least 32 bytes */
            ctx->mmo_tempBuf = (u8*)malloc(BLOCK_SIZE * 2 * sizeof(u8));
        } else {
            ctx->mmo_tempBuf = (u8*)malloc((BLOCK_SIZE + msg_len) * sizeof(u8));
        }

        if (!ctx->mmo_tempBuf) {
            retVal = MMO_MALLOC_ERROR;
            break;
        }

        ctx->mmo_tempBufLen = msg_len + BLOCK_SIZE;

        /* generate M1: M1 = key1 || M */
        memcpy(UI8_PTR(ctx->mmo_tempBuf), UI8_PTR(ctx->key1), BLOCK_SIZE);
        memcpy(UI8_PTR(ctx->mmo_tempBuf) + BLOCK_SIZE, msg, msg_len);

        /* hash1 = Hash(M1), result store in ctx->hash_val */
        mmo_hash(ctx->mmo_tempBuf, ctx->mmo_tempBufLen, ctx);

        /* generate M2: M2 = key2 || hash1 */
        memcpy(UI8_PTR(ctx->mmo_tempBuf), UI8_PTR(ctx->key2), BLOCK_SIZE);
        memcpy(UI8_PTR(ctx->mmo_tempBuf) + BLOCK_SIZE, UI8_PTR(ctx->hash_val), BLOCK_SIZE);
        ctx->mmo_tempBufLen = 2 * BLOCK_SIZE;

        /* hash2 = Hash(M2) */
        mmo_hash(ctx->mmo_tempBuf, ctx->mmo_tempBufLen, ctx);

        printf("Result:\n");
        for (j=0;j<16;j++) {
            printf("%x ", UI8_PTR(ctx->hash_val)[j]);
        }
        printf("\n");

        free(ctx->mmo_tempBuf);


    }while(0);

    return retVal;

#else

    mmo_hash(msg, msg_len, ctx);
    memcpy(result, UI8_PTR(ctx->hash_val), BLOCK_SIZE);

    printf("\nResult:\n");
    for (j=0;j<16;j++) {
        printf("%x ", result[j]);
    }
    printf("\n");

    return RETURN_GOOD;

#endif
}

#if defined(__cplusplus)
}
#endif
