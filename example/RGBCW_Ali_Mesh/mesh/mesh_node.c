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
#include "mesh/remote_prov.h"
#include "mesh/fast_provision_model.h"
#include "mesh/app_heartbeat.h"
#include "mesh/app_health.h"
#include "version.h"

#if (ALI_MD_TIME_EN)
#include "user_ali_time.h"
#endif

#if (__PROJECT_MESH_SWITCH__ && (0 == PM_DEEPSLEEP_RETENTION_ENABLE))
#define MESH_CMD_SNO_SAVE_DELTA             (0x04)	// can't set too much, becaush of deep sleep
#else
#define MESH_CMD_SNO_SAVE_DELTA             (0x80)
#endif

/** @addtogroup Mesh_Common
  * @{
  */
  
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
mesh_tid_t mesh_tid;
u8 switch_project_flag = 0;

#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
u8 my_rf_power_index = MY_RF_POWER_INDEX;   // use in library
#endif

/*PTS test*/
u8 pts_test_en = PTS_TEST_EN;

MYFIFO_INIT(mesh_adv_cmd_fifo, sizeof(mesh_cmd_bear_unseg_t), MESH_ADV_CMD_BUF_CNT);
MYFIFO_INIT_NO_RET(mesh_adv_fifo_relay, sizeof(mesh_relay_buf_t), MESH_ADV_BUF_RELAY_CNT);
#if !__PROJECT_MESH_PRO__
#if __PROJECT_SPIRIT_LPN__
MYFIFO_INIT_NO_RET(blt_notify_fifo, 36, 64);//save retention
#else
#if FEATURE_LOWPOWER_EN
MYFIFO_INIT_NO_RET(blt_notify_fifo, 36, 8);
#else
MYFIFO_INIT(blt_notify_fifo, 36, 64);
#endif
#endif
#endif

STATIC_ASSERT(sizeof(mesh_net_key_t) % 16 == 0);
STATIC_ASSERT(sizeof(mesh_app_key_t) % 4 == 0);
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key) == ARRAY_SIZE(mesh_fri_key_lpn));
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key[0]) == ARRAY_SIZE(mesh_fri_key_lpn[0]));
STATIC_ASSERT(ARRAY_SIZE(mesh_key.net_key[0]) == ARRAY_SIZE(mesh_fri_key_fn[0]));
STATIC_ASSERT(NET0 == 0);
STATIC_ASSERT(ELE_CNT <= ELE_CNT_MAX_LIB);  // don't cancle this assert
STATIC_ASSERT((NET_KEY_MAX <= 255) && (APP_KEY_MAX <= 255));          // only one byte for array index
STATIC_ASSERT(MD_SERVER_EN || MD_CLIENT_EN);
STATIC_ASSERT((BUILD_VERSION & 0x00FF0000) != 0x00A50000);  // because ram[840004] is a special flag which is 0xA5, in cstartup_S.

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

#define MD_ID_ARRAY_CFG         SIG_MD_CFG_SERVER, MD_ID_ARRAY_CFG_CLIENT       \
                                SIG_MD_HEALTH_SERVER, SIG_MD_HEALTH_CLIENT,     \
                                MD_ID_REMOTE_PROV_SERVER MD_ID_REMOTE_PROV_CLIENT \



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

#if MD_CLIENT_EN
#define MD_ID_ARRAY_MESH_OTA_CLIENT     SIG_MD_FW_DISTRIBUT_C,
#else
#define MD_ID_ARRAY_MESH_OTA_CLIENT  
#endif
#if MD_MESH_OTA_EN
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
#define MD_ID_ARRAY_MESH_OTA_DISTRIBUTE       SIG_MD_FW_DISTRIBUT_S,
    #else
#define MD_ID_ARRAY_MESH_OTA_DISTRIBUTE       
    #endif
#define MD_ID_ARRAY_MESH_OTA    SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C,     \
                                MD_ID_ARRAY_MESH_OTA_DISTRIBUTE  MD_ID_ARRAY_MESH_OTA_CLIENT \
                                SIG_MD_OBJ_TRANSFER_S, SIG_MD_OBJ_TRANSFER_C,
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
#define MD_ID_ARRAY_SENSOR_SERVER		SIG_MD_SENSOR_S, SIG_MD_SENSOR_SETUP_S,
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
#define	MD_ID_ARRAY_SENSOR_SERVER
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
#define MD_ID_ARRAY_SENSOR_CLIENT		SIG_MD_SENSOR_C,
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
#define	MD_ID_ARRAY_SENSOR_CLIENT
#define MD_ID_ARRAY_PROP_CLIENT			
#define	MD_ID_ARRAY_BATTERY_CLIENT
#define MD_ID_ARRAY_LOCATION_CLIENT
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

#if (0 == DEBUG_EVB_EN)
#define MD_ID_ARRAY_PRIMARY     MD_ID_ARRAY_CFG   MD_ID_ARRAY_MESH_OTA   MD_ID_ARRAY_TIME   MD_ID_ARRAY_COMMON
#else
#define MD_ID_ARRAY_PRIMARY     MD_ID_ARRAY_CFG   MD_ID_ARRAY_MESH_OTA   MD_ID_ARRAY_ONOFF
#endif
#define MD_ID_ARRAY_PRIMARY_VD  MD_ID_ARRAY_VENDOR_SERVER  MD_ID_ARRAY_VENDOR_CLIENT

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
                FEATURE_FRIEND_EN,      // u16 frid        :1;
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

void ev_handle_traversal_cps_ll(u32 ev, u8 *par, u16 ele_adr, u32 model_id, int sig_model)
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

u8 find_ele_support_model_and_match_dst(mesh_adr_list_t *adr_list, u16 adr_dst, u32 model_id, int sig_model)
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

int mesh_sec_msg_dec_virtual (u8 *key, u8 *nonce, u8 *dat, int n, int mic_length, u16 adr_dst, u8 *dat_org)
{
	#if MESH_MONITOR_EN
	return -1;
	#endif
	
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
u8 get_ele_offset_by_model(mesh_page0_t * p_page0, u32 len_page0, u16 node_adr, u16 obj_adr, u32 model_id, int sig_model)
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
int is_support_model_dst(u16 adr_dst, u32 model_id, int sig_model)
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
        if(is_cfg_model(op_res.id, op_res.sig)){
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

u32 get_mesh_pub_interval_ms(u32 model_id, int sig_model, mesh_pub_period_t *period)
{
	u32 step_res_ms = get_transition_100ms((trans_time_t *)period)*100;
	step_res_ms = step_res_ms + (step_res_ms / PUB_RANDOM_RATE);
	if((sig_model && (SIG_MD_HEALTH_SERVER == model_id)) && (model_sig_health.srv.health_mag.cur_sts.cur_fault_idx != 0)){
		step_res_ms = step_res_ms>>model_sig_health.srv.health_mag.period_sts.fast_period_log;
	}
	#if(MD_SERVER_EN && MD_SENSOR_EN)
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
        //static u8 AA_1_cps;AA_1_cps++;
		u8 rsp[1 + sizeof(model_sig_cfg_s_cps.page0)];
		rsp[0] = 0;
		memcpy(rsp+1, &model_sig_cfg_s_cps.page0, sizeof(model_sig_cfg_s_cps.page0));
        return mesh_tx_cmd_rsp_cfg_model(COMPOSITION_DATA_STATUS, rsp, sizeof(rsp), cb_par->adr_src);
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
	#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
	if(!is_own_ele(adr) && is_unicast_adr(adr)){
		return VC_master_get_other_node_dev_key(adr);
	}
	#endif
	
	return mesh_key.dev_key;
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

		int new_key_flag = is_key_refresh_use_new_key(p_mat->nk_array_idx);
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
            p_key_str = (mesh_app_key_t *)mesh_cfg_cmd_dev_key_get(p_mat->adr_src);
        }
		else{
			p_key_str = (mesh_app_key_t *)mesh_cfg_cmd_dev_key_get(p_mat->adr_dst);
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

mesh_net_key_t * is_mesh_net_key_exist(u16 key_idx)
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
		if((p_netkey->valid)&&(key_idx == p_netkey->index)){
			return p_netkey;
		}
	}
    return 0;
}

int is_net_key_save()
{
	u32 val = 0;
	flash_read_page(FLASH_ADR_MESH_KEY, 4, (u8 *)&val);
	return (0xffffffff != val);
}

void net_key_set2(mesh_net_key_t *key, const u8 *nk, u16 key_idx, int save)
{
	memcpy(key->key, nk, 16);
	mesh_sec_get_nid_ek_pk_master(&key->nid_m, key->ek_m, key->pk_m, key->key);
	mesh_sec_get_network_id(key->nw_id, key->key);
	mesh_sec_get_identity_key (key->idk, key->key);
	mesh_sec_get_beacon_key (key->bk, key->key);
	// update friend key later
	
	key->index = key_idx;
	key->valid = KEY_VALID;
	key->node_identity = model_sig_cfg_s.node_identity_def;
	
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

u8 GetNKArrayIdxByPointer(mesh_net_key_t *key)
{
    u8 nk_arry_idx = (((((u32)key)-((u32)&mesh_key.net_key))/sizeof(mesh_net_key_t))/2)%NET_KEY_MAX;
    return nk_arry_idx;
}

void mesh_friend_key_refresh(mesh_net_key_t *new_key)
{
    u8 nk_array_idx = GetNKArrayIdxByPointer(new_key);
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
        if(clock_time_exceed(mesh_rx_seg_par.tick_last, SEG_RX_TIMEOUT_MS*1000)){
            mesh_rx_seg_par.tick_last = mesh_rx_seg_par.tick_seg_idle = 0;
            mesh_rx_seg_par.status = SEG_RX_STATE_TIMEOUT;
        }else if(clock_time_exceed(mesh_rx_seg_par.tick_seg_idle, SEG_RX_ACK_IDLE_MS*1000)){
            mesh_rx_seg_par.tick_seg_idle = clock_time()|1;
            mesh_add_seg_ack(&mesh_rx_seg_par);
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


void mesh_friend_key_RevokingOld(mesh_net_key_t *new_key)
{
    u8 nk_array_idx = GetNKArrayIdxByPointer(new_key);
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
    SIG_MD_G_ONOFF_S, SIG_MD_LIGHTNESS_S, SIG_MD_FW_UPDATE_S, SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_TEMP_S, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_XYL_S
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
	}

	if(!bind_flag){
        memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
	}
}
#endif

void appkey_bind_filter(int bind_flag, u16 ele_adr, u32 model_id, int sig_model, u16 ak_idx, int fac_filter_en)
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

u8 mesh_sub_search_ele_and_set(u16 op, u16 ele_adr, u16 sub_adr, u8 *uuid, u32 model_id, int sig_model)
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
					#if PROVISION_FLOW_SIMPLE_EN
						#if DUAL_VENDOR_EN
						if(DUAL_VENDOR_ST_ALI == provision_mag.dual_vendor_st)
						#endif
						{
    						if(!mesh_init_flag){
    						    if(get_all_appkey_cnt() == 1){
                                    #if (FEATURE_LOWPOWER_EN || SPIRIT_PRIVATE_LPN_EN)
                                    if(!lpn_provision_ok)
                                    #endif
    						        {
    						            node_binding_tick = clock_time() | 1;
    						        }
    						        
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
	
    if(is_actived_factory_test_mode()){
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
	node_binding_tick = clock_time()|1;
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
    return ((adr >= ele_adr_primary)&&(adr < ele_adr_primary + ELE_CNT));   // is_ele_in_node_()
}

#define IV_UPDATE_START_SNO             (0xC00000)  // margin should be enough, because sometimes can't keep 96 hour powered. so, should be enough margin to restart iv update flow next power up. 
#define IV_UPDATE_KEEP_TMIE_MIN_S       (96*3600)//(96*3600)       // 96 hour
#define IV_UPDATE_KEEP_TMIE_MIN_RX_S    (96*3600)//(96*3600)       // 96 hour

int is_sno_exhausted()
{
    return (mesh_adv_tx_cmd_sno >= IV_UPDATE_START_SNO);
}

int is_iv_update_keep_enough_time_ll() // for trigger role
{
    return (iv_idx_st.keep_time_s > IV_UPDATE_KEEP_TMIE_MIN_S);
}

int is_iv_update_keep_enough_time_rx()      // for rx role
{
#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
    return 1;
#else
    return (iv_idx_st.keep_time_s >= IV_UPDATE_KEEP_TMIE_MIN_RX_S);
#endif
}

//receive security network beacon that it's iv index is equal to (current_iv_index + 1) in normal stage.
void mesh_receive_ivi_plus_one_in_normal_cb()
{
#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
	//set searching mode enter IV Index Recovery procedure
    mesh_iv_update_enter_search_mode();
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
    unsigned char ble_moudle_id_is_kmadongle();
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
	if(pub_md){
		ak_array_idx = get_ak_arr_idx(mesh_key.netkey_sel_dec, pub_md->pub_par.appkey_idx);
	}else{
		ak_array_idx = mesh_key.appkey_sel_dec; // use the same key with RX command. 
	}
	
//	LOG_MSG_LIB(TL_LOG_NODE_SDK,par,par_len,"cmd data rsp: adr_src0x%04x,dst adr 0x%04x ",adr_src,adr_dst);
	
	set_material_tx_cmd(&mat, op, par, par_len, adr_src, adr_dst, g_reliable_retry_cnt_def, 0, uuid, mesh_key.netkey_sel_dec, ak_array_idx, pub_md);
	return mesh_tx_cmd_unreliable(&mat);
}

int mesh_tx_cmd_rsp_cfg_model(u16 op, u8 *par, u32 par_len, u16 adr_dst)
{
    if(is_tx_status_cmd2self(op, adr_dst)){
        return 0;
    }
	material_tx_cmd_t mat;
	set_material_tx_cmd(&mat, op, par, par_len, ele_adr_primary, adr_dst, g_reliable_retry_cnt_def, 0, 0, mesh_key.netkey_sel_dec, -1, 0);
	return mesh_tx_cmd_unreliable(&mat);
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
    int sig_model;
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
	{1, {SIG_MD_TIME_S, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, SIG_MD_SCHED_S, SIG_MD_SCHED_C, SIG_MD_SCHED_SETUP_S}, FLASH_ADR_MD_TIME_SCHEDULE},
	{1, {SIG_MD_SCENE_S, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, -1, -1, -1}, FLASH_ADR_MD_SCENE},
    {1, {SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C, SIG_MD_FW_DISTRIBUT_S, SIG_MD_FW_DISTRIBUT_C, SIG_MD_OBJ_TRANSFER_S, SIG_MD_OBJ_TRANSFER_C}, FLASH_ADR_MD_MESH_OTA},
	{1, {SIG_MD_G_POWER_ONOFF_S, SIG_MD_G_POWER_ONOFF_C, SIG_MD_G_POWER_ONOFF_SETUP_S, SIG_MD_G_DEF_TRANSIT_TIME_S, SIG_MD_G_DEF_TRANSIT_TIME_C, -1}, FLASH_ADR_MD_G_POWER_ONOFF},
#if MD_REMOTE_PROV
    {1, {SIG_MD_REMOTE_PROV_SERVER, SIG_MD_REMOTE_PROV_CLIENT, -1, -1, -1, -1}, FLASH_ADR_MD_REMOTE_PROV},
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
    {FLASH_ADR_MD_REMOTE_PROV, (u8 *)&model_remote_prov, &mesh_md_rp_addr, sizeof(model_remote_prov)},
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
				flash_write_page(adr_read+i, 1, &p_in[i]);
			}
		}
		
		len -= len_read;
		pos += len_read;
	}
	mesh_flash_write_check(adr, in, size);
}

void flash_write_with_check(u32 adr, u32 size, const u8 *in)
{
	flash_write_page(adr, size, in);
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
void mesh_par_store(const u8 *in, u32 *p_adr, u32 adr_base, u32 size){
#if WIN32 
    mesh_par_retrieve_store_win32((u8 *)in, p_adr, adr_base, size,MESH_PARA_STORE_VAL);
#else
	u32 size_save = (size + SIZE_SAVE_FLAG);
	if(*p_adr > (adr_base + FLASH_SECTOR_SIZE - size_save - SIZE_SAVE_FLAG)){	// make sure "0xffffffff" at the last for retrieve
        *p_adr = adr_base;
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
	// make sure when change to the save_flag ,the flash must be write already .
	u32 save_flag = SAVE_FLAG_PRE;
	flash_write_with_check(*p_adr, SIZE_SAVE_FLAG, (u8 *)&save_flag);
	flash_write_with_check(*p_adr + SIZE_SAVE_FLAG, size, in);
	save_flag = SAVE_FLAG;
	flash_write_with_check(*p_adr, SIZE_SAVE_FLAG, (u8 *)&save_flag);
	
	if(*p_adr >= adr_base + size_save){
		u32 zero = 0;
    	flash_write_page(adr_last, SIZE_SAVE_FLAG, (u8 *)&zero);  // clear last flag
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
    mesh_tx_sec_nw_beacon_all_net(1);
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
				}
			}
		}
	}
	
    return err;
}

// model save
int mesh_model_retrieve_and_store(int sig_model, u32 md_id, int save)
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

int mesh_model_retrieve(int sig_model, u32 md_id)
{
	return mesh_model_retrieve_and_store(sig_model, md_id, 0);
}

int mesh_model_store(int sig_model, u32 md_id)
{
	return mesh_model_retrieve_and_store(sig_model, md_id, 1);
}

// common
void mesh_flash_save_check()
{
	mesh_sno_save_check();
	#if 0
	mesh_md_cfg_s_save_check();
	#endif
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
	#if __PROJECT_MESH_PRO__
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
			if(p_br->beacon.type != SECURE_BEACON){
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
#if !WIN32
	#if __PROJECT_MESH_PRO__
	return 0;
	#else
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
	#endif
#else
	return 0;
#endif
}
#define FULL_REALY_PROC_ENABLE	1

int relay_adv_prepare_handler(rf_packet_adv_t * p)  // no random for relay transmit
{
    my_fifo_t *p_fifo = &mesh_adv_fifo_relay;
#if FULL_REALY_PROC_ENABLE
    my_fifo_poll_relay(p_fifo);   // must before get buffer.
    mesh_relay_buf_t *p_relay = my_fifo_get_relay(p_fifo);
    if(p_relay){
        #if WIN32
        LOG_MSG_INFO(TL_LOG_NODE_BASIC,(u8 *)p_relay,p_relay->bear.len + 6,"Relay Buff:",0);
        #else
        mesh_adv_cmd_set((u8 *)p, (u8 *)&p_relay->bear);
        #endif

        if(0 == p_relay->cnt){
            p_relay->valid = 0;
            if((u8 *)p_relay == my_fifo_get(p_fifo)){
                my_fifo_pop(p_fifo);
                #if WIN32
                LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0, "Relay buffer pop", 0);
                #endif
            }else{
                #if WIN32
                LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0, "Relay buffer pop later in poll", 0);
                #endif
            }
        }else{
            if(p_relay->tick_10ms){ // no need, but should be better.
                p_relay->tick_10ms--;   // start tick
            }
        }
        return 1;
    }
#else
	mesh_relay_buf_t *p_relay = (mesh_relay_buf_t *)my_fifo_get(p_fifo);
	if(p_relay){
		mesh_transmit_t *p_trans_par = (mesh_transmit_t *)(&p_relay->bear.trans_par_val);
		static u32 relay_10ms_cnt =0;
		relay_10ms_cnt++;
		if(relay_10ms_cnt > p_trans_par->invl_steps){
			relay_10ms_cnt =0;
			mesh_adv_cmd_set((u8 *)p, (u8 *)&p_relay->bear);
			if(p_trans_par->count == 0){
				my_fifo_pop(p_fifo);
			}else{
				p_trans_par->count--;
			}
			return 1;
		}
	}
#endif
    return 0;
	
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
#endif

void proc_node_reset()
{
	if(del_node_tick && clock_time_exceed(del_node_tick, del_node_delay_ms * 1000)){
		del_node_tick = 0;	// must for WIN32
		kick_out();	// will reboot inside it.	
	}
	#if MANUAL_FACTORY_RESET_TX_STATUS_EN
	else if(manual_factory_reset){
	    my_fifo_reset(&mesh_adv_cmd_fifo);  // discard all command in buffer.
        mesh_tx_cmd_rsp_cfg_model(NODE_RESET_STATUS, 0, 0, ADR_ALL_NODES);
		kick_out();	// will wait tx cmd completed, and reboot later	
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

    		if (my_fifo_data_cnt_get(&blt_notify_fifo) < 10)
    		{
    		    online_st_report_t report = {MESH_ADV_TYPE_ONLINE_ST};
    		    report.len_node_st_val = sizeof(mesh_node_st_val_t);
    		    report.sno = rand();
    			if (mesh_node_report_status ((u8 *)&report.node, ARRAY_SIZE(report.node)))
    			{
                    online_st_gatt_enc((u8 *)&report, sizeof(report));
                    u8 header[1] = {ONLINE_ST_ATT_HANDLE_SLAVE};
    				my_fifo_push(&blt_notify_fifo,(u8 *)&report,sizeof(report),header,sizeof(header));
    			}
    		}
    	}
	}
}

void online_st_force_notify_check(mesh_cmd_bear_unseg_t *p_bear, u8 *ut_dec, int src_type)
{
    if(mesh_node_report_enable && (ADV_FROM_GATT == src_type)){
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

void mesh_loop_process()
{
    #if MD_REMOTE_PROV
    // remote prov proc
    mesh_cmd_sig_rp_loop_proc();
    #endif
    // provision loop proc
	mesh_prov_proc_loop();
	// mesh beacon proc 
	mesh_beacon_send_proc();
	// mesh proxy proc 
	caculate_proxy_adv_hash_task();
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
	mesh_ota_master_proc();
	mesh_kr_cfgcl_proc();
	check_mesh_kr_cfgcl_timeout();
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
	#if !__PROJECT_MESH_PRO__
	mesh_notifyfifo_rxfifo();
	#endif
	mesh_tid_timeout_check();
	#if RELIABLE_CMD_EN
	mesh_tx_reliable_proc();
	#endif
	mesh_flash_save_check();
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

u8 mesh_init_flag = 1;
void mesh_init_all()
{	
    mesh_init_flag = 1;
	uart_simu_send_bytes("          \r\n", 12);//第一行数据总是乱码
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
    // read parameters
    mesh_flash_retrieve();	// should be first, get unicast addr from config modle.
	mesh_provision_para_init();
	//unprovision beacon send part 
	beacon_str_init();
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
    mesh_init_flag = 0;
}

int app_event_handler_adv(u8 *p_payload, int src_type, u8 need_proxy_and_trans_par_val)
{
	int err = 0;
	#if (MESH_MONITOR_EN)
	if(need_proxy_and_trans_par_val){
		app_event_handler_adv_monitor(p_payload);
		return 0;
	}
	#endif
	
	mesh_cmd_bear_unseg_t *p_br = GET_BEAR_FROM_ADV_PAYLOAD(p_payload);
	u8 adv_type = p_br->type;
	if(adv_type == MESH_ADV_TYPE_MESSAGE){
        lpn_debug_set_event_handle_pin(1);
		err = mesh_rc_data_layer_network(p_payload, src_type, need_proxy_and_trans_par_val);
        lpn_debug_set_event_handle_pin(0);
        if(is_lpn_support_and_en){
            suspend_quick_check();    // rx handle ok
        }
	}
	else if((adv_type == MESH_ADV_TYPE_BEACON)&&(p_br->beacon.type == SECURE_BEACON)){
		mesh_rc_data_beacon_sec(p_payload, 0);
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
	app_event_handler_adv(&p_bear->len, ADV_FROM_GATT, p_bear->trans_par_val);
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
    if(len_payload > 31){
        return 0;
    }
    
    rf_packet_adv_t *p = (rf_packet_adv_t *)p_adv;
    
    p->header.type = LL_TYPE_ADV_NONCONN_IND;  
    memcpy(p->advA,tbl_mac,6);
    memcpy(p->data, &p_br->len, len_payload);
    p->rf_len = 6 + len_payload;
    p->dma_len = p->rf_len + 2;

    return 1;
}
#endif
#endif

//--------------------app key bind flow------------------------------//
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
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
	
	if(is_busy_segment_flow() || (KR_CFGCL_NORMAL == p->st) || (!p->next_st_flag)){
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

	if(is_busy_segment_flow() || (KR_CFGCL_NORMAL == p->st)){
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
    return (!is_cfg_model(*p_mode_id, 1) && (SIG_MD_HEALTH_CLIENT != *p_mode_id));
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
		#endif
	#if MD_REMOTE_PROV
	}else if (mesh_rsp_opcode_is_rp(op)){
        #if WIN32
        mesh_rp_client_rx_cb(rsp->data,rsp->src);
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
				#if GATEWAY_ENABLE
				gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_CPS_INFO,(u8*)&(rsp->src),rsp->len);
				#endif
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
					key_refresh_cfgcl_proc.model_bind_index++;
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

/**
  * @}
  */

/**
  * @}
  */

