/********************************************************************************************************
 * @file     pm_8266.h 
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

#if(__TL_LIB_8266__ || MCU_CORE_TYPE == MCU_CORE_8266)

static inline void usb_dp_pullup_en (int en)
{
	unsigned char dat = ReadAnalogReg(0x00);
	if (en) {
		dat &= ~BIT(4);
	}
	else {
		dat |= BIT(4);
	}

	WriteAnalogReg (0x00, dat);
}

enum {
	// WAKEUP_SRC_ANA 0 -- 2  not supported
	 PM_WAKEUP_CORE  = BIT(5),
	 PM_WAKEUP_TIMER = BIT(6),
	 PM_WAKEUP_COMP  = BIT(7),
	 PM_WAKEUP_PAD   = BIT(8),
};


enum {
	 WAKEUP_STATUS_COMP   = BIT(0),  //wakeup by comparator
	 WAKEUP_STATUS_TIMER  = BIT(1),
	 WAKEUP_STATUS_CORE   = BIT(2),
	 WAKEUP_STATUS_PAD    = BIT(3),
};

#define 	WAKEUP_STATUS_TIMER_CORE	( WAKEUP_STATUS_TIMER | WAKEUP_STATUS_CORE)

#define DEEP_ANA_REG0    0x34
#define DEEP_ANA_REG1    0x35
#define DEEP_ANA_REG2    0x36
#define DEEP_ANA_REG3    0x37
#define DEEP_ANA_REG4    0x38
#define DEEP_ANA_REG5    0x39
#define DEEP_ANA_REG6    0x3a

#define ADV_DEEP_FLG	 0x01
#define CONN_DEEP_FLG	 0x02


void cpu_stall_wakeup_by_timer0(u32 tick_stall);
void cpu_stall_wakeup_by_timer1(u32 tick_stall);
void cpu_stall_wakeup_by_timer2(u32 tick_stall);


typedef int (*suspend_handler_t)(void);
void	bls_pm_registerFuncBeforeSuspend (suspend_handler_t func );


enum{
	// WAKEUP_SRC_ANA 0 -- 2  not supported
	 WAKEUP_PC3_GRP0 = BIT(0),
	 WAKEUP_PC4_GRP0 = BIT(1),
	 WAKEUP_PC5_GRP0 = BIT(2),
	 WAKEUP_PD0_GRP0 = BIT(3),
	 WAKEUP_PD1_GRP1 = BIT(4),
	 WAKEUP_PD2_GRP1 = BIT(5),
	 WAKEUP_PD3_GRP1 = BIT(6),
	 WAKEUP_PD4_GRP1 = BIT(7),
	 WAKEUP_PD5_GRP2 = BIT(8),
	 WAKEUP_PD6_GRP2 = BIT(9),
	 WAKEUP_PD7_GRP2 = BIT(10),
	 WAKEUP_PA0_GRP2 = BIT(11),
	 WAKEUP_PA1_GRP3 = BIT(12),
	 WAKEUP_PA2_GRP3 = BIT(13),
	 WAKEUP_PA3_GRP3 = BIT(14),
	 WAKEUP_PA4_GRP3 = BIT(15),
	 WAKEUP_PA7_GRP4 = BIT(16),
	 WAKEUP_PC6_GRP4 = BIT(17),
	 WAKEUP_PC7_GRP4 = BIT(18),
	 WAKEUP_PE0_GRP4 = BIT(19),
	 WAKEUP_PE1_GRP5 = BIT(20),
	 WAKEUP_PE2_GRP5 = BIT(21),
	 WAKEUP_PA5_GRP5 = BIT(22),
	 WAKEUP_PA6_GRP5 = BIT(23),
};
/*wakeup-level*/
enum{
	WAKEUP_GRP0_POS_EDG = 0,
	WAKEUP_GRP1_POS_EDG = 0,
	WAKEUP_GRP2_POS_EDG = 0,
	WAKEUP_GRP3_POS_EDG = 0,
	WAKEUP_GRP4_POS_EDG = 0,
	WAKEUP_GRP5_POS_EDG = 0,

	WAKEUP_GRP0_NEG_EDG = BIT(0),
	WAKEUP_GRP1_NEG_EDG = BIT(1),
	WAKEUP_GRP2_NEG_EDG = BIT(2),
	WAKEUP_GRP3_NEG_EDG = BIT(3),
	WAKEUP_GRP4_NEG_EDG = BIT(4),
	WAKEUP_GRP5_NEG_EDG = BIT(5),

};



///////////////////////////////////////////////////////////////////////
////////////////////////////battery dectect////////////////////////////
////////////////////////////////////////////////////////////////////////
/*test data
 * standard      test
 * 0.98v <--->1.022v ~ 1.024v
 * 1.1v  <--->1.144v ~ 1.150v
 * 1.18v <--->1.214v ~ 1.218v
 * 1.25v <--->1.285v ~ 1.289v
 * 1.3v  <--->1.355v ~ 1.358v
 * 1.6v  <--->1.701v ~ 1.708v
 * */
enum  COMP_CHANNALE {
	COMP_ANA3 = 0x00,	COMP_GPIO_GP11 = 0x00,
	COMP_ANA4 = 0x02,	COMP_GPIO_GP12 = 0x02,
	COMP_ANA5 = 0x04,	COMP_GPIO_SWS = 0x04,
	COMP_ANA6 = 0x06,	COMP_GPIO_CN = 0x06,
	COMP_ANA7 = 0x08,	COMP_GPIO_CK = 0x08,
	COMP_ANA8 = 0x0a,	COMP_GPIO_DO = 0x0a,
	COMP_ANA9 = 0x0c,	COMP_GPIO_DI = 0x0c,
	COMP_ANA10 = 0x0e, 	COMP_GPIO_MSCN = 0x0e,
	COMP_ANA11 = 0x10,	COMP_GPIO_MCLK = 0x10,
	COMP_ANA12 = 0x12,	COMP_GPIO_MSDO = 0x12,
	COMP_ANA13 = 0x14,	COMP_GPIO_MSDI = 0x14,
	COMP_ANA14 = 0x16,	COMP_GPIO_DMIC_CK = 0x16,	COMP_GPIO_I2S_REFCLK = 0x16,
	COMP_ANA15 = 0x18,	COMP_GPIO_I2S_BCK = 0x18,
	COMP_AVDD =  0x1a
};

enum{
	V0P98,
	V1P1,
	V1P18,
	V1P25,
	V1P3,
	V1P66,
};

#define SCALING_SELECT_QUARTER 		0x00//25%
#define SCALING_SELECT_HALF 		0x20//50%
#define SCALING_SELECT_3QUARTER 	0x40//75%
#define SCALING_SELECT_FULL 		0x60//100%

#define REF_VOLTAGE_SEL_0			0x00//float
#define REF_VOLTAGE_SEL_1			0x01//981mv
#define REF_VOLTAGE_SEL_2			0x02//937mv
#define REF_VOLTAGE_SEL_3			0x03//885mv
#define REF_VOLTAGE_SEL_4			0x04//832mv
#define REF_VOLTAGE_SEL_5			0x05//ana3
#define REF_VOLTAGE_SEL_6			0x06//ain9
#define REF_VOLTAGE_SEL_7			0x07//avddh

#ifndef		VBAT_LOW_LEVLE
#define		VBAT_LOW_LEVLE		V0P98
#endif

#ifndef		VBAT_CHANNEL
#if BATTERY_DETECTION_WITH_LDO_SET
#define		VBAT_CHANNEL		COMP_AVDD
#else
#define		VBAT_CHANNEL		COMP_ANA8
#endif
#endif

#define		V0P98_REF			REF_VOLTAGE_SEL_1
#define		V0P98_SCALE			SCALING_SELECT_FULL

#define		V1P1_REF			REF_VOLTAGE_SEL_4
#define		V1P1_SCALE			SCALING_SELECT_3QUARTER

#define		V1P18_REF			REF_VOLTAGE_SEL_3
#define		V1P18_SCALE			SCALING_SELECT_3QUARTER

#define		V1P25_REF			REF_VOLTAGE_SEL_2
#define		V1P25_SCALE			SCALING_SELECT_3QUARTER

#define		V1P3_REF			REF_VOLTAGE_SEL_1
#define		V1P3_SCALE			SCALING_SELECT_3QUARTER

#define		V1P66_REF			REF_VOLTAGE_SEL_4
#define		V1P66_SCALE			SCALING_SELECT_HALF

#define		VBAT_LOW_SCALE		(VBAT_LOW_LEVLE==V0P98 ? V0P98_SCALE  : V1P1_SCALE )
#define		VBAT_LOW_REF		(VBAT_LOW_LEVLE==V0P98 ? V0P98_REF : V1P1_REF)

int battery_detection_with_ldo (u8 chn, int set);
int battery_direct_detection (u8 chn, int set);
int battery_low_by_set ( u8 chn, u8 v_ref, u8 v_scale ) ;
int battery_low ();
void battery_by_comp_init();



#define SUSPEND_MODE	0
#define DEEPSLEEP_MODE	1

#define WAKEUP_LEVEL_L 	0
#define WAKEUP_LEVEL_H 	1

// usually, we don't use gpio wakeup in suspend mode.
// If you do need it,  pls turn on this micro, add set  wakeup pin before calling cpu_sleep_wakeup

// like:  
// reg_gpio_f_wakeup_en = SUSPEND_WAKEUP_SRC_PWM0;
// reg_gpio_f_pol = SUSPEND_WAKEUP_SRC_PWM0;
// cpu_sleep_wakeup(1, 50, 0, 0)
#define PM_SUSPEND_WAKEUP_BY_GPIO_ENABLE		0

void pm_init(void);


//deepsleep mode must use this function for resume 1.8V analog register
void cpu_wakeup_init(void);
void cpu_set_gpio_wakeup (int pin, int pol, int en);
int cpu_sleep_wakeup (int deepsleep, int wakeup_src, u32 wakeup_tick);



void blc_pm_disableFlashShutdown_when_suspend(void);


#endif
