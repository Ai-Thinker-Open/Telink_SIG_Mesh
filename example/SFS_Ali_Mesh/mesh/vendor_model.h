/********************************************************************************************************
 * @file     vendor_model.h 
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

#include "proj/tl_common.h"

#if (VENDOR_MD_NORMAL_EN)
// vendor model id
#if TESTCASE_FLAG_ENABLE
#define VENDOR_MD_LIGHT_S               ((0x0000<<16) | (0))
#define VENDOR_MD_LIGHT_C               ((0x0001<<16) | (0))
#define VENDOR_MD_LIGHT_S2              ((0x0002<<16) | (0))
#else
    #if AIS_ENABLE
#define TEMP_VD_ID_MODEL                (SHA256_BLE_MESH_PID)   // TEMP_VD_ID_MODEL is just use in this file.
    #else
#define TEMP_VD_ID_MODEL                (VENDOR_ID)
    #endif
#define VENDOR_MD_LIGHT_S               ((0x0000<<16) | (TEMP_VD_ID_MODEL))
#define VENDOR_MD_LIGHT_C               ((0x0001<<16) | (TEMP_VD_ID_MODEL))
#define VENDOR_MD_LIGHT_S2              ((0x0002<<16) | (TEMP_VD_ID_MODEL))
#endif

// op cmd 11xxxxxx yyyyyyyy yyyyyyyy (vendor)
// ---------------------------------from 0xC0 to 0xFF
#if (VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_SPIRIT)
// all op from 0xC0 to 0xFF
    #if DEBUG_VENDOR_CMD_EN
#define VD_LIGHT_ONOFF_GET				0xC1
#define VD_LIGHT_ONOFF_SET				0xC2
#define VD_LIGHT_ONOFF_SET_NOACK		0xC3
#define VD_LIGHT_ONOFF_STATUS		    0xC4
    #endif
    
    #if SPIRIT_VENDOR_EN
#define VD_MSG_ATTR_GET					0xD0
#define VD_MSG_ATTR_SET					0xD1
#define VD_MSG_ATTR_SET_NACK			0xD2
#define VD_MSG_ATTR_STS					0xD3
#define VD_MSG_ATTR_INDICA				0xD4
#define VD_MSG_ATTR_CONFIRM				0xD5
		#if ALI_MD_TIME_EN
#define VD_MSG_ATTR_UPD_TIME_REQ		0xDE
#define VD_MSG_ATTR_UPD_TIME_RSP		0xDF
		#endif
    #endif

#elif(VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_DEFAULT)
// ------ 0xC0 to 0xDF for telink used
#define VD_RC_KEY_REPORT				0xC0
    #if DEBUG_VENDOR_CMD_EN
#define VD_LIGHT_ONOFF_GET				0xC1
#define VD_LIGHT_ONOFF_SET				0xC2
#define VD_LIGHT_ONOFF_SET_NOACK		0xC3
#define VD_LIGHT_ONOFF_STATUS		    0xC4
    #endif
    
    #if FAST_PROVISION_ENABLE
#define VD_MESH_RESET_NETWORK			0xC5
#define VD_MESH_ADDR_GET				0xC6
#define VD_MESH_ADDR_GET_STS			0xC7
#define VD_MESH_ADDR_SET				0xC8
#define VD_MESH_ADDR_SET_STS			0xC9
#define VD_MESH_PROV_DATA_SET			0xCA
#define VD_MESH_PROV_CONFIRM			0xCB
#define VD_MESH_PROV_CONFIRM_STS		0xCC
#define VD_MESH_PROV_COMPLETE			0xCD
    #endif

#define VD_OP_RESERVE_FOR_TELINK_START  0xCE
#define VD_OP_RESERVE_FOR_TELINK_END    0xDF

//------0xE0 ~ 0xFF for customer
    #if LPN_VENDOR_SENSOR_EN
#define VD_LPN_SENSOR_GET               0xE0    // user can use this op code for other fuction
#define VD_LPN_SENSOR_STATUS            0xE1    // user can use this op code for other fuction
    #endif

//------ end
#endif

//------op parameters
typedef struct{
	u8 code;
	u8 rsv[7];
}vd_rc_key_report_t;

typedef struct{
	u8 onoff;
	u8 tid;
}vd_light_onoff_set_t;

typedef struct{
	u8 present_onoff;
}vd_light_onoff_st_t;

// ------------------
#if SPIRIT_VENDOR_EN
#define ATTR_TYPE_MAX_NUM 		2
#define ATTR_PAR_MAX_LEN		64
#define ATTR_TYPE_NOT_EXIST		0xff

//----------------system attributes--------------------------
#define ATTR_ERR_CODE				0x0000
#define ATTR_VERSION				0xff01
#define ATTR_DEVICE_FEATURE			0xff02
#define ATTR_FLASH_CAPACITY			0xff03
#define ATTR_FLASH_USED				0xff04
#define ATTR_FLASH_FREE				0xff05
#define ATTR_ENGINEER_MODE			0xff06
//------------------general attributes-----------------------
#define ATTR_WORKING_STATUS			0xf001
#define ATTR_USER_ID				0xf002
#define ATTR_DEVICE_NAME			0xf003
#define ATTR_SCENE_MODE				0xf004
#define	ATTR_REGULARLY_REPORT		0xf005
#define	ATTR_REGULARLY_SET			0xf006
#define	ATTR_REMAINING_TIME			0xf007
#define	ATTR_ONOFF_PLAN				0xf008
#define ATTR_EVENT					0xf009
#define ATTR_RSSI					0xf00a
#define ATTR_PARA_STEP				0xf00b
#define ATTR_ELEMENT_NUM			0xf00c
//------------------general physical event
#define ATTR_ONOFF					0x0100
#define ATTR_DATE					0x0101
#define	ATTR_TIME					0x0102
#define ATTR_UNIX_TIME				0x0103
#define ATTR_BAT_PERCENT			0x0104
#define ATTR_VOLTAGE				0x0105
#define ATTR_CURRENT				0x0106
#define ATTR_POWER					0x0107
#define ATTR_TARGET_TEMP			0x010c
#define ATTR_CURRENT_TEMP			0x010d

//------------------error code
#define DEVICE_NOT_READY	 		0x80
#define ATTR_NOT_SUPPORT			0x81
#define OPERATION_NOT_SUPPORT		0x82
#define PARAMETER_ERROR				0x83
#define DEVICE_STS_ERR				0x84

typedef struct{
	u8 tid;
	u16 attr_type;
}vd_msg_attr_get_t;

typedef struct{
	u16 attr_type;
	union{
		u8 attr_par[ATTR_PAR_MAX_LEN];
		u16 attr_u16;
	};
}vd_msg_attr_t;

typedef struct{
	u8 tid;
	u16 attr_type;
	u8 attr_par[ATTR_PAR_MAX_LEN];
}vd_msg_attr_set_t;

typedef struct{
	u8 tid;
	u16 attr_type;
	union{
	u8 err_code;
	u8 attr_par[ATTR_PAR_MAX_LEN];
	};
}vd_msg_attr_sts_t;

typedef struct{
	u8 op;
	u8 tid;
	u16 attr_type;
	u8 attr_par[ATTR_PAR_MAX_LEN];
}vd_msg_attr_indica_t;

typedef struct{
	u8 tid;
	u16 attr_type;
}vd_msg_attr_confirm_t;

typedef struct{
    u32 tick;
	u32 interval_tick;
	u8 busy;
    u8 retry_cnt;
	u8 tid;
    material_tx_cmd_t mat;
    mesh_match_type_t match_type;
    u8 ac_par[ATTR_PAR_MAX_LEN];  
}mesh_tx_indication_t;

typedef struct{
    u32 last_tick;
	u32 run_time_us;
	u32 sleep_time_us;
	u8  appWakeup_flg;
	u8  soft_timer_pending;
	u8  soft_timer_send_flag;
}mesh_sleep_pre_t;

extern mesh_tx_indication_t mesh_indication_retry;
extern mesh_sleep_pre_t	mesh_sleep_time;

#endif


//------------------vendor op end-------------------
void mesh_tx_indication_proc();

int vd_cmd_key_report(u16 adr_dst, u8 key_code);
int vd_cmd_onoff(u16 adr_dst, u8 rsp_max, u8 onoff, int ack);
int vd_light_onoff_st_publish(u8 idx);
int access_cmd_attr_indication(u16 op, u16 adr_dst, u16 attr_type, u8 *attr_par, u8 par_len);
void APP_set_vd_id_mesh_cmd_vd_func(u16 vd_id);

int mesh_search_model_id_by_op_vendor(mesh_op_resource_t *op_res, u16 op, u8 tx_flag);
int is_cmd_with_tid_vendor(u8 *tid_pos_out, u16 op, u8 tid_pos_vendor_app);
int cb_app_vendor_all_cmd(mesh_cmd_ac_vd_t *ac, int ac_len, mesh_cb_fun_par_vendor_t *cb_par);


int cb_vd_lpn_sensor_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_lpn_sensor_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_lpn_sensor_st_publish(u8 idx);
int vd_lp_sensor_st_send(u8 idx);
void mesh_vd_lpn_pub_set();
void sensor_proc_loop();
void i2c_io_init();


#endif

