/********************************************************************************************************
 * @file     scene.c 
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
#include "lighting_model.h"
#include "lighting_model_HSL.h"
#include "lighting_model_xyL.h"
#include "lighting_model_LC.h"
#include "scene.h"


#if (MD_SCENE_EN)
model_scene_t	model_sig_scene;
u32 mesh_md_scene_addr = FLASH_ADR_MD_SCENE;
#if MD_SERVER_EN

scene_proc_t	scene_proc[LIGHT_CNT];
u8 tansition_forced_by_recall_flag = 0;

void scene_active_set(int idx, u16 scene_id, int trans_flag)
{
	scene_proc_t *p_scene_proc = &scene_proc[idx];
	if(trans_flag){
		p_scene_proc->current_scene = 0;
		p_scene_proc->target_scene = scene_id;
	}else{
		p_scene_proc->current_scene = scene_id;
		p_scene_proc->target_scene = 0;
	}
}

void scene_target_complete_check(int idx)
{
	scene_proc_t *p_scene = &scene_proc[idx];
	if(p_scene->target_scene){
		p_scene->current_scene = p_scene->target_scene;
		p_scene->target_scene = 0;
	}
}

void scene_status_change_check(int idx, s16 present_level, int st_trans_type)
{
	scene_proc_t *p_scene = &scene_proc[idx];
	if((p_scene->current_scene) && (0 == p_scene->target_scene)){
		foreach(i,SCENE_CNT_MAX){
			scene_data_t *p = &model_sig_scene.data[idx][i];
			if(p->id == p_scene->current_scene){
				if(ST_TRANS_LIGHTNESS == st_trans_type){
					if(present_level != p->lightness_s16){
						p_scene->current_scene = 0; // clear
					}
				}
			}
		}
	}
}

/*
	model command callback function ----------------
*/	

int mesh_tx_cmd_scene_reg_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st)
{
	scene_reg_status_t rsp = {0};
	rsp.st = st;
	rsp.current_id = scene_proc[idx].current_scene;
	int cnt = 0;
	foreach(i,SCENE_CNT_MAX){
		scene_data_t *p = &model_sig_scene.data[idx][i];
		if(p->id){
			rsp.id[cnt++] = p->id;
		}
	}
	u32 par_len = OFFSETOF(scene_reg_status_t, id) + cnt*2;
	return mesh_tx_cmd_rsp(SCENE_REG_STATUS, (u8 *)&rsp, par_len, ele_adr, dst_adr, 0, 0);
}

#if 0	// no need publish for scene
int mesh_scene_reg_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_scene.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_scene_reg_st(idx, ele_adr, pub_adr, SCENE_ST_SUCCESS, uuid, p_com_md);
}
#endif

int mesh_scene_reg_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_scene_reg_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st);
}

int mesh_cmd_sig_scene_reg_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_scene_reg_st_rsp(cb_par, SCENE_ST_SUCCESS);
}

u8 mesh_cmd_sig_scene_set_ll(u16 scene_id, mesh_cb_fun_par_t *cb_par)
{
    u8 st = SCENE_ST_SUCCESS;
    u16 op = cb_par->op;
    int save = 0;
    if((SCENE_STORE == op)||(SCENE_STORE_NOACK == op)){
		scene_data_t *p_exist = 0;
		scene_data_t *p_null = 0;
		
		foreach(i,SCENE_CNT_MAX){
			scene_data_t *p = &model_sig_scene.data[cb_par->model_idx][i];
			if((0 == p_null) && (0 == p->id)){
				p_null = p; 	// the first empty
			}
			if(scene_id == p->id){
				p_exist = p;
				break;	// exist
			}
		}
		
		scene_data_t *p_save = 0;
		if(p_exist){
			p_save = p_exist;
		}else if(p_null){
			p_save = p_null;
		}
		
		if(p_save){
			save = 1;
			p_save->id = scene_id;
			p_save->lightness_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_LIGHTNESS);
			#if LIGHT_TYPE_CT_EN
			p_save->temp_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_CTL_TEMP);
			p_save->delta_uv_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_CTL_D_UV);
			p_save->ct_flag = ct_flag;
			#endif
			#if LIGHT_TYPE_HSL_EN
			p_save->hue_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_HSL_HUE);
			p_save->sat_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_HSL_SAT);
			#endif
			#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
			p_save->x_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_XYL_X);
			p_save->y_s16 = light_g_level_present_get(cb_par->model_idx, ST_TRANS_XYL_Y);
			#endif
			#if MD_LIGHT_CONTROL_EN
			scene_get_lc_par(p_save, cb_par->model_idx);
			#endif
			CB_NL_PAR_NUM_2(p_nl_get_vendor_scene_data, scene_id, p_save->nl_data);
			scene_active_set(cb_par->model_idx, scene_id, 0);
		}else{
			st = SCENE_ST_REG_FULL;
		}
    }else{	// del
		foreach(i,SCENE_CNT_MAX){
			scene_data_t *p = &model_sig_scene.data[cb_par->model_idx][i];
			if(scene_id == p->id){
				memset(p, 0, sizeof(scene_data_t));
				scene_proc_t *p_scene = &scene_proc[cb_par->model_idx];
				if(scene_id == p_scene->current_scene){
					p_scene->current_scene = 0;
				}
				
				if(scene_id == p_scene->target_scene){
					p_scene->target_scene = 0;
				}
				save = 1;
				break;	// exist
			}
		}
    }
    
    if(save){
		mesh_model_store(1, SIG_MD_SCENE_SETUP_S);
	}

    return st;
}

int mesh_cmd_sig_scene_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
    u16 scene_id = par[0] + (par[1]<<8);
    if(SCENE_ID_INVALID == scene_id){
    	return 0;
    }
    
	u8 st = mesh_cmd_sig_scene_set_ll(scene_id, cb_par);
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_scene_reg_st_rsp(cb_par, st);
	}
	
    return err;
}

//--
void mesh_scene_st_rsp_par_fill(scene_status_t *rsp, u8 idx)
{
	mesh_cmd_g_level_st_t level_st; 
	light_g_level_get((u8 *)&level_st, idx, ST_TRANS_LIGHTNESS);
	rsp->remain_t = level_st.remain_t;  // because ST_TRANS_LIGHTNESS is  forced to transmiting, when scene recall.
}

int mesh_tx_cmd_scene_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st, u8 *uuid, model_common_t *pub_md)
{
	scene_status_t rsp = {0};
	rsp.st = st;

	u32 len = sizeof(rsp) - 3;
	scene_proc_t *p_scene = &scene_proc[idx];
	if(p_scene->target_scene){
		mesh_scene_st_rsp_par_fill(&rsp, idx);
		if(rsp.remain_t){
			rsp.target_id = p_scene->target_scene;
			len = sizeof(rsp);
		}
	}
	rsp.current_id = p_scene->current_scene;
	return mesh_tx_cmd_rsp(SCENE_STATUS, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_scene_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_scene.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_scene_st(idx, ele_adr, pub_adr, SCENE_ST_SUCCESS, uuid, p_com_md);
}

int mesh_scene_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_scene_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st, 0, 0);
}

int mesh_cmd_sig_scene_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_scene_st_rsp(cb_par, SCENE_ST_SUCCESS);
}

int mesh_cmd_sig_scene_recall(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	u8 st = SCENE_ST_NOT_FOUND;
	scene_recall_t *p_recall = (scene_recall_t *)par;
	if(SCENE_ID_INVALID == p_recall->id){  
	    return -1;
	}

	if(!cb_par->retransaction){
        tansition_forced_by_recall_flag = 1;
    	foreach(i,SCENE_CNT_MAX){
    		scene_data_t *p = &model_sig_scene.data[cb_par->model_idx][i];
    		if(p_recall->id == p->id){
				CB_NL_PAR_NUM_2(p_nl_scene_server_state_recalled, p_recall->id, (u8 *)&model_sig_scene.data[cb_par->model_idx][i].nl_data);
    			st = SCENE_ST_SUCCESS;
    			mesh_cmd_g_level_set_t level_set_tmp = {0};

    			#if LIGHT_TYPE_CT_EN
                ct_flag = p->ct_flag;
                mesh_cmd_light_ctl_set_t ctl_set_tmp = {0};
                #endif
                #if LIGHT_TYPE_HSL_EN
                mesh_cmd_light_hue_set_t hue_set_tmp = {0};
                mesh_cmd_light_sat_set_t sat_set_tmp = {0};
                #endif
                #if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
                mesh_cmd_light_x_set_t x_set_tmp = {0};
                mesh_cmd_light_y_set_t y_set_tmp = {0};
                #endif
                int is_len_with_trans = (par_len >= sizeof(scene_recall_t));
                int trans_flag_scene = 0;
                
    			if(is_len_with_trans){
    			    trans_flag_scene = is_transition_need(p_recall->transit_t, p_recall->delay);
                    memcpy(&level_set_tmp.tid, &p_recall->tid, 3);
                    #if LIGHT_TYPE_CT_EN
                    memcpy(&ctl_set_tmp.tid, &p_recall->tid, 3);
                    #endif
                    #if LIGHT_TYPE_HSL_EN
                    memcpy(&hue_set_tmp.tid, &p_recall->tid, 3);
                    memcpy(&sat_set_tmp.tid, &p_recall->tid, 3);
                    #endif
                    #if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
                    memcpy(&x_set_tmp.tid, &p_recall->tid, 3);
                    memcpy(&y_set_tmp.tid, &p_recall->tid, 3);
                    #endif
    			}else{
        			trans_flag_scene = GET_TRANSITION_STEP(g_def_trans_time_val(cb_par->model_idx));
    			}
                int pub_flag_scene = trans_flag_scene ? ST_G_LEVEL_SET_PUB_TRANS : ST_G_LEVEL_SET_PUB_NOW;
    			scene_active_set(cb_par->model_idx, p_recall->id, trans_flag_scene);
            	if(pub_flag_scene){ // always TURE
            	    model_pub_check_set(pub_flag_scene, (u8 *)(&(model_sig_scene.srv[cb_par->model_idx])), 1);
            	}

    			// don't set status by send command to self, because every command will trigger all status publish,
    			level_set_tmp.level = p->lightness_s16;
    			int len_tmp = GET_LEVEL_PAR_LEN(is_len_with_trans);
                st_pub_list_t pub_list = {{0}};
                // all para in scene data have been check valid before, no need check again
    			g_level_set_and_update_last((u8 *)&level_set_tmp, len_tmp, G_LEVEL_SET_NOACK, cb_par->model_idx, 0, ST_TRANS_LIGHTNESS, 1, &pub_list); // force = 1: because PTS just recall but no state change,
    			#if LIGHT_TYPE_CT_EN
    			if(p->ct_flag){
    			    // Temp
    			    level_set_tmp.level = p->temp_s16;
                    g_level_set_and_update_last((u8 *)&level_set_tmp, len_tmp, G_LEVEL_SET_NOACK, cb_par->model_idx, 0, ST_TRANS_CTL_TEMP, 0, &pub_list);
                    // delta uv
                    level_set_tmp.level = p->delta_uv_s16;
                    err = g_level_set_and_update_last((u8 *)&level_set_tmp, len_tmp, G_LEVEL_SET_NOACK, cb_par->model_idx, 0, ST_TRANS_CTL_D_UV, 0, &pub_list);
                }else
                #endif
    			{
                    #if LIGHT_TYPE_HSL_EN
        			hue_set_tmp.hue = s16_to_u16(p->hue_s16);
        			sat_set_tmp.sat = s16_to_u16(p->sat_s16);
                    len_tmp = GET_PAR_LEN_BY_TRANS(sizeof(hue_set_tmp), is_len_with_trans);
                    light_hue_set(&hue_set_tmp, len_tmp, LIGHT_HSL_HUE_SET_NOACK, cb_par->model_idx, 0, &pub_list);
                    light_sat_set(&sat_set_tmp, len_tmp, LIGHT_HSL_SAT_SET_NOACK, cb_par->model_idx, 0, &pub_list);
                    #endif
                    #if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
        			x_set_tmp.x = s16_to_u16(p->x_s16);
        			y_set_tmp.y = s16_to_u16(p->y_s16);
                    len_tmp = GET_PAR_LEN_BY_TRANS(sizeof(x_set_tmp), is_len_with_trans);
                    light_x_set(&x_set_tmp, len_tmp, LIGHT_XYL_SET_NOACK, cb_par->model_idx, 0, &pub_list);
                    light_y_set(&y_set_tmp, len_tmp, LIGHT_XYL_SET_NOACK, cb_par->model_idx, 0, &pub_list);
                    #endif
                }
                #if MD_LIGHT_CONTROL_EN
                scene_load_lc_par(p, cb_par->model_idx);
                #endif
                
                mesh_publish_all_manual(&pub_list, SIG_MD_LIGHTNESS_S, 1);
    			break;	// exist
    		}
    	}
        tansition_forced_by_recall_flag = 0;
	}
    
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_scene_st_rsp(cb_par, st);
	}
	
    return err;
}
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_scene_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
int mesh_cmd_sig_scene_reg_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
#endif


//--model command interface-------------------
//-----------access command--------
int access_cmd_scene_store(u16 adr_dst, u32 rsp_max, u16 id, int ack)
{
	return SendOpParaDebug(adr_dst, rsp_max, ack ? SCENE_STORE : SCENE_STORE_NOACK,(u8 *)&id, sizeof(id));
}

int access_cmd_scene_recall(u16 adr_dst, u32 rsp_max, u16 id, int ack, transition_par_t *trs_par)
{
	scene_recall_t par = {0};
	u32 par_len = OFFSETOF(scene_recall_t,transit_t);
	par.id = id;
	if(trs_par){
		par_len = sizeof(scene_recall_t);
		memcpy(&par.transit_t, trs_par, 2);
	}

	return SendOpParaDebug(adr_dst, rsp_max, ack ? SCENE_RECALL : SCENE_RECALL_NOACK,(u8 *)&par, par_len);
}

int access_cmd_scene_get(u16 adr, u32 rsp_max)
{
	return SendOpParaDebug(adr, rsp_max, SCENE_GET, 0, 0);
}

int access_cmd_scene_reg_get(u16 adr, u32 rsp_max)
{
	return SendOpParaDebug(adr, rsp_max, SCENE_REG_GET, 0, 0);
}

int access_cmd_scene_del(u16 adr_dst, u32 rsp_max, u16 id, int ack)
{
	return SendOpParaDebug(adr_dst, rsp_max, ack ? SCENE_DEL : SCENE_DEL_NOACK,(u8 *)&id, sizeof(id));
}

//--model command interface end----------------


#endif

