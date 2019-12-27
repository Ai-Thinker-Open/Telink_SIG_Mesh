/********************************************************************************************************
 * @file     gpio_8263.h 
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
#include "../common/bit.h"
#include "../common/tutility.h"
#include "../mcu/compiler.h"
#include "../mcu/register.h"
#include "gpio_default_8263.h"

enum{
	GPIO_GROUP0 	= 0x000,
	GPIO_GROUP1 	= 0x100,
	GPIO_GROUP2 	= 0x200,
	GPIO_GROUP3 	= 0x300,
	GPIO_GROUP4 	= 0x400,
	GPIO_GROUP_MASK = 0x700,
};

enum{
    GPIO_GP0  = 0x000 | BIT(0),
	GPIO_GP1  = 0x000 | BIT(1),
	GPIO_GP2  = 0x000 | BIT(2),
	GPIO_GP3  = 0x000 | BIT(3),
	GPIO_GP4  = 0x000 | BIT(4),
	GPIO_GP5  = 0x000 | BIT(5),
	GPIO_GP6  = 0x000 | BIT(6),
	GPIO_GP7  = 0x000 | BIT(7),

	GPIO_GP8  = 0x100 | BIT(0),
	GPIO_GP9  = 0x100 | BIT(1),
	GPIO_GP10 = 0x100 | BIT(2),
	GPIO_GP11 = 0x100 | BIT(3),
	GPIO_GP12 = 0x100 | BIT(4),
	GPIO_GP13 = 0x100 | BIT(5),
	GPIO_GP14 = 0x100 | BIT(6),
	GPIO_GP15 = 0x100 | BIT(7),

	GPIO_GP16 = 0x200 | BIT(0),
	GPIO_GP17 = 0x200 | BIT(1),
	GPIO_GP18 = 0x200 | BIT(2),
	GPIO_GP19 = 0x200 | BIT(3),
	GPIO_GP20 = 0x200 | BIT(4),
	GPIO_GP21 = 0x200 | BIT(5),
	GPIO_GP22 = 0x200 | BIT(6),
	GPIO_GP23 = 0x200 | BIT(7),

	GPIO_GP24 = 0x300 | BIT(0),
	GPIO_GP25 = 0x300 | BIT(1),
	GPIO_GP26 = 0x300 | BIT(2),
	GPIO_GP27 = 0x300 | BIT(3),
	GPIO_GP28 = 0x300 | BIT(4),
	GPIO_GP29 = 0x300 | BIT(5),
	GPIO_GP30 = 0x300 | BIT(6),
	GPIO_GP31 = 0x300 | BIT(7),

	GPIO_GP32 = 0x400 | BIT(0),
	GPIO_MSCN = 0x400 | BIT(1),
	GPIO_MCLK = 0x400 | BIT(2),
	GPIO_MSDO = 0x400 | BIT(3),
	GPIO_MSDI = 0x400 | BIT(4),
	GPIO_SWS  = 0x400 | BIT(5),

	GPIO_PULLUPDN_COUNT = 33,
	GPIO_MAX_COUNT = 38,
};


#define reg_gpio_in(i)			REG_ADDR8(0x580+((i>>8)<<3))
#define reg_gpio_ie(i)			REG_ADDR8(0x581+((i>>8)<<3))
#define reg_gpio_oen(i)			REG_ADDR8(0x582+((i>>8)<<3))
#define reg_gpio_out(i)			REG_ADDR8(0x583+((i>>8)<<3))
#define reg_gpio_pol(i)			REG_ADDR8(0x584+((i>>8)<<3))
#define reg_gpio_ds(i)			REG_ADDR8(0x585+((i>>8)<<3))
#define reg_gpio_gpio_func(i)	REG_ADDR8(0x586+((i>>8)<<3))
#define reg_gpio_irq_en(i)		REG_ADDR8(0x587+((i>>8)<<3))
#define reg_gpio_2risc0(i)		REG_ADDR8(0x5a8+(i>>8))
#define reg_gpio_2risc1(i)		REG_ADDR8(0x5b0+(i>>8))


static inline int gpio_is_output_en(u32 pin){
	return !BM_IS_SET(reg_gpio_oen(pin), pin & 0xff);
}

static inline int gpio_is_input_en(u32 pin){
	return BM_IS_SET(reg_gpio_ie(pin), pin & 0xff);
}

static inline void gpio_set_output_en(u32 pin, u32 value){
	u8	bit = pin & 0xff;
	if(!value){
		BM_SET(reg_gpio_oen(pin), bit);
	}else{
		BM_CLR(reg_gpio_oen(pin), bit);
	}
}

static inline void gpio_set_input_en(u32 pin, u32 value){
	u8	bit = pin & 0xff;
	if(value){
		BM_SET(reg_gpio_ie(pin), bit);
	}else{
		BM_CLR(reg_gpio_ie(pin), bit);
	}
}

static inline void gpio_set_data_strength(u32 pin, u32 value){
	u8	bit = pin & 0xff;
	if(value){
		BM_SET(reg_gpio_ds(pin), bit);
	}else{
		BM_CLR(reg_gpio_ds(pin), bit);
	}
}

static inline void gpio_en_interrupt(u32 pin){
	u8	bit = pin & 0xff;
	BM_SET(reg_gpio_irq_en(pin), bit);
}

static inline void gpio_set_interrupt(u32 pin, u32 falling){
	u8	bit = pin & 0xff;
	BM_SET(reg_gpio_irq_en(pin), bit);
	if(falling){
		BM_SET(reg_gpio_pol(pin), bit);
	}else{
		BM_CLR(reg_gpio_pol(pin), bit);
	}
}

static inline void gpio_set_interrupt_pol(u32 pin, u32 falling){
	u8	bit = pin & 0xff;
	if(falling){
		BM_SET(reg_gpio_pol(pin), bit);
	}else{
		BM_CLR(reg_gpio_pol(pin), bit);
	}
}

static inline void gpio_clr_interrupt(u32 pin){
	u8	bit = pin & 0xff;
	BM_CLR(reg_gpio_irq_en(pin), bit);
}

static inline void gpio_write(u32 pin, u32 value){
	u8	bit = pin & 0xff;
	if(value){
		BM_SET(reg_gpio_out(pin), bit);
	}else{
		BM_CLR(reg_gpio_out(pin), bit);
	}
}

static inline u32 gpio_read(u32 pin){
	return BM_IS_SET(reg_gpio_in(pin), pin & 0xff);
}

static inline u32 gpio_read_cache(u32 pin, u8 *p){
	return p[pin>>8] & (pin & 0xff);
}

static inline void gpio_read_all(u8 *p){
	p[0] = REG_ADDR8(0x580);
	p[1] = REG_ADDR8(0x588);
	p[2] = REG_ADDR8(0x590);
	p[3] = REG_ADDR8(0x598);
	p[4] = REG_ADDR8(0x5a0);
}

static inline void gpio_core_wakeup_enable_all (int en)
{
#if 0
	if (en) {
		BM_SET(reg_gpio_wakeup_en, FLD_GPIO_WAKEUP_EN);
	}
	else {
		BM_CLR(reg_gpio_wakeup_en, FLD_GPIO_WAKEUP_EN);
	}
#endif
}

//level = 1 : high wakeup
static inline void gpio_enable_wakeup_pin(u32 pin, u32 level, int en){
	u8	bit = pin & 0xff;

	if(level){
		BM_CLR(reg_gpio_pol(pin), bit);
	}else{
		BM_SET(reg_gpio_pol(pin), bit);
	}

	if (en) {
		BM_SET(reg_gpio_irq_en(pin), bit);
	}
	else {
		BM_CLR(reg_gpio_irq_en(pin), bit);
	}
}

/*******************************************************************************

*******************************************************************************/
#define GPIO_VALUE(type,pol,n)					(GPIO##n##_##type==(pol)?(1<<(n&0x07)):0)
#define GPIO_FUNC_VALUE(type,pol,func,pos)		(func##_##type==(pol)?(1<<pos):0)

#define GPIO_REG_VALUE_GROUP0(type,pol)		\
 (  GPIO_VALUE(type, pol , 0) | GPIO_VALUE(type, pol , 1) | \
	GPIO_VALUE(type, pol , 2) | GPIO_VALUE(type, pol , 3) | \
	GPIO_VALUE(type, pol , 4) | GPIO_VALUE(type, pol , 5) | \
	GPIO_VALUE(type, pol , 6) | GPIO_VALUE(type, pol , 7) )

#define GPIO_REG_VALUE_GROUP1(type,pol)		\
 (  GPIO_VALUE(type, pol , 8) | GPIO_VALUE(type, pol , 9) | \
	GPIO_VALUE(type, pol ,10) | GPIO_VALUE(type, pol ,11) | \
	GPIO_VALUE(type, pol ,12) | GPIO_VALUE(type, pol ,13) | \
	GPIO_VALUE(type, pol ,14) | GPIO_VALUE(type, pol ,15) )

#define GPIO_REG_VALUE_GROUP2(type,pol)		\
 (  GPIO_VALUE(type, pol ,16) | GPIO_VALUE(type, pol ,17) | \
	GPIO_VALUE(type, pol ,18) | GPIO_VALUE(type, pol ,19) | \
	GPIO_VALUE(type, pol ,20) | GPIO_VALUE(type, pol ,21) | \
	GPIO_VALUE(type, pol ,22) | GPIO_VALUE(type, pol ,23) )

#define GPIO_REG_VALUE_GROUP3(type,pol)		\
 (  GPIO_VALUE(type, pol ,24) | GPIO_VALUE(type, pol ,25) | \
	GPIO_VALUE(type, pol ,26) | GPIO_VALUE(type, pol ,27) | \
	GPIO_VALUE(type, pol ,28) | GPIO_VALUE(type, pol ,29) | \
	GPIO_VALUE(type, pol ,30) | GPIO_VALUE(type, pol ,31) )

#define GPIO_REG_VALUE_GROUP4(type,pol)		\
 (  GPIO_VALUE(type, pol ,32) 		   | GPIO_FUNC_VALUE(type, pol ,MSCN,1) | \
	GPIO_FUNC_VALUE(type, pol ,MCLK,2) | GPIO_FUNC_VALUE(type, pol ,MSDO,3) | \
	GPIO_FUNC_VALUE(type, pol ,MSDI,4) | GPIO_FUNC_VALUE(type, pol ,SWS, 5) )


static inline void gpio_init(void){

	reg_gpio_group0_ie 	 = (u8)GPIO_REG_VALUE_GROUP0(INPUT_ENABLE,1);
	reg_gpio_group0_oen  = (u8)GPIO_REG_VALUE_GROUP0(OUTPUT_ENABLE,0);
	reg_gpio_group0_out  = (u8)GPIO_REG_VALUE_GROUP0(DATA_OUT,1);
	reg_gpio_group0_ds 	 = (u8)GPIO_REG_VALUE_GROUP0(DATA_STRENGTH,1);
	reg_gpio_group0_func = (u8)GPIO_REG_VALUE_GROUP0(FUNC,0);

	reg_gpio_group1_ie 	 = (u8)GPIO_REG_VALUE_GROUP1(INPUT_ENABLE,1);
	reg_gpio_group1_oen  = (u8)GPIO_REG_VALUE_GROUP1(OUTPUT_ENABLE,0);
	reg_gpio_group1_out  = (u8)GPIO_REG_VALUE_GROUP1(DATA_OUT,1);
	reg_gpio_group1_ds 	 = (u8)GPIO_REG_VALUE_GROUP1(DATA_STRENGTH,1);
	reg_gpio_group1_func = (u8)GPIO_REG_VALUE_GROUP1(FUNC,0);

	reg_gpio_group2_ie 	 = (u8)GPIO_REG_VALUE_GROUP2(INPUT_ENABLE,1);
	reg_gpio_group2_oen  = (u8)GPIO_REG_VALUE_GROUP2(OUTPUT_ENABLE,0);
	reg_gpio_group2_out  = (u8)GPIO_REG_VALUE_GROUP2(DATA_OUT,1);
	reg_gpio_group2_ds 	 = (u8)GPIO_REG_VALUE_GROUP2(DATA_STRENGTH,1);
	reg_gpio_group2_func = (u8)GPIO_REG_VALUE_GROUP2(FUNC,0);

	reg_gpio_group3_ie 	 = (u8)GPIO_REG_VALUE_GROUP3(INPUT_ENABLE,1);
	reg_gpio_group3_oen  = (u8)GPIO_REG_VALUE_GROUP3(OUTPUT_ENABLE,0);
	reg_gpio_group3_out  = (u8)GPIO_REG_VALUE_GROUP3(DATA_OUT,1);
	reg_gpio_group3_ds 	 = (u8)GPIO_REG_VALUE_GROUP3(DATA_STRENGTH,1);

	reg_gpio_group4_ie 	 = (u8)GPIO_REG_VALUE_GROUP4(INPUT_ENABLE,1);
	reg_gpio_group4_oen  = (u8)GPIO_REG_VALUE_GROUP4(OUTPUT_ENABLE,0);
	reg_gpio_group4_out  = (u8)GPIO_REG_VALUE_GROUP4(DATA_OUT,1);
	reg_gpio_group4_ds 	 = (u8)GPIO_REG_VALUE_GROUP4(DATA_STRENGTH,1);
	reg_gpio_group4_func = (u8)GPIO_REG_VALUE_GROUP4(FUNC,0);

	analog_write (0x08,  PULL_WAKEUP_SRC_GPIO17 |
						(PULL_WAKEUP_SRC_GPIO18<<2) |
						(PULL_WAKEUP_SRC_GPIO19<<4) |
						(PULL_WAKEUP_SRC_GPIO20<<6));

	analog_write (0x09,  PULL_WAKEUP_SRC_GPIO21 |
						(PULL_WAKEUP_SRC_GPIO22<<2) |
						(PULL_WAKEUP_SRC_GPIO23<<4) |
						(PULL_WAKEUP_SRC_GPIO24<<6));

	analog_write (0x28,  (PULL_WAKEUP_SRC_GPIO32==PM_PIN_PULLDOWN_100K) |
						((PULL_WAKEUP_SRC_GPIO0==PM_PIN_PULLDOWN_100K)<<1) |
						((PULL_WAKEUP_SRC_GPIO1==PM_PIN_PULLDOWN_100K)<<2) |
						((PULL_WAKEUP_SRC_GPIO2==PM_PIN_PULLDOWN_100K)<<3) |
						((PULL_WAKEUP_SRC_GPIO3==PM_PIN_PULLDOWN_100K)<<4) |
						((PULL_WAKEUP_SRC_GPIO4==PM_PIN_PULLDOWN_100K)<<5) |
						((PULL_WAKEUP_SRC_GPIO5==PM_PIN_PULLDOWN_100K)<<6) |
						((PULL_WAKEUP_SRC_GPIO6==PM_PIN_PULLDOWN_100K)<<7));

	analog_write (0x29,  (PULL_WAKEUP_SRC_GPIO7==PM_PIN_PULLDOWN_100K) |
						((PULL_WAKEUP_SRC_GPIO8==PM_PIN_PULLDOWN_100K)<<1)  |
						((PULL_WAKEUP_SRC_GPIO9==PM_PIN_PULLDOWN_100K)<<2)  |
						((PULL_WAKEUP_SRC_GPIO10==PM_PIN_PULLDOWN_100K)<<3) |
						((PULL_WAKEUP_SRC_GPIO11==PM_PIN_PULLDOWN_100K)<<4) |
						((PULL_WAKEUP_SRC_GPIO12==PM_PIN_PULLDOWN_100K)<<5) |
						((PULL_WAKEUP_SRC_GPIO13==PM_PIN_PULLDOWN_100K)<<6) |
						((PULL_WAKEUP_SRC_GPIO14==PM_PIN_PULLDOWN_100K)<<7));

	analog_write (0x2a,  (PULL_WAKEUP_SRC_GPIO15==PM_PIN_PULLDOWN_100K) |
						((PULL_WAKEUP_SRC_GPIO16==PM_PIN_PULLDOWN_100K)<<1) |
						(PULL_WAKEUP_SRC_GPIO26<<2) |
						(PULL_WAKEUP_SRC_GPIO27<<4) |
						(PULL_WAKEUP_SRC_GPIO31<<6));



    #if (__BLE__DEBUG_GPIO__)
	/* Set DEBUG GPIO functions as output gpio*/
	BM_SET(reg_gpio_gpio_func(DBG_PIN0), DBG_PIN0 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN1), DBG_PIN1 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN2), DBG_PIN2 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN3), DBG_PIN3 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN4), DBG_PIN4 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN5), DBG_PIN5 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN6), DBG_PIN6 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN7), DBG_PIN7 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN8), DBG_PIN8 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN9), DBG_PIN9 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN10), DBG_PIN10 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN11), DBG_PIN11 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN12), DBG_PIN12 & 0xff);
	BM_SET(reg_gpio_gpio_func(DBG_PIN13), DBG_PIN13 & 0xff);



	gpio_set_input_en(DBG_PIN0, 0);
	gpio_set_input_en(DBG_PIN1, 0);
	gpio_set_input_en(DBG_PIN2, 0);
	gpio_set_input_en(DBG_PIN3, 0);
	gpio_set_input_en(DBG_PIN4, 0);
	gpio_set_input_en(DBG_PIN5, 0);
	gpio_set_input_en(DBG_PIN6, 0);
	gpio_set_input_en(DBG_PIN7, 0);
	gpio_set_input_en(DBG_PIN8, 0);
	gpio_set_input_en(DBG_PIN9, 0);
	gpio_set_input_en(DBG_PIN10, 0);
	gpio_set_input_en(DBG_PIN11, 0);
	gpio_set_input_en(DBG_PIN12, 0);
	gpio_set_input_en(DBG_PIN13, 0);


	gpio_set_output_en(DBG_PIN0, 1);
	gpio_set_output_en(DBG_PIN1, 1);
	gpio_set_output_en(DBG_PIN2, 1);
	gpio_set_output_en(DBG_PIN3, 1);
	gpio_set_output_en(DBG_PIN4, 1);
	gpio_set_output_en(DBG_PIN5, 1);
	gpio_set_output_en(DBG_PIN6, 1);
	gpio_set_output_en(DBG_PIN7, 1);
	gpio_set_output_en(DBG_PIN8, 1);
	gpio_set_output_en(DBG_PIN9, 1);
	gpio_set_output_en(DBG_PIN10, 1);
	gpio_set_output_en(DBG_PIN11, 1);
	gpio_set_output_en(DBG_PIN12, 1);
	gpio_set_output_en(DBG_PIN13, 1);
	#endif


}


static inline void gpio_set_func(u32 pin, u32 func){
	u8	bit = pin & 0xff;
	if(func == AS_GPIO){
		BM_SET(reg_gpio_gpio_func(pin), bit);
		return;
	}else{
		BM_CLR(reg_gpio_gpio_func(pin), bit);
	}

}

#if (__BLE__DEBUG_GPIO__)
#define BLE_DBG_GPIO_WRITE(pin, v)      do{ if(pin){gpio_write(pin, v);}}while(0);
#else
#define BLE_DBG_GPIO_WRITE(pin, v)
#endif

extern void gpio_setup_up_down_resistor(u32 gpio, u32 up_down);

