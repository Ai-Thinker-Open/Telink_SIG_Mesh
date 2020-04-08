/********************************************************************************************************
 * @file     lighting_model_xyl.c 
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
#include "proj/tl_common.h"
#ifndef WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "lighting_model_xyL.h"
#include "lighting_model.h"


#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
model_light_xyl_t       model_sig_light_xyl;

/*
	lighting model command callback function ----------------
*/

#if MD_SERVER_EN
// ----------- light XYL model-------------
void mesh_light_xyl_st_rsp_par_fill(mesh_cmd_light_xyl_st_t *rsp, u8 idx, u16 op_rsp)
{
    int target_flag = (LIGHT_XYL_TARGET_STATUS == op_rsp);
    rsp->lightness = get_u16_val_and_remain_time(idx, ST_TRANS_LIGHTNESS, &rsp->remain_t, target_flag);
    rsp->x = get_u16_val_and_remain_time(idx, ST_TRANS_XYL_X, &rsp->remain_t, target_flag);
    rsp->y = get_u16_val_and_remain_time(idx, ST_TRANS_XYL_Y, &rsp->remain_t, target_flag);
}

int mesh_tx_cmd_light_xyl_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_light_xyl_st_t rsp = {0};
	mesh_light_xyl_st_rsp_par_fill(&rsp, idx, op_rsp);
	
	u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 1;
	}

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_light_xyl_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_xyl.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_light_xyl_st(idx, ele_adr, pub_adr, LIGHT_XYL_STATUS, uuid, p_com_md);
}

int mesh_light_xyl_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_light_xyl_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, cb_par->op_rsp, 0, 0);
}

int mesh_cmd_sig_light_xyl_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_xyl_st_rsp(cb_par);
}

int light_x_set(mesh_cmd_light_x_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list)
{
	return level_u16_set((mesh_cmd_lightness_set_t *)p_set, par_len, op, idx, retransaction, ST_TRANS_XYL_X, pub_list);
}

int light_y_set(mesh_cmd_light_y_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list)
{
	return level_u16_set((mesh_cmd_lightness_set_t *)p_set, par_len, op, idx, retransaction, ST_TRANS_XYL_Y, pub_list);	
}

int mesh_cmd_sig_light_xyl_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	mesh_cmd_light_xyl_set_t *p_set = (mesh_cmd_light_xyl_set_t *)par;
	
    st_pub_list_t pub_list = {{0}};
	mesh_cmd_lightness_set_t lightness_set_tmp = {0};
	lightness_set_tmp.lightness = p_set->lightness;
	int len_tmp = (par_len >= sizeof(mesh_cmd_light_xyl_set_t)) ? sizeof(mesh_cmd_lightness_set_t) : 2;
	memcpy(&lightness_set_tmp.tid, &p_set->tid, 3);
	err = lightness_set(&lightness_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
	if(err){
		return err;
	}

	mesh_cmd_light_x_set_t *p_x_set_tmp = (mesh_cmd_light_x_set_t *)&lightness_set_tmp;
	p_x_set_tmp->x = p_set->x;
	err = light_x_set(p_x_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
	if(err){
		return err;
	}

	mesh_cmd_light_y_set_t *p_y_set_tmp = (mesh_cmd_light_y_set_t *)&lightness_set_tmp;
	p_y_set_tmp->y = p_set->y;
	err = light_y_set(p_y_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
	if(err){
		return err;
	}
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_xyl_st_rsp(cb_par);
	}else{
		VC_RefreshUI_level(cb_par);
	}
	
    model_pub_check_set_bind_all(&pub_list, cb_par,  0);

	return err;
}

int mesh_cmd_sig_light_xyl_target_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_xyl_st_rsp(cb_par);
}

// ----------- light xyl default-------------
int mesh_light_xyl_def_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    mesh_cmd_light_xyl_def_st_t rsp = {0};
    rsp.lightness = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_LIGHTNESS);
    rsp.x = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_XYL_X);
    rsp.y = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_XYL_Y);
    
    return mesh_tx_cmd_rsp(LIGHT_XYL_DEF_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_xyl_def_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_xyl_def_st_rsp(cb_par);
}

int mesh_cmd_sig_light_xyl_def_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	mesh_cmd_light_xyl_def_set_t *p_set = (mesh_cmd_light_xyl_def_set_t *)par;
	light_g_level_def_set_u16(p_set->lightness, cb_par->model_idx, ST_TRANS_LIGHTNESS);
	light_g_level_def_set_u16(p_set->x, cb_par->model_idx, ST_TRANS_XYL_X);
	light_g_level_def_set_u16(p_set->y, cb_par->model_idx, ST_TRANS_XYL_Y);

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_xyl_def_st_rsp(cb_par);
	}

	return err;
}

// ----------- light xyl range-------------
int mesh_xyl_range_st_rsp(u8 st, mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    mesh_cmd_light_xyl_range_st_t rsp = {0};
    rsp.st = st;
    light_g_level_range_get_u16((light_range_u16_t *)&rsp.x_min, cb_par->model_idx, ST_TRANS_XYL_X);
    light_g_level_range_get_u16((light_range_u16_t *)&rsp.y_min, cb_par->model_idx, ST_TRANS_XYL_Y);
    
    return mesh_tx_cmd_rsp(LIGHT_XYL_RANGE_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_xyl_range_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_xyl_range_st_rsp(RANGE_SET_SUCCESS, cb_par);
}

int mesh_cmd_sig_light_xyl_range_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	mesh_cmd_light_xyl_range_set_t *p_set = (mesh_cmd_light_xyl_range_set_t *)par;
	if(!(p_set->x_min <= p_set->x_max)
	|| !(p_set->y_min <= p_set->y_max)){
		return 0;
	}
	
	light_g_level_range_set(p_set->x_min, p_set->x_max, cb_par->model_idx, ST_TRANS_XYL_X);
	light_g_level_range_set(p_set->y_min, p_set->y_max, cb_par->model_idx, ST_TRANS_XYL_Y);

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_xyl_range_st_rsp(RANGE_SET_SUCCESS, cb_par);
	}

	return err;
}
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_light_xyl_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

int mesh_cmd_sig_light_xyl_target_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

// ----------- light xyl default-------------
int mesh_cmd_sig_light_xyl_def_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

// ----------- light xyl range-------------
int mesh_cmd_sig_light_xyl_range_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}
#endif

//--lighting model command interface-------------------
//-----------access command--------
// light XYL

//--lighting model command interface end----------------


#endif

