/********************************************************************************************************
 * @file     BaseTypes.h
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

#ifndef __curve25519_base_type_h__
#define __curve25519_base_type_h__

#if WIN32
#include <stdint.h>
#endif

/* Little-endian as default */
#ifndef ECP_CONFIG_BIG_ENDIAN
#define ECP_CONFIG_LITTLE_ENDIAN
#endif

typedef unsigned char       U8;
typedef signed   char       S8;

#if defined(_MSC_VER)
typedef unsigned __int16    U16;
typedef signed   __int16    S16;
typedef unsigned __int32    U32;
typedef signed   __int32    S32;
typedef unsigned __int64    U64;
typedef signed   __int64    S64;
#else
typedef unsigned short      U16;
typedef signed short        S16;
typedef unsigned int         U32;
typedef signed int             S32;
typedef unsigned long long            U64;
typedef signed long long             S64;
#endif

typedef unsigned int        SZ;

#ifdef ECP_CONFIG_BIG_ENDIAN
typedef union
{
    U16 u16;
    S16 s16;
    U8 bytes[2];
    struct { U8 b1, b0; } u8;
    struct { S8 b1; U8 b0; } s8;
} M16;
typedef union
{
    U32 u32;
    S32 s32;
    U8 bytes[4];
    struct { U16 w1, w0; } u16;
    struct { S16 w1; U16 w0; } s16;
    struct { U8 b3, b2, b1, b0; } u8;
    struct { M16 hi, lo; } m16;
} M32;
typedef union
{
    U64 u64;
    S64 s64;
    U8 bytes[8];
    struct { U32 hi, lo; } u32;
    struct { S32 hi; U32 lo; } s32;
    struct { U16 w3, w2, w1, w0; } u16;
    struct { U8 b7, b6, b5, b4, b3, b2, b1, b0; } u8;
    struct { M32 hi, lo; } m32;
} M64;
#else
typedef union
{
    U16 u16;
    S16 s16;
    U8 bytes[2];
    struct { U8 b0, b1; } u8;
    struct { U8 b0; S8 b1; } s8;
} M16;
typedef union
{
    U32 u32;
    S32 s32;
    U8 bytes[4];
    struct { U16 w0, w1; } u16;
    struct { U16 w0; S16 w1; } s16;
    struct { U8 b0, b1, b2, b3; } u8;
    struct { M16 lo, hi; } m16;
} M32;
typedef union
{
    U64 u64;
    S64 s64;
    U8 bytes[8];
    struct { U32 lo, hi; } u32;
    struct { U32 lo; S32 hi; } s32;
    struct { U16 w0, w1, w2, w3; } u16;
    struct { U8 b0, b1, b2, b3, b4, b5, b6, b7; } u8;
    struct { M32 lo, hi; } m32;
} M64;
#endif

#define IN
#define OUT

#endif
