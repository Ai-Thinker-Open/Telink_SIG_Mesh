/********************************************************************************************************
 * @file     external_calls.h
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

#ifndef __external_calls_h__
#define __external_calls_h__
#if WIN32
#include <memory.h>
#include <malloc.h>
#endif
#define	mem_alloc(size)				malloc(size)
#define	mem_free(addr)				free(addr)
#define mem_clear(addr,size)		memset(addr,0,size)
#define mem_fill(addr,data,size)	memset(addr,data,size)

#endif  /* __external_calls_h__ */
