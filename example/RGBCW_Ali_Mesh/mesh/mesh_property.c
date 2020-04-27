/********************************************************************************************************
 * @file     mesh_property.c 
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
#include "mesh_property.h"

#if (MD_PROPERTY_EN)
model_property_t	model_sig_property;
u32 mesh_md_property_addr = FLASH_ADR_MD_PROPERTY;
#if MD_SERVER_EN

#define PROP_ID_TEST_PTS    (0x004E)        // comfirm later
#define PROP_HEAD_LEN       (OFFSETOF(mesh_property_t,val) - 1)

STATIC_ASSERT(((MD_SENSOR_EN == 0) && (MD_BATTERY_EN == 0) && (MD_LOCATION_EN == 0)) || (FLASH_ADR_MD_SENSOR != FLASH_ADR_MD_PROPERTY));   // because use same flash sector to save

void mesh_property_global_init()
{
    foreach(i,MESH_PROPERTY_CNT_MAX){
        mesh_property_t *p_prop = &model_sig_property.user_prop[i];
        p_prop->id = PROP_ID_TEST_PTS; // CHARACTERISTIC_UUID_MANU_NAME_STRING;
        p_prop->access = MESH_ACCESS_RW;
        u8 str[] = {0x84,0xEF,0x48};
        u32 len_val = sizeof(str);
        if(len_val <= sizeof(p_prop->val)){
            memcpy(p_prop->val, str, len_val);
            p_prop->len_val = len_val;
        }
    }
    
    foreach(i,MESH_PROPERTY_CNT_MAX){
        mesh_property_t *p_prop = &model_sig_property.client_prop[i];
        p_prop->id = PROP_ID_TEST_PTS; // CHARACTERISTIC_UUID_MANU_NAME_STRING;
        p_prop->access = MESH_ACCESS_READ;
        u8 str[] = {0x11,0x22,0x33};
        u32 len_val = sizeof(str);
        if(len_val <= sizeof(p_prop->val)){
            memcpy(p_prop->val, str, len_val);
            p_prop->len_val = len_val;
        }
    }
}


//-- properties --
STATIC_ASSERT(MESH_PROPERTY_CNT_MAX <= 8);  // because ids in mesh_tx_cmd_properties_st_

static inline int is_valid_prop_id(u16 prop_id)
{
    return (INVALID_PROP_ID != prop_id);
}

mesh_property_t * get_property_base(u16 op_rsp, u32 *p_max_out)
{
    mesh_property_t *p_prop = 0;
    *p_max_out = 0;
    if(G_CLIENT_PROPERTIES_STATUS == op_rsp){
        p_prop = model_sig_property.client_prop;
        *p_max_out = ARRAY_SIZE(model_sig_property.client_prop);
    }else{
        p_prop = model_sig_property.user_prop;
        *p_max_out = ARRAY_SIZE(model_sig_property.user_prop);
    }
    
    if(p_prop){
        return p_prop;
    }

    return 0;
}

int is_prop_readable(u16 op_rsp, u8 access)
{
    return (((G_USER_PROPERTY_STATUS != op_rsp) && (G_USER_PROPERTIES_STATUS != op_rsp))
          || ((MESH_ACCESS_READ == access)||(MESH_ACCESS_RW == access)));
}

int is_prop_writable(u16 op_rsp, u8 access)
{
    return ((G_USER_PROPERTY_STATUS != op_rsp) 
          || ((MESH_ACCESS_WRITE == access)||(MESH_ACCESS_RW == access)));
}

static inline int mesh_tx_cmd_properties_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 start_id)
{
    u16 ids[8] = {0};
    int cnt = 0;
    u32 max = 0;
    mesh_property_t *p_prop_base = get_property_base(op_rsp, &max);    
    if(max > ARRAY_SIZE(ids)){
        max = ARRAY_SIZE(ids);
    }
    
    if(p_prop_base){
        foreach(i,max){
            mesh_property_t *p_prop = &p_prop_base[i];
            if(is_valid_prop_id(p_prop->id) && (p_prop->id >= start_id)){
                if(is_prop_readable(op_rsp, p_prop->access)){
                    ids[cnt++] = p_prop->id;
                }
            }
        }
    }
    
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&ids, cnt*2, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_cmd_sig_properties_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    u16 start_id = 0;
    if(G_CLIENT_PROPERTIES_GET == cb_par->op){
        start_id = par[0] + (par[1]<<8);
    }
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_properties_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, cb_par->op_rsp, start_id);
}

// -- property --
mesh_property_t * get_property_by_id(u16 prop_id, u16 op_rsp)
{
    if(!is_valid_prop_id(prop_id)){
        return 0;
    }
    
    u32 max = 0;
    mesh_property_t *p_prop_base = get_property_base(op_rsp, &max);
    if(p_prop_base){
        foreach(i,max){
            if(p_prop_base[i].id == prop_id){
                return &p_prop_base[i];
            }
        }
    }

    return 0;
}

int mesh_tx_cmd_property_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 prop_id, u16 op_rsp, int set_flag)
{
    if(!is_valid_prop_id(prop_id)){
        return -1;
    }
    
    u8 *p_rsp = (u8 *)&prop_id;
    int len = 2;
    mesh_property_t *p_prop = get_property_by_id(prop_id, op_rsp);
    if(p_prop && (!(set_flag && !is_prop_writable(op_rsp, p_prop->access)))){
        if(!is_prop_readable(op_rsp, p_prop->access)){
            return -1;
        }
        p_rsp = (u8 *)&p_prop->id;
        len = PROP_HEAD_LEN + p_prop->len_val;
    }
    
	return mesh_tx_cmd_rsp(op_rsp, p_rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_property_st_publish_ll(u8 idx, u16 op_rsp)
{
    model_common_t *p_com_md = 0;
    if(G_USER_PROPERTY_STATUS == op_rsp){
        p_com_md = &model_sig_property.user_srv[idx].com;
    }else if(G_ADMIN_PROPERTY_STATUS == op_rsp){
        p_com_md = &model_sig_property.admin_srv[idx].com;
    }else if(G_MFG_PROPERTY_STATUS == op_rsp){
        p_com_md = &model_sig_property.mfg_srv[idx].com;
    }

    if(p_com_md){
    	u16 ele_adr = p_com_md->ele_adr;
    	u16 pub_adr = p_com_md->pub_adr;
    	if(!pub_adr){
    		return -1;
    	}
    	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
    	
    	u32 max = 0;
        mesh_property_t *p_prop = get_property_base(op_rsp, &max);
        u16 prop_id = p_prop[0].id; // comfirm later
    	return mesh_tx_cmd_property_st(idx, ele_adr, pub_adr, uuid, p_com_md, prop_id, op_rsp, 0);
	}
	
	return -1;
}

int mesh_property_st_publish_user(u8 idx)
{
	return mesh_property_st_publish_ll(idx, G_USER_PROPERTY_STATUS);
}

int mesh_property_st_publish_admin(u8 idx)
{
	return mesh_property_st_publish_ll(idx, G_ADMIN_PROPERTY_STATUS);
}

int mesh_property_st_publish_mfg(u8 idx)
{
	return mesh_property_st_publish_ll(idx, G_MFG_PROPERTY_STATUS);
}

int mesh_property_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id, int set_flag)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_property_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, prop_id, cb_par->op_rsp, set_flag);
}

int mesh_cmd_sig_property_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    u16 id = par[0] + (par[1]<<8);
	return mesh_property_st_rsp(cb_par, id, 0);
}

int mesh_cmd_sig_property_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = -1;
    if((par_len <= sizeof(mesh_property_set_user_t)) && (par_len >= OFFSETOF(mesh_property_set_user_t,data))){
        int save = 0;
        u16 op_stats = (cb_par->op_rsp != STATUS_NONE) ? cb_par->op_rsp : (cb_par->op + 1);
        u16 prop_id = par[0] + (par[1]<<8);
        if(!is_valid_prop_id(prop_id)){
            return -1;
        }
        
        mesh_property_t *p_prop = get_property_by_id(prop_id, op_stats);
        if(p_prop){
            if(G_USER_PROPERTY_STATUS == op_stats){
                if((par_len <= sizeof(mesh_property_set_user_t)) && (par_len >= OFFSETOF(mesh_property_set_user_t,data))){
                    mesh_property_set_user_t *p_set = (mesh_property_set_user_t *)par;
                    u8 len_val_set = par_len - OFFSETOF(mesh_property_set_user_t,data);
                    if(p_prop->len_val == len_val_set){ // valid parameters check, comfirm later
                        if((MESH_ACCESS_WRITE == p_prop->access)||(MESH_ACCESS_RW == p_prop->access)){
                            memset(p_prop->val, 0, sizeof(p_prop->val));
                            p_prop->len_val = len_val_set;
                            memcpy(p_prop->val, p_set->data, p_prop->len_val); // replaced
                            save = 1;
                        }
                        err = 0;
                    }
                }
            }else if(G_ADMIN_PROPERTY_STATUS == op_stats){
                if((par_len <= sizeof(mesh_property_set_admin_t)) && (par_len >= OFFSETOF(mesh_property_set_admin_t,data))){
                    mesh_property_set_admin_t *p_set = (mesh_property_set_admin_t *)par;
                    u8 len_val_set = par_len - OFFSETOF(mesh_property_set_admin_t,data);
                    if((p_prop->len_val == len_val_set) && (p_set->access < MESH_ACCESS_MAX)){ // valid parameters check, comfirm later
                        p_prop->access = p_set->access;
                        memset(p_prop->val, 0, sizeof(p_prop->val));
                        p_prop->len_val = len_val_set;
                        memcpy(p_prop->val, p_set->data, p_prop->len_val); // replaced
                        save = 1;
                        err = 0;
                    }
                }
            }else if(G_MFG_PROPERTY_STATUS == op_stats){
                mesh_property_set_mfg_t *p_set = (mesh_property_set_mfg_t *)par;
                if(p_set->access <= MESH_MFG_ACCESS_READ){
                    p_prop->access = p_set->access;
                    save = 1;
                    err = 0;
                }
            }
            if(err){
                return err;
            }
        }
        
        err = 0;
        if(cb_par->op_rsp != STATUS_NONE){
            err = mesh_property_st_rsp(cb_par, prop_id, 1);
        }
        
        if(save){
            mesh_model_store(1, SIG_MD_G_USER_PROP_S);
            mesh_property_st_publish_ll(cb_par->model_idx, op_stats);
        }
    }
    
	return err;
}
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_properties_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //if( == cb_par->op){
        //}
    }
    return err;
}
int mesh_cmd_sig_property_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //if( == cb_par->op){
        //}
    }
    return err;
}
#endif

//--model command interface-------------------
//-----------access command--------
/*int access_cmd_xxx_get(u16 adr, u32 rsp_max)
{
	return SendOpParaDebug(adr, rsp_max, , 0, 0);
}

int access_cmd_xxx_set(u16 adr, u32 rsp_max,  *p_set)
{
	return SendOpParaDebug(adr, rsp_max, , (u8 *)p_set, sizeof());
}*/


//--model command interface end----------------


#endif

