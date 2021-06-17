/********************************************************************************************************
 * @file     mesh_config.h 
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

#include "../user_app/user_app_config.h"

//------------hardware parameters
// HCI Select
#define PROXY_HCI_GATT			1
#define PROXY_HCI_USB			2
#define PROXY_HCI_SEL			PROXY_HCI_GATT

#define GATEWAY_MODEL_PLUS_EN   ((!WIN32) && __PROJECT_MESH_PRO__ && (CHIP_TYPE != CHIP_TYPE_8269))

#if (PROXY_HCI_SEL == PROXY_HCI_GATT)
    #if (__DEBUG_MESH_DONGLE_IN_VC__)
#define DEBUG_MESH_DONGLE_IN_VC_EN		1		// must 1
    #else
        #if WIN32
#define DEBUG_MESH_DONGLE_IN_VC_EN		0
        #else
#define DEBUG_MESH_DONGLE_IN_VC_EN		0
        #endif
    #endif
#else
#define DEBUG_MESH_DONGLE_IN_VC_EN		0		// must 0
#endif

#define DEBUG_CFG_CMD_GROUP_AK_EN		0       // not for user
#define DEBUG_CFG_CMD_GROUP_USE_AK(addr)    (DEBUG_CFG_CMD_GROUP_AK_EN && (addr & 0x8000))

#if(DEBUG_CFG_CMD_GROUP_AK_EN)
#define VC_NODE_INFO_MULTI_SECTOR_EN	1		
#else
#define VC_NODE_INFO_MULTI_SECTOR_EN	0		// use 2 or more sector to save VC_node_info(default use one sector, max support 200 nodes)
#endif

#define SHOW_VC_SELF_NO_NW_ENC		1
#define SHOW_VC_SELF_NW_ENC			2	// can not send reliable cmd with segment, such as netkey add,...

#if (WIN32 && (0 == DEBUG_MESH_DONGLE_IN_VC_EN))
#define NODE_CAN_SEND_ADV_FLAG			0
#define DEBUG_SHOW_VC_SELF_EN			0 // SHOW_VC_SELF_NO_NW_ENC // SHOW_VC_SELF_NW_ENC
#define VC_CHECK_NEXT_SEGMENT_EN		1		// must 1
#else
#define NODE_CAN_SEND_ADV_FLAG			1		// must 1
#define DEBUG_SHOW_VC_SELF_EN			0		// must 0
#define VC_CHECK_NEXT_SEGMENT_EN		0		// must 0
#endif

#define MESH_DLE_MODE_NONE              0
#define MESH_DLE_MODE_GATT              1       // only GATT, mesh unsegment size will still be 11 bytes.
#define MESH_DLE_MODE_EXTEND_BEAR       2       // mesh unsegment size will depend on DLE length.
//------------ project define-------------
#define PROJECT_VC_DONGLE 	1
#define PROJECT_VC_MESH 	2
#define PROJECT_VC_MAX 		10	// VC project max
#define PROJECT_FW_MESH		11	//FW start from 11

#if WIN32
#define PROJECT_SEL			PROJECT_VC_DONGLE // PROJECT_VC_MESH // 
#else
#define PROJECT_SEL			PROJECT_FW_MESH
#endif

#define IS_VC_PROJECT			(PROJECT_SEL <= PROJECT_VC_MAX)
#define IS_VC_PROJECT_MASTER	(IS_VC_PROJECT && (!DEBUG_MESH_DONGLE_IN_VC_EN))

#define PROXY_GATT_WITH_HEAD ((DEBUG_MESH_DONGLE_IN_VC_EN)||(PROXY_HCI_SEL == PROXY_HCI_USB))
#if WIN32
#define 	__PROJECT_MESH_PRO__	 1
#define FAST_PROVISION_ENABLE		 1
#else
#define FAST_PROVISION_ENABLE		 0
#endif

#define BEACON_ENABLE			0
//ms, should be multiple of 10
#define BEACON_INTERVAL			100



#define ATT_TAB_SWITCH_ENABLE 				1
#if(__PROJECT_MESH__)
#define ATT_REPLACE_PROXY_SERVICE_EN		0 // 1:0x7fdd will not existed in att
#endif

#if WIN32
#define TESTCASE_FLAG_ENABLE 		0	// must 0
#define PTS_TEST_EN                 0   // must 0
#else
#define TESTCASE_FLAG_ENABLE 		0
    #if (0 == TESTCASE_FLAG_ENABLE)
#define PTS_TEST_EN                 0
    #else
#define PTS_TEST_EN                 1
    #endif
#define DEBUG_EVB_EN 		        0   // just telink internal used, don't modity
#endif

#define PTS_TEST_OTA_EN             (PTS_TEST_EN || 0)

#if __PROJECT_MESH_LPN__
#define LPN_VENDOR_SENSOR_EN        0
#else
#define LPN_VENDOR_SENSOR_EN        0
#endif

#if (__PROJECT_MESH__)
#define GATT_LPN_EN					0   // only mesh and lpn project can enable
#elif(__PROJECT_MESH_LPN__)
#define GATT_LPN_EN					0   // only mesh and lpn project can enable
#endif

//------------ mesh config-------------
#define MD_CFG_CLIENT_EN            (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__ || TESTCASE_FLAG_ENABLE)   // don't modify
#define RELIABLE_CMD_EN             (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)   // don't modify

//------------ mesh config (user can config)-------------
#define MESH_NORMAL_MODE		0
#define MESH_CLOUD_ENABLE		1
#define MESH_SPIRIT_ENABLE		2// use this mode should burn in the para in 0x78000,or use init para should enable the  con_sec_data
#define MESH_AES_ENABLE 		3
#define MESH_GN_ENABLE 		    4
#define MESH_MI_ENABLE          5
#define MESH_MI_SPIRIT_ENABLE   6   // dual vendor
#define MESH_IRONMAN_MENLO_ENABLE   7   // inclue boot_loader.bin and light.bin
#define MESH_ZB_BL_DUAL_ENABLE      8   // mesh && zigbee normal dual mode with bootloader
#define MESH_PIPA_ENABLE        9   // 
#define MESH_TAIBAI_ENABLE			10

#ifndef MESH_USER_DEFINE_MODE
#if __PROJECT_MESH_PRO__
#define MESH_USER_DEFINE_MODE 	MESH_NORMAL_MODE // must normal
#elif __PROJECT_SPIRIT_LPN__
#define MESH_USER_DEFINE_MODE 	MESH_SPIRIT_ENABLE // must spirit
#else
#define MESH_USER_DEFINE_MODE 	MESH_NORMAL_MODE
#endif
#endif

// vendor id list
#define SHA256_BLE_MESH_PID		0x01A8
#define VENDOR_ID_MI		    0x038F

// mi product type 
#define MI_PRODUCT_TYPE_CT_LIGHT		0x01
#define MI_PRODUCT_TYPE_LAMP			0x02


#define MI_PRODUCT_TYPE_ONE_ONOFF		0x11
#define MI_PRODUCT_TYPE_TWO_ONOFF		0x12
#define MI_PRODUCT_TYPE_THREE_ONOFF		0x13

#define MI_PRODUCT_TYPE_PLUG			0x21
#define MI_PRODUCT_TYPE_FANS			0x30

// SHARE subscription list and publish address
#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
#define SUBSCRIPTION_SHARE_EN		1
#define VENDOR_ID 					SHA256_BLE_MESH_PID
#define AIS_ENABLE					1
#define PROVISION_FLOW_SIMPLE_EN    1
#define ALI_MD_TIME_EN				0
#define ALI_NEW_PROTO_EN			0
#elif(MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define SUBSCRIPTION_SHARE_EN		1
#define VENDOR_ID 					0x011c
#define AIS_ENABLE					1
#define PROVISION_FLOW_SIMPLE_EN    1
#define ALI_MD_TIME_EN				0
#define ALI_NEW_PROTO_EN			0
#define DU_FW_MAIN_VER	1
#define DU_FW_ADD_VER	0
#define DU_FW_INFO_VER	0
#define DU_FW_VER	((DU_FW_MAIN_VER<<12)|(DU_FW_ADD_VER<<8)|(DU_FW_INFO_VER))


#elif(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
#define SUBSCRIPTION_SHARE_EN		1
#define VENDOR_ID 					SHA256_BLE_MESH_PID
#define AIS_ENABLE					1
#define PROVISION_FLOW_SIMPLE_EN    1
#elif(MESH_USER_DEFINE_MODE == MESH_GN_ENABLE)
#define SUBSCRIPTION_SHARE_EN		1
#define AIS_ENABLE					0
#define PROVISION_FLOW_SIMPLE_EN    1
#elif(MESH_USER_DEFINE_MODE == MESH_MI_ENABLE)
#define SUBSCRIPTION_SHARE_EN		1
#define AIS_ENABLE					0
#define PROVISION_FLOW_SIMPLE_EN    0
#define MI_API_ENABLE               1
#define MI_SWITCH_LPN_EN			0   // only support 825x serials 
#define STEP_PUB_MODE_EN 			1
	#if MI_SWITCH_LPN_EN
#define BLT_SOFTWARE_TIMER_ENABLE	1
#define MI_PRODUCT_TYPE				MI_PRODUCT_TYPE_ONE_ONOFF
	#else
#define MI_PRODUCT_TYPE				MI_PRODUCT_TYPE_CT_LIGHT	
	#endif
#elif(MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
#define STEP_PUB_MODE_EN 			1
#define SUBSCRIPTION_SHARE_EN		1
#define VENDOR_ID                   VENDOR_ID_MI // use mi vendor_id and mi mac by default
#define AIS_ENABLE					1
#define PROVISION_FLOW_SIMPLE_EN    1	
#define ALI_MD_TIME_EN				0
#define MI_API_ENABLE               1
#define MI_SWITCH_LPN_EN			0   // only support 825x serials 
	#if MI_SWITCH_LPN_EN
#define BLT_SOFTWARE_TIMER_ENABLE	1
#define STEP_PUB_MODE_EN 			1
#define MI_PRODUCT_TYPE				MI_PRODUCT_TYPE_ONE_ONOFF
	#else
#define MI_PRODUCT_TYPE				MI_PRODUCT_TYPE_CT_LIGHT	
	#endif
#elif((MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)||(MESH_USER_DEFINE_MODE == MESH_ZB_BL_DUAL_ENABLE))
#define SUBSCRIPTION_SHARE_EN		1
#define AIS_ENABLE					0
#define PROVISION_FLOW_SIMPLE_EN    0
    #if __PROJECT_BOOTLOADER__
#define FW_START_BY_BOOTLOADER_EN   0
    #else
#define FW_START_BY_BOOTLOADER_EN   1
    #endif
#define DUAL_MESH_ZB_BL_EN          1
#elif (DEBUG_CFG_CMD_GROUP_AK_EN)
#define SUBSCRIPTION_SHARE_EN		1
#define AIS_ENABLE					0
#define PROVISION_FLOW_SIMPLE_EN    1
#else // (MESH_USER_DEFINE_MODE == MESH_NORMAL_MODE  and all other)
#define SUBSCRIPTION_SHARE_EN		1
#define AIS_ENABLE					0
#define PROVISION_FLOW_SIMPLE_EN    0
#endif

#if (MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define DU_ENABLE 	1
#define DU_LPN_EN	0
	#if DU_LPN_EN
		#define DU_ADV_INTER_MS				1700
		#define DU_ADV_INTER_VAL			(DU_ADV_INTER_MS*1000/625)
		#define BLT_SOFTWARE_TIMER_ENABLE	1
	#endif
#else
#define DU_LPN_EN	0
#define DU_ENABLE 0
#endif

#ifndef MI_API_ENABLE
#define MI_API_ENABLE 0
#endif
#ifndef NL_API_ENABLE
#define NL_API_ENABLE 0
#endif
#ifndef FW_START_BY_BOOTLOADER_EN
#define FW_START_BY_BOOTLOADER_EN  0
#endif
#ifndef DUAL_MESH_ZB_BL_EN
#define DUAL_MESH_ZB_BL_EN  0
#endif


#if MI_API_ENABLE
	#if MCU_CORE_TYPE == MCU_CORE_8269
	#define BLT_SOFTWARE_TIMER_ENABLE	0
	#endif
	#ifndef BLT_SOFTWARE_TIMER_ENABLE
	#define BLT_SOFTWARE_TIMER_ENABLE 	1
	#endif
#endif


#if __PROJECT_SPIRIT_LPN__
#define SPIRIT_PRIVATE_LPN_EN		1//must
#define SPIRIT_VENDOR_EN			1//must
#else
#define SPIRIT_PRIVATE_LPN_EN		0
#if((MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE) ||\
	(MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)||\
	(MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE))
#define SPIRIT_VENDOR_EN			1
#else
#define SPIRIT_VENDOR_EN			0 //must
#endif
#endif

#if (MESH_USER_DEFINE_MODE == MESH_PIPA_ENABLE)
#define DUAL_OTA_NEED_LOGIN_EN      1
#define ENCODE_OTA_BIN_EN           1
#endif

#ifndef MI_SWITCH_LPN_EN
#define MI_SWITCH_LPN_EN 0
#endif

#define VENDOR_MD_MI_EN             (MI_API_ENABLE)
#define VENDOR_MD_NORMAL_EN         ((!MI_API_ENABLE) || AIS_ENABLE)    // include ali

#define DUAL_VENDOR_EN              (VENDOR_MD_MI_EN && VENDOR_MD_NORMAL_EN)

#define DRAFT_FEATURE_VENDOR_TYPE_NONE          0
#define DRAFT_FEATURE_VENDOR_TYPE_ONE_OP        1   // only use "C0"
#define DRAFT_FEATURE_VENDOR_TYPE_MULTY_OP      2

#if (WIN32)
#define DEBUG_VENDOR_CMD_EN 		    0		// must 0, if 1, debug vendor id as server node.
#define DRAFT_FEATURE_VENDOR_TYPE_SEL   DRAFT_FEATURE_VENDOR_TYPE_NONE
#define VC_SUPPORT_ANY_VENDOR_CMD_EN	(!DEBUG_VENDOR_CMD_EN && (DRAFT_FEATURE_VENDOR_TYPE_SEL == DRAFT_FEATURE_VENDOR_TYPE_NONE))
#else
#define DRAFT_FEATURE_VENDOR_TYPE_SEL   DRAFT_FEATURE_VENDOR_TYPE_NONE
    #if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define DEBUG_VENDOR_CMD_EN 		    0       // must 0.
    #else
        #if (DRAFT_FEATURE_VENDOR_TYPE_SEL != DRAFT_FEATURE_VENDOR_TYPE_NONE)
#define DEBUG_VENDOR_CMD_EN 		    0       // must 0. because they use the same op code.
        #else
#define DEBUG_VENDOR_CMD_EN 		    1
        #endif
    #endif
#define VC_SUPPORT_ANY_VENDOR_CMD_EN	__PROJECT_MESH_PRO__		
#endif

#define DRAFT_FEAT_VD_MD_EN             (DRAFT_FEATURE_VENDOR_TYPE_SEL == DRAFT_FEATURE_VENDOR_TYPE_MULTY_OP)

#define LIGHT_TYPE_NONE				0
#define LIGHT_TYPE_CT				1
#define LIGHT_TYPE_HSL				2
#define LIGHT_TYPE_XYL				3
#define LIGHT_TYPE_POWER			4
#define LIGHT_TYPE_CT_HSL			5
#define LIGHT_TYPE_DIM			    6   // only single PWM
#define LIGHT_TYPE_PANEL			7   // only ONOFF model
#define LIGHT_TYPE_LPN_ONOFF_LEVEL  8   // only ONOFF , LEVEL model
#define TYPE_TOOTH_BRUSH			9

/*LIGHT_TYPE_SEL   means instance type select*/
#ifndef LIGHT_TYPE_SEL  // user can define in user_app_config.h
#if (WIN32 || GATEWAY_MODEL_PLUS_EN) // __PROJECT_MESH_PRO__
#define LIGHT_TYPE_SEL				LIGHT_TYPE_CT_HSL  // for APP and gateway
#elif __PROJECT_MESH_LPN__
#define LIGHT_TYPE_SEL				LIGHT_TYPE_LPN_ONOFF_LEVEL // LIGHT_TYPE_CT
#elif __PROJECT_MESH_GW_NODE_HK__
#define LIGHT_TYPE_SEL				LIGHT_TYPE_HSL
#elif GATT_LPN_EN
#define LIGHT_TYPE_SEL 				LIGHT_TYPE_LPN_ONOFF_LEVEL
#else
	#if MI_API_ENABLE
		#if MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT
#define LIGHT_TYPE_SEL				LIGHT_TYPE_CT
		#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP
#define LIGHT_TYPE_SEL				LIGHT_TYPE_CT		
		#elif (	MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF ||\
				MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF ||\
				MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
#define LIGHT_TYPE_SEL				LIGHT_TYPE_PANEL
		#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_PLUG)
#define LIGHT_TYPE_SEL				LIGHT_TYPE_PANEL
		#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
#define LIGHT_TYPE_SEL				LIGHT_TYPE_PANEL
		#endif
	#else
#define LIGHT_TYPE_SEL				LIGHT_TYPE_CT	// 
	#endif
#endif
#endif

#ifndef LIGHT_TYPE_CT_EN
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_CT) || (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
#define LIGHT_TYPE_CT_EN            1
#else
#define LIGHT_TYPE_CT_EN            0
#endif
#endif

#ifndef LIGHT_TYPE_HSL_EN
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL) || (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
#define LIGHT_TYPE_HSL_EN           1
#else
#define LIGHT_TYPE_HSL_EN           0
#endif
#endif

#if ((LIGHT_TYPE_CT_EN) || (LIGHT_TYPE_HSL_EN) || (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL))
    #if((WIN32) || (TESTCASE_FLAG_ENABLE))
#define MD_LIGHT_CONTROL_EN			1	// must 1
    #else
#define MD_LIGHT_CONTROL_EN			0
    #endif
#else
#define MD_LIGHT_CONTROL_EN			0	// must 0
#endif

#if ((LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL) || (LIGHT_TYPE_SEL == TYPE_TOOTH_BRUSH))
#define MD_LIGHTNESS_EN             0
#define MD_LEVEL_EN                 0
#elif (LIGHT_TYPE_SEL == LIGHT_TYPE_LPN_ONOFF_LEVEL)
#define MD_LIGHTNESS_EN             0
#define MD_LEVEL_EN                 1
#else
#define MD_LIGHTNESS_EN             1
#define MD_LEVEL_EN                 1   // must 1
#endif

#if(DUAL_VENDOR_EN)
#define CMD_LINEAR_EN               0
#else
#define CMD_LINEAR_EN               1
#endif

#define MESH_RX_TEST	((0 || DEBUG_CFG_CMD_GROUP_AK_EN) &&(!WIN32))
#define MESH_DELAY_TEST_EN		0

#if WIN32
	#define CERTIFY_BASE_ENABLE 	1
#else
	#if MI_API_ENABLE
	#define CERTIFY_BASE_ENABLE 	0
	#else
	#define CERTIFY_BASE_ENABLE		0
	#endif
#endif

#ifndef MD_MESH_OTA_EN
#if (DEBUG_CFG_CMD_GROUP_AK_EN || PTS_TEST_OTA_EN)
#define MD_MESH_OTA_EN				1	// just for internal test.
#elif (__PROJECT_MESH_PRO__)   // app & gateway
    #if WIN32
#define MD_MESH_OTA_EN				1
    #else // gateway
#define MD_MESH_OTA_EN				0   // dufault disable before released by SIG.
    #endif
#else
	#if ((MESH_USER_DEFINE_MODE == MESH_MI_ENABLE) || (LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL) || __PROJECT_MESH_LPN__ || SPIRIT_PRIVATE_LPN_EN || (LIGHT_TYPE_SEL == TYPE_TOOTH_BRUSH))
#define MD_MESH_OTA_EN				0   // must 0
    #elif (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define MD_MESH_OTA_EN				0	// 
    #elif (AIS_ENABLE)
#define MD_MESH_OTA_EN				0	// default disable
	#else
#define MD_MESH_OTA_EN				0   // dufault disable before released by SIG.
	#endif
#endif
#endif

#if (MD_MESH_OTA_EN && (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__))
#define DISTRIBUTOR_UPDATE_CLIENT_EN    1
#else
#define DISTRIBUTOR_UPDATE_CLIENT_EN    0
#endif

#define MD_ONOFF_EN                 1   // must 1
#define SENSOR_LIGHTING_CTRL_EN     0

#if SENSOR_LIGHTING_CTRL_EN
#define SENSOR_GPIO_PIN             GPIO_PD5
#define SENSOR_LIGHTING_CTRL_ON_MS  10000       // ms
#endif

#if WIN32
#define MD_DEF_TRANSIT_TIME_EN      1   // must 1
#define MD_POWER_ONOFF_EN           1   // must 1
#define MD_TIME_EN                  1   // must 1
#define MD_SCENE_EN                 1   // must 1
#define MD_SCHEDULE_EN              1   // must 1
#define MD_PROPERTY_EN				1	
#define	MD_LOCATION_EN				1	// 
#define MD_BATTERY_EN				1
#define MD_DF_EN					1
#define MD_SBR_EN					1
#if DEBUG_SHOW_VC_SELF_EN
#define MD_SERVER_EN                1   // SIG and vendor MD
#else
#define MD_SERVER_EN                0   // SIG and vendor MD
#endif
#define MD_CLIENT_EN                1   // just SIG MD
#define MD_CLIENT_VENDOR_EN         1
#define MD_VENDOR_2ND_EN            (DEBUG_VENDOR_CMD_EN && MI_API_ENABLE)
#define MD_SENSOR_SERVER_EN			MD_SERVER_EN	
#define MD_SENSOR_CLIENT_EN		    MD_CLIENT_EN	
#elif(1 == TESTCASE_FLAG_ENABLE)
#define MD_DEF_TRANSIT_TIME_EN      1  
#define MD_POWER_ONOFF_EN           1   
#define MD_TIME_EN                  1  
#define MD_SCENE_EN                 1   
#define MD_SCHEDULE_EN              1   
#define MD_PROPERTY_EN				0
#define	MD_LOCATION_EN				0	// location,sensor,battery,property use same flash addr, but one sector max store 6 models			
#define MD_BATTERY_EN				0
#define MD_DF_EN					0
#define MD_SBR_EN					0

#define MD_SERVER_EN                1   // SIG and vendor MD
#define MD_CLIENT_EN                1   // just SIG MD
#define MD_CLIENT_VENDOR_EN         1
    #if __PROJECT_MESH_PRO__
#define MD_SENSOR_SERVER_EN			0	
#define MD_SENSOR_CLIENT_EN		    0	
    #else
#define MD_SENSOR_SERVER_EN			MD_SERVER_EN	
#define MD_SENSOR_CLIENT_EN		    MD_CLIENT_EN	
    #endif
#elif(__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)     // GATEWAY
	#if MCU_CORE_TYPE == MCU_CORE_8269 // ram is not enough ,it will have different settings for 69 and 5x .
#define MD_DEF_TRANSIT_TIME_EN      1
#define MD_POWER_ONOFF_EN           MD_DEF_TRANSIT_TIME_EN 	// because both of them save in same flash sector.
#define MD_TIME_EN                  0
#define MD_SCENE_EN                 0
#define MD_SCHEDULE_EN              MD_TIME_EN  // because both of them save in same flash sector.
#define MD_PROPERTY_EN				0
#define	MD_LOCATION_EN				0	// location,sensor,battery use same flash addr, but one sector max store 6 models
#define MD_BATTERY_EN				0
		#if(__PROJECT_MESH_GW_NODE__)
#define MD_SERVER_EN                1  
#define MD_CLIENT_EN                1 
		#else
#define MD_SERVER_EN                0   // SIG and vendor MD
#define MD_CLIENT_EN                (!MD_SERVER_EN) // just SIG MD
		#endif
#define MD_CLIENT_VENDOR_EN         0
#define MD_VENDOR_2ND_EN            (DEBUG_VENDOR_CMD_EN && MI_API_ENABLE)
#define MD_SENSOR_SERVER_EN			0	
#define MD_SENSOR_CLIENT_EN		    0	
	#else // core type is 825x serial ,and the ram is enough .
#define MD_DEF_TRANSIT_TIME_EN      1
#define MD_POWER_ONOFF_EN           MD_DEF_TRANSIT_TIME_EN 	// because both of them save in same flash sector.
#define MD_TIME_EN                  1
#define MD_SCENE_EN                 1
#define MD_SCHEDULE_EN              MD_TIME_EN  // because both of them save in same flash sector.
#define MD_PROPERTY_EN				0
#define	MD_LOCATION_EN				0	// location,sensor,battery use same flash addr, but one sector max store 6 models
#define MD_BATTERY_EN				0
#define MD_DF_EN					0
#define MD_SBR_EN					0

		#if(__PROJECT_MESH_GW_NODE__)
#define MD_SERVER_EN                1  
#define MD_CLIENT_EN                1 
		#else
#define MD_SERVER_EN                0   // SIG and vendor MD
#define MD_CLIENT_EN                (!MD_SERVER_EN) // just SIG MD
		#endif
#define MD_CLIENT_VENDOR_EN         1
#define MD_VENDOR_2ND_EN            (DEBUG_VENDOR_CMD_EN && MI_API_ENABLE)
#define MD_SENSOR_SERVER_EN			MD_SERVER_EN	// 0
#define MD_SENSOR_CLIENT_EN		    MD_CLIENT_EN	// 0
	#endif
#else                           // light node
    #if ((LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL) || (LIGHT_TYPE_SEL == LIGHT_TYPE_LPN_ONOFF_LEVEL) \
        || DUAL_VENDOR_EN || (LIGHT_TYPE_SEL == TYPE_TOOTH_BRUSH))
#define MD_DEF_TRANSIT_TIME_EN      0
    #else
#define MD_DEF_TRANSIT_TIME_EN      1
    #endif
    #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define MD_POWER_ONOFF_EN           0						// compatible with older version
    #else
#define MD_POWER_ONOFF_EN           MD_DEF_TRANSIT_TIME_EN 	// because both of them save in same flash sector.
    #endif
#define MD_TIME_EN                  0
#define MD_SCENE_EN                 0
#define MD_SCHEDULE_EN              MD_TIME_EN  // because both of them save in same flash sector.
#define MD_PROPERTY_EN				0
#define	MD_LOCATION_EN				0	// location,sensor,battery use same flash addr, but one sector max store 6 models
#define MD_BATTERY_EN				0
#define MD_DF_EN					0
#define MD_SBR_EN					0

#define MD_SERVER_EN                1   // SIG and vendor MD
#define MD_CLIENT_EN                0   // just SIG MD
    #if(DUAL_VENDOR_EN)
#define MD_CLIENT_VENDOR_EN         0
    #elif ((LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL) || SPIRIT_VENDOR_EN)
#define MD_CLIENT_VENDOR_EN         1
    #else
#define MD_CLIENT_VENDOR_EN         0
    #endif
#define MD_VENDOR_2ND_EN            (DEBUG_VENDOR_CMD_EN && MI_API_ENABLE)
#define MD_SENSOR_SERVER_EN			0	// MD_SERVER_EN
    #if MD_LIGHT_CONTROL_EN
#define MD_SENSOR_CLIENT_EN		    1	// must 1
    #else
#define MD_SENSOR_CLIENT_EN		    0	// MD_CLIENT_EN
    #endif
#endif

#if WIN32
#define MD_PRIVACY_BEA			1
#define PRIVATE_PROXY_FUN_EN	1
#else
#define MD_PRIVACY_BEA			0
#define PRIVATE_PROXY_FUN_EN	0
#endif

#ifndef MD_REMOTE_PROV
#if (WIN32)
#define MD_REMOTE_PROV              1
#elif (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
#define MD_REMOTE_PROV              0   // default disable
#elif (MI_API_ENABLE)
#define MD_REMOTE_PROV              0   // must 0
#elif (__PROJECT_MESH__)
#define MD_REMOTE_PROV              0   // dufault disable before released by SIG.
#else
#define MD_REMOTE_PROV              0   // must 0, only project_mesh support now. dufault disable before released by SIG.
#endif
#endif

#define MD_SENSOR_EN	    (MD_SENSOR_SERVER_EN || MD_SENSOR_CLIENT_EN)


#define STRUCT_MD_TIME_SCHEDULE_EN          (MD_TIME_EN || MD_SCHEDULE_EN)
#define STRUCT_MD_DEF_TRANSIT_TIME_POWER_ONOFF_EN   (MD_DEF_TRANSIT_TIME_EN || MD_POWER_ONOFF_EN)

#define FACTORY_TEST_MODE_ENABLE    1
#define MANUAL_FACTORY_RESET_TX_STATUS_EN       0
#define KEEP_ONOFF_STATE_AFTER_OTA	0
#define DF_TEST_MODE_EN  			(0&&MD_DF_EN)

//------------ mesh config(user can config) end -------------

/*ELE_CNT_EVERY_LIGHT means element count of one instance*/
#if (__PROJECT_MESH_PRO__ || (0 == MD_SERVER_EN)) // && (!DEBUG_SHOW_VC_SELF_EN))
#define ELE_CNT_EVERY_LIGHT         1   // APP and gateway use 1 element always,
#else
    #if (LIGHT_TYPE_SEL == LIGHT_TYPE_CT)
        #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define ELE_CNT_EVERY_LIGHT         1   // 2, comfirm later
        #else
#define ELE_CNT_EVERY_LIGHT         2
        #endif
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL)
        #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define ELE_CNT_EVERY_LIGHT         1   // 3, comfirm later
        #else
#define ELE_CNT_EVERY_LIGHT         3
        #endif
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
#define ELE_CNT_EVERY_LIGHT         3
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
        #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define ELE_CNT_EVERY_LIGHT         1   // 4, comfirm later
        #else
#define ELE_CNT_EVERY_LIGHT         4
        #endif
    #else
#define ELE_CNT_EVERY_LIGHT         1
    #endif
#endif

#define DEBUG_VC_FUNCTION			0
#define DEBUG_BLE_EVENT_ENABLE 		0
#define DEBUG_PREINSTALL_APP_KEY_EN 0

#define REFRESH_ST_BY_HW_EN			0

#ifndef MESH_MONITOR_EN
#define MESH_MONITOR_EN 			0
#endif

#if VC_APP_ENABLE ||__PROJECT_MESH_PRO__   
#define MD_BIND_WHITE_LIST_EN 		0
#else
#define MD_BIND_WHITE_LIST_EN 		0
#endif

#if WIN32
// log enable
#define LOG_VC_RX_TEST_EN			0
#define LOG_WRITE_CMD_FIFO_EN		1
#define LOG_PROVISION_EN 			0
#endif

// feture part ,enable or disable to cut down the ram cost 
#if (__PROJECT_MESH_LPN__ && (!WIN32))
#define FEATURE_FRIEND_EN 		0
#define FEATURE_LOWPOWER_EN		1
#define FEATURE_PROV_EN 		1
#define FEATURE_RELAY_EN		0
#define FEATURE_PROXY_EN 		1
#elif (__PROJECT_SPIRIT_LPN__ && (!WIN32))
#define FEATURE_FRIEND_EN 		0
#define FEATURE_LOWPOWER_EN		0
#define FEATURE_PROV_EN 		1
#define FEATURE_RELAY_EN		0
#define FEATURE_PROXY_EN 		1
#elif (MI_SWITCH_LPN_EN &&(!WIN32))
#define FEATURE_FRIEND_EN 		0
#define FEATURE_LOWPOWER_EN		0
#define FEATURE_PROV_EN 		1
#define FEATURE_RELAY_EN		0
#define FEATURE_PROXY_EN 		0
#elif (GATT_LPN_EN &&(!WIN32))
#define FEATURE_FRIEND_EN 		0
#define FEATURE_LOWPOWER_EN		0
#define FEATURE_PROV_EN 		1
#define FEATURE_RELAY_EN		0
#define FEATURE_PROXY_EN 		1
#else
#define FEATURE_FRIEND_EN 		1   // WIN 32 should be suport disable: model_sig_cfg_s.frid
#define FEATURE_LOWPOWER_EN		0
#define FEATURE_PROV_EN 		1
    #if (0 == NODE_CAN_SEND_ADV_FLAG)
#define FEATURE_RELAY_EN		0	// must 0, because it must be proxyed by another node. and messages havee been relay by this node.
#define FEATURE_PROXY_EN 		0	// must 0, 
    #else
#define FEATURE_RELAY_EN		1
#define FEATURE_PROXY_EN 		1
    #endif
#endif

#if (FEATURE_LOWPOWER_EN || SPIRIT_PRIVATE_LPN_EN)
#define MAX_LPN_NUM					1   // must 1
#elif FEATURE_FRIEND_EN
#define MAX_LPN_NUM					2   // should be less than or equal to 16
#define FN_PRIVATE_SEG_CACHE_EN     0   // not push all segments to friend cache at the same time
#else
#define MAX_LPN_NUM					0   // must 0
#endif

#if 1 // for debug
#define DEBUG_PROXY_FRIEND_SHIP		0
#define PROXY_FRIEND_SHIP_MAC_FN	(0x82690014)
#define PROXY_FRIEND_SHIP_MAC_LPN1	(0x82690015)
#define PROXY_FRIEND_SHIP_MAC_LPN2	(0x82690016)

#define DEBUG_IV_UPDATE_TEST_EN     (0)
#endif

#define USER_DEFINE_SET_CCC_ENABLE 	1   // must 1

#define PROXY_PDU_TIMEOUT_TICK 		20*1000*1000

#ifndef SEND_STATUS_WHEN_POWER_ON
#if (WIN32 || __PROJECT_MESH_SWITCH__)
#define SEND_STATUS_WHEN_POWER_ON			0
#elif (DEBUG_MESH_DONGLE_IN_VC_EN && (!IS_VC_PROJECT))
#define SEND_STATUS_WHEN_POWER_ON			0
#else
#define SEND_STATUS_WHEN_POWER_ON			1
#endif
#endif


#if VENDOR_MD_NORMAL_EN
// vendor op mode define
#define VENDOR_OP_MODE_SPIRIT       1
#define VENDOR_OP_MODE_DEFAULT      2

#if (AIS_ENABLE)
#define VENDOR_OP_MODE_SEL          VENDOR_OP_MODE_SPIRIT
#else
#define VENDOR_OP_MODE_SEL          VENDOR_OP_MODE_DEFAULT  // don't modify this setting
#endif
#endif

#if (!__PROJECT_MESH_PRO__)
#define PROV_APP_KEY_SETUP_TIMEOUT_CHECK_EN     0
#endif

// flash save flag 
#define SAVE_FLAG_PRE	(0xAF)
#define SAVE_FLAG       (0xA5)

//------------ mesh keycode-------------
#define	RC_KEY_1_ON			0x01
#define	RC_KEY_1_OFF		0x02
#define	RC_KEY_2_ON			0x03
#define	RC_KEY_2_OFF		0x04
#define	RC_KEY_3_ON			0x05
#define	RC_KEY_3_OFF		0x06
#define	RC_KEY_4_ON			0x07
#define	RC_KEY_4_OFF		0x08
#define	RC_KEY_A_ON			0x09
#define	RC_KEY_A_OFF		0x0a
#define	RC_KEY_UP			0x0b
#define	RC_KEY_DN			0x0c
#define	RC_KEY_L			0x0d
#define	RC_KEY_R			0x0e
#define	RC_KEY_M			0x0f

//-------------CMD
#define REPORT_KEY_ONLY_EN		0

#define			LGT_CMD_LIGHT_ON				0x10
#define			LGT_CMD_LIGHT_OFF				0x11//internal use
#define			LGT_CMD_LUM_UP					0x12//internal use
#define			LGT_CMD_LUM_DOWN				0x13//internal use
#define			LGT_CMD_LEFT_KEY				0x14//internal use
#define			LGT_CMD_RIGHT_KEY				0x15//internal use

#define			IRQ_TIMER1_ENABLE  			    0
#define			IRQ_TIME1_INTERVAL			    (1000) // unit: us
#define			IRQ_GPIO_ENABLE  			    0

#if (!WIN32)
// extend adv should be running on the chip with 64k or more RAM.
#define EXTENDED_ADV_ENABLE				0   // BLE mesh extend ADV
#endif

#if EXTENDED_ADV_ENABLE
#define ADV_RF_LEN_MAX					(255) // (253)
#define ADV_PDU_LEN_MAX					(ADV_RF_LEN_MAX-6-4)    // 6: mac, 4: extend adv head;   // payload
#else
#define ADV_RF_LEN_MAX					(37)
#define ADV_PDU_LEN_MAX					(ADV_RF_LEN_MAX-6)      // 6: mac
#endif


/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
