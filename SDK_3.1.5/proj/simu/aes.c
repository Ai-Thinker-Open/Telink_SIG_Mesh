/********************************************************************************************************
 * @file     aes.c 
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
#if WIN32
#include <windows.h>
#endif

#include "../common/types.h"
#include "aes_api.h"
#include "aes.h"
#include "aes/inc/ecb.h"
#include "aes/inc/cbc.h"
#include "aes/inc/mmo.h"
#include "aes/inc/ccm.h"
#include "aes/inc/ctr.h"
#include "aes/inc/error.h"

/**************************** Private Macro Definitions ***********************/



/**************************** Private type Definitions ************************/

/* None */

/**************************** Private Function Prototypes *********************/

u32 aes_verifyKeyLen(u32 len);
u32 ecb_handler(u8 * src, u8 * dst, ecb_cfg_t* cfg);
u32 cbc_handler(u8 * src, u8 * dst, cbc_cfg_t* cfg);
u32 mmo_handler(u8 * src, u8 * dst, mmo_cfg_t* cfg);
u32 ccm_handler(u8 * src, u8 * dst, ccm_cfg_t* cfg);
u32 ctr_handler(u8 * src, u8 * dst, ctr_cfg_t* cfg);

/**************************** Public Functions Definitions ********************/

u32 soft_aes(u8 aesMode, u8 * src, u8 * dst, void * parameter)
{
    u32 rst = SUCCESS;

    switch (aesMode) {
        case AES_MODE_CCM:
            rst = ccm_handler(src, dst, (ccm_cfg_t*)parameter);
            break;

        case AES_MODE_CTR:
            rst = ctr_handler(src, dst, (ctr_cfg_t*)parameter);
            break;

        case AES_MODE_CBC:
            rst = cbc_handler(src, dst, (cbc_cfg_t*)parameter);
            break;

        case AES_MODE_ECB:
            rst = ecb_handler(src, dst, (ecb_cfg_t*)parameter);
            break;

        case AES_MODE_MMO:
            rst = mmo_handler(src, dst, (mmo_cfg_t*)parameter);
            break;

        default: break;
    }

    return rst;

}


/**************************** Private Functions Definitions ********************/

u32 ecb_handler(u8 * src, u8 * dst, ecb_cfg_t* cfg)
{
    ecb_ctx ctx[1];
    u32 rst;

    /* verify key len */
    rst = aes_verifyKeyLen(cfg->keyLen);
    if (SUCCESS != rst) {
        return rst;
    }

    memcpy(dst, src, cfg->srcLen);

    /* encryption */
    if (AES_STS_ENCRYPTION == cfg->op) {
        rst = ecb_init_and_enkey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ecb_encrypt_message(cfg->srcLen, dst, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_ENCRYPT_FAIL;
        }
    }
     /* decryption */
    else if (AES_STS_DECRYPTION == cfg->op) {
        rst = ecb_init_and_dekey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ecb_decrypt_message(cfg->srcLen, dst, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_DECRYPT_FAIL;
        }
    }

    return SUCCESS;

}

/*****************************************************************************/

u32 cbc_handler(u8 * src, u8 * dst, cbc_cfg_t* cfg)
{
    cbc_ctx ctx[1];
    u8 micE[16];
    u32 rst = SUCCESS;

    /* verify key len */
    rst = aes_verifyKeyLen(cfg->keyLen);
    if (SUCCESS != rst) {
        return rst;
    }

    if (cfg->ivLen != 16) {
        return ERR_AES_INVALID_CBC_IV_LEN;
    }

    memcpy(dst, src, cfg->srcLen);

    /* encryption */
    if (AES_STS_ENCRYPTION == cfg->op) {
        rst = cbc_init_and_enkey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = cbc_encrypt_message(cfg->iv, cfg->ivLen, dst, cfg->srcLen, cfg->micE, cfg->micLen>2?cfg->micLen:2, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_ENCRYPT_FAIL;
        }
    }
     /* decryption */
    else if (AES_STS_DECRYPTION == cfg->op) {
        rst = cbc_init_and_dekey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = cbc_decrypt_message(cfg->iv, cfg->ivLen, dst, cfg->srcLen, cfg->micE, cfg->micLen>2?cfg->micLen:2, cfg->micD, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_DECRYPT_FAIL;
        }
    }

    return SUCCESS;
}

/*****************************************************************************/

u32 mmo_handler(u8 * src, u8 * dst, mmo_cfg_t* cfg)
{
    mmo_ctx ctx[1];
    u32 rst = SUCCESS;

    /* verify key len */
    rst = aes_verifyKeyLen(cfg->keyLen);
    if (SUCCESS != rst) {
        return rst;
    }

    /* only encryption mode */
    rst = mmo_init_and_key(cfg->key, cfg->keyLen, ctx);
    if (rst != RETURN_GOOD) {
       return ERR_AES_KEY_INIT_FAIL;
    }

    rst = mmo_encrypt_message(cfg->srcLen, src, ctx, dst);
    if (rst != RETURN_GOOD) {
        return ERR_AES_ENCRYPT_FAIL;
    }

    return SUCCESS;
}

/*****************************************************************************/

u32 ccm_handler(u8 * src, u8 * dst, ccm_cfg_t* cfg)
{
    ccm_ctx ctx[1];
    u32 rst = SUCCESS;

    /* verify key len */
    rst = aes_verifyKeyLen(cfg->keyLen);
    if (SUCCESS != rst) {
        return rst;
    }

    memcpy(dst, src, cfg->srcLen);

     /* encryption */
    if (AES_STS_ENCRYPTION == cfg->op) {
        rst = ccm_init_and_key(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ccm_encrypt_message(cfg->iv, cfg->ivLen, cfg->aStr, cfg->aStrLen,
                                  dst, cfg->srcLen, cfg->micE, cfg->micLen>2?cfg->micLen:2, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_ENCRYPT_FAIL;
        }
    }
     /* decryption */
    else if (AES_STS_DECRYPTION == cfg->op) {
        rst = ccm_init_and_key(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ccm_decrypt_message(cfg->iv, cfg->ivLen, cfg->aStr, cfg->aStrLen, 
                                  dst, cfg->srcLen, cfg->micE, cfg->micLen>2?cfg->micLen:2, cfg->micD, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_DECRYPT_FAIL;
        }
    }

    
    return SUCCESS;
}

/*****************************************************************************/

u32 ctr_handler(u8 * src, u8 * dst, ctr_cfg_t* cfg)
{
    ctr_ctx ctx[1];
    u32 rst = SUCCESS;

    /* verify key len */
    rst = aes_verifyKeyLen(cfg->keyLen);
    if (SUCCESS != rst) {
        return rst;
    }

    memcpy(dst, src, cfg->srcLen);

     /* encryption */
    if (AES_STS_ENCRYPTION == cfg->op) {
        rst = ctr_init_and_enkey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ctr_encrypt_message(cfg->iv, cfg->ivLen, cfg->counterMod, cfg->initCount,
                                  cfg->srcLen, dst, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_ENCRYPT_FAIL;
        }
    }
     /* decryption */
    else if (AES_STS_DECRYPTION == cfg->op) {
        rst = ctr_init_and_enkey(cfg->key, cfg->keyLen, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_KEY_INIT_FAIL;
        }

        rst = ctr_decrypt_message(cfg->iv, cfg->ivLen, cfg->counterMod, cfg->initCount,
                                  cfg->srcLen, dst, ctx);
        if (rst != RETURN_GOOD) {
            return ERR_AES_DECRYPT_FAIL;
        }
    }
}

/*****************************************************************************/

u32 aes_verifyKeyLen(u32 len)
{
    if (len == 16 || len == 32 || len == 24) {
        return SUCCESS;
    }
    else {
        return ERR_AES_INVALID_KEY_LEN;
    }
}





u8 aes_ccm_encryption(u8 *key, u8 *iv, u8 *aStr, u8 *mic, u8 mStrLen, u8 *mStr, u8 *result)
{
    u8 rst;
    ccm_cfg_t ccmCfg;
    ccmCfg.op = AES_STS_ENCRYPTION;
    ccmCfg.keyLen = 16;
    ccmCfg.key    = key;
    ccmCfg.iv     = iv;
    ccmCfg.ivLen  = 13;
    ccmCfg.srcLen = mStrLen;
    ccmCfg.aStrLen = 1;
    ccmCfg.aStr = aStr;
    ccmCfg.micLen = 4;
    ccmCfg.micE   = mic;
    ccmCfg.micD   = mic;
    rst = soft_aes(AES_MODE_CCM, mStr, result, (void*)&ccmCfg);
    return rst;
}

u8 aes_ccm_decryption(u8 *key, u8 *iv, u8 *aStr, u8 *mic, u8 mStrLen, u8 *mStr, u8 *result)
{
    u8 rst;
    ccm_cfg_t ccmCfg;
    ccmCfg.op = AES_STS_DECRYPTION;
    ccmCfg.keyLen = 16;
    ccmCfg.key    = key;
    ccmCfg.iv     = iv;
    ccmCfg.ivLen  = 13;
    ccmCfg.srcLen = mStrLen;
    ccmCfg.aStrLen = 1;
    ccmCfg.aStr = aStr;
    ccmCfg.micLen = 4;
    ccmCfg.micE   = mic;
    ccmCfg.micD   = mic;
    rst = soft_aes(AES_MODE_CCM, mStr, result, (void*)&ccmCfg);
    return rst;
}

u8 aes_ecb_encryption(u8 *key, u8 mStrLen, u8 *mStr, u8 *result)
{
    u8 rst;
    ecb_cfg_t ecbCfg;
    ecbCfg.op = AES_STS_ENCRYPTION;
    ecbCfg.keyLen = 16;
    ecbCfg.key    = key;
    ecbCfg.srcLen = 16;
    rst = soft_aes(AES_MODE_ECB, mStr, result, (void*)&ecbCfg);
    return rst;
}