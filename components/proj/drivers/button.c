/********************************************************************************************************
 * @file     button.c 
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
#include "button.h"

#if(0)

u32 button_table[MAX_BUTTON_NUM];
u8  button_curNum = 0;
buttonPressedCb_t button_cb;
u32 buttonLastTime = 0;

void button_config(u32 pin)
{
	if (button_curNum >= MAX_BUTTON_NUM) {
		return;
	}

	gpio_set_func(pin, AS_GPIO); 
	gpio_set_input_en(pin, 1);
	gpio_set_interrupt(pin, 1);

	reg_irq_mask |= FLD_IRQ_GPIO_RISC2_EN;

	button_table[button_curNum++] = pin;
}



void button_registerCb(buttonPressedCb_t cb)
{
	button_cb = cb;
}



/*********************************************************************
 * @fn      gpio_user_irq_handler
 *
 * @brief   GPIO interrupt handler.
 *
 * @param   None
 *
 * @return  None
 */

u8 T_BTN_IRQ;
void gpio0_user_irq_handler(void)
{    
    u32 arg = 0;
    u8 i;
	T_BTN_IRQ++;

#if(SP_SRC_SPI)
	extern void spi_irq_callback();
	spi_irq_callback();
#endif
	/* This procedure is used to DISAPPEARS SHAKES */
    if ( !clock_time_exceed(buttonLastTime, BUTTON_DISAPPEAR_SHAKE_TIMER) ) {
        return;
    }
    
    buttonLastTime = clock_time();
    for(i = 0; i < button_curNum; i++) {
    	if (0 == gpio_read(button_table[i])) {
    		arg = button_table[i];
    		break;
    	}
    }

    if(arg != 0 && button_cb) {
        EV_SCHEDULE_TASK((ev_task_callback_t)button_cb, (void*)arg);
    }

}
#endif

