/********************************************************************************************************
 * @file     mmochk.c 
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

/* ************************************************************
 * This simple test application encrypts a message and then
 * decrypts it right back, and prints the calculated MIC.
 * The Key, Nonce, Header etc. used are whatever is assigned
 * to the variables "Key", "Nnc", "Hdr" etc. via conditional
 * compilation.
 *
 * Usage: getmic
 *
 * ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>

#include "../../common/types.h"
#include "inc/ecb.h"
#include "inc/mmo.h"
#include "inc/debug.h"
#include "inc/mmochk.h"


void mmoCheck(mmo_info_t  *pMMO)
{
    mmo_ctx     ctx[1];
    uint8_t     errVal = NO_MMO_ERROR;
    ret_type    retVal = RETURN_GOOD;


    dbgp_app("-------------------Enter MMO Check------------------\n\n");

    dbgp_app("Key Length: %d\n", pMMO->keyLen);

    if (pMMO->keyLen) {
        outputHexStr("Key:\n\r", pMMO->key, pMMO->keyLen);
    } else {
        printf("\n   no key to use ...\n");
    }

    /* First encrypt */

    dbgp_app("Message Length: %d\n", pMMO->mstrLen);
    if (pMMO->mstrLen) {
        outputHexStr("Plain Text:\n\r", pMMO->mStr, pMMO->mstrLen);
    } else {
        printf("   no data to encrypt (only header authenticated)....\n");
    }

    retVal = mmo_init_and_key(pMMO->key, pMMO->keyLen, ctx);
    if (retVal != RETURN_GOOD) {
        errVal = MMOKEY_ERROR;
        return;
    }

    printf("\nEncrypting...\n\r");
    retVal = mmo_encrypt_message(pMMO->mstrLen, pMMO->mStr, ctx, pMMO->result);
    if (retVal != RETURN_GOOD) {
        errVal = MMOENCR_ERROR;
        //do_exit(errVal);
    }


    printf("Hit any key for the next message...\n\r");
    //getchar();


}
