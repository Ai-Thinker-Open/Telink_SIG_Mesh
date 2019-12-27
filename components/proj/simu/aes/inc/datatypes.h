/********************************************************************************************************
 * @file     datatypes.h 
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
/* *******************************************************
 * This file uses the sizes of the basic data types from
 * the system file to typedef the 8, 16, 32 and 64-bit
 * data types to be used in the rest of the code.  
 * ******************************************************/

#ifndef _DATATYPES_H
#define _DATATYPES_H

#include <limits.h>


#ifndef HTEST8_T
#define HTEST8_T
# if UCHAR_MAX == 0xFFu
//    typedef char int8_t;
    typedef unsigned char uint8_t;
# else
#   error Define uint8_t as an 8-bit unsigned integer type in this file
#   error Define int8_t as an 8-bit integer type in this file
# endif   /* if UCHAR_MAX == 0xFFu */
#endif   /* ifndef HTEST8_T */


#ifndef HTEST16_T
#define HTEST16_T
# if USHRT_MAX == 0xFFFFu
    typedef short int16_t;
    typedef unsigned short uint16_t;
# else
#   error Define uint16_t as a 16-bit unsigned short type in datatypes.h
#   error Define int16_t as a 16-bit short type in datatypes.h
# endif /* if USHRT_MAX == 0xFFFFu */
#endif /* ifndef HTEST16_T */


#ifndef HTEST32_T
#define HTEST32_T
# if UINT_MAX == 0xFFFFFFFFu
    typedef int int32_t;
    typedef unsigned int uint32_t;
# elif ULONG_MAX == 0xFFFFFFFFul
    typedef long int32_t;
    typedef unsigned long uint32_t;
# else
#   error Define uint32_t as a 32-bit unsigned integer type in datatypes.h
#   error Define int32_t as a 32-bit integer type in datatypes.h
# endif /* if UINT_MAX == 0xFFFFFFFFu */
#endif /* ifndef HTEST32_T */


#ifndef HTEST64_T
#define HTEST64_T
# if defined(UINT_MAX) && UINT_MAX > 0xFFFFFFFFu
#   if UINT_MAX == 0xFFFFFFFFFFFFFFFFu
#     define HTEST64_T
      typedef int int64_t;
      typedef unsigned int uint64_t;
#   endif
# elif defined(ULONG_MAX) && ULONG_MAX > 0xFFFFFFFFu
#   if ULONG_MAX == 0xFFFFFFFFFFFFFFFFul
#     define HTEST64_T
      typedef long int64_t;
      typedef unsigned long uint64_t;
#   endif
# elif defined(ULLONG_MAX) && ULLONG_MAX > 0xFFFFFFFFu
#   if ULLONG_MAX == 0xFFFFFFFFFFFFFFFFull
#     define HTEST64_T
      typedef long long int64_t;
      typedef unsigned long long uint64_t;
#    endif
# elif defined(ULONG_LONG_MAX) && ULONG_LONG_MAX > 0xFFFFFFFFu
#   if ULONG_LONG_MAX == 0xFFFFFFFFFFFFFFFFull
#     define HTEST64_T
      typedef long long int64_t;
      typedef unsigned long long uint64_t;
#   endif
# endif
#endif   /* ifndef HTEST64_T */


#endif /* _DATATYPES_H */

