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
#define PCBA_8258_DONGLE_48PIN          1
#define PCBA_8258_C1T139A30_V1_0        2
#define PCBA_8258_C1T139A30_V1_2        3
#define PCBA_8258_C1T140A3_V1_1         4   // 32pin

#if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define PCBA_8258_SEL			PCBA_8258_C1T140A3_V1_1  // PCBA_8258_DONGLE_48PIN   //
#else
#define PCBA_8258_SEL			PCBA_8258_DONGLE_48PIN
#endif


#define _USER_CONFIG_DEFINED_	1	// must define this macro to make others known
#define	__LOG_RT_ENABLE__		0
//#define	__DEBUG_PRINT__			0

#if DUAL_MESH_ZB_BL_EN
#define FLASH_1M_ENABLE         1
#elif DUAL_VENDOR_EN
#define FLASH_1M_ENABLE         0
#else
#define FLASH_1M_ENABLE         0
#endif

#if FLASH_1M_ENABLE
#if DUAL_MESH_ZB_BL_EN
#define PINGPONG_OTA_DISABLE    1 // it can disable only when 1M flash.
#else
#define PINGPONG_OTA_DISABLE    0 // it can disable only when 1M flash.
#endif
#if	PINGPONG_OTA_DISABLE
#define SWITCH_FW_ENABLE		0 // set to 0, just for particular customer 
#endif
#endif

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
	#if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define DEBUG_INFO_TX_PIN           		(PCBA_8258_SEL == PCBA_8258_C1T140A3_V1_1 ? GPIO_PB6 : GPIO_PD7)
	#else
#define DEBUG_INFO_TX_PIN           		GPIO_PB2
	#endif
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

#if DUAL_MESH_ZB_BL_EN
#define DUAL_MODE_ADAPT_EN 			1   // dual mode as master with Zigbee
#else
#define DUAL_MODE_ADAPT_EN 			0   // dual mode as master with Zigbee
#endif
#if (0 == DUAL_MODE_ADAPT_EN)
#define DUAL_MODE_WITH_TLK_MESH_EN  0   // dual mode as slave with Telink mesh
#endif

/////////////////// mesh project config /////////////////////////////////
#define TRANSITION_TIME_DEFAULT_VAL (0x00)  // 0x41: 1 second // 0x00: means no default transition time

#if EXTENDED_ADV_ENABLE
#define MESH_DLE_MODE               MESH_DLE_MODE_EXTEND_BEAR
#define DLE_LEN_MAX_RX              (MAX_OCTETS_DATA_LEN_EXTENSION) // must MAX_OCTETS_DATA_LEN_EXTENSION
#define DLE_LEN_MAX_TX              (40)
#elif GATT_LPN_EN
#define MESH_DLE_MODE               MESH_DLE_MODE_GATT
#define DLE_LEN_MAX_RX              (56)
#define DLE_LEN_MAX_TX              (40)
#else
#define MESH_DLE_MODE               0 //MESH_DLE_MODE_EXTEND_BEAR // MESH_DLE_MODE_GATT
    #if MESH_DLE_MODE
#define DLE_LEN_MAX_RX              (MAX_OCTETS_DATA_LEN_EXTENSION)
#define DLE_LEN_MAX_TX              (40)
    #endif
#endif

/////////////////// MODULE /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE			1
#if BLE_REMOTE_PM_ENABLE
#define PM_DEEPSLEEP_RETENTION_ENABLE   1   // must
#else
#define PM_DEEPSLEEP_RETENTION_ENABLE   1
#endif
#define BLE_REMOTE_SECURITY_ENABLE      0
#define BLE_IR_ENABLE					0

#ifndef BLT_SOFTWARE_TIMER_ENABLE
#define BLT_SOFTWARE_TIMER_ENABLE		1
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
#define KB_REPEAT_KEY_NUM				1
//

//----------------------- GPIO for UI --------------------------------
#define LPN_DEBUG_PIN_EN        0

//---------------  Button 
#if (PCBA_8258_SEL == PCBA_8258_DONGLE_48PIN)
#define PULL_WAKEUP_SRC_PD6     PM_PIN_PULLUP_1M	//btn
#define PULL_WAKEUP_SRC_PD5     PM_PIN_PULLUP_1M	//btn
#define PD6_INPUT_ENABLE		1
#define PD5_INPUT_ENABLE		1
#define	SW1_GPIO				GPIO_PD6
#define	SW2_GPIO				GPIO_PD5
#elif (PCBA_8258_SEL == PCBA_8258_C1T140A3_V1_1)
#define PULL_WAKEUP_SRC_PD7     PM_PIN_PULLUP_1M	//btn
#define PULL_WAKEUP_SRC_PA1     PM_PIN_PULLUP_1M	//btn
#define PD7_INPUT_ENABLE		1
#define PA1_INPUT_ENABLE		1
#define	SW1_GPIO				GPIO_PD7
#define	SW2_GPIO				GPIO_PA1
#elif(PCBA_8258_SEL == PCBA_8258_C1T139A30_V1_0)
#define PULL_WAKEUP_SRC_PD2     PM_PIN_PULLUP_1M	//btn
#define PULL_WAKEUP_SRC_PD1     PM_PIN_PULLUP_1M	//btn
#define PD2_INPUT_ENABLE		1
#define PD1_INPUT_ENABLE		1
#define	SW1_GPIO				GPIO_PD2
#define	SW2_GPIO				GPIO_PD1
#endif

//---------------  LED / PWM
#if(PCBA_8258_SEL == PCBA_8258_DONGLE_48PIN)
#define PWM_R       GPIO_PWM1A3		//red
#define PWM_G       GPIO_PWM0A2		//green
#define PWM_B       GPIO_PWM3B0		//blue
#define PWM_W       GPIO_PWM4B1		//white
#elif(PCBA_8258_SEL == PCBA_8258_C1T140A3_V1_1)
#define PWM_R       GPIO_PWM2ND4    //red
#define PWM_G       GPIO_PWM0NA0    //green
#define PWM_B       GPIO_PWM1ND3    //blue
#define PWM_W       GPIO_PWM3D2		//yellow as white
#elif(PCBA_8258_SEL == PCBA_8258_C1T139A30_V1_0)   // PCBA_8258_DEVELOPMENT_BOARD
#define PWM_R       GPIO_PWM1ND3	//red
#define PWM_G       GPIO_PWM2ND4	//green
#define PWM_B       GPIO_PD5		//blue
#define PWM_W       GPIO_PWM3D2		//white
#elif(PCBA_8258_SEL == PCBA_8258_C1T139A30_V1_2)
#define PWM_R       GPIO_PD5	//red
#define PWM_G       GPIO_PWM1ND3	//green
#define PWM_B       GPIO_PWM3D2		//blue
#define PWM_W       GPIO_PWM2ND4		//white

#endif

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

#define GPIO_LED	PWM_R


/////////////open SWS digital pullup to prevent MCU err, this is must ////////////
#define PA7_DATA_OUT			1


//save suspend current
#define PA5_FUNC 	AS_GPIO     // USB DM
#define PA6_FUNC 	AS_GPIO     // USB DP

/////////////////// Clock  /////////////////////////////////
#define	USE_SYS_TICK_PER_US
#define CLOCK_SYS_TYPE  		CLOCK_TYPE_PLL	//  one of the following:  CLOCK_TYPE_PLL, CLOCK_TYPE_OSC, CLOCK_TYPE_PAD, CLOCK_TYPE_ADC

#if DUAL_MESH_ZB_BL_EN // keep same with zb
#define CLOCK_SYS_CLOCK_HZ  	32000000
#else
#define CLOCK_SYS_CLOCK_HZ  	32000000
#endif
//////////////////Extern Crystal Type///////////////////////
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal

/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		1
#define WATCHDOG_INIT_TIMEOUT		2000  //ms


/////////////////// set default   ////////////////

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
