/********************************************************************************************************
 * @file     sha1_telink.h 
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

#ifndef _SHA1_TELINK_H
#define _SHA1_TELINK_H

//#include <string.h>
//#include <stddef.h>


#include <stdint.h>
#ifndef NULL
#define NULL 	0
#endif


/********
SHA-1 
********/
typedef struct{
	uint32_t total[2];			/*!< number of bytes processed	*/
	uint32_t state[5];			/*!< intermediate digest state	*/
	unsigned char buffer[64];	/*!< data block being processed */
}mbedtls_sha1_context;

void mbedtls_sha1_init( mbedtls_sha1_context *ctx );

void mbedtls_sha1_free( mbedtls_sha1_context *ctx );

void mbedtls_sha1_clone( mbedtls_sha1_context *dst,
                         const mbedtls_sha1_context *src );

void mbedtls_sha1_starts( mbedtls_sha1_context *ctx );
void mbedtls_sha1_update( mbedtls_sha1_context *ctx, const unsigned char *input, size_t ilen );

void mbedtls_sha1_finish( mbedtls_sha1_context *ctx, unsigned char output[20] );
void mbedtls_sha1_process( mbedtls_sha1_context *ctx, const unsigned char data[64] );
void mbedtls_sha1( const unsigned char *input, size_t ilen, unsigned char output[20] );

#endif
