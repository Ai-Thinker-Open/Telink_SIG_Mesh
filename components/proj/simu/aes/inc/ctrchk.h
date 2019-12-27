/********************************************************************************************************
 * @file     ctrchk.h 
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

#ifndef _CTRCHK_H
#define _CTRCHK_H

#pragma pack(1)

#include "types.h"

/* ====== Typedefs ====== */


enum
{
  NO_CTR_ERROR = 0,

  /* keep the following in alphabetical order */
  CTRDECR_ERROR,
  CTRENCR_ERROR,
  CTRKEY_ERROR,
  CTRMALLOC_ERROR,
};


typedef struct ctr_info {
    uint8_t  fEncryption;
    uint8_t  micLen;
    uint8_t  keyLen;
    uint8_t  nonceLen;
    uint32_t ahdrLen;
    uint32_t mstrLen;
    uint8_t  *key;
    uint8_t  *nonce;
    uint8_t  *aHdr;
    uint8_t  *mStr;
    uint8_t  *micE;
    uint8_t  *micD;
    uint8_t  counterMod;  /* used in CTR mode */
    uint32_t initCounter; /* used in CTR mode */
} ctr_info_t;

void ctrCheck(ctr_info_t* pCTR);


#endif /* _CTRCHK_H */
