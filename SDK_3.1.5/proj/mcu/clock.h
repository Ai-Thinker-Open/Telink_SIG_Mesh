/********************************************************************************************************
 * @file     clock.h 
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

#if WIN32
#include <time.h>
#endif
#include "compiler.h"
// constants
// system clock

#ifndef WIN32
#include "proj/mcu/config.h"
#if(__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/clock.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/clock.h"
#else

//  must use macro,  because used in #if
#define	CLOCK_TYPE_PLL	0
#define	CLOCK_TYPE_OSC	1
#define	CLOCK_TYPE_PAD	2
#define	CLOCK_TYPE_ADC	3

enum{
	CLOCK_SEL_32M_RC = 	0,
	CLOCK_SEL_HS_DIV = 	1,
	CLOCK_SEL_16M_PAD =	2,
	CLOCK_SEL_32M_PAD =	3,
	CLOCK_SEL_SPI  	  = 4,
	CLOCK_SEL_40M_INTERNAL = 5,
	CLOCK_SEL_32K_RC  =	6,
};

enum{
	CLOCK_HS_240M_PLL =	0,
	CLOCK_HS_40M_ADC = 	1,
	CLOCK_HS_32M_OSC =	2,
	CLOCK_HS_16M_OSC = 	3,
};

enum{
	CLOCK_PLL_CLOCK = 192000000,

	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
	CLOCK_SYS_CLOCK_4S = CLOCK_SYS_CLOCK_1S << 2,
	CLOCK_MAX_MS = (U32_MAX / CLOCK_SYS_CLOCK_1MS),
	CLOCK_MAX_US = (U32_MAX / CLOCK_SYS_CLOCK_1US),
};

enum{
	CLOCK_MCU_RUN_CODE_1S = CLOCK_SYS_CLOCK_HZ,         // 8258: not same with CLOCK_SYS_CLOCK_1S.
	CLOCK_MCU_RUN_CODE_1MS = (CLOCK_MCU_RUN_CODE_1S / 1000),
	CLOCK_MCU_RUN_CODE_1US = (CLOCK_MCU_RUN_CODE_1S / 1000000),
};

#ifdef	USE_SYS_TICK_PER_US  //for 55nm chip
	extern	u32 sys_tick_per_us;
#else
	#define 	sys_tick_per_us		CLOCK_SYS_CLOCK_1US
#endif

void 	set_tick_per_us (u32 t);

enum{
	CLOCK_MODE_SCLK = 0,
	CLOCK_MODE_GPIO = 1,
	CLOCK_MODE_WIDTH_GPI = 2,
	CLOCK_MODE_TICK = 3
};

#if(CLOCK_SYS_TYPE == CLOCK_TYPE_PLL)
	#define CLK_FHS_MZ		192
#elif(CLOCK_SYS_TYPE == CLOCK_TYPE_PAD)
	#if(CLOCK_SYS_CLOCK_HZ == 32000000)
		#define CLK_FHS_MZ		32
	#elif(CLOCK_SYS_CLOCK_HZ == 12000000)
		#define CLK_FHS_MZ		12
	#else
		#error
	#endif
#elif(CLOCK_SYS_TYPE == CLOCK_TYPE_OSC)
	#if(CLOCK_SYS_CLOCK_HZ == 32000000)
		#define CLK_FHS_MZ		192
	#elif(CLOCK_SYS_CLOCK_HZ == 16000000)
		#define CLK_FHS_MZ		32			//  DIVIDE == 2,  32/2 = 16, see reg 0x66
	#elif(CLOCK_SYS_CLOCK_HZ == 8000000)
		#define CLK_FHS_MZ		32			//  DIVIDE == 2,  32/2 = 16, see reg 0x66
	#else
		#error
	#endif
#else
		#error
#endif

void clock_init();
_attribute_ram_code_ void sleep_us (u32 microsec);		//  use register counter to delay 

static inline void delay(int us){						// use no register counter to delay 
	for(volatile int i = 0; i < us * CLOCK_SYS_CLOCK_HZ / (1000*1000); ++i){
	}
}

//  delay precisely
#define		CLOCK_DLY_1_CYC    _ASM_NOP_
#define		CLOCK_DLY_2_CYC    _ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_3_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_4_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_5_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_6_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_7_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_8_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_9_CYC    _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_
#define		CLOCK_DLY_10_CYC   _ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_;_ASM_NOP_

#if (CLOCK_SYS_CLOCK_HZ == 30000000 || CLOCK_SYS_CLOCK_HZ == 32000000)
	#define		CLOCK_DLY_100NS		CLOCK_DLY_3_CYC							// 100,  94
	#define		CLOCK_DLY_200NS		CLOCK_DLY_6_CYC							// 200, 188
	#define 	CLOCK_DLY_600NS 	CLOCK_DLY_10_CYC;CLOCK_DLY_10_CYC 		// 200, 188
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
	#define 	CLOCK_DLY_63NS 		CLOCK_DLY_3_CYC 		//  63 ns
	#define		CLOCK_DLY_100NS		CLOCK_DLY_4_CYC			//  100 ns
	#define		CLOCK_DLY_200NS		CLOCK_DLY_8_CYC			//  200 ns
	#define 	CLOCK_DLY_600NS 	CLOCK_DLY_10_CYC 		//	600 ns
#elif (CLOCK_SYS_CLOCK_HZ == 12000000 || CLOCK_SYS_CLOCK_HZ == 16000000)
	#define 	CLOCK_DLY_63NS 		CLOCK_DLY_1_CYC 		//  63 ns
	#define		CLOCK_DLY_100NS		CLOCK_DLY_2_CYC			//  128 ns
	#define		CLOCK_DLY_200NS		CLOCK_DLY_4_CYC			//  253 ns
	#define 	CLOCK_DLY_600NS 	CLOCK_DLY_10_CYC 		//	253 ns
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
	#define		CLOCK_DLY_100NS		CLOCK_DLY_5_CYC			// 104
	#define		CLOCK_DLY_200NS		CLOCK_DLY_10_CYC		// 208
	#define 	CLOCK_DLY_600NS 	CLOCK_DLY_10_CYC;CLOCK_DLY_10_CYC;CLOCK_DLY_10_CYC		//	600 ns
#elif (CLOCK_SYS_CLOCK_HZ == 6000000 || CLOCK_SYS_CLOCK_HZ == 8000000)
	#define		CLOCK_DLY_100NS		CLOCK_DLY_1_CYC			//  125 ns
	#define		CLOCK_DLY_200NS		CLOCK_DLY_2_CYC			//  250
	#define 	CLOCK_DLY_600NS 	CLOCK_DLY_5_CYC 		//  725
#else
#define		CLOCK_DLY_100NS		CLOCK_DLY_1_CYC			//  125 ns
#define		CLOCK_DLY_200NS		CLOCK_DLY_2_CYC			//  250
#define 	CLOCK_DLY_600NS 	CLOCK_DLY_5_CYC 		//  725
#endif
#endif

#else

#define CLOCK_SYS_CLOCK_HZ  	32000000

enum{
	CLOCK_PLL_CLOCK = 192000000,

	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
	CLOCK_SYS_CLOCK_4S = CLOCK_SYS_CLOCK_1S << 2,
	CLOCK_MAX_MS = (U32_MAX / CLOCK_SYS_CLOCK_1MS),
	CLOCK_MAX_US = (U32_MAX / CLOCK_SYS_CLOCK_1US),
};

#define     sys_tick_per_us     CLOCK_SYS_CLOCK_1US
#endif
