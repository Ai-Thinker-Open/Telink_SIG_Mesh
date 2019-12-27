/********************************************************************************************************
 * @file     types.h 
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


#pragma once
#include <stddef.h>  /* size_t */
#if !WIN32
#include <stdbool.h>
#endif
typedef unsigned char u8 ;
typedef signed char s8;

typedef unsigned short int u16;
typedef signed short int s16;

typedef int s32;
typedef unsigned int u32;

typedef long long s64;
typedef unsigned long long u64;

typedef unsigned char           word8;	
typedef unsigned short          word16;	
typedef unsigned long           word32;

#if !WIN32
typedef unsigned char           uint8_t;	
typedef unsigned short          uint16_t;	
typedef unsigned long           uint32_t;
#endif

#ifndef NULL
#define NULL 	0
#endif

#ifndef __cplusplus
//typedef u8 bool;

#ifndef FALSE
#define FALSE 	0
#endif
#ifndef TRUE
#define TRUE 	(!FALSE)
#endif

//#define false 	FALSE
//#define true 	TRUE

#endif

// There is no way to directly recognise whether a typedef is defined
// http://stackoverflow.com/questions/3517174/how-to-check-if-a-datatype-is-defined-with-typedef
#ifdef __GNUC__
typedef	u16	wchar_ts;		
#endif
/*
#ifndef WIN32
	#ifndef SIZE_T
	#define SIZE_T	1
	typedef unsigned int size_t;
	#endif
#endif
*/
#define U32_MAX ((u32)0xffffffff)
#define U16_MAX ((u16)0xffff)
#define U8_MAX ((u8)0xff)
#define U31_MAX ((u32)0x7fffffff)
#define U15_MAX ((u16)0x7fff)
#define U7_MAX ((u8)0x7f)


#if WIN32
#   ifndef FALSE
#        define FALSE 0
#    endif

#   ifndef TRUE
#        define TRUE 1
#   endif
#endif

#define SUCCESS                   0x00
#define FAILURE                   0x01

typedef u32 UTCTime;
typedef u32 arg_t;
typedef u32 status_t;


