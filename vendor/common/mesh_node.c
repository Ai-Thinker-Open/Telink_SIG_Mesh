/********************************************************************************************************
 * @file     mesh_node.c 
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
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "vendor/common/remote_prov.h"
#include "vendor/common/fast_provision_model.h"
#include "vendor/common/app_heartbeat.h"
#include "vendor/common/app_health.h"
#include "vendor/common/directed_forwarding.h"
#include "subnet_bridge.h"
#include "vendor/common/mesh_ota.h"
#include "vendor/common/lighting_model_LC.h"
#include "version.h"
#include "app_privacy_beacon.h"
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/ble/service/ble_ll_ota.h"

#ifdef WIN32
#include "sig_mesh_json_info.h"
#endif

#if (ALI_MD_TIME_EN)
#include "user_ali_time.h"
#endif

#if (DU_ENABLE)
#include "user_du.h"
#endif

#if (__PROJECT_MESH_SWITCH__ && (0 == PM_DEEPSLEEP_RETENTION_ENABLE))
#define MESH_CMD_SNO_SAVE_DELTA             (0x04)	// can't set too much, becaush of deep sleep
#else
#define MESH_CMD_SNO_SAVE_DELTA             (0x80)
#endif

/** @addtogroup Mesh_Common
  * @{
  */
#if EXTENDED_ADV_ENABLE
rf_packet_adv_t	pkt_adv = {	// redefine to replace the weak define in library.
	sizeof (rf_packet_adv_t) - 4,		// dma_len
	{LL_TYPE_ADV_IND, 0, 0, 0, 0},					// type
	sizeof (rf_packet_adv_t) - 6,		// rf_len
	{0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5},	// advA
	// data
};
#endif

/** @defgroup Mesh_Node
  * @brief Mesh Node Code.
  * @{
  */

void app_key_del2(mesh_app_key_t *p_appkey);
void mesh_ota_master_proc();
void check_prov_timeout();


u16 ele_adr_primary = 0;    // save in configure server model.
u8 g_ele_cnt = ELE_CNT;
u8 g_bind_key_max = BIND_KEY_MAX;
u8 key_bind_all_ele_en = 0;
u8 node_need_store_misc=1; 
u32 node_binding_tick = 0;  // key add command also refresh this tick
static u32 mesh_key_addr = FLASH_ADR_MESH_KEY;
static u32 mesh_misc_addr = FLASH_ADR_MISC;
static u32 mesh_adv_tx_cmd_sno_last;


mesh_iv_idx_st_t iv_idx_st = {
    /*.cur = */IV_IDX_CUR,    // store in big endianness
};

mesh_key_t mesh_key = {{0}};  // not init here to decrease firmware size.

#if FEATURE_LOWPOWER_EN
friend_key_t mesh_fri_key_lpn[NET_KEY_MAX][2];
#else
friend_key_t mesh_fri_key_fn[MAX_LPN_NUM][2];
#endif
#if MD_DF_EN
directed_key_t directed_key[NET_KEY_MAX][2];
#endif
mesh_tid_t mesh_tid;
u8 switch_project_flag = 0;

//#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
#if WIN32
u8 my_rf_power_index ;   // win32 use 
#else
u8 my_rf_power_index = MY_RF_POWER_INDEX;   // use in library
u8 tx_bear_extend_en = (MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR);
#endif
//#endif

/*PTS test*/
u8 pts_test_en = PTS_TEST_EN;

#if (FEATURE_LOWPOWER_EN || GATT_LPN_EN)
u8 proxy_message_between_gatt_and_adv_en = 0;
#else
u8 proxy_message_between_gatt_and_adv_en = FEATURE_PROXY_EN;
#endif

MYFIFO_INIT(mesh_adv_cmd_fifo, sizeof(mesh_cmd_bear_unseg_t)+DELTA_EXTEND_AND_NORMAL_ALIGN4_BUF, MESH_ADV_CMD_BUF_CNT);
#if (FEATURE_RELAY_EN || WIN32)
MYFIFO_INIT_NO_RET(mesh_adv_fifo_relay, sizeof(mesh_relay_buf_t)+DELTA_EXTEND_AND_NORMAL_ALIGN4_BUF, MESH_ADV_BUF_RELAY_CNT);
#endif

#if MESH_BLE_NOTIFY_FIFO_EN
#define BLT_NOTIFY_FIFO_SIZE    36  // if need to change, should not use blt_notify_fifo and push blt_txfifo_ directly. 
    
    #if __PROJECT_SPIRIT_LPN__
MYFIFO_INIT_NO_RET(blt_notify_fifo, BLT_NOTIFY_FIFO_SIZE, 64);//save retention
    #elif (FEATURE_LOWPOWER_EN || GATT_LPN_EN)
MYFIFO_INIT(blt_notify_fifo, BLT_NOTIFY_FIFO_SIZE, 8);
    #else
MYFIFO_INIT(blt_notify_fifo, BLT_NOTIFY_FIFO_SIZE, 64);
    #endif
#endif

STATIC_ASSERT(sizeof(mesh_net_key_t) % 16 == 0);
STATIC_ASSERT(sizeof(mesh_app_key_t) % 4 == 0);
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key) == ARRAY_SIZE(mesh_fri_key_lpn));
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key[0]) == ARRAY_SIZE(mesh_fri_key_lpn[0]));
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key[0]) == ARRAY_SIZE(mesh_fri_key_fn[0]));
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key[0]) == ARRAY_SIZE(directed_key[0]));
STATIC_ASSERT(NET0 == 0);
STATIC_ASSERT(ELE_CNT <= ELE_CNT_MAX_LIB);  // don't cancle this assert
STATIC_ASSERT((NET_KEY_MAX <= 255) && (APP_KEY_MAX <= 255));          // only one byte for array index
STATIC_ASSERT(MD_SERVER_EN || MD_CLIENT_EN);
STATIC_ASSERT((BUILD_VERSION & 0x00FF0000) != 0x00A50000);  // because ram[840004] is a special flag which is 0xA5, in cstartup_S.

#if (!WIN32)
#if MESH_DLE_MODE
STATIC_ASSERT(((DLE_LEN_MAX_RX >= MAX_OCTETS_DATA_LEN_EXTENSION)||((DLE_LEN_MAX_RX + 8) % 16 == 0)) && (DLE_LEN_MAX_RX > 40)); // because DLE is 27 if "<=40".
STATIC_ASSERT(((DLE_LEN_MAX_TX >= MAX_OCTETS_DATA_LEN_EXTENSION)||(DLE_LEN_MAX_TX % 4 == 0)) && (DLE_LEN_MAX_TX > 28)); // because DLE is 27 if "<=28".
#endif

#if EXTENDED_ADV_ENABLE
STATIC_ASSERT((MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR) && (DLE_LEN_MAX_RX == MAX_OCTETS_DATA_LEN_EXTENSION));
#endif
#endif

// composition data struct
#if MD_CFG_CLIENT_EN
#define MD_ID_ARRAY_CFG_CLIENT  SIG_MD_CFG_CLIENT,
#else
#define MD_ID_ARRAY_CFG_CLIENT  
#endif

#if MD_REMOTE_PROV
    #if MD_CLIENT_EN
#define MD_ID_REMOTE_PROV_CLIENT    SIG_MD_REMOTE_PROV_CLIENT,
    #endif
    #if MD_SERVER_EN
#define MD_ID_REMOTE_PROV_SERVER    SIG_MD_REMOTE_PROV_SERVER,
    #endif
#endif

#ifndef MD_ID_REMOTE_PROV_SERVER
#define MD_ID_REMOTE_PROV_SERVER    
#endif
#ifndef MD_ID_REMOTE_PROV_CLIENT
#define MD_ID_REMOTE_PROV_CLIENT    
#endif

#define MD_ID_ARRAY_REMOTE_PROV     MD_ID_REMOTE_PROV_SERVER  MD_ID_REMOTE_PROV_CLIENT

#if MD_PRIVACY_BEA
	#if MD_CLIENT_EN
#define MD_ID_PRIVACY_BEA_CLIENT    SIG_MD_PRIVATE_BEACON_CLIENT,
    #endif
    #if MD_SERVER_EN
#define MD_ID_PRIVACY_BEA_SERVER    SIG_MD_PRIVATE_BEACON_SERVER,
    #endif
#endif
#ifndef MD_ID_PRIVACY_BEA_SERVER
#define MD_ID_PRIVACY_BEA_SERVER    
#endif
#ifndef MD_ID_PRIVACY_BEA_CLIENT
#define MD_ID_PRIVACY_BEA_CLIENT    
#endif

#if MD_DF_EN
	#if MD_CLIENT_EN
#define MD_ID_DF_CFG_CLIENT    SIG_MD_DF_CFG_C,
    #endif
    #if MD_SERVER_EN
#define MD_ID_DF_CFG_SERVER    SIG_MD_DF_CFG_S,
    #endif
#endif

#ifndef MD_ID_DF_CFG_SERVER
#define MD_ID_DF_CFG_SERVER
#endif
#ifndef MD_ID_DF_CFG_CLIENT
#define MD_ID_DF_CFG_CLIENT
#endif

#if MD_SBR_EN
	#if MD_CLIENT_EN
#define MD_ID_BRIDGE_CFG_CLIENT    SIG_MD_BRIDGE_CFG_CLIENT,
    #endif
    #if MD_SERVER_EN
#define MD_ID_BRIDGE_CFG_SERVER    SIG_MD_BRIDGE_CFG_SERVER,
    #endif
#endif

#ifndef MD_ID_BRIDGE_CFG_SERVER
#define MD_ID_BRIDGE_CFG_SERVER
#endif
#ifndef MD_ID_BRIDGE_CFG_CLIENT
#define MD_ID_BRIDGE_CFG_CLIENT
#endif

#define MD_ID_ARRAY_CFG         SIG_MD_CFG_SERVER, MD_ID_ARRAY_CFG_CLIENT       \
                                SIG_MD_HEALTH_SERVER, SIG_MD_HEALTH_CLIENT,     \
                                MD_ID_PRIVACY_BEA_SERVER MD_ID_PRIVACY_BEA_CLIENT\
								MD_ID_DF_CFG_SERVER MD_ID_DF_CFG_CLIENT\
								MD_ID_BRIDGE_CFG_SERVER MD_ID_BRIDGE_CFG_CLIENT


#if MD_SERVER_EN
    #if (VENDOR_MD_MI_EN)	// default use vendor mi for dual vendor.
#define MD_ID_ARRAY_VENDOR_SERVER1       MIOT_SEPC_VENDOR_MODEL_SER,
    #elif (VENDOR_MD_NORMAL_EN)
#define MD_ID_ARRAY_VENDOR_SERVER1       VENDOR_MD_LIGHT_S,
    #endif
#else
#define MD_ID_ARRAY_VENDOR_SERVER1       
#endif

#if MD_CLIENT_VENDOR_EN
    #if (VENDOR_MD_MI_EN)
#define MD_ID_ARRAY_VENDOR_CLIENT       MIOT_SEPC_VENDOR_MODEL_CLI,
    #elif (VENDOR_MD_NORMAL_EN)
#define MD_ID_ARRAY_VENDOR_CLIENT       VENDOR_MD_LIGHT_C,
    #endif
#else
#define MD_ID_ARRAY_VENDOR_CLIENT       
#endif

#if MD_VENDOR_2ND_EN
    #if (VENDOR_MD_MI_EN)
#define MD_ID_ARRAY_VENDOR2     MIOT_VENDOR_MD_SER,
    #elif (VENDOR_MD_NORMAL_EN)
#define MD_ID_ARRAY_VENDOR2     VENDOR_MD_LIGHT_S2,
    #endif
#else
#define MD_ID_ARRAY_VENDOR2     
#endif
#define MD_ID_ARRAY_VENDOR_SERVER       MD_ID_ARRAY_VENDOR_SERVER1  MD_ID_ARRAY_VENDOR2

#if MD_MESH_OTA_EN
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
#define MD_ID_ARRAY_MESH_OTA_CLIENT     SIG_MD_FW_DISTRIBUT_C, SIG_MD_FW_DISTRIBUT_S, \
                                        SIG_MD_FW_UPDATE_C, SIG_MD_BLOB_TRANSFER_C,
    #else
#define MD_ID_ARRAY_MESH_OTA_CLIENT  
    #endif
    #if 1 // MD_SERVER_EN gateway also need OTA
#define MD_ID_ARRAY_MESH_OTA_SERVER     SIG_MD_FW_UPDATE_S,SIG_MD_BLOB_TRANSFER_S,
    #else
#define MD_ID_ARRAY_MESH_OTA_SERVER       
    #endif
#define MD_ID_ARRAY_MESH_OTA    MD_ID_ARRAY_MESH_OTA_SERVER  MD_ID_ARRAY_MESH_OTA_CLIENT
                                
#endif

#if MD_SERVER_EN
#define MD_ID_ARRAY_ONOFF_SERVER        SIG_MD_G_ONOFF_S,
#define MD_ID_ARRAY_LEVEL_SERVER        SIG_MD_G_LEVEL_S,
#define MD_ID_ARRAY_DEF_TRANSIT_TIME_SERVER     SIG_MD_G_DEF_TRANSIT_TIME_S,
#define MD_ID_ARRAY_POWER_ONOFF_SERVER  SIG_MD_G_POWER_ONOFF_S, SIG_MD_G_POWER_ONOFF_SETUP_S,
#define MD_ID_ARRAY_TIME_SERVER         SIG_MD_TIME_S, SIG_MD_TIME_SETUP_S,
#define MD_ID_ARRAY_SCENE_SERVER        SIG_MD_SCENE_S, SIG_MD_SCENE_SETUP_S,
#define MD_ID_ARRAY_SCHED_SERVER        SIG_MD_SCHED_S, SIG_MD_SCHED_SETUP_S,
#define MD_ID_ARRAY_POWER_LEVEL_SERVER  SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_SETUP_S,
#define MD_ID_ARRAY_LIGHTNESS_SERVER    SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_SETUP_S,
//#define MD_ID_ARRAY_LIGHT_CTL_SERVER    SIG_MD_LIGHT_CTL_S, XXX,
//#define MD_ID_ARRAY_LIGHT_HSL_SERVER    SIG_MD_LIGHT_HSL_S, XXX
#define MD_ID_ARRAY_LIGHT_XYL_SERVER    SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_SETUP_S,
#define MD_ID_ARRAY_LIGHT_LC_SERVER     SIG_MD_LIGHT_LC_S, SIG_MD_LIGHT_LC_SETUP_S,
#define MD_ID_ARRAY_PROP_SERVER		    SIG_MD_G_ADMIN_PROP_S, SIG_MD_G_MFG_PROP_S, SIG_MD_G_USER_PROP_S, SIG_MD_G_CLIENT_PROP_S,
#define	MD_ID_ARRAY_BATTERY_SERVER		SIG_MD_G_BAT_S,
#define MD_ID_ARRAY_LOCATION_SERVER		SIG_MD_G_LOCATION_S, SIG_MD_G_LOCATION_SETUP_S,
#else
#define MD_ID_ARRAY_ONOFF_SERVER        
#define MD_ID_ARRAY_LEVEL_SERVER        
#define MD_ID_ARRAY_DEF_TRANSIT_TIME_SERVER     
#define MD_ID_ARRAY_POWER_ONOFF_SERVER  
#define MD_ID_ARRAY_TIME_SERVER         
#define MD_ID_ARRAY_SCENE_SERVER        
#define MD_ID_ARRAY_SCHED_SERVER        
#define MD_ID_ARRAY_POWER_LEVEL_SERVER  
#define MD_ID_ARRAY_LIGHTNESS_SERVER    
//#define MD_ID_ARRAY_LIGHT_CTL_SERVER    
//#define MD_ID_ARRAY_LIGHT_HSL_SERVER    
#define MD_ID_ARRAY_LIGHT_XYL_SERVER    
#define MD_ID_ARRAY_LIGHT_LC_SERVER    
#define MD_ID_ARRAY_PROP_SERVER			
#define	MD_ID_ARRAY_BATTERY_SERVER
#define MD_ID_ARRAY_LOCATION_SERVER
#endif

#if MD_CLIENT_EN
#define MD_ID_ARRAY_ONOFF_CLIENT        SIG_MD_G_ONOFF_C,
#define MD_ID_ARRAY_LEVEL_CLIENT        SIG_MD_G_LEVEL_C,
#define MD_ID_ARRAY_DEF_TRANSIT_TIME_CLIENT     SIG_MD_G_DEF_TRANSIT_TIME_C,
#define MD_ID_ARRAY_POWER_ONOFF_CLIENT  SIG_MD_G_POWER_ONOFF_C,
#define MD_ID_ARRAY_TIME_CLIENT         SIG_MD_TIME_C,
#define MD_ID_ARRAY_SCENE_CLIENT        SIG_MD_SCENE_C,
#define MD_ID_ARRAY_SCHED_CLIENT        SIG_MD_SCHED_C,
#define MD_ID_ARRAY_POWER_LEVEL_CLIENT  SIG_MD_G_POWER_LEVEL_C,
#define MD_ID_ARRAY_LIGHTNESS_CLIENT    SIG_MD_LIGHTNESS_C,
#define MD_ID_ARRAY_LIGHT_CTL_CLIENT    SIG_MD_LIGHT_CTL_C,
#define MD_ID_ARRAY_LIGHT_HSL_CLIENT    SIG_MD_LIGHT_HSL_C,
#define MD_ID_ARRAY_LIGHT_XYL_CLIENT    SIG_MD_LIGHT_XYL_C,
#define MD_ID_ARRAY_LIGHT_LC_CLIENT     SIG_MD_LIGHT_LC_C,
#define MD_ID_ARRAY_PROP_CLIENT		    SIG_MD_G_PROP_C,
#define	MD_ID_ARRAY_BATTERY_CLIENT		SIG_MD_G_BAT_C,
#define MD_ID_ARRAY_LOCATION_CLIENT		SIG_MD_G_LOCATION_C,
#else
#define MD_ID_ARRAY_ONOFF_CLIENT        
#define MD_ID_ARRAY_LEVEL_CLIENT        
#define MD_ID_ARRAY_DEF_TRANSIT_TIME_CLIENT     
#define MD_ID_ARRAY_POWER_ONOFF_CLIENT  
#define MD_ID_ARRAY_TIME_CLIENT         
#define MD_ID_ARRAY_SCENE_CLIENT        
#define MD_ID_ARRAY_SCHED_CLIENT        
#define MD_ID_ARRAY_POWER_LEVEL_CLIENT  
#define MD_ID_ARRAY_LIGHTNESS_CLIENT    
#define MD_ID_ARRAY_LIGHT_CTL_CLIENT    
#define MD_ID_ARRAY_LIGHT_HSL_CLIENT    
#define MD_ID_ARRAY_LIGHT_XYL_CLIENT    
#define MD_ID_ARRAY_LIGHT_LC_CLIENT    
#define MD_ID_ARRAY_PROP_CLIENT			
#define	MD_ID_ARRAY_BATTERY_CLIENT
#define MD_ID_ARRAY_LOCATION_CLIENT
#endif

#if MD_SENSOR_SERVER_EN
#define MD_ID_ARRAY_SENSOR_SERVER		SIG_MD_SENSOR_S, SIG_MD_SENSOR_SETUP_S,
#else
#define MD_ID_ARRAY_SENSOR_SERVER		
#endif
#if MD_SENSOR_CLIENT_EN
#define MD_ID_ARRAY_SENSOR_CLIENT		SIG_MD_SENSOR_C,
#else
#define	MD_ID_ARRAY_SENSOR_CLIENT
#endif

#if MD_ONOFF_EN
#define MD_ID_ARRAY_ONOFF       MD_ID_ARRAY_ONOFF_SERVER  MD_ID_ARRAY_ONOFF_CLIENT
#endif
#if MD_LEVEL_EN
#define MD_ID_ARRAY_LEVEL       MD_ID_ARRAY_LEVEL_SERVER  MD_ID_ARRAY_LEVEL_CLIENT
#endif
#if MD_DEF_TRANSIT_TIME_EN
#define MD_ID_ARRAY_DEF_TRANSIT_TIME    MD_ID_ARRAY_DEF_TRANSIT_TIME_SERVER  MD_ID_ARRAY_DEF_TRANSIT_TIME_CLIENT
#endif
#if MD_POWER_ONOFF_EN
#define MD_ID_ARRAY_POWER_ONOFF MD_ID_ARRAY_POWER_ONOFF_SERVER  MD_ID_ARRAY_POWER_ONOFF_CLIENT
#endif
#if MD_TIME_EN
#define MD_ID_ARRAY_TIME        MD_ID_ARRAY_TIME_SERVER  MD_ID_ARRAY_TIME_CLIENT
#endif
#if MD_SCENE_EN
#define MD_ID_ARRAY_SCENE       MD_ID_ARRAY_SCENE_SERVER  MD_ID_ARRAY_SCENE_CLIENT
#endif
#if MD_SCHEDULE_EN
#define MD_ID_ARRAY_SCHED       MD_ID_ARRAY_SCHED_SERVER  MD_ID_ARRAY_SCHED_CLIENT
#endif
#if MD_SENSOR_EN					
#define MD_ID_ARRAY_SENSOR		MD_ID_ARRAY_SENSOR_SERVER  MD_ID_ARRAY_SENSOR_CLIENT
#endif
#if MD_PROPERTY_EN
#define MD_ID_ARRAY_PROPERTY    MD_ID_ARRAY_PROP_SERVER  MD_ID_ARRAY_PROP_CLIENT
#endif
#if MD_BATTERY_EN
#define MD_ID_ARRAY_BATTERY		MD_ID_ARRAY_BATTERY_SERVER  MD_ID_ARRAY_BATTERY_CLIENT
#endif
#if MD_LOCATION_EN					
#define MD_ID_ARRAY_LOCATION	MD_ID_ARRAY_LOCATION_SERVER  MD_ID_ARRAY_LOCATION_CLIENT
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
#define MD_ID_ARRAY_POWER_LEVEL MD_ID_ARRAY_POWER_LEVEL_SERVER  MD_ID_ARRAY_POWER_LEVEL_CLIENT
#elif MD_LIGHTNESS_EN
#define MD_ID_ARRAY_LIGHTNESS   MD_ID_ARRAY_LIGHTNESS_SERVER MD_ID_ARRAY_LIGHTNESS_CLIENT
#endif

#if (LIGHT_TYPE_CT_EN)
    #if (ELE_CNT_EVERY_LIGHT >= 2)
#define MD_CTL_TEMP_S_PRIMARY   
    #else
#define MD_CTL_TEMP_S_PRIMARY   SIG_MD_LIGHT_CTL_TEMP_S, 
    #endif
    #if MD_SERVER_EN
#define MD_ID_ARRAY_LIGHT_CTL_SERVER    SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_SETUP_S, MD_CTL_TEMP_S_PRIMARY
    #else
#define MD_ID_ARRAY_LIGHT_CTL_SERVER    
    #endif
#define MD_ID_ARRAY_LIGHT_CTL   MD_ID_ARRAY_LIGHT_CTL_SERVER  MD_ID_ARRAY_LIGHT_CTL_CLIENT
#endif

#if (LIGHT_TYPE_HSL_EN)
    #if (ELE_CNT_EVERY_LIGHT >= 2)
#define MD_HSL_HUE_SAT_S_PRIMARY    
    #else
#define MD_HSL_HUE_SAT_S_PRIMARY    SIG_MD_LIGHT_HSL_HUE_S,SIG_MD_LIGHT_HSL_SAT_S,
    #endif
    #if MD_SERVER_EN
#define MD_ID_ARRAY_LIGHT_HSL_SERVER    SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_SETUP_S, MD_HSL_HUE_SAT_S_PRIMARY
    #else
#define MD_ID_ARRAY_LIGHT_HSL_SERVER    
    #endif
#define MD_ID_ARRAY_LIGHT_HSL   MD_ID_ARRAY_LIGHT_HSL_SERVER  MD_ID_ARRAY_LIGHT_HSL_CLIENT
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
#define MD_ID_ARRAY_LIGHT_XYL   MD_ID_ARRAY_LIGHT_XYL_SERVER  MD_ID_ARRAY_LIGHT_XYL_CLIENT
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
#endif

#if (MD_LIGHT_CONTROL_EN)
#define MD_ID_ARRAY_LIGHT_LC    MD_ID_ARRAY_LIGHT_LC_SERVER  MD_ID_ARRAY_LIGHT_LC_CLIENT
#endif

#ifndef MD_ID_ARRAY_VENDOR_SERVER
#define MD_ID_ARRAY_VENDOR_SERVER      
#endif
#ifndef MD_ID_ARRAY_MESH_OTA
#define MD_ID_ARRAY_MESH_OTA      
#endif
#ifndef MD_ID_ARRAY_ONOFF
#define MD_ID_ARRAY_ONOFF      
#endif
#ifndef MD_ID_ARRAY_LEVEL
#define MD_ID_ARRAY_LEVEL      
#endif
#ifndef MD_ID_ARRAY_DEF_TRANSIT_TIME
#define MD_ID_ARRAY_DEF_TRANSIT_TIME      
#endif
#ifndef MD_ID_ARRAY_POWER_ONOFF
#define MD_ID_ARRAY_POWER_ONOFF      
#endif
#ifndef MD_ID_ARRAY_TIME
#define MD_ID_ARRAY_TIME      
#endif
#ifndef MD_ID_ARRAY_SCENE
#define MD_ID_ARRAY_SCENE      
#endif
#ifndef MD_ID_ARRAY_SCHED
#define MD_ID_ARRAY_SCHED      
#endif
#ifndef MD_ID_ARRAY_POWER_LEVEL
#define MD_ID_ARRAY_POWER_LEVEL      
#endif
#ifndef MD_ID_ARRAY_LIGHTNESS
#define MD_ID_ARRAY_LIGHTNESS      
#endif
#ifndef MD_ID_ARRAY_LIGHT_CTL
#define MD_ID_ARRAY_LIGHT_CTL      
#endif
#ifndef MD_ID_ARRAY_LIGHT_HSL
#define MD_ID_ARRAY_LIGHT_HSL      
#endif
#ifndef MD_ID_ARRAY_LIGHT_XYL
#define MD_ID_ARRAY_LIGHT_XYL      
#endif
#ifndef MD_ID_ARRAY_LIGHT_LC
#define MD_ID_ARRAY_LIGHT_LC      
#endif
#ifndef MD_ID_ARRAY_SENSOR
#define MD_ID_ARRAY_SENSOR      
#endif
#ifndef MD_ID_ARRAY_PROPERTY
#define MD_ID_ARRAY_PROPERTY      
#endif
#ifndef MD_ID_ARRAY_BATTERY
#define MD_ID_ARRAY_BATTERY      
#endif
#ifndef MD_ID_ARRAY_LOCATION					
#define MD_ID_ARRAY_LOCATION
#endif


#define MD_ID_ARRAY_COMMON     \
    MD_ID_ARRAY_ONOFF               MD_ID_ARRAY_LEVEL \
    MD_ID_ARRAY_DEF_TRANSIT_TIME    MD_ID_ARRAY_POWER_ONOFF \
    MD_ID_ARRAY_SCENE \
    MD_ID_ARRAY_SCHED               MD_ID_ARRAY_POWER_LEVEL \
    MD_ID_ARRAY_LIGHTNESS           MD_ID_ARRAY_LIGHT_CTL \
    MD_ID_ARRAY_LIGHT_HSL           MD_ID_ARRAY_LIGHT_XYL \
    MD_ID_ARRAY_LIGHT_LC 			MD_ID_ARRAY_SENSOR      \
    MD_ID_ARRAY_BATTERY				MD_ID_ARRAY_LOCATION    \
    MD_ID_ARRAY_PROPERTY

#define MD_ID_ARRAY_SIG2VD          MD_ID_ARRAY_MESH_OTA    MD_ID_ARRAY_REMOTE_PROV
#if DRAFT_FEAT_VD_MD_EN
#define MD_ID_ARRAY_SIG2VD_SIG       
#define MD_ID_ARRAY_SIG2VD_VD       MD_ID_ARRAY_SIG2VD
#else
#define MD_ID_ARRAY_SIG2VD_SIG      MD_ID_ARRAY_SIG2VD
#define MD_ID_ARRAY_SIG2VD_VD        
#endif

#if (0 == DEBUG_EVB_EN)
#define MD_ID_ARRAY_PRIMARY     MD_ID_ARRAY_CFG   MD_ID_ARRAY_SIG2VD_SIG   MD_ID_ARRAY_TIME   MD_ID_ARRAY_COMMON
#else
#define MD_ID_ARRAY_PRIMARY     MD_ID_ARRAY_CFG   MD_ID_ARRAY_MESH_OTA   MD_ID_ARRAY_ONOFF
#endif
#define MD_ID_ARRAY_PRIMARY_VD  MD_ID_ARRAY_SIG2VD_VD   MD_ID_ARRAY_VENDOR_SERVER  MD_ID_ARRAY_VENDOR_CLIENT

const u16 md_id_sig_primary[] =     {MD_ID_ARRAY_PRIMARY};
const u32 md_id_vendor_primary[] =  {MD_ID_ARRAY_PRIMARY_VD};
#if (ELE_CNT_EVERY_LIGHT >= 2)
STATIC_ASSERT(MD_LEVEL_EN);
    #if (LIGHT_TYPE_SEL == LIGHT_TYPE_CT)
#define MD_ID_ARRAY_SECOND          SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_CTL_TEMP_S  // only server model
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL)
#define MD_ID_ARRAY_SECOND          SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_HSL_HUE_S   // only server model
#define MD_ID_ARRAY_THIRD           SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_HSL_SAT_S   // only server model
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
#define MD_ID_ARRAY_SECOND          SIG_MD_G_LEVEL_S                            // only server model
#define MD_ID_ARRAY_THIRD           SIG_MD_G_LEVEL_S                            // only server model
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
#define MD_ID_ARRAY_SECOND          SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_CTL_TEMP_S  // only server model
#define MD_ID_ARRAY_THIRD           SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_HSL_HUE_S   // only server model
#define MD_ID_ARRAY_FOURTH          SIG_MD_G_LEVEL_S,  SIG_MD_LIGHT_HSL_SAT_S   // only server model
    #endif

const u16 md_id_sig_second[] =      {MD_ID_ARRAY_SECOND};
    #if (ELE_CNT_EVERY_LIGHT >= 3)
const u16 md_id_sig_third[] =       {MD_ID_ARRAY_THIRD};
STATIC_ASSERT(ARRAY_SIZE(md_id_sig_third) == ARRAY_SIZE(md_id_sig_second)); // because of page0_local_t
    #endif
    #if (ELE_CNT_EVERY_LIGHT >= 4)
const u16 md_id_sig_fourth[] =  {MD_ID_ARRAY_FOURTH};
STATIC_ASSERT(ARRAY_SIZE(md_id_sig_fourth) == ARRAY_SIZE(md_id_sig_second));
    #endif
#else
const u16 md_id_sig_second[] =      {MD_ID_ARRAY_COMMON};
    #if MD_SERVER_EN
const u32 md_id_vendor_second[] =   {MD_ID_ARRAY_VENDOR_SERVER};
    #endif
#endif

typedef struct{
	u16 loc;
	u8 nums;
	u8 numv;
	u16 md_sig[ARRAY_SIZE(md_id_sig_primary)];
	u32 md_vendor[ARRAY_SIZE(md_id_vendor_primary)];
}mesh_element_primary_t;

typedef struct{
	u16 loc;
	u8 nums;
	u8 numv;
	u16 md_sig[ARRAY_SIZE(md_id_sig_second)];
	#if (LIGHT_CNT > 1)
	u32 md_vendor[ARRAY_SIZE(md_id_vendor_second)];
	#endif
}mesh_element_second_t;


typedef struct{
	mesh_page0_head_t head;
	mesh_element_primary_t ele_primary;
	#if (ELE_CNT > 1)
	mesh_element_second_t ele_second[ELE_CNT-1];
	#endif
}page0_local_t;

typedef struct{
	page0_local_t page0;     // differrent page with differrent struct, so can not use array.
}mesh_composition_data_local_t;

#define CPS_DATA_ELE_SECOND  {\
    0, \
    ARRAY_SIZE(md_id_sig_second),\
    ARRAY_SIZE(md_id_vendor_second),\
    {MD_ID_ARRAY_COMMON},\
    {MD_ID_ARRAY_VENDOR_SERVER},\
}

/*please refer to spec "4.2.1 Composition Data"*/
mesh_composition_data_local_t model_sig_cfg_s_cps = {   // can't extern, must static
    {
        // head =
        {
            VENDOR_ID,      // u16 cid = ;
            MESH_PID_SEL,   // u16 pid = ;
            MESH_VID,       // u16 vid = ;
            MESH_NODE_MAX_NUM, //CACHE_BUF_MAX,  // u16 crpl = ;
            //mesh_page_feature_t feature = 
            {
                FEATURE_RELAY_EN,       // u16 relay       :1;
                FEATURE_PROXY_EN,        // u16 proxy       :1;
                #if WIN32
                0,
                #else
                FEATURE_FRIEND_EN,      // u16 frid        :1;
                #endif
                FEATURE_LOWPOWER_EN,    // u16 low_power   :1;
                0,                      // u16 rfu         :12;
            },
        },
        // mesh_element_primary_t ele_primary = 
        {
            0,                                  // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_primary),      // u8 nums;
            ARRAY_SIZE(md_id_vendor_primary),   // u8 numv;
            {MD_ID_ARRAY_PRIMARY},              // u16 md_sig[] = 
            {MD_ID_ARRAY_PRIMARY_VD},           // u32 md_vendor[] = 
        },
#if (ELE_CNT >= 2)
    #if (ELE_CNT_EVERY_LIGHT >= 2)
        {
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_second),       // u8 nums;
            0,
            {MD_ID_ARRAY_SECOND},},             // u16 md_sig[] = 
        #if (ELE_CNT_EVERY_LIGHT >= 3)
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_third),        // u8 nums;
            0,
            {MD_ID_ARRAY_THIRD},},              // u16 md_sig[] = 
        #endif
    	#if (ELE_CNT_EVERY_LIGHT >= 4)
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_fourth),       // u8 nums;
            0,
            {MD_ID_ARRAY_FOURTH},},             // u16 md_sig[] = 
    	#endif
        },
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL)
        // mesh_element_second_t ele_second[] = 
        {
            CPS_DATA_ELE_SECOND,
            #if ELE_CNT >= 3
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 4
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 5
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 6
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 7
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 8
            CPS_DATA_ELE_SECOND,
            #endif
        },
    #endif
#endif
    },
};
#if MD_REMOTE_PROV
mesh_composition_data_local_t model_sig_cfg_s_cps_page128 = {   // can't extern, must static
    {
        // head =
        {
            VENDOR_ID,      // u16 cid = ;
            MESH_PID_SEL,   // u16 pid = ;
            MESH_VID,       // u16 vid = ;
            MESH_NODE_MAX_NUM, //CACHE_BUF_MAX,  // u16 crpl = ;
            //mesh_page_feature_t feature = 
            {
                FEATURE_RELAY_EN,       // u16 relay       :1;
                FEATURE_PROXY_EN,        // u16 proxy       :1;
                FEATURE_FRIEND_EN,      // u16 frid        :1;
                FEATURE_LOWPOWER_EN,    // u16 low_power   :1;
                0,                      // u16 rfu         :12;
            },
        },
        // mesh_element_primary_t ele_primary = 
        {
            1,                                  // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_primary),      // u8 nums;
            ARRAY_SIZE(md_id_vendor_primary),   // u8 numv;
            {MD_ID_ARRAY_PRIMARY},              // u16 md_sig[] = 
            {MD_ID_ARRAY_PRIMARY_VD},           // u32 md_vendor[] = 
        },
#if (ELE_CNT >= 2)
    #if (ELE_CNT_EVERY_LIGHT >= 2)
        {
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_second),       // u8 nums;
            0,
            {MD_ID_ARRAY_SECOND},},             // u16 md_sig[] = 
        #if (ELE_CNT_EVERY_LIGHT >= 3)
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_third),        // u8 nums;
            0,
            {MD_ID_ARRAY_THIRD},},              // u16 md_sig[] = 
        #endif
    	#if (ELE_CNT_EVERY_LIGHT >= 4)
            {0,                                 // u16 loc; 0x0000 means unknown
            ARRAY_SIZE(md_id_sig_fourth),       // u8 nums;
            0,
            {MD_ID_ARRAY_FOURTH},},             // u16 md_sig[] = 
    	#endif
        },
    #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_PANEL)
        // mesh_element_second_t ele_second[] = 
        {
            CPS_DATA_ELE_SECOND,
            #if ELE_CNT >= 3
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 4
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 5
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 6
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 7
            CPS_DATA_ELE_SECOND,
            #endif
            #if ELE_CNT >= 8
            CPS_DATA_ELE_SECOND,
            #endif
        },
    #endif
#endif
    },
};

u8 mesh_cps_data_page0_page128_is_same()
{
	if(!memcmp(&model_sig_cfg_s_cps,&model_sig_cfg_s_cps_page128,sizeof(model_sig_cfg_s_cps))){
		return 1;
	}else{
		return 0;
	}
}

u8 mesh_cps_data_update_page0_from_page128()
{
	if(!mesh_cps_data_page0_page128_is_same()){
		memcpy(&model_sig_cfg_s_cps,&model_sig_cfg_s_cps_page128,sizeof(model_sig_cfg_s_cps));
	}
	return 1;
}
#endif

u16 get_op_u16(u8 *p_op)
{
    if(OP_TYPE_SIG2 == GET_OP_TYPE(p_op[0])){
        return (u16)(p_op[0] + p_op[1]*256);
    }else{
        return (p_op[0]);
    }
}

/*const */mesh_page0_t * const gp_page0 = (mesh_page0_t *)(&model_sig_cfg_s_cps.page0);
const u32 SIZE_OF_PAGE0_LOCAL = sizeof(model_sig_cfg_s_cps.page0);

int mesh_search_model_id_by_op(mesh_op_resource_t *op_res, u16 op, u8 tx_flag);

u32 get_cps_vd_model_id(const mesh_element_head_t *p_ele, u32 index)
{
	u8 *p_vd_id = (u8 *)((u8 *)(p_ele->md_sig) + p_ele->nums * 2 + index*4);
	u32 vd_model_id = p_vd_id[0]+(p_vd_id[1]<<8)
					 +(p_vd_id[2]<<16)+(p_vd_id[3]<<24);
	return vd_model_id;
}

u32 get_cps_ele_len(const mesh_element_head_t *p_ele)
{
	return (OFFSETOF(mesh_element_head_t, md_sig) + p_ele->nums * 2 + p_ele->numv * 4);
}

void ev_handle_traversal_cps_ll(u32 ev, u8 *par, u16 ele_adr, u32 model_id, bool4 sig_model)
{
	switch(ev){
		case EV_TRAVERSAL_SET_MD_ELE_ADR:
			mesh_set_model_ele_adr(ele_adr, model_id, sig_model);
			break;
		case EV_TRAVERSAL_BIND_APPKEY:
			{u16 ak_idx = par[0] + (par[1] << 8);
			mesh_appkey_bind(MODE_APP_BIND, ele_adr, model_id, sig_model, ak_idx);
			}
			break;
		case EV_TRAVERSAL_UNBIND_APPKEY:
			{u16 ak_idx = par[0] + (par[1] << 8);
			mesh_appkey_bind(MODE_APP_UNBIND, ele_adr, model_id, sig_model, ak_idx);
			}
			break;
		case EV_TRAVERSAL_PUB_PERIOD:
			mesh_tx_pub_period(ele_adr, model_id, sig_model);
			break;
		default :
			break;
	}
}

void mesh_set_all_model_ele_adr()
{
	ev_handle_traversal_cps(EV_TRAVERSAL_SET_MD_ELE_ADR, 0);
}

void mesh_unbind_by_del_appkey(u16 appkey_index)
{
	ev_handle_traversal_cps(EV_TRAVERSAL_UNBIND_APPKEY, (u8 *)&appkey_index);
}

void mesh_pub_period_proc()
{
	static u32 tick_pub_period_check;
	if(clock_time_exceed(tick_pub_period_check, 40*1000)){
		//if(buf_is_enough){
			tick_pub_period_check = clock_time();
			ev_handle_traversal_cps(EV_TRAVERSAL_PUB_PERIOD, 0);
		//}
	}
}

void ev_handle_traversal_cps(u32 ev, u8 *par)
{
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    const mesh_element_head_t *p_ele = &gp_page0->ele;
    int total_len_ele = SIZE_OF_PAGE0_LOCAL - offset_ele;
    u16 ele_adr = ele_adr_primary;
	while (pos < total_len_ele)
	{
        foreach(i, p_ele->nums){
        	ev_handle_traversal_cps_ll(ev, par, ele_adr, p_ele->md_sig[i], 1);
        }

        foreach(i, p_ele->numv){
        	u32 vd_model_id = get_cps_vd_model_id(p_ele, i);
        	ev_handle_traversal_cps_ll(ev, par, ele_adr, vd_model_id, 0);
        }
        
        int len_ele = get_cps_ele_len(p_ele);
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
		pos += len_ele;
		ele_adr += 1;
	}
}

u8 find_ele_support_model_and_match_dst(mesh_adr_list_t *adr_list, u16 adr_dst, u32 model_id, bool4 sig_model)
{
    memset(adr_list, 0, sizeof(mesh_adr_list_t));
    u8 st = ST_UNSPEC_ERR;
    int unicast_flag = is_unicast_adr(adr_dst);
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    const mesh_element_head_t *p_ele = &gp_page0->ele;
    int total_len_ele = SIZE_OF_PAGE0_LOCAL - offset_ele;
    u16 ele_adr = ele_adr_primary;
	while (pos < total_len_ele)
	{
        if((!unicast_flag) || (adr_dst == ele_adr)){
            st = ST_INVALID_MODEL;
            if(sig_model){
                foreach(i, p_ele->nums){
                    if(model_id == p_ele->md_sig[i]){
                        if(adr_list->adr_cnt < ELE_CNT_MAX_LIB){
                            adr_list->adr[adr_list->adr_cnt++] = ele_adr;
                        }
                        
                        if(unicast_flag){
                            return ST_SUCCESS;
                        }
                    }
                }
            }else{
                foreach(i, p_ele->numv){
                    if(model_id == get_cps_vd_model_id(p_ele, i)){
                        if(adr_list->adr_cnt < ELE_CNT_MAX_LIB){
                            adr_list->adr[adr_list->adr_cnt++] = ele_adr;
                        }
                        
                        if(unicast_flag){
                            return ST_SUCCESS;
                        }						
                    }
                }
            }

            if(unicast_flag){
                return st;
            }
        }
		else{
			st = ST_INVALID_ADR;
		}
        
        int len_ele = get_cps_ele_len(p_ele);
		pos += len_ele;
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
        ele_adr += 1;
	}
	return st;
}

int mesh_sec_msg_dec_virtual (u8 *key, u8 *nonce, u8 *dat, int n, int mic_length, u16 adr_dst, const u8 *dat_org)
{
	#if MESH_MONITOR_EN
	return -1;
	#endif
	
	#if VIRTUAL_ADDR_ENABLE
	if(!is_virtual_adr(adr_dst)){
		return -1;
	}
	
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    const mesh_element_head_t *p_ele = &gp_page0->ele;
    int total_len_ele = SIZE_OF_PAGE0_LOCAL - offset_ele;
    u16 ele_adr = ele_adr_primary;
	while (pos < total_len_ele)
	{
		u32 md_id;
        foreach(i, p_ele->nums){
        	md_id = p_ele->md_sig[i];
        	if(!mesh_sec_msg_dec_virtual_ll(ele_adr, md_id, 1, key, nonce, dat, n, mic_length, adr_dst, dat_org)){
        		return 0;
        	}
        }
        
        foreach(i, p_ele->numv){
            md_id = get_cps_vd_model_id(p_ele, i);
        	if(!mesh_sec_msg_dec_virtual_ll(ele_adr, md_id, 0, key, nonce, dat, n, mic_length, adr_dst, dat_org)){
        		return 0;
        	}
        }
        
        int len_ele = get_cps_ele_len(p_ele);
		pos += len_ele;
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
        ele_adr += 1;
	}
	#endif
	
	return -1;
}

u8 get_ele_cnt_by_traversal_cps(mesh_page0_t * p_page0, u32 len_page0)
{
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    mesh_element_head_t *p_ele = &p_page0->ele;
    int total_len_ele = len_page0 - offset_ele;
    u32 cnt = 0;
	while (pos < total_len_ele)
	{
        int len_ele = get_cps_ele_len(p_ele);
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
		pos += len_ele;
		if(pos > total_len_ele){
		    return 0;   // length error  
		}
		cnt++;
	}
	return (u8)cnt;
}

void traversal_cps_reset_vendor_id(u16 vd_id)
{
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    const mesh_element_head_t *p_ele = &gp_page0->ele;
    int total_len_ele = SIZE_OF_PAGE0_LOCAL - offset_ele;
    //u16 ele_adr = ele_adr_primary;

    g_vendor_id = cps_cid = vd_id;
	while (pos < total_len_ele)
	{
        //foreach(i, p_ele->nums){
        //}

        foreach(i, p_ele->numv){
            u8 *p_vd_id = (u8 *)((u8 *)(p_ele->md_sig) + p_ele->nums * 2 + i*4);
            p_vd_id[0] = vd_id & 0xff;
            p_vd_id[1] = (vd_id >> 8) & 0xff;
            // id value is same between the two vendor model.
        }
        
        int len_ele = get_cps_ele_len(p_ele);
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
		pos += len_ele;
		//ele_adr += 1;
	}
}

/**
* function: get offset between object addr and element addr which include the model.
*/
u8 get_ele_offset_by_model(mesh_page0_t * p_page0, u32 len_page0, u16 node_adr, u16 obj_adr, u32 model_id, bool4 sig_model)
{
	int pos = 0;
	int offset_ele = OFFSETOF(mesh_page0_t, ele);
    mesh_element_head_t *p_ele = &p_page0->ele;
    int total_len_ele = len_page0 - offset_ele;
    u32 index = 0;
	while (pos < total_len_ele)
	{
	    if(obj_adr >= node_adr){
    	    if(sig_model){
                foreach(i, p_ele->nums){
                	if(p_ele->md_sig[i] == model_id){
                	    return index;
                	}
                }
            }else{
                foreach(i, p_ele->numv){
                    u32 vd_model_id = get_cps_vd_model_id(p_ele, i);
                	if(vd_model_id == model_id){
                	    return index;
                	}
                }
            }
        }

        int len_ele = get_cps_ele_len(p_ele);
        p_ele = (mesh_element_head_t *)((u8 *)p_ele + len_ele);
		pos += len_ele;
		index++;
	}
    // LOG_MSG_ERR(TL_LOG_COMMON, 0, 0, "MODEL_NOT_FOUND!........................",0);
	return MODEL_NOT_FOUND;
}

#if (IS_VC_PROJECT_MASTER)
int is_support_model_dst(u16 adr_dst, u32 model_id, bool4 sig_model)
{
    // not use 'get_ele_offset_by_model_VC_node_info_()', because should not ouput ERR log.
    VC_node_info_t * p_info = get_VC_node_info(adr_dst, 0);
    if(p_info){
        u8 offset = get_ele_offset_by_model((mesh_page0_t *)(&p_info->cps.page0_head), p_info->cps.len_cps, p_info->node_adr, adr_dst, model_id, sig_model);
        if(offset != MODEL_NOT_FOUND){
            #if 0   // because it may there is several same mode in a node, for example level model.
            return ((p_info->node_adr + offset) == ele_adr);
            #else
            return 1;
            #endif
        }
    }
    return 0;
}

int is_support_op_dst(u16 op, u16 adr_dst)     // provisioner check dst unicast when tx command.
{
    mesh_op_resource_t op_res = {0};
    if(0 == mesh_search_model_id_by_op(&op_res, op, 0)){
        if(is_use_device_key(op_res.id, op_res.sig)){
            return 1;
        }else{
            return is_support_model_dst(adr_dst, op_res.id, op_res.sig);
        }
    }
    return 0;
}

#if VC_APP_ENABLE
int is_support_op_dst_VC_APP(u16 op, u16 adr_dst)
{
    #if 0   // because no get cps in some mode.
    if(is_provision_success()){
        if((OP_TYPE_VENDOR != GET_OP_TYPE(op)) && is_unicast_adr(adr_dst)){
            if(!is_support_op_dst(op, adr_dst)){
                MessageBoxVC("dst address not support this op, please check!");
                return 0;
            }
        }
    }
    #endif
    return 1;
}
#endif
#endif

int is_mesh_adv_cmd_fifo_empty()
{
    return (0 == my_fifo_data_cnt_get(&mesh_adv_cmd_fifo));
}

u16 get_primary_adr(u16 ele_adr)
{
    VC_node_info_t * p_info = get_VC_node_info(ele_adr, 0);
    if(p_info){
        return p_info->node_adr;
    }

    LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"get primary address failed: VC node info not found",0);
    return NODE_ADR_AUTO;
}

u16 get_primary_adr_with_check(u16 node_adr, u16 ele_adr)
{
    #if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
    if(NODE_ADR_AUTO == node_adr){
        node_adr = get_primary_adr(ele_adr);
    }
    #endif
    
    return node_adr;
}

u8 * get_virtual_adr_uuid(u16 pub_adr, model_common_t *p_com_md)
{
	if(is_virtual_adr(pub_adr)){
		return p_com_md->pub_uuid;
	}
	return 0;
}

#if ((MD_BIND_WHITE_LIST_EN) && (0 == DEBUG_MESH_DONGLE_IN_VC_EN))
void get_cps_from_keybind_list(mesh_page0_t *page0_out, mesh_page0_t *p_rsp_page0)
{
	if(key_bind_list_cnt == 0){
		return;
	}
	mesh_element_head_t *p_primary_dst = &page0_out->ele;
	mesh_element_head_t *p_primary = &p_rsp_page0->ele;
	u8 idx_nums=0;
	for(int i=0;i<p_primary->nums;i++){
		if(is_model_in_key_bind_list(p_primary->md_sig+i)){
			p_primary_dst->md_sig[idx_nums]= p_primary->md_sig[i];
			idx_nums++;
		}
	}
	p_primary_dst->nums = idx_nums;
	p_primary_dst->numv = p_primary->numv;
	memcpy((u8 *)(p_primary_dst->md_sig+idx_nums),(u8 *)(p_primary->md_sig+p_primary->nums),(p_primary->numv)*4);
	return;
}
#endif

#if VC_APP_ENABLE
#include <iostream>     // include rand_
#endif

// cache
cache_buf_t cache_buf[CACHE_BUF_MAX];
u16 g_cache_buf_max = CACHE_BUF_MAX;
#ifndef WIN32
//STATIC_ASSERT(sizeof(cache_buf_t) % 4 == 0);   // for compare sno quickly
//STATIC_ASSERT(CACHE_BUF_MAX <= 110);
#endif

void mesh_network_cache_buf_init()
{
    memset(cache_buf, 0, sizeof(cache_buf));
}

#if DEBUG_CFG_CMD_GROUP_AK_EN
nw_notify_record_t nw_notify_record;
#endif
// cache end

#define PUB_RANDOM_DELAY_MAX_MS         (2000)
#define PUB_RANDOM_RATE                 (10)        // (1/10)

u16 publish_powerup_random_ms = 0;

/*
    return : interval_ms  * 10%
*/
u32 get_random_delay_pub_tick_ms(u32 interval_ms)
{
    #if 0   // if no need random
    return 0;
    #else
    u32 rand_max_ms = interval_ms / (PUB_RANDOM_RATE + 1) ;   // because interval is add 10% before.
    if(rand_max_ms > PUB_RANDOM_DELAY_MAX_MS){
        rand_max_ms = PUB_RANDOM_DELAY_MAX_MS;
    }
    u16 rand_val =0;
    #if !WIN32
    rand_val = rand();
    #endif
    #if WIN32
    rand_val = (u16)((clock_time() & 0xffff) ^ rand_val);
    #endif
    rand_val = (rand_val % (rand_max_ms*2));  // because interval is add (1 / PUB_RANDOM_RATE)
    //LOG_MSG_INFO (TL_LOG_COMMON, 0, 0, "rand val: %d",rand_val - rand_max_ms);
    return rand_val;
    #endif
}

u32 get_mesh_pub_interval_ms(u32 model_id, bool4 sig_model, mesh_pub_period_t *period)
{
	u32 step_res_ms = get_transition_100ms((trans_time_t *)period)*100;
	step_res_ms = step_res_ms + (step_res_ms / PUB_RANDOM_RATE);
	if((sig_model && (SIG_MD_HEALTH_SERVER == model_id)) && (model_sig_health.srv.health_mag.cur_sts.cur_fault_idx != 0)){
		step_res_ms = step_res_ms>>model_sig_health.srv.health_mag.period_sts.fast_period_log;
	}
	#if(MD_SENSOR_SERVER_EN)
	else if(sig_model && (SIG_MD_SENSOR_S == model_id)){
		u8 fast_candence = 0;
		if((model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_hight>=model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_low)){
			if((model_sig_sensor.sensor_states[0].sensor_data < model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_hight) && 
			(model_sig_sensor.sensor_states[0].sensor_data > model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_low)){
				fast_candence = 1;
			}
		}
		else{
			if((model_sig_sensor.sensor_states[0].sensor_data < model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_hight) || 
			(model_sig_sensor.sensor_states[0].sensor_data > model_sig_sensor.sensor_states[0].cadence.cadence_unit.cadence_low)){
				fast_candence = 1;
			}
		}
		
		if(fast_candence){
			step_res_ms = step_res_ms>>model_sig_sensor.sensor_states[0].cadence.fast_period_div;
			if(step_res_ms < (((u32)1<<model_sig_sensor.sensor_states[0].cadence.cadence_unit.min_interval))){
				step_res_ms = ((u32)1<<model_sig_sensor.sensor_states[0].cadence.cadence_unit.min_interval);
			}
		}
	}
	#endif
	
	return step_res_ms;
}

// config model
int mesh_cmd_sig_cfg_cps_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    if(1){//(0 == par[0]) || (0xff == par[0])){    // page0
    u8 rsp[1 + sizeof(model_sig_cfg_s_cps.page0)];
	u32 rsp_len = sizeof(rsp);
	rsp[0] = 0;     
	#if MD_REMOTE_PROV
		// when cpy and compare ,now we only suppose the page0 and page128 is the same .
		if(par[0]== 0x80){
			rsp_len = 1+sizeof(model_sig_cfg_s_cps_page128.page0);
			memcpy(rsp+1, &model_sig_cfg_s_cps_page128.page0, sizeof(model_sig_cfg_s_cps_page128.page0));
		}else{
			rsp_len = 1+sizeof(model_sig_cfg_s_cps.page0);
			memcpy(rsp+1, &model_sig_cfg_s_cps.page0, sizeof(model_sig_cfg_s_cps.page0));
		}
		return mesh_tx_cmd_rsp_cfg_model(COMPOSITION_DATA_STATUS, rsp, rsp_len, cb_par->adr_src);
	#else		
		#if (DRAFT_FEATURE_VENDOR_TYPE_SEL == DRAFT_FEATURE_VENDOR_TYPE_ONE_OP)
		rsp_len = 1 + get_cps_vendor_op_extend(rsp+1);
		#else
		memcpy(rsp+1, &model_sig_cfg_s_cps.page0, sizeof(model_sig_cfg_s_cps.page0));
		#endif
        return mesh_tx_cmd_rsp_cfg_model(COMPOSITION_DATA_STATUS, rsp, rsp_len, cb_par->adr_src);
	#endif
    }else{
        return 0;
    }
}

int mesh_cmd_sig_cfg_cps_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    	mesh_kr_cfgcl_check_set_next_flag();
    }
    return err;
}

// iv index --------------
void mesh_calculate_ivi(u8 *p_ivi, u8 add){
    u32 ivi_t;
    memcpy(&ivi_t, p_ivi, 4);
    endianness_swap_u32((u8 *)&ivi_t);  // swap to little endianness
    if(add){
        ivi_t++;
    }else{
        ivi_t--;
    }
    endianness_swap_u32((u8 *)&ivi_t);  // restore to big endianness
    memcpy(p_ivi, &ivi_t, 4);
}

void mesh_increase_ivi(u8 *p_ivi){
    mesh_calculate_ivi(p_ivi, 1);
}

void mesh_decrease_ivi(u8 *p_ivi){
    mesh_calculate_ivi(p_ivi, 0);
}

int mesh_ivi_greater_or_equal(const u8 *p_ivi1, const u8 *p_ivi2, u32 val){
    u32 ivi1, ivi2;
    memcpy(&ivi1, p_ivi1, 4);
    endianness_swap_u32((u8 *)&ivi1);  // swap to little endianness
    memcpy(&ivi2, p_ivi2, 4);
    endianness_swap_u32((u8 *)&ivi2);  // swap to little endianness
    if(ivi1 >= ivi2){
    	if(ivi1 - ivi2 >= val){
        	return 1;
        }
    }
    return 0;
}

int mesh_ivi_equal(const u8 *p_ivi1, const u8 *p_ivi2, u32 delta){
    u32 ivi1, ivi2;
    memcpy(&ivi1, p_ivi1, 4);
    endianness_swap_u32((u8 *)&ivi1);  // swap to little endianness
    memcpy(&ivi2, p_ivi2, 4);
    endianness_swap_u32((u8 *)&ivi2);  // swap to little endianness
    return (ivi1 - ivi2 == delta);
}

void mesh_set_iv_idx_rx(u8 ivi){
    memcpy(iv_idx_st.rx, iv_idx_st.cur, 4);
    if((ivi & 1) != (iv_idx_st.cur[3] & 1)){
        mesh_decrease_ivi(iv_idx_st.rx);
    }else{
        // use current iv index
    }
}

void mesh_iv_idx_init_cb(int rst_sno)
{
	if(rst_sno){// clear the cache_init
		#if MI_API_ENABLE
		cache_init(ADR_ALL_NODES);
		#endif
	}
}

u32 mesh_net_key_empty_search()
{
	u32 key_max = NET_KEY_MAX;
	#if TESTCASE_FLAG_ENABLE
	if(netkey_list_test_mode_en){
		key_max = 1;
	}
	#else
	key_max = key_max;
	#endif
	
    foreach(i,key_max){
        if(KEY_UNVALID == mesh_key.net_key[i][0].valid){
            return i;
        }
    }
    return NET_KEY_MAX;
}

u32 mesh_net_key_find(u16 key_index)
{
    foreach(i,NET_KEY_MAX){
        if((KEY_VALID == mesh_key.net_key[i][0].valid)
        && (key_index == mesh_key.net_key[i][0].index)){
            return i;
        }
    }
    return NET_KEY_MAX;
}

u8 * mesh_cfg_cmd_dev_key_get(const u16 adr)
{
	#if DEBUG_CFG_CMD_GROUP_AK_EN
    if(DEBUG_CFG_CMD_GROUP_USE_AK(adr)){
        return mesh_key.net_key[0][0].app_key[0].key;
    }
    #endif
	
	#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
	if(!is_own_ele(adr) && is_unicast_adr(adr)){
		return VC_master_get_other_node_dev_key(adr);
	}
	#endif
	
	return mesh_key.dev_key;
}

#if MD_REMOTE_PROV
u8 * mesh_cfg_cmd_dev_key_candi_get(const u16 adr)
{
	#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
	if(!is_own_ele(adr) && is_unicast_adr(adr)){
		return VC_master_get_other_node_dev_key_candi(adr);
	}
	#endif
	
	return node_devkey_candi;
}
#endif


static u8 devkey_candi_en = 0;
void mesh_tx_en_devkey_candi(u8 en)
{
	devkey_candi_en = en;
}

u8 *mesh_cfg_devkey_candi_get(u16 adr)
{
	#if MD_REMOTE_PROV
	if(devkey_candi_en){
		return mesh_cfg_cmd_dev_key_candi_get(adr);
	}else
	#endif
	{
		return mesh_cfg_cmd_dev_key_get(adr);
	}
}
mesh_app_key_t *mesh_tx_access_key_get(u8 *mat, u8 akf)
{
	material_tx_cmd_t *p_mat = (material_tx_cmd_t *)mat;
	mesh_app_key_t *p_key_str = NULL;
	if(akf){
		if((p_mat->nk_array_idx >= NET_KEY_MAX) || (p_mat->ak_array_idx >= APP_KEY_MAX)){
			LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"tx_access:key array index not existed,nk:%x,ak:%x",p_mat->nk_array_idx,p_mat->ak_array_idx);
			return 0;
		}

		int new_key_flag = is_key_refresh_use_new_key(p_mat->nk_array_idx, 0);
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"mesh_tx_access_key_get:print index part%d,%d,%d  ",p_mat->nk_array_idx,new_key_flag,p_mat->ak_array_idx);
		p_key_str = &(mesh_key.net_key[p_mat->nk_array_idx][new_key_flag].app_key[p_mat->ak_array_idx]);
		
		if(!(mesh_key.net_key[p_mat->nk_array_idx][new_key_flag].valid) || (!p_key_str->valid)){
			LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"mesh_tx_access_key_get:key index unvalid",0);
			return 0;
		}
	}else{
		mesh_op_resource_t op_resource = {0};
		mesh_search_model_id_by_op(&op_resource, p_mat->op, 1);
        if(op_resource.status_cmd){
            p_key_str = (mesh_app_key_t *)mesh_cfg_devkey_candi_get(p_mat->adr_src);
        }
		else{
			p_key_str = (mesh_app_key_t *)mesh_cfg_devkey_candi_get(p_mat->adr_dst);
		}
	}

	return p_key_str;
}

static inline void APP_set_self_dev_key2node_info()
{
#if (IS_VC_PROJECT_MASTER)
	// TODO
#endif
}

const u32 MODEL_ID_DEV_KEY[] = {
    SIG_MD_CFG_SERVER,              SIG_MD_CFG_CLIENT,
    SIG_MD_REMOTE_PROV_SERVER,      SIG_MD_REMOTE_PROV_CLIENT,
	SIG_MD_DF_CFG_S,                SIG_MD_DF_CFG_C,
	SIG_MD_BRIDGE_CFG_SERVER,       SIG_MD_BRIDGE_CFG_CLIENT,
	SIG_MD_PRIVATE_BEACON_SERVER,   SIG_MD_PRIVATE_BEACON_CLIENT
};

int is_use_device_key(u32 model_id, int sig_flag)
{
    if(sig_flag){
        foreach_arr(i,MODEL_ID_DEV_KEY){
            if(model_id == MODEL_ID_DEV_KEY[i]){
                return 1;
            }
        }
	}else{
	    #if DRAFT_FEAT_VD_MD_EN
	    if((SIG_MD_REMOTE_PROV_SERVER == model_id)||(SIG_MD_REMOTE_PROV_CLIENT == model_id)){
            return 1;
	    }
	    #endif
    }
	
	return 0;
}

void set_dev_key(u8 *dev_key)
{
	memcpy(mesh_key.dev_key, dev_key, 16);
	APP_set_self_dev_key2node_info();
	mesh_key_save();
}

u32 mesh_net_key_get(u8 *list)		// get all net key
{
	mesh_netkey_list_t *p_list = (mesh_netkey_list_t *)list;
	memset(p_list, 0, sizeof(mesh_netkey_list_t));
	u32 cnt = 0;
    unsigned int i;
    for(i = 0; i < (NET_KEY_MAX); ++i){
    	mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
    	if(p_netkey->valid){
			if(cnt & 1){
				SET_KEY_INDEX_H(p_list->key_idx_enc[cnt/2], p_netkey->index);
			}else{
				SET_KEY_INDEX_L(p_list->key_idx_enc[cnt/2], p_netkey->index);
			}
			cnt++;
		}
	}
    
    return cnt;
}



void net_key_del2(mesh_net_key_t *p_key)
{
	foreach_arr(i,p_key->app_key){
		if(p_key->app_key[i].valid){
			app_key_del2(&p_key->app_key[i]);
		}
	}
#if (MD_DF_EN&&!WIN32)
	int key_offset = get_mesh_net_key_offset(p_key->index);
	if(key_offset < NET_KEY_MAX){
		memset(&proxy_mag.directed_server[key_offset], 0x00, sizeof(proxy_mag.directed_server[key_offset]));
		mesh_directed_proxy_capa_report(key_offset);
	}
#endif	

#if (MD_SBR_EN&&!WIN32)
	mesh_subnet_bridge_bind_state_update();
	mesh_model_store(1, SIG_MD_BRIDGE_CFG_SERVER);
#endif
	memset(p_key, 0, sizeof(mesh_net_key_t));
	mesh_key_save();
}

u32 get_all_appkey_cnt()
{
    u8 cnt =0;
	foreach(i,NET_KEY_MAX){
		foreach(j,APP_KEY_MAX){
		    // just compare old key is enough, because appkey_idx of old key is alway valid and same with new, if existed.
			mesh_app_key_t *p = &(mesh_key.net_key[i][0].app_key[j]);
			if(p->valid){
				cnt++;
			}
		}
	}
	return cnt;
}

u32 get_net_key_cnt()
{
	int cnt = 0;
	foreach(i,NET_KEY_MAX){
		if(mesh_key.net_key[i][0].valid){
			cnt++;
		}
	}
    return cnt;
}

int is_exist_valid_network_key()
{
	return (is_provision_success() && get_net_key_cnt());
}

mesh_net_key_t * mesh_get_netkey_by_idx(u16 key_idx)
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
		if((p_netkey->valid)&&(key_idx == p_netkey->index)){
			return p_netkey;
		}
	}
    return 0;
}

mesh_net_key_t * is_mesh_net_key_exist(u16 key_idx)
{
    return mesh_get_netkey_by_idx(key_idx);
}

int get_mesh_net_key_offset(u16 key_idx)
{
	int offset = -1;
	foreach(i,NET_KEY_MAX){
		u32 cnt = is_key_refresh_use_old_and_new_key(i) ? 2 : 1;
    	foreach(k,cnt){
			mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
			if((p_netkey->valid)&&(key_idx == p_netkey->index)){
				return i;
			}
    	}
	}
    return offset;
}

int is_net_key_save()
{
	u32 val = 0;
	flash_read_page(FLASH_ADR_MESH_KEY, 4, (u8 *)&val);
	return (0xffffffff != val);
}

void mesh_private_identity_change_by_proxy_service(mesh_net_key_t *p_netkey)
{
	#if FEATURE_PROXY_EN
	if(p_netkey->node_identity == NODE_IDENTITY_SUB_NET_RUN){
		p_netkey->priv_identity = PRIVATE_NODE_IDENTITY_DISABLE;
	}
	#else
	p_netkey->priv_identity = PRIVATE_NODE_IDENTITY_NOT_SUPPORT;
	#endif
}


void net_key_set2(mesh_net_key_t *key, const u8 *nk, u16 key_idx, int save)
{
	memcpy(key->key, nk, 16);
	mesh_sec_get_nid_ek_pk_master(&key->nid_m, key->ek_m, key->pk_m, key->key);
	mesh_sec_get_nid_ek_pk_directed(&key->nid_d, key->ek_d, key->pk_d, key->key);
	mesh_sec_get_network_id(key->nw_id, key->key);
	mesh_sec_get_identity_key (key->idk, key->key);
	mesh_sec_get_beacon_key (key->bk, key->key);
	mesh_sec_get_privacy_bacon_key(key->prik,key->key);
	#if !WIN32
	mesh_private_identity_change_by_proxy_service(key);
	#endif
	// update friend key later
	
	key->index = key_idx;
	key->valid = KEY_VALID;
	caculate_proxy_adv_hash(key);
#if (MD_DF_EN&&!WIN32)
	if(get_net_key_cnt()>1){
		int key_offset = get_mesh_net_key_offset(key_idx);
		if((UNSET_CLIENT == proxy_mag.proxy_client_type) || (DIRECTED_PROXY_CLIENT == proxy_mag.proxy_client_type)){
			mesh_directed_proxy_capa_report(key_offset);		
		}
	}
#endif
	if(save){
		mesh_key_save();
	}
}

u8 mesh_net_key_set(u16 op, const u8 *nk, u16 key_idx, int save)
{
    u8 st = ST_UNSPEC_ERR;
    
	mesh_net_key_t *p_netkey_exist = is_mesh_net_key_exist(key_idx);
	int same = p_netkey_exist && (!memcmp(p_netkey_exist->key, nk, 16));

	if(NETKEY_ADD == op){
		if(p_netkey_exist){
			st = (same ? ST_SUCCESS : ST_KEYIDX_ALREADY_STORE);
		}else{
			u32 idx = mesh_net_key_empty_search();
			if(idx < NET_KEY_MAX){
				net_key_set2(&mesh_key.net_key[idx][0], nk, key_idx, save);
				st = ST_SUCCESS;
			}else{
				st = ST_INSUFFICIENT_RES;
			}
		}
	}else if(NETKEY_UPDATE == op){
		if(p_netkey_exist){
			mesh_net_key_t *p_net_key_new = p_netkey_exist+1;
			if(KEY_REFRESH_NORMAL == p_netkey_exist->key_phase){	// have not been start key refresh.
				memcpy(p_net_key_new, p_netkey_exist, sizeof(mesh_net_key_t));
				net_key_set2(p_net_key_new, nk, key_idx, 0);
                mesh_friend_key_refresh(p_net_key_new);
				mesh_key_refresh_phase_handle(KEY_REFRESH_PHASE1, p_netkey_exist);
				st = ST_SUCCESS;
			}else{
				st = ST_CAN_NOT_UPDATE;
			}
		}else{
			st = ST_INVALID_NETKEY;
		}
	}else if(NETKEY_DEL == op){
		if(p_netkey_exist){
			if(get_net_key_cnt() <= 1){
				st = ST_CAN_NOT_REMOVE;
			}else{
				net_key_del2(p_netkey_exist);
				#if MD_REMOTE_PROV
				mesh_cmd_sig_rsp_scan_init();
				#endif
				st = ST_SUCCESS;
			}
		}else{
			st = ST_SUCCESS;
		}
	}

    return st;
}

void mesh_net_key_add_by_provision(u8 *nk, u16 key_index, u8 key_fresh_flag)
{
    // factory test key have been clear before, in factory_test_key_bind_(0);
	u8 st = mesh_net_key_set(NETKEY_ADD, nk, key_index, 1);	// must at last, because save in it.
	if(ST_SUCCESS != st){
		mesh_key_flash_sector_init();
		mesh_net_key_set(NETKEY_ADD, nk, key_index, 1);
	}
	
	#if (DEBUG_PREINSTALL_APP_KEY_EN && (!TESTCASE_FLAG_ENABLE) && (0 == DEBUG_MESH_DONGLE_IN_VC_EN))
	mesh_app_key_set_default(key_index, 1);
	#endif

	if(key_fresh_flag){
		mesh_nk_update_self_and_change2phase2(nk, key_index);
	}
	
#if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
    dual_mode_select();
#endif
}

void mesh_sec_get_nid_ek_pk_friend(u8 lpn_idx, u8 *nid, u8 *ek, u8 *pk, u8 *nk)
{
    //k2(NetKey, 0x01 || LPNAddress || FriendAddress || LPNCounter || FriendCounter) // big endianness
    mesh_fri_sec_par_t fri_sec_par;
    fri_sec_par.flag = 1;
    #if FEATURE_LOWPOWER_EN
    fri_sec_par.LPNAdr = mesh_lpn_par.LPNAdr;
    fri_sec_par.FriAdr = mesh_lpn_par.FriAdr;
    fri_sec_par.LPNCounter = mesh_lpn_par.req.LPNCounter;
    fri_sec_par.FriCounter = mesh_lpn_par.offer.FriCounter;
    #elif FEATURE_FRIEND_EN
    fri_sec_par.LPNAdr = fn_other_par[lpn_idx].LPNAdr;
    fri_sec_par.FriAdr = fn_other_par[lpn_idx].FriAdr;
    fri_sec_par.LPNCounter = fn_req[lpn_idx].LPNCounter;
    fri_sec_par.FriCounter = fn_offer[lpn_idx].FriCounter;
    #endif
    endianness_swap_fri_sec_par((u8 *)&fri_sec_par);
    mesh_sec_func_k2 (nid, ek, pk, nk, &fri_sec_par.flag, 9);
}

u8 GetNKArrayIdxByPointer(mesh_net_key_t *key)
{
    u8 nk_arry_idx = (((((u32)key)-((u32)&mesh_key.net_key))/sizeof(mesh_net_key_t))/2)%NET_KEY_MAX;
    return nk_arry_idx;
}

void mesh_friend_key_refresh(mesh_net_key_t *new_key)
{
#if FEATURE_FRIEND_EN || FEATURE_LOWPOWER_EN
    u8 nk_array_idx = GetNKArrayIdxByPointer(new_key);
#endif

#if FEATURE_LOWPOWER_EN
    if(is_friend_ship_link_ok_lpn()){
        friend_key_t * p_fri_key = &mesh_fri_key_lpn[nk_array_idx][1];
        mesh_sec_get_nid_ek_pk_friend(0, &(p_fri_key->nid_f), p_fri_key->ek_f, p_fri_key->pk_f, new_key->key);
    }
#elif FEATURE_FRIEND_EN
    foreach(i,MAX_LPN_NUM){
        if((nk_array_idx == fn_other_par[i].nk_sel_dec_fn) && is_friend_ship_link_ok_fn(i)){
            friend_key_t * p_fri_key = &mesh_fri_key_fn[i][1];
            mesh_sec_get_nid_ek_pk_friend(i, &(p_fri_key->nid_f), p_fri_key->ek_f, p_fri_key->pk_f, new_key->key);
        }
    }
#endif
}

void mesh_seg_ack_poll_rx()
{
	if(is_lpn_support_and_en){
		return ;
	}
	
    if(mesh_rx_seg_par.tick_last && clock_time_exceed(mesh_rx_seg_par.tick_last, SEG_RX_ACK_IDLE_MS*1000)){
        int timeout_flag = 0;
        if(clock_time_exceed(mesh_rx_seg_par.tick_last, SEG_RX_TIMEOUT_MS*1000)){
            timeout_flag = 1;
        }else if(clock_time_exceed(mesh_rx_seg_par.tick_seg_idle, SEG_RX_ACK_IDLE_MS*1000)){
            if(is_seg_block_ack(mesh_rx_seg_par.dst)){
                mesh_rx_seg_par.tick_seg_idle = clock_time()|1;
                mesh_add_seg_ack(&mesh_rx_seg_par);
            }else{
                if(clock_time_exceed(mesh_rx_seg_par.tick_seg_idle, SEG_GROUP_RX_TIMEOUT_MS*1000)){
                    timeout_flag = 1;
                }
            }
        }

        if(timeout_flag){
            mesh_rx_seg_par.tick_last = mesh_rx_seg_par.tick_seg_idle = 0;
            mesh_rx_seg_par.status = SEG_RX_STATE_TIMEOUT;
        }
    }
}

void mesh_seg_ack_poll_tx()
{
    if(!mesh_tx_seg_par.busy){
    	return ;
    }
    
    if(mesh_tx_seg_par.tick_wait_ack && clock_time_exceed(mesh_tx_seg_par.tick_wait_ack, (SEG_TX_ACK_WAIT_MS)*1000)){ // default :(1440+tx cost)ms
        mesh_tx_seg_par.tick_wait_ack = 0;  // wait for next round
        if(clock_time_exceed(mesh_tx_seg_par.timeout, SEG_TX_TIMEOUT_MS*1000)){
            mesh_tx_segment_finished();
			LOG_MSG_INFO(TL_LOG_MESH,0,0,"mesh_seg_ack_poll_tx:segment tx timeout ",0);
			#if RELIABLE_CMD_EN
            if(mesh_tx_reliable.busy && (!mesh_tx_seg_par.ack_received)){
            	mesh_tx_reliable_finish();
            }
            #endif
        }else{
			if(!is_lpn_support_and_en){
            	mesh_rx_seg_ack_handle(1);
            }
			LOG_MSG_INFO(TL_LOG_MESH,0,0,"mesh_seg_ack_poll_tx:segment tx retry ",0);
        }
    }

    #if RELIABLE_CMD_EN
    mesh_tx_reliable_tick_refresh_proc(0, 0);
    #endif
    
    #if VC_CHECK_NEXT_SEGMENT_EN
    VC_check_next_segment_pkt();
    #endif
}

#if RELIABLE_CMD_EN
    #if VC_APP_ENABLE
typedef struct{
	u16 num;
	u16 addr[10];
}mesh_node_retry_st_t;

mesh_node_retry_st_t mesh_node_retry;

void reliable_rsp_check_ll(u16 *addr, u16 cnt)
{
	if(cnt){
		if(cnt > ARRAY_SIZE(mesh_node_retry.addr)){
			cnt = ARRAY_SIZE(mesh_node_retry.addr);
		}
		mesh_node_retry.num = cnt;
		memcpy(mesh_node_retry.addr, addr, cnt*2);
	}
}

void reliable_rsp_check_app()
{
	u16 adr_missing[ARRAY_SIZE(mesh_node_retry.addr)], len;
	extern int vc_check_reliable_rsp(u16 *missing_addr, u16 max_num);
	len = vc_check_reliable_rsp(adr_missing, ARRAY_SIZE(mesh_node_retry.addr));
	
	reliable_rsp_check_ll(adr_missing, len);
}
    #endif

void mesh_tx_reliable_tick_refresh()
{
	mesh_tx_reliable.tick = clock_time();
}

void mesh_tx_reliable_start(u8 retry_cnt)
{
    mesh_tx_reliable.busy = 1;
    mesh_tx_reliable_tick_refresh();
    mesh_tx_reliable.retry_cnt = retry_cnt;
	LOG_MSG_INFO(TL_LOG_MESH,0,0,"mesh_tx_reliable_start:start reliable cmd ",0);
    rf_link_slave_read_status_start();
}

void mesh_tx_reliable_finish()
{
#if VC_APP_ENABLE
		if(mesh_node_retry.num){
			mesh_node_retry.num--;
			mesh_tx_reliable.mat.adr_dst = mesh_node_retry.addr[mesh_node_retry.num];
			mesh_tx_reliable_start(g_reliable_retry_cnt_def);
			return;
		}
#endif

#if FAST_PROVISION_ENABLE
	mesh_fast_prov_reliable_finish_handle();
#endif

#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
    int report_finish = 0;
    u16 op = 0;
    u32 rsp_max = 0, rsp_cnt = 0;
	u16 dst =0;
    if(mesh_tx_reliable.busy){
        report_finish = 1;
        op = mesh_tx_reliable.mat.op;
        rsp_max = mesh_tx_reliable.mat.rsp_max;
        rsp_cnt = mesh_tx_reliable.rsp_cnt;
		dst = mesh_tx_reliable.mat.adr_dst;
	}
#endif
	if(mesh_tx_reliable.busy){
    	if((NODE_RESET == mesh_tx_reliable.mat.op) && is_actived_factory_test_mode()){
    	    client_node_reset_cb(mesh_tx_reliable.mat.adr_dst);
    	}
    }
    
    //if(mesh_tx_reliable.busy){
        memset(&mesh_tx_reliable, 0, sizeof(mesh_tx_reliable));
        rf_link_slave_read_status_stop ();
    //}
    
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__) //gateway will also have the reliable stop report cmd 
    if(report_finish){  // shoulb be after init mesh tx reliable
        extern void mesh_tx_reliable_stop_report(u16 op,u16 adr_dst, u32 rsp_max, u32 rsp_cnt);
        mesh_tx_reliable_stop_report(op, dst,rsp_max, rsp_cnt);
    }
#endif
}

void mesh_tx_reliable_check_and_init(material_tx_cmd_t *p)
{
#if (MD_MESH_OTA_EN && MD_CLIENT_EN)
    if(FW_DISTRIBUT_CANCEL == p->op){
        mesh_tx_reliable_finish();
        memset(&mesh_tx_seg_par, 0, sizeof(mesh_tx_seg_par));    // init
    }else if(fw_distribut_srv_proc.st && is_mesh_ota_tx_client_model(mesh_tx_reliable.match_type.id, mesh_tx_reliable.match_type.sig)){
		mesh_op_resource_t op_res_tx;
		if(0 == mesh_search_model_id_by_op(&op_res_tx, p->op, 1)){
		    if(!is_mesh_ota_tx_client_model(op_res_tx.id, op_res_tx.sig)){
                clr_mesh_ota_master_wait_ack(); // clear current state and try again later, so that it can send other control command at once.
                if(mesh_tx_seg_par.match_type.mat.op == mesh_tx_reliable.match_type.mat.op){
                    memset(&mesh_tx_seg_par, 0, sizeof(mesh_tx_seg_par));    // init
                }
                mesh_tx_reliable_finish();
            }
        }
    }
#endif
}

void mesh_tx_reliable_proc()
{
    if(mesh_tx_reliable.busy){
        if(clock_time_exceed(mesh_tx_reliable.tick, mesh_tx_reliable.invl_ms * 1000)){
			mesh_tx_reliable_tick_refresh();
            if(mesh_tx_reliable.retry_cnt){
				LOG_MSG_LIB(TL_LOG_NODE_BASIC,0,0,"mesh_tx_reliable_proc:retry cnt %d", mesh_tx_reliable.retry_cnt);
                mesh_tx_reliable.retry_cnt--;
                mesh_tx_cmd2_access(&mesh_tx_reliable.mat, 0, &mesh_tx_reliable.match_type);
            }else{            
				#if (VC_APP_ENABLE && !DEBUG_CFG_CMD_GROUP_AK_EN)
				if((mesh_tx_reliable.mat.op != FW_UPDATE_CANCEL) && (mesh_fast_prov_sts_get() ==FAST_PROV_IDLE) ){
				    reliable_rsp_check_app();
				}
				#endif
				#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
				if((mesh_tx_reliable.mat.op == APPKEY_ADD) && key_refresh_cfgcl_proc.fast_bind){
					mesh_cfg_keybind_end_event(MESH_KEYBIND_EVE_TIMEOUT,key_refresh_cfgcl_proc.node_adr);
				}
				#endif
				
                mesh_tx_reliable_finish();
				#if 1 // MD_MESH_OTA_EN, must enable in library
                mesh_ota_master_ack_timeout_handle();
				#endif
            }
        }
        
		rf_link_slave_read_status_update ();
    }
}
#endif

void mesh_friend_key_RevokingOld(mesh_net_key_t *new_key)
{
    #if FEATURE_FRIEND_EN || FEATURE_LOWPOWER_EN
    u8 nk_array_idx = GetNKArrayIdxByPointer(new_key);
    #endif
    foreach(i,MAX_LPN_NUM){
        #if FEATURE_FRIEND_EN
        if(nk_array_idx == fn_other_par[i].nk_sel_dec_fn)
        #endif
        {
            #if FEATURE_LOWPOWER_EN
            friend_key_t * p_fri_key = &mesh_fri_key_lpn[nk_array_idx][0];
            #else
            friend_key_t * p_fri_key = &mesh_fri_key_fn[i][0];
            #endif
            friend_key_t * p_fri_key_new = p_fri_key+1;
        	memcpy(p_fri_key, p_fri_key_new, sizeof(friend_key_t));
        	memset(p_fri_key_new, 0, sizeof(friend_key_t));
    	}
    }
}

void mesh_friend_key_update_all_nk(u8 lpn_idx, u8 nk_arr_idx)	// lpn_idx set to 0 for LPN
{
    foreach_arr(k,mesh_key.net_key[0]){
        mesh_net_key_t *key = &mesh_key.net_key[nk_arr_idx][k];
        if(KEY_UNVALID != key->valid){
            #if FEATURE_LOWPOWER_EN
            friend_key_t * p_fri_key = &mesh_fri_key_lpn[nk_arr_idx][k];
            #else
            friend_key_t * p_fri_key = &mesh_fri_key_fn[lpn_idx][0];
            #endif
            mesh_sec_get_nid_ek_pk_friend(lpn_idx, &(p_fri_key->nid_f), p_fri_key->ek_f, p_fri_key->pk_f, key->key);
        }
    }
	// mesh_key_save();	// no need to save friend key
}

#if 0
int mesh_nid_check(u8 nid)
{
    foreach(i,NET_KEY_MAX){
        if((KEY_UNVALID != mesh_key.net_key[i][0].valid)
        && ((nid == mesh_key.net_key[i][0].nid_m)||(nid == mesh_key.net_key[i][0].nid_f))){
            return 0;
        }
    }
    return -1;
}
#endif

#if 1 // FACTORY_TEST_MODE_ENABLE
u8 factory_test_mode_en = 1;    // user config, default is enable
#if RELIABLE_CMD_EN
u8 factory_test_cache_not_equal_mode_en = 0;
#else
u8 factory_test_cache_not_equal_mode_en = 0;    // must
#endif
u8 mesh_adv_txrx_self_en = 0; //must

const u16 factory_test_model_array[] = {
    SIG_MD_G_ONOFF_S, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_TEMP_S, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_XYL_S,
#if (!DRAFT_FEAT_VD_MD_EN)
    SIG_MD_FW_UPDATE_S,
#endif
};
const u16 factory_test_cfg_op_array[] = {COMPOSITION_DATA_GET, NODE_RESET};

void mesh_adv_txrx_to_self_en(u8 en)
{
	mesh_adv_txrx_self_en = en;
}

int is_actived_factory_test_mode()
{
    return (factory_test_mode_en && (!is_provision_success()));
}

int is_valid_cfg_op_when_factory_test(u16 op)
{
#if (!__PROJECT_MESH_PRO__)
    if(is_actived_factory_test_mode()){
        foreach_arr(i,factory_test_cfg_op_array){
            if(op == factory_test_cfg_op_array[i]){
                return 1;
            }
        }
        return 0;
    }
#endif

    return 1;
}

void mesh_node_refresh_binding_tick()
{
	#if (FEATURE_LOWPOWER_EN || SPIRIT_PRIVATE_LPN_EN)
    if(!lpn_provision_ok)
    #endif
    {
        node_binding_tick = clock_time() | 1;
    }
}

void factory_test_key_bind(int bind_flag)
{
	u16 ak_idx = 0;
	u8 st = ST_SUCCESS;
	if(bind_flag){
        u8 net_key_test[16] = NETKEY_A;
        mesh_net_key_set(NETKEY_ADD, net_key_test, NET0, 0);
        u8 ak[16] = APPKEY_A; // 
	    st = mesh_app_key_set(APPKEY_ADD, ak, ak_idx, NET0, 0);
	}

	if(ST_SUCCESS == st){
		appkey_bind_all(bind_flag, ak_idx, 1);
		node_binding_tick = 0;
	}

	if(!bind_flag){
        memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
	}
}
#endif

void appkey_bind_filter(int bind_flag, u16 ele_adr, u32 model_id, bool4 sig_model, u16 ak_idx, int fac_filter_en)
{
    u16 op = bind_flag ? MODE_APP_BIND : MODE_APP_UNBIND;
    int result = 1;
    if(fac_filter_en){
		if(sig_model){
		    result = 0;
	        foreach_arr(m,factory_test_model_array){
	            if(model_id == factory_test_model_array[m]){
	                result = 1;
	                break;
	            }
	        }
		}else{
		    // support all vendor model
		}
    }
    
    if(result){
        mesh_appkey_bind(op, ele_adr, model_id, sig_model, ak_idx);
    }
}

void appkey_bind_all(int bind_flag, u16 ak_idx, int fac_filter_en)
{
    foreach_arr(i,model_sig_cfg_s_cps.page0.ele_primary.md_sig){
        u16 model_id = model_sig_cfg_s_cps.page0.ele_primary.md_sig[i];
        appkey_bind_filter(bind_flag, ele_adr_primary, model_id, 1, ak_idx, fac_filter_en);
    }

    foreach_arr(i,model_sig_cfg_s_cps.page0.ele_primary.md_vendor){
        u32 model_id = model_sig_cfg_s_cps.page0.ele_primary.md_vendor[i];
        appkey_bind_filter(bind_flag, ele_adr_primary, model_id, 0, ak_idx, fac_filter_en);
    }
    
#if (ELE_CNT > 1)
    foreach_arr(i,model_sig_cfg_s_cps.page0.ele_second){
        mesh_element_second_t * p_ele = &model_sig_cfg_s_cps.page0.ele_second[i];
        u16 ele_adr = ele_adr_primary + 1 + i;
        foreach_arr(k,p_ele->md_sig){
            appkey_bind_filter(bind_flag, ele_adr, p_ele->md_sig[k], 1, ak_idx, fac_filter_en);
        }

        #if (LIGHT_CNT > 1)
        foreach_arr(k,p_ele->md_vendor){
            appkey_bind_filter(bind_flag, ele_adr, p_ele->md_vendor[k], 0, ak_idx, fac_filter_en);
        }
        #endif
    }
#endif
}

u8 mesh_app_key_set_and_bind(u16 netkey_idx, u8 *ak, u16 ak_idx, int save)
{
	u8 st = mesh_app_key_set(APPKEY_ADD, ak, ak_idx, netkey_idx, save);

	if(ST_SUCCESS == st){
		appkey_bind_all(1, ak_idx, 0);
	}
	return st;
}

void mesh_app_key_set_default(u16 netkey_idx, int save)
{
	u8 app_key_test[16] = APPKEY_A; // 
	mesh_app_key_set_and_bind(netkey_idx, app_key_test, NET0_APP0, save);
}

mesh_app_key_t * mesh_app_key_empty_search(mesh_net_key_t *p_netkey)
{
    foreach(i,APP_KEY_MAX){
    	mesh_app_key_t * p_app_key = &p_netkey->app_key[i];
        if(KEY_UNVALID == p_app_key->valid){
            return p_app_key;
        }
    }
    return 0;
}

mesh_app_key_t * is_mesh_app_key_exist(u16 app_key_idx, u16 *p_net_key_idx)
{
    foreach(i,NET_KEY_MAX){
        mesh_net_key_t *key = &mesh_key.net_key[i][0];
        if(KEY_UNVALID != key->valid){
			foreach(i,APP_KEY_MAX){
				mesh_app_key_t *p_appkey = &(key->app_key[i]);
				if((p_appkey->valid)&&(app_key_idx == p_appkey->index)){
					*p_net_key_idx = key->index;
					return p_appkey;
				}
			}
        }
    }
    return 0;
}

void app_key_set2(mesh_app_key_t *p_appkey, const u8 *ak, u16 app_key_idx, int save)
{
	memcpy(p_appkey->key, ak, 16);
	p_appkey->aid = mesh_sec_get_aid(p_appkey->key);
	p_appkey->index = app_key_idx;
	p_appkey->valid = KEY_VALID;

	if(save){
		mesh_key_save();
	}
}

void app_key_del2(mesh_app_key_t *p_appkey)
{
	mesh_unbind_by_del_appkey(p_appkey->index);
	memset(p_appkey->key, 0, sizeof(mesh_app_key_t));
	mesh_key_save();
}

u8 mesh_sub_search_ele_and_set(u16 op, u16 ele_adr, u16 sub_adr, u8 *uuid, u32 model_id, bool4 sig_model)
{
    u8 offset = get_ele_offset_by_model((mesh_page0_t *)(gp_page0), SIZE_OF_PAGE0_LOCAL, ele_adr, ele_adr, model_id, sig_model);
    if(offset == MODEL_NOT_FOUND){
        return ST_INVALID_MODEL;
    }else{
        return mesh_sub_search_and_set(op, ele_adr+offset,sub_adr, uuid, model_id, sig_model);
    }
}

u8 mesh_app_key_set(u16 op, const u8 *ak, u16 app_key_idx, u16 net_key_idx, int save)
{
    u8 st = ST_UNSPEC_ERR;
    u16 net_key_idx_found = -1;
	mesh_app_key_t *p_appkey_exist = is_mesh_app_key_exist(app_key_idx, &net_key_idx_found);
	if(p_appkey_exist){
		mesh_net_key_t * p_netkey = is_mesh_net_key_exist(net_key_idx);
		if(net_key_idx == net_key_idx_found){
			int same = !memcmp(ak, p_appkey_exist->key, 16);
			if(APPKEY_ADD == op){
				st = (same ? ST_SUCCESS : ST_KEYIDX_ALREADY_STORE);
			}else if(APPKEY_UPDATE == op){
				if(KEY_REFRESH_PHASE1 == p_netkey->key_phase){
					u8 *p_ak_new = ((u8 *)p_appkey_exist) + sizeof(mesh_net_key_t);
					app_key_set2((mesh_app_key_t *)p_ak_new, ak, app_key_idx, save);
					st = ST_SUCCESS;
				}else{
					st = ST_CAN_NOT_UPDATE;
				}
			}else if(APPKEY_DEL == op){
				app_key_del2(p_appkey_exist);
				st = ST_SUCCESS;
			}
		}else{
			if((APPKEY_UPDATE == op) && p_netkey){
				st = ST_INVALID_BIND;
			}else{
				st = ST_INVALID_NETKEY;
			}
		}
	}else{
		mesh_net_key_t * p_netkey = is_mesh_net_key_exist(net_key_idx);
		if(p_netkey){
			if(APPKEY_ADD == op){
				if(p_netkey){
					mesh_app_key_t * p_ak_empty = mesh_app_key_empty_search(p_netkey);
					if(p_ak_empty){
						app_key_set2(p_ak_empty, ak, app_key_idx, save);
				        mesh_node_refresh_binding_tick();
					#if PROVISION_FLOW_SIMPLE_EN
						#if DUAL_VENDOR_EN
						if(DUAL_VENDOR_ST_ALI == provision_mag.dual_vendor_st)
						#endif
						{
    						if(!mesh_init_flag){
    						    if(get_all_appkey_cnt() == 1){    						        
                                    ev_handle_traversal_cps(EV_TRAVERSAL_BIND_APPKEY, (u8 *)&app_key_idx);
                                    #if MD_SERVER_EN
                                    // bind share model 
                                    user_set_def_sub_adr();
                                    #endif
    						    }                                         
    						}
						}
					#endif
						st = ST_SUCCESS;
					}else{
						st = ST_INSUFFICIENT_RES;
					}
				}else{
					st = ST_UNSPEC_ERR;
					// while(1);	// should not happen
				}
			}else if(APPKEY_UPDATE == op){
				st = ST_INVALID_APPKEY;
			}else if(APPKEY_DEL == op){
				st = ST_SUCCESS;
			}
		}else{
			st = ST_INVALID_NETKEY;
		}
	}

    #if PROV_APP_KEY_SETUP_TIMEOUT_CHECK_EN
    if((APPKEY_ADD == op) && (st = ST_SUCCESS)){
        prov_app_key_setup_tick = 0;    // kill timer
    }
    #endif
    
    return st;
}

u32 mesh_app_key_get(u8 *list)		// get all app key
{
    u8 st = ST_UNSPEC_ERR;
	mesh_appkey_list_t *p_list = (mesh_appkey_list_t *)list;
	u32 cnt = 0;
    unsigned int i;
    for(i = 0; i < (NET_KEY_MAX); ++i){
    	mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
    	if((p_netkey->valid) && (p_list->netkey_idx == p_netkey->index)){
    		memset(p_list->appkey_idx_enc, 0, sizeof(p_list->appkey_idx_enc));
    		foreach(i,APP_KEY_MAX){
				mesh_app_key_t *p_appkey = &(p_netkey->app_key[i]);
    			if(p_appkey->valid){
    				if(cnt & 1){
    					SET_KEY_INDEX_H(p_list->appkey_idx_enc[cnt/2], p_appkey->index);
    				}else{
    					SET_KEY_INDEX_L(p_list->appkey_idx_enc[cnt/2], p_appkey->index);
    				}
    				cnt++;
    			}
    		}
    		
    		st = ST_SUCCESS;
		    break;
		}
	}

	if(NET_KEY_MAX == i){
		st = ST_INVALID_NETKEY;
	}

    p_list->st = st;
    
    return cnt;
}

mesh_app_key_t * mesh_app_key_search_by_index(u16 netkey_idx, u16 appkey_idx)
{
    foreach(i, NET_KEY_MAX){
    	mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
    	if(p_netkey->valid && (p_netkey->index == netkey_idx)){
    		foreach(k,APP_KEY_MAX){
				mesh_app_key_t *p_appkey = &(p_netkey->app_key[k]);
    			if(p_appkey->valid && (p_appkey->index == appkey_idx)){
    				return p_appkey;
    			}
    		}
		}
	}
	return 0;
}

u8 get_ak_arr_idx(u8 nk_array_idx, u16 appkey_idx) // Note: nk_array_idx is a array index
{
	if(nk_array_idx < NET_KEY_MAX){
		foreach(i,APP_KEY_MAX){
			mesh_app_key_t *p_appkey = &(mesh_key.net_key[nk_array_idx][0].app_key[i]);
			if(p_appkey->valid && (p_appkey->index == appkey_idx)){
				return i;
			}
		}
	}
	return APP_KEY_MAX;
}

u8 get_nk_arr_idx(u16 netkey_idx)
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_netkey = &(mesh_key.net_key[i][0]);
		if(p_netkey->valid && (p_netkey->index == netkey_idx)){
			return i;
		}
	}
	return NET_KEY_MAX;
}

u8 get_ak_arr_idx_first_valid(u8 nk_array_idx)
{
	if(nk_array_idx < NET_KEY_MAX){
		foreach(i,APP_KEY_MAX){
			mesh_app_key_t *p_appkey = &(mesh_key.net_key[nk_array_idx][0].app_key[i]);
			if(p_appkey->valid){
				return i;
			}
		}
	}
	return APP_KEY_MAX;
}

u8 get_nk_arr_idx_first_valid()
{
	foreach(i,NET_KEY_MAX){
		if(mesh_key.net_key[i][0].valid){
			return i;
		}
	}
	return NET_KEY_MAX;
}

mesh_net_key_t *get_nk_first_valid()
{
	foreach(i,NET_KEY_MAX){
		if(mesh_key.net_key[i][0].valid){
			return mesh_key.net_key[i];
		}
	}
	return 0;
}

mesh_net_key_t *get_nk_primary()
{
	mesh_net_key_t *p_nk = is_mesh_net_key_exist(0);   // primary NET key
	if(0 == p_nk){
	    p_nk = get_nk_first_valid(); // should not happen
	}
    return p_nk;
}

void mesh_factory_test_mode_en(u8 en)
{
    factory_test_mode_en = en;
}

int mesh_provision_par_set(u8 *prov_pars)
{
	provison_net_info_str *prov_par;
	prov_par = (provison_net_info_str *)prov_pars;
	if(!is_unicast_adr(prov_par->unicast_address)){
		return -1;
	}else{
	}
	
    if(factory_test_mode_en){ // is_actived_factory_test_mode 
        factory_test_key_bind(0);
    }
	provision_mag.gatt_mode = GATT_PROXY_MODE;
#if (DUAL_VENDOR_EN)
	if(provision_mag.dual_vendor_st != DUAL_VENDOR_ST_MI){
    	provision_mag.dual_vendor_st = DUAL_VENDOR_ST_ALI;
    	mesh_ais_global_var_set();
	}
#endif
    node_need_store_misc = 1;
	memcpy(&provision_mag.pro_net_info,prov_par,sizeof(provison_net_info_str));
	if(ADR_UNASSIGNED != prov_par->unicast_address){
		mesh_set_ele_adr(prov_par->unicast_address);
	}

	mesh_net_key_add_by_provision(prov_par->net_work_key, prov_par->key_index, prov_par->prov_flags.KeyRefresh);
	provision_set_ivi_para(prov_pars);	// must after net key add
	#if MI_API_ENABLE
	// after provision ,need to enter search mode first .
	mesh_iv_update_enter_search_mode();
	#endif
	rf_link_light_event_callback(LGT_CMD_PROV_SUC_EVE);
	provision_mag_cfg_s_store();
	return 0;
}

// used by the app to set the internal provisioner's node info 
int mesh_provision_par_set_dir(u8 *prov_par)
{
	int err =-1;
	memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
	mesh_key_retrieve(); //provision.cfg may change after network retrive
	provison_net_info_str *prov = (provison_net_info_str *)prov_par;
	//restore provision para 
	err = mesh_provision_par_set((u8 *)prov);
	APP_set_self_dev_key2node_info();
	return err;
}
void mesh_provision_par_handle(u8 *net_info)
{
// if open the switch att tab ,should switch the att part 
#if MD_REMOTE_PROV
	// if open the remote prov part ,we need to update the dev random part ,for the devkey should be different
	provision_random_data_init();// update the dev random part .
#endif
#if DEBUG_MESH_DONGLE_IN_VC_EN
	debug_mesh_report_provision_par2usb(net_info);
#else
	mesh_provision_par_set(net_info);  // must at first in this function, because provision_mag.gatt_mode is use later.
#endif

#if ATT_TAB_SWITCH_ENABLE&&!WIN32
	extern void my_att_init(u8 mode);
	my_att_init (provision_mag.gatt_mode);
#endif 
}

u8 mesh_provision_and_bind_self(u8 *p_prov_data, u8 *p_dev_key, u16 appkey_idx, u8 *p_app_key){
	//save device key 
	set_dev_key(p_dev_key);
	
	provison_net_info_str *p_net =(provison_net_info_str *)p_prov_data;
	//save provision flag
	prov_para.provison_rcv_state = STATE_PRO_SUC;
	//save prov data
	mesh_provision_par_handle((u8 *)p_net);
	mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_SUC);
	//cache init
	cache_init(ADR_ALL_NODES);
	
	//add appkey and bind it to models
	return mesh_app_key_set_and_bind(p_net->key_index, p_app_key, appkey_idx, 1);	
}

void mesh_set_ele_adr_ll(u16 adr, int save)
{
	if(/*(ADR_UNASSIGNED == adr) &&*/ (!is_unicast_adr(adr))){
		return ;
	}

	mesh_model_ele_adr_init();
	
    ele_adr_primary = adr;    // for test, should be provisioned later
    mesh_set_all_model_ele_adr();
    #if (!(IS_VC_PROJECT || AIS_ENABLE))
	mesh_scan_rsp_init();// update ele_adr_primary
	#endif

    if(save){
    	mesh_common_save_all_md();			// save all ele_adr in model array
    }
    
    #if ONLINE_STATUS_EN
    mesh_node_online_st_init ();
    device_status_update();
    #endif
}

void mesh_set_ele_adr(u16 adr)
{
	mesh_set_ele_adr_ll(adr, 1);
}

int is_ele_in_node(u16 ele_adr, u16 node_adr, u32 cnt)  // use for provisioner.
{
    #if WIN32
    if((0 == cnt) && (ele_adr == node_adr)){
        LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"element count is invalid: adr:0x%04x, cnt:%d", node_adr, cnt);
        // cnt = 1;
    }
    #endif
    
    return (node_adr && ((ele_adr >= node_adr)&&(ele_adr < node_adr + cnt)));   // 
}

int is_own_ele(u16 adr)
{
    return ((adr >= ele_adr_primary)&&(adr < ele_adr_primary + g_ele_cnt));   // is_ele_in_node_()
}

#define IV_UPDATE_START_SNO             (0xC00000)  // margin should be enough, because sometimes can't keep 96 hour powered. so, should be enough margin to restart iv update flow next power up. 
#define IV_UPDATE_KEEP_TMIE_MIN_S       (96*3600)//(96*3600)       // 96 hour
#define IV_UPDATE_KEEP_TMIE_MIN_RX_S    (96*3600)//(96*3600)       // 96 hour

int is_sno_exhausted()
{
#if MI_API_ENABLE
	static u8 exhaust_flag =1;
	if(exhaust_flag && mesh_adv_tx_cmd_sno >= IV_UPDATE_START_SNO){
		// send sno exhaust
		exhaust_flag = 0;
		mi_cb_ivi_event_send(MI_SNO_OVERFLOW,(u8*)&mesh_adv_tx_cmd_sno);// send only once 
	}
#endif

    return (mesh_adv_tx_cmd_sno >= IV_UPDATE_START_SNO);
}

int is_iv_update_keep_enough_time_ll() // for trigger role
{
    return (iv_idx_st.keep_time_s > IV_UPDATE_KEEP_TMIE_MIN_S);
}

int is_iv_update_keep_enough_time_rx()      // for rx role
{
#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
    return 1;
#else
    return (iv_idx_st.keep_time_s >= IV_UPDATE_KEEP_TMIE_MIN_RX_S);
#endif
}

//receive security network beacon that it's iv index is equal to (current_iv_index + 1) in normal stage.
void mesh_receive_ivi_plus_one_in_normal_cb()
{
#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE || MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
	//set searching mode enter IV Index Recovery procedure
    mesh_iv_update_enter_search_mode();
#elif(MESH_USER_DEFINE_MODE == MESH_MI_ENABLE||MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
	mesh_iv_idx_init(iv_idx_st.cur+1, 1);
#endif	
}
/*
function: no need response when tx reliable command to self.
*/
int is_tx_status_cmd2self(u16 op, u16 adr_dst)
{
//#ifndef WIN32
    #if (!MD_CLIENT_EN)
    if(is_own_ele(adr_dst)){
        mesh_op_resource_t op_resource = {0};
        if(0 == mesh_search_model_id_by_op(&op_resource, op, 1)){
            if(op_resource.status_cmd){
                return 1;
            }
        }
    }
    #endif
//#endif

    return 0;
}

static inline u8 get_rsp_sec_type()
{
    if(FRIENDSHIP == mesh_key.sec_type_sel){
        return MASTER;
    }
    return mesh_key.sec_type_sel;
}

/**
 * @brief  Called after receiving the command, and when it is published
 * @param  op: Opcode.
 * @param  par: Pointer to message data (excluding Opcode).
 * @param  par_len: The length of the message data.
 * @param  adr_src: Source address.
 * @param  adr_dst: Destination address.
 * @param  uuid: When publishing, and the destination address is a virtual 
 *   address, uuid needs to be passed in, otherwise fill in 0.
 * @param  pub_md: When publishing, you need to pass in parameters.
 *   For non-publish, pass in 0.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_tx_cmd_rsp(u16 op, u8 *par, u32 par_len, u16 adr_src, u16 adr_dst, u8 *uuid, model_common_t *pub_md)
{
    if(is_tx_status_cmd2self(op, adr_dst)){
        return 0;
    }
    
    #if 0 // (WIN32 && MD_SERVER_EN)
    #if VC_APP_ENABLE
    if(ble_moudle_id_is_kmadongle()) // there is only client in gateway 
    #endif
    {
        if(!is_own_ele(adr_dst)){
            return 0;
        }
    }
    #endif
    
	material_tx_cmd_t mat;
	u8 ak_array_idx;
	u8 immutable_flag = (MASTER==mesh_key.netkey_sel_dec)?1:0;
	if(pub_md){
		ak_array_idx = get_ak_arr_idx(mesh_key.netkey_sel_dec, pub_md->pub_par.appkey_idx);
		if(pub_md->directed_pub_policy){// publish callback
			immutable_flag = 0;
		}
	}else{
		ak_array_idx = mesh_key.appkey_sel_dec; // use the same key with RX command. 
	}
	
//	LOG_MSG_LIB(TL_LOG_NODE_SDK,par,par_len,"cmd data rsp: adr_src0x%04x,dst adr 0x%04x ",adr_src,adr_dst);	
	set_material_tx_cmd(&mat, op, par, par_len, adr_src, adr_dst, g_reliable_retry_cnt_def, 0, uuid, mesh_key.netkey_sel_dec, ak_array_idx, pub_md, immutable_flag);
	int ret = mesh_tx_cmd_unreliable(&mat);
	return ret;
}

int mesh_tx_cmd_rsp_cfg_model(u16 op, u8 *par, u32 par_len, u16 adr_dst)
{
    if(is_tx_status_cmd2self(op, adr_dst)){
        return 0;
    }
	u8 immutable_flag = (MASTER==mesh_key.netkey_sel_dec)?1:0;
	material_tx_cmd_t mat;
	set_material_tx_cmd(&mat, op, par, par_len, ele_adr_primary, adr_dst, g_reliable_retry_cnt_def, 0, 0, mesh_key.netkey_sel_dec, -1, 0, immutable_flag);
	int ret = mesh_tx_cmd_unreliable(&mat);
	return ret;
}

#if 1   // save par
// common save
#define FLASH_CHECK_SIZE_MAX	(64)
#define SIZE_SAVE_FLAG		(4)

typedef struct{
    u32 adr_base;
    u8 *p_save_par;
    u32 *p_adr;
    u32 size_save_par;		// exclude save flag
}mesh_save_map_t;

#define MODEL_MAX_ONE_SECTOR	(6)

typedef struct{
    bool4 sig_model;
    u32 md_id[MODEL_MAX_ONE_SECTOR];
    u32 adr_base;
}mesh_md_adr_map_t;

static u32 mesh_sw_level_addr = FLASH_ADR_SW_LEVEL;
static u32 mesh_md_cfg_s_addr = FLASH_ADR_MD_CFG_S;
static u32 mesh_md_health_addr = FLASH_ADR_MD_HEALTH;
static u32 mesh_provision_mag_addr = FLASH_ADR_PROVISION_CFG_S;
static u32 mesh_md_g_onoff_level_addr = FLASH_ADR_MD_G_ONOFF_LEVEL;
extern u32 mesh_md_time_schedule_addr;
extern u32 mesh_md_scene_addr;
extern u32 mesh_md_sensor_addr;
extern u32 mesh_md_mesh_ota_addr;

// share area
extern u32 mesh_md_g_power_onoff_addr;
static u32 mesh_md_lightness_addr = FLASH_ADR_MD_LIGHTNESS;
extern u32 mesh_md_light_lc_addr;
extern u32 mesh_md_light_hsl_addr;
#if (LIGHT_TYPE_SEL != LIGHT_TYPE_POWER)
/*static */u32 mesh_md_light_ctl_addr = FLASH_ADR_MD_LIGHT_CTL;
#endif
// share area end
static u32 mesh_md_vd_light_addr = FLASH_ADR_MD_VD_LIGHT;

#if !WIN32
const 
#endif
mesh_md_adr_map_t mesh_md_adr_map[] = {
#if (DUAL_VENDOR_EN)
    {0, {MIOT_SEPC_VENDOR_MODEL_SER, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_VENDOR_MD_SER, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S2}, FLASH_ADR_MD_VD_LIGHT}, // must first
#elif (MI_API_ENABLE)
    {0, {MIOT_SEPC_VENDOR_MODEL_SER, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_VENDOR_MD_SER, -1, -1, -1}, FLASH_ADR_MD_VD_LIGHT}, // must first
#else// (VENDOR_MD_NORMAL_EN)
	{0, {VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S2, -1, -1, -1}, FLASH_ADR_MD_VD_LIGHT}, // must first
#endif
	{1, {SIG_MD_CFG_SERVER, -1, -1, -1, -1, -1}, FLASH_ADR_MD_CFG_S},
	{1, {SIG_MD_HEALTH_SERVER, SIG_MD_HEALTH_CLIENT, -1, -1, -1, -1}, FLASH_ADR_MD_HEALTH},
	{1, {SIG_MD_G_ONOFF_S, SIG_MD_G_ONOFF_C, SIG_MD_G_LEVEL_S, SIG_MD_G_LEVEL_C, -1, -1}, FLASH_ADR_MD_G_ONOFF_LEVEL},
#if (STRUCT_MD_TIME_SCHEDULE_EN)
	{1, {SIG_MD_TIME_S, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, SIG_MD_SCHED_S, SIG_MD_SCHED_C, SIG_MD_SCHED_SETUP_S}, FLASH_ADR_MD_TIME_SCHEDULE},
#endif
#if (MD_SCENE_EN)
	{1, {SIG_MD_SCENE_S, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_SCENE},
#endif
#if (MD_MESH_OTA_EN)
    {!DRAFT_FEAT_VD_MD_EN, {SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C, SIG_MD_FW_DISTRIBUT_S, SIG_MD_FW_DISTRIBUT_C, SIG_MD_BLOB_TRANSFER_S, SIG_MD_BLOB_TRANSFER_C}, FLASH_ADR_MD_MESH_OTA},
#endif
#if (STRUCT_MD_DEF_TRANSIT_TIME_POWER_ONOFF_EN)
	{1, {SIG_MD_G_POWER_ONOFF_S, SIG_MD_G_POWER_ONOFF_C, SIG_MD_G_POWER_ONOFF_SETUP_S, SIG_MD_G_DEF_TRANSIT_TIME_S, SIG_MD_G_DEF_TRANSIT_TIME_C, -1}, FLASH_ADR_MD_G_POWER_ONOFF},
#endif
#if MD_REMOTE_PROV
    {!DRAFT_FEAT_VD_MD_EN, {SIG_MD_REMOTE_PROV_SERVER, SIG_MD_REMOTE_PROV_CLIENT, -1, -1, -1, -1}, FLASH_ADR_MD_REMOTE_PROV},
#endif
#if MD_PRIVACY_BEA
	{1, {SIG_MD_PRIVATE_BEACON_SERVER, SIG_MD_PRIVATE_BEACON_CLIENT, -1, -1, -1, -1}, FLASH_ADR_MD_PRIVATE_BEACON},
#endif
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
	{1, {SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_LIGHTNESS},
#else
	{1, {SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_LIGHTNESS},
	#if MD_LIGHT_CONTROL_EN
	{1, {SIG_MD_LIGHT_LC_S, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_LIGHT_LC},
	#endif
	#if (LIGHT_TYPE_CT_EN)
	{1, {SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_SETUP_S, SIG_MD_LIGHT_CTL_TEMP_S, -1, -1}, FLASH_ADR_MD_LIGHT_CTL},
    #endif
	#if (LIGHT_TYPE_HSL_EN)
	{1, {SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SETUP_S, SIG_MD_LIGHT_HSL_HUE_S, SIG_MD_LIGHT_HSL_SAT_S, -1}, FLASH_ADR_MD_LIGHT_HSL},
    #endif
	#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
	{1, {SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_LIGHT_CTL},
	#endif
#endif
#if(MD_PROPERTY_EN)
	{1, {SIG_MD_G_ADMIN_PROP_S, SIG_MD_G_MFG_PROP_S, SIG_MD_G_USER_PROP_S, SIG_MD_G_CLIENT_PROP_S, SIG_MD_G_PROP_C, -1}, FLASH_ADR_MD_PROPERTY},
#endif
#if(MD_LOCATION_EN)
	{1, {SIG_MD_G_LOCATION_S, SIG_MD_G_LOCATION_SETUP_S, SIG_MD_G_LOCATION_C, -1, -1, -1}, FLASH_ADR_MD_SENSOR},
#elif((MD_SENSOR_EN || MD_BATTERY_EN))
	{1, {SIG_MD_SENSOR_S, SIG_MD_SENSOR_SETUP_S, SIG_MD_SENSOR_C, SIG_MD_G_BAT_S, SIG_MD_G_BAT_C, -1}, FLASH_ADR_MD_SENSOR},
#elif((MD_DF_EN || MD_SBR_EN ))
	{1, {SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C, SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT, -1, -1}, FLASH_ADR_MD_DF_SBR},
#endif
};

const mesh_save_map_t mesh_save_map[] = {
	{FLASH_ADR_SW_LEVEL, (u8 *)&light_res_sw_save, &mesh_sw_level_addr, sizeof(light_res_sw_save)},
	{FLASH_ADR_MD_CFG_S, (u8 *)&model_sig_cfg_s, &mesh_md_cfg_s_addr, sizeof(model_sig_cfg_s)},
	{FLASH_ADR_MD_HEALTH, (u8 *)&model_sig_health, &mesh_md_health_addr, sizeof(model_sig_health)},
	{FLASH_ADR_MD_G_ONOFF_LEVEL, (u8 *)&model_sig_g_onoff_level, &mesh_md_g_onoff_level_addr, sizeof(model_sig_g_onoff_level)},
#if (STRUCT_MD_TIME_SCHEDULE_EN)
	{FLASH_ADR_MD_TIME_SCHEDULE, (u8 *)&model_sig_time_schedule, &mesh_md_time_schedule_addr, sizeof(model_sig_time_schedule)},
#endif
	{FLASH_ADR_PROVISION_CFG_S, (u8 *)&provision_mag, &mesh_provision_mag_addr, sizeof(provision_mag)},
#if MD_REMOTE_PROV
    //{FLASH_ADR_MD_REMOTE_PROV, (u8 *)&model_remote_prov, &mesh_md_rp_addr, sizeof(model_remote_prov)},
#endif
#if MD_PRIVACY_BEA
	{FLASH_ADR_MD_PRIVATE_BEACON, (u8 *)&model_private_beacon, &mesh_md_pri_beacon_addr, sizeof(model_remote_prov)},
#endif
#if MD_SCENE_EN
	{FLASH_ADR_MD_SCENE, (u8 *)&model_sig_scene, &mesh_md_scene_addr, sizeof(model_sig_scene)},
#endif
#if MD_MESH_OTA_EN
    {FLASH_ADR_MD_MESH_OTA, (u8 *)&model_mesh_ota, &mesh_md_mesh_ota_addr, sizeof(model_mesh_ota)},
#endif

#if STRUCT_MD_DEF_TRANSIT_TIME_POWER_ONOFF_EN
	{FLASH_ADR_MD_G_POWER_ONOFF, (u8 *)&model_sig_g_power_onoff, &mesh_md_g_power_onoff_addr, sizeof(model_sig_g_power_onoff)},
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
	{FLASH_ADR_MD_LIGHTNESS, (u8 *)&model_sig_lightness, &mesh_md_lightness_addr, sizeof(model_sig_lightness)},
#else
	{FLASH_ADR_MD_LIGHTNESS, (u8 *)&model_sig_lightness, &mesh_md_lightness_addr, sizeof(model_sig_lightness)},
	#if MD_LIGHT_CONTROL_EN
	{FLASH_ADR_MD_LIGHT_LC, (u8 *)&model_sig_light_lc, &mesh_md_light_lc_addr, sizeof(model_sig_light_lc)},
	#endif
	#if (LIGHT_TYPE_CT_EN)
	{FLASH_ADR_MD_LIGHT_CTL, (u8 *)&model_sig_light_ctl, &mesh_md_light_ctl_addr, sizeof(model_sig_light_ctl)},
    #endif
	#if (LIGHT_TYPE_HSL_EN)
	{FLASH_ADR_MD_LIGHT_HSL, (u8 *)&model_sig_light_hsl, &mesh_md_light_hsl_addr, sizeof(model_sig_light_hsl)},
    #endif
	#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
	{FLASH_ADR_MD_LIGHT_CTL, (u8 *)&model_sig_light_xyl, &mesh_md_light_ctl_addr, sizeof(model_sig_light_xyl)},
	#endif
#endif
#if(MD_SENSOR_EN || MD_BATTERY_EN || MD_LOCATION_EN)  
	{FLASH_ADR_MD_SENSOR, (u8 *)&model_sig_sensor, &mesh_md_sensor_addr, sizeof(model_sig_sensor)},
#endif
#if(MD_PROPERTY_EN)
	{FLASH_ADR_MD_PROPERTY, (u8 *)&model_sig_property, &mesh_md_property_addr, sizeof(model_sig_property)},
#elif((MD_DF_EN || MD_SBR_EN))
	{FLASH_ADR_MD_DF_SBR, (u8 *)&model_sig_g_df_sbr_cfg, &mesh_md_df_sbr_addr, sizeof(model_sig_g_df_sbr_cfg)},
#endif
	{FLASH_ADR_MD_VD_LIGHT, (u8 *)&model_vd_light, &mesh_md_vd_light_addr, sizeof(model_vd_light)},
#if (ALI_MD_TIME_EN)
	{FLASH_ADR_VD_TIME_INFO, (u8 *)&model_vd_ali_time, &mesh_md_vd_ali_time_addr, sizeof(model_vd_ali_time)},
#endif
};

#if WIN32
void APP_set_vd_id_cps(u16 vd_id)
{
    foreach_arr(i, model_sig_cfg_s_cps.page0.ele_primary.md_vendor){
        mesh_element_primary_t *p = &model_sig_cfg_s_cps.page0.ele_primary;
        p->md_vendor[i] &= 0xffff0000;
        p->md_vendor[i] |= vd_id;
    }
}

void APP_set_vd_id_mesh_save_map(u16 vd_id)
{
    mesh_md_adr_map_t *p_map = &mesh_md_adr_map[0];
    foreach_arr(i,mesh_md_adr_map[0].md_id){
        if(p_map->md_id[i] == VENDOR_MD_LIGHT_S){
            p_map->md_id[i] = g_vendor_md_light_vc_s;
		#if MD_VENDOR_2ND_EN
        }if(p_map->md_id[i] == VENDOR_MD_LIGHT_S2){
            p_map->md_id[i] = g_vendor_md_light_vc_s2;
		#endif
        }else if(p_map->md_id[i] == VENDOR_MD_LIGHT_C){
            p_map->md_id[i] = g_vendor_md_light_vc_c;
        }
    }
    APP_set_vd_id_cps(vd_id);
}
#endif

/*
	void mesh_par_retrieve():
	input parameters:
	out: just parameters, no save flag
*/
int mesh_par_retrieve(u8 *out, u32 *p_adr, u32 adr_base, u32 size){
#if WIN32 
    return mesh_par_retrieve_store_win32(out, p_adr, adr_base, size,MESH_PARA_RETRIEVE_VAL);
#else
    int err = -1;
    for(int i = 0; i < (FLASH_SECTOR_SIZE); i += (size + SIZE_SAVE_FLAG)){
		*p_adr = adr_base + i;

        u8 save_flag;
        flash_read_page(*p_adr, 1, &save_flag);
		if(SAVE_FLAG == save_flag){
            flash_read_page(*p_adr + SIZE_SAVE_FLAG, size, out);
            err = 0;
		}else if (SAVE_FLAG_PRE == save_flag){
			continue;
		}else if(save_flag != 0xFF){
		    continue;   //invalid
		}else{
		    break;      // 0xFF: end
		}
	}
	return err;
#endif
}

static u16 flash_write_err_cnt;
int mesh_flash_write_check(u32 adr, const u8 *in, u32 size)
{
	u8 data_read[FLASH_CHECK_SIZE_MAX];
	u32 len = size;
	u32 pos = 0;
	while(len){
		// just only 2 bytes different generally.because it just happen when add / del subscription adr.
		u32 len_read = len > FLASH_CHECK_SIZE_MAX ? FLASH_CHECK_SIZE_MAX : len;
		u32 adr_read = adr + pos;
		const u8 *p_in = in+pos;
		flash_read_page(adr_read, len_read, data_read);
		if(memcmp(data_read, p_in, len_read)){
			flash_write_err_cnt++;
			return -1;
		}
		
		len -= len_read;
		pos += len_read;
	}
	return 0;
}

void mesh_flash_write_replace(u32 adr, const u8 *in, u32 size)
{
	u8 data_read[FLASH_CHECK_SIZE_MAX];
	u32 len = size;
	u32 pos = 0;
	while(len){
		// just only 2 bytes different generally.because it just happen when add / del subscription adr.
		u32 len_read = len > FLASH_CHECK_SIZE_MAX ? FLASH_CHECK_SIZE_MAX : len;
		u32 adr_read = adr + pos;
		const u8 *p_in = in+pos;
		flash_read_page(adr_read, len_read, data_read);
		foreach(i,len_read){
			if(data_read[i] != p_in[i]){
				flash_write_page(adr_read+i, 1, (u8 *)&p_in[i]);
			}
		}
		
		len -= len_read;
		pos += len_read;
	}
	mesh_flash_write_check(adr, in, size);
}

void flash_write_with_check(u32 adr, u32 size, const u8 *in)
{
	flash_write_page(adr, size, (u8 *)in);
	mesh_flash_write_check(adr, in, size);
}

enum{
	FLASH_CMP_DIFF_AFTER_AND	 = 0,
	FLASH_CMP_ALL_SAME,
	FLASH_CMP_SAME_AFTER_AND,
};

int mesh_flash_data_cmp(u32 adr, const u8 *in, u32 size)
{
	u8 data_read[FLASH_CHECK_SIZE_MAX];
	int same = 1;
	u32 len = size;
	u32 pos = 0;
	while(len){
		u32 len_read = len > FLASH_CHECK_SIZE_MAX ? FLASH_CHECK_SIZE_MAX : len;
		u32 adr_read = adr + pos;
		const u8 *p_in = in + pos;
		flash_read_page(adr_read, len_read, data_read);
		if(memcmp(data_read, p_in, len_read)){
			same = 0;
			foreach(i,len_read){
				data_read[i] &= p_in[i];
				if(data_read[i] != p_in[i]){
					return FLASH_CMP_DIFF_AFTER_AND;
				}
			}
		}else{
			if((len <= FLASH_CHECK_SIZE_MAX) && same){
				return FLASH_CMP_ALL_SAME;
			}
		}
		
		len -= len_read;
		pos += len_read;
	}

	return FLASH_CMP_SAME_AFTER_AND;
}

/*
	void mesh_par_store((u8 *in, u32 *p_adr, u32 adr_base, u32 size)):
	input parameters:
	in: just parameters, no save flag
*/
void mesh_par_write_with_check(u32 addr, u32 size, const u8 *in)
{
	// make sure when change to the save_flag ,the flash must be write already .
	u32 save_flag = SAVE_FLAG_PRE;
	flash_write_with_check(addr, SIZE_SAVE_FLAG, (u8 *)&save_flag);
	flash_write_with_check(addr + SIZE_SAVE_FLAG, size, in);
	save_flag = SAVE_FLAG;
	flash_write_with_check(addr, SIZE_SAVE_FLAG, (u8 *)&save_flag);
}

void mesh_par_store(const u8 *in, u32 *p_adr, u32 adr_base, u32 size){
#if WIN32 
    mesh_par_retrieve_store_win32((u8 *)in, p_adr, adr_base, size,MESH_PARA_STORE_VAL);
#else
	u32 size_save = (size + SIZE_SAVE_FLAG);
	if(*p_adr > (adr_base + FLASH_SECTOR_SIZE - size_save - SIZE_SAVE_FLAG)){	// make sure "0xffffffff" at the last for retrieve
        *p_adr = adr_base;
		if(FLASH_ADR_MISC == adr_base){	//backup to prevent unexpected power down
			mesh_par_write_with_check(FLASH_ADR_FRIEND_SHIP, size, in);
		}
        flash_erase_sector(adr_base);
	}
	u32 adr_last = adr_base;
    if(*p_adr >= adr_base + size_save){
    	adr_last = *p_adr - size_save;
		foreach_arr(i,mesh_save_map){
			if(adr_base == mesh_save_map[i].adr_base){
				u32 adr_par_last = adr_last+SIZE_SAVE_FLAG;
				int ret = mesh_flash_data_cmp(adr_par_last, in, size);
				if(FLASH_CMP_ALL_SAME == ret){
					return ;
				}
				break;
			}
		}
    }
	
	mesh_par_write_with_check(*p_adr, size, in);
	if(*p_adr >= adr_base + size_save){
		u32 zero = 0;
    	flash_write_page(adr_last, SIZE_SAVE_FLAG, (u8 *)&zero);  // clear last flag
	}

	if(FLASH_ADR_MISC == adr_base){
		u32 tmp = 0;
		flash_read_page(FLASH_ADR_FRIEND_SHIP, sizeof(tmp), (u8 *)&tmp);
		if(U32_MAX != tmp){
			flash_erase_sector(FLASH_ADR_FRIEND_SHIP);	// always erase misc backup sector
		}		
	}
	*p_adr += size_save;
#endif
}

int mesh_common_retrieve_by_index(u8 index){
	const mesh_save_map_t *p_map = &mesh_save_map[index];
	int err = mesh_par_retrieve(p_map->p_save_par, p_map->p_adr, p_map->adr_base, p_map->size_save_par);
    
    mesh_common_retrieve_cb(err, p_map->adr_base);
	return err;
}

int mesh_common_store_by_index(u8 index){
	const mesh_save_map_t *p_map = &mesh_save_map[index];
    mesh_par_store(p_map->p_save_par, p_map->p_adr, p_map->adr_base, p_map->size_save_par);
    return 0;	// err
}

int mesh_common_retrieve_and_store(u32 adr_base, int save)
{
	int err = -1;
	foreach_arr(i,mesh_save_map){
		if((adr_base == mesh_save_map[i].adr_base)){
			if(save){
				mesh_common_store_by_index(i);
				return 0;
			}else{
				return mesh_common_retrieve_by_index(i);
			}
		}
	}
	return err;
}

int mesh_common_retrieve(u32 adr_base)
{
	return mesh_common_retrieve_and_store(adr_base, 0);
}

int mesh_common_store(u32 adr_base)
{
	return mesh_common_retrieve_and_store(adr_base, 1);
}

void mesh_common_retrieve_all()
{
	foreach_arr(i,mesh_save_map){
		mesh_common_retrieve_by_index(i);
	}
	
	#if (DUAL_VENDOR_EN)
    vendor_id_check_and_update();
	#endif
	// should be after retrieve all model to make sure cover the val read from flash. 
	mesh_set_ele_adr_ll(ele_adr_primary, 0);  // for test, should be provisioned later
}

void mesh_common_save_all_md()
{
	foreach_arr(i,mesh_md_adr_map){
		mesh_common_store(mesh_md_adr_map[i].adr_base);
	}
}

void mesh_common_retrieve_cb(int err, u32 adr_base){
	if(FLASH_ADR_MD_CFG_S == adr_base){
	    if(!err){
	        ele_adr_primary = model_sig_cfg_s.com.ele_adr;
	    }
    }
}

void mesh_common_reset_all()
{
	u8 reset;
	flash_read_page(FLASH_ADR_MESH_KEY, sizeof(reset), &reset);
	
	foreach_arr(i,mesh_save_map){
		const mesh_save_map_t *p_map = &mesh_save_map[i];		
		
		if(0xff == reset){
			*p_map->p_adr =  p_map->adr_base;
		}
		if(p_map->adr_base != FLASH_ADR_PROVISION_CFG_S){
			memset(p_map->p_save_par, 0x00,p_map->size_save_par);
		}
	}
	
	// addr not in mesh_save_map[]
	flash_read_page(FLASH_ADR_MESH_KEY, sizeof(reset), &reset);
	if(0xff == reset){
		mesh_key_addr = FLASH_ADR_MESH_KEY;
		node_need_store_misc = 1; 
	}
	else{
		node_need_store_misc = 0;// if have mesh info, don't store iv index 
	}
	
	flash_read_page(FLASH_ADR_MISC, sizeof(reset), &reset);
	if(0xff == reset){
		mesh_misc_addr = FLASH_ADR_MISC;
	}
	
	mesh_global_var_init();
	mesh_set_ele_adr_ll(ele_adr_primary, 0);
	mesh_par_retrieve((u8 *)&light_res_sw_save, &mesh_sw_level_addr, FLASH_ADR_SW_LEVEL, sizeof(light_res_sw_save));//retrive light_res_sw_save
	mesh_par_retrieve((u8 *)&provision_mag, &mesh_provision_mag_addr, FLASH_ADR_PROVISION_CFG_S, sizeof(provision_mag));//retrive oob
}

STATIC_ASSERT((sizeof(misc_save_t)+SIZE_SAVE_FLAG)% 16 == 0);
void mesh_misc_store(){
	if(!node_need_store_misc){
		return;
	}
	
    misc_save_t misc_save = {0};
    if(1){//((mesh_adv_tx_cmd_sno <= 10)||(mesh_adv_tx_cmd_sno >= (mesh_adv_tx_cmd_sno_last + MESH_CMD_SNO_SAVE_DELTA))){
        misc_save.sno = mesh_adv_tx_cmd_sno_last = mesh_adv_tx_cmd_sno;
    }/*else{
        misc_save.sno = mesh_adv_tx_cmd_sno_last;   // not save sno when save other parameters should be better ?.
      }*/
    misc_save.ct_flag = ct_flag;
	memcpy(misc_save.iv_index, iv_idx_st.cur, 4);
    if(IV_UPDATE_STEP1 == iv_idx_st.update_proc_flag){
        mesh_decrease_ivi(misc_save.iv_index);// must decrease 1, if not, when power restart will make iv index error if sno is greater than 0xc00000.
        if(!is_sno_exhausted()){
            misc_save.iv_update_trigger_flag = 1;
        }
	}else{
        misc_save.iv_update_trigger_flag = iv_idx_st.update_trigger_by_save;    // must, because store in mesh_misc_retrieve_
	}
    // in the win32 mode will proc it directly 
#if WIN32
	#if JSON_FILE_ENABLE
	mesh_json_update_ivi_index(iv_idx_st.cur);  // save misc_save.iv_update_trigger_flag to json later
	#endif
#endif

    // in the gateway mode 
#if DONGLE_PROVISION_EN
    gateway_upload_ividx(iv_idx_st.cur);
#endif
    mesh_par_store((u8 *)&misc_save, &mesh_misc_addr, flash_adr_misc, sizeof(misc_save_t));
}


int mesh_misc_retrieve(){
    misc_save_t misc_save;
	
	int err = mesh_par_retrieve((u8 *)&misc_save, &mesh_misc_addr, flash_adr_misc, sizeof(misc_save_t));
	if(err){
		u32 back_sector = FLASH_ADR_FRIEND_SHIP;
		err = mesh_par_retrieve((u8 *)&misc_save, &back_sector, FLASH_ADR_FRIEND_SHIP, sizeof(misc_save_t));
	}
	
    if(!err){
        mesh_adv_tx_cmd_sno_last = mesh_adv_tx_cmd_sno = misc_save.sno + MESH_CMD_SNO_SAVE_DELTA;
        if(!is_actived_factory_test_mode()){
            ct_flag = misc_save.ct_flag;
        }
		u32 temp = -1;
		if(memcmp(misc_save.iv_index, &temp, 4)){	// for compatibility
			memcpy(iv_idx_st.cur, misc_save.iv_index, 4);
		}
		iv_idx_st.update_trigger_by_save = misc_save.iv_update_trigger_flag;
		
    }
	misc_flag =1;//need to store the sno later 
    return err;
}

void mesh_sno_save_check()
{
    if(mesh_adv_tx_cmd_sno > (mesh_adv_tx_cmd_sno_last + MESH_CMD_SNO_SAVE_DELTA)){
        mesh_misc_store();
    }
}

#if DEBUG_IV_UPDATE_TEST_EN
void iv_index_read_print_test()
{
    misc_save_t misc_save;
    int err = mesh_par_retrieve((u8 *)&misc_save, &mesh_misc_addr, flash_adr_misc, sizeof(misc_save_t));
    if(err){
        LOG_MSG_INFO(TL_LOG_IV_UPDATE,0, 0,"IV index Read flash error:",0);
    }else{
        LOG_MSG_INFO(TL_LOG_IV_UPDATE,misc_save.iv_index, 4,"sno:0x%06x, IV index in flash:",mesh_adv_tx_cmd_sno);
        LOG_MSG_INFO(TL_LOG_IV_UPDATE,(u8 *)&iv_idx_st, sizeof(iv_idx_st),"IV index in RAM:",0);
    }
}

void iv_index_set_keep_time_test()
{
    if(iv_idx_st.keep_time_s < 0x60000){
        iv_idx_st.keep_time_s = 0x60000;
	    LOG_MSG_INFO(TL_LOG_IV_UPDATE,0, 0,"iv_index_set_keep_time set 0x60000",0);
    }else{
	    LOG_MSG_INFO(TL_LOG_IV_UPDATE,0, 0,"already set keep time ok before",0);
    }
}

void iv_index_set_sno_test()
{
    #if 0
    mesh_tx_sec_private_beacon_proc(1);
    #else
    if(mesh_adv_tx_cmd_sno < 0xc10000){
        mesh_adv_tx_cmd_sno = 0xc10000;
	    LOG_MSG_INFO(TL_LOG_COMMON,0, 0,"mesh_adv_tx_cmd_sno set 0xc10000",0);
	}else{
	    LOG_MSG_INFO(TL_LOG_COMMON,0, 0,"already set sno ok before",0);
    }
    #endif
}

enum{
    IV_TEST_BUTTON_NULL             = 0,
    IV_TEST_BUTTON_SET_SNO          = 1,
    IV_TEST_BUTTON_SET_KEEP_TIME    = 2,
    IV_TEST_BUTTON_READ             = 0x0d,
};

void iv_index_test_button_firmware()
{
    static volatile u8 A_1button;
    if(A_1button){
        if(IV_TEST_BUTTON_READ == A_1button){
            iv_index_read_print_test();
        }else if(IV_TEST_BUTTON_SET_SNO == A_1button){
            iv_index_set_sno_test();
        }else if(IV_TEST_BUTTON_SET_KEEP_TIME == A_1button){
            iv_index_set_keep_time_test();
        }else{
            // skip
        }
        A_1button = 0;
    }
}
#endif

// mesh key save
STATIC_ASSERT(APP_KEY_MAX < 16);
STATIC_ASSERT(sizeof(mesh_key_save_t) % 16 == 0);


#if KEY_SAVE_ENCODE_ENABLE
#define SAVE_ENCODE_SK_USER         "0123456789ABCDEF"    // MAX 16 BYTE

int encode_decode_password(u8 *pd, u32 pd_len, u32 mic_len, int encode)
{
    u8 sk_user[16+1] = {SAVE_ENCODE_SK_USER};
    if(encode){
        encode_password_ll(sk_user, pd, pd_len, mic_len);
        return 0;
    }else{
        return decode_password_ll(sk_user, pd, pd_len, mic_len);
    }
}
#endif

void mesh_key_flash_sector_init()
{
	mesh_key_addr = FLASH_ADR_MESH_KEY;
	flash_erase_sector(FLASH_ADR_MESH_KEY);
	memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
}

/**
 * @brief  node identity initialization, check the Node Identity state,
 *         and set the timer stamp for reference
 * @retval None
 */
void mesh_key_node_identity_init() // after power on ,we need to detect the flag ,and set timer part 
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_in = &mesh_key.net_key[i][0];
		if(p_in->valid){
			if(p_in->node_identity == NODE_IDENTITY_SUB_NET_RUN){
				p_in->start_identity_s = clock_time_s();
			}
		}
	}
	return ;
}

/**
 * @brief  after gatt provision, set the Node Identity state Enabled, and set the time stamp
 *         for the duration of the Node Identity Advertisement
 * @retval None
 */
void mesh_key_node_identity_set_prov_set()// after gatt provision ,use this fun for setting part 
{
	u8 identity_trigger =0;
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_in = &mesh_key.net_key[i][0];
		if(p_in->valid){
			p_in->start_identity_s = clock_time_s();
			if(p_in->node_identity != NODE_IDENTITY_SUB_NET_RUN){
				p_in->node_identity = NODE_IDENTITY_SUB_NET_RUN;
				identity_trigger++;
			}
		}
	}
	if(identity_trigger){
		mesh_key_save();
	}
	return ;
}

/*
void mesh_key_node_identity_prov_set(u8 identity,u8 netkey_idx)// client send node identity cfg cmd to set 
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_in = &mesh_key.net_key[i][0];
		if(p_in->valid && (p_in->index == netkey_idx)){
			if(identity == NODE_IDENTITY_SUB_NET_RUN){
				p_in->start_identity_s = clock_time_s();
			}
			if(p_in->node_identity != identity){
				p_in->node_identity = identity;
				mesh_key_save();	
			}	
			return ;	
		}
	}
	return ;
}
*/

/**
 * @brief  This is running in main loop to handle the Node/Network Identity switch,
 *         ref: MshPRFv1.0.1 7.2.2.2.3.
 *         In between the state, the Node Identity shouldn't not transmit
 *         more than 60s, the NODE_IDENTITY_TIMEOUT_S defines the duration.
 * @retval None
 */
void mesh_switch_identity_proc()// run in loop
{
	u8 identity_trigger =0;
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_in = &mesh_key.net_key[i][0];
		if(p_in->valid && p_in->node_identity == NODE_IDENTITY_SUB_NET_RUN){
			if(clock_time_exceed_s(p_in->start_identity_s,NODE_IDENTITY_TIMEOUT_S)){
				identity_trigger++;
				p_in->node_identity =NODE_IDENTITY_SUB_NET_STOP;
			}
		}
		if(p_in->valid && p_in->priv_identity == PRIVATE_NODE_IDENTITY_ENABLE){
			if(clock_time_exceed_s(p_in->priv_identity_s,NODE_IDENTITY_TIMEOUT_S)){
				identity_trigger++;
				p_in->priv_identity_s =PRIVATE_NODE_IDENTITY_DISABLE;
			}
		}
	}
	if(identity_trigger){
		mesh_key_save();
	}
	return ;
}


void mesh_key_save()
{
    mesh_key_save_t key_save = {{0}};
    memcpy(key_save.dev_key, mesh_key.dev_key,16);

    foreach(i,NET_KEY_MAX){
    	mesh_net_key_save_t *p_save = &key_save.net_key[i];
    	mesh_net_key_t *p_in = &mesh_key.net_key[i][0];
    	if(p_in->valid){
    		p_save->valid = p_in->valid;
    		p_save->index = p_in->index;
    		p_save->node_identity = p_in->node_identity;
			p_save->priv_identity = p_in->priv_identity;
		    memcpy(p_save->key, p_in->key, 16);
		    foreach(j,APP_KEY_MAX){
				if(p_in->app_key[j].valid){
					p_save->app_key[j].valid = p_in->app_key[j].valid;
					p_save->app_key[j].index = p_in->app_key[j].index;
		    		memcpy(p_save->app_key[j].key, p_in->app_key[j].key,16);
		    	}
		    }
	    }
    }

    #if KEY_SAVE_ENCODE_ENABLE
    encode_decode_password((u8 *)&key_save, OFFSETOF(mesh_key_save_t,mic_val), sizeof(key_save.mic_val), 1);
    key_save.encode_flag = KEY_SAVE_ENCODE_FLAG;
    #endif
    mesh_par_store((u8 *)&key_save, &mesh_key_addr, FLASH_ADR_MESH_KEY, sizeof(key_save));
}

int mesh_key_retrieve(){
    mesh_key_save_t key_save = {{0}};
    int err = mesh_par_retrieve((u8 *)&key_save, &mesh_key_addr, FLASH_ADR_MESH_KEY, sizeof(key_save));
    
    #if KEY_SAVE_ENCODE_ENABLE
    if((0 == err) && KEY_SAVE_ENCODE_FLAG == key_save.encode_flag){
        err = encode_decode_password((u8 *)&key_save, OFFSETOF(mesh_key_save_t,encode_flag), sizeof(key_save.mic_val), 0);
    }
    #endif

	if(err){	// key init
		#if 1 // DEBUG_MESH_DONGLE_IN_VC_EN // DEBUG_PREINSTALL_APP_KEY_EN
		if(factory_test_mode_en){
    		factory_test_key_bind(1);
    		// primary adr was set in mesh_common_retrieve_cb()
		}
		#endif
	}else{		// load and recalculate
		memcpy(mesh_key.dev_key, key_save.dev_key, 16);
		
		foreach(i,NET_KEY_MAX){
			mesh_net_key_save_t *p_save = &key_save.net_key[i];
			if(p_save->valid){
				mesh_net_key_set(NETKEY_ADD, p_save->key, p_save->index, 0);
				foreach(j,APP_KEY_MAX){
					if(p_save->app_key[j].valid){
						mesh_app_key_set(APPKEY_ADD, p_save->app_key[j].key, p_save->app_key[j].index, p_save->index, 0);
					}
				}
				
				mesh_net_key_t *p_netkey_exist = is_mesh_net_key_exist(p_save->index);
				if(p_netkey_exist){
					p_netkey_exist->node_identity = p_save->node_identity;
					p_netkey_exist->priv_identity = p_save->priv_identity;
				}
			}
		}
	}
	
    return err;
}

// model save
int mesh_model_retrieve_and_store(bool4 sig_model, u32 md_id, int save)
{    
	int err = -1;
    if(is_actived_factory_test_mode() && save){
        return 0;
    }
    
	foreach_arr(i,mesh_md_adr_map){
		if(sig_model == mesh_md_adr_map[i].sig_model){
			foreach_arr(k,mesh_md_adr_map[i].md_id){
				if(md_id == mesh_md_adr_map[i].md_id[k]){
					return mesh_common_retrieve_and_store(mesh_md_adr_map[i].adr_base, save);
				}
			}
		}
	}
	return err;
}

int mesh_model_retrieve(bool4 sig_model, u32 md_id)
{
	return mesh_model_retrieve_and_store(sig_model, md_id, 0);
}

int mesh_model_store(bool4 sig_model, u32 md_id)
{
	return mesh_model_retrieve_and_store(sig_model, md_id, 1);
}

// common
void mesh_flash_save_check()
{
	mesh_sno_save_check();
}

void mesh_flash_retrieve()
{
#if (PROJECT_SEL == PROJECT_VC_DONGLE)
	u8 mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	flash_read_page(CFG_ADR_MAC, sizeof(mac), mac);
	u32 *pmac = (u32 *)mac;
	if (*pmac != 0xffffffff){
	    memcpy (tbl_mac, pmac, 6);
	}else{
		u32 random_val = clock_time_vc_hw();
		memcpy(tbl_mac, &random_val, 4);
        flash_write_page (CFG_ADR_MAC, 6, tbl_mac);
    }
#endif
    ele_adr_primary = (tbl_mac[0] + (tbl_mac[1] << 8)) & (~(BIT(15)));    // init to mac
    if(0 == ele_adr_primary){
        ele_adr_primary = 1;
    }
    mesh_common_retrieve_all();		// should be first, because include model_sig_cfg_s
	mesh_key_retrieve();            // should be after mesh common retrieve all() ,because of ele_adr_primary
	mesh_misc_retrieve();           // should be after key mesh_common_retrieve_all(), because use is_provision_success() in it

	#if MD_SERVER_EN
	mesh_model_cb_pub_st_register();	// must after mesh_common_retrieve_all
	#endif
	
#if (WIN32 && DEBUG_SHOW_VC_SELF_EN) // for test: because it have not save in VC.
    #if MD_LIGHT_CONTROL_EN
    foreach(i,LIGHT_CNT){
        model_sig_light_lc.mode[i] = 1;
    }
    light_LC_global_init();
    #endif

    #if 0
    mesh_model_pub_par_t pub_par = {0};
    pub_par.appkey_idx = 0;
    pub_par.ttl = TTL_PUB_USE_DEFAULT;
    pub_par.period.steps = 0;
    pub_par.period.res = 0;
    pub_par.transmit.count = PUBLISH_RETRANSMIT_CNT;
    pub_par.transmit.invl_steps = PUBLISH_RETRANSMIT_INVL_STEPS;
    cfg_cmd_pub_set(ele_adr_primary, ele_adr_primary, 0xc001, &pub_par, SIG_MD_LIGHT_LC_S, 1);
    #endif

    model_vd_light.srv[0].com.bind_key[0].bind_ok = 1;
    model_vd_light.clnt[0].com.bind_key[0].bind_ok = 1;
#endif
}

#endif

void mesh_node_init()
{    
    // ivi index init
    if(ADR_LPN1 == ele_adr_primary){
        //iv_idx_st.cur[3] -= 0x40;    // for test
    }
    mesh_iv_update_enter_search_mode();

    // cache init
    cache_init(ADR_ALL_NODES);
    
    //static int B_ret;
    //B_ret = test_mesh_sec_message();
    #if FEATURE_FRIEND_EN
    mesh_feature_set_fn();
    #elif FEATURE_LOWPOWER_EN
    mesh_feature_set_lpn();
    #endif

    #if ONLINE_STATUS_EN
    online_status_timeout = ONLINE_STATUS_TIMEOUT;
    #endif
	register_nl_callback();

#if 0 // PTS_TEST_OTA_EN
    model_sig_cfg_s.relay_retransmit.count = 2;
    model_sig_cfg_s.relay_retransmit.invl_steps = 1;
#endif
}

int mesh_get_netkey_idx_appkey_idx(mesh_bulk_cmd_par_t *p_cmd)
{
	p_cmd->nk_idx = 0;//mesh_key.net_key[0][0].index;
	p_cmd->ak_idx = 0;//mesh_key.net_key[0][0].app_key[0].index;
	return 1;
}

// common
static const u16 loop_interval_us = 10*1000;
int is_mesh_latency_window()
{
#if (__TL_LIB_8269__ || (MCU_CORE_TYPE == MCU_CORE_8269))
    #if (!FEATURE_LOWPOWER_EN)
    if(loop_interval_us){
        static u32 tick_loop = BIT(31); // make sure actived when power on
        if(clock_time_exceed(tick_loop, loop_interval_us)){
            tick_loop = clock_time();
            return 0;
        }else{
            return 1;
        }
    }
    #endif
#endif

    return 0;
}

#if (!IS_VC_PROJECT)
int mesh_nw_pdu_report_to_gatt(u8 *p, u8 len, u8 adv_type)	// from 8269 proxy node to adv from app or VC
{
	u8 proxy_type = MSG_NETWORK_PDU;
	if(MESH_ADV_TYPE_MESSAGE == adv_type){
		proxy_type = MSG_NETWORK_PDU;
	}else if(MESH_ADV_TYPE_PRO == adv_type){
		proxy_type = MSG_PROVISION_PDU;
	}else if(MESH_ADV_TYPE_BEACON == adv_type){
		proxy_type = MSG_MESH_BEACON;
	}else if(MESH_ADV_TYPE_PROXY_CFG_FLAG == adv_type){
		proxy_type = MSG_PROXY_CONFIG;
	}
	#if (!MESH_BLE_NOTIFY_FIFO_EN)
	return notify_pkts(p, len, GATT_PROXY_HANDLE, proxy_type);
	#else
	u8 header[2] = {GATT_PROXY_HANDLE};
	header[1] = proxy_type;
	return my_fifo_push(&blt_notify_fifo,p,len,header,sizeof(header));
	#endif
}
#endif

#if (!IS_VC_PROJECT)
int mesh_proxy_adv2gatt(u8 *bear, u8 adv_type)	// from 8269 proxy node to adv from app or VC
{
	int err=0;
	mesh_cmd_bear_unseg_t *p_br = (mesh_cmd_bear_unseg_t *)bear;
	if(PROXY_HCI_GATT == mesh_get_proxy_hci_type()){
		if(MESH_ADV_TYPE_BEACON == adv_type){
			if(	p_br->beacon.type != SECURE_BEACON &&
				p_br->beacon.type != PRIVACY_BEACON){
				err = mesh_nw_pdu_report_to_gatt((u8 *)&p_br->beacon.type, p_br->len-1 , MESH_ADV_TYPE_BEACON);
			}
		}else{
			err = mesh_nw_pdu_report_to_gatt((u8 *)&p_br->nw, p_br->len - 1, adv_type);
		}
	}
	#if (DEBUG_MESH_DONGLE_IN_VC_EN || MESH_MONITOR_EN)
	else{
		err = mesh_dongle_adv_report2vc(&p_br->len, MESH_ADV_PAYLOAD);
	}
	#endif
	
	if(err){
		static u8 proxy_adv2gatt_err;proxy_adv2gatt_err++;
	}

	return err;
}
#endif

int mesh_notifyfifo_rxfifo()
{
#if (!WIN32 && MESH_BLE_NOTIFY_FIFO_EN)
	if(blt_state == BLS_LINK_STATE_CONN){
		my_fifo_buf_t *fifo = (my_fifo_buf_t *)my_fifo_get(&blt_notify_fifo);
		if(fifo){
			u8 ret = 0;
			#if ONLINE_STATUS_EN
			u8 att_handle = fifo->data[0];
			if(ONLINE_ST_ATT_HANDLE_SLAVE == att_handle){
                ret = bls_att_pushNotifyData(att_handle,fifo->data+1,fifo->len-1);
			}else
			#endif
			{
			    ret = notify_pkts(fifo->data+2, fifo->len-2, GATT_PROXY_HANDLE, fifo->data[1]);
			}
			
			if(ret == BLE_SUCCESS){
				my_fifo_pop(&blt_notify_fifo);
				return 0;
			}else{
				return -1;
			}
		}else{
			return -1;
		}
	}else{
		// clear the blt_nofity fifo part 
		blt_notify_fifo.rptr = blt_notify_fifo.wptr;
		return 0;
	}
#else
	return 0;
#endif
}

#if FEATURE_RELAY_EN
#define FULL_REALY_PROC_ENABLE	1

int relay_adv_prepare_handler(rf_packet_adv_t * p, int rand_en)  // no random for relay transmit
{
    my_fifo_t *p_fifo = &mesh_adv_fifo_relay;
#if FULL_REALY_PROC_ENABLE
    my_fifo_poll_relay(p_fifo);   // must before get buffer.
    mesh_relay_buf_t *p_relay = my_fifo_get_relay(p_fifo);
    if(p_relay){
        int ret = 0;
        #if WIN32
        LOG_MSG_INFO(TL_LOG_NODE_BASIC,(u8 *)p_relay,p_relay->bear.len + 6,"Relay Buff:",0);
        #else
        ret = mesh_adv_cmd_set((u8 *)p, (u8 *)&p_relay->bear);
        #endif

        if(0 == p_relay->cnt){
            p_relay->valid = 0;
            if((u8 *)p_relay == my_fifo_get(p_fifo)){
                my_fifo_pop(p_fifo);
				#if DF_TEST_MODE_EN
				cfg_led_event(LED_EVENT_FLASH_2HZ_2S);
				#endif
                #if WIN32
                LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0, "Relay buffer pop", 0);
                #endif
            }else{
                #if WIN32
                LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0, "Relay buffer pop later in poll", 0);
                #endif
            }
        }else{
            if(p_relay->tick_10ms){ // not neccessary to check not zero, but should be better.
                p_relay->tick_10ms--;   // start tick
                if(rand_en){
                    set_random_adv_delay(1);    // random 10~20ms
                }
            }
        }
        return ret;
    }
#else
	mesh_relay_buf_t *p_relay = (mesh_relay_buf_t *)my_fifo_get(p_fifo);
	if(p_relay){
		mesh_transmit_t *p_trans_par = (mesh_transmit_t *)(&p_relay->bear.trans_par_val);
		static u32 relay_10ms_cnt =0;
		relay_10ms_cnt++;
		if(relay_10ms_cnt > p_trans_par->invl_steps){
			relay_10ms_cnt =0;
            int ret = mesh_adv_cmd_set((u8 *)p, (u8 *)&p_relay->bear);
			if(p_trans_par->count == 0){
				my_fifo_pop(p_fifo);
			}else{
				p_trans_par->count--;
			}
			return ret;
		}
	}
#endif
    return 0;
	
}
#endif

int my_fifo_push_relay (mesh_cmd_bear_unseg_t *p_in, u8 n, u8 ow)    // ow: over_write
{
#if FEATURE_RELAY_EN
    return my_fifo_push_relay_ll(&mesh_adv_fifo_relay, p_in, n, ow);
#else
    return 0;
#endif
}

// --------- node reset proc --------
static u32 del_node_tick = 0;
static u16 del_node_delay_ms = 0;

#if MANUAL_FACTORY_RESET_TX_STATUS_EN
u8 manual_factory_reset = 0;
#endif

/*********************************
cb_node_reset() will be called by Node_RESET command
return value: noly factory mode actived, return 0 means no response. return 1 with response.
*/
u32 node_reset_start()
{
	del_node_delay_ms = 1500;
	del_node_tick = clock_time()|1;
	return 1;
}

#if RELIABLE_CMD_EN
static u32 client_del_node_tick = 0;
static u16 client_del_node_adr_dst = 0;

void client_node_reset_cb(u16 adr_dst)
{
    if(adr_dst){
        client_del_node_tick = clock_time() | 1;
        client_del_node_adr_dst = adr_dst;
    }
}

#define RELIABLE_INTERVAL_MS_MIN       (2 * CMD_INTERVAL_MS + MESH_RSP_BASE_DELAY_STEP*10 + 300)	// relay + response
#if DEBUG_CFG_CMD_GROUP_AK_EN
#define RELIABLE_INTERVAL_MS_MAX       (max2((RELIABLE_INTERVAL_MS_MIN + 500), 4000))
#else
#define RELIABLE_INTERVAL_MS_MAX       (max2((RELIABLE_INTERVAL_MS_MIN + 500), 2000))
#endif
u32 get_reliable_interval_ms_min()
{
    return RELIABLE_INTERVAL_MS_MIN;
}

u32 get_reliable_interval_ms_max()
{
    return RELIABLE_INTERVAL_MS_MAX;
}
#endif

void proc_node_reset()
{
	if(del_node_tick && clock_time_exceed(del_node_tick, del_node_delay_ms * 1000)){
		del_node_tick = 0;	// must for WIN32
		kick_out(1);	// will reboot inside it.	
	}
	#if MANUAL_FACTORY_RESET_TX_STATUS_EN
	else if(manual_factory_reset){
	    my_fifo_reset(&mesh_adv_cmd_fifo);  // discard all command in buffer.
        mesh_tx_cmd_rsp_cfg_model(NODE_RESET_STATUS, 0, 0, ADR_ALL_NODES);
		kick_out(1);	// will wait tx cmd completed, and reboot later	
	}
	#endif

#if RELIABLE_CMD_EN
    if(client_del_node_tick && clock_time_exceed(client_del_node_tick, 1000*1000)){
        client_del_node_tick = 0;
        cache_init(client_del_node_adr_dst);
    }
#endif
}
// --------end-------

// ------fw revision UUID-----
const u16 my_fwRevisionUUID = CHARACTERISTIC_UUID_FW_REVISION_STRING;
u8 my_fwRevisionCharacter = CHAR_PROP_READ;
const u8  my_fwRevision_value [FW_REVISION_VALUE_LEN] = {
    (BUILD_VERSION & 0xFF),((BUILD_VERSION >> 8) & 0xFF),
    ((BUILD_VERSION >> 16) & 0xFF),((BUILD_VERSION >> 24) & 0xFF),
    (FW_VERSION_TELINK_RELEASE & 0xFF),((FW_VERSION_TELINK_RELEASE >> 8) & 0xFF),
    0x00, 0x00, 0x00
};
// --------end-------

// service change request
void mesh_service_change_report()
{
#if (!MI_API_ENABLE || DUAL_VENDOR_EN)
    #if DUAL_VENDOR_EN
    if(DUAL_VENDOR_ST_MI != provision_mag.dual_vendor_st)
    #endif
    {
        // force to do the service changes 
        #if !WIN32
        u8 service_data[4]={0x01,0x00,0xff,0x00};
        // should keep the service change in the last 
        bls_att_pushIndicateData(SERVICE_CHANGE_ATT_HANDLE_SLAVE,service_data,sizeof(service_data));

        #if DUAL_MODE_WITH_TLK_MESH_EN
        // dual_mode_TLK_service_change
        if(DUAL_MODE_SUPPORT_ENABLE == dual_mode_state){
            rf_packet_att_data_t pkt_srv = {13, 2, 11, 7, 4, ATT_OP_HANDLE_VALUE_IND};
            pkt_srv.hl = 0x28; // hanle of service change in telink mesh SDK 
            if(sizeof(pkt_srv.dat) >= sizeof(service_data)){
                memcpy(pkt_srv.dat, service_data, sizeof(service_data));
            }
            bls_ll_pushTxFifo(BLS_CONN_HANDLE, (u8 *)&pkt_srv.type);    // no need to check indication comfirm
        }
        #endif
        #endif
    }
#endif
}

#if ONLINE_STATUS_EN
/////////////// online status ////////////////////////////////////////////////////
mesh_node_st_t mesh_node_st[ONLINE_ST_NODE_MAX_NUM];

u32	mesh_node_mask[(ONLINE_ST_NODE_MAX_NUM + 31)>>5];
u16 online_st_node_max_num = ONLINE_ST_NODE_MAX_NUM;
u8 mesh_node_st_val_len = MESH_NODE_ST_VAL_LEN;
u8 mesh_node_st_par_len = MESH_NODE_ST_PAR_LEN;
u8 mesh_node_st_len = sizeof(mesh_node_st_t);

//STATIC_ASSERT((ONLINE_ST_NODE_MAX_NUM <= 256) && ((ONLINE_ST_NODE_MAX_NUM % 32) == 0));
STATIC_ASSERT((MESH_NODE_ST_VAL_LEN >= 4) && ((MESH_NODE_ST_VAL_LEN <= 10)));
STATIC_ASSERT(MESH_NODE_ST_PAR_LEN <= 7);

void device_status_update()
{
    // packet
    u8 st_val_par[MESH_NODE_ST_PAR_LEN] = {0};
    memset(st_val_par, 0xFF, sizeof(st_val_par));
    // led_lum should not be 0, because app will take it to be light off
    st_val_par[0] = light_lum_get(0, 1);
    #if (LIGHT_TYPE_CT_EN)
	st_val_par[1] = light_ct_lum_get(0, 1);
	#else
	st_val_par[1] = 0xff;   // rsv
	#endif
    // end
    
    ll_device_status_update(st_val_par, sizeof(st_val_par));
}

void mesh_node_buf_init ()
{
	for (int i=0; i<online_st_node_max_num; i++)
	{
	    memset(&mesh_node_st[i], 0, sizeof(mesh_node_st_t));
	}
	device_status_update();
}

void online_st_rc_mesh_pkt(u8 *p_payload)
{
    u8 adv_type = p_payload[1];
    if(adv_type == MESH_ADV_TYPE_ONLINE_ST){
        if(1 == online_st_adv_dec(p_payload)){
            mesh_cmd_bear_unseg_t *p = GET_BEAR_FROM_ADV_PAYLOAD(p_payload);
            u8 data_len = p->len-1-OFFSETOF(online_st_adv_t,data)-ONLINE_ST_MIC_LEN;
        	mesh_node_update_status(p->online_st_adv.data, (data_len)/mesh_node_st_val_len);
        }else{
        }
    }
}

STATIC_ASSERT(sizeof(online_st_report_t) <= 20);

void online_st_proc()
{
	static u32 online_st_tx_tick;
    if(clock_time_exceed(online_st_tx_tick, ONLINE_ST_INTERVAL_MS*1000)){
        online_st_tx_tick = clock_time();
	    mesh_send_online_status ();
	}

	if((blt_state == BLS_LINK_STATE_CONN)&&mesh_node_report_enable){
    	static u32 online_st_report_tick;
        if(clock_time_exceed(online_st_report_tick, 40*1000)){
            online_st_report_tick = clock_time();
    		mesh_node_flush_status ();

            #if 1 // MESH_BLE_NOTIFY_FIFO_EN
    		if (my_fifo_data_cnt_get(&blt_notify_fifo) < 10)
            #else
    		if (my_fifo_data_cnt_get(&blt_txfifo) <= 10)
    		#endif
    		{
    		    online_st_report_t report = {MESH_ADV_TYPE_ONLINE_ST};
    		    report.len_node_st_val = sizeof(mesh_node_st_val_t);
    		    report.sno = rand();
    			if (mesh_node_report_status ((u8 *)&report.node, ARRAY_SIZE(report.node)))
    			{
                    online_st_gatt_enc((u8 *)&report, sizeof(report));
                    #if 1 // MESH_BLE_NOTIFY_FIFO_EN
                    u8 header[1] = {ONLINE_ST_ATT_HANDLE_SLAVE};
    				my_fifo_push(&blt_notify_fifo,(u8 *)&report,sizeof(report),header,sizeof(header));
                    #else
                    bls_att_pushNotifyData(ONLINE_ST_ATT_HANDLE_SLAVE,(u8 *)&report,sizeof(report));
    				#endif
    			}
    		}
    	}
	}
}

void online_st_force_notify_check(mesh_cmd_bear_unseg_t *p_bear, u8 *ut_dec, int src_type)
{
    if(mesh_node_report_enable && (MESH_BEAR_GATT == src_type)){
        online_st_force_notify_check3(p_bear, ut_dec);
    }
}

void light_node_status_change_cb(u8 *p, u8 new_node){
    mesh_node_st_val_t *p_data = (mesh_node_st_val_t*)p;
    //extern u8  sync_time_enable;
    //if(sync_time_enable){
        //p_data->par[0] &= ~FLD_SYNCED;   //Note: bit7 of par[0] have been use internal
    //}
    static u16 dev_addr = 0;
    if(new_node){
        static u8 dev_new_node = 0;
        dev_new_node++;
        dev_addr = p_data->dev_adr;
    }else{
        static u8 dev_old_node = 0;
        dev_old_node++;
        dev_addr = p_data->dev_adr;
        if(0 == p_data->sn){    // off line
            // rssi_online_status_pkt_cb(CONTAINER_OF(p_data,mesh_node_st_t,val), 0, 0);
        }
    }
}

void rssi_online_status_pkt_cb(mesh_node_st_t *p_node_st, u8 rssi, int online_again)
{
    #if MESH_RSSI_RECORD_EN
    // just only can handle rssi, don't modify any other par inside p_node_st
    if((p_node_st->rssi != rssi) || online_again){     // online_again: new device or offline before
        #if 0 // test
        static u32 A_buf_cnt;
        static u8 A_buf[100][2];
        if(A_buf_cnt < ARRAY_SIZE(A_buf)){
            A_buf[A_buf_cnt][0] = p_node_st->val.dev_adr;
            A_buf[A_buf_cnt][1] = rssi;
            A_buf_cnt++;
        }
        #endif
        p_node_st->rssi = rssi; // rssi -= 110;
        // TODO: only report rssi to master here
    }
    #else
    // TODO: only report rssi to master here
    #endif
}
#else
void online_st_force_notify_check(mesh_cmd_bear_unseg_t *p_bear, u8 *ut_dec, int src_type){}
#endif

u8 get_online_st_adv_type()
{
    return MESH_ADV_TYPE_ONLINE_ST;
}

void sleep_us_clear_dog(u32 us)
{
    #define INV_CLEAR_DOG_US    (100*1000)
    while(us){
        if(us > INV_CLEAR_DOG_US){
            us -= INV_CLEAR_DOG_US;
            sleep_us(INV_CLEAR_DOG_US);
        }else{
            sleep_us(us);
            us = 0;
            //break;    // no need
        }
    }
}

void mesh_loop_proc_prior()
{
	if(is_fn_support_and_en){
		foreach(i,g_max_lpn_num){
			mesh_friend_response_delay_proc_fn(i);
		}
	}
}

#if PROV_APP_KEY_SETUP_TIMEOUT_CHECK_EN
#define PROV_APP_KEY_SETUP_TIMEOUT_MS       (30*1000)   // include provision reconnect GATT
u32 prov_app_key_setup_tick = 0;

void mesh_prov_app_key_setup_timeout_check()
{
    if(prov_app_key_setup_tick){
        if(clock_time_exceed(prov_app_key_setup_tick, PROV_APP_KEY_SETUP_TIMEOUT_MS * 1000)){
            kick_out(0);    // reboot inside.
        }
    }
}
#endif

void mesh_loop_process()
{
    CB_USER_MAIN_LOOP();
	#if GATEWAY_ENABLE && DEBUG_CFG_CMD_GROUP_AK_EN
    mesh_ota_comm_test();
	#endif
    #if MD_REMOTE_PROV
    // remote prov proc
    mesh_cmd_sig_rp_loop_proc();
    #endif
	#if MD_PRIVACY_BEA
		#if MD_SERVER_EN
	mesh_prov_para_random_proc();
		#endif
	#endif
	// node identity proc 
	mesh_switch_identity_proc();
    // provision loop proc
	mesh_prov_proc_loop();
	// mesh beacon proc 
	mesh_beacon_send_proc();
	// mesh proxy proc 
	mesh_proxy_sar_timeout_terminate();
	// node reset 
	proc_node_reset();
	#if (!WIN32)
	mesh_ota_reboot_proc();
	#endif
	// gatt provision timeout proc 
	#if WIN32
	check_prov_timeout();
	#endif 
    // publish proc 
	mesh_pub_period_proc();
    // keybind part 
	#if ((WIN32) || GATEWAY_ENABLE)
	    #if MD_MESH_OTA_EN
	mesh_ota_master_proc();
	    #endif
	mesh_kr_cfgcl_proc();
	check_mesh_kr_cfgcl_timeout();
	#endif
	#if MD_MESH_OTA_EN
	mesh_ota_proc();
	#endif
	// mesh proc 
	#if (!(DEBUG_MESH_DONGLE_IN_VC_EN && (!IS_VC_PROJECT)))
	mesh_seg_ack_poll();
	if(is_lpn_support_and_en){
	    #if FEATURE_LOWPOWER_EN
		mesh_friend_ship_proc_LPN(NULL);
        mesh_main_loop_LPN();
		#endif
	}else if(is_fn_support_and_en){
		mesh_friend_ship_proc_FN(NULL);
	}
	
	    #if MESH_BLE_NOTIFY_FIFO_EN
	mesh_notifyfifo_rxfifo();
	    #endif
	mesh_tid_timeout_check();
	    #if RELIABLE_CMD_EN
	mesh_tx_reliable_proc();
	    #endif
	mesh_flash_save_check();
	    #if PROV_APP_KEY_SETUP_TIMEOUT_CHECK_EN
	mesh_prov_app_key_setup_timeout_check();
	    #endif
	system_time_run(); // must at last of main_loop(), before mesh_lpn_proc_suspend()
	#endif
	
	
	
	#if (!IS_VC_PROJECT)
	//if((!reg_usb_host_conn) && my_fifo_get(&hci_tx_fifo)){
		//my_fifo_pop(&hci_tx_fifo);	// no need to report to HCI
	//}
	
	if(irq_ev_one_pkt_completed){
		irq_ev_one_pkt_completed = 0;
		check_and_send_next_segment_pkt();
	}
	#endif	
}

// for the adv will not send adv at the same time to control 
void mesh_random_delay()
{
#if !WIN32
    u16 tr = (REG_ADDR16(0x448) ^ clock_time ());   // 0 -- 65ms
    sleep_us (tr);
#endif 
}

#if IS_VC_PROJECT
FLASH_ADDRESS_DEFINE;
#endif

/** @
  * @relay_flag: 1: relay; 0: tx command
  */
int is_pkt_notify_only(u16 dst_adr, int relay_flag)
{
    if(is_proxy_support_and_en){
        if(relay_flag){
            //reduce tx time of GATT connected node to receive more messages. especially when get all.
            return (app_adr == dst_adr); // other nodes will relay this address.
        }else{ // tx command
            #if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
            // other proxy client's filter address may subscribe this address. //return 0;
            #else
            return (app_adr == dst_adr);
            #endif
        }
    }
    return 0;
}

u8 mesh_init_flag = 1;

void prov_random_proc(u8 *p_random)
{
    #if !WIN32
    for(int i=0;i<8;i++){
            p_random[i]= rand()&0xff;
    }
	memcpy(prov_para.random,p_random,8);
    #endif
}

void mesh_init_all()
{	
    mesh_init_flag = 1;
	#if DU_ENABLE
	blc_att_setRxMtuSize(247);
	#endif
    LOG_MSG_INFO(TL_LOG_COMMON,0, 0,"System start ............",0);
#if WIN32
	mesh_global_var_init();	// must call first in user init() for firmware SDK.
#endif

#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
	VC_node_info_retrieve();
#endif
	//rf_link_light_event_callback(PROV_START_LED_CMD);// for the ota tag part 
#if IS_VC_PROJECT
	FLASH_ADDRESS_CONFIG;
#endif
	// read proxy list part 
	set_proxy_initial_mode(0);
	// init the heartbeat msg 
	init_heartbeat_str();
    u8 node_ident_random[8];    // because it will be used in both mesh_flash_retrieve_()->mesh_net_key_set_() and mesh_provision_para_init_()
	prov_random_proc(node_ident_random);
    // read parameters
    mesh_flash_retrieve();	// should be first, get unicast addr from config modle.
    mesh_key_node_identity_init();// should be after key retrieve .
    provision_random_data_init();
	mesh_provision_para_init(node_ident_random);
	//unprovision beacon send part 
	#if (!(DU_LPN_EN || (GATT_LPN_EN && __PROJECT_MESH__)))
	beacon_str_init();
	#endif
	mesh_node_init();
	

	// init the health part 
	init_health_para();
	mesh_vd_init();
	init_ecc_key_pair();
#if (!FEATURE_LOWPOWER_EN)
	mesh_random_delay();
#endif
#if TEST_CASE_HBS_BV_05_EN
	friend_add_special_grp_addr();
#endif
#if MD_PRIVACY_BEA
	#if MD_SERVER_EN
	mesh_private_para_init();
	#endif
#endif
    mesh_init_flag = 0;
}

#if MI_API_ENABLE
u8 mi_sec_beacon_mac[6];
#endif

void mesh_ivi_proc_cb(u8 search_flag)
{
	#if MI_API_ENABLE
	if(search_flag){
		mi_cb_ivi_event_send(MI_RCV_BEACON_SAR,mi_sec_beacon_mac);		
	}else{
		mi_cb_ivi_event_send(MI_RCV_BEACON,mi_sec_beacon_mac);	
	}
	#endif
}

int app_event_handler_adv(u8 *p_payload, int src_type, u8 need_proxy_and_trans_par_val)
{
	int err = 0;
	
	mesh_cmd_bear_unseg_t *p_br = GET_BEAR_FROM_ADV_PAYLOAD(p_payload);
	#if (0 == PROXY_GATT_WITH_HEAD)
	if(MESH_BEAR_ADV == src_type){
	    if(p_br->len + SIZEOF_MEMBER (mesh_cmd_bear_unseg_t,len) > ADV_PDU_LEN_MAX){
	        return -1;
	    }
	}else
	#endif
	{
	    // GATT may be a long packet, and the 'len' is caculated from payload length, so it's no need to check
	}
	
	u8 adv_type = p_br->type;
	if(adv_type == MESH_ADV_TYPE_MESSAGE){
        lpn_debug_set_event_handle_pin(1);
		err = mesh_rc_data_layer_network(p_payload, src_type, need_proxy_and_trans_par_val);
        lpn_debug_set_event_handle_pin(0);
	}
	else if((adv_type == MESH_ADV_TYPE_BEACON)&&(p_br->beacon.type == SECURE_BEACON)){
		#if MI_API_ENABLE
		if(p_br->beacon.type == SECURE_BEACON){
			event_adv_report_t *pa = (event_adv_report_t *)(p_payload-11);
			memcpy(mi_sec_beacon_mac,pa->mac,6);
		}
		#endif
		mesh_rc_data_beacon_sec(p_payload, 0);
	}
	else if((adv_type == MESH_ADV_TYPE_BEACON)&&(p_br->beacon.type == PRIVACY_BEACON)){
		#if MD_PRIVACY_BEA
		mesh_rc_data_beacon_privacy(p_payload, 0);
		#endif
	}
	else if((adv_type == MESH_ADV_TYPE_PRO)
		|| ((adv_type == MESH_ADV_TYPE_BEACON)&&(p_br->beacon.type == UNPROVISION_BEACON))){
	    #if DEBUG_VC_FUNCTION && (!WIN32)
		send_vc_fifo(TSCRIPT_CMD_VC_DEBUG,(u8 *)p_payload, mesh_adv_payload_len_get(p_br));
	    #else
		u8 irq_rev = irq_disable();
		mesh_provision_rcv_process(p_payload,0);
		irq_restore(irq_rev);
	    #endif
	#if ONLINE_STATUS_EN
	}else{
	    online_st_rc_mesh_pkt(p_payload);
	#endif
	}
	return err;
}

#if (!IS_VC_PROJECT || DEBUG_MESH_DONGLE_IN_VC_EN)
void mesh_gatt_bear_handle(u8 *bear)
{
	mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)bear;
	app_event_handler_adv(&p_bear->len, MESH_BEAR_GATT, p_bear->trans_par_val);
}

int mesh_is_proxy_ready()
{
	if(PROXY_HCI_GATT == mesh_get_proxy_hci_type()){
		#if (IS_VC_PROJECT && DEBUG_MESH_DONGLE_IN_VC_EN)
		return 1;
		#else
		return (BLS_LINK_STATE_CONN == blt_state);
		#endif
	}else{
		return 1;
	}
}

    #if (!IS_VC_PROJECT)
void mesh_nw_pdu_from_gatt_handle(u8 *p_bear)	// adv from app or VC to 8269 proxy node
{
	#if DEBUG_MESH_DONGLE_IN_VC_EN
	if(FEATURE_PROXY_EN && (BLS_LINK_STATE_CONN == blt_state)){
		debug_mesh_dongle_adv_bear2usb(p_bear);
	}
	#else
	if(is_proxy_support_and_en){
		mesh_gatt_bear_handle(p_bear);
	}
	#endif
}

int mesh_adv_cmd_set(u8 *p_adv, u8 *p_bear)
{
    mesh_cmd_bear_unseg_t *p_br = (mesh_cmd_bear_unseg_t *)p_bear;
    u8 len_payload = p_br->len + 1;
    if(len_payload > ADV_PDU_LEN_MAX){
        return 0;
    }
    
    rf_packet_adv_t *p = (rf_packet_adv_t *)p_adv;
    
    p->header.type = LL_TYPE_ADV_NONCONN_IND;  
    memcpy(p->advA,tbl_mac,6);
    memcpy(p->data, &p_br->len, len_payload);
    p->rf_len = 6 + len_payload;
    p->dma_len = p->rf_len + 2;
#if EXTENDED_ADV_ENABLE
    if(MESH_ADV_TYPE_MESSAGE == p_br->type){
        if(len_payload > 31){
            return PREPARE_HANDLE_ADV_EXTEND;
        }
    }
#endif

    return 1;
}

ble_sts_t  bls_ll_setAdvParam_interval(u16 min_ms, u16 rand_ms)
{
    u16 min = GET_ADV_INTERVAL_LEVEL(min_ms);
    u16 max = GET_ADV_INTERVAL_LEVEL(min_ms + rand_ms);
	if(blta.advInt_min != min){
	    bls_ll_setAdvInterval(min, max);
	}
	return BLE_SUCCESS;
}

    #endif
#endif

//--------------------app key bind flow------------------------------//
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__ || PTS_TEST_EN)
#define KR_CL_NK_INDEX			(0x000)
#define KR_CL_AK_INDEX			(0x000)

key_refresh_cfgcl_proc_t key_refresh_cfgcl_proc;

void mesh_kr_cfgcl_start(u16 node_adr)
{
	key_refresh_cfgcl_proc_t *p = &key_refresh_cfgcl_proc;
	p->st = 1;
	
	mesh_net_key_t *p_nk = &mesh_key.net_key[0][0];
	u8 ak[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	memcpy(p->ak, ak, sizeof(p->ak));
	
	u8 nk[16] = {0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x20};
	nk[15] = p_nk->key[15] + 1;
	memcpy(p->nk_new, nk, sizeof(p->nk_new));
	p->next_st_flag = 1;
	p->node_adr = node_adr;
}

void mesh_kr_cfgcl_retry_init()
{
	key_refresh_cfgcl_proc.retry_cnt = MESH_KR_CFG_RETRY_MAX_CNT;
}

void mesh_kr_cfgcl_retry_decrease()
{
	key_refresh_cfgcl_proc.retry_cnt--;
	if(0 == key_refresh_cfgcl_proc.retry_cnt){
		LOG_MSG_INFO(TL_LOG_MESH,0, 0,"mesh_kr_cfgcl_retry_decrease:app key decrease times overflow",0);
		#if  ANDROID_APP_ENABLE || IOS_APP_ENABLE
		App_key_bind_end_callback(MESH_KR_CFGCL_RETRY_TIMES_ERR);	
		#endif 
		//key_refresh_cfgcl_proc.st = KR_CFGCL_NORMAL;
	}		
}
#if WIN32 
#define MESH_KR_CFGCL_TIMEOUT_MAX_TIME 	60*1000*1000
#else
#define MESH_KR_CFGCL_TIMEOUT_MAX_TIME 	60*1000*1000
#endif
void check_mesh_kr_cfgcl_timeout()
{
	key_refresh_cfgcl_proc_t *p_cfgcl = &(key_refresh_cfgcl_proc);
	if(p_cfgcl->timeout && clock_time_exceed(p_cfgcl->timeout,MESH_KR_CFGCL_TIMEOUT_MAX_TIME)){
		LOG_MSG_INFO(TL_LOG_MESH,(u8 *)p_cfgcl, sizeof(key_refresh_cfgcl_proc_t),"check_mesh_kr_cfgcl_timeout:appkey bind terminate\r\n",0);
		#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)		
		mesh_cfg_keybind_end_event(MESH_KEYBIND_EVE_TIMEOUT,0);
		#endif 
	}
}

void mesh_ker_cfgcl_proc_init()
{
	key_refresh_cfgcl_proc_t *p_cfgcl = &(key_refresh_cfgcl_proc);
	// need to reset the timeout tick 
	p_cfgcl->timeout = 0;
	// reset the status of the states of the key_refresh_cfgcl_proc
	p_cfgcl->st = KR_CFGCL_NORMAL;
	return ;

}

void mesh_kr_cfgcl_proc()
{
#if TESTCASE_FLAG_ENABLE
	key_refresh_cfgcl_proc_t *p = &key_refresh_cfgcl_proc;
	
	if(is_busy_segment_or_reliable_flow() || (KR_CFGCL_NORMAL == p->st) || (!p->next_st_flag)){
		return ;
	}

	switch(p->st){
		case KR_CFGCL_GET_CPS:
			cfg_cmd_cps_get(p->node_adr, 0xff);
			p->st = KR_CFGCL_ADD_APPKEY;
			break;
		case KR_CFGCL_ADD_APPKEY:
			cfg_cmd_ak_add(p->node_adr, p->nk_idx, p->ak_idx, p->ak);
			p->st = KR_CFGCL_BIND_APPKEY_MD2;
			break;
		case KR_CFGCL_BIND_APPKEY_MD2:
			cfg_cmd_ak_bind(p->node_adr, p->node_adr, p->ak_idx, 2, 1);
			p->st = KR_CFGCL_BIND_APPKEY_MD3;
			break;
		case KR_CFGCL_BIND_APPKEY_MD3:
			cfg_cmd_ak_bind(p->node_adr, p->node_adr, p->ak_idx, 3, 1);
			p->st = KR_CFGCL_GET_PHASE0;
			break;
		case KR_CFGCL_GET_PHASE0:
			cfg_cmd_key_phase_get(p->node_adr, p->nk_idx);
			p->st = KR_CFGCL_SEND_NEW_NETKEY;
			break;
		case KR_CFGCL_SEND_NEW_NETKEY:
			mesh_net_key_set(NETKEY_UPDATE, p->nk_new, p->nk_idx, 0);	// update self first
			cfg_cmd_nk_update(p->node_adr, p->nk_idx, p->nk_new);
			p->st = KR_CFGCL_SEND_NEW_APPKEY;
			break;
		case KR_CFGCL_SEND_NEW_APPKEY:
			cfg_cmd_ak_update(p->node_adr, p->nk_idx, p->ak_idx, p->ak_new);
			p->st = KR_CFGCL_SET_PHASE2;
			break;
		case KR_CFGCL_SET_PHASE2:
			key_refresh_phase_set_by_index(p->nk_idx, KEY_REFRESH_TRANS2);	// update self first
			cfg_cmd_key_phase_set(p->node_adr, p->nk_idx, KEY_REFRESH_TRANS2);
			p->st = KR_CFGCL_SET_PHASE3;
			break;
		case KR_CFGCL_SET_PHASE3:
			cfg_cmd_key_phase_set(p->node_adr, p->nk_idx, KEY_REFRESH_TRANS3);
			key_refresh_phase_set_by_index(p->nk_idx, KEY_REFRESH_TRANS3);	// set self to nomal later
			p->st = KR_CFGCL_NORMAL;
			break;
		case KR_CFGCL_BC_KR_ON:
			key_refresh_phase_set_by_index(p->nk_idx, KEY_REFRESH_TRANS2);
			p->st = KR_CFGCL_NORMAL;
			break;
		case KR_CFGCL_BC_KR_OFF:
			key_refresh_phase_set_by_index(p->nk_idx, KEY_REFRESH_TRANS3);
			p->st = KR_CFGCL_NORMAL;
			break;
		default :
			break;
	}

	p->next_st_flag = 0;	// init
#else
#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)


#if  (ANDROID_APP_ENABLE || IOS_APP_ENABLE)
	static u32 tick_loop_for_bind =0;
	if(clock_time_exceed(tick_loop_for_bind,400*1000)){
		// should send bind cmd per 400ms ,or the receive buf will clapse
		tick_loop_for_bind = clock_time();
	}else{
		return ;
	}
#endif 
	key_refresh_cfgcl_proc_t *p = &key_refresh_cfgcl_proc;

	if(is_busy_segment_or_reliable_flow() || my_fifo_data_cnt_get(&mesh_adv_cmd_fifo) || (KR_CFGCL_NORMAL == p->st)){
		return ;
	}

	switch(p->st){		
		case KR_CFGCL_GET_CPS:
			LOG_MSG_INFO(TL_LOG_KEY_BIND,0,0,"SEND: get composition data",0);
			cfg_cmd_cps_get(p->node_adr, 0xff);
			break;
		case KR_CFGCL_ADD_APPKEY:
			LOG_MSG_INFO(TL_LOG_KEY_BIND,p->ak,16,"SEND: appkey add ,0x%d is the appkey index\r\n",p->ak_idx);
			cfg_cmd_ak_add(p->node_adr, p->nk_idx, p->ak_idx, p->ak);
			break;
		case KR_CFGCL_BIND_DEFAULT_APPKEY:
			VC_search_and_bind_model();
			break;
		default:
			break;
	}
	mesh_kr_cfgcl_retry_decrease();
#endif	
#endif
}

void mesh_kc_cfgcl_mode_para(u16 apk_idx,u8 *p_appkey)
{
	key_refresh_cfgcl_proc.ak_idx = apk_idx;
	memcpy(key_refresh_cfgcl_proc.ak,p_appkey,16);
	return ;
}

void mesh_kr_cfgcl_mode_set(u16 addr, u8 mode,u16 nk_idx)
{
	key_refresh_cfgcl_proc_t * p = (key_refresh_cfgcl_proc_t *)&key_refresh_cfgcl_proc;
	mesh_kr_cfgcl_retry_init();
	LOG_MSG_INFO(TL_LOG_MESH,(u8 *)&mesh_key, (u8)sizeof(mesh_key),"mesh_kr_cfgcl_mode_set:mesh key printf\r\n",0);
	p->node_adr = addr;
	p->next_st_flag = KR_CFGCL_NORMAL;
	if(KR_CFGCL_BIND_DEFAULT_APPKEY == mode){
		p->timeout = clock_time()|1;
		p->st = KR_CFGCL_GET_CPS;
		p->next_st_flag = KR_CFGCL_BIND_DEFAULT_APPKEY;
		p->ele_bind_index = p->model_bind_index = 0;
		p->nk_idx = nk_idx;
		#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)//set provisioner appkey
		mesh_app_key_set_and_bind(p->nk_idx, p->ak, p->ak_idx, 1);
		#endif
	}else{
		p->st = mode;
	}
	return;
}

void mesh_kc_cfgcl_mode_para_set(u16 apk_idx,u8 *p_appkey,u16 unicast,u16 nk_idx,u8 fast_bind)
{
    key_refresh_cfgcl_proc.fast_bind = fast_bind;
    cache_init(ADR_ALL_NODES);// fix the cache problem 
    if(fast_bind){
		key_refresh_cfgcl_proc.timeout = clock_time()|1;
        // bind the provisioner ,and send the cmd appkey add 
        #if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)//set provisioner appkey
		mesh_app_key_set_and_bind(nk_idx, p_appkey, apk_idx, 1);
		#endif
		
		cfg_cmd_ak_add(unicast, nk_idx, apk_idx, p_appkey);
    }else{
        mesh_kc_cfgcl_mode_para(apk_idx,p_appkey);
	    mesh_kr_cfgcl_mode_set(unicast,KR_CFGCL_BIND_DEFAULT_APPKEY,nk_idx);
    }
	return ;
}

void mesh_kr_cfgcl_appkey_set(u16 index, u8 *key)
{
	key_refresh_cfgcl_proc.ak_idx = index;
	memcpy(key_refresh_cfgcl_proc.ak, key, sizeof(key_refresh_cfgcl_proc.ak));
	return;
}

#if (0 == DEBUG_MESH_DONGLE_IN_VC_EN)
    #if (MD_BIND_WHITE_LIST_EN)
#define MAX_KEY_BIND_WHITE_LIST_CNT 	53
u16 key_bind_list_buf[MAX_KEY_BIND_WHITE_LIST_CNT];
u8 key_bind_list_cnt =0;
    #endif
u8 is_model_in_key_bind_list(u16 *p_mode_id)
{
#if (MD_BIND_WHITE_LIST_EN)
	for(int i=0;i<key_bind_list_cnt;i++){
		if(*p_mode_id == key_bind_list_buf[i]){
			return 1;
		}
	}
	return 0;
#else
    return (!is_use_device_key(*p_mode_id, 1) && (SIG_MD_HEALTH_CLIENT != *p_mode_id));
#endif
}

void mesh_kr_cfgcl_status_update(mesh_rc_rsp_t *rsp)
{
	u16 op = rf_link_get_op_by_ac(rsp->data);

	key_refresh_cfgcl_proc_t *p = &key_refresh_cfgcl_proc;
	if(op == NODE_RESET_STATUS){
		VC_cmd_clear_all_node_info(rsp->src);
	}else if(op == APPKEY_STATUS && p->fast_bind){
        #if WIN32
		App_key_bind_end_callback(MESH_APP_KEY_BIND_EVENT_SUC); 
		#else
		mesh_cfg_keybind_end_event(MESH_KEYBIND_EVE_SUC,key_refresh_cfgcl_proc.node_adr);
		#endif
	#if MD_REMOTE_PROV
	}else if (mesh_rsp_opcode_is_rp(op)){
        #if WIN32
        mesh_rp_client_rx_cb(rsp);
        #endif
    #endif
	}
	
	switch(p->st){
		case KR_CFGCL_GET_CPS:
			if(COMPOSITION_DATA_STATUS == op){
				mesh_page0_t *p_page0_rsp = (mesh_page0_t *)(rsp->data+2);
			    #if 0
				LOG_MSG_INFO(TL_LOG_KEY_BIND,(u8 *)(rsp->data+2),
					rsp->len-OFFSETOF(rsp,data)-2,"RCV: the composition data rsp about model is\r\n");
				#endif
				#if (MD_BIND_WHITE_LIST_EN)
				model_need_key_bind_whitelist(key_bind_list_buf,&key_bind_list_cnt,MAX_KEY_BIND_WHITE_LIST_CNT);
				if(key_bind_list_cnt){
				    u8 buf_ut_rx[MESH_CMD_ACCESS_LEN_MAX];
					mesh_page0_t *p_page0 = (mesh_page0_t *)buf_ut_rx;
					memcpy(p_page0, p_page0_rsp, sizeof(mesh_page0_head_t)+2);  // +2: loc
					get_cps_from_keybind_list(p_page0, p_page0_rsp);
					LOG_MSG_INFO(TL_LOG_KEY_BIND,(u8 *)(p_page0->ele.md_sig),
					(p_page0->ele.nums)*2+(p_page0->ele.numv)*4,"the model need to bind are:\r\n",0);
					VC_node_cps_save(p_page0,rsp->src,sizeof(mesh_page0_head_t)+get_cps_ele_len(&p_page0->ele));
				}else
				#endif
				{
					VC_node_cps_save(p_page0_rsp,rsp->src,rsp->len - 4 - 2);   // 2: op + page no
				}
				mesh_kr_cfgcl_retry_init();
				if(KR_CFGCL_BIND_DEFAULT_APPKEY == p->next_st_flag){
					p->st = KR_CFGCL_ADD_APPKEY;
				}
				else{
					p->st = KR_CFGCL_NORMAL;
				}
			}
			break;
		case KR_CFGCL_ADD_APPKEY:
			if(APPKEY_STATUS == op){
				mesh_kr_cfgcl_retry_init();
				if(KR_CFGCL_BIND_DEFAULT_APPKEY == p->next_st_flag){
					p->st = KR_CFGCL_BIND_DEFAULT_APPKEY;
				}
				else{
					p->st = KR_CFGCL_NORMAL;
				}
			}
		case KR_CFGCL_BIND_DEFAULT_APPKEY:
			if(MODE_APP_STATUS == op){
				mesh_kr_cfgcl_retry_init();
				if(KR_CFGCL_BIND_DEFAULT_APPKEY == p->next_st_flag){
					mesh_app_bind_status_t *p_bind_rsp = (mesh_app_bind_status_t *)(rsp->data+2);
					
					int par_len = rsp->len - (OFFSETOF(mesh_rc_rsp_t,data) - OFFSETOF(mesh_rc_rsp_t,src)) - 2;
					bool4 sig_model = (sizeof(mesh_app_bind_status_t) - 2 == par_len);
					u32 model_id = sig_model ? (p_bind_rsp->bind.model_id & 0xFFFF) : p_bind_rsp->bind.model_id;
					if((model_id == p->model_id) && p->is_new_model){
						p->is_new_model = 0;
						p->model_bind_index++;
					}
				}
				else{
					p->st = KR_CFGCL_NORMAL;
				}
			}
		default:
			break;
	}
	return;
}
#endif
#endif
//--------------------app key bind flow end------------------------------//

#if 1 // move from mesh_ota.c
#if (0 == DISTRIBUTOR_UPDATE_CLIENT_EN)
void mesh_ota_master_ack_timeout_handle(){}
#endif

int is_valid_tlk_fw_buf(u8 *p_flag)
{
    u32 startup_flag = START_UP_FLAG;
    return (0 == memcmp(p_flag, &startup_flag, 4));
}

void mesh_ota_read_data(u32 adr, u32 len, u8 * buf)
{
#if WIN32
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
    extern u8 fw_ota_data_rx[];
    memcpy(buf, fw_ota_data_rx + adr, len);
    #endif
#else
	flash_read_page(ota_program_offset + adr, len, buf);
#endif
}

u32 get_fw_len()
{
	u32 fw_len = 0;
	mesh_ota_read_data(0x18, 4, (u8 *)&fw_len);	// use flash read should be better
	return fw_len;
}

u8 get_ota_check_type()
{
    u8 ota_type[2] = {0};
    mesh_ota_read_data(6, sizeof(ota_type), ota_type);
	if(ota_type[0] == 0x5D){
		return ota_type[1];
	}
	return FW_CHECK_NONE;
}

u32 get_total_crc_type1_new_fw()
{
	u32 crc = 0;
	u32 len = get_fw_len();
	u32 addr_crc = ((len + 15)/16 - 1) * 16;
	mesh_ota_read_data(addr_crc, 4, (u8 *)&crc);
    return crc;
}

#define OTA_DATA_LEN_1      (16)    

int is_valid_ota_check_type1()
{	
	u32 crc_org = 0;
	u32 len = get_fw_len();
	mesh_ota_read_data(len - 4, 4, (u8 *)&crc_org);

    u8 buf[2 + OTA_DATA_LEN_1];
    u32 num = (len - 4 + (OTA_DATA_LEN_1 - 1))/OTA_DATA_LEN_1;
	u32 crc_new = 0;
    for(u32 i = 0; i < num; ++i){
    	buf[0] = i & 0xff;
    	buf[1] = (i>>8) & 0xff;
        mesh_ota_read_data((i * OTA_DATA_LEN_1), OTA_DATA_LEN_1, buf+2);
        if(!i){     // i == 0
             buf[2+8] = 0x4b;	// must
        }
        
        crc_new += crc16(buf, sizeof(buf));
        if(0 == (i & 0x0fff)){
			// about take 88ms for 10k firmware;
			#if (MODULE_WATCHDOG_ENABLE&&!WIN32)
			wd_clear();
			#endif
        }
    }
    
    return (crc_org == crc_new);
}

u32 get_blk_crc_tlk_type1(u8 *data, u32 len, u32 addr)
{	
    u8 buf[2 + OTA_DATA_LEN_1];
    u32 num = len / OTA_DATA_LEN_1; // sizeof firmware data which exclude crc, is always 16byte aligned.
    //int end_flag = ((len % OTA_DATA_LEN_1) != 0);
	u32 crc = 0;
    for(u32 i = 0; i < num; ++i){
        u32 line = (addr / 16) + i;
    	buf[0] = line & 0xff;
    	buf[1] = (line>>8) & 0xff;
    	memcpy(buf+2, data + (i * OTA_DATA_LEN_1), OTA_DATA_LEN_1);
        
        crc += crc16(buf, sizeof(buf));
    }
    return crc;
}

// check 2
static const unsigned long crc32_half_tbl[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

//_attribute_ram_code_
unsigned long crc32_half_cal(unsigned long crc, unsigned char* input, unsigned long* table, int len)
{
    unsigned char* pch = input;
    for(int i=0; i<len; i++)
    {
        crc = (crc>>4) ^ table[(crc^*pch) & 0x0f];
        pch++;
    }

    return crc;
}

u32 get_crc32_16bytes(unsigned long crc_init, unsigned char* data)
{
    //split 16 bytes OTA data into 32 half bytes to caculate CRC.
    u8 ota_dat[32];
    for(int i=0;i<16;i++){
        ota_dat[i*2] = data[i]&0x0f;
        ota_dat[i*2+1] = data[i]>>4;
    }
    return crc32_half_cal(crc_init, ota_dat, (unsigned long* )crc32_half_tbl, 32);
}

u32 get_blk_crc_tlk_type2(u32 crc_init, u8 *data, u32 len)
{	
    u32 num = len / OTA_DATA_LEN_1; // sizeof firmware data which exclude crc, is always 16byte aligned.
    for(u32 i = 0; i < num; ++i){
        crc_init = get_crc32_16bytes(crc_init, data + OTA_DATA_LEN_1 * i);
    }
    return crc_init;
}
#endif

#if (0 == MD_DF_EN)    // for compile
int is_directed_forwarding_en(u16 netkey_offset){return 0;}
int is_directed_relay_en(u16 netkey_offset){return 0;}
int is_directed_proxy_en(u16 netkey_offset){return 0;}
int is_directed_friend_en(u16 netkey_offset){return 0;}
int is_directed_forwarding_op(u16 op){return 0;}
void mesh_directed_forwarding_bind_state_update(){return;}
void mesh_directed_forwarding_proc(u8 *bear, u8 *par, int par_len, int src_type){}
path_entry_com_t *get_forwarding_entry(u16 netkey_offset, u16 src_address, u16 destination){return 0;}
int mesh_df_path_monitoring(path_entry_com_t *p_entry){return 0;}
int is_address_in_dependent_list(path_entry_com_t *p_fwd_entry, u16 addr){return 0;}
int directed_forwarding_initial_start(u16 netkey_offset, u16 destination, u16 dependent_addr, u16 dependent_ele_cnt){return 0;}
int is_proxy_use_directed(u16 netkey_offset){return 0;}
u8 get_directed_proxy_dependent_ele_cnt(u16 netkey_offset, u16 addr){return 0;}
u8 get_directed_friend_dependent_ele_cnt(u16 netkey_offset, u16 addr){return 0;}
int directed_forwarding_dependents_update_start(u16 netkey_offset, u8 type, u16 path_enpoint, u16 dependent_addr, u8 dependent_ele_cnt){return 0;}
#endif

#if (0 == MD_SBR_EN)
int is_subnet_bridge_en(){return 0;}
int get_subnet_bridge_index(u16 netkey_index, u16 src, u16 dst){return -1;}
#endif
#if (SLEEP_FUNCTION_DISABLE && ((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278)))
/*
@ This function should not be called anytime
*/
int  cpu_sleep_wakeup_none(SleepMode_TypeDef sleep_mode,  SleepWakeupSrc_TypeDef wakeup_src, unsigned int  wakeup_tick)
{
    while(1){
        wd_clear();
    }
}

/*
@ This function should not be called anytime
*/
unsigned int pm_tim_recover_none(unsigned int now_tick_32k)
{
    while(1){
        wd_clear();
    }
    return 0;
}

void blc_pm_select_none()
{
	cpu_sleep_wakeup 	 	= cpu_sleep_wakeup_none;
	pm_tim_recover  	 	= pm_tim_recover_none;

	blt_miscParam.pm_enter_en 	= 1; // allow enter pm, 32k rc does not need to wait for 32k clk to be stable
}
#endif

#if (DRAFT_FEATURE_VENDOR_TYPE_SEL == DRAFT_FEATURE_VENDOR_TYPE_ONE_OP)
u32 get_cps_vendor_op_extend(u8 *cps_out)
{
    mesh_composition_data_local_t *p_cps = &model_sig_cfg_s_cps;
    mesh_composition_data_local_t *p_rsp_cps = (mesh_composition_data_local_t *)(cps_out);
    u8 *wptr = (u8 *)p_rsp_cps;
    u32 len_cp = OFFSETOF(mesh_composition_data_local_t, page0.ele_primary.md_sig);
    memcpy(wptr, p_cps, len_cp);
    wptr += len_cp;
    mesh_element_primary_t *p_src = &p_cps->page0.ele_primary;
    mesh_element_primary_t *p_dst = &p_rsp_cps->page0.ele_primary;
    p_dst->nums = 0;
    foreach_arr(i,p_dst->md_sig){
        if(!is_client_tx_extend_model(p_src->md_sig[i])){
            memcpy(wptr, &p_src->md_sig[i], sizeof(p_dst->md_sig[0]));
            p_dst->nums ++;
            wptr += sizeof(p_dst->md_sig[0]);
        }
    }
    len_cp = sizeof(mesh_composition_data_local_t) - OFFSETOF(mesh_composition_data_local_t, page0.ele_primary.md_vendor);
    memcpy(wptr, p_cps->page0.ele_primary.md_vendor, len_cp);
    wptr += len_cp;
    
    return (u32)((u32)wptr - (u32)cps_out);
}

int rf_link_get_vendor_op_para_extend(u8 *ac,  int len_ac, u16 *op, u8 **params, int *par_len)
{
    mesh_cmd_ac_vd_t *p_ac = (mesh_cmd_ac_vd_t *)ac;
    if(is_vendor_extend_op(p_ac->op)){
        u8 op_type = GET_OP_TYPE(p_ac->data[0]);
        if(OP_TYPE_VENDOR == op_type){
            return GET_OP_FAILED;   // should not happen here.
        }else{
            int ret = rf_link_get_op_para(p_ac->data, len_ac - OP_TYPE_VENDOR, op, params, par_len);
            return (GET_OP_FAILED != ret) ? GET_OP_SUCCESS_EXTEND : GET_OP_FAILED;
        }
    }

    return GET_OP_FAILED;
}

const u16 CLIENT_TX_EXTEND_MODEL[] = {MD_ID_ARRAY_MESH_OTA  MD_ID_ARRAY_REMOTE_PROV};
int is_client_tx_extend_model(u16 model)
{
    foreach_arr(i,CLIENT_TX_EXTEND_MODEL){
        if(CLIENT_TX_EXTEND_MODEL[i] == model){
            return 1;
        }
    }
    return 0;
}

int is_op_need_extend(u16 op)
{
    mesh_op_resource_t op_res;
    if(0 == mesh_search_model_id_by_op(&op_res, op, 1)){
        if(is_client_tx_extend_model(op_res.id)){
            return 1;
        }
    }
    return 0;
}

u16 get_vendor2sig_op(u16 op, u8 *p_sig_op)
{
    if(is_vendor_extend_op(op)){
        return get_op_u16(p_sig_op);
    }
    return 0;
}

u32 is_op_need_extend_and_fill_op_par(u16 op, u8 *ac_sig_out, u8 *par, u32 par_len)
{
    u32 op_size = GET_OP_TYPE(op);
    if(is_op_need_extend(op)){
        mesh_cmd_ac_vd_t *ac_vd = (mesh_cmd_ac_vd_t *)ac_sig_out;
        ac_vd->op = VD_EXTEND_CMD0;
        ac_vd->vd_id = g_vendor_id;
        memcpy(ac_vd->data, &op, op_size);
        memcpy(ac_vd->data + op_size, par, par_len);
        
        LOG_MSG_LIB(TL_LOG_NODE_BASIC, ac_sig_out, OP_TYPE_VENDOR,"TX fill extend op",0);
        return OP_TYPE_VENDOR;
    }

    return 0;
}

u32 is_op_need_extend_and_remove_op_par(u8 *rc_rsp_data, u8 *ac, int len_ac)
{
    if(is_vendor_extend_op(ac[0]) && (len_ac > OP_TYPE_VENDOR)){
        memcpy(rc_rsp_data, ac + OP_TYPE_VENDOR, len_ac - OP_TYPE_VENDOR);
        return OP_TYPE_VENDOR;
    }
    return 0;
}

int is_rx_need_extend_invalid_model(u16 model, int get_op_st)
{
    if((GET_OP_SUCCESS_EXTEND != get_op_st) && is_client_tx_extend_model(model)){
        LOG_MSG_LIB(TL_LOG_NODE_BASIC, 0, 0,"should not handle draft model ID: 0x%04x", model);
        return 1;
    }
    return 0;
}

bind_key_t * is_exist_bind_key_extend_op(u16 appkey_idx)
{
    return is_exist_bind_key(&model_vd_light.srv[0].com, appkey_idx);
}
#else
int is_client_tx_extend_model(u16 model){return 0;}
int rf_link_get_vendor_op_para_extend(u8 *ac,  int len_ac, u16 *op, u8 **params, int *par_len){return GET_OP_FAILED;}
u32 is_op_need_extend_and_fill_op_par(u16 op, u8 *ac_sig_out, u8 *par, u32 par_len){return 0;}
u32 is_op_need_extend_and_remove_op_par(u8 *rc_rsp_data, u8 *ac, int len_ac){return 0;}
int is_rx_need_extend_invalid_model(u16 model, int get_op_st){return 0;}
bind_key_t * is_exist_bind_key_extend_op(u16 appkey_idx){return 0;}
#endif

// -------- clock ---------
void clock_switch_to_highest()
{
#if (!WIN32 && (MCU_CORE_TYPE >= MCU_CORE_8258))
	#if (CLOCK_SYS_CLOCK_HZ < 48000000)
	unsigned char r = irq_disable();
	clock_init(SYS_CLK_48M_Crystal);
	irq_restore(r);
	#endif
#endif
}

void clock_switch_to_normal()
{
#if (!WIN32 && (MCU_CORE_TYPE >= MCU_CORE_8258))
	#if (CLOCK_SYS_CLOCK_HZ < 48000000)
	unsigned char r = irq_disable();
	clock_init(SYS_CLK_CRYSTAL);
	    #if ((MCU_CORE_TYPE == MCU_CORE_8258) && (CLOCK_SYS_CLOCK_HZ < 48000000))
	analog_write(0x0c, 0xc4);   // restore DCDC
	    #endif
	irq_restore(r);
	#endif
#endif
}

#if (EXTENDED_ADV_ENABLE)
	
#include "stack/ble/ble.h"


#define	APP_ADV_SETS_NUMBER						  1			// Number of Supported Advertising Sets
#define APP_MAX_LENGTH_ADV_DATA					  320		// Maximum Advertising Data Length,   (if legacy ADV, max length 31 bytes is enough)
#define APP_MAX_LENGTH_SCAN_RESPONSE_DATA		  31		// Maximum Scan Response Data Length, (if legacy ADV, max length 31 bytes is enough)


//_attribute_data_retention_
u8	app_adv_set_param[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];
//_attribute_data_retention_
u8	app_primary_adv_pkt[MAX_LENGTH_PRIMARY_ADV_PKT * APP_ADV_SETS_NUMBER];
//_attribute_data_retention_
u8	app_secondary_adv_pkt[MAX_LENGTH_SECOND_ADV_PKT * APP_ADV_SETS_NUMBER];
//_attribute_data_retention_
u8	app_advData[APP_MAX_LENGTH_ADV_DATA * APP_ADV_SETS_NUMBER];
//_attribute_data_retention_
u8	app_scanRspData[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];


void mesh_blc_ll_initExtendedAdv()
{
    blc_ll_initExtendedAdvertising_module(app_adv_set_param, app_primary_adv_pkt, APP_ADV_SETS_NUMBER);
    blc_ll_initExtSecondaryAdvPacketBuffer(app_secondary_adv_pkt, MAX_LENGTH_SECOND_ADV_PKT);
    blc_ll_initExtAdvDataBuffer(app_advData, APP_MAX_LENGTH_ADV_DATA);
    //  blc_ll_initExtScanRspDataBuffer(app_scanRspData, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);
    blc_ll_setAuxAdvChnIdxByCustomers(0);
}

void mesh_blc_ll_setExtAdvData(u8 adv_pdu_len, u8 *data)    // called in blt library
{
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, adv_pdu_len, data);
}

u8 mesh_blc_ll_setExtAdvParamAndEnable()
{
    u32 my_adv_interval_min = ADV_INTERVAL_MIN;
    u32 my_adv_interval_max = ADV_INTERVAL_MAX;


    blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, my_adv_interval_min,            my_adv_interval_max,
           BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
           ADV_FP_NONE,         TX_POWER_8dBm,                                                  BLE_PHY_1M,                     0,
           BLE_PHY_1M,          ADV_SID_0,                                                      0);

#if 1+TEST_EXT_ADV
    u8  testAdvData[300];

#if 0   //AdvData: 100 bytes, check that APP_MAX_LENGTH_ADV_DATA must bigger than 100
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, 100, testAdvData);
#elif 1 //AdvData: 251 bytes, check that APP_MAX_LENGTH_ADV_DATA must bigger than 300
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, 241, testAdvData);
#elif 0 //AdvData: 300 bytes, check that APP_MAX_LENGTH_ADV_DATA must bigger than 300
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_FIRST,    DATA_FRAGM_ALLOWED, 251, testAdvData);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_LAST,     DATA_FRAGM_ALLOWED, 49,  testAdvData + 251);
#elif 0 //AdvData: 600 bytes, check that APP_MAX_LENGTH_ADV_DATA must bigger than 600
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_FIRST,    DATA_FRAGM_ALLOWED, 251, testAdvData);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_INTER,    DATA_FRAGM_ALLOWED, 251, testAdvData + 251);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_LAST,     DATA_FRAGM_ALLOWED, 98,  testAdvData + 502);
#elif 1 //AdvData: 1010 bytes,  check that APP_MAX_LENGTH_ADV_DATA must bigger than 1010
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_FIRST,    DATA_FRAGM_ALLOWED, 251, testAdvData);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_INTER,    DATA_FRAGM_ALLOWED, 251, testAdvData + 251);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_INTER,    DATA_FRAGM_ALLOWED, 251, testAdvData + 502);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_INTER,    DATA_FRAGM_ALLOWED, 251, testAdvData + 753);
    blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_LAST,     DATA_FRAGM_ALLOWED, 6,   testAdvData + 1004);
#endif
#endif

    u8 status = blc_ll_setExtAdvEnable_1( BLC_ADV_ENABLE, 1, ADV_HANDLE0, 0 , 0);

    return status;
}
#endif

u32 mesh_max_payload_get (u32 ctl, bool4 extend_adv_short_unseg)
{
    mesh_cmd_lt_ctl_seg_t *p_lt_ctl_seg = NULL;
    mesh_cmd_lt_seg_t *p_lt_seg = NULL;
    
    u8 delta_extend_len = GET_DELTA_EXTEND_BEAR;

    #if ((MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR) || WIN32)
    if(extend_adv_short_unseg){
        delta_extend_len = 0;
    }
    #endif
    
    return ((ctl ? sizeof(p_lt_ctl_seg->data) : sizeof(p_lt_seg->data)) + delta_extend_len);
}

#if (GATEWAY_ENABLE && EXTENDED_ADV_ENABLE)
u8 g_gw_extend_adv_option = EXTEND_ADV_OPTION_ADV_ONLY;
#endif

int is_extend_unseg2short_unseg(u16 op)
{
#if (WIN32 || (GATEWAY_ENABLE && EXTENDED_ADV_ENABLE))
    #if WIN32
    u8 option_val = isVC_DLEModeExtendBearer();
    #else
    u8 option_val = g_gw_extend_adv_option;
    #endif
    
    if(EXTEND_ADV_OPTION_ALL == option_val){ // all op use extend adv
        return 0;
    }else if(EXTEND_ADV_OPTION_NONE == option_val){ // all op use extend adv
        return 1;
    }
#endif

    // -- EXTEND_ADV_OPTION_ADV_ONLY
#if ((MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR) || WIN32)
    if((FW_UPDATE_START == op)||(BLOB_CHUNK_TRANSFER == op)||(BLOB_BLOCK_STATUS == op)){  // TODO : only Chunk data now.
        // use update start message to check whether node support extend adv
        return 0;
    }else{
        return 1;
    }
#endif

    return 0;
}

int is_extend_unseg2short_unseg_ctl(u16 ctl_op)
{
#if ((MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR) || WIN32)
    return 1; // always use short unsegment now
#endif

    return 0;
}

/**
  * @}
  */

/**
  * @}
  */

