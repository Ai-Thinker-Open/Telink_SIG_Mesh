/********************************************************************************************************
 * @file     keyboard.h 
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

#include "../drivers/usbkeycode.h"

#define KB_RETURN_KEY_MAX	6

#define	KB_NUMLOCK_STATUS_INVALID			BIT(7)
#define	KB_NUMLOCK_STATUS_POWERON			BIT(15)

#define DEEPBACK_KEY_IDLE					0
#define DEEPBACK_KEY_CACHE					1   //there is deepback key cached in kb_event_cache
#define DEEPBACK_KEY_WAIT_RELEASE   		2


#ifndef		KB_REPEAT_KEY_ENABLE
#define		KB_REPEAT_KEY_ENABLE			0
#endif

#define KEY_NONE	  	0
#define KEY_CHANGE  	1
#define KEY_SAME		2
typedef struct{
	u8 key_change_flg;
	u8 key_repeat_flg;
	u8 keycode0;
	u8 keycode1;
	u32 key_change_tick;
}repeatKey_t;

extern repeatKey_t repeat_key;

#ifndef		KEYSCAN_IRQ_TRIGGER_MODE
#define		KEYSCAN_IRQ_TRIGGER_MODE		0
#endif

typedef struct{
	u8 cnt;
	u8 ctrl_key;
	u8 keycode[KB_RETURN_KEY_MAX];
	//u8 padding[2];	//  for  32 bit padding,  if KB_RETURN_KEY_MAX change,  this should be changed
}kb_data_t;

extern kb_data_t	kb_event;
extern kb_data_t	kb_event_cache;
extern u8 deepback_key_state;
extern u32 deepback_key_tick;


#ifndef		LONG_PRESS_KEY_POWER_OPTIMIZE
#define		LONG_PRESS_KEY_POWER_OPTIMIZE		0
#endif


#ifndef		STUCK_KEY_PROCESS_ENABLE
#define		STUCK_KEY_PROCESS_ENABLE			0
#endif





int kb_is_data_same(kb_data_t *a, kb_data_t *b);

static inline int kb_is_key_valid(kb_data_t *p){
	return (p->cnt || p->ctrl_key);
}
static inline void kb_set_key_invalid(kb_data_t *p){
	p->cnt = p->ctrl_key = 0;
}


extern u32 kb_key_pressed(u8 * gpio);
extern u32 kb_scan_key_value (int numlock_status, int read_key,u8 * gpio);
void global_var_no_ret_init_kb();

extern u32	scan_pin_need;


static inline u32 kb_scan_key (int numlock_status, int read_key) {
	u8 gpio[8];

#if(KEYSCAN_IRQ_TRIGGER_MODE)
	static u8 key_not_released = 0;

	if(numlock_status & KB_NUMLOCK_STATUS_POWERON){
		key_not_released = 1;
	}

	if(reg_irq_src & FLD_IRQ_GPIO_EN){  //FLD_IRQ_GPIO_RISC2_EN
		key_not_released = 1;
		reg_irq_src = FLD_IRQ_GPIO_EN;  //FLD_IRQ_GPIO_RISC2_EN
	}
	else{  //no key press
		if(!key_not_released && !(numlock_status & KB_NUMLOCK_STATUS_POWERON)){
			return 0;
		}
	}
#endif

	scan_pin_need = kb_key_pressed (gpio);
	if(scan_pin_need){
		return  kb_scan_key_value(numlock_status,read_key,gpio);
	}
	else{
#if (KB_REPEAT_KEY_ENABLE)
		repeat_key.key_change_flg = KEY_NONE;
#endif
#if (KEYSCAN_IRQ_TRIGGER_MODE)
		key_not_released = 0;
#endif
		return 0;
	}
}
