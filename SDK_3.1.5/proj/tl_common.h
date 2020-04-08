/********************************************************************************************************
 * @file     tl_common.h 
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

#include "mcu/config.h"
#include "common/types.h"
#include "common/bit.h"
#include "common/tutility.h"
#include "common/static_assert.h"
#include "common/assert.h"
#include "mesh/user_config.h"
#include "common/compatibility.h"
#if(MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/analog.h"
#include "drivers/8258/compiler.h"
#include "drivers/8258/register.h"
#include "drivers/8258/gpio.h"
#include "drivers/8258/pwm.h"
#include "drivers/8258/dma.h"
#include "drivers/8258/clock.h"
#include "drivers/8258/random.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/analog.h"
#include "drivers/8278/compiler.h"
#include "drivers/8278/register.h"
#include "drivers/8278/gpio.h"
#include "drivers/8278/pwm.h"
#include "drivers/8278/dma.h"
#include "drivers/8278/clock.h"
#include "drivers/8278/random.h"
#else
#include "mcu/analog.h"
#include "mcu/compiler.h"
#include "mcu/register.h"
#include "mcu/gpio.h"
#include "mcu/pwm.h"
#include "mcu/cpu.h"
#include "mcu/dma.h"
#include "mcu/clock.h"
#include "mcu/clock_i.h"
#include "mcu/random.h"
#endif
#include "mcu/irq_i.h"
#include "common/breakpoint.h"
#include "common/log.h"
#if !WIN32
#include "drivers/8258/flash.h"
#endif
//#include "../ble/ble_globals.h"

#if WIN32
#include <stdio.h>
#include <string.h>
#else
#include "common/printf.h"
#include "common/tstring.h"
#endif

#define DEBUG_STOP()	{reg_tmr_ctrl = 0; reg_gpio_pb_ie = 0xff; while(1);}	// disable watchdog;  DP/DM en
#define DEBUG_SWS_EN()	{reg_tmr_ctrl = 0; reg_gpio_pb_ie = 0xff;}	// disable watchdog;  DP/DM en

