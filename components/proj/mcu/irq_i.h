/********************************************************************************************************
 * @file     irq_i.h 
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

#if !WIN32
#include "proj/mcu/config.h"
#endif
#include "../config/user_config.h"
#include "../common/bit.h"
#include "register.h"
#include "../common/assert.h"

// NOTE("If irq switches changed, pls change IRQ_INIT_VALUE accordingly.");
// enalbe interrupt
#ifndef WIN32
#if(__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/irq.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/irq.h"
#else

static inline u8 irq_enable(){
	u8 r = reg_irq_en;		// don't worry,  the compiler will optimize the return value if not used
	reg_irq_en = 1;
	return r;
}
static inline u8 irq_disable(){
	u8 r = reg_irq_en;		// don't worry,  the compiler will optimize the return value if not used
	reg_irq_en = 0;
	return r;
}
static inline void irq_restore(u8 en){
	reg_irq_en = en;
}

// enable one interrupt
static inline u32 irq_get_mask(){
	return reg_irq_mask;
}

static inline void irq_set_mask(u32 msk){ 
    SET_FLD(reg_irq_mask, msk);
}

static inline void irq_clr_mask(u32 msk){
    CLR_FLD(reg_irq_mask, msk);
}

// interrupt source
static inline u32 irq_get_src(){
	return reg_irq_src;
}

static inline void irq_clr_src(){
	reg_irq_src = ONES_32;	// set to clear
}

static inline void irq_enable_type(u32 msk){
	irq_set_mask(msk);
}
static inline void irq_disable_type(u32 msk){
	irq_clr_mask(msk);
}
#endif
#else // Debug only,  to indicate in irq_handler

static int irq_in_handler = 0;
static void irq_set_in_handler(void){
	irq_in_handler = 1;
}
static  void irq_set_out_handler(void){
	irq_in_handler = 0;
}
static  u32 irq_is_in_handler(void){
	return irq_in_handler;
}
static inline u8 irq_enable(){
	u8 r=0 ;
	return r;
}
static inline u8 irq_disable(){
	u8 r =0;
	return r;
}
static inline void irq_restore(u8 en){
	irq_in_handler = en;
}

#endif

