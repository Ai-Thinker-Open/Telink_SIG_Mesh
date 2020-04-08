/********************************************************************************************************
 * @file     ed25519_signature.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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
#ifndef __ed25519_signature_h__
#define __ed25519_signature_h__


#ifdef __cplusplus
extern "C" {
#endif

/* -- ed25519-sign ------------------------------------------------------------- */

#define ed25519_public_key_size     32
#define ed25519_secret_key_size     32
#define ed25519_private_key_size    64
#define ed25519_signature_size      64

/* Generate public key associated with the secret key */
void ed25519_CreateKeyPair(
    unsigned char *pubKey,              /* OUT: public key */
    unsigned char *privKey,             /* OUT: private key */
    const void *blinding,               /* IN: [optional] null or blinding context */
    const unsigned char *sk);           /* IN: secret key (32 bytes) */

/* Generate message signature */
void ed25519_SignMessage(
    unsigned char *msg,           /*  IN: [msg_size bytes] message to sign */
	int msg_size,
    const unsigned char *privKey,       /*  IN: [64 bytes] private key (sk,pk) */
    const unsigned char *pubkey,               /*  IN: [optional] null or blinding context */
   unsigned char *signature           /* OUT: [64 bytes] signature (R,S) */
);                 /* IN: size of message */

void *ed25519_Blinding_Init(
    void *context,                      /* IO: null or ptr blinding context */
    const unsigned char *seed,          /* IN: [size bytes] random blinding seed */
    size_t size);                       /* IN: size of blinding seed */

void ed25519_Blinding_Finish(
    void *context);                     /* IN: blinding context */

/* -- ed25519-verify ----------------------------------------------------------- */

/*  Single-phased signature validation.
    Returns 1 for SUCCESS and 0 for FAILURE
*/
int ed25519_VerifySignature(
    const unsigned char *signature,     /* IN: [64 bytes] signature (R,S) */
    const unsigned char *publicKey,     /* IN: [32 bytes] public key */
    const unsigned char *msg,           /* IN: [msg_size bytes] message to sign */
    size_t msg_size);                   /* IN: size of message */

/*  First part of two-phase signature validation.
    This function creates context specifc to a given public key.
    Needs to be called once per public key
*/
void * ed25519_Verify_Init(
    void *context,                      /* IO: null or verify context to use */
    const unsigned char *publicKey);    /* IN: [32 bytes] public key */

/*  Second part of two-phase signature validation.
    Input context is output of ed25519_Verify_Init() for associated public key.
    Call it once for each message/signature pairs
    Returns 1 for SUCCESS and 0 for FAILURE
*/
int ed25519_Verify_Check(
    const void          *context,       /* IN: created by ed25519_Verify_Init */
    const unsigned char *signature,     /* IN: signature (R,S) */
    const unsigned char *msg,           /* IN: message to sign */
    size_t msg_size);                   /* IN: size of message */

/* Free up context memory */
void ed25519_Verify_Finish(void *ctx);

#ifdef __cplusplus
}
#endif
#endif  /* __ed25519_signature_h__ */