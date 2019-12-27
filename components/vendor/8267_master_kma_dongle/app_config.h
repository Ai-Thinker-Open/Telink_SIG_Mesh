/********************************************************************************************************
 * @file     app_config.h 
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

#define _USER_CONFIG_DEFINED_	1	// must define this macro to make others known
#define	__LOG_RT_ENABLE__		0
//#define	__DEBUG_PRINT__			0
//////////// product  Information  //////////////////////////////
#define ID_VENDOR				0x248a			// for report
#define ID_PRODUCT_BASE			0x880C
#define STRING_VENDOR			L"Telink"
#define STRING_PRODUCT			L"BLE Remote KMA Dongle"
#define STRING_SERIAL			L"TLSR826X"

#define	FLOW_NO_OS				1

#define SIG_MESH_TEST_ENABLE 	0

#if(__PROJECT_8261_MASTER_KMA_DONGLE__)
	#define FLASH_ADR_PARING   0x1e000
#else
	#define FLASH_ADR_PARING   0x78000
#endif


/////////////////// MODULE /////////////////////////////////
#define BLE_HOST_SMP_ENABLE							0  //1 for standard security management,  0 for telink referenced paring&bonding(no security)
#define BLE_HOST_SIMPLE_SDP_ENABLE					1  //simple service discovery

#define UI_AUDIO_ENABLE								1
#define UI_BUTTON_ENABLE							1
#define UI_LED_ENABLE								1
#define BLS_TELINK_MESH_SCAN_MODE_ENABLE		0

#define LL_MASTER_MULTI_CONNECTION				0

#if (__PROJECT_8261_MASTER_KMA_DONGLE__)
	#define KMA_DONGLE_OTA_ENABLE				    0 //slave ota test
#else
	#define KMA_DONGLE_OTA_ENABLE					1  //slave ota test
#endif


//////////////////////CAN NOT CHANGE CONFIG ABOVE ////////////////////////////
#define MESH_FUNCTION                       1


#define LL_MASTER_SINGLE_CONNECTION					1

#define SIG_MESH_GATT_TEST 1

#define APPLICATION_DONGLE							1
#if(APPLICATION_DONGLE)
	#define	USB_PRINTER_ENABLE 		1
	#define	USB_SPEAKER_ENABLE 		0
	#define	USB_MIC_ENABLE 			0
	#define	USB_MOUSE_ENABLE 		1
	#define	USB_KEYBOARD_ENABLE 	1
	#define	USB_SOMATIC_ENABLE      0   //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
	#define USB_CUSTOM_HID_REPORT	1
#endif

//////////////////// Audio /////////////////////////////////////
#define MIC_RESOLUTION_BIT		16
#define MIC_SAMPLE_RATE			16000
#define MIC_CHANNLE_COUNT		1
#define	MIC_ENOCDER_ENABLE		0

#define AUDIO_SDM_ENBALE		0//if using sdm playback, should better disable USB MIC

////////////////////////////////////////////////////
#define HCI_NONE		0    //ble host on app.c
#define HCI_USE_UART	1    //ble host on windows/android/linux through uart  115200
#define HCI_USE_USB		2    //ble host on windows/android/linux through usb bulk in out
#define HCI_ACCESS		HCI_USE_USB


#if(HCI_ACCESS == HCI_USE_UART)
	#define LINK_LAYER_TEST_ENABLE				0  //just for controller debug
	#define TEST_SPECAIL_ADV_ACCESS_CODE		0  //just for controller scan debug
#endif


////////////////////////// MIC BUFFER /////////////////////////////
#define BLE_DMIC_ENABLE					0  //0: Amic   1: Dmic
#define	MIC_ADPCM_FRAME_SIZE		128 //128
#define	MIC_SHORT_DEC_SIZE			248 //248

#if BLE_DMIC_ENABLE
	#define	TL_MIC_32K_FIR_16K			0
#else
	#define	TL_MIC_32K_FIR_16K			1
#endif

#if (AUDIO_SDM_ENBALE)
#define TL_SDM_BUFFER_SIZE			1024
#endif

#if TL_MIC_32K_FIR_16K
	#define	TL_MIC_BUFFER_SIZE				1984
#else
	#define	TL_MIC_BUFFER_SIZE				992
#endif



#if __PROJECT_8261_MASTER_KMA_DONGLE__ || __PROJECT_8267_MASTER_KMA_DONGLE__ || __PROJECT_8269_MASTER_KMA_DONGLE__
	//----------------------- GPIO for UI --------------------------------
	//---------------  Button ----------------------------------
	#define PD2_INPUT_ENABLE		1
	#define PC5_INPUT_ENABLE		1
	#define	SW2_GPIO				GPIO_PC5
	#define	SW1_GPIO				GPIO_PD2
	//PC5 1m pullup not very stable, so we use 10k pullup
	//#define PULL_WAKEUP_SRC_PC5     PM_PIN_PULLUP_1M	//btn
	#define PULL_WAKEUP_SRC_PC5     PM_PIN_PULLUP_10K	//btn
	#define PULL_WAKEUP_SRC_PD2     PM_PIN_PULLUP_1M  	//btn

	// 8267
	#define	 GPIO_LED_WHITE			GPIO_PB4
	#define	 GPIO_LED_GREEN			GPIO_PB6
	#define	 GPIO_LED_RED			GPIO_PC2
	#define	 GPIO_LED_BLUE			GPIO_PC3
	#define	 GPIO_LED_YELLOW		GPIO_PC3


	#define  PB4_INPUT_ENABLE		0
	#define	 PB4_OUTPUT_ENABLE		1
	#define  PB6_INPUT_ENABLE		0
	#define	 PB6_OUTPUT_ENABLE		1
	#define  PC2_INPUT_ENABLE		0
	#define  PC2_OUTPUT_ENABLE		1
	#define  PC3_INPUT_ENABLE		0
	#define	 PC3_OUTPUT_ENABLE		1
	#define  PC0_INPUT_ENABLE		0
	#define	 PC0_OUTPUT_ENABLE		1
#else  //8266 kma master dongle
	#define PD4_INPUT_ENABLE		1
	#define PD5_INPUT_ENABLE		1
	#define	SW1_GPIO				GPIO_PD5
	#define	SW2_GPIO				GPIO_PD4
	#define PULL_WAKEUP_SRC_PD4		PM_PIN_PULLUP_10K	//btn
	#define PULL_WAKEUP_SRC_PD5		PM_PIN_PULLUP_10K	//btn

	#define	 GPIO_LED_GREEN			GPIO_PC0
	#define	 GPIO_LED_RED			GPIO_PC4
	#define	 GPIO_LED_BLUE			GPIO_PC2
	#define	 GPIO_LED_WHITE			GPIO_PA1

	#define  PC0_INPUT_ENABLE		0
	#define	 PC0_OUTPUT_ENABLE		1
	#define  PC4_INPUT_ENABLE		0
	#define	 PC4_OUTPUT_ENABLE		1
	#define  PC2_INPUT_ENABLE		0
	#define  PC2_OUTPUT_ENABLE		1
	#define  PA1_INPUT_ENABLE		0
	#define	 PA1_OUTPUT_ENABLE		1
#endif


#define LED_ON_LEVAL 		1 		//gpio output high voltage to turn on led





/////////////////// Clock  /////////////////////////////////
#define	USE_SYS_TICK_PER_US
#define CLOCK_SYS_TYPE  		CLOCK_TYPE_PLL	//  one of the following:  CLOCK_TYPE_PLL, CLOCK_TYPE_OSC, CLOCK_TYPE_PAD, CLOCK_TYPE_ADC
#define CLOCK_SYS_CLOCK_HZ  	32000000

//////////////////Extern Crystal Type///////////////////////
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal


/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		2000  //ms









#define DEBUG_GPIO_ENABLE							0

#if(DEBUG_GPIO_ENABLE)
	#if (__PROJECT_8266_MASTER_KMA_DONGLE__)
		//ch0-ch7: B0 A5 E5 F0 F1 E7 E6 E4
		#define PB0_INPUT_ENABLE					0
		#define PA5_INPUT_ENABLE					0
		#define PE5_INPUT_ENABLE					0
		#define PF0_INPUT_ENABLE					0
		#define PF1_INPUT_ENABLE					0
		#define PE7_INPUT_ENABLE					0
		#define PE6_INPUT_ENABLE					0
		#define PE4_INPUT_ENABLE					0
		#define PB0_OUTPUT_ENABLE					1
		#define PA5_OUTPUT_ENABLE					1
		#define PE5_OUTPUT_ENABLE					1
		#define PF0_OUTPUT_ENABLE					1
		#define PF1_OUTPUT_ENABLE					1
		#define PE7_OUTPUT_ENABLE					1
		#define PE6_OUTPUT_ENABLE					1
		#define PE4_OUTPUT_ENABLE					1

		#define DBG_CHN0_LOW		( *(unsigned char *)0x80058b &= (~0x01) )   //PB0
		#define DBG_CHN0_HIGH		( *(unsigned char *)0x80058b |= 0x01 )
		#define DBG_CHN0_TOGGLE		( *(unsigned char *)0x80058b ^= 0x01 )
		#define DBG_CHN1_LOW		( *(unsigned char *)0x800583 &= (~0x20) )   //PA5
		#define DBG_CHN1_HIGH		( *(unsigned char *)0x800583 |= 0x20 )
		#define DBG_CHN1_TOGGLE		( *(unsigned char *)0x800583 ^= 0x20 )
		#define DBG_CHN2_LOW		( *(unsigned char *)0x8005a3 &= (~0x20) )   //PE5
		#define DBG_CHN2_HIGH		( *(unsigned char *)0x8005a3 |= 0x20 )
		#define DBG_CHN2_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x20 )
		#define DBG_CHN3_LOW		( *(unsigned char *)0x8005ab &= (~0x01) )   //PF0
		#define DBG_CHN3_HIGH		( *(unsigned char *)0x8005ab |= 0x01 )
		#define DBG_CHN3_TOGGLE		( *(unsigned char *)0x8005ab ^= 0x01 )
		#define DBG_CHN4_LOW		( *(unsigned char *)0x8005ab &= (~0x02) )   //PF1
		#define DBG_CHN4_HIGH		( *(unsigned char *)0x8005ab |= 0x02 )
		#define DBG_CHN4_TOGGLE		( *(unsigned char *)0x8005ab ^= 0x02 )
		#define DBG_CHN5_LOW		( *(unsigned char *)0x8005a3 &= (~0x80) )   //PE7
		#define DBG_CHN5_HIGH		( *(unsigned char *)0x8005a3 |= 0x80 )
		#define DBG_CHN5_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x80 )
		#define DBG_CHN6_LOW		( *(unsigned char *)0x8005a3 &= (~0x40) )   //PE6
		#define DBG_CHN6_HIGH		( *(unsigned char *)0x8005a3 |= 0x40 )
		#define DBG_CHN6_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x40 )
		#define DBG_CHN7_LOW		( *(unsigned char *)0x8005a3 &= (~0x10) )   //PE4
		#define DBG_CHN7_HIGH		( *(unsigned char *)0x8005a3 |= 0x10 )
		#define DBG_CHN7_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x10 )
	#else  //8261/8267/8269
		//ch0-ch7: A7 A4 A3 E0 A1 A0 E1 D3
		#define PA0_INPUT_ENABLE					0
		#define PA1_INPUT_ENABLE					0
		#define PA3_INPUT_ENABLE					0
		#define PA4_INPUT_ENABLE					0
		#define PA7_INPUT_ENABLE					0
		#define PD3_INPUT_ENABLE					0
		#define PE0_INPUT_ENABLE					0
		#define PE1_INPUT_ENABLE					0
		#define PA0_OUTPUT_ENABLE					1
		#define PA1_OUTPUT_ENABLE					1
		#define PA3_OUTPUT_ENABLE					1
		#define PA4_OUTPUT_ENABLE					1
		#define PA7_OUTPUT_ENABLE					1
		#define PD3_OUTPUT_ENABLE					1
		#define PE0_OUTPUT_ENABLE					1
		#define PE1_OUTPUT_ENABLE					1

		#define DBG_CHN0_LOW		( *(unsigned char *)0x800583 &= (~0x80) )   //PA7
		#define DBG_CHN0_HIGH		( *(unsigned char *)0x800583 |= 0x80 )
		#define DBG_CHN0_TOGGLE		( *(unsigned char *)0x800583 ^= 0x80 )
		#define DBG_CHN1_LOW		( *(unsigned char *)0x800583 &= (~0x10) )   //PA4
		#define DBG_CHN1_HIGH		( *(unsigned char *)0x800583 |= 0x10 )
		#define DBG_CHN1_TOGGLE		( *(unsigned char *)0x800583 ^= 0x10 )
		#define DBG_CHN2_LOW		( *(unsigned char *)0x800583 &= (~0x08) )   //PA3
		#define DBG_CHN2_HIGH		( *(unsigned char *)0x800583 |= 0x08 )
		#define DBG_CHN2_TOGGLE		( *(unsigned char *)0x800583 ^= 0x08 )
		#define DBG_CHN3_LOW		( *(unsigned char *)0x8005a3 &= (~0x01) )   //PE0
		#define DBG_CHN3_HIGH		( *(unsigned char *)0x8005a3 |= 0x01 )
		#define DBG_CHN3_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x01 )
		#define DBG_CHN4_LOW		( *(unsigned char *)0x800583 &= (~0x02) )   //PA1
		#define DBG_CHN4_HIGH		( *(unsigned char *)0x800583 |= 0x02 )
		#define DBG_CHN4_TOGGLE		( *(unsigned char *)0x800583 ^= 0x02 )
		#define DBG_CHN5_LOW		( *(unsigned char *)0x800583 &= (~0x01) )   //PA0
		#define DBG_CHN5_HIGH		( *(unsigned char *)0x800583 |= 0x01 )
		#define DBG_CHN5_TOGGLE		( *(unsigned char *)0x800583 ^= 0x01 )
		#define DBG_CHN6_LOW		( *(unsigned char *)0x8005a3 &= (~0x02) )   //PE1
		#define DBG_CHN6_HIGH		( *(unsigned char *)0x8005a3 |= 0x02 )
		#define DBG_CHN6_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x02 )
		#define DBG_CHN7_LOW		( *(unsigned char *)0x80059b &= (~0x08) )   //PD3
		#define DBG_CHN7_HIGH		( *(unsigned char *)0x80059b |= 0x08 )
		#define DBG_CHN7_TOGGLE		( *(unsigned char *)0x80059b ^= 0x08 )
	#endif
#else
	#define DBG_CHN0_LOW
	#define DBG_CHN0_HIGH
	#define DBG_CHN0_TOGGLE
	#define DBG_CHN1_LOW
	#define DBG_CHN1_HIGH
	#define DBG_CHN1_TOGGLE
	#define DBG_CHN2_LOW
	#define DBG_CHN2_HIGH
	#define DBG_CHN2_TOGGLE
	#define DBG_CHN3_LOW
	#define DBG_CHN3_HIGH
	#define DBG_CHN3_TOGGLE
	#define DBG_CHN4_LOW
	#define DBG_CHN4_HIGH
	#define DBG_CHN4_TOGGLE
	#define DBG_CHN5_LOW
	#define DBG_CHN5_HIGH
	#define DBG_CHN5_TOGGLE
	#define DBG_CHN6_LOW
	#define DBG_CHN6_HIGH
	#define DBG_CHN6_TOGGLE
	#define DBG_CHN7_LOW
	#define DBG_CHN7_HIGH
	#define DBG_CHN7_TOGGLE
#endif  //end of DEBUG_GPIO_ENABLE






/////////////////// set default   ////////////////

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
