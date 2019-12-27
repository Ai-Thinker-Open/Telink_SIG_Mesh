/********************************************************************************************************
 * @file     cbc.c 
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
#include "inc/cbc.h"
#include "inc/mode_hdr.h"
#include "inc/debug.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define _CBC_STEALING 1

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
extern int j;


/*****************************************************************************/

u32 cbc_init_and_enkey(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            cbc_ctx ctx[1])                /* the mode context            */
{
  ctx->state = RETURN_GOOD;
  if ((key_len != 16) && (key_len != 24) && (key_len != 32))
  {
    ctx->state = CBC_BAD_KEY;
  }
  aes_encrypt_key(key, (int)key_len, ctx->aes);
  return RETURN_GOOD;
}


u32 cbc_init_and_dekey(                 /* initialise mode and set key */
            const unsigned char key[],     /* the key value               */
            unsigned long key_len,         /* and its length in bytes     */
            cbc_ctx ctx[1])                /* the mode context            */
{
  ctx->state = RETURN_GOOD;
  if ((key_len != 16) && (key_len != 24) && (key_len != 32))
  {
    ctx->state = CBC_BAD_KEY;
  }
  aes_decrypt_key(key, (int)key_len, (aes_decrypt_ctx*)&ctx->aes);
  return RETURN_GOOD;
}

/*****************************************************************************/

u32 cbc_init_message(                 /* initialise for a new message */
            length_t hdr_len,              /* the associated data length   */
            const unsigned char iv[],      /* the initialisation vector    */
            length_t iv_len,               /* message data length          */
            unsigned long tag_len,         /* authentication field length  */
            cbc_ctx ctx[1])                /* the mode context             */
{
  u32 cnt;


  ctx->state = RETURN_GOOD;

  ctx->hdr_len = hdr_len;
  ctx->tag_len = tag_len;
  ctx->cnt = 0;
  ctx->txt_acnt = 0;
  ctx->hdr_lim = hdr_len;

  /* move the iv into the block */
  for(cnt = 0;
      cnt < (u32)BLOCK_SIZE;
      ++cnt)
  {
    UI8_PTR(ctx->cbc_buf)[cnt] = iv[cnt];
  }

  printf("B_0:\n");
  for (j=0;j<16;j++) {
     printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  }
  printf("\n");

  /* encrypt the cbc block    */
  aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

  printf("AES(B_0):\n");
  for (j=0;j<16;j++) {
     printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  }
  printf("\n");

  ctx->cnt = 0;
  ctx->state = MSG_STATE;

  ctx->hdr_lim = ctx->hdr_len + ctx->cnt;
  ctx->txt_acnt = ctx->cnt;

  return RETURN_GOOD;
}

/*****************************************************************************/

u32 cbc_encrypt_data(                    /* authenticate the plaintext        */
            length_t iv_len,               /* the initialisation vector length  */
            const unsigned char iv[],      /* the initialisation vector         */
            const unsigned char data[],    /* the data buffer                   */
            unsigned long data_len,        /* and its length in bytes           */
            cbc_ctx ctx[1])                /* the mode context                  */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->txt_acnt) & BLK_ADR_MASK;
  u32  blockNum = 0;
  u32 dataOffset = 0;
  u32  i = 0;
  cbc_buf_t lastBlock;


  if (ctx->state < 0)
  {
    return ctx->state;
  }

  if (ctx->state == AUTH_STATE)
  {
    return (ctx->state = CBC_AUTH_LENGTH_ERROR);
  }

  if (ctx->txt_acnt + data_len > ctx->msg_len)
  {
    return (ctx->state = CBC_MSG_LENGTH_ERROR);
  }


  /* move the iv into the block */
  for(cnt = 0;
      cnt < (u32)BLOCK_SIZE;
      ++cnt)
  {
    UI8_PTR(ctx->cbc_buf)[cnt] = iv[cnt];
  }



  cnt = 0;


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

      printf("C_%d:\n", blockNum++);
      for (j=0;j<16;j++) {
          printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
      }
      printf("\n");
      memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
      xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
      cnt += BLOCK_SIZE;
      dataOffset += BLOCK_SIZE;
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

#ifdef _CBC_STEALING
  /* CBC Stealing */
  if(cnt < data_len) {



        /* The n-1 block  */
        aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
        memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);


      //memset(UI8_PTR(lastBlock), 0, BLOCK_SIZE);
        memcpy(UI8_PTR(lastBlock), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);

        /* encoder last block */
        for (i = 0; i < data_len - cnt; i++) {
              UI8_PTR(lastBlock)[i] = UI8_PTR(data + cnt)[i];
        }
        //xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, UI8_PTR(lastBlock));

        /* The n block  */
        aes_encrypt(UI8_PTR(lastBlock), UI8_PTR(ctx->cbc_buf), ctx->aes);

        /* swap last two block */
      memcpy(UI8_PTR(lastBlock), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
      memcpy(UI8_PTR(ctx->cbc_buf), data+dataOffset, BLOCK_SIZE);
      memcpy((u8*)(data+dataOffset), UI8_PTR(lastBlock), BLOCK_SIZE);
      dataOffset += BLOCK_SIZE;
      for (i = 0; i < data_len - cnt; i++) {
              UI8_PTR(data+dataOffset)[i] = UI8_PTR(ctx->cbc_buf)[i];
        }

        b_pos = 0;
  }
#else
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
#endif

  if (b_pos == BLOCK_SIZE)
  {
    aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
    memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
    printf("C_%d:\n", blockNum);
    for (j=0;j<16;j++) {
        printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
    }
    printf("\n");
  }



  ctx->txt_acnt += cnt;
  return RETURN_GOOD;
}



u32 cbc_decrypt_data(                    /* authenticate the plaintext        */
            length_t iv_len,               /* the initialisation vector length  */
            const unsigned char iv[],      /* the initialisation vector         */
            const unsigned char data[],    /* the data buffer                   */
            unsigned long data_len,        /* and its length in bytes           */
            cbc_ctx ctx[1])                /* the mode context                  */
{
  u32  cnt = 0;
  u32  b_pos = ((u32)ctx->txt_acnt) & BLK_ADR_MASK;
  u32  blockNum = 0;
  u32 dataOffset = 0;
  u32  i = 0;
  cbc_buf_t lastBlock;
  cbc_buf_t prevBlock;
  cbc_buf_t tempBlock;
  cbc_buf_t lastThirdBlock;
  volatile u32 temp;


  if (ctx->state < 0)
  {
    return ctx->state;
  }

  if (ctx->state == AUTH_STATE)
  {
    return (ctx->state = CBC_AUTH_LENGTH_ERROR);
  }

  if (ctx->txt_acnt + data_len > ctx->msg_len)
  {
    return (ctx->state = CBC_MSG_LENGTH_ERROR);
  }


  /* move the c1 into the block */
  for(cnt = 0;
      cnt < (u32)BLOCK_SIZE;
      ++cnt)
  {
    UI8_PTR(ctx->cbc_buf)[cnt] = data[cnt];
    UI8_PTR(prevBlock)[cnt] = data[cnt];
  }

  aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), (aes_decrypt_ctx*)&ctx->aes);



  cnt = 0;

    //temp = (data - (UI8_PTR(ctx->cbc_buf) + b_pos))  & BUF_ADRMASK;
  if (1)
  {
    while (cnt < data_len && (b_pos & BUF_ADRMASK))
    {
      UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
    }

    while (cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
    {
      *UNIT_PTR(UI8_PTR(ctx->cbc_buf) + b_pos) ^= *UNIT_PTR(iv + cnt);
      cnt += BUF_INC; b_pos += BUF_INC;
    }


    while (cnt + BLOCK_SIZE <= data_len)
    {
      //aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);

      printf("C_%d:\n", blockNum++);
      for (j=0;j<16;j++) {
          printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
      }
      printf("\n");
      /* save result */
      memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);

      /* get next block */
      dataOffset += BLOCK_SIZE;
      memcpy(UI8_PTR(ctx->cbc_buf), data+dataOffset, BLOCK_SIZE);

      if (cnt + BLOCK_SIZE * 3 > data_len && cnt + BLOCK_SIZE * 3 < data_len + BLOCK_SIZE) {
          memcpy(UI8_PTR(lastThirdBlock), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
      }
      memcpy(UI8_PTR(tempBlock), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
      aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), (aes_decrypt_ctx*)&ctx->aes);

      /* used in stealing mode */
      memcpy(UI8_PTR(lastBlock), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);

      /* Pj = CIPH(Cj) xor Cj-1 */
      xor_block_aligned(ctx->cbc_buf, ctx->cbc_buf, prevBlock);
      memcpy(UI8_PTR(prevBlock), UI8_PTR(tempBlock), BLOCK_SIZE);
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
      aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), (aes_decrypt_ctx*)&ctx->aes);
      xor_block(ctx->cbc_buf, ctx->cbc_buf, data + cnt);
      cnt += BLOCK_SIZE;
    }
  } /* else */

#ifdef _CBC_STEALING
  /* CBC Stealing */
  if(cnt < data_len) {
        /* The n-1 block  */

        dataOffset += BLOCK_SIZE;
        memset(UI8_PTR(ctx->cbc_buf), 0, BLOCK_SIZE);

      for (i = 0; i < data_len - cnt; i++) {
              UI8_PTR(ctx->cbc_buf)[i] = UI8_PTR(data + cnt)[i];
        }
        for (; i < BLOCK_SIZE; i++) {
            UI8_PTR(ctx->cbc_buf)[i] = UI8_PTR(lastBlock)[i];
        }

        aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), (aes_decrypt_ctx*)&ctx->aes);
        xor_block(ctx->cbc_buf, lastThirdBlock, ctx->cbc_buf);

      /* save result */
        dataOffset -= BLOCK_SIZE;
        memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
        dataOffset += BLOCK_SIZE;
        memcpy((u8*)(data+dataOffset), UI8_PTR(lastBlock), data_len - cnt);

        b_pos = 0;
  }
#else
  while (cnt < data_len)
  {
    if (b_pos == BLOCK_SIZE)
    {
      aes_decrypt(UI8_PTR(ctx->cbc_buf),       \
                  UI8_PTR(ctx->cbc_buf),       \
                  ctx->aes), b_pos = 0;
    }
    UI8_PTR(ctx->cbc_buf)[b_pos++] ^= data[cnt++];
  }
#endif

  if (b_pos == BLOCK_SIZE)
  {
    //aes_decrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), ctx->aes);
    memcpy((u8*)(data+dataOffset), UI8_PTR(ctx->cbc_buf), BLOCK_SIZE);
    printf("C_%d:\n", blockNum);
    for (j=0;j<16;j++) {
        printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
    }
    printf("\n");
  }



  ctx->txt_acnt += cnt;
  return RETURN_GOOD;
}

/*****************************************************************************/

u32 cbc_compute_tag(                  /* compute authentication tag */
            unsigned char tag[],           /* the buffer for the tag     */
            unsigned long tag_len,         /* and its length in bytes    */
            cbc_ctx ctx[1])                /* the mode context           */
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
    return (ctx->state = CBC_AUTH_LENGTH_ERROR);
  }

  if (ctx->txt_acnt != ctx->cnt && ctx->cnt > 0)
  {
    return RETURN_ERROR;
  }

  if ((ctx->cnt < ctx->msg_len) ||
      (ctx->cnt + tag_len > ctx->msg_len + ctx->tag_len))
  {
    return (ctx->state = CBC_MSG_LENGTH_ERROR);
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

  printf("After AES:\n");
  for (j=0;j<16;j++) {
     printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
  }
  printf("\n");

  printf("CBC-MAC:\n");
  for (j=0;j<tag_len;j++) {
     printf("%x ", UI8_PTR(ctx->cbc_buf)[j]);
     tag[j] = UI8_PTR(ctx->cbc_buf)[j];
  }
  printf("\n");

  /*
  cnt = 0;
  pos = (u32)(ctx->cnt - ctx->msg_len);
  while (cnt < tag_len)
  {
    tag[cnt++] = UI8_PTR(ctx->cbc_buf)[pos] ^ UI8_PTR(ctx->enc_ctr)[pos];
    ++pos;
  }
  */

  return (ctx->cnt == ctx->txt_acnt ? RETURN_GOOD : RETURN_WARN);
}



u32 cbc_decrypt (                    /* authenticate the plaintext        */
            length_t iv_len,               /* the initialisation vector length  */
            const unsigned char iv[],      /* the initialisation vector         */
            const unsigned char data[],    /* the data buffer                   */
            unsigned long data_len,        /* and its length in bytes           */
            cbc_ctx ctx[1])                /* the mode context                  */
{
    u32 cnt = 0;

    memset(UI8_PTR(ctx->cbc_buf), 0, BLOCK_SIZE);

    memcpy(UI8_PTR(ctx->cbc_buf), data+cnt, BLOCK_SIZE);

    aes_encrypt(UI8_PTR(ctx->cbc_buf), UI8_PTR(ctx->cbc_buf), (aes_encrypt_ctx*)ctx);

    xor_block(ctx->cbc_buf, ctx->cbc_buf, iv);
    return 0;
}



/*****************************************************************************/

u32 cbc_encrypt_message(              /* encrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char tag[],           /* the buffer for the tag    */
            unsigned long tag_len,         /* and its length in bytes   */
            cbc_ctx ctx[1])                /* the mode context          */
{

    u32 retVal;

    do {

        //retVal = cbc_init_message(0, iv, iv_len, tag_len, ctx);
        //if (RETURN_GOOD != retVal) {
        //    break;
        //}

        ctx->cnt = 0;
        ctx->txt_acnt = 0;
        ctx->msg_len = msg_len;

        retVal = cbc_encrypt_data(iv_len, iv, msg, msg_len, ctx);
        if (RETURN_GOOD != retVal) {
            break;
        }

        ctx->cnt += msg_len;

        //retVal = cbc_compute_tag(tag, tag_len, ctx);
        //if (RETURN_GOOD != retVal) {
        //    break;
        //}
    }while(0);

    return retVal;

}



/*****************************************************************************/

u32 cbc_decrypt_message(              /* encrypt an entire message */
            const unsigned char iv[],      /* the initialisation vector */
            unsigned long iv_len,          /* and its length in bytes   */
            unsigned char msg[],           /* the message buffer        */
            length_t msg_len,              /* and its length in bytes   */
            unsigned char tag[],           /* the buffer for the tag    */
            unsigned long tag_len,         /* and its length in bytes   */
            unsigned char ctag[],          /* the buffer for the tag    */
            cbc_ctx ctx[1])                /* the mode context          */
{

    u32 retVal;

    u8 local_tag[BLOCK_SIZE];

    do {

        //retVal = cbc_init_message(0, msg, msg_len, tag_len, ctx);
        //if (RETURN_GOOD != retVal) {
        //    break;
        //}

        ctx->cnt = 0;
        ctx->txt_acnt = 0;
        ctx->msg_len = msg_len;

        retVal = cbc_decrypt_data(iv_len, iv, msg, msg_len, ctx);
        //retVal = cbc_decrypt(iv_len, iv, msg, msg_len, ctx);
        if (RETURN_GOOD != retVal) {
            break;
        }

        ctx->cnt += msg_len;

        //retVal = cbc_compute_tag(local_tag, tag_len, ctx);
        //if (RETURN_GOOD != retVal || memcmp(tag, local_tag, tag_len)) {
        //    memcpy (ctag, local_tag, tag_len);
        //    retVal = RETURN_ERROR;
        //    break;
        //}

    }while(0);

    return retVal;

}





#if defined(__cplusplus)
}
#endif
