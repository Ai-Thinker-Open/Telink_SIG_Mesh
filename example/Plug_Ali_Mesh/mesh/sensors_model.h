/********************************************************************************************************
 * @file     sensors_model.h 
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
#include "proj_lib/sig_mesh/app_mesh.h"
/****************************************************
Sensor Property ID
https://www.bluetooth.com/specifications/gatt/characteristics
*****************************************************/

#define PROP_ID						0x004E		

//----------------------------------- op code
// op cmd 0xxxxxxx (SIG)
// op cmd 10xxxxxx xxxxxxxx (SIG)

//sensor
#define SENSOR_DESCRIP_GET			0x3082
#define SENSOR_DESCRIP_STATUS		0x51
#define SENSOR_GET					0x3182
#define SENSOR_STATUS				0x52
#define SENSOR_COLUMN_GET			0x3282
#define SENSOR_COLUMN_STATUS		0x53
#define SENSOR_SERIES_GET			0x3382
#define SENSOR_SERIES_STATUS		0x54

//sensor setup
#define SENSOR_CANDECE_GET			0x3482
#define SENSOR_CANDECE_SET			0x55
#define SENSOR_CANDECE_SET_NOACK	0x56
#define SENSOR_CANDECE_STATUS		0x57
#define SENSOR_SETTINGS_GET			0x3582
#define SENSOR_SETTINGS_STATUS		0x58
#define SENSOR_SETTING_GET			0x3682
#define SENSOR_SETTING_SET			0x59
#define SENSOR_SETTING_SET_NOACK	0x5A
#define SENSOR_SETTING_STATUS		0x5B

typedef struct{
	u16 prop_id;
	u32 positive_tolerance:12;
	u32 negative_tolerance:12;
	u32 sampling_func:8;
	u8  measure_period;
	u8	update_interval;
}mesh_cmd_sensor_descript_st_t;

typedef struct{
	u16 format:1;  // 0
	u16 length:4;
	u16 prop_id:11;
	u8  raw_value[4];
}sensor_mpid_a_t;

typedef struct{
 	u8 format:1;   // 1
	u8 length:7;
	u16 prop_id;
	u8 raw_value[4];
 }sensor_mpid_b_t;

typedef struct{
	u8 raw_len;
 	sensor_mpid_b_t sensor_mpid;
 }sensor_mpid_b_st_t;

typedef struct{
 	u8 format:1;   // 1
	u8 length:7;
	u8 raw_value[4];
 }sensor_npid_b_t;

typedef struct{
	u16 prop_id;
	u16 setting_prop_id[SENSOR_SETTINGS_NUMS];
} sensor_settings_st_t;

typedef struct{
	u8 fast_period_div:7;
	u8 trig_type:1;
	u32 delta_down:24;
	u32 delta_up:24;
	u32 min_interval:8;
	u32 cadence_low:24;
	u32 cadence_hight:24;
}sensor_cadence_u24_t;

typedef struct{
	u16 prop_id;
	sensor_cadence_t cadence;
}sensor_cadence_st_t;

typedef struct{
	u16 prop_id;
	u16 setting_id;
}sensor_setting_get_t;

typedef struct{
	u16 prop_id;
	u16 setting_id;
	u8  setting_access;
	u16  setting_raw;
}sensor_setting_st_t;

typedef struct{
	u16 prop_id;
	u16 setting_id;
	u16  setting_raw;
}sensor_setting_set_t;

typedef struct{
	u16 prop_id;
	u8 raw_val_x1;
	u8 raw_val_x2;
}sensor_series_get_t;

typedef struct{
	u16 prop_id;
	sensor_series_col_t series_raw;
}sensor_series_st_t;

typedef struct{
	u16 prop_id;
	u16 raw_val_X;
}sensor_col_get_t;

typedef struct{
	u16 prop_id;
	sensor_series_col_t raw_value;
}sensor_col_st_t;

//sensor sampling function
enum{
	UNSPECIFIED,
	INST,
	ARITHMETIC,
	RMS,
	MAX,
	MIN,
	ACCUMULATED,
	CNT,
};

enum{
	RFU_T,
	BOOLEAN_T,
	BIT2_T,
	NIBBLE_T,
	UINT8_T,
	UINT12_T,
	UINT16_T,
	UINT24_T,
	UINT32_T,
	UINT48_T,
	UINT64_T,
	UINT128_T,
};

enum{
	ID_UNKNOWN = 0xfffffffe,
	ID_PROHIBITED = 0xffffffff,
};

enum{
	PROHIBITED=0,
	READ=1,
	READ_WRITE=3,
};

void mesh_global_var_init_sensor_descrip();
int mesh_sensor_st_publish(u8 idx);
int mesh_sensor_setup_st_publish(u8 idx);
#if MD_SERVER_EN
int mesh_cmd_sig_sensor_descript_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_cadence_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_cadence_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_settings_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_setting_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_setting_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_column_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_series_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_sensor_descript_get                (0)
#define mesh_cmd_sig_sensor_get                         (0)
#define mesh_cmd_sig_sensor_cadence_get                 (0)
#define mesh_cmd_sig_sensor_cadence_set                 (0)
#define mesh_cmd_sig_sensor_settings_get                (0)
#define mesh_cmd_sig_sensor_setting_get                 (0)
#define mesh_cmd_sig_sensor_setting_set                 (0)
#define mesh_cmd_sig_sensor_column_get                  (0)
#define mesh_cmd_sig_sensor_series_get                  (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_sensor_descript_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_cadence_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_settings_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_setting_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_column_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sensor_series_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_sensor_descript_status             (0)
#define mesh_cmd_sig_sensor_status                      (0)
#define mesh_cmd_sig_sensor_cadence_status              (0)
#define mesh_cmd_sig_sensor_settings_status             (0)
#define mesh_cmd_sig_sensor_setting_status              (0)
#define mesh_cmd_sig_sensor_column_status               (0)
#define mesh_cmd_sig_sensor_series_status               (0)
#endif

u32 sensor_measure_proc();
#if SENSOR_LIGHTING_CTRL_EN
void sensor_lighting_ctrl_proc();
#endif

extern model_sensor_t			model_sig_sensor;
extern u32 sensure_measure_quantity;
