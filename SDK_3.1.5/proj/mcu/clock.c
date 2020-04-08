/********************************************************************************************************
 * @file     clock.c 
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

#include "../tl_common.h"
#ifndef WIN32 
#if MODULE_AUDIO_ENABLE
#define CLK_SBC_ENABLE		1
#define CLK_AUD_ENABLE		1
#define CLK_DFIFO_ENABLE	1
#define CLK_USB_ENABLE		1
#endif

#if (MODULE_USB_ENABLE)
#define CLK_AUD_ENABLE		1
#endif

#ifndef CLK_SBC_ENABLE
#define CLK_SBC_ENABLE		1
#endif
#ifndef CLK_AUD_ENABLE
#define CLK_AUD_ENABLE		1
#endif
#ifndef CLK_DFIFO_ENABLE
#define CLK_DFIFO_ENABLE	1
#endif
#ifndef CLK_USB_ENABLE
#define CLK_USB_ENABLE		(APPLICATION_DONGLE)
#endif


void clock_init(){

	reg_rst_clk0 = 0xff000000 | (CLK_USB_ENABLE ? FLD_CLK_USB_EN: 0);

#if(CLOCK_SYS_TYPE == CLOCK_TYPE_PLL)
	reg_clk_sel = MASK_VAL(FLD_CLK_SEL_DIV, (CLOCK_PLL_CLOCK / CLOCK_SYS_CLOCK_1S), FLD_CLK_SEL_SRC, CLOCK_SEL_HS_DIV);
#elif(CLOCK_SYS_TYPE == CLOCK_TYPE_PAD)

	//STATIC_ASSERT(CLK_FHS_MZ == 32);
	#if(CLOCK_SYS_CLOCK_HZ == 12000000)
		reg_clk_sel = 0x40;
	#else
		#error clock not set properly
	#endif
	
#elif(CLOCK_SYS_TYPE == CLOCK_TYPE_OSC)
	#if(MCU_CORE_TYPE == MCU_CORE_8267 || MCU_CORE_TYPE == MCU_CORE_8261 || MCU_CORE_TYPE == MCU_CORE_8269)
		#if(CLOCK_SYS_CLOCK_HZ == 32000000)
			reg_fhs_sel = 0;
			reg_clk_sel = 0x80;	// bit[7] must be "1"
		#elif(CLOCK_SYS_CLOCK_HZ == 16000000)
			reg_fhs_sel = 0;
			reg_clk_sel = 0xa2;
		#elif(CLOCK_SYS_CLOCK_HZ == 8000000)
			reg_fhs_sel = 0;
			reg_clk_sel = 0xa4;
		#else
			#error clock not set properly
		#endif
	#else
		#if(CLOCK_SYS_CLOCK_HZ == 32000000)
			reg_fhs_sel = 0;
			reg_clk_sel = 0;	// must be zero
		#elif(CLOCK_SYS_CLOCK_HZ == 16000000)
			reg_fhs_sel = FHS_SEL_32M_OSC;
			reg_clk_sel = MASK_VAL(FLD_CLK_SEL_DIV, 2, FLD_CLK_SEL_SRC, CLOCK_SEL_HS_DIV);
		#elif(CLOCK_SYS_CLOCK_HZ == 8000000)
			reg_fhs_sel = FHS_SEL_32M_OSC;
			reg_clk_sel = MASK_VAL(FLD_CLK_SEL_DIV, 4, FLD_CLK_SEL_SRC, CLOCK_SEL_HS_DIV);
		#else
			#error clock not set properly
		#endif
	#endif
#else
	#error clock not set properly
#endif
	//reg_clk_en = 0xff | CLK_EN_TYPE;
	reg_tmr_ctrl = MASK_VAL(FLD_TMR0_EN, 1
		, FLD_TMR_WD_CAPT, (MODULE_WATCHDOG_ENABLE ? (WATCHDOG_INIT_TIMEOUT * CLOCK_SYS_CLOCK_1MS >> WATCHDOG_TIMEOUT_COEFF):0)
		, FLD_TMR_WD_EN, (MODULE_WATCHDOG_ENABLE?1:0));
}

_attribute_ram_code_ void sleep_us (u32 us)
{
	u32 t = clock_time();
	while(!clock_time_exceed(t, us)){
	}
}


#ifdef	USE_SYS_TICK_PER_US
u32		sys_tick_per_us = 16;
void set_tick_per_us (u32 t)
{
	sys_tick_per_us = t;
}
#else
void set_tick_per_us (u32 t)
{

}
#endif
#endif



