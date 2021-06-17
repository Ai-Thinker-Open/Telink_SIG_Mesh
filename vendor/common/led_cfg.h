/********************************************************************************************************
 * @file     led_cfg.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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

#ifndef _LED_CONFIG_H_
#define _LED_CONFIG_H_
#include "light.h"

#define PMW_MAX_TICK                        PWM_MAX_TICK


#if 0
//////////////////////////////////////////////////////////////////////
//    LED
//////////////////////////////////////////////////////////////////////
#define LED_INDICATE_LUM_VAL                (0xFF>>4)   // LED indicate frequency (lum)
#define PMW_MAX_TICK_BASE                   (255)
#define PMW_MAX_TICK_MULTI                  (256)
#define PMW_MAX_TICK                        (PMW_MAX_TICK_BASE*PMW_MAX_TICK_MULTI)

#define LED_MASK                            0x0F        // (Bit3:W, Bit2:R, Bit1:B, Bit0:G)
#define config_led_event(on,off,n,sel)      (on | (off<<8) | (n<<16) | (sel<<24))

#define LED_EVENT_FLASH_STOP                config_led_event(1,1,1,LED_MASK)

//#define LED_EVENT_FLASH_1HZ                 config_led_event(100,100,250,LED_MASK)
#define LED_EVENT_FLASH_1HZ_2TIMES          config_led_event(100,100,2*2,LED_MASK)
#define LED_EVENT_FLASH_1HZ_3TIMES          config_led_event(100,100,3*2,LED_MASK)
#define LED_EVENT_FLASH_1HZ_4TIMES          config_led_event(100,100,4*2,LED_MASK)
#define LED_EVENT_FLASH_1HZ_5TIMES          config_led_event(100,100,5*2,LED_MASK)
#define LED_EVENT_FLASH_1HZ_10TIMES         config_led_event(100,100,10*2,LED_MASK)
#define LED_EVENT_FLASH_1HZ_1_SECOND        config_led_event(100,100,1*2*1,LED_MASK)
#define LED_EVENT_FLASH_1HZ_2_SECOND        config_led_event(100,100,2*2*1,LED_MASK)
#define LED_EVENT_FLASH_1HZ_3_SECOND        config_led_event(100,100,3*2*1,LED_MASK)
#define LED_EVENT_FLASH_1HZ_4_SECOND        config_led_event(100,100,4*2*1,LED_MASK)
#define LED_EVENT_FLASH_1HZ_5_SECOND        config_led_event(100,100,5*2*1,LED_MASK)

#define LED_EVENT_FLASH_2HZ                 config_led_event(50,50,250,LED_MASK)
#define LED_EVENT_FLASH_2HZ_2TIMES          config_led_event(50,50,2*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_3TIMES          config_led_event(50,50,3*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_4TIMES          config_led_event(50,50,4*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_5TIMES          config_led_event(50,50,5*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_10TIMES         config_led_event(50,50,10*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_1_SECOND        config_led_event(50,50,1*2*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_2_SECOND        config_led_event(50,50,2*2*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_3_SECOND        config_led_event(50,50,3*2*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_4_SECOND        config_led_event(50,50,4*2*2,LED_MASK)
#define LED_EVENT_FLASH_2HZ_5_SECOND        config_led_event(50,50,5*2*2,LED_MASK)

//#define LED_EVENT_FLASH_4HZ                 config_led_event(25,25,250,LED_MASK)
#define LED_EVENT_FLASH_4HZ_5_SECOND        config_led_event(25,25,5*2*4,LED_MASK)

#define LED_EVENT_FLASH_5HZ                 config_led_event(20,20,250,LED_MASK)
#define LED_EVENT_FLASH_5HZ_3TIMES          config_led_event(20,20,3*2,LED_MASK)
#define LED_EVENT_FLASH_5HZ_5_SECOND        config_led_event(20,20,5*2*5,LED_MASK)
#endif

#endif
