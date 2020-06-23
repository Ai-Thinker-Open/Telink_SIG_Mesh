/********************************************************************************************************
 * @file     lighting_model.h 
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


/** @addtogroup Mesh_Common
  * @{
  */

/** @defgroup Lighting_Model
  * @brief Lighting Models Code.
  * @{
  */


//----------------------------------- op code
// op cmd 0xxxxxxx (SIG)

// op cmd 10xxxxxx xxxxxxxx (SIG)
#define LIGHTNESS_GET		        	0x4B82
#define LIGHTNESS_SET		        	0x4C82
#define LIGHTNESS_SET_NOACK				0x4D82
#define LIGHTNESS_STATUS		    	0x4E82
#define LIGHTNESS_LINEAR_GET		    0x4F82
#define LIGHTNESS_LINEAR_SET		    0x5082
#define LIGHTNESS_LINEAR_SET_NOACK		0x5182
#define LIGHTNESS_LINEAR_STATUS		    0x5282
#define LIGHTNESS_LAST_GET		    	0x5382
#define LIGHTNESS_LAST_STATUS		    0x5482
#define LIGHTNESS_DEFULT_GET		    0x5582
#define LIGHTNESS_DEFULT_STATUS		    0x5682
#define LIGHTNESS_RANGE_GET		    	0x5782
#define LIGHTNESS_RANGE_STATUS		    0x5882
#define LIGHTNESS_DEFULT_SET		    0x5982
#define LIGHTNESS_DEFULT_SET_NOACK		0x5A82
#define LIGHTNESS_RANGE_SET		    	0x5B82
#define LIGHTNESS_RANGE_SET_NOACK		0x5C82
#define LIGHT_CTL_GET		        	0x5D82
#define LIGHT_CTL_SET		        	0x5E82
#define LIGHT_CTL_SET_NOACK				0x5F82
#define LIGHT_CTL_STATUS		    	0x6082
#define LIGHT_CTL_TEMP_GET		        0x6182
#define LIGHT_CTL_TEMP_RANGE_GET		0x6282
#define LIGHT_CTL_TEMP_RANGE_STATUS		0x6382
#define LIGHT_CTL_TEMP_SET				0x6482
#define LIGHT_CTL_TEMP_SET_NOACK		0x6582
#define LIGHT_CTL_TEMP_STATUS			0x6682
#define LIGHT_CTL_DEFULT_GET			0x6782
#define LIGHT_CTL_DEFULT_STATUS		    0x6882
#define LIGHT_CTL_DEFULT_SET			0x6982
#define LIGHT_CTL_DEFULT_SET_NOACK		0x6A82
#define LIGHT_CTL_TEMP_RANGE_SET		0x6B82
#define LIGHT_CTL_TEMP_RANGE_SET_NOACK	0x6C82


//------op parameters

//------------------vendor op end-------------------

enum{
	ONPOWER_UP_OFF		= 0,
	ONPOWER_UP_DEFAULT,     // always on
	ONPOWER_UP_STORE,
	ONPOWER_UP_MAX,
};

#define ONPOWER_UP_VAL(index)	model_sig_g_power_onoff.on_powerup[index]

typedef struct{
	u16 lightness;
	u8 tid;
	u8 transit_t;
	u8 delay;		// unit 5ms
}mesh_cmd_lightness_set_t;

typedef struct{
	u16 present;
	u16 target;
	u8 remain_t;
}mesh_cmd_lightness_st_t;

typedef struct{
	u16 lightness;
}mesh_cmd_lightness_last_st_t;

typedef struct{
	u16 lightness;
}mesh_cmd_lightness_def_set_t;

typedef struct{
	u16 lightness;
}mesh_cmd_lightness_def_st_t;

typedef struct{
	u16 min;
	u16 max;
}mesh_cmd_lightness_range_set_t;

typedef struct{
    u8 status;
	u16 min;
	u16 max;
}mesh_cmd_lightness_range_st_t;

typedef struct{
	u16 min;
	u16 max;
}mesh_cmd_light_ctl_temp_range_set_t;

typedef struct{
    u8 status;
	u16 min;
	u16 max;
}mesh_cmd_light_ctl_temp_range_st_t;

typedef struct{
	u16 lightness;		// lightness ctl is same with lightness
	u16 temp;			// 0x0320 -- 0x4E20  :800--20000
	s16 delta_uv;
	u8 tid;
	u8 transit_t;
	u8 delay;		// unit 5ms
}mesh_cmd_light_ctl_set_t;

typedef struct{
	u16 present_lightness;
	u16 present_temp;
	u16 target_lightness;
	u16 target_temp;
	u8 remain_t;
}mesh_cmd_light_ctl_st_t;

typedef struct{
	u16 lightness;
	u16 temp;
	s16 delta_uv;
}mesh_cmd_light_ctl_def_set_t;

typedef struct{
	u16 lightness;
	u16 temp;
	s16 delta_uv;
}mesh_cmd_light_ctl_def_st_t;

typedef struct{
	u16 temp;			// 0x0320 -- 0x4E20  :800--20000
	s16 delta_uv;
	u8 tid;
	u8 transit_t;
	u8 delay;		// unit 5ms
}mesh_cmd_light_ctl_temp_set_t;

typedef struct{
	u16 present_temp;
	s16 present_delta_uv;
	u16 target_temp;
	s16 target_delta_uv;
	u8 remain_t;
}mesh_cmd_light_ctl_temp_st_t;

#if WIN32
void VC_RefreshUI_level(mesh_cb_fun_par_t *cb_par);
#else
#define VC_RefreshUI_level(cb_par)		
#endif

#define mesh_publish_all_manual(pub_list, md_id_priority, sig_flag)   do{\
    mesh_op_resource_t res = {0};\
    res.id = md_id_priority;\
    res.sig = sig_flag;\
    mesh_cb_fun_par_t cb_par = {0};\
    cb_par.res = (u8 *)&res;\
    model_pub_check_set_bind_all(pub_list, &cb_par, 0);\
}while(0)

u16 lightness_to_linear(u16 val);
u16 linear_to_lightness(u16 val);
int is_valid_lum(u8 lum);

int g_level_set(u8 *par, int par_len, u16 op, int idx, u8 retransaction, int st_trans_type, int force, st_pub_list_t *pub_list);
int g_level_set_and_update_last(u8 *par, int par_len, u16 op, int idx, u8 retransaction, int st_trans_type, int force, st_pub_list_t *pub_list);
s16 get_val_with_check_range(s32 level_target, s16 min, s16 max, int st_trans_type);
void mesh_g_level_st_rsp_par_fill(mesh_cmd_g_level_st_t *rsp, u8 idx);
int is_valid_transition_step(u8 transit_t);
void model_pub_check_set_bind_all(st_pub_list_t *pub_flag, mesh_cb_fun_par_t *cb_par, int linear);
int light_ctl_temp_set(mesh_cmd_light_ctl_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list);
s16 get_level_from_ctl_temp_light_idx(u16 temp, int light_idx);

int mesh_lightness_st_publish_ll(u8 idx, u16 op_rsp);
int mesh_lightness_st_publish(u8 idx);
int mesh_lightness_st_rsp(mesh_cb_fun_par_t *cb_par);
int mesh_light_ctl_st_publish(u8 idx);
int mesh_light_ctl_st_rsp(mesh_cb_fun_par_t *cb_par);

#if MD_SERVER_EN
int mesh_cmd_sig_lightness_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_g_level_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_last_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_linear_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_linear_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_def_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_def_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_range_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_range_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_def_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_def_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_range_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_range_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_lightness_get              (0)
#define mesh_cmd_sig_lightness_set              (0)
#define mesh_cmd_sig_g_level_set                (0)
#define mesh_cmd_sig_lightness_last_get         (0)
#define mesh_cmd_sig_lightness_linear_get       (0)
#define mesh_cmd_sig_lightness_linear_set       (0)
#define mesh_cmd_sig_lightness_def_get          (0)
#define mesh_cmd_sig_lightness_def_set          (0)
#define mesh_cmd_sig_lightness_range_get        (0)
#define mesh_cmd_sig_lightness_range_set        (0)
#define mesh_cmd_sig_light_ctl_get              (0)
#define mesh_cmd_sig_light_ctl_set              (0)
#define mesh_cmd_sig_light_ctl_def_get          (0)
#define mesh_cmd_sig_light_ctl_def_set          (0)
#define mesh_cmd_sig_light_ctl_temp_range_get   (0)
#define mesh_cmd_sig_light_ctl_temp_range_set   (0)
#define mesh_cmd_sig_light_ctl_temp_get         (0)
#define mesh_cmd_sig_light_ctl_temp_set         (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_lightness_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_last_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_linear_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_def_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lightness_range_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_def_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_range_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_light_ctl_temp_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_lightness_status           (0)
#define mesh_cmd_sig_lightness_last_status      (0)
#define mesh_cmd_sig_lightness_linear_status    (0)
#define mesh_cmd_sig_lightness_def_status       (0)
#define mesh_cmd_sig_lightness_range_status     (0)
#define mesh_cmd_sig_light_ctl_status           (0)
#define mesh_cmd_sig_light_ctl_def_status       (0)
#define mesh_cmd_sig_light_ctl_temp_range_status    (0)
#define mesh_cmd_sig_light_ctl_temp_status      (0)
#endif

int lightness_set(mesh_cmd_lightness_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list);
int mesh_light_ctl_temp_st_publish(u8 idx);
int mesh_tx_cmd_lightness_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md);

int level_s16_set(mesh_cmd_g_level_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, int st_trans_type, st_pub_list_t *pub_list);
int level_u16_set(mesh_cmd_lightness_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, int st_trans_type, st_pub_list_t *pub_list);
u16 get_u16_val_and_remain_time(u8 idx, int st_trans_type, u8 *p_remain_t, int target_flag);
void mesh_level_u16_st_rsp_par_fill(mesh_cmd_lightness_st_t *rsp, u8 idx, int st_trans_type);
int mesh_level_def_u16_st_rsp(mesh_cb_fun_par_t *cb_par, int st_trans_type);
int mesh_range_st_rsp(u8 st, mesh_cb_fun_par_t *cb_par, int st_trans_type);


// access_cmd
int access_cmd_get_lightness(u16 adr,u32 rsp_max);
int access_cmd_set_lightness(u16 adr, u8 rsp_max, u16 lightness, int ack, transition_par_t *trs_par);

/**
  * @}
  */

/**
  * @}
  */


