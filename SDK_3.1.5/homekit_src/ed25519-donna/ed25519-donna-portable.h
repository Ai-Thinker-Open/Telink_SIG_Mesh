/********************************************************************************************************
 * @file     ed25519-donna-portable.h
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
#pragma once

#include "ed25519-donna-portable-identify.h"


#if 1
//#define mul32x32_64(a,b) ((u64)((a)*(b)) << 31)
//#define mul32x32_64(a,b) ((a)*(b) + (u64)0x3300000000)
//#define mul32x32_64(a,b) (((u64) (a)) * (b))

//#define mul32x32_64(a,b) (((u64)(a))*(b))
//#define mul32x32_64(a,b) (((a))*(b) + \
	//		((((a>>16) * (b) >> 16) + ((b>>16)*(a) >> 16)) + (a>>16) * (b>>16)))

#else
u64	func_mul32x32_64 (u32 a, u32 b);
#endif

/*
#define mul32x32_64(a,b) (							\
					(((u64)((a>>16)*(b>>16)))<<32) + \
					(((u64)((a>>0)*(b>>0)))<0) + \
					(((u64)((a>>16)*(b>>0)))<16) + \
					(((u64)((a>>0)*(b>>16)))<16) )
					*/

/* platform */
#if 0
	#define DONNA_INLINE inline __attribute__((always_inline))
	#define DONNA_NOINLINE __attribute__((noinline))
	#define ALIGN(x) __attribute__((aligned(x)))
	#define ROTL32(a,b) (((a) << (b)) | ((a) >> (32 - b)))
	#define ROTR32(a,b) (((a) >> (b)) | ((a) << (32 - b)))
#else
	//#define	_attribute_ram_code_
#ifdef WIN32

	#define	_attribute_ram_code_
#ifdef __cplusplus
#define DONNA_INLINE		inline
#else
#define DONNA_INLINE 		__inline
#endif
#define DONNA_NOINLINE
#define ALIGN(x) 			__declspec(align(x))

#else
#define DONNA_INLINE		inline
#define DONNA_NOINLINE   	__attribute__((noinline)) 
#define ALIGN(x) 			__attribute__((aligned(x)))
#endif
//	#define DONNA_INLINE 
//	#define DONNA_NOINLINE 
	static inline unsigned int ROTL32(unsigned int x, unsigned int n){
		unsigned int x1 = x << n;
		unsigned int x2 = x >> (32 - n);
		return x1 | x2;
	}
	static inline unsigned int ROTR32(unsigned int x, unsigned int n){
		unsigned int x1 = x >> n;
		unsigned int x2 = x << (32 - n);
		return x1 | x2;
	}
//	#define ROTL32(a,b) (((a) << (b)) | ((a) >> (32 - b)))
//	#define ROTR32(a,b) (((a) >> (b)) | ((a) << (32 - b)))
#endif

/* endian */
#if !defined(ED25519_OPENSSLRNG)
#if 0
void U32TO8_LE(unsigned char *p, const uint32_t v) {
	p[0] = (unsigned char)(v      );
	p[1] = (unsigned char)(v >>  8);
	p[2] = (unsigned char)(v >> 16);
	p[3] = (unsigned char)(v >> 24);
}
#else
#define U32TO8_LE(p, v)										\
	{((unsigned char*)p)[0] = (unsigned char)((v)      );		\
	((unsigned char*)p)[1] = (unsigned char)((v) >>  8);		\
	((unsigned char*)p)[2] = (unsigned char)((v) >> 16);		\
	((unsigned char*)p)[3] = (unsigned char)((v) >> 24);}
#endif
#endif

#define U8TO32_LE(p)										\
	(((uint32_t)(((unsigned char*)p)[0])		) 	| 		\
	((uint32_t)(((unsigned char*)p)[1]) <<  8) 	|		\
	((uint32_t)(((unsigned char*)p)[2]) << 16) 	|		\
	((uint32_t)(((unsigned char*)p)[3]) << 24))	

