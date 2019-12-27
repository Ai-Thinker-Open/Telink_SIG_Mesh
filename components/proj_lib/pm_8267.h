/********************************************************************************************************
 * @file     pm_8267.h 
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

#if(__TL_LIB_8267__ || (MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8261__ || (MCU_CORE_TYPE == MCU_CORE_8261) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE == MCU_CORE_8269)	)

typedef void (*pm_optimize_handler_t)(void);

#if 1 // use 1.5K
static inline void usb_dp_pullup_en (int en)
{
	unsigned char dat = ReadAnalogReg(0x00);
	if (en) {
		dat &= ~(BIT(4));
	}
	else {
		dat |= BIT(4);
	}

	WriteAnalogReg (0x00, dat);
}
#else  // use 10K
static inline void usb_dp_pullup_en (int en)
{
	unsigned char dat = ReadAnalogReg(0x08);
	if (en) {
		dat = (dat & 0x3f) | BIT(7);
	}
	else {
		dat = (dat & 0x3f) | BIT(6);
	}

	WriteAnalogReg (0x08, dat);
}
#endif



#define SUSPEND_MODE	0
#define DEEPSLEEP_MODE	1


//8267 analog register 0x34-0x3e can store infomation when MCU in deepsleep mode
//store your information in these ana_regs before deepsleep by calling analog_write function
//when MCU wakeup from deepsleep, read the information by by calling analog_read function

//these five below are stable
#define DEEP_ANA_REG0    0x3a
#define DEEP_ANA_REG1    0x3b
#define DEEP_ANA_REG2    0x3c
#define DEEP_ANA_REG3    0x3d
#define DEEP_ANA_REG4  	 0x3e

//these six below may have some problem when user enter deepsleep but ERR wakeup
// for example, when set a GPIO PAD high wakeup deepsleep, but this gpio is high before
// you call func cpu_sleep_wakeup, then deepsleep will be ERR wakeup, these 6 register
//   infomation loss.
#define DEEP_ANA_REG5    0x34
#define DEEP_ANA_REG6    0x35
#define DEEP_ANA_REG7    0x36
#define DEEP_ANA_REG8    0x37
#define DEEP_ANA_REG9    0x38
#define DEEP_ANA_REG10   0x39


#define ADV_DEEP_FLG	 0x01
#define CONN_DEEP_FLG	 0x02





void cpu_stall_wakeup_by_timer0(u32 tick_stall);
void cpu_stall_wakeup_by_timer1(u32 tick_stall);
void cpu_stall_wakeup_by_timer2(u32 tick_stall);

typedef int (*suspend_handler_t)(void);
void	bls_pm_registerFuncBeforeSuspend (suspend_handler_t func );


//deepsleep mode must use this function for resume 1.8V analog register
void cpu_wakeup_init(void);
void cpu_set_gpio_wakeup (int pin, int pol, int en);

int cpu_sleep_wakeup (int deepsleep, int wakeup_src, u32 wakeup_tick);

//set wakeup source
enum {
	 PM_WAKEUP_PAD   = BIT(4),
	 PM_WAKEUP_CORE  = BIT(5),
	 PM_WAKEUP_TIMER = BIT(6),
	 PM_WAKEUP_COMP  = BIT(7),
};


//wakeup status from return value of "cpu_sleep_wakeup"
enum {
	 WAKEUP_STATUS_COMP   = BIT(0),  //wakeup by comparator
	 WAKEUP_STATUS_TIMER  = BIT(1),
	 WAKEUP_STATUS_CORE   = BIT(2),
	 WAKEUP_STATUS_PAD    = BIT(3),
};

#define 	WAKEUP_STATUS_TIMER_CORE	( WAKEUP_STATUS_TIMER | WAKEUP_STATUS_CORE)




#endif
