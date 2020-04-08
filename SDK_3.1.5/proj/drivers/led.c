/********************************************************************************************************
 * @file     led.c 
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

#include "led.h"
#include "mesh/led_cfg.h"

led_count_init_function led_count_init_cb;
static inline void led_on(int led){
	gpio_write(led, 1);
}

static inline void led_off(int led){
	gpio_write(led, 0);
}

extern led_ctrl_t led_ctrl[];

void led_set_pattern(int led, int pat){
    if(led_count_init_cb){
        led_count_init_cb();
    }
	led_ctrl[led].pattern = pat;
	ET_INIT(&led_ctrl[led].et);
}

int led_handler(int led){
	int t;
	et_t *et = &led_ctrl[led].et;
	ET_BEGIN(et);

	t = led_ctrl[led].pattern_time[led_ctrl[led].pattern * 2];
	if(t > 0){
		led_on(led_ctrl[led].gpio);
		led_ctrl[led].duration = (t << 0);	// convert to us
		ET_YIELD_TIME_R(et, led_ctrl[led].duration, LED_MIN_RESP_TIME);
		
		t = led_ctrl[led].pattern_time[led_ctrl[led].pattern * 2 + 1];
		if(t > 0){		//	else < 0, always on...
			led_off(led_ctrl[led].gpio);
			if(t > LED_MIN_RESP_TIME){
				led_ctrl[led].duration = (t - LED_MIN_RESP_TIME) << 0;
				ET_YIELD_TIME_R(et, led_ctrl[led].duration, LED_MIN_RESP_TIME);
			}
		}
        led_set_next_pattern(led);
		
	}else{
		led_off(led_ctrl[led].gpio);
	}
	ET_END_R(et, LED_MIN_RESP_TIME);

}

int led_timer_handler(void *data){
	foreach_arr(i, led_ctrl){
		led_handler(i);
	}
	return 0;
}

void led_init(void){
	static ev_time_event_t led_timer = {led_timer_handler};
	ev_on_timer(&led_timer, LED_MIN_RESP_TIME*1000);
}

