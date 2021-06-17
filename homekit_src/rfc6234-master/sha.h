/********************************************************************************************************
 * @file     sha.h
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

#include "../Configuration.h"
#include "../sha/sha512.h"
#include "../../common/types.h"

enum {
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError,      /* called Input after FinalBits or Result */
    shaBadParam         /* passed a bad parameter */
};

/*
 *  This structure will hold context information for the HMAC
 *  keyed-hashing operation.
 */
typedef struct HMACContext {
    int hashSize;               /* hash size of SHA being used */
    int blockSize;              /* block size of SHA being used */
    sha512_context shaContext;     /* SHA context */
    unsigned char k_opad[SHA_BlockSize];
                        /* outer padding - key XORd with opad */
    int Computed;               /* Is the MAC computed? */
    int Corrupted;              /* Cumulative corruption code */

} HMACContext;

/*
 *  This structure will hold context information for the HKDF
 *  extract-and-expand Key Derivation Functions.
 */
typedef struct HKDFContext {
    HMACContext hmacContext;
    int hashSize;               /* hash size of SHA being used */
    unsigned char prk[SHA_DIGESTSIZE];
                        /* pseudo-random key - output of hkdfInput */
    int Computed;               /* Is the key material computed? */
    int Corrupted;              /* Cumulative corruption code */
} HKDFContext;

/*
 *  Function Prototypes
 */

int SHA512Reset(sha512_context *c);
int SHA512Input(sha512_context *c, const void *data, unsigned int len);
int SHA512FinalBits(sha512_context *c, const void *data, unsigned int len);
int SHA512Result(sha512_context *c, unsigned char *md);

/*
 * HMAC Keyed-Hashing for Message Authentication, RFC 2104,
 * for all SHAs.
 * This interface allows a fixed-length text input to be used.
 */
extern int hmac(const unsigned char *text,     /* pointer to data stream */
    int text_len,                  /* length of data stream */
    const unsigned char *key,      /* pointer to authentication key */
    int key_len,                   /* length of authentication key */
    uint8_t digest[SHA_DIGESTSIZE]); /* caller digest to fill in */

/*
 * HMAC Keyed-Hashing for Message Authentication, RFC 2104,
 * for all SHAs.
 * This interface allows any length of text input to be used.
 */
extern int hmacReset(HMACContext *context,
                     const unsigned char *key, int key_len);
extern int hmacInput(HMACContext *context, const unsigned char *text,
                     int text_len);
extern int hmacFinalBits(HMACContext *context, uint8_t bits,
                         unsigned int bit_count);
extern int hmacResult(HMACContext *context,
                      uint8_t digest[SHA_DIGESTSIZE]);

/*
 * HKDF HMAC-based Extract-and-Expand Key Derivation Function,
 * RFC 5869, for all SHAs.
 */
extern int hkdf(const unsigned char *salt,
                int salt_len, const unsigned char *ikm, int ikm_len,
                const unsigned char *info, int info_len,
                uint8_t okm[ ], int okm_len);
extern int hkdfExtract(const unsigned char *salt,
                       int salt_len, const unsigned char *ikm,
                       int ikm_len, uint8_t prk[SHA_DIGESTSIZE]);
extern int hkdfExpand(const uint8_t prk[ ],
                      int prk_len, const unsigned char *info,
                      int info_len, uint8_t okm[ ], int okm_len);

/*
 * HKDF HMAC-based Extract-and-Expand Key Derivation Function,
 * RFC 5869, for all SHAs.
 * This interface allows any length of text input to be used.
 */
extern int hkdfReset(HKDFContext *context,
                     const unsigned char *salt, int salt_len);
extern int hkdfInput(HKDFContext *context, const unsigned char *ikm,
                     int ikm_len);
extern int hkdfFinalBits(HKDFContext *context, uint8_t ikm_bits,
                         unsigned int ikm_bit_count);
extern int hkdfResult(HKDFContext *context,
                      uint8_t prk[SHA_DIGESTSIZE],
                      const unsigned char *info, int info_len,
                      uint8_t okm[SHA_DIGESTSIZE], int okm_len);

