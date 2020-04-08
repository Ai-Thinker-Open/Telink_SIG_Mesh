/********************************************************************************************************
 * @file     app_config_8269.h 
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

#include "mesh/version.h"    // include mesh_config.h inside.


#define _USER_CONFIG_DEFINED_	1	// must define this macro to make others known
#define	__LOG_RT_ENABLE__		0
//#define	__DEBUG_PRINT__			0
//////////// product  Information  //////////////////////////////
#define ID_VENDOR				0x248a			// for report
#define ID_PRODUCT_BASE			0x880C
#define STRING_VENDOR			L"Telink"
#define STRING_PRODUCT			L"BLE Mesh"
#define STRING_SERIAL			L"TLSR826X"

#define DEV_NAME                "SigMesh"

#define APPLICATION_DONGLE		0					// or else APPLICATION_DEVICE
#define	USB_PRINTER				1
#define	FLOW_NO_OS				1
#define XIAOMI_MODULE_ENABLE	MI_API_ENABLE
#define XIAOMI_TEST_CODE_ENABLE 	0

/////////////////////HCI ACCESS OPTIONS///////////////////////////////////////
#define HCI_USE_NONE	0
#define HCI_USE_UART	1
#define HCI_USE_USB		2

#if WIN32
#define HCI_ACCESS		HCI_USE_USB
#else
#define HCI_ACCESS		HCI_USE_NONE	
#endif

#if (HCI_ACCESS==HCI_USE_UART)
#define UART_GPIO_SEL           UART_GPIO_8267_PC2_PC3
#endif

#define HCI_LOG_FW_EN   0
#if HCI_LOG_FW_EN
#define DEBUG_INFO_TX_PIN           		GPIO_PA0
#define PRINT_DEBUG_INFO                    1
#endif

#define ADC_ENABLE		0
#if ADC_ENABLE
#define ADC_CHNM_ANA_INPUT 		AVSS
#define ADC_CHNM_REF_SRC 		RV_1P428
#endif

#define SIG_MESH_LOOP_PROC_10MS_EN	1

// -------- 
#if !WIN32   // VC no need send online status pkt
#define ONLINE_STATUS_EN        0

#define DUAL_MODE_ADAPT_EN 			0   // dual mode as master with Zigbee
#if (0 == DUAL_MODE_ADAPT_EN)
#define DUAL_MODE_WITH_TLK_MESH_EN  0   // dual mode as slave with Telink mesh
#endif
#endif
//

/////////////////// mesh project config /////////////////////////////////
#if (MESH_RX_TEST || (!MD_DEF_TRANSIT_TIME_EN))
#define TRANSITION_TIME_DEFAULT_VAL (0)
#else
	#if MI_API_ENABLE
#define TRANSITION_TIME_DEFAULT_VAL (0)
	#else
#define TRANSITION_TIME_DEFAULT_VAL (GET_TRANSITION_TIME_WITH_STEP(1, TRANSITION_STEP_RES_1S)) // (0x41)  // 0x41: 1 second // 0x00: means no default transition time
	#endif
#endif

/////////////////// MODULE /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE			0
#define BLE_REMOTE_SECURITY_ENABLE      0
#define BLE_IR_ENABLE					0
#define BLE_SIG_MESH_CERTIFY_ENABLE 	0

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
#define KB_REPEAT_KEY_NUM				1
//

//----------------------- GPIO for UI --------------------------------
//---------------  Button 
#define PD2_INPUT_ENABLE		1
#define PC5_INPUT_ENABLE		1
#define	SW2_GPIO				GPIO_PC5
#define	SW1_GPIO				GPIO_PD2
//PC5 1m pullup not very stable, so we use 10k pullup
#define PULL_WAKEUP_SRC_PC5     PM_PIN_PULLUP_10K	//btn
#define PULL_WAKEUP_SRC_PD2     PM_PIN_PULLUP_1M	//btn

//---------------  LED / PWM
#define BOARD_MHCB04P	1
#define BOARD_25MODULE_CUSTOMER	2
#define BOARD_8269_DONGLE 3
#define BOARD_MI_TEL_BOARD	4 // use the same settings with the module part 

#define BOARD_MODE 	BOARD_8269_DONGLE


#if XIAOMI_MODULE_ENABLE 
	#if (BOARD_MODE == BOARD_MHCB04P)
#define PWM_R     GPIO_PA0 			//red
#define PWM_G     GPIO_PA3 			//green
#define PWM_B	  GPIO_PB4			//blue
#define PWM_W	  GPIO_PB6			//while
    #elif(BOARD_MODE == BOARD_MI_TEL_BOARD)
#define PWM_R     GPIO_PB6 			//red
#define PWM_G     GPIO_PB4 			//green
#define PWM_B	  GPIO_PA0			//blue
#define PWM_W	  GPIO_PA3			//while

	#elif(BOARD_MODE == BOARD_25MODULE_CUSTOMER)
#define PWM_R     GPIO_PE0 			//red
#define PWM_G     GPIO_PE1 			//green
#define PWM_B	  GPIO_PB4			//blue
#define PWM_W	  GPIO_PB6			//while
	#elif(BOARD_MODE == BOARD_8269_DONGLE)
	    #if (HCI_ACCESS == HCI_USE_USB)
#define PWM_R     GPIO_PC2			//red
#define PWM_G     GPIO_PC3			//green
#define PWM_B     GPIO_PB6			//blue
#define PWM_W     GPIO_PB4			//while
        #else 
#define PWM_R     GPIO_PA0			//red
#define PWM_G     GPIO_PB4			//green
#define PWM_B     GPIO_PB6			//blue
#define PWM_W     GPIO_PA3			//while    
        #endif
	#else
	#endif
#else 
#define PWM_W     GPIO_PB4			//while
#define PWM_B     GPIO_PB6			//blue
    #if (HCI_ACCESS == HCI_USE_UART)
#define PWM_R     PWM_W			    //because PC2 is confliced with UART TX
#define PWM_G     PWM_B			    //because PC3 is confliced with UART RX
    #else
#define PWM_R     GPIO_PC2			//red
#define PWM_G     GPIO_PC3			//green
    #endif
#endif

#define PWMID_R     (GET_PWMID(PWM_R))
#define PWMID_G     (GET_PWMID(PWM_G))
#define PWMID_B     (GET_PWMID(PWM_B))
#define PWMID_W     (GET_PWMID(PWM_W))

#if (XIAOMI_MODULE_ENABLE && (BOARD_MODE == BOARD_MHCB04P))
#define PWM_INV_R   (GET_PWM_INVERT_VAL(PWM_R))
#define PWM_INV_G   (!GET_PWM_INVERT_VAL(PWM_G))
#define PWM_INV_B   (!GET_PWM_INVERT_VAL(PWM_B))
#define PWM_INV_W   (!GET_PWM_INVERT_VAL(PWM_W))
#else
#define PWM_INV_R   (GET_PWM_INVERT_VAL(PWM_R))
#define PWM_INV_G   (GET_PWM_INVERT_VAL(PWM_G))
#define PWM_INV_B   (GET_PWM_INVERT_VAL(PWM_B))
#define PWM_INV_W   (GET_PWM_INVERT_VAL(PWM_W))
#endif

#define GPIO_LED	PWM_R


/////////////open SWS digital pullup to prevent MCU err, this is must ////////////
#if 1
#define PB0_DATA_OUT			1
#else 	// 8266
#define PA0_DATA_OUT			1
#endif

#if 0	// 8266
// PB5/PB6 dp/dm for 8266
//USB DM DP input enable
#define PB5_INPUT_ENABLE		1
#define PB6_INPUT_ENABLE		1
#endif

/////////////////// Clock  /////////////////////////////////
#define	USE_SYS_TICK_PER_US
#define CLOCK_SYS_TYPE  		CLOCK_TYPE_PLL	//  one of the following:  CLOCK_TYPE_PLL, CLOCK_TYPE_OSC, CLOCK_TYPE_PAD, CLOCK_TYPE_ADC
#define CLOCK_SYS_CLOCK_HZ  	32000000

//////////////////Extern Crystal Type///////////////////////
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal

/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		1
#if (MESH_USER_DEFINE_MODE == MESH_MI_ENABLE)
#define WATCHDOG_INIT_TIMEOUT		20000  //in mi mode the watchdog timeout is 20s
#else
#define WATCHDOG_INIT_TIMEOUT		2000  //in mi mode the watchdog timeout is 20s
#endif


/////////////////// set default   ////////////////

#include "mesh/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
