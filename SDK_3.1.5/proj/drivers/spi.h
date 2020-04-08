/********************************************************************************************************
 * @file     spi.h 
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

#include "../common/types.h"
#if(MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/spi.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/spi.h"
#else

void spi_write(u8 d);
u8 spi_read();

typedef void (*spi_callback_func)(u8 *);
#endif

