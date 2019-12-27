/********************************************************************************************************
 * @file     cbcchk.c 
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

#include "inc/cbc.h"
#include "inc/debug.h"
#include "inc/cbcchk.h"


void cbcCheck(cbc_info_t *pCBC)
{
    cbc_ctx     ctx[1];
    uint8_t     errVal = NO_CBC_ERROR;
    ret_type    retVal = RETURN_GOOD;


    dbgp_app("-------------------Enter CBC Check------------------\n\n");

    dbgp_app("Key Length: %d\n", pCBC->keyLen);
    
    if (pCBC->keyLen) {
        outputHexStr("Key:\n\r", pCBC->key, pCBC->keyLen);
    } else {
        printf("\n   no key to use ...\n");
        getchar();
        return;
    }

    if (pCBC->nonceLen) {
        outputHexStr("IV:\n\r", pCBC->nonce,pCBC->nonceLen);
    } else {
        printf("\n   no iv to use ...\n");
        getchar();
        return;
    }

    if (pCBC->nonceLen != 16) {
        printf("\n   the iv length is wrong ...\n");
        getchar();
        return;
    }


    /* First encrypt */
    if (pCBC->fEncryption) {
        dbgp_app("Message Length: %d\n", pCBC->mstrLen);
        if (pCBC->mstrLen) {
            outputHexStr("Plain Text:\n\r", pCBC->mStr, pCBC->mstrLen);
        } else {
            printf("   no data to encrypt (only header authenticated)....\n");
            getchar();
            return;
        }

        retVal = cbc_init_and_enkey(pCBC->key, pCBC->keyLen, ctx);
        if (retVal != RETURN_GOOD) {
            errVal = CBCKEY_ERROR;
            //do_exit(errVal);
        }

        printf("Encrypting...\n\r");
        retVal = cbc_encrypt_message(pCBC->nonce, pCBC->nonceLen, pCBC->mStr, pCBC->mstrLen, pCBC->micE, pCBC->micLen>2?pCBC->micLen:2, ctx);
        if (retVal != RETURN_GOOD) {
            errVal = CBCENCR_ERROR;
            do_exit(errVal);
        }

        if (pCBC->mstrLen) {
            outputHexStr("Cipher Text:\n\r", pCBC->mStr, pCBC->mstrLen);
        }
    }


    /* Then decrypt */
    else {
        printf("Decrypting...\n\r");
        dbgp_app("Message Length: %d\n", pCBC->mstrLen);
        if (pCBC->mstrLen) {
            outputHexStr("Cipher Text:\n\r", pCBC->mStr, pCBC->mstrLen);
        } else {
            printf("   no data to decrypt (only header authenticated)....\n");
            getchar();
            return;
        }
        
        retVal = cbc_init_and_dekey(pCBC->key, pCBC->keyLen, ctx);
        if (retVal != RETURN_GOOD) {
            errVal = CBCKEY_ERROR;
            //do_exit(errVal);
        }
        retVal = cbc_decrypt_message(pCBC->nonce, pCBC->nonceLen, pCBC->mStr, pCBC->mstrLen, pCBC->micE, pCBC->micLen>2?pCBC->micLen:2, pCBC->micD, ctx);
        if (pCBC->mstrLen) {
            outputHexStr("Plain Text:\n\r", pCBC->mStr, pCBC->mstrLen);
        } else {
            printf("   no data to decrypt (only header authenticated)....\n");
        }
        if (RETURN_GOOD == retVal) {
            printf ("MIC match\n\r");
        }
        else {
            outputHexStr("MIC on decrypt side:\n\r", pCBC->micD, pCBC->micLen>2?pCBC->micLen:0);
        }
        
    }


}

