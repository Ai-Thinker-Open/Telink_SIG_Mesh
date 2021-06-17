/********************************************************************************************************
 * @file     blt_config.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2015
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
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

//////////////////////////////////////////////////////////////////////////////
/**
 *  @brief  Definition for Device info
 */
#include "drivers.h"
#include "proj/tl_common.h"

#define  MAX_DEV_NAME_LEN 				18

#ifndef DEV_NAME
#define DEV_NAME                        "tModule"
#endif

#if 1 // add by weixiong in mesh
/*
 * only 1 can be set
 */
static inline void blc_app_setExternalCrystalCapEnable(u8  en)
{
	blt_miscParam.ext_cap_en = en;

	WriteAnalogReg(0x8a,ReadAnalogReg(0x8a)|0x80);//close internal cap

}

static inline void check_and_set_1p95v_to_zbit_flash()
{
	if(1 == zbit_flash_flag){ // use "== 1"" should be better than "ture"
		analog_write(0x09, ((analog_read(0x09) & 0x8f) | (FLASH_VOLTAGE_1V95 << 4)));	 	//ldo mode flash ldo trim 1.95V
		analog_write(0x0c, ((analog_read(0x0c) & 0xf8) | FLASH_VOLTAGE_1V9));				//dcdc mode flash ldo trim 1.90V
	}
}


static inline void blc_app_loadCustomizedParameters(void)
{
	 if(!blt_miscParam.ext_cap_en)
	 {
		 //customize freq_offset adjust cap value, if not customized, default ana_81 is 0xd0
		 //for 512K Flash, flash_sector_calibration equals to 0x77000
		 //for 1M  Flash, flash_sector_calibration equals to 0xFE000
		 if(flash_sector_calibration){
			 u8 cap_frqoft = *(unsigned char*) (flash_sector_calibration + CALIB_OFFSET_CAP_INFO);
			 if( cap_frqoft != 0xff ){
				 analog_write(0x8A, (analog_read(0x8A) & 0xc0)|(cap_frqoft & 0x3f));
			 }
		 }
	 }


	if(!pm_is_MCU_deepRetentionWakeup()){
		zbit_flash_flag = flash_is_zb();
	}

	u16 calib_value = *(unsigned short*)(flash_sector_calibration+CALIB_OFFSET_FLASH_VREF);

	if((0xffff == calib_value) || (0 != (calib_value & 0xf8f8)))
	{
		check_and_set_1p95v_to_zbit_flash();
	}
	else
	{
		analog_write(0x09, ((analog_read(0x09) & 0x8f)  | ((calib_value & 0xff00) >> 4) ));
		analog_write(0x0c, ((analog_read(0x0c) & 0xf8)  | (calib_value & 0xff)));
	}

}
#endif




///////////////////  Feature ////////////////////////////


#ifndef SECURE_CONNECTION_ENABLE
#define SECURE_CONNECTION_ENABLE							1
#endif




#if 1
#define		BLS_TELINK_MESH_SCAN_MODE_ENABLE				1
#define 	BLS_TELINK_WHITE_LIST_ENABLE					0
#define 	RAMCODE_OPTIMIZE_CONN_POWER_NEGLECT_ENABLE		1
#define 	DEEP_RET_ENTRY_CONDITIONS_OPTIMIZATION_EN		1
#endif






#ifndef  LL_MASTER_MULTI_CONNECTION
#define  LL_MASTER_MULTI_CONNECTION							0
#endif




#ifndef		BLS_ADV_INTERVAL_CHECK_ENABLE
#define		BLS_ADV_INTERVAL_CHECK_ENABLE					0
#endif

#if 1 // add by weixiong
/////////////////  scan mode config  //////////////////////////
#if LIB_TELINK_MESH_SCAN_MODE_ENABLE
#define		BLS_TELINK_MESH_SCAN_MODE_ENABLE				1
#define 	BLS_TELINK_WHITE_LIST_ENABLE					0
#define 	RAMCODE_OPTIMIZE_CONN_POWER_NEGLECT_ENABLE		1
#endif

#ifndef		BLS_TELINK_MESH_SCAN_MODE_ENABLE
#define		BLS_TELINK_MESH_SCAN_MODE_ENABLE				0
#endif
#endif

#define	BLS_BLE_RF_IRQ_TIMING_EXTREMELY_SHORT_EN			0



//conn param update/map update
#ifndef	BLS_PROC_MASTER_UPDATE_REQ_IN_IRQ_ENABLE
#define BLS_PROC_MASTER_UPDATE_REQ_IN_IRQ_ENABLE			1
#endif


#ifndef LE_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT_EN
#define LE_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT_EN			0
#endif


#ifndef FIX_HW_CRC24_EN
	#if (MCU_CORE_TYPE != MCU_CORE_8278)
		#define	FIX_HW_CRC24_EN										1
	#else
		#define	FIX_HW_CRC24_EN										0
	#endif
#endif


#ifndef DEEP_RET_ENTRY_CONDITIONS_OPTIMIZATION_EN
#define	DEEP_RET_ENTRY_CONDITIONS_OPTIMIZATION_EN			0
#endif


#ifndef HOST_CONTROLLER_DATA_FLOW_IMPROVE_EN
#define	HOST_CONTROLLER_DATA_FLOW_IMPROVE_EN				1
#endif

#if (HOST_CONTROLLER_DATA_FLOW_IMPROVE_EN)
	#define ATT_RSP_BIG_MTU_PROCESS_EN						1
#endif






//Link layer feature enable flag default setting
#ifndef BLE_CORE42_DATA_LENGTH_EXTENSION_ENABLE
#define BLE_CORE42_DATA_LENGTH_EXTENSION_ENABLE						1
#endif

#ifndef LL_FEATURE_SUPPORT_LE_2M_PHY
#define LL_FEATURE_SUPPORT_LE_2M_PHY								1
#endif

#ifndef LL_FEATURE_SUPPORT_LE_CODED_PHY
#define LL_FEATURE_SUPPORT_LE_CODED_PHY								1
#endif

#ifndef LL_FEATURE_SUPPORT_LE_EXTENDED_ADVERTISING
#define LL_FEATURE_SUPPORT_LE_EXTENDED_ADVERTISING					1
#endif

#ifndef LL_FEATURE_SUPPORT_LE_PERIODIC_ADVERTISING
#define LL_FEATURE_SUPPORT_LE_PERIODIC_ADVERTISING					0
#endif

#ifndef LL_FEATURE_SUPPORT_CHANNEL_SELECTION_ALGORITHM2
#define LL_FEATURE_SUPPORT_CHANNEL_SELECTION_ALGORITHM2				1
#endif




#ifndef BQB_LOWER_TESTER_ENABLE
#define BQB_LOWER_TESTER_ENABLE										0
#endif



#ifndef ZBIT_FLASH_WRITE_TIME_LONG_WORKAROUND_EN
#define ZBIT_FLASH_WRITE_TIME_LONG_WORKAROUND_EN					1
#endif

#ifndef ZBIT_FLASH_BRX4B_WRITE__EN
#define ZBIT_FLASH_BRX4B_WRITE__EN									0
#endif


///////////////////////////////////////dbg channels///////////////////////////////////////////
#ifndef	DBG_CHN0_TOGGLE
#define DBG_CHN0_TOGGLE
#endif

#ifndef	DBG_CHN0_HIGH
#define DBG_CHN0_HIGH
#endif

#ifndef	DBG_CHN0_LOW
#define DBG_CHN0_LOW
#endif

#ifndef	DBG_CHN1_TOGGLE
#define DBG_CHN1_TOGGLE
#endif

#ifndef	DBG_CHN1_HIGH
#define DBG_CHN1_HIGH
#endif

#ifndef	DBG_CHN1_LOW
#define DBG_CHN1_LOW
#endif

#ifndef	DBG_CHN2_TOGGLE
#define DBG_CHN2_TOGGLE
#endif

#ifndef	DBG_CHN2_HIGH
#define DBG_CHN2_HIGH
#endif

#ifndef	DBG_CHN2_LOW
#define DBG_CHN2_LOW
#endif

#ifndef	DBG_CHN3_TOGGLE
#define DBG_CHN3_TOGGLE
#endif

#ifndef	DBG_CHN3_HIGH
#define DBG_CHN3_HIGH
#endif

#ifndef	DBG_CHN3_LOW
#define DBG_CHN3_LOW
#endif

#ifndef	DBG_CHN4_TOGGLE
#define DBG_CHN4_TOGGLE
#endif

#ifndef	DBG_CHN4_HIGH
#define DBG_CHN4_HIGH
#endif

#ifndef	DBG_CHN4_LOW
#define DBG_CHN4_LOW
#endif

#ifndef	DBG_CHN5_TOGGLE
#define DBG_CHN5_TOGGLE
#endif

#ifndef	DBG_CHN5_HIGH
#define DBG_CHN5_HIGH
#endif

#ifndef	DBG_CHN5_LOW
#define DBG_CHN5_LOW
#endif

#ifndef	DBG_CHN6_TOGGLE
#define DBG_CHN6_TOGGLE
#endif

#ifndef	DBG_CHN6_HIGH
#define DBG_CHN6_HIGH
#endif

#ifndef	DBG_CHN6_LOW
#define DBG_CHN6_LOW
#endif

#ifndef	DBG_CHN7_TOGGLE
#define DBG_CHN7_TOGGLE
#endif

#ifndef	DBG_CHN7_HIGH
#define DBG_CHN7_HIGH
#endif

#ifndef	DBG_CHN7_LOW
#define DBG_CHN7_LOW
#endif



#ifndef	BLC_REGISTER_DBG_GPIO_IN_STACK
#define BLC_REGISTER_DBG_GPIO_IN_STACK		0
#endif
