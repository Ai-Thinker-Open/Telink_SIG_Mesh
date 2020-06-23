/********************************************************************************************************
 * @file     lighting_model_LC.c 
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
#include "lighting_model_LC.h"
#include "lighting_model.h"


#if (MD_LIGHT_CONTROL_EN)
model_light_lc_t       model_sig_light_lc;
u32 mesh_md_light_lc_addr = FLASH_ADR_MD_LIGHT_LC;
#if MD_SERVER_EN

void light_LC_global_init()
{
    foreach_arr(i,model_sig_light_lc.prop){
        light_lc_property_t *p_prop = &model_sig_light_lc.prop[i];
        p_prop->id = 0;
        u8 val[sizeof(p_prop->val)] = {0x00,0x00,0x00};
        p_prop->len = LEN_LC_PROP_MAX;
        memcpy(p_prop->val, val, sizeof(p_prop->val));
    }
}

int is_light_lc_op(u16 op)
{
    return ((LIGHT_LC_ONOFF_SET == op)||(LIGHT_LC_ONOFF_SET_NOACK == op));
}

/*
	lighting model command callback function ----------------
*/	
int mesh_tx_cmd_lc_mode_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	u8 *par_rsp = &model_sig_light_lc.mode[idx];
	return mesh_tx_cmd_rsp(LIGHT_LC_MODE_STATUS, par_rsp, 1, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_lc_mode_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_lc.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_lc_mode_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

void scene_get_lc_par(scene_data_t *p_scene, int light_idx)
{
    p_scene->lc_mode = model_sig_light_lc.mode[light_idx];
    memcpy(&p_scene->lc_propty, &model_sig_light_lc.prop[light_idx], sizeof(p_scene->lc_propty));
}

void scene_load_lc_par(scene_data_t *p_scene, int light_idx)
{
    if(p_scene->lc_mode != model_sig_light_lc.mode[light_idx]){
        model_sig_light_lc.mode[light_idx] = p_scene->lc_mode;
        mesh_lc_mode_st_publish(light_idx);
    }

    light_lc_property_t *p_prop = &model_sig_light_lc.prop[light_idx];
    if(memcmp(&p_scene->lc_propty, p_prop, sizeof(p_scene->lc_propty))){
        memcpy(p_prop, &p_scene->lc_propty, sizeof(p_scene->lc_propty));
        mesh_lc_prop_st_publish(light_idx);
    }
}

int mesh_lc_mode_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_lc_mode_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_lc_mode_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_lc_mode_st_rsp(cb_par);
}

int mesh_cmd_sig_lc_mode_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
    u8 mode_new = par[0];
    u8 *p_mode = &model_sig_light_lc.mode[cb_par->model_idx];
    if( mode_new < LC_MODE_MAX){
    	if((*p_mode != mode_new) || PTS_TEST_EN){
			*p_mode = mode_new;
			mesh_model_store(1, SIG_MD_LIGHT_LC_S);
            model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 1);
            model_pub_st_cb_re_init_lc_srv(&mesh_lc_mode_st_publish);    // re-init
		}
	
		if(cb_par->op_rsp != STATUS_NONE){
			err = mesh_lc_mode_st_rsp(cb_par);
		}
    }
    return err;
}

int mesh_tx_cmd_lc_om_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	u8 *par_rsp = &model_sig_light_lc.om[idx];
	return mesh_tx_cmd_rsp(LIGHT_LC_OM_STATUS, par_rsp, 1, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_lc_om_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_lc.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_lc_om_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_lc_om_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_lc_om_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_lc_om_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_lc_om_st_rsp(cb_par);
}

int mesh_cmd_sig_lc_om_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
    u8 om_new = par[0];
    u8 *p_om = &model_sig_light_lc.om[cb_par->model_idx];
    if( om_new < LC_OM_MAX){
    	if(*p_om != om_new){
			*p_om = om_new;
			mesh_model_store(1, SIG_MD_LIGHT_LC_S);
            model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 1);
            model_pub_st_cb_re_init_lc_srv(&mesh_lc_om_st_publish);    // re-init
		}
        LC_property_tick_set(cb_par->model_idx);
	
		if(cb_par->op_rsp != STATUS_NONE){
			err = mesh_lc_om_st_rsp(cb_par);
		}
    }
    return err;
}

int mesh_tx_cmd_lc_prop_st(u8 idx, u16 id_get, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	light_lc_property_t *p_prop = &model_sig_light_lc.prop[idx];
	light_lc_property_t rsp_prop = {0};
	rsp_prop.id = id_get;
	u32 len_rsp = LEN_LC_PROP_MAX;
	if(p_prop->id == id_get){       // also response when not found,  MMDL-SR-LLCS-BV01
		memcpy(&rsp_prop.id, &p_prop->id, p_prop->len);
		len_rsp = p_prop->len;
	}

	return mesh_tx_cmd_rsp(LIGHT_LC_PROPERTY_STATUS, (u8 *)&rsp_prop.id, len_rsp, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_lc_prop_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_light_lc.setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_lc_prop_st(idx, model_sig_light_lc.prop[idx].id, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_cmd_sig_lc_prop_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u16 id_get = par[0]+(par[1]<<8);
	if(0 == id_get){
		return 0;
	}
	
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mesh_tx_cmd_lc_prop_st(cb_par->model_idx, id_get, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_lc_prop_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // MMDL-SR-LLC-BV06 / 07 will be set value 2bytes.
    // MMDL-SR-LLCS-BI01 require egnore 1bytes.
	if((par_len <= LEN_LC_PROP_MAX) && (par_len >= (2+2))){
		u16 id = par[0]+(par[1]<<8);
		if(0 == id){
			return 0;
		}
		
		light_lc_property_t *p_prop = &model_sig_light_lc.prop[cb_par->model_idx];
		memcpy(&p_prop->id, par, par_len);
		p_prop->len = par_len;
		
		mesh_model_store(1, SIG_MD_LIGHT_LC_S);
        model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 0);
		if(cb_par->op_rsp != STATUS_NONE){
			return mesh_cmd_sig_lc_prop_get(par, par_len, cb_par);
		}
	}
	return 0;
}

typedef struct{
    u32 tick;
}lc_property_proc_t;

lc_property_proc_t g_lc_prop_proc[ARRAY_SIZE(model_sig_light_lc.prop)];

void LC_property_tick_set(int idx)
{
    if(idx < ARRAY_SIZE(g_lc_prop_proc)){
        g_lc_prop_proc[idx].tick = clock_time() | 1;
    }
}

void LC_property_proc()
{
    foreach_arr(i,g_lc_prop_proc){
        light_lc_property_t *p_prop = &model_sig_light_lc.prop[i];
        lc_property_proc_t *p_proc = &g_lc_prop_proc[i];
        if(0x0030 == p_prop->id){   // Light LC Lightness Standby state: MMDL-SR-LLC-BV06 / BV07
            if(p_proc->tick && clock_time_exceed(p_proc->tick, 4000*1000)){ // comfirm later
                p_proc->tick = 0;
                #if (PTS_TEST_EN)
    			mesh_cmd_g_level_set_t level_set_tmp = {0};
    			int len_tmp = GET_LEVEL_PAR_LEN(0);
                u16 lightness = p_prop->val[0]+(p_prop->val[1]<<8);
    			level_set_tmp.level = u16_to_s16(lightness);
                st_pub_list_t pub_list = {{0}};
    			g_level_set_and_update_last((u8 *)&level_set_tmp, len_tmp, G_LEVEL_SET_NOACK, i, 0, ST_TRANS_LIGHTNESS, 0, &pub_list);
    			#endif
            }
        }
    }
}
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_lc_mode_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

int mesh_cmd_sig_lc_om_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

int mesh_cmd_sig_lc_prop_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
#endif

//---------- LC ONOFF use generic onoff--------
//---------- LC ONOFF end--------


//--lighting model command interface-------------------
//-----------access command--------
// light HSL

//--lighting model command interface end----------------


#endif

