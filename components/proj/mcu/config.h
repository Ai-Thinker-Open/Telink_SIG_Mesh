/********************************************************************************************************
 * @file     config.h 
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

#define	CHIP_PACKAGE_QFN64  1
#define	CHIP_PACKAGE_QFN48  2
#define	CHIP_PACKAGE_QFN40  3
#define	CHIP_PACKAGE_QFN32  4

#ifndef CHIP_PACKAGE
#define	CHIP_PACKAGE 		CHIP_PACKAGE_QFN64
#endif

#include "config_chip_type.h"


#ifndef CHIP_TYPE
#if(WIN32)
#define	CHIP_TYPE 			CHIP_TYPE_8269
#else
#define	CHIP_TYPE 			1000
#endif
#endif

#ifndef CHIP_8366_A1
#define CHIP_8366_A1    1
#endif

#define	MCU_CORE_8266 		1
#define	MCU_CORE_8366 		2
#define MCU_CORE_8368		3
#define	MCU_CORE_8267 		4
#define MCU_CORE_8263 		5
#define MCU_CORE_8261 		6
#define MCU_CORE_8269 		7
#define MCU_CORE_8258 		8
#define MCU_CORE_8278 		9

#if(CHIP_TYPE == CHIP_TYPE_8266)
	#define MCU_CORE_TYPE	MCU_CORE_8266
#elif(CHIP_TYPE == CHIP_TYPE_8267)
	#define MCU_CORE_TYPE	MCU_CORE_8267
#elif(CHIP_TYPE == CHIP_TYPE_8366)
	#define MCU_CORE_TYPE	MCU_CORE_8366
#elif(CHIP_TYPE == CHIP_TYPE_8368)
	#define MCU_CORE_TYPE	MCU_CORE_8368
#elif(CHIP_TYPE == CHIP_TYPE_8263)
	#define MCU_CORE_TYPE	MCU_CORE_8263
#elif(CHIP_TYPE == CHIP_TYPE_8261)
	#define MCU_CORE_TYPE	MCU_CORE_8261
#elif(CHIP_TYPE == CHIP_TYPE_8269)
	#define MCU_CORE_TYPE	MCU_CORE_8269
#elif(CHIP_TYPE == CHIP_TYPE_8258)
	#define MCU_CORE_TYPE	MCU_CORE_8258
#elif(CHIP_TYPE == CHIP_TYPE_8278)
	#define MCU_CORE_TYPE	MCU_CORE_8278
#else
	#define MCU_CORE_TYPE	1000
#endif


