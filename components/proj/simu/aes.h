/********************************************************************************************************
 * @file     aes.h 
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

#ifndef __AES_H__
#define __AES_H__


 /**
 *  @brief zigBee API AES mode definition
 */
typedef enum aes_mode {
    AES_MODE_ECB = 0,        /*!< AES mode: ECB */
    AES_MODE_CBC,            /*!< AES mode: CBC */
    AES_MODE_CTR,            /*!< AES mode: CTR */
    AES_MODE_CCM = 5,        /*!< AES mode: CCM */
    AES_MODE_MMO,            /*!< AES mode: MMO */
    AES_MODE_BYPASS,         /*!< AES mode: Bypass */
    AES_END,
} aes_mode_t;

/**  
 *  @brief AES enc/dec select type definition 
 */                                                                                     
typedef enum aes_operation{                                                                         
    AES_STS_ENCRYPTION = 0,   /*!< AES ENC/DEC select: ENCRYPTION */                                                      
    AES_STS_DECRYPTION,       /*!< AES ENC/DEC select: DECRYPTION */                                                                                                                            
} aes_operation_t;                                                                                   


/**  
 *  @brief ECB mode configure type definition 
 */
typedef struct ecb_cfg {
    aes_operation_t op;       /*!< AES operation: Encryption or Decryption */
    u8  keyLen;          /*!< AES key size */
    u8  *key;            /*!< AES key content */
    u32 srcLen;          /*!< AES text length, the content of text is in the mzsp_aes parameter */
} ecb_cfg_t;

/**  
 *  @brief CBC mode configure type definition 
 */
typedef struct cbc_cfg {
    aes_operation_t op;        /*!< AES operation: Encryption or Decryption */
    u8 keyLen;            /*!< AES key size */
    u8 *key;              /*!< AES key content */
    u8 ivLen;             /*!< AES initialize vector length */
    u8 *iv;               /*!< AES initialize vector content */
    u8 micLen;            /*!< AES mic length */
    u8 *micE;             /*!< AES mic content, used in Encryption */
    u8 *micD;             /*!< AES mic content, used in Decryption */
    u32 srcLen;           /*!< AES text length, the content of text is in the mzsp_aes parameter */
} cbc_cfg_t;

/**  
 *  @brief CTR mode configure type definition 
 */
typedef struct ctr_cfg {
    aes_operation_t op;        /*!< AES operation: Encryption or Decryption */
    u8   keyLen;          /*!< AES key size */
    u8   *key;            /*!< AES key content */
    u8   ivLen;           /*!< AES initialize vector length */
    u8   *iv;             /*!< AES initialize vector content */
    u32  counterMod;      /*!< AES counter mod */
    u32  initCount;       /*!< AES initialize count */
    u32  srcLen;          /*!< AES text length, the content of text is in the mzsp_aes parameter */
} ctr_cfg_t;

/**  
 *  @brief MMO mode configure type definition 
 */
typedef struct mmo_cfg {
    aes_operation_t op;       /*!< AES operation: Encryption or Decryption */
    u8 keyLen;           /*!< AES key size */
    u8 *key;             /*!< AES key content */
    u32 srcLen;          /*!< AES text length, the content of text is in the mzsp_aes parameter */
} mmo_cfg_t;

/**  
 *  @brief CCM mode configure type definition 
 */
typedef struct ccm_cfg {
    aes_operation_t op;      /*!< AES operation: Encryption or Decryption */
    u8  keyLen;         /*!< AES key size */
    u8  *key;           /*!< AES key content */
    u8  ivLen;          /*!< AES initialize vector length */
    u8  *iv;            /*!< AES initialize vector content */
    u8  micLen;         /*!< AES mic length */
    u8  *micE;          /*!< AES mic content, used in Encryption */
    u8  *micD;          /*!< AES mic content, used in Decryption */
    u32 aStrLen;        /*!< AES aheader length */
    u8  *aStr;          /*!< AES aheader content */
    u32 srcLen;         /*!< AES text length, the content of text is in the mzsp_aes parameter */
} ccm_cfg_t;

u32 mzsp_aes(u8 aesMode, u8 * src, u8 * dst, void * parameter);




#endif /* __AES_H__ */
