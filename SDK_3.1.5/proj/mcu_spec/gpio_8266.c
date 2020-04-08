/********************************************************************************************************
 * @file     gpio_8266.c 
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


#include "../config/user_config.h"
#include "../mcu/config.h"

#if(__TL_LIB_8266__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8266))

#include "../common/types.h"
#include "../common/compatibility.h"
#include "../common/bit.h"
#include "../common/tutility.h"
#include "../common/static_assert.h"
#include "../mcu/compiler.h"
#include "../mcu/register.h"
#include "../mcu/anareg.h"
#include "../mcu/analog.h"

#include "../mcu/gpio.h"
#include "proj_lib/pm.h"

void gpio_set_wakeup(u32 pin, u32 level, int en)
{
    u8 bit = pin & 0xff;
    if (en) {
        BM_SET(reg_gpio_irq_en0(pin), bit);
    }
    else {
        BM_CLR(reg_gpio_irq_en0(pin), bit);
    }
    if(level){
        BM_CLR(reg_gpio_pol(pin), bit);
    }else{
        BM_SET(reg_gpio_pol(pin), bit);
    }
}



/************
 *
 * gpio:         indicate the pin
 * up_down:      0	  : float
 * 				 1    :	1M   pullup
 * 				 2    : 10K  pullup
 * 				 3    : 100K pulldown
 *
 *     BIT(7.6)   BIT(5.4)   BIT(3.2)   BIT(1.0)
mask_not 0x3f       0xcf	  0xf3       0xfc

 0a		 PA1         PA0
 0b		 PA5         PA4      PA3        PA2		0
 0c		 PB1         PB0      PA7        PA6
 0d		 PB5         PB4      PB3        PB2		1
 0e		 PC1         PC0      PB7        PB6
 0f		 PC5         PC4      PC3        PC2		2
 10		 PD1         PD0      PC7        PC6
 11		 PD5         PD4      PD3        PD2		3
 12		 PE1         PE0      PD7        PD6
 13		 PE5         PE4      PE3        PE2		4
 14		 PF1         PF0      PE7        PE6
 */
//if GPIO_DP,please check usb_dp_pullup_en() valid or not first.
void gpio_setup_up_down_resistor(u32 gpio, u32 up_down) {
	u8 r_val;

	if(up_down == PM_PIN_UP_DOWN_FLOAT) {
		r_val = 0;
	}
	else if(up_down == PM_PIN_PULLUP_1M) {
		r_val = PM_PIN_PULLUP_1M;
	}
	else if(up_down == PM_PIN_PULLUP_10K) {
		r_val = PM_PIN_PULLUP_10K;
	}
	else {
		r_val = PM_PIN_PULLDOWN_100K;
	}
	
	u8 pin = gpio & 0xff;
	u8 base_ana_reg = 0x0b + ((gpio >> 8) << 1);
	u8 mask_not = 0xfc;   //default for  PX2  PX6
	u8 shift_num = 0;

	if(pin & 0x03) {
		base_ana_reg -= 1;
	}
	else if(pin & 0xc0) {
		base_ana_reg += 1;
	}
	else {
	}

	if(pin & 0x88) { //PX3  PX7
		mask_not = 0xf3;
		shift_num = 2;
	}
	else if(pin & 0x11) {   //PX0  PX4
		mask_not = 0xcf;
		shift_num = 4;
	}
	else if(pin & 0x22) {   //PX1  PX5
		mask_not = 0x3f;
		shift_num = 6;
	}

    if(GPIO_DP == gpio){
        usb_dp_pullup_en (0);
    }

	analog_write(base_ana_reg, (analog_read(base_ana_reg) & mask_not) | (r_val << shift_num));
}

/********************************************************
*
*	@brief		gpio_set_interrupt_init
*
*	@param		pin - gpio.
*				up_down - PM_PIN_PULLUP_10K; PM_PIN_PULLDOWN_100K.
*				falling - 0:rising edge; 1:falling edge.
*
*	@return		None
*/
void gpio_set_interrupt_init(u32 pin, u32 up_down, u32 falling)
{
    gpio_setup_up_down_resistor(pin, up_down);
    gpio_set_func(pin, AS_GPIO);
    gpio_set_output_en(pin, 0);
    gpio_set_input_en(pin, 1);
    gpio_write(pin, 0);
    gpio_set_interrupt(pin, falling);
    reg_irq_mask |= FLD_IRQ_GPIO_RISC2_EN;
}

#endif

