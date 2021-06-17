/********************************************************************************************************
 * @file     app_config_8258.h 
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

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "vendor/common/version.h"    // include mesh_config.h inside.
//////////////////board sel/////////////////////////////////////


#define _USER_CONFIG_DEFINED_	1	// must define this macro to make others known
#define	__LOG_RT_ENABLE__		0
//#define	__DEBUG_PRINT__			0
//////////// product  Information  //////////////////////////////
#define ID_VENDOR				0x248a			// for report
#define ID_PRODUCT_BASE			0x880C
#define STRING_VENDOR			L"Telink"
#define STRING_PRODUCT			L"BLE Mesh"
#define STRING_SERIAL			L"TLSR825X"

#define DEV_NAME                "SigMesh"

#define APPLICATION_DONGLE		0					// or else APPLICATION_DEVICE
#define	USB_PRINTER				1
#define	FLOW_NO_OS				1

/////////////////////HCI ACCESS OPTIONS///////////////////////////////////////
#define HCI_USE_NONE	0
#define HCI_USE_UART	1
#define HCI_USE_USB		2
#define HCI_ACCESS		HCI_USE_NONE

#if (HCI_ACCESS==HCI_USE_UART)
#define UART_TX_PIN		UART_TX_PB1
#define UART_RX_PIN		UART_RX_PB0
#endif

#define HCI_LOG_FW_EN   0
#if HCI_LOG_FW_EN
#define DEBUG_INFO_TX_PIN           		GPIO_PB2
#define PRINT_DEBUG_INFO                    1
#endif

#define BATT_CHECK_ENABLE       			1   //must enable
#if (BATT_CHECK_ENABLE)
//telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	//use PC5 output high level, then adc measure this high level voltage
	#define GPIO_VBAT_DETECT				GPIO_PC5
	#define PC5_FUNC						AS_GPIO
	#define PC5_INPUT_ENABLE				0
	#define ADC_INPUT_PCHN					C5P    //corresponding  ADC_InputPchTypeDef in adc.h
#endif

#define ADC_ENABLE		0
#if ADC_ENABLE
#define ADC_BASE_MODE	1	//GPIO voltage
#define ADC_VBAT_MODE	2	//Battery Voltage

#define ADC_MODE		ADC_VBAT_MODE
#define ADC_CHNM_ANA_INPUT 		GPIO_PB3 // one of ADC_GPIO_tab[]
#define ADC_PRESCALER	ADC_PRESCALER_1F8
#endif

/////////////////// mesh project config /////////////////////////////////
#define TRANSITION_TIME_DEFAULT_VAL (0x00)  // 0x41: 1 second // 0x00: means no default transition time

/////////////////// MODULE /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE			0
#if BLE_REMOTE_PM_ENABLE
#define PM_DEEPSLEEP_RETENTION_ENABLE   1   // must
#else
#define PM_DEEPSLEEP_RETENTION_ENABLE   0
#endif
#define BLE_REMOTE_SECURITY_ENABLE      0
#define BLE_IR_ENABLE					0

#ifndef BLT_SOFTWARE_TIMER_ENABLE
#define BLT_SOFTWARE_TIMER_ENABLE		0
#endif

//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K

#define	KB_LINE_HIGH_VALID				0   //dirve pin output 0 when keyscan, scanpin read 0 is valid
#define DEEPBACK_FAST_KEYSCAN_ENABLE	1   //proc fast scan when deepsleep back trigged by key press, in case key loss
#define KEYSCAN_IRQ_TRIGGER_MODE		0
#define LONG_PRESS_KEY_POWER_OPTIMIZE	1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE		0
#define STUCK_KEY_ENTERDEEP_TIME		60  //in s

//repeat key
#define KB_REPEAT_KEY_ENABLE			0
#define	KB_REPEAT_KEY_INTERVAL_MS		200
#define KB_REPEAT_KEY_NUM				4

#define	KB_MAP_REPEAT	{RC_KEY_UP,RC_KEY_DN,RC_KEY_R,RC_KEY_L}

//

//----------------------- GPIO for UI --------------------------------
//---------------  Button //invalid code
#if 1
#define	SW1_GPIO				GPIO_PC4
#define	SW2_GPIO				GPIO_PC4
#endif

//---------------  LED / PWM //invalid code
#if 1
#define PWM_R       GPIO_PB1	//red
#define PWM_G       GPIO_PB1	//green
#define PWM_B       GPIO_PB1		//blue
#define PWM_W       GPIO_PB1		//white

#define PWM_FUNC_R  AS_PWM  // AS_PWM_SECOND
#define PWM_FUNC_G  AS_PWM  // AS_PWM_SECOND
#define PWM_FUNC_B  AS_PWM  // AS_PWM_SECOND
#define PWM_FUNC_W  AS_PWM  // AS_PWM_SECOND

#define PWMID_R     (GET_PWMID(PWM_R, PWM_FUNC_R))
#define PWMID_G     (GET_PWMID(PWM_G, PWM_FUNC_G))
#define PWMID_B     (GET_PWMID(PWM_B, PWM_FUNC_B))
#define PWMID_W     (GET_PWMID(PWM_W, PWM_FUNC_W))
                    
#define PWM_INV_R   (GET_PWM_INVERT_VAL(PWM_R, PWM_FUNC_R))
#define PWM_INV_G   (GET_PWM_INVERT_VAL(PWM_G, PWM_FUNC_G))
#define PWM_INV_B   (GET_PWM_INVERT_VAL(PWM_B, PWM_FUNC_B))
#define PWM_INV_W   (GET_PWM_INVERT_VAL(PWM_W, PWM_FUNC_W))
#endif

#define GPIO_LED	GPIO_PD4


/////////////open SWS digital pullup to prevent MCU err, this is must ////////////
#define PA7_DATA_OUT			1


/////////////////// Clock  /////////////////////////////////
#define	USE_SYS_TICK_PER_US
#define CLOCK_SYS_TYPE  		CLOCK_TYPE_PLL	//  one of the following:  CLOCK_TYPE_PLL, CLOCK_TYPE_OSC, CLOCK_TYPE_PAD, CLOCK_TYPE_ADC
#define CLOCK_SYS_CLOCK_HZ  	16000000

//////////////////Extern Crystal Type///////////////////////
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal

/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		1
#define WATCHDOG_INIT_TIMEOUT		2000  //ms


#define	REGA_TID			0x3A

#define	KB_MAP_NORMAL	{\
			{RC_KEY_UP,			RC_KEY_R,			RC_KEY_4_ON}, \
			{RC_KEY_L,			RC_KEY_DN,			RC_KEY_4_OFF}, \
			{RC_KEY_1_ON,		RC_KEY_2_ON,		RC_KEY_3_ON}, \
			{RC_KEY_1_OFF,		RC_KEY_2_OFF,		RC_KEY_3_OFF}, \
			{RC_KEY_A_ON,		RC_KEY_A_OFF,		RC_KEY_M}, }

#define		KB_MAP_NUM		KB_MAP_NORMAL
#define		KB_MAP_FN		KB_MAP_NORMAL

#define	MATRIX_ROW_PULL		PM_PIN_PULLDOWN_100K
#define	MATRIX_COL_PULL		PM_PIN_PULLUP_10K
#define	KB_LINE_HIGH_VALID	0

#define	PM_PIN_PULL_DEFAULT	0   // 0:float, reduce sleep current

#define  KB_DRIVE_PINS  {GPIO_PB7, GPIO_PD7, GPIO_PA1}
#define  KB_SCAN_PINS   {GPIO_PB4, GPIO_PB5, GPIO_PB6, GPIO_PC1, GPIO_PD3}


#define PB7_FUNC		AS_GPIO
#define PD7_FUNC		AS_GPIO
#define PA1_FUNC		AS_GPIO

#define PULL_WAKEUP_SRC_PB7           MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD7           MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PA1           MATRIX_ROW_PULL

#define PB7_INPUT_ENABLE		1
#define PD7_INPUT_ENABLE		1
#define PA1_INPUT_ENABLE		1


#define PB4_FUNC		AS_GPIO
#define PB5_FUNC 		AS_GPIO
#define PB6_FUNC		AS_GPIO
#define PC1_FUNC 		AS_GPIO
#define PD3_FUNC		AS_GPIO


#define PULL_WAKEUP_SRC_PB4           MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB5           MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB6           MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PC1           MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PD3           MATRIX_COL_PULL

#define PB4_INPUT_ENABLE		1
#define PB5_INPUT_ENABLE		1
#define PB6_INPUT_ENABLE		1
#define PC1_INPUT_ENABLE		1
#define PD3_INPUT_ENABLE		1


#define MULTI_ADDR_FOR_SWITCH_EN	0
/////////////////// set default   ////////////////

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
