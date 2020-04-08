/********************************************************************************************************
 * @file     ctr.c 
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
#include <math.h>
#include "../../common/types.h"
#include "inc/ctr.h"
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

#define MAX_NONCE_LEN   13

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


void mod_inc_ctr(ctr_buf_t x, ctr_buf_t modVal, u8 mod)
{
    u8 i;

    if (!memcmp(UI8_PTR(x), UI8_PTR(modVal), BLOCK_SIZE)) {
        /* identical, that means counter go to the UPP value, reload to 0 */
        if (mod >=0 && mod <= 15) {
            memset(UI8_PTR(x), 0, BLOCK_SIZE);
        }
        else {
            u8 byteNum = mod / 8;
            u8 bitNum  = mod % 8;
            for (i = 0; i < byteNum; i++) {
                UI8_PTR(x)[BLOCK_SIZE - i - 1] = 0;
            }

            for(i = 0; i < bitNum; i++) {
                UI8_PTR(x)[BLOCK_SIZE - byteNum - 1] &= ~1<<i;
            }
        }


    }
    else {
        u8 *_p = (u8*)(x) + BLOCK_SIZE;
        do {
            ++(*--_p);
        } while(!*_p);
    }



}


/*****************************************************************************/

u32 ctr_init_and_enkey(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            ctr_ctx ctx[1])                /* the mode context            */
{
  ctx->state = RETURN_GOOD;
  if ((key_len != 16) && (key_len != 24) && (key_len != 32))
  {
    ctx->state = CTR_BAD_KEY;
  }
  aes_encrypt_key(key, (int)key_len, ctx->aes);
  return RETURN_GOOD;
}

u32 ctr_init_and_dekey(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            ctr_ctx ctx[1])                /* the mode context            */
{
  ctx->state = RETURN_GOOD;
  if ((key_len != 16) && (key_len != 24) && (key_len != 32))
  {
    ctx->state = CTR_BAD_KEY;
  }
  aes_decrypt_key(key, (int)key_len, (aes_decrypt_ctx*)&ctx->aes);
  return RETURN_GOOD;
}

/*****************************************************************************/

u32 ctr_init_message(                 /* initialise for a new message   */
            const unsigned char iv[],      /* the initialisation vector      */
            unsigned long iv_len,          /* the nonce length               */
            u32 initCounter,          /* the initialisation counter     */
            u8  mod,                  /* the initialisation counter mod */
            ctr_ctx ctx[1])                /* the mode context               */
{
  u32 cnt;
  u8  nonceLen;
  u8  byteNum;
  u8  bitNum;
  u8  i;

  ctx->state = RETURN_GOOD;

  ctx->mod = mod;
  ctx->cnt = 0;
  ctx->txt_acnt = 0;


#if 1
  memset(UI8_PTR(ctx->ctr_val), 0, 16);

  for(cnt = 0;
      cnt < iv_len;
      ++cnt)
  {
    UI8_PTR(ctx->ctr_val)[cnt] = iv[cnt];
  }

  /* initialize T1 */
  memcpy(UI8_PTR(ctx->mod_val),UI8_PTR(ctx->ctr_val), BLOCK_SIZE);

  if (ctx->mod >= 0 && ctx->mod <= 15) {
      memset(UI8_PTR(ctx->mod_val), 0xff, BLOCK_SIZE);
  }
  else {
      byteNum = ctx->mod / 8;
      bitNum  = ctx->mod % 8;

      for (i = 0; i < byteNum; i++) {
          UI8_PTR(ctx->mod_val)[BLOCK_SIZE - i - 1] |= 0xff;
      }

      for (i = 0; i < bitNum; i++) {
          UI8_PTR(ctx->mod_val)[BLOCK_SIZE-byteNum - 1] |= 1<<i;
      }
  }


#else

  UI8_PTR(ctx->ctr_val)[0] = 0x01;

  /* move the iv into the block */
  for(cnt = 1;
      cnt < (u32)BLOCK_SIZE - UI8_PTR(ctx->ctr_val)[0] - 1;
      ++cnt)
  {
    UI8_PTR(ctx->ctr_val)[cnt] = iv[cnt - 1];
  }

  /* set initial counter     */
  UI8_PTR(ctx->ctr_val)[BLOCK_SIZE - 1] = 1;


#endif


  return RETURN_GOOD;
}

/*****************************************************************************/

u32 ctr_encrypt_data(                   /* encrypt or decrypt data */
            unsigned char data[],          /* the data buffer         */
            unsigned long data_len,        /* and its length in bytes */
            ctr_ctx ctx[1])                /* the mode context        */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->cnt) & BLK_ADR_MASK;


  if (ctx->state < 0)
  {
    //dbgp_ccm("   ctx->state\n");
    return ctx->state;
  }

  if (!data_len)
  {
    //dbgp_ccm("   ctx->data_len = 0\n");
    return RETURN_GOOD;
  }

  if (b_pos == 0)
  {
    aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
    //inc_ctr(ctx->ctr_val, ctx->mod);
    mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
  }

  if (!((data - (UI8_PTR(ctx->enc_ctr) + b_pos)) & BUF_ADRMASK))
  {
    while ((cnt < data_len) && (b_pos & BUF_ADRMASK))
    {
      data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    while ((cnt + BUF_INC <= data_len) && (b_pos <= BLOCK_SIZE - BUF_INC))
    {
      *UNIT_PTR(data + cnt) ^= *UNIT_PTR(UI8_PTR(ctx->enc_ctr) + b_pos);
      cnt += BUF_INC; b_pos += BUF_INC;
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      xor_block_aligned(data + cnt, data + cnt, ctx->enc_ctr);
      cnt += BLOCK_SIZE;
    }
  }
  else
  {
    while ((cnt < data_len) && (b_pos < BLOCK_SIZE))
    {
      data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      xor_block(data + cnt, data + cnt, ctx->enc_ctr);
      cnt += BLOCK_SIZE;
    }
  }

  while (cnt < data_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      b_pos = 0;
    }
    data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
  }

  ctx->cnt += cnt;
  return RETURN_GOOD;
}


u32 ctr_decrypt_data(                   /* encrypt or decrypt data */
            unsigned char data[],          /* the data buffer         */
            unsigned long data_len,        /* and its length in bytes */
            ctr_ctx ctx[1])                /* the mode context        */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->cnt) & BLK_ADR_MASK;


  if (ctx->state < 0)
  {
    //dbgp_ccm("   ctx->state\n");
    return ctx->state;
  }

  if (!data_len)
  {
    //dbgp_ccm("   ctx->data_len = 0\n");
    return RETURN_GOOD;
  }

  if (b_pos == 0)
  {
    aes_decrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), (aes_decrypt_ctx*)&ctx->aes);
    //inc_ctr(ctx->ctr_val, ctx->mod);
    mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
  }

  if (!((data - (UI8_PTR(ctx->enc_ctr) + b_pos)) & BUF_ADRMASK))
  {
    while ((cnt < data_len) && (b_pos & BUF_ADRMASK))
    {
      data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    while ((cnt + BUF_INC <= data_len) && (b_pos <= BLOCK_SIZE - BUF_INC))
    {
      *UNIT_PTR(data + cnt) ^= *UNIT_PTR(UI8_PTR(ctx->enc_ctr) + b_pos);
      cnt += BUF_INC; b_pos += BUF_INC;
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_decrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), (aes_decrypt_ctx*)&ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      xor_block_aligned(data + cnt, data + cnt, ctx->enc_ctr);
      cnt += BLOCK_SIZE;
    }
  }
  else
  {
    while ((cnt < data_len) && (b_pos < BLOCK_SIZE))
    {
      data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_decrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), (aes_decrypt_ctx*)&ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      xor_block(data + cnt, data + cnt, ctx->enc_ctr);
      cnt += BLOCK_SIZE;
    }
  }

  while (cnt < data_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_decrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), (aes_decrypt_ctx*)&ctx->aes);
      mod_inc_ctr(ctx->ctr_val, ctx->mod_val, ctx->mod);
      b_pos = 0;
    }
    data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
  }

  ctx->cnt += cnt;
  return RETURN_GOOD;
}

/*****************************************************************************/

u32 ctr_encrypt_message(              /* encrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            u32 mod,                  /* counter mod */
            u32 initCounter,          /* initialize counter */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            ctr_ctx ctx[1])                /* the mode context          */
{

    u32 retVal;

    do {

        if (msg_len == 0) {
        }

        retVal = ctr_init_message(iv, iv_len, initCounter, mod, ctx);
        if (RETURN_GOOD != retVal) {
            break;
        }

        retVal = ctr_encrypt_data(msg, msg_len, ctx);
        if (retVal != RETURN_GOOD) {
            break;
        }


    }while(0);

    return retVal;

}



/*****************************************************************************/

u32 ctr_decrypt_message(              /* encrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            u32 mod,                  /* counter mod */
            u32 initCounter,          /* initialize counter */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            ctr_ctx ctx[1])                /* the mode context          */
{

    u32 retVal;

    do {

        retVal = ctr_init_message(iv, iv_len, initCounter, mod, ctx);
        if (RETURN_GOOD != retVal) {
            break;
        }

        retVal = ctr_encrypt_data(msg, msg_len, ctx);
        if (retVal != RETURN_GOOD) {
            break;
        }

    }while(0);

    return retVal;

}





#if defined(__cplusplus)
}
#endif
