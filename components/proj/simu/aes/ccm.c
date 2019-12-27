/********************************************************************************************************
 * @file     ccm.c 
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
#include "inc/ccm.h"
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

#define inc_ctr(x)                                  \
  {                                                 \
    u8 *_p = (u8*)(x) + BLOCK_SIZE;       \
    length_t _l = ctr_len(x);                       \
    while (_l-- && ++(*--_p) == 0) ;                \
  }

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
int j;

u32 ccm_init_and_key(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            ccm_ctx ctx[1])                /* the mode context            */
{
  ctx->state = RETURN_GOOD;
  if ((key_len != 16) && (key_len != 24) && (key_len != 32))
  {
    ctx->state = CCM_BAD_KEY;
  }
  aes_encrypt_key(key, (int)key_len, ctx->aes);
  return RETURN_GOOD;
}


u32 ccm_init_message(                 /* initialise for a new message */
            const unsigned char iv[],      /* the initialisation vector    */
            unsigned long iv_len,          /* the nonce length             */
            length_t hdr_len,              /* the associated data length   */
            length_t msg_len,              /* message data length          */
            unsigned long tag_len,         /* authentication field length  */
            ccm_ctx ctx[1])                /* the mode context             */
{
  u32 cnt;


  ctx->state = RETURN_GOOD;
  if ((tag_len < 2) || (tag_len > 16) || (tag_len & 1))
  {
    ctx->state = CCM_BAD_AUTH_FIELD_LENGTH;
  }
  else if (iv_len && (iv_len < 7) || (iv_len > 13))
  {
    ctx->state = CCM_BAD_NONCE_LENGTH;
  }
  if (ctx->state)
  {
    return ctx->state;
  }

  ctx->iv_len = iv_len;
  ctx->hdr_len = hdr_len;
  ctx->msg_len = msg_len;
  ctx->tag_len = tag_len;
  ctx->cnt = 0;
  ctx->txt_acnt = 0;
  ctx->hdr_lim = hdr_len;

  UI8_PTR(ctx->ctr_val)[0] = (u8)(iv_len ?            \
                             BLOCK_SIZE - 2 - iv_len :     \
                             mlen_len(ctx->msg_len));

  /* move the iv into the block */
  for(cnt = 1;
      cnt < (u32)BLOCK_SIZE - UI8_PTR(ctx->ctr_val)[0] - 1;
      ++cnt)
  {
    UI8_PTR(ctx->ctr_val)[cnt] = iv[cnt - 1];
  }

  /* clear the counter value */
  clr_ctr(ctx->ctr_val, UI8_PTR(ctx->ctr_val)[0] + 1);
  /* copy block to CBC buffer */
  memcpy(ctx->cbc_buf, ctx->ctr_val, BLOCK_SIZE);
  /* set initial counter     */
  UI8_PTR(ctx->ctr_val)[BLOCK_SIZE - 1] = 1;
  /* store the message length */
  set_ctr(ctx->cbc_buf, ctx->msg_len);

  UI8_PTR(ctx->cbc_buf)[0] |= (ctx->hdr_lim ? 0x40 : 0) +
                              ((ctx->tag_len - 2) << 2);

  //printf("B_0:\n");
  //for (j=0;j<16;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //}
  //printf("\n");
  /* encrypt the cbc block    */
  aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

  //printf("AES(B_0):\n");
  //for (j=0;j<16;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //}
  //printf("\n");

  if (ctx->hdr_len)
  {
    /* encode the length field if there is some associated data */
    if (ctx->hdr_len < 65536 - 256)
    {
      UI8_PTR(ctx->cbc_buf)[0] ^= (u8)(ctx->hdr_lim >> 8);
      UI8_PTR(ctx->cbc_buf)[1] ^= (u8) ctx->hdr_lim;
      ctx->cnt = 2;
    }
#ifndef LONG_MESSAGES
    else
    {
      ctx->state = CCM_AUTH_LENGTH_ERROR;
      return ctx->state;
    }
#else
    else if (ctx->hdr_len < 0x0000000100000000ll)
    {
      UI8_PTR(ctx->cbc_buf)[0] ^= 0xff;
      UI8_PTR(ctx->cbc_buf)[1] ^= 0xfe;
      UI8_PTR(ctx->cbc_buf)[2] ^= (u8)(ctx->hdr_lim >> 24);
      UI8_PTR(ctx->cbc_buf)[3] ^= (u8)(ctx->hdr_lim >> 16);
      UI8_PTR(ctx->cbc_buf)[4] ^= (u8)(ctx->hdr_lim >>  8);
      UI8_PTR(ctx->cbc_buf)[5] ^= (u8) ctx->hdr_lim;
      ctx->cnt = 6;
    }
    else
    {
      UI8_PTR(ctx->cbc_buf)[0] ^= 0xff;
      UI8_PTR(ctx->cbc_buf)[1] ^= 0xff;
      UI8_PTR(ctx->cbc_buf)[2] ^= (u8)(ctx->hdr_lim >> 56);
      UI8_PTR(ctx->cbc_buf)[3] ^= (u8)(ctx->hdr_lim >> 48);
      UI8_PTR(ctx->cbc_buf)[4] ^= (u8)(ctx->hdr_lim >> 40);
      UI8_PTR(ctx->cbc_buf)[5] ^= (u8)(ctx->hdr_lim >> 32);
      UI8_PTR(ctx->cbc_buf)[6] ^= (u8)(ctx->hdr_lim >> 24);
      UI8_PTR(ctx->cbc_buf)[7] ^= (u8)(ctx->hdr_lim >> 16);
      UI8_PTR(ctx->cbc_buf)[8] ^= (u8)(ctx->hdr_lim >>  8);
      UI8_PTR(ctx->cbc_buf)[9] ^= (u8) ctx->hdr_lim;
      ctx->cnt = 10;
    }
#endif
    ctx->hdr_lim += ctx->cnt;
    ctx->state = AUTH_STATE;
  }
  else    /* there is no associated data  */
  {
    ctx->cnt = 0;
    ctx->state = MSG_STATE;
  }
  //printf("AES(B_0) XOR L(a):\n");
  //for (j=0;j<16;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //}
  //printf("\n");
  ctx->hdr_lim = ctx->hdr_len + ctx->cnt;
  ctx->txt_acnt = ctx->cnt;

  return RETURN_GOOD;
}


u32 ccm_auth_header(                  /* authenticate the header */
            const unsigned char hdr[],     /* the header buffer       */
            length_t hdr_len,         /* and its length in bytes */
            ccm_ctx ctx[1])                /* the mode context        */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->cnt) & BLK_ADR_MASK;


  if (!hdr_len)
  {
    return RETURN_GOOD;
  }

  if (ctx->state < 0)
  {
    return ctx->state;
  }

  if (ctx->state != AUTH_STATE)
  {
    return (ctx->state = CCM_BAD_AUTH_CALL);
  }

  if (ctx->cnt + hdr_len > ctx->hdr_lim)
  {
    return (ctx->state = CCM_AUTH_LENGTH_ERROR);
  }

  if (!((hdr - (UI8_PTR(ctx->cbc_buf) + b_pos)) & BUF_ADRMASK))
  {
    while ((cnt < hdr_len) && (b_pos & BUF_ADRMASK))
    {
      UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];
    }
    while (cnt + BUF_INC <= hdr_len && b_pos <= BLOCK_SIZE - BUF_INC)
    {
      *UNIT_PTR(UI8_PTR(ctx->cbc_buf) + b_pos) ^= *UNIT_PTR(hdr + cnt);
      cnt += BUF_INC; b_pos += BUF_INC;
    }
    //printf("a:AES(B_0) XOR B_1:\n");
    //for (j=0;j<16;j++) {
    // printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
    //}
    //printf("\n");
    while (cnt + BLOCK_SIZE <= hdr_len)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
      //printf("a: X_%d:\n",cnt/16+1);
      //for (j=0;j<16;j++) {
      //  printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
      //}
      //printf("\n");
      xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, hdr + cnt);
      cnt += BLOCK_SIZE;
    }
  } /* if */
  else
  {
    while (cnt < hdr_len && b_pos < BLOCK_SIZE)
    {
      UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];
    }
    //printf("b: AES(B_0) XOR B_1:\n");
    //for (j=0;j<16;j++) {
    // printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
    //}
    //printf("\n");

    while (cnt + BLOCK_SIZE <= hdr_len)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
      //printf("b: X_%d:\n",cnt/16+1);
      //for (j=0;j<16;j++) {
      //  printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
      //  }
      //printf("\n");
      xor_block(ctx->cbc_buf, ctx->cbc_buf, hdr + cnt);
      cnt += BLOCK_SIZE;
    }
  } /* else */

  while (cnt < hdr_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
      //printf("X_%d:\n",cnt/16+1);
      //for (j=0;j<16;j++) {
      //  printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
      //  }
      //printf("\n");
      b_pos = 0;
    }
    UI8_PTR(ctx->cbc_buf)[b_pos++] ^= hdr[cnt++];

  }

  ctx->cnt += cnt;
  if ((b_pos == BLOCK_SIZE) || (ctx->cnt == ctx->hdr_lim))
  {
    aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
  }

  if (ctx->cnt == ctx->hdr_lim)
  {
    ctx->state = MSG_STATE;
    ctx->cnt = 0;
    ctx->txt_acnt = 0;
  }
  return RETURN_GOOD;
}

u32 ccm_auth_data(                    /* authenticate the plaintext */
            const unsigned char data[],    /* the data buffer            */
            unsigned long data_len,        /* and its length in bytes    */
            ccm_ctx ctx[1])                /* the mode context           */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->txt_acnt) & BLK_ADR_MASK;


  if (ctx->state < 0)
  {
    return ctx->state;
  }

  if (ctx->state == AUTH_STATE)
  {
    return (ctx->state = CCM_AUTH_LENGTH_ERROR);
  }

  if (ctx->txt_acnt + data_len > ctx->msg_len)
  {
    return (ctx->state = CCM_MSG_LENGTH_ERROR);
  }

  if (!((data - (UI8_PTR(ctx->cbc_buf) + b_pos)) & BUF_ADRMASK))
  {
    while (cnt < data_len && (b_pos & BUF_ADRMASK))
    {
      UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
    }

    while (cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
    {
      *UNIT_PTR(UI8_PTR(ctx->cbc_buf) + b_pos) ^= *UNIT_PTR(data + cnt);
      cnt += BUF_INC; b_pos += BUF_INC;
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
      xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
      cnt += BLOCK_SIZE;
    }
  } /* if */

  else
  {
    while (cnt < data_len && b_pos < BLOCK_SIZE)
    {
      UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
    }

    while (cnt + BLOCK_SIZE <= data_len)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
      xor_block(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
      cnt += BLOCK_SIZE;
    }
  } /* else */

  while (cnt < data_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_encrypt(UI8_PTR(ctx->cbc_buf),       \
                  UI8_PTR(ctx->cbc_buf),       \
                  ctx->aes), b_pos = 0;
    }
    UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
  }

  if (b_pos == BLOCK_SIZE)
  {
    aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
  }

  //printf("AES(B_n-1) XOR B_n:\n");
  //  for (j=0;j<16;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //  }
  //  printf("\n");

  ctx->txt_acnt += cnt;
  return RETURN_GOOD;
}


u32 ccm_crypt_data(                   /* encrypt or decrypt data */
            unsigned char data[],          /* the data buffer         */
            unsigned long data_len,        /* and its length in bytes */
            ccm_ctx ctx[1])                /* the mode context        */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->cnt) & BLK_ADR_MASK;


  if (ctx->state < 0)
  {
    //dbgp_ccm("   ctx->state\n");
    return ctx->state;
  }

  if (ctx->state == AUTH_STATE)
  {
    //dbgp_ccm("   ctx->AUTHstate\n");
    return (ctx->state = CCM_AUTH_LENGTH_ERROR);
  }

  if (ctx->cnt + data_len > ctx->msg_len)
  {
    //dbgp_ccm("   ctx->msg_len\n");
    return (ctx->state = CCM_MSG_LENGTH_ERROR);
  }

  if (!data_len)
  {
    //dbgp_ccm("   ctx->data_len = 0\n");
    return RETURN_GOOD;
  }

  if (b_pos == 0)
  {
    aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
    inc_ctr(ctx->ctr_val);
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
      inc_ctr(ctx->ctr_val);
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
      inc_ctr(ctx->ctr_val);
      xor_block(data + cnt, data + cnt, ctx->enc_ctr);
      cnt += BLOCK_SIZE;
    }
  }

  while (cnt < data_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
      inc_ctr(ctx->ctr_val);
      b_pos = 0;
    }
    data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
  }

  ctx->cnt += cnt;
  return RETURN_GOOD;
}

u32 ccm_compute_tag(                  /* compute authentication tag */
            unsigned char tag[],           /* the buffer for the tag     */
            unsigned long tag_len,         /* and its length in bytes    */
            ccm_ctx ctx[1])                /* the mode context           */
{
  u32 cnt, pos;


  if (ctx->state < 0)
  {
    return ctx->state;
  }

  if ((tag_len != ctx->tag_len)  ||                    \
      ((ctx->state == INIT_STATE) && ctx->hdr_lim) ||  \
      (ctx->state == AUTH_STATE))
  {
    return (ctx->state = CCM_AUTH_LENGTH_ERROR);
  }

  if (ctx->txt_acnt != ctx->cnt && ctx->cnt > 0)
  {
    return RETURN_ERROR;
  }

  if ((ctx->cnt < ctx->msg_len) ||
      (ctx->cnt + tag_len > ctx->msg_len + ctx->tag_len))
  {
    return (ctx->state = CCM_MSG_LENGTH_ERROR);
  }

  /* if at the start of the authentication field */
  if (tag_len > 0 && ctx->cnt == ctx->msg_len)
  {
    if (ctx->cnt & BLK_ADR_MASK)
    {
        aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
    }
    set_ctr(ctx, 0);
    aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
  }


  //printf("After AES:\n");
  //for (j=0;j<16;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //}
  //printf("\n");

  //printf("CBC-MAC:\n");
  //for (j=0;j<tag_len;j++) {
  //   printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  //}
  //printf("\n");

  cnt = 0;
  pos = (u32)(ctx->cnt - ctx->msg_len);
  while (cnt < tag_len)
  {
    tag[cnt++] = UI8_PTR(ctx->cbc_buf)[pos] ^ UI8_PTR(ctx->enc_ctr)[pos];
    ++pos;
  }

  return (ctx->cnt == ctx->txt_acnt ? RETURN_GOOD : RETURN_WARN);
}


u32 ccm_end(                          /* clean up and end operation */
            ccm_ctx ctx[1])                /* the mode context           */
{
  memset(ctx, 0, sizeof(ccm_ctx));
  return RETURN_GOOD;
}


u32 ccm_encrypt(                      /* authenticate & encrypt data */
            unsigned char data[],          /* the data buffer             */
            unsigned long data_len,        /* and its length in bytes     */
            ccm_ctx ctx[1])                /* the mode context            */
{
  ccm_auth_data(data, data_len, ctx);
  ccm_crypt_data(data, data_len, ctx);
  return RETURN_GOOD;
}


u32 ccm_decrypt(                      /* decrypt & authenticate data */
            unsigned char data[],          /* the data buffer             */
            unsigned long data_len,        /* and its length in bytes     */
            ccm_ctx ctx[1])                /* the mode context            */
{
  ccm_crypt_data(data, data_len, ctx);
  ccm_auth_data(data, data_len, ctx);
  return RETURN_GOOD;
}


u32 ccm_encrypt_message(              /* encrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            const unsigned char hdr[],     /* the header buffer         */
            length_t hdr_len,              /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char tag[],           /* the buffer for the tag    */
            unsigned long tag_len,         /* and its length in bytes   */
            ccm_ctx ctx[1])                /* the mode context          */
{

#if 0  /* orig code */

  ccm_init_message(iv, iv_len, hdr_len, msg_len, tag_len, ctx);
  ccm_auth_header(hdr, hdr_len, ctx);
  ccm_encrypt(msg, msg_len, ctx);
  return ccm_compute_tag(tag, tag_len, ctx) ? RETURN_ERROR : RETURN_GOOD;

#else   /* more error checking */

  u32 retVal = ccm_init_message(iv, iv_len, hdr_len,
                                     msg_len, tag_len, ctx);

  if (retVal == RETURN_GOOD)
  {
    retVal = ccm_auth_header(hdr, hdr_len, ctx);
    if (retVal == RETURN_GOOD)
    {
      retVal = ccm_encrypt(msg, msg_len, ctx);
      if (retVal == RETURN_GOOD)
      {
        retVal = ccm_compute_tag(tag, tag_len, ctx);
      }
    }
  }

  return retVal;

#endif
}


u32 ccm_decrypt_message(              /* decrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            const unsigned char hdr[],     /* the header buffer         */
            length_t hdr_len,              /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            length_t msg_len,              /* and its length in bytes   */
            const unsigned char tag[],     /* the buffer for the tag    */
            unsigned long tag_len,         /* and its length in bytes   */
            unsigned char ctag[],           /* the buffer for the tag    */
            ccm_ctx ctx[1])                /* the mode context          */
{
  u8 local_tag[BLOCK_SIZE];

#if 0  /* orig code */

  u32 rr;

  ccm_init_message(iv, iv_len, hdr_len, msg_len, tag_len, ctx);
  ccm_auth_header(hdr, hdr_len, ctx);
  ccm_decrypt(msg, msg_len, ctx);
  rr = ccm_compute_tag(local_tag, tag_len, ctx);
  return (rr != RETURN_GOOD || memcmp(tag, local_tag, tag_len)) ?  \
          RETURN_ERROR : RETURN_GOOD;

#else   /* more error checking */

  u32 retVal = ccm_init_message(iv, iv_len, hdr_len,
                                     msg_len, tag_len, ctx);
  if (retVal == RETURN_GOOD)
  {
    retVal = ccm_auth_header(hdr, hdr_len, ctx);
    if (retVal == RETURN_GOOD)
    {
      retVal = ccm_decrypt(msg, msg_len, ctx);
      if (retVal == RETURN_GOOD)
      {
        retVal = ccm_compute_tag(local_tag, tag_len, ctx);
        if ((retVal != RETURN_GOOD)
             || memcmp(tag, local_tag, tag_len)
            )
        {
          memcpy (ctag, local_tag, tag_len);
          retVal = RETURN_ERROR;
        }
      }
    }
  }

  return retVal;

#endif
}


#if defined(__cplusplus)
}
#endif
