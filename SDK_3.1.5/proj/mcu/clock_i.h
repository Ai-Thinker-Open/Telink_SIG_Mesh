/********************************************************************************************************
 * @file     clock_i.h 
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

#ifndef WIN32
#include "proj/mcu/config.h"
#if(__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/clock.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/clock.h"
#else

// we use clock insteady of timer, to differentiate OS timers utility
static inline void clock_enable_clock(int tmr, u32 en){
	if(0 == tmr){
		SET_FLD_V(reg_tmr_ctrl, FLD_TMR0_EN, en);
	}else if(1 == tmr){
		SET_FLD_V(reg_tmr_ctrl, FLD_TMR1_EN, en);
	}else{
		SET_FLD_V(reg_tmr_ctrl, FLD_TMR2_EN, en);
	}
}

#if WIN32
static inline unsigned __int64   clock_get_cycle()
{ 
  __asm   _emit   0x0F 
  __asm   _emit   0x31 
}
#endif

static inline u32 clock_time(){
#if WIN32
#if (__LOG_RT_ENABLE__)
	unsigned __int64 tt = (unsigned __int64)clock_get_cycle();
	u32 tick = (u32)(tt * 1000000 / (2*1000*1000*1000));	// assuming the cpu clock of PC is 2 giga HZ
	return tick;
#else
	static u32 tick = 0;
	// 经验数据，不是太准。比较好的做法是，设置一个10秒的定时器，然后看实际时间差多少
	// 再自行调整比例。不能用系统时钟，因为需要在断点后保持timer 准确
	tick += 2; //(CLOCK_SYS_CLOCK_1US * 20);	
	return tick;
#endif
#else
	//return reg_tmr0_tick;
	return reg_system_tick;
#endif
}

static inline u32 clock_time2(int tmr){
	return reg_tmr_tick(tmr);
}

// check if the current time is exceed span_us from ref time
#ifndef			USE_SYS_TICK_PER_US
static inline u32 clock_time_exceed(u32 ref, u32 span_us){
	return ((u32)(clock_time() - ref) > span_us * CLOCK_SYS_CLOCK_1US);
}
#else
static inline u32 clock_time_exceed(u32 ref, u32 span_us){
	return ((u32)(clock_time() - ref) > span_us * sys_tick_per_us);
}
#endif

// more efficient than clock_set_interval
static inline void clock_set_tmr_interval(int tmr, u32 intv){
	reg_tmr_capt(tmr) = intv;
}

static inline void clock_set_tmr_mode(int tmr, u32 m){
	if(0 == tmr){
		SET_FLD_V(reg_tmr_ctrl16, FLD_TMR0_MODE, m);
	}else if(1 == tmr){
		SET_FLD_V(reg_tmr_ctrl16, FLD_TMR1_MODE, m);
	}else{
		SET_FLD_V(reg_tmr_ctrl16, FLD_TMR2_MODE, m);
	}
}

static inline u32 clock_get_tmr_status(int tmr){
	if(0 == tmr){
		return reg_tmr_ctrl & FLD_TMR0_STA;
	}else if(1 == tmr){
		return reg_tmr_ctrl & FLD_TMR1_STA;
	}else{
		return reg_tmr_ctrl & FLD_TMR2_STA;
	}
}
#endif
#else
#include "../../../reference/tl_bulk/lib_file/hw_fun.h"
#endif
