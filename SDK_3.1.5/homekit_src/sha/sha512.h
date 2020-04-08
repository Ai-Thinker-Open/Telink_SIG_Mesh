/********************************************************************************************************
 * @file     sha512.h
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

#ifndef POLARSSL_SHA512_H
#define POLARSSL_SHA512_H


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #define UL64(x) x##ui64
  typedef unsigned __int64 uint64_t;
#else
//  #include <inttypes.h>
  #define UL64(x) x##ULL
#endif

#define POLARSSL_ERR_SHA512_FILE_IO_ERROR              -0x007A  /**< Read/write error in file. */

#if !defined(POLARSSL_SHA512_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-512 context structure
 */
typedef struct
{
    uint64_t total[2];          /*!< number of bytes processed  */
    uint64_t state[8];          /*!< intermediate digest state  */
    unsigned char buffer[128];  /*!< data block being processed */

    unsigned char ipad[128];    /*!< HMAC: inner padding        */
    unsigned char opad[128];    /*!< HMAC: outer padding        */
    int is384;                  /*!< 0 => SHA-512, else SHA-384 */
}
sha512_context;

/**
 * \brief          Initialize SHA-512 context
 *
 * \param ctx      SHA-512 context to be initialized
 */
void sha512_init( sha512_context *ctx );
void sha512_init_start( sha512_context *ctx );

/**
 * \brief          Clear SHA-512 context
 *
 * \param ctx      SHA-512 context to be cleared
 */
void sha512_free( sha512_context *ctx );

/**
 * \brief          SHA-512 context setup
 *
 * \param ctx      context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512_starts( sha512_context *ctx, int is384 );

/**
 * \brief          SHA-512 process buffer
 *
 * \param ctx      SHA-512 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha512_update( sha512_context *ctx, const unsigned char *input,
                    size_t ilen );

/**
 * \brief          SHA-512 final digest
 *
 * \param ctx      SHA-512 context
 * \param output   SHA-384/512 checksum result
 */
void sha512_finish( sha512_context *ctx, unsigned char output[64] );

#ifdef __cplusplus
}
#endif

#else  /* POLARSSL_SHA512_ALT */
#include "sha512_alt.h"
#endif /* POLARSSL_SHA512_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = SHA-512( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512( const unsigned char *input, size_t ilen,
             unsigned char output[64], int is384 );

/**
 * \brief          Output = SHA-512( file contents )
 *
 * \param path     input file name
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 *
 * \return         0 if successful, or POLARSSL_ERR_SHA512_FILE_IO_ERROR
 */
int sha512_file( const char *path, unsigned char output[64], int is384 );

/**
 * \brief          SHA-512 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void sha512_hmac_starts( sha512_context *ctx, const unsigned char *key,
                         size_t keylen, int is384 );

/**
 * \brief          SHA-512 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha512_hmac_update( sha512_context *ctx, const unsigned char *input,
                         size_t ilen );

/**
 * \brief          SHA-512 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-384/512 HMAC checksum result
 */
void sha512_hmac_finish( sha512_context *ctx, unsigned char output[64] );

/**
 * \brief          SHA-512 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void sha512_hmac_reset( sha512_context *ctx );

/**
 * \brief          Output = HMAC-SHA-512( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-384/512 result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void sha512_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[64], int is384 );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int sha512_self_test( int verbose );

/* Internal use */
void sha512_process( sha512_context *ctx, const unsigned char data[128] );

#define SHA_DIGESTSIZE 		64
#define SHA_BlockSize 		128

#ifdef __cplusplus
}
#endif

#endif /* sha512.h */
