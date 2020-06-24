/********************************************************************************************************
 * @file     lighting_model_HSL.c 
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
#include "lighting_model_HSL.h"
#include "lighting_model.h"


#if (LIGHT_TYPE_HSL_EN)
model_light_hsl_t       model_sig_light_hsl;
u32 mesh_md_light_hsl_addr = FLASH_ADR_MD_LIGHT_HSL;

#if MD_SERVER_EN
/*
	lighting model command callback function ----------------
*/

// ----------- light HSL model-------------
void mesh_light_hsl_st_rsp_par_fill(mesh_cmd_light_hsl_st_t *rsp, u8 idx, u16 op_rsp)
{
    int target_flag = (LIGHT_HSL_TARGET_STATUS == op_rsp);
    rsp->lightness = get_u16_val_and_remain_time(idx, ST_TRANS_LIGHTNESS, &rsp->remain_t, target_flag);
    rsp->hue = get_u16_val_and_remain_time(idx, ST_TRANS_HSL_HUE, &rsp->remain_t, target_flag);
    rsp->sat = get_u16_val_and_remain_time(idx, ST_TRANS_HSL_SAT, &rsp->remain_t, target_flag);
}

int mesh_tx_cmd_light_hsl_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md)
{
	return 0;
	mesh_cmd_light_hsl_st_t rsp = {0};
	mesh_light_hsl_st_rsp_par_fill(&rsp, idx, op_rsp);
	
	u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 1;
	}

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_light_hsl_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_hsl.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_light_hsl_st(idx, ele_adr, pub_adr, LIGHT_HSL_STATUS, uuid, p_com_md);
}

int mesh_light_hsl_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_light_hsl_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, cb_par->op_rsp, 0, 0);
}

int mesh_cmd_sig_light_hsl_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_hsl_st_rsp(cb_par);
}

int light_hue_set(mesh_cmd_light_hue_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list)
{
	//if(is_valid_hue(p_set->hue)){
		return level_u16_set((mesh_cmd_lightness_set_t *)p_set, par_len, op, idx, retransaction, ST_TRANS_HSL_HUE, pub_list);
	//}
	
	return 0;
}

int light_sat_set(mesh_cmd_light_sat_set_t *p_set, int par_len, u16 op, int idx, u8 retransaction, st_pub_list_t *pub_list)
{
	//if(is_valid_sat(p_set->sat)){
		return level_u16_set((mesh_cmd_lightness_set_t *)p_set, par_len, op, idx, retransaction, ST_TRANS_HSL_SAT, pub_list);
	//}
	
	return 0;
}

int mesh_cmd_sig_light_hsl_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	set_ct_mode(0);
	int err = 0;
	mesh_cmd_light_hsl_set_t *p_set = (mesh_cmd_light_hsl_set_t *)par;
	
    st_pub_list_t pub_list = {{0}};
	mesh_cmd_lightness_set_t lightness_set_tmp = {0};
	lightness_set_tmp.lightness = p_set->lightness;
	int len_tmp = (par_len >= sizeof(mesh_cmd_light_hsl_set_t)) ? sizeof(mesh_cmd_lightness_set_t) : 2;
	memcpy(&lightness_set_tmp.tid, &p_set->tid, 3);
	err = lightness_set(&lightness_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
    if(err){
        return err;
    }

	mesh_cmd_light_hue_set_t *p_hue_set_tmp = (mesh_cmd_light_hue_set_t *)&lightness_set_tmp;
	p_hue_set_tmp->hue = p_set->hue;
	err = light_hue_set(p_hue_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
    if(err){
        return err;
    }

	mesh_cmd_light_sat_set_t *p_sat_set_tmp = (mesh_cmd_light_sat_set_t *)&lightness_set_tmp;
	p_sat_set_tmp->sat = p_set->sat;
	err = light_sat_set(p_sat_set_tmp, len_tmp, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
    if(err){
        return err;
    }
    
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_hsl_st_rsp(cb_par);
	}else{
		VC_RefreshUI_level(cb_par);
	}
	
    model_pub_check_set_bind_all(&pub_list, cb_par, 0);

	return err;
}

int mesh_cmd_sig_light_hsl_target_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_hsl_st_rsp(cb_par);
}

// ----------- light hsl default-------------
int mesh_light_hsl_def_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    mesh_cmd_light_hsl_def_st_t rsp = {0};
    rsp.lightness = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_LIGHTNESS);
    rsp.hue = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_HSL_HUE);
    rsp.sat = light_g_level_def_get_u16(cb_par->model_idx, ST_TRANS_HSL_SAT);
    
    return mesh_tx_cmd_rsp(LIGHT_HSL_DEF_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_hsl_def_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_hsl_def_st_rsp(cb_par);
}

int mesh_cmd_sig_light_hsl_def_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	mesh_cmd_light_hsl_def_set_t *p_set = (mesh_cmd_light_hsl_def_set_t *)par;
	light_g_level_def_set_u16(p_set->lightness, cb_par->model_idx, ST_TRANS_LIGHTNESS);
	light_g_level_def_set_u16(p_set->hue, cb_par->model_idx, ST_TRANS_HSL_HUE);
	light_g_level_def_set_u16(p_set->sat, cb_par->model_idx, ST_TRANS_HSL_SAT);

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_hsl_def_st_rsp(cb_par);
	}

	return err;
}

// ----------- light hsl range-------------
int mesh_hsl_range_st_rsp(u8 st, mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    mesh_cmd_light_hsl_range_st_t rsp = {0};
    rsp.st = st;
    light_g_level_range_get_u16((light_range_u16_t *)&rsp.hue_min, cb_par->model_idx, ST_TRANS_HSL_HUE);
    light_g_level_range_get_u16((light_range_u16_t *)&rsp.sat_min, cb_par->model_idx, ST_TRANS_HSL_SAT);
    
    return mesh_tx_cmd_rsp(LIGHT_HSL_RANGE_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_hsl_range_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_hsl_range_st_rsp(RANGE_SET_SUCCESS, cb_par);
}

int mesh_cmd_sig_light_hsl_range_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	mesh_cmd_light_hsl_range_set_t *p_set = (mesh_cmd_light_hsl_range_set_t *)par;
	if(!(p_set->hue_min <= p_set->hue_max)
	|| !(p_set->sat_min <= p_set->sat_max)){
		return 0;
	}
	
	light_g_level_range_set(p_set->hue_min, p_set->hue_max, cb_par->model_idx, ST_TRANS_HSL_HUE);
	light_g_level_range_set(p_set->sat_min, p_set->sat_max, cb_par->model_idx, ST_TRANS_HSL_SAT);

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_hsl_range_st_rsp(RANGE_SET_SUCCESS, cb_par);
	}

	return err;
}

//----HUE
void mesh_light_hue_st_rsp_par_fill(mesh_cmd_light_hue_st_t *rsp, u8 idx)
{
	mesh_cmd_g_level_st_t level_st; 
	light_g_level_get((u8 *)&level_st, idx, ST_TRANS_HSL_HUE);
	rsp->present = get_lightness_from_level(level_st.present_level);
	rsp->target = get_lightness_from_level(level_st.target_level);
	rsp->remain_t = level_st.remain_t;
}

int mesh_tx_cmd_light_hue_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_light_hue_st_t rsp = {0};
	mesh_light_hue_st_rsp_par_fill(&rsp, idx);
	
	u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 3;
	}

	return mesh_tx_cmd_rsp(LIGHT_HSL_HUE_STATUS, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_light_hue_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_hsl.hue[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_light_hue_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_light_hue_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_light_hue_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_hue_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_hue_st_rsp(cb_par);
}

int mesh_cmd_sig_light_hue_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	set_ct_mode(0);
	int err = 0;
	mesh_cmd_light_hue_set_t *p_set = (mesh_cmd_light_hue_set_t *)par;
    st_pub_list_t pub_list = {{0}};
	err = light_hue_set(p_set, par_len, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
	if(err){
	    return err;
	}
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_hue_st_rsp(cb_par);
	}else{
		VC_RefreshUI_level(cb_par);
	}
	
	model_pub_check_set_bind_all(&pub_list, cb_par, 0);

	return err;
}

//----SAT
void mesh_light_sat_st_rsp_par_fill(mesh_cmd_light_sat_st_t *rsp, u8 idx)
{
	mesh_cmd_g_level_st_t level_st; 
	light_g_level_get((u8 *)&level_st, idx, ST_TRANS_HSL_SAT);
	rsp->present = get_lightness_from_level(level_st.present_level);
	rsp->target = get_lightness_from_level(level_st.target_level);
	rsp->remain_t = level_st.remain_t;
}

int mesh_tx_cmd_light_sat_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_light_sat_st_t rsp = {0};
	mesh_light_sat_st_rsp_par_fill(&rsp, idx);
	
	u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 3;
	}

	return mesh_tx_cmd_rsp(LIGHT_HSL_SAT_STATUS, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_light_sat_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_hsl.sat[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_light_sat_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_light_sat_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_light_sat_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_light_sat_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_light_sat_st_rsp(cb_par);
}

int mesh_cmd_sig_light_sat_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	set_ct_mode(0);
	int err = 0;
	mesh_cmd_light_sat_set_t *p_set = (mesh_cmd_light_sat_set_t *)par;
    st_pub_list_t pub_list = {{0}};
	err = light_sat_set(p_set, par_len, cb_par->op, cb_par->model_idx, cb_par->retransaction, &pub_list);
    if(err){
        return err;
    }
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_light_sat_st_rsp(cb_par);
	}else{
		VC_RefreshUI_level(cb_par);
	}
	
	model_pub_check_set_bind_all(&pub_list, cb_par, 0);

	return err;
}

#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_light_hsl_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

int mesh_cmd_sig_light_hsl_target_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

// ----------- light hsl default-------------
int mesh_cmd_sig_light_hsl_def_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

// ----------- light hsl range-------------
int mesh_cmd_sig_light_hsl_range_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

//----HUE
int mesh_cmd_sig_light_hue_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

//----SAT
int mesh_cmd_sig_light_sat_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
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
// light HSL
int access_cmd_get_light_hsl(u16 adr,u32 rsp_max)
{
	u8 par[1];
	return SendOpParaDebug(adr, rsp_max, LIGHT_HSL_GET, par, 0);
}

int access_cmd_set_light_hsl(u16 adr, u8 rsp_max, u16 lightness, u16 hue, u16 sat, int ack, transition_par_t *trs_par)
{
	mesh_cmd_light_hsl_set_t par = {0};
	u32 par_len = OFFSETOF(mesh_cmd_light_hsl_set_t,transit_t);
	par.lightness = lightness;
	par.hue = hue;
	par.sat = sat;
	if(trs_par){
		par_len = sizeof(mesh_cmd_light_hsl_set_t);
		memcpy(&par.transit_t, trs_par, 2);
	}
	
	return SendOpParaDebug(adr, rsp_max, ack ? LIGHT_HSL_SET : LIGHT_HSL_SET_NOACK, 
						   (u8 *)&par, par_len);
}

int access_cmd_set_light_hsl_100(u16 adr, u8 rsp_max, u8 lum, u8 hue_100, u8 sat_100, int ack, transition_par_t *trs_par)
{
	if((!is_valid_lum(lum)) || (!is_valid_val_100(hue_100)) || (!is_valid_val_100(sat_100))){
		return -2;
	}

	u16 lightness = lum2_lightness(lum);
	u16 hue = lum2_lightness(hue_100);
	u16 sat = lum2_lightness(sat_100);
	return access_cmd_set_light_hsl(adr, rsp_max, lightness, hue, sat, ack, trs_par);
}

int access_cmd_get_light_hsl_hue(u16 adr,u32 rsp_max)
{
	u8 par[1];
	return SendOpParaDebug(adr, rsp_max, LIGHT_HSL_HUE_GET, par, 0);
}

int access_cmd_get_light_hsl_sat(u16 adr,u32 rsp_max)
{
	u8 par[1];
	return SendOpParaDebug(adr, rsp_max, LIGHT_HSL_SAT_GET, par, 0);
}

int access_cmd_set_light_hsl_hue_sat(u16 adr, u8 rsp_max, u16 hue_sat, int ack, transition_par_t *trs_par, int hue_flag)
{
	mesh_cmd_light_hue_set_t par = {0};
	u32 par_len = OFFSETOF(mesh_cmd_light_hue_set_t,transit_t);
	par.hue = hue_sat;
	if(trs_par){
		par_len = sizeof(mesh_cmd_light_hue_set_t);
		memcpy(&par.transit_t, trs_par, 2);
	}

	u16 op;
	if(hue_flag){
        op = ack ? LIGHT_HSL_HUE_SET : LIGHT_HSL_HUE_SET_NOACK;
	}else{
        op = ack ? LIGHT_HSL_SAT_SET : LIGHT_HSL_SAT_SET_NOACK;
	}
	
	return SendOpParaDebug(adr, rsp_max, op, (u8 *)&par, par_len);
}

int access_cmd_set_light_hsl_hue_sat_100(u16 adr, u8 rsp_max, u8 hue_sat_100, int ack, transition_par_t *trs_par, int hue_flag)
{
	if(!is_valid_val_100(hue_sat_100)){
		return -2;
	}

	u16 hue = lum2_lightness(hue_sat_100);
	return access_cmd_set_light_hsl_hue_sat(adr, rsp_max, hue, ack, trs_par, hue_flag);
}

int access_cmd_set_light_hsl_hue_100(u16 adr, u8 rsp_max, u8 hue_100, int ack, transition_par_t *trs_par)
{
	return access_cmd_set_light_hsl_hue_sat_100(adr, rsp_max, hue_100, ack, trs_par, 1);
}

int access_cmd_set_light_hsl_sat_100(u16 adr, u8 rsp_max, u8 sat_100, int ack, transition_par_t *trs_par)
{
	return access_cmd_set_light_hsl_hue_sat_100(adr, rsp_max, sat_100, ack, trs_par, 0);
}

//--lighting model command interface end----------------


#endif

