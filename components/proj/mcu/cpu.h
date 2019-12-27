/********************************************************************************************************
 * @file     cpu.h 
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
#include "register.h"
#ifndef WIN32
static inline void cpu_reset(void){
	SET_FLD(reg_rst_clk0, FLD_RST_MCU | FLD_RST_SOFT_MCIC);		// Reset instruction cache
}

static inline void cpu_set_wakeup_src(int src){
	SET_FLD(reg_wakeup_en, src);
}

static inline void cpu_clr_wakeup_src(int src){
	CLR_FLD(reg_wakeup_en, src);
}

static inline void cpu_disable_wakeup(void){
	reg_wakeup_en = 0;
}

void cpu_suspend(void);
void cpu_reboot(void);
#endif
#ifdef __GNUC__
#define _ASM_NOP_				asm("tnop")
#else
#define _ASM_NOP_
#endif

