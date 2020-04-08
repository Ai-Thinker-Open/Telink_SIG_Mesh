/********************************************************************************************************
 * @file     ctrchk.c 
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>

#include "ctr.h"
#include "debug.h"
#include "ctrchk.h"


void ctrCheck(ctr_info_t  *pCTR)
{
    ctr_ctx     ctx[1];
    uint8_t     errVal = NO_CTR_ERROR;
    ret_type    retVal = RETURN_GOOD;


    dbgp_app("-------------------Enter CTR Check------------------\n\n");

    dbgp_app("Key Length: %d\n", pCTR->keyLen);

    /* Check parameter */
    if (pCTR->keyLen) {
        outputHexStr("Key:\n\r", pCTR->key, pCTR->keyLen);
    } else {
        printf("\n   no key to use ...\n");
    }

    if (pCTR->nonceLen) {
        outputHexStr("Nonce:\n\r", pCTR->nonce,pCTR->nonceLen);
    } else {
        printf("\n   no nonce to use ...\n");
    }

    if (pCTR->counterMod >= 0 && pCTR->counterMod <= 15)
        dbgp_app("\nCounter Mod: %d\n", 128);
    else if (pCTR->counterMod < 128)
        dbgp_app("\nCounter Mod: %d\n", pCTR->counterMod);
    else
    {
        dbgp_app("\nCounter Mod: error\n");
        exit(-1);
    }


    /* First encrypt */
    if (pCTR->fEncryption) {
        dbgp_app("Message Length: %d\n", pCTR->mstrLen);
        if (pCTR->mstrLen) {
            outputHexStr("Plain Text:\n\r", pCTR->mStr, pCTR->mstrLen);
        } else {
            printf("   no data to encrypt (only header authenticated)....\n");
        }

        retVal = ctr_init_and_enkey(pCTR->key, pCTR->keyLen, ctx);
        if (retVal != RETURN_GOOD) {
            errVal = CTRKEY_ERROR;
            //do_exit(errVal);
        }

        printf("\nEncrypting...\n\r");
        retVal = ctr_encrypt_message(pCTR->nonce, pCTR->nonceLen, pCTR->counterMod, pCTR->initCounter, pCTR->mstrLen, pCTR->mStr, ctx);
        if (retVal != RETURN_GOOD) {
            errVal = CTRENCR_ERROR;
            //do_exit(errVal);
        }

        if (pCTR->mstrLen) {
            outputHexStr("Cipher Text:\n\r", pCTR->mStr, pCTR->mstrLen);
        }
    }

    /* Then decrypt */
    else {
        printf("Decrypting...\n\r");
        dbgp_app("Message Length: %d\n", pCTR->mstrLen);
        if (pCTR->mstrLen) {
            outputHexStr("Cipher Text:\n\r", pCTR->mStr, pCTR->mstrLen);
        } else {
            printf("   no data to encrypt (only header authenticated)....\n");
        }
        retVal = ctr_init_and_enkey(pCTR->key, pCTR->keyLen, ctx);
        retVal = ctr_decrypt_message(pCTR->nonce, pCTR->nonceLen, pCTR->counterMod, pCTR->initCounter, pCTR->mstrLen, pCTR->mStr, ctx);
        if (pCTR->mstrLen) {
            outputHexStr("Plain Text:\n\r", pCTR->mStr, pCTR->mstrLen);
        } else {
            printf("   no data to decrypt (only header authenticated)....\n");
        }

    }
    


}

