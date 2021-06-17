/********************************************************************************************************
 * @file     generic_model.c 
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
#include "vendor/common/user_config.h"
#include "app_health.h"
#include "app_heartbeat.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "vendor/common/remote_prov.h"
#include "vendor/common/lighting_model.h"
#include "vendor/common/lighting_model_HSL.h"
#include "vendor/common/lighting_model_xyL.h"
#include "vendor/common/lighting_model_LC.h"
#include "vendor/common/scene.h"
#include "vendor/common/time_model.h"
#include "vendor/common/sensors_model.h"
#include "vendor/common/scheduler.h"
#include "vendor/common/mesh_ota.h"
#include "vendor/common/mesh_property.h"
#include "generic_model.h"
#include "directed_forwarding.h"
#include "app_privacy_beacon.h"
#include "subnet_bridge.h"
/** @addtogroup Mesh_Common
  * @{
  */


/** @defgroup General_Model
  * @brief General Models Code.
  * @{
  */

#if MESH_RX_TEST
mesh_rcv_t mesh_rcv_cmd;
mesh_rcv_t mesh_rcv_ack;
u16 mesh_rsp_rec_addr;
#endif

model_g_onoff_level_t	model_sig_g_onoff_level;
model_g_power_onoff_trans_time_t   model_sig_g_power_onoff;
u32 mesh_md_g_power_onoff_addr = FLASH_ADR_MD_G_POWER_ONOFF;	// share with default transition time model

#if (1)
#if MD_SERVER_EN
//-------- sig generic model
//----generic onoff
int is_valid_transition_step(u8 transit_t)
{
	return (GET_TRANSITION_STEP(transit_t) <= TRANSITION_STEP_MAX);
}

/**
 * @brief  Fill in the parameters of the structure mesh_cmd_g_onoff_st_t.
 * @param  rsp: Pointer to structure mesh_cmd_g_onoff_st_t.
 * @param  idx: Light Count index.
 * @retval None
 */
void mesh_g_onoff_st_rsp_par_fill(mesh_cmd_g_onoff_st_t *rsp, u8 idx)
{
	mesh_cmd_g_level_st_t level_st; 
	light_g_level_get((u8 *)&level_st, idx, ST_TRANS_LIGHTNESS);

	rsp->present_onoff = get_onoff_from_level(level_st.present_level);
	rsp->target_onoff = get_onoff_from_level(level_st.target_level);
	rsp->remain_t = level_st.remain_t;
#if MESH_RX_TEST
	for(int i=0;i<sizeof(rsp->data);i++) {
		rsp->data[i] = i;
	}
#endif
}


/**
 * @brief  Send General Onoff Status message.
 * @param  idx: Light Count index.
 * @param  ele_adr: Element address.
 * @param  dst_adr: Destination address.
 * @param  uuid: When publishing, and the destination address is a virtual 
 *   address, uuid needs to be passed in, otherwise fill in 0.
 * @param  pub_md: When publishing, you need to pass in parameters. 
 *   For non-publish, pass in 0.
 * @param  op_rsp: The opcode to response
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_tx_cmd_g_onoff_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp)
{
    mesh_cmd_g_onoff_st_t rsp = {0};
    mesh_g_onoff_st_rsp_par_fill(&rsp, idx);
    u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 2;
	}
#if MESH_RX_TEST
	u8 par[8];
	memcpy(par, &rsp, sizeof(mesh_cmd_g_onoff_st_t));
	memcpy(par+3, &mesh_rcv_cmd.send_tick, 4);
	par[7] = mesh_rcv_cmd.send_index;
	len = 8;
	if(mesh_rcv_cmd.ack_pkt_num>1){
		len = 12*mesh_rcv_cmd.ack_pkt_num-6;
	}
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&par, len, ele_adr, dst_adr, uuid, pub_md);
#endif
    return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

/**
 * @brief  Send Generic OnOff Status message.
 * @param  cb_par  Some parameters about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_g_onoff_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mesh_tx_cmd_g_onoff_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, cb_par->op_rsp);
}

/**
 * @brief  Publish Generic OnOff Status.
 * @param  idx: Light Count index.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_g_onoff_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_g_onoff_level.onoff_srv[idx].com;	
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	
    return mesh_tx_cmd_g_onoff_st(idx, ele_adr, pub_adr, uuid, p_com_md, G_ONOFF_STATUS);
}

/**
 * @brief  When the Generic OnOff Get message is received, this function 
 *   will be called
 * @param  par: Pointer to message data (excluding Opcode).
 * @param  par_len: The length of the message data.
 * @param  cb_par: Some parameters about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_cmd_sig_g_onoff_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	#if DEBUG_CFG_CMD_GROUP_AK_EN
	memset(&nw_notify_record, 0x00, sizeof(nw_notify_record));
	#endif
    return mesh_g_onoff_st_rsp(cb_par);
}


/**
 * @brief  Set Generic OnOff States.
 * @param  p_set: Pointer to the set parameter structure.
 * @param  par_len: The length of the parameter p_set.
 * @param  force_last: When getting the current onoff state, whether 
 *   to force the last one.
 *     @arg 0: from last state when light on or from Non-volatile storage 
 *             when light off.
 *     @arg 1: Force from last state.
 * @param  idx: Light Count index.
 * @param  retransaction: Retransmission flag.
 *     @arg 0: Non-retransmission.
 *     @arg 1: Retransmission.
 * @param  pub_list: list of publication.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int g_onoff_set(mesh_cmd_g_onoff_set_t *p_set, int par_len, int force_last, int idx, bool4 retransaction, st_pub_list_t *pub_list)
{
	int err = -1;
	if(p_set->onoff < G_ONOFF_RSV){
		int st_trans_type = ST_TRANS_LIGHTNESS;
		mesh_cmd_g_level_set_t level_set_tmp;
		memcpy(&level_set_tmp.tid, &p_set->tid, sizeof(mesh_cmd_g_level_set_t) - OFFSETOF(mesh_cmd_g_level_set_t,tid));
		level_set_tmp.level = get_light_g_level_by_onoff(idx, p_set->onoff, st_trans_type, force_last);
		int len_tmp = GET_LEVEL_PAR_LEN(par_len >= sizeof(mesh_cmd_g_onoff_set_t));
		err = g_level_set((u8 *)&level_set_tmp, len_tmp, G_LEVEL_SET_NOACK, idx, retransaction, st_trans_type, 0, pub_list);
        if(!err){
		    set_on_power_up_onoff(idx, st_trans_type, p_set->onoff);
		}
	}
	
	return err;
}


/**
 * @brief  When the Generic OnOff Set or Generic OnOff Set Unacknowledged 
 *   message is received, this function will be called.
 * @param  par     Pointer to message data (excluding Opcode).
 * @param  par_len The length of the message data.
 * @param  cb_par  Some information about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_cmd_sig_g_onoff_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
    mesh_cmd_g_onoff_set_t *p_set = (mesh_cmd_g_onoff_set_t *)par;
#if MESH_RX_TEST
	if(par_len>sizeof(mesh_cmd_g_onoff_set_t)){
		memcpy(&mesh_rcv_cmd.send_tick, par+4, 4);
		mesh_rcv_cmd.send_index= par[8];
		mesh_rcv_cmd.rcv_cnt++;
		mesh_rcv_cmd.ack_pkt_num = par[3];
		mesh_rcv_cmd.rcv_time[par[8]%TEST_CNT] = (clock_time() - mesh_rcv_cmd.send_tick)/32/1000;
	}
	else{
		mesh_rcv_cmd.ack_pkt_num = 1;
	}
#endif

    st_pub_list_t pub_list = {{0}};
    err = g_onoff_set(p_set, par_len, 0, cb_par->model_idx, cb_par->retransaction, &pub_list);
	if(err){
		return err;
	}
	
#if MI_API_ENABLE
	// have transmit and delay 
	if(par_len == sizeof(mesh_cmd_g_onoff_set_t)){
		mi_pub_sigmodel_inter(p_set->transit_t,p_set->delay,1);
	}else{
		mi_pub_sigmodel_inter(0,0,0);
	}
#endif

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_g_onoff_st_rsp(cb_par);
	}else{
		VC_RefreshUI_level(cb_par);
	}

    model_pub_check_set_bind_all(&pub_list, cb_par, 0);

    return err;
}

//----generic level
/**
 * @brief  Fill in the parameters of the structure mesh_cmd_g_level_st_t.
 * @param  rsp: Pointer to structure mesh_cmd_g_level_st_t.
 * @param  model_idx: Element index.
 * @retval None
 */
void mesh_g_level_st_rsp_par_fill(mesh_cmd_g_level_st_t *rsp, u8 model_idx)
{
    int light_idx = get_light_idx_from_level_md_idx(model_idx);
    int trans_type = get_trans_type_from_level_md_idx(model_idx);
    light_g_level_get((u8 *)rsp, light_idx, trans_type);
}

/**
 * @brief  Send General Level Status message.
 * @param  idx: Light Count index.
 * @param  ele_adr: Element address.
 * @param  dst_adr: Destination address.
 * @param  uuid: When publishing, and the destination address is a virtual 
 *   address, uuid needs to be passed in, otherwise fill in 0.
 * @param  pub_md: When publishing, you need to pass in parameters. 
 *   For non-publish, pass in 0.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_tx_cmd_g_level_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
    mesh_cmd_g_level_st_t rsp = {0};
    mesh_g_level_st_rsp_par_fill(&rsp, idx);
    
    u32 len = sizeof(rsp);
	if(0 == rsp.remain_t){
		len -= 3;
	}
#if MESH_RX_TEST
	if (idx > 0){
		return 0;
	}
	mesh_rcv_t *p_result;
	if(BLS_LINK_STATE_CONN == blt_state){
		p_result = &mesh_rcv_ack;
	}
	else{
		p_result = &mesh_rcv_cmd;
	}
	len = OFFSETOF(mesh_rcv_t,max_time);
#if MESH_DELAY_TEST_EN
	u32 total_time = 0;
	len = sizeof(mesh_rcv_t);
	p_result->max_time= p_result->min_time = total_time = p_result->rcv_time[0];
	u8 valid_cnt = (p_result->rcv_cnt>TEST_CNT)? TEST_CNT:p_result->rcv_cnt;
	for(u8 i=1; i<valid_cnt; i++){
		if(p_result->rcv_time[i] > p_result->max_time){
			p_result->max_time= p_result->rcv_time[i];
		}
		if(p_result->rcv_time[i] < p_result->min_time){
			p_result->min_time= p_result->rcv_time[i];
		}
		total_time += p_result->rcv_time[i];
	}
	p_result->avr_time = total_time/valid_cnt;
#endif
	return mesh_tx_cmd_rsp(G_LEVEL_STATUS, (u8 *)p_result, len, ele_adr, dst_adr, uuid, pub_md);
#endif
    return mesh_tx_cmd_rsp(G_LEVEL_STATUS, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

#if MD_LEVEL_EN
int mesh_g_level_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_g_onoff_level.level_srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_g_level_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}
#endif

/**
 * @brief  Send Generic Level Status message.
 * @param  cb_par  Some parameters about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_g_level_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_g_level_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

/**
 * @brief  When the Generic Level Get message is received, 
 *   this function will be called.
 * @param  par: Pointer to message data (excluding Opcode).
 * @param  par_len: The length of the message data.
 * @param  cb_par: Some parameters about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_cmd_sig_g_level_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mesh_g_level_st_rsp(cb_par);
}

/**
 * @brief  When the Generic Level Set or Generic Level Set Unacknowledged 
 * message is received, this function will be called.
 * @param  par: Pointer to message data (excluding Opcode).
 * @param  par_len: The length of the message data.
 * @param  cb_par: Some parameters about function callbacks.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int mesh_cmd_sig_g_level_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
#if MESH_RX_TEST
	memset(&mesh_rcv_cmd, 0x00, sizeof(mesh_rcv_cmd));
	memset(&mesh_rcv_ack, 0x00, sizeof(mesh_rcv_ack));
#endif
    int err = 0;
    int light_idx = get_light_idx_from_level_md_idx(cb_par->model_idx);
    int trans_type = get_trans_type_from_level_md_idx(cb_par->model_idx);
#if CMD_LINEAR_EN
	if(trans_type == ST_TRANS_LIGHTNESS){
		clear_light_linear_flag(cb_par->model_idx);
	}
#endif
    u8 *par_set = par;
    st_pub_list_t pub_list = {{0}};
	err = g_level_set_and_update_last(par_set, par_len, cb_par->op, light_idx, cb_par->retransaction, trans_type, 0, &pub_list);
    if(err){
        return err;
    }

    if(cb_par->op_rsp != STATUS_NONE){
        err = mesh_g_level_st_rsp(cb_par);
    }else{
		VC_RefreshUI_level(cb_par);
    }
    
    model_pub_check_set_bind_all(&pub_list, cb_par, 0);
	
    return err;
}

#if MD_DEF_TRANSIT_TIME_EN
//generic default transition time
int mesh_tx_cmd_def_trans_time_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	u8 *par_rsp = (u8 *)&model_sig_g_power_onoff.trans_time[idx];
	return mesh_tx_cmd_rsp(G_DEF_TRANS_TIME_STATUS, par_rsp, 1, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_def_trans_time_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_g_power_onoff.def_trans_time_srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_def_trans_time_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_cmd_sig_def_trans_time_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mesh_tx_cmd_def_trans_time_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_def_trans_time_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
    trans_time_t *val_new = (trans_time_t *)par;
    trans_time_t *p_trans = &model_sig_g_power_onoff.trans_time[cb_par->model_idx];
    if(is_valid_transition_step(val_new->val)){
    	if((p_trans->val != val_new->val)){
			p_trans->val = val_new->val;
			mesh_model_store(1, SIG_MD_G_DEF_TRANSIT_TIME_S);
			model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 0);
		}
		
		if(cb_par->op_rsp != STATUS_NONE){
			err = mesh_cmd_sig_def_trans_time_get(par, par_len, cb_par);
		}else{
			err = 0;
		}
    }
    return err;
}
#endif

#if MD_POWER_ONOFF_EN
//generic power onoff
int mesh_tx_cmd_on_powerup_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	u8 *par_rsp = (u8 *)&model_sig_g_power_onoff.on_powerup[idx];
	return mesh_tx_cmd_rsp(G_ON_POWER_UP_STATUS, par_rsp, 1, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_on_powerup_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_g_power_onoff.pw_onoff_srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_on_powerup_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_cmd_sig_g_on_powerup_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mesh_tx_cmd_on_powerup_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_g_on_powerup_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    u8 val_new = par[0];
    u8 *p_on_powerup = &model_sig_g_power_onoff.on_powerup[cb_par->model_idx];
    if( val_new < G_ON_POWERUP_MAX){
    	if(*p_on_powerup != val_new){
			*p_on_powerup = val_new;
			mesh_model_store(1, SIG_MD_G_POWER_ONOFF_S);
			model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 0);
		}
		
		if(cb_par->op_rsp != STATUS_NONE){
			return mesh_cmd_sig_g_on_powerup_get(par, par_len, cb_par);
		}
		return 0;
    }
    return -1;
}
#endif

//----generic power
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
static inline int is_valid_power(u16 val)
{
	return (val != 0);
}

//generic power level
int mesh_g_power_st_publish(u8 idx)
{
	return mesh_lightness_st_publish_ll(idx, G_POWER_LEVEL_STATUS);
}

int mesh_cmd_sig_g_power_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mesh_cmd_sig_lightness_get(par, par_len, cb_par);
}

int mesh_cmd_sig_g_power_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_set(par, par_len, cb_par);
}

//----generic power last
int mesh_cmd_sig_g_power_last_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_last_get(par, par_len, cb_par);
}

//----generic power default
int mesh_cmd_sig_g_power_def_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_def_get(par, par_len, cb_par);
}

int mesh_cmd_sig_g_power_def_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_def_set(par, par_len, cb_par);
}

//----generic power range
int mesh_cmd_sig_g_power_range_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_range_get(par, par_len, cb_par);
}

int mesh_cmd_sig_g_power_range_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_lightness_range_set(par, par_len, cb_par);
}
#endif

//battery model
#if(MD_BATTERY_EN)
#if !WIN32
STATIC_ASSERT(MD_LOCATION_EN == 0);// because use same flash sector to save in mesh_save_map, and should be care of OTA new firmware which add MD_SENSOR_EN
#endif

int mesh_tx_cmd_battery_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_battery_st_t rsp = {0};
	rsp.battery_leve = 0xff;
	rsp.discharge_time = 0xffffff;
	rsp.charge_time = 0xffffff;
	rsp.battery_flag = 0xff;
	u32 len = sizeof(rsp);

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_cmd_sig_battery_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_battery_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, cb_par->op_rsp, 0, 0);
}

int mesh_cmd_sig_g_battery_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_cmd_sig_battery_get(par, par_len, cb_par);
}
#endif

//location model
#if(MD_LOCATION_EN)
#if !WIN32
STATIC_ASSERT((MD_SENSOR_EN == 0) && (MD_BATTERY_EN== 0));// because use same flash sector to save, and should be care of OTA new firmware which add MD_SENSOR_EN
#endif

mesh_generic_location_t mesh_generic_location = {
{0x80000000, 0x80000000, 0x7fff},
{0x8000, 0x8000, 0x7fff, 0xff, 0x00}
};

int mesh_tx_cmd_location_global_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_location_global_st_t *p_rsp = {0};
	p_rsp = (mesh_cmd_location_global_st_t *)&mesh_generic_location.location_global;
	u32 len = sizeof(mesh_cmd_location_global_st_t);

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)p_rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_location_global_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_location_global_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, cb_par->op_rsp, 0, 0);
}

int mesh_cmd_sig_g_location_global_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_location_global_st_rsp(cb_par);
}

int mesh_tx_cmd_location_local_st(u8 idx, u16 ele_adr, u16 dst_adr, u16 op_rsp, u8 *uuid, model_common_t *pub_md)
{
	mesh_cmd_location_local_st_t *p_rsp = {0};
	p_rsp = (mesh_cmd_location_local_st_t *)&mesh_generic_location.location_local;
	u32 len = sizeof(mesh_cmd_location_local_st_t);

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)p_rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_location_local_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_location_local_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, cb_par->op_rsp, 0, 0);
}

int mesh_cmd_sig_g_location_local_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_location_local_st_rsp(cb_par);
}

int mesh_cmd_sig_g_location_global_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	
	if(par_len == sizeof(mesh_cmd_location_global_st_t)){
		memcpy(&mesh_generic_location.location_global, par, sizeof(mesh_cmd_location_global_st_t));
	}

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_location_global_st_rsp(cb_par );
	}

	return err;
}

int mesh_cmd_sig_g_global_local_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	
	if(par_len == sizeof(mesh_cmd_location_local_st_t)){
		memcpy(&mesh_generic_location.location_local, par, sizeof(mesh_cmd_location_local_st_t));
	}

	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_location_local_st_rsp(cb_par );
	}

	return err;
}
#endif
#endif

//----generic onoff   // use for MESH_RX_TEST also when was in server model.
int mesh_cmd_sig_g_onoff_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
#if MESH_RX_TEST
	u32 tick;
	memcpy(&tick, par+3, 4);

	if(mesh_rsp_rec_addr != 0xffff){
		if(cb_par->adr_src == mesh_rsp_rec_addr){
			mesh_rcv_ack.rcv_cnt++;
			mesh_rcv_ack.rcv_time[par[7]%TEST_CNT] = (clock_time() - tick)/32/1000;
		}
	}
	else{
		mesh_rcv_ack.rcv_time[mesh_rcv_ack.rcv_cnt%TEST_CNT] = (clock_time() - tick)/32/1000;
		mesh_rcv_ack.rcv_cnt++;
	}
#endif
    return err;
}

#if MD_CLIENT_EN
//----generic level
int mesh_cmd_sig_g_level_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
//generic default transition time
int mesh_cmd_sig_def_trans_time_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
//generic power onoff
int mesh_cmd_sig_g_on_powerup_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
//----generic power
int mesh_cmd_sig_g_power_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//----generic power last
int mesh_cmd_sig_g_power_last_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

//----generic power default
int mesh_cmd_sig_g_power_def_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

//----generic power range
int mesh_cmd_sig_g_power_range_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

//battery model
int mesh_cmd_sig_g_battery_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

//location model
int mesh_cmd_sig_g_location_global_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int mesh_cmd_sig_g_location_local_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}
#endif

#if (MD_CLIENT_EN)
int mesh_cmd_sig_cfg_cps_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_sec_nw_bc_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_def_ttl_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_friend_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_gatt_proxy_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_key_phase_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_relay_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_nw_transmit_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_node_identity_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_lpn_poll_timeout_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_netkey_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_netkey_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_appkey_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_appkey_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_sig_model_app_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_vd_model_app_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_bind_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_node_reset_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_model_pub_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_cfg_model_sub_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_sig_model_sub_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_vendor_model_sub_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_cfg_cps_status                 (0)
#define mesh_cmd_sig_cfg_sec_nw_bc_status           (0)
#define mesh_cmd_sig_cfg_def_ttl_status             (0)
#define mesh_cmd_sig_cfg_friend_status              (0)
#define mesh_cmd_sig_cfg_gatt_proxy_status          (0)
#define mesh_cmd_sig_cfg_key_phase_status           (0)
#define mesh_cmd_sig_cfg_relay_status               (0)
#define mesh_cmd_sig_cfg_nw_transmit_status         (0)
#define mesh_cmd_sig_cfg_node_identity_status       (0)
#define mesh_cmd_sig_cfg_lpn_poll_timeout_status    (0)
#define mesh_cmd_sig_cfg_netkey_status              (0)
#define mesh_cmd_sig_cfg_netkey_list                (0)
#define mesh_cmd_sig_cfg_appkey_status              (0)
#define mesh_cmd_sig_cfg_appkey_list                (0)
#define mesh_cmd_sig_cfg_sig_model_app_list         (0)
#define mesh_cmd_sig_cfg_vd_model_app_list          (0)
#define mesh_cmd_sig_cfg_bind_status                (0)
#define mesh_cmd_sig_cfg_node_reset_status          (0)
#define mesh_cmd_sig_cfg_model_pub_status           (0)
#define mesh_cmd_sig_cfg_model_sub_status           (0)
#define mesh_cmd_sig_sig_model_sub_list             (0)
#define mesh_cmd_sig_vendor_model_sub_list          (0)
#endif

const mesh_cmd_sig_func_t mesh_cmd_sig_func[] = {
	// OP_TYPE_SIG1
    {APPKEY_ADD, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_appkey_set, APPKEY_STATUS},
	{APPKEY_UPDATE, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_appkey_set, APPKEY_STATUS},
    {COMPOSITION_DATA_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_cps_status, STATUS_NONE},
    {CFG_MODEL_PUB_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_pub_set, CFG_MODEL_PUB_STATUS},
	{HEALTH_CURRENT_STATUS,1,SIG_MD_HEALTH_SERVER,SIG_MD_HEALTH_CLIENT,mesh_cmd_sig_health_cur_sts,STATUS_NONE},
	{HEALTH_FAULT_STATUS,1,SIG_MD_HEALTH_SERVER,SIG_MD_HEALTH_CLIENT,mesh_cmd_sig_health_fault_sts,STATUS_NONE},
    {HEARTBEAT_PUB_STATUS,1,SIG_MD_CFG_SERVER,SIG_MD_CFG_CLIENT, mesh_cmd_sig_heart_pub_status ,STATUS_NONE },

	// OP_TYPE_SIG2
    // config
    {APPKEY_DEL, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_appkey_set, APPKEY_STATUS},
	{APPKEY_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_appkey_get, APPKEY_LIST},
	{APPKEY_LIST, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_appkey_list, STATUS_NONE},
	{APPKEY_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_appkey_status, STATUS_NONE},
    {COMPOSITION_DATA_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_cps_get, COMPOSITION_DATA_STATUS},
    {CFG_BEACON_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_sec_nw_bc_get, CFG_BEACON_STATUS},
	{CFG_BEACON_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_sec_nw_bc_set, CFG_BEACON_STATUS},
    {CFG_BEACON_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_sec_nw_bc_status, STATUS_NONE},
    {CFG_DEFAULT_TTL_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_def_ttl_get, CFG_DEFAULT_TTL_STATUS},
	{CFG_DEFAULT_TTL_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_def_ttl_set, CFG_DEFAULT_TTL_STATUS},
    {CFG_DEFAULT_TTL_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_def_ttl_status, STATUS_NONE},
	{CFG_FRIEND_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_friend_get, CFG_FRIEND_STATUS},
	{CFG_FRIEND_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_friend_set, CFG_FRIEND_STATUS},
	{CFG_FRIEND_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_friend_status, STATUS_NONE},
	{CFG_GATT_PROXY_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_gatt_proxy_get, CFG_GATT_PROXY_STATUS},
	{CFG_GATT_PROXY_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_gatt_proxy_set, CFG_GATT_PROXY_STATUS},
	{CFG_GATT_PROXY_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_gatt_proxy_status, STATUS_NONE},
	{CFG_KEY_REFRESH_PHASE_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_key_phase_get, CFG_KEY_REFRESH_PHASE_STATUS},
	{CFG_KEY_REFRESH_PHASE_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_key_phase_set, CFG_KEY_REFRESH_PHASE_STATUS},
	{CFG_KEY_REFRESH_PHASE_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_key_phase_status, STATUS_NONE},
	{CFG_NW_TRANSMIT_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_nw_transmit_get, CFG_NW_TRANSMIT_STATUS},
	{CFG_NW_TRANSMIT_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_nw_transmit_set, CFG_NW_TRANSMIT_STATUS},
	{CFG_NW_TRANSMIT_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_nw_transmit_status, STATUS_NONE},
    {CFG_RELAY_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_relay_get, CFG_RELAY_STATUS},
	{CFG_RELAY_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_relay_set, CFG_RELAY_STATUS},
    {CFG_RELAY_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_relay_status, STATUS_NONE},
	{CFG_LPN_POLL_TIMEOUT_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_lpn_poll_timeout_get, CFG_LPN_POLL_TIMEOUT_STATUS},
	{CFG_LPN_POLL_TIMEOUT_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_lpn_poll_timeout_status, STATUS_NONE},
    {CFG_MODEL_PUB_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_pub_get, CFG_MODEL_PUB_STATUS},
    {CFG_MODEL_PUB_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_model_pub_status, STATUS_NONE},
	{CFG_MODEL_PUB_VIRTUAL_ADR_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_pub_set_vr, CFG_MODEL_PUB_STATUS},
    {CFG_MODEL_SUB_ADD, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
    {CFG_MODEL_SUB_DEL, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
    {CFG_MODEL_SUB_OVER_WRITE, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
    {CFG_MODEL_SUB_DEL_ALL, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
    {CFG_MODEL_SUB_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_model_sub_status, STATUS_NONE},
	{CFG_MODEL_SUB_VIRTUAL_ADR_ADD, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
	{CFG_MODEL_SUB_VIRTUAL_ADR_DEL, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
	{CFG_MODEL_SUB_VIRTUAL_ADR_OVER_WRITE, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_sub_set, CFG_MODEL_SUB_STATUS},
    {CFG_SIG_MODEL_SUB_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_sig_model_sub_get, CFG_SIG_MODEL_SUB_LIST},
    {CFG_SIG_MODEL_SUB_LIST, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_sig_model_sub_list, STATUS_NONE},
    {CFG_VENDOR_MODEL_SUB_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_vendor_model_sub_get, CFG_VENDOR_MODEL_SUB_LIST},
    {CFG_VENDOR_MODEL_SUB_LIST, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_vendor_model_sub_list, STATUS_NONE},
    	// heart beat pub
    {HEARTBEAT_PUB_GET,0,SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER,mesh_cmd_sig_heartbeat_pub_get,HEARTBEAT_PUB_STATUS},
    {HEARTBEAT_PUB_SET,0,SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER,mesh_cmd_sig_heartbeat_pub_set,HEARTBEAT_PUB_STATUS},
    // heart beat sub HEARTBEAT_SUB_STATUS
    {HEARTBEAT_SUB_GET,0,SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER,mesh_cmd_sig_heartbeat_sub_get,HEARTBEAT_SUB_STATUS},
    {HEARTBEAT_SUB_SET,0,SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER,mesh_cmd_sig_heartbeat_sub_set,HEARTBEAT_SUB_STATUS},
    {HEARTBEAT_SUB_STATUS,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_heartbeat_sub_status,STATUS_NONE},
	{MODE_APP_BIND, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_bind, MODE_APP_STATUS},
	{MODE_APP_UNBIND, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_bind, MODE_APP_STATUS},
	{MODE_APP_STATUS,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_cfg_bind_status,STATUS_NONE},
	{NETKEY_ADD, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_netkey_set, NETKEY_STATUS},
	{NETKEY_DEL, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_netkey_set, NETKEY_STATUS},
	{NETKEY_UPDATE, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_netkey_set, NETKEY_STATUS},
	{NETKEY_STATUS,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_cfg_netkey_status,STATUS_NONE},
	{NODE_ID_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_node_identity_get, NODE_ID_STATUS},
	{NODE_ID_SET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_node_identity_set, NODE_ID_STATUS},
	{NODE_ID_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_node_identity_status, STATUS_NONE},
	{NETKEY_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_netkey_get, NETKEY_LIST},
	{NETKEY_LIST,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_cfg_netkey_list,STATUS_NONE},
	{NODE_RESET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_node_reset, NODE_RESET_STATUS},
	{NODE_RESET_STATUS, 1, SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT, mesh_cmd_sig_cfg_node_reset_status, STATUS_NONE},
	{SIG_MODEL_APP_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_app_get, SIG_MODEL_APP_LIST},
	{SIG_MODEL_APP_LIST,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_cfg_sig_model_app_list,STATUS_NONE},
	{VENDOR_MODEL_APP_GET, 0, SIG_MD_CFG_CLIENT, SIG_MD_CFG_SERVER, mesh_cmd_sig_cfg_model_app_get, VENDOR_MODEL_APP_LIST},
	{VENDOR_MODEL_APP_LIST,1,SIG_MD_CFG_SERVER, SIG_MD_CFG_CLIENT,mesh_cmd_sig_cfg_vd_model_app_list,STATUS_NONE},
	
	// health fault part 
	{HEALTH_FAULT_GET,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_fault_get,HEALTH_FAULT_STATUS},
    {HEALTH_FAULT_CLEAR,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_fault_clr,HEALTH_FAULT_STATUS},
    {HEALTH_FAULT_CLEAR_NOACK,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_fault_clr_unrel,STATUS_NONE},
    {HEALTH_FAULT_TEST,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_fault_test,HEALTH_FAULT_STATUS},
    {HEALTH_FAULT_TEST_NOACK,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_fault_test_unrel,STATUS_NONE},
    // health period part 
    {HEALTH_PERIOD_GET,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_period_get,HEALTH_PERIOD_STATUS},
    {HEALTH_PERIOD_SET,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_period_set,HEALTH_PERIOD_STATUS},
    {HEALTH_PERIOD_SET_NOACK,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_period_set_unrel,STATUS_NONE},
    {HEALTH_PERIOD_STATUS,1,SIG_MD_HEALTH_SERVER,SIG_MD_HEALTH_CLIENT,mesh_cmd_sig_period_status,STATUS_NONE},
    // attention timer 
    {HEALTH_ATTENTION_GET,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_attention_get,HEALTH_ATTENTION_STATUS},
    {HEALTH_ATTENTION_SET,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_attention_set,HEALTH_ATTENTION_STATUS},
    {HEALTH_ATTENTION_SET_NOACK,0,SIG_MD_HEALTH_CLIENT,SIG_MD_HEALTH_SERVER,mesh_cmd_sig_attention_set_unrel,STATUS_NONE},
    {HEALTH_ATTENTION_STATUS,1,SIG_MD_HEALTH_SERVER,SIG_MD_HEALTH_CLIENT,mesh_cmd_sig_attention_status,STATUS_NONE},

#if MD_DF_EN
	// directed forwarding
	{DIRECTED_CONTROL_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_get,DIRECTED_CONTROL_STATUS},
	{DIRECTED_CONTROL_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_set,DIRECTED_CONTROL_STATUS},
    {DIRECTED_CONTROL_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_control_status,STATUS_NONE},
    {PATH_METRIC_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_metric_get,PATH_METRIC_STATUS},
    {PATH_METRIC_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_metric_set,PATH_METRIC_STATUS},
    {PATH_METRIC_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_path_metric_status,STATUS_NONE},
	{DISCOVERY_TABLE_CAPABILITIES_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_dsc_tbl_capa_get,DISCOVERY_TABLE_CAPABILITIES_STATUS},
	{DISCOVERY_TABLE_CAPABILITIES_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_dsc_tbl_capa_set,DISCOVERY_TABLE_CAPABILITIES_STATUS},
    {DISCOVERY_TABLE_CAPABILITIES_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_dsc_tbl_capa_status,STATUS_NONE},
    {FORWARDING_TABLE_ADD,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_add,FORWARDING_TABLE_STATUS},
    {FORWARDING_TABLE_DELETE,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_delete,FORWARDING_TABLE_STATUS},
    {FORWARDING_TABLE_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_forwarding_tbl_status,STATUS_NONE},    
    {FORWARDING_TABLE_DEPENDENTS_ADD,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_dependents_add,FORWARDING_TABLE_DEPENDENTS_STATUS},
    {FORWARDING_TABLE_DEPENDENTS_DELETE,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_dependents_delete,FORWARDING_TABLE_DEPENDENTS_STATUS},
    {FORWARDING_TABLE_DEPENDENTS_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_forwarding_tbl_dependents_status,STATUS_NONE},
	{FORWARDING_TABLE_DEPENDENTS_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_dependents_get,FORWARDING_TABLE_DEPENDENTS_GET_STATUS},
   	{FORWARDING_TABLE_DEPENDENTS_GET_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_forwarding_tbl_dependents_get_status,STATUS_NONE},   	
	{FORWARDING_TABLE_ENTRIES_COUNT_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_entries_count_get,FORWARDING_TABLE_ENTRIES_COUNT_STATUS},
	{FORWARDING_TABLE_ENTRIES_COUNT_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_forwarding_tbl_entries_count_status,STATUS_NONE},
	{FORWARDING_TABLE_ENTRIES_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_forwarding_tbl_entries_get,FORWARDING_TABLE_ENTRIES_STATUS},
	{FORWARDING_TABLE_ENTRIES_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_forwarding_tbl_entries_status,STATUS_NONE},
	{WANTED_LANES_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_wanted_lanes_get,WANTED_LANES_STATUS},
	{WANTED_LANES_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_wanted_lanes_set,WANTED_LANES_STATUS},
    {WANTED_LANES_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_wanted_lanes_status,STATUS_NONE},
    {TWO_WAY_PATH_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_two_way_path_get,TWO_WAY_PATH_STATUS},
	{TWO_WAY_PATH_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_two_way_path_set,TWO_WAY_PATH_STATUS},
    {TWO_WAY_PATH_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_two_way_path_status,STATUS_NONE},
	{PATH_ECHO_INTERVAL_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_echo_interval_get,PATH_ECHO_INTERVAL_STATUS},
	{PATH_ECHO_INTERVAL_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_echo_interval_set,PATH_ECHO_INTERVAL_STATUS},
	{PATH_ECHO_INTERVAL_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_path_echo_interval_status,STATUS_NONE},
	{DIRECTED_NETWORK_TRANSMIT_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_network_transmit_get,DIRECTED_NETWORK_TRANSMIT_STATUS},
    {DIRECTED_NETWORK_TRANSMIT_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_network_transmit_set,DIRECTED_NETWORK_TRANSMIT_STATUS},
    {DIRECTED_NETWORK_TRANSMIT_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_network_transmit_status,STATUS_NONE},
	{DIRECTED_RELAY_RETRANSMIT_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_relay_retransmit_get,DIRECTED_RELAY_RETRANSMIT_STATUS},
    {DIRECTED_RELAY_RETRANSMIT_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_relay_retransmit_set,DIRECTED_RELAY_RETRANSMIT_STATUS},
    {DIRECTED_RELAY_RETRANSMIT_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_relay_retransmit_status,STATUS_NONE},
	{RSSI_THRESHOLD_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_rssi_threshold_get,RSSI_THRESHOLD_STATUS},
	{RSSI_THRESHOLD_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_rssi_threshold_set,RSSI_THRESHOLD_STATUS},
	{RSSI_THRESHOLD_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_rssi_threshold_status,STATUS_NONE}, 
	{DIRECTED_PATHS_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_paths_get,DIRECTED_PATHS_STATUS},
    {DIRECTED_PATHS_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_paths_status,STATUS_NONE},
    {DIRECTED_PUBLISH_POLICY_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_publish_policy_get,DIRECTED_PUBLISH_POLICY_STATUS},
    {DIRECTED_PUBLISH_POLICY_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_publish_policy_set,DIRECTED_PUBLISH_POLICY_STATUS},
    {DIRECTED_PUBLISH_POLICY_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_publish_policy_status,STATUS_NONE},
	{PATH_DISCOVERY_TIMING_CONTROL_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_discovery_timing_control_get,PATH_DISCOVERY_TIMING_CONTROL_STATUS},
	{PATH_DISCOVERY_TIMING_CONTROL_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_path_discovery_timing_control_set,PATH_DISCOVERY_TIMING_CONTROL_STATUS},
	{PATH_DISCOVERY_TIMING_CONTROL_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_path_discovery_timing_control_status,STATUS_NONE},
    {DIRECTED_CONTROL_NETWORK_TRANSMIT_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_network_transmit_get,DIRECTED_CONTROL_NETWORK_TRANSMIT_STATUS},
    {DIRECTED_CONTROL_NETWORK_TRANSMIT_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_network_transmit_set,DIRECTED_CONTROL_NETWORK_TRANSMIT_STATUS},
    {DIRECTED_CONTROL_NETWORK_TRANSMIT_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_control_network_transmit_status,STATUS_NONE},    
    {DIRECTED_CONTROL_RELAY_RETRANSMIT_GET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_relay_transmit_get,DIRECTED_CONTROL_RELAY_RETRANSMIT_STATUS},
    {DIRECTED_CONTROL_RELAY_RETRANSMIT_SET,0,SIG_MD_DF_CFG_C, SIG_MD_DF_CFG_S,mesh_cmd_sig_cfg_directed_control_relay_transmit_set,DIRECTED_CONTROL_RELAY_RETRANSMIT_STATUS},
    {DIRECTED_CONTROL_RELAY_RETRANSMIT_STATUS,1,SIG_MD_DF_CFG_S, SIG_MD_DF_CFG_C,mesh_cmd_sig_cfg_directed_control_relay_transmit_status,STATUS_NONE},      
#endif

#if MD_SBR_EN
	// subnet bridge
	{SUBNET_BRIDGE_GET,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_subnet_bridge_get,SUBNET_BRIDGE_STATUS},
	{SUBNET_BRIDGE_SET,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_subnet_bridge_set,SUBNET_BRIDGE_STATUS},
    {SUBNET_BRIDGE_STATUS,1,SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT,mesh_cmd_sig_cfg_subnet_bridge_status,STATUS_NONE},    
    {BRIDGING_TABLE_ADD,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_bridging_tbl_add,BRIDGING_TABLE_STATUS},
	{BRIDGING_TABLE_REMOVE,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_bridging_tbl_remove,BRIDGING_TABLE_STATUS},
    {BRIDGING_TABLE_STATUS,1,SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT,mesh_cmd_sig_cfg_bridging_tbl_status,STATUS_NONE},
	{BRIDGED_SUBNETS_GET,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_bridged_subnets_get,BRIDGED_SUBNETS_LIST},
    {BRIDGED_SUBNETS_LIST,1,SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT,mesh_cmd_sig_cfg_bridged_subnets_list,STATUS_NONE},	
	{BRIDGING_TABLE_GET,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_bridging_tbl_get,BRIDGING_TABLE_LIST},
    {BRIDGING_TABLE_LIST,1,SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT,mesh_cmd_sig_cfg_bridging_tbl_list,STATUS_NONE},	
	{BRIDGE_CAPABILITY_GET,0,SIG_MD_BRIDGE_CFG_CLIENT, SIG_MD_BRIDGE_CFG_SERVER,mesh_cmd_sig_cfg_bridge_capa_get,BRIDGE_CAPABILITY_STATUS},
    {BRIDGE_CAPABILITY_STATUS,1,SIG_MD_BRIDGE_CFG_SERVER, SIG_MD_BRIDGE_CFG_CLIENT,mesh_cmd_sig_cfg_bridging_tbl_list,STATUS_NONE},
#endif

#if MD_REMOTE_PROV  
    // remote provision scan capability 
    {REMOTE_PROV_SCAN_CAPA_GET,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_scan_capa,REMOTE_PROV_SCAN_CAPA_STS},
    {REMOTE_PROV_SCAN_CAPA_STS,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_scan_capa_sts,STATUS_NONE},

    // remote provision scan parameters 
    {REMOTE_PROV_SCAN_GET,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_scan_get,REMOTE_PROV_SCAN_STS},
    #if WIN32
	{REMOTE_PROV_SCAN_START,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_scan_start,STATUS_NONE},
	#else
    {REMOTE_PROV_SCAN_START,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_scan_start,REMOTE_PROV_SCAN_STS},
	#endif
	{REMOTE_PROV_SCAN_STOP,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_scan_stop,REMOTE_PROV_SCAN_STS},
    {REMOTE_PROV_SCAN_STS,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_scan_sts,STATUS_NONE},
    {REMOTE_PROV_SCAN_REPORT,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_scan_report,STATUS_NONE},
    // remote provision extend scan part 
    {REMOTE_PROV_EXTEND_SCAN_START,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_extend_scan_start,STATUS_NONE},
    {REMOTE_PROV_EXTEND_SCAN_REPORT,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_extend_scan_report,STATUS_NONE},

    // remote provision link parameters 
    {REMOTE_PROV_LINK_GET,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_link_get,REMOTE_PROV_LINK_STS},
	#if WIN32
	// binding the link report ,when send the link open cmd ,it should rsp the link report .
	{REMOTE_PROV_LINK_OPEN,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_link_open,REMOTE_PROV_LINK_REPORT},
	#else
	{REMOTE_PROV_LINK_OPEN,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_link_open,REMOTE_PROV_LINK_STS},
	#endif
	{REMOTE_PROV_LINK_CLOSE,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_link_close,REMOTE_PROV_LINK_STS},
    {REMOTE_PROV_LINK_STS,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_link_sts,STATUS_NONE},
    {REMOTE_PROV_LINK_REPORT,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_link_report,STATUS_NONE},
    {REMOTE_PROV_PDU_SEND,0,SIG_MD_REMOTE_PROV_CLIENT,SIG_MD_REMOTE_PROV_SERVER,mesh_cmd_sig_rp_pdu_send,STATUS_NONE},
    {REMOTE_PROV_PDU_OUTBOUND_REPORT,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_pdu_outbound_report,STATUS_NONE},
    {REMOTE_PROV_PDU_REPORT,1,SIG_MD_REMOTE_PROV_SERVER,SIG_MD_REMOTE_PROV_CLIENT,mesh_cmd_sig_rp_pdu_report,STATUS_NONE},
#endif

#if MD_PRIVACY_BEA
	{PRIVATE_BEACON_GET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_beacon_get,PRIVATE_BEACON_STATUS},
	{PRIVATE_BEACON_SET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_beacon_set,PRIVATE_BEACON_STATUS},
	{PRIVATE_BEACON_STATUS,1,SIG_MD_PRIVATE_BEACON_SERVER,SIG_MD_PRIVATE_BEACON_CLIENT,mesh_cmd_sig_beacon_sts,STATUS_NONE},

	{PRIVATE_GATT_PROXY_GET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_gatt_proxy_get,PRIVATE_GATT_PROXY_STATUS},
	{PRIVATE_GATT_PROXY_SET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_gatt_proxy_set,PRIVATE_GATT_PROXY_STATUS},
	{PRIVATE_GATT_PROXY_STATUS,1,SIG_MD_PRIVATE_BEACON_SERVER,SIG_MD_PRIVATE_BEACON_CLIENT,mesh_cmd_sig_gatt_proxy_sts,STATUS_NONE},

	{PRIVATE_NODE_IDENTITY_GET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_private_node_identity_get,PRIVATE_NODE_IDENTITY_STATUS},
	{PRIVATE_NODE_IDENTITY_SET,0,SIG_MD_PRIVATE_BEACON_CLIENT,SIG_MD_PRIVATE_BEACON_SERVER,mesh_cmd_sig_private_node_identity_set,PRIVATE_NODE_IDENTITY_STATUS},
	{PRIVATE_NODE_IDENTITY_STATUS,1,SIG_MD_PRIVATE_BEACON_SERVER,SIG_MD_PRIVATE_BEACON_CLIENT,mesh_cmd_sig_private_node_identity_sts,STATUS_NONE},
#endif

    // generic
    {G_ONOFF_GET, 0, SIG_MD_G_ONOFF_C, SIG_MD_G_ONOFF_S, mesh_cmd_sig_g_onoff_get, G_ONOFF_STATUS},
    {G_ONOFF_SET, 0, SIG_MD_G_ONOFF_C, SIG_MD_G_ONOFF_S, mesh_cmd_sig_g_onoff_set, G_ONOFF_STATUS},
    {G_ONOFF_SET_NOACK, 0, SIG_MD_G_ONOFF_C, SIG_MD_G_ONOFF_S, mesh_cmd_sig_g_onoff_set, STATUS_NONE},
    {G_ONOFF_STATUS, 1, SIG_MD_G_ONOFF_S, SIG_MD_G_ONOFF_C, mesh_cmd_sig_g_onoff_status, STATUS_NONE},

	// battery server
#if MD_BATTERY_EN
	{G_BATTERY_GET, 0, SIG_MD_G_BAT_C, SIG_MD_G_BAT_S, mesh_cmd_sig_g_battery_get, G_BATTERY_STATUS},
    {G_BATTERY_STATUS, 1, SIG_MD_G_BAT_S, SIG_MD_G_BAT_C, mesh_cmd_sig_g_battery_status, STATUS_NONE},
#endif

#if MD_LOCATION_EN
	{G_LOCATION_GLOBAL_GET, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_S, mesh_cmd_sig_g_location_global_get, G_LOCATION_GLOBAL_STATUS},
	{G_LOCATION_GLOBAL_STATUS, 1, SIG_MD_G_LOCATION_S, SIG_MD_G_LOCATION_C, mesh_cmd_sig_g_location_global_status, STATUS_NONE},	
	{G_LOCATION_LOCAL_GET, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_S, mesh_cmd_sig_g_location_local_get, G_LOCATION_LOCAL_STATUS},
	{G_LOCATION_LOCAL_STATUS, 1, SIG_MD_G_LOCATION_S, SIG_MD_G_LOCATION_C, mesh_cmd_sig_g_location_local_status, STATUS_NONE},	
	{G_LOCATION_GLOBAL_SET, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_SETUP_S, mesh_cmd_sig_g_location_global_set, G_LOCATION_GLOBAL_STATUS},
	{G_LOCATION_GLOBAL_SET_NOACK, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_SETUP_S, mesh_cmd_sig_g_location_global_set, STATUS_NONE},
	{G_LOCATION_LOCAL_SET, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_SETUP_S, mesh_cmd_sig_g_global_local_set, G_LOCATION_LOCAL_STATUS},
	{G_LOCATION_LOCAL_SET_NOACK, 0, SIG_MD_G_LOCATION_C, SIG_MD_G_LOCATION_SETUP_S, mesh_cmd_sig_g_global_local_set, STATUS_NONE},
#endif

#if MD_LEVEL_EN    
    {G_LEVEL_GET, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_get, G_LEVEL_STATUS},
    {G_LEVEL_SET, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, G_LEVEL_STATUS},
    {G_LEVEL_SET_NOACK, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, STATUS_NONE},
	{G_DELTA_SET, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, G_LEVEL_STATUS},
	{G_DELTA_SET_NOACK, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, STATUS_NONE},
	{G_MOVE_SET, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, G_LEVEL_STATUS},
	{G_MOVE_SET_NOACK, 0, SIG_MD_G_LEVEL_C, SIG_MD_G_LEVEL_S, mesh_cmd_sig_g_level_set, STATUS_NONE},
    {G_LEVEL_STATUS, 1, SIG_MD_G_LEVEL_S, SIG_MD_G_LEVEL_C, mesh_cmd_sig_g_level_status, STATUS_NONE},
#endif

#if MD_DEF_TRANSIT_TIME_EN
	{G_DEF_TRANS_TIME_GET, 0, SIG_MD_G_DEF_TRANSIT_TIME_C, SIG_MD_G_DEF_TRANSIT_TIME_S, mesh_cmd_sig_def_trans_time_get, G_DEF_TRANS_TIME_STATUS},
	{G_DEF_TRANS_TIME_SET, 0, SIG_MD_G_DEF_TRANSIT_TIME_C, SIG_MD_G_DEF_TRANSIT_TIME_S, mesh_cmd_sig_def_trans_time_set, G_DEF_TRANS_TIME_STATUS},
	{G_DEF_TRANS_TIME_SET_NOACK, 0, SIG_MD_G_DEF_TRANSIT_TIME_C, SIG_MD_G_DEF_TRANSIT_TIME_S, mesh_cmd_sig_def_trans_time_set, STATUS_NONE},
	{G_DEF_TRANS_TIME_STATUS, 1, SIG_MD_G_DEF_TRANSIT_TIME_S, SIG_MD_G_DEF_TRANSIT_TIME_C, mesh_cmd_sig_def_trans_time_status, STATUS_NONE},
#endif

#if MD_POWER_ONOFF_EN
	{G_ON_POWER_UP_GET, 0, SIG_MD_G_POWER_ONOFF_C, SIG_MD_G_POWER_ONOFF_S, mesh_cmd_sig_g_on_powerup_get, G_ON_POWER_UP_STATUS},
	{G_ON_POWER_UP_SET, 0, SIG_MD_G_POWER_ONOFF_C, SIG_MD_G_POWER_ONOFF_SETUP_S, mesh_cmd_sig_g_on_powerup_set, G_ON_POWER_UP_STATUS},
	{G_ON_POWER_UP_SET_NOACK, 0, SIG_MD_G_POWER_ONOFF_C, SIG_MD_G_POWER_ONOFF_SETUP_S, mesh_cmd_sig_g_on_powerup_set, STATUS_NONE},
	{G_ON_POWER_UP_STATUS, 1, SIG_MD_G_POWER_ONOFF_S, SIG_MD_G_POWER_ONOFF_C, mesh_cmd_sig_g_on_powerup_status, STATUS_NONE},
#endif

#if MD_SCENE_EN
	{SCENE_STORE, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, mesh_cmd_sig_scene_set, SCENE_REG_STATUS},
	{SCENE_STORE_NOACK, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, mesh_cmd_sig_scene_set, STATUS_NONE},
	{SCENE_RECALL, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_S, mesh_cmd_sig_scene_recall, SCENE_STATUS},
	{SCENE_RECALL_NOACK, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_S, mesh_cmd_sig_scene_recall, STATUS_NONE},
	{SCENE_GET, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_S, mesh_cmd_sig_scene_get, SCENE_STATUS},
	{SCENE_STATUS, 1, SIG_MD_SCENE_S, SIG_MD_SCENE_C, mesh_cmd_sig_scene_status, STATUS_NONE},
	{SCENE_REG_GET, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_S, mesh_cmd_sig_scene_reg_get, SCENE_REG_STATUS},
	{SCENE_REG_STATUS, 1, SIG_MD_SCENE_S, SIG_MD_SCENE_C, mesh_cmd_sig_scene_reg_status, STATUS_NONE},
	{SCENE_DEL, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, mesh_cmd_sig_scene_set, SCENE_REG_STATUS},
	{SCENE_DEL_NOACK, 0, SIG_MD_SCENE_C, SIG_MD_SCENE_SETUP_S, mesh_cmd_sig_scene_set, STATUS_NONE},
#endif
#if MD_TIME_EN
	{TIME_GET, 0, SIG_MD_TIME_C, SIG_MD_TIME_S, mesh_cmd_sig_time_get, TIME_STATUS},
	{TIME_SET, 0, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, mesh_cmd_sig_time_set, TIME_STATUS},
	{TIME_STATUS, 1, SIG_MD_TIME_S, SIG_MD_TIME_C, mesh_cmd_sig_time_status, STATUS_NONE},
	{TIME_ZONE_GET, 0, SIG_MD_TIME_C, SIG_MD_TIME_S, mesh_cmd_sig_time_zone_get, TIME_ZONE_STATUS},
	{TIME_ZONE_SET, 0, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, mesh_cmd_sig_time_zone_set, TIME_ZONE_STATUS},
	{TIME_ZONE_STATUS, 1, SIG_MD_TIME_S, SIG_MD_TIME_C, mesh_cmd_sig_time_zone_status, STATUS_NONE},
	{TAI_UTC_DELTA_GET, 0, SIG_MD_TIME_C, SIG_MD_TIME_S, mesh_cmd_sig_time_TAI_UTC_delta_get, TAI_UTC_DELTA_STATUS},
	{TAI_UTC_DELTA_SET, 0, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, mesh_cmd_sig_time_TAI_UTC_delta_set, TAI_UTC_DELTA_STATUS},
	{TAI_UTC_DELTA_STATUS, 1, SIG_MD_TIME_S, SIG_MD_TIME_C, mesh_cmd_sig_time_TAI_UTC_delta_status, STATUS_NONE},
	{TIME_ROLE_GET, 0, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, mesh_cmd_sig_time_role_get, TIME_ROLE_STATUS},
	{TIME_ROLE_SET, 0, SIG_MD_TIME_C, SIG_MD_TIME_SETUP_S, mesh_cmd_sig_time_role_set, TIME_ROLE_STATUS},
	{TIME_ROLE_STATUS, 1, SIG_MD_TIME_SETUP_S, SIG_MD_TIME_C, mesh_cmd_sig_time_role_status, STATUS_NONE},
#endif
#if MD_SCHEDULE_EN
	{SCHD_GET, 0, SIG_MD_SCHED_C, SIG_MD_SCHED_S, mesh_cmd_sig_scheduler_get, SCHD_STATUS},
	{SCHD_STATUS, 1, SIG_MD_SCHED_S, SIG_MD_SCHED_C, mesh_cmd_sig_scheduler_status, STATUS_NONE},
	{SCHD_ACTION_GET, 0, SIG_MD_SCHED_C, SIG_MD_SCHED_SETUP_S, mesh_cmd_sig_schd_action_get, SCHD_ACTION_STATUS},
	{SCHD_ACTION_SET, 0, SIG_MD_SCHED_C, SIG_MD_SCHED_SETUP_S, mesh_cmd_sig_schd_action_set, SCHD_ACTION_STATUS},
	{SCHD_ACTION_SET_NOACK, 0, SIG_MD_SCHED_C, SIG_MD_SCHED_S, mesh_cmd_sig_schd_action_set, STATUS_NONE},
	{SCHD_ACTION_STATUS, 1, SIG_MD_SCHED_S, SIG_MD_SCHED_C, mesh_cmd_sig_schd_action_status, STATUS_NONE},
#endif
#if MD_SENSOR_EN
	{SENSOR_DESCRIP_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_descript_get, SENSOR_DESCRIP_STATUS},
	{SENSOR_DESCRIP_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_descript_status, STATUS_NONE},
	{SENSOR_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_get, SENSOR_STATUS},
	{SENSOR_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_status, STATUS_NONE},
	{SENSOR_COLUMN_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_column_get, SENSOR_COLUMN_STATUS},
	{SENSOR_COLUMN_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_column_status, STATUS_NONE},
	{SENSOR_SERIES_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_series_get, SENSOR_SERIES_STATUS},
	{SENSOR_SERIES_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_series_status, STATUS_NONE},
	{SENSOR_CANDECE_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_cadence_get, SENSOR_CANDECE_STATUS},
	{SENSOR_CANDECE_SET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_SETUP_S, mesh_cmd_sig_sensor_cadence_set, SENSOR_CANDECE_STATUS},
	{SENSOR_CANDECE_SET_NOACK, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_SETUP_S, mesh_cmd_sig_sensor_cadence_set, STATUS_NONE},
	{SENSOR_CANDECE_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_cadence_status, STATUS_NONE},
	{SENSOR_SETTINGS_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_settings_get, SENSOR_SETTINGS_STATUS},
	{SENSOR_SETTINGS_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_settings_status, STATUS_NONE},
	{SENSOR_SETTING_GET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_S, mesh_cmd_sig_sensor_setting_get, SENSOR_SETTING_STATUS},
	{SENSOR_SETTING_SET, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_SETUP_S, mesh_cmd_sig_sensor_setting_set, SENSOR_SETTING_STATUS},
	{SENSOR_SETTING_SET_NOACK, 0, SIG_MD_SENSOR_C, SIG_MD_SENSOR_SETUP_S, mesh_cmd_sig_sensor_setting_set, STATUS_NONE},
	{SENSOR_SETTING_STATUS, 1, SIG_MD_SENSOR_S, SIG_MD_SENSOR_C, mesh_cmd_sig_sensor_setting_status, STATUS_NONE},
#endif
    // ----- mesh ota
#if MD_MESH_OTA_EN
    {FW_UPDATE_INFO_GET, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_info_get, FW_UPDATE_INFO_STATUS},
    {FW_UPDATE_INFO_STATUS, 1, SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C, mesh_cmd_sig_fw_update_info_status, STATUS_NONE},
    {FW_UPDATE_METADATA_CHECK, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_metadata_check, FW_UPDATE_METADATA_CHECK_STATUS},
    {FW_UPDATE_METADATA_CHECK_STATUS, 1, SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C, mesh_cmd_sig_fw_update_metadata_check_status, STATUS_NONE},
    {FW_UPDATE_GET, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_get, FW_UPDATE_STATUS},
    {FW_UPDATE_START, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_start, FW_UPDATE_STATUS},
    {FW_UPDATE_CANCEL, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_cancel, FW_UPDATE_STATUS},
    {FW_UPDATE_APPLY, 0, SIG_MD_FW_UPDATE_C, SIG_MD_FW_UPDATE_S, mesh_cmd_sig_fw_update_apply, FW_UPDATE_STATUS},
    {FW_UPDATE_STATUS, 1, SIG_MD_FW_UPDATE_S, SIG_MD_FW_UPDATE_C, mesh_cmd_sig_fw_update_status, STATUS_NONE},
    #if __PROJECT_MESH_PRO__
    {FW_DISTRIBUT_GET, 0, SIG_MD_FW_DISTRIBUT_C, SIG_MD_FW_DISTRIBUT_S, mesh_cmd_sig_fw_distribut_get, FW_DISTRIBUT_STATUS},
    {FW_DISTRIBUT_START, 0, SIG_MD_FW_DISTRIBUT_C, SIG_MD_FW_DISTRIBUT_S, mesh_cmd_sig_fw_distribut_start, FW_DISTRIBUT_STATUS},
    {FW_DISTRIBUT_CANCEL, 0, SIG_MD_FW_DISTRIBUT_C, SIG_MD_FW_DISTRIBUT_S, mesh_cmd_sig_fw_distribut_cancel, FW_DISTRIBUT_STATUS},
    {FW_DISTRIBUT_STATUS, 1, SIG_MD_FW_DISTRIBUT_S, SIG_MD_FW_DISTRIBUT_C, mesh_cmd_sig_fw_distribut_status, STATUS_NONE},
    {FW_DISTRIBUT_DETAIL_GET, 0, SIG_MD_FW_DISTRIBUT_C, SIG_MD_FW_DISTRIBUT_S, mesh_cmd_sig_fw_distribut_detail_get, FW_DISTRIBUT_DETAIL_LIST},
    {FW_DISTRIBUT_DETAIL_LIST, 1, SIG_MD_FW_DISTRIBUT_S, SIG_MD_FW_DISTRIBUT_C, mesh_cmd_sig_fw_distribut_detail_list, STATUS_NONE},
    #endif
    {BLOB_TRANSFER_GET, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_transfer_get, BLOB_TRANSFER_STATUS},
    {BLOB_TRANSFER_START, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_transfer_handle, BLOB_TRANSFER_STATUS},
    {BLOB_TRANSFER_CANCEL, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_transfer_handle, BLOB_TRANSFER_STATUS},
    {BLOB_TRANSFER_STATUS, 1, SIG_MD_BLOB_TRANSFER_S, SIG_MD_BLOB_TRANSFER_C, mesh_cmd_sig_blob_transfer_status, STATUS_NONE},
    {BLOB_BLOCK_START, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_block_start, BLOB_BLOCK_STATUS},
    {BLOB_CHUNK_TRANSFER, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_chunk_transfer, STATUS_NONE},
    {BLOB_BLOCK_GET, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_block_get, BLOB_BLOCK_STATUS},
    {BLOB_BLOCK_STATUS, 1, SIG_MD_BLOB_TRANSFER_S, SIG_MD_BLOB_TRANSFER_C, mesh_cmd_sig_blob_block_status, STATUS_NONE},
    {BLOB_INFO_GET, 0, SIG_MD_BLOB_TRANSFER_C, SIG_MD_BLOB_TRANSFER_S, mesh_cmd_sig_blob_info_get, BLOB_INFO_STATUS},
    {BLOB_INFO_STATUS, 1, SIG_MD_BLOB_TRANSFER_S, SIG_MD_BLOB_TRANSFER_C, mesh_cmd_sig_blob_info_status, STATUS_NONE},
#endif
    
	// lighting model
#if ((MD_LIGHTNESS_EN) && (LIGHT_TYPE_SEL != LIGHT_TYPE_POWER))
    {LIGHTNESS_GET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_get, LIGHTNESS_STATUS},
    {LIGHTNESS_SET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_set, LIGHTNESS_STATUS},
    {LIGHTNESS_SET_NOACK, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_set, STATUS_NONE},
	{LIGHTNESS_STATUS, 1, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, mesh_cmd_sig_lightness_status, STATUS_NONE},
	#if CMD_LINEAR_EN
	{LIGHTNESS_LINEAR_GET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_linear_get, LIGHTNESS_LINEAR_STATUS},
	{LIGHTNESS_LINEAR_SET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_linear_set, LIGHTNESS_LINEAR_STATUS},
	{LIGHTNESS_LINEAR_SET_NOACK, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_linear_set, STATUS_NONE},
	{LIGHTNESS_LINEAR_STATUS, 1, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, mesh_cmd_sig_lightness_linear_status, STATUS_NONE},
	#endif
	{LIGHTNESS_LAST_GET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_last_get, LIGHTNESS_LAST_STATUS},
	{LIGHTNESS_LAST_STATUS, 1, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, mesh_cmd_sig_lightness_last_status, STATUS_NONE},
	{LIGHTNESS_DEFULT_GET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_def_get, LIGHTNESS_DEFULT_STATUS},
	{LIGHTNESS_DEFULT_SET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_SETUP_S, mesh_cmd_sig_lightness_def_set, LIGHTNESS_DEFULT_STATUS},
	{LIGHTNESS_DEFULT_SET_NOACK, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_SETUP_S, mesh_cmd_sig_lightness_def_set, STATUS_NONE},
	{LIGHTNESS_DEFULT_STATUS, 1, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, mesh_cmd_sig_lightness_def_status, STATUS_NONE},
	{LIGHTNESS_RANGE_GET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_S, mesh_cmd_sig_lightness_range_get, LIGHTNESS_RANGE_STATUS},
	{LIGHTNESS_RANGE_SET, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_SETUP_S, mesh_cmd_sig_lightness_range_set, LIGHTNESS_RANGE_STATUS},
	{LIGHTNESS_RANGE_SET_NOACK, 0, SIG_MD_LIGHTNESS_C, SIG_MD_LIGHTNESS_SETUP_S, mesh_cmd_sig_lightness_range_set, STATUS_NONE},
	{LIGHTNESS_RANGE_STATUS, 1, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_C, mesh_cmd_sig_lightness_range_status, STATUS_NONE},
#endif
	
#if MD_LIGHT_CONTROL_EN
    {LIGHT_LC_MODE_GET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_mode_get, LIGHT_LC_MODE_STATUS},
    {LIGHT_LC_MODE_SET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_mode_set, LIGHT_LC_MODE_STATUS},
    {LIGHT_LC_MODE_SET_NOACK, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_mode_set, STATUS_NONE},
	{LIGHT_LC_MODE_STATUS, 1, SIG_MD_LIGHT_LC_S, SIG_MD_LIGHT_LC_C, mesh_cmd_sig_lc_mode_status, STATUS_NONE},
	{LIGHT_LC_OM_GET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_om_get, LIGHT_LC_OM_STATUS},
	{LIGHT_LC_OM_SET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_om_set, LIGHT_LC_OM_STATUS},
	{LIGHT_LC_OM_SET_NOACK, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_om_set, STATUS_NONE},
	{LIGHT_LC_OM_STATUS, 1, SIG_MD_LIGHT_LC_S, SIG_MD_LIGHT_LC_C, mesh_cmd_sig_lc_om_status, STATUS_NONE},
	{LIGHT_LC_ONOFF_GET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_onoff_get, LIGHT_LC_ONOFF_STATUS},
	{LIGHT_LC_ONOFF_SET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_onoff_set, LIGHT_LC_ONOFF_STATUS},
	{LIGHT_LC_ONOFF_SET_NOACK, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_S, mesh_cmd_sig_lc_onoff_set, STATUS_NONE},
	{LIGHT_LC_ONOFF_STATUS, 1, SIG_MD_LIGHT_LC_S, SIG_MD_LIGHT_LC_C, mesh_cmd_sig_lc_onoff_status, STATUS_NONE},
	{LIGHT_LC_PROPERTY_GET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_SETUP_S, mesh_cmd_sig_lc_prop_get, LIGHT_LC_PROPERTY_STATUS},
	{LIGHT_LC_PROPERTY_SET, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_SETUP_S, mesh_cmd_sig_lc_prop_set, LIGHT_LC_PROPERTY_STATUS},
	{LIGHT_LC_PROPERTY_SET_NOACK, 0, SIG_MD_LIGHT_LC_C, SIG_MD_LIGHT_LC_SETUP_S, mesh_cmd_sig_lc_prop_set, STATUS_NONE},
	{LIGHT_LC_PROPERTY_STATUS, 1, SIG_MD_LIGHT_LC_SETUP_S, SIG_MD_LIGHT_LC_C, mesh_cmd_sig_lc_prop_status, STATUS_NONE},
#endif

#if (LIGHT_TYPE_CT_EN)
    {LIGHT_CTL_GET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_S, mesh_cmd_sig_light_ctl_get, LIGHT_CTL_STATUS},
    {LIGHT_CTL_SET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_S, mesh_cmd_sig_light_ctl_set, LIGHT_CTL_STATUS},
    {LIGHT_CTL_SET_NOACK, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_S, mesh_cmd_sig_light_ctl_set, STATUS_NONE},
	{LIGHT_CTL_STATUS, 1, SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_C, mesh_cmd_sig_light_ctl_status, STATUS_NONE},
	{LIGHT_CTL_DEFULT_GET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_S, mesh_cmd_sig_light_ctl_def_get, LIGHT_CTL_DEFULT_STATUS},
	{LIGHT_CTL_DEFULT_SET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_SETUP_S, mesh_cmd_sig_light_ctl_def_set, LIGHT_CTL_DEFULT_STATUS},
	{LIGHT_CTL_DEFULT_SET_NOACK, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_SETUP_S, mesh_cmd_sig_light_ctl_def_set, STATUS_NONE},
	{LIGHT_CTL_DEFULT_STATUS, 1, SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_C, mesh_cmd_sig_light_ctl_def_status, STATUS_NONE},
	{LIGHT_CTL_TEMP_RANGE_GET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_S, mesh_cmd_sig_light_ctl_temp_range_get, LIGHT_CTL_TEMP_RANGE_STATUS},
	{LIGHT_CTL_TEMP_RANGE_SET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_SETUP_S, mesh_cmd_sig_light_ctl_temp_range_set, LIGHT_CTL_TEMP_RANGE_STATUS},
	{LIGHT_CTL_TEMP_RANGE_SET_NOACK, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_SETUP_S, mesh_cmd_sig_light_ctl_temp_range_set, STATUS_NONE},
	{LIGHT_CTL_TEMP_RANGE_STATUS, 1, SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_C, mesh_cmd_sig_light_ctl_temp_range_status, STATUS_NONE},
	{LIGHT_CTL_TEMP_GET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_TEMP_S, mesh_cmd_sig_light_ctl_temp_get, LIGHT_CTL_TEMP_STATUS},
	{LIGHT_CTL_TEMP_SET, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_TEMP_S, mesh_cmd_sig_light_ctl_temp_set, LIGHT_CTL_TEMP_STATUS},
	{LIGHT_CTL_TEMP_SET_NOACK, 0, SIG_MD_LIGHT_CTL_C, SIG_MD_LIGHT_CTL_TEMP_S, mesh_cmd_sig_light_ctl_temp_set, STATUS_NONE},
	{LIGHT_CTL_TEMP_STATUS, 1, SIG_MD_LIGHT_CTL_TEMP_S, SIG_MD_LIGHT_CTL_C, mesh_cmd_sig_light_ctl_temp_status, STATUS_NONE},
#endif
#if (LIGHT_TYPE_HSL_EN)
    {LIGHT_HSL_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_get, LIGHT_HSL_STATUS},
    {LIGHT_HSL_SET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_set, LIGHT_HSL_STATUS},
    {LIGHT_HSL_SET_NOACK, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_set, STATUS_NONE},
	{LIGHT_HSL_STATUS, 1, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_hsl_status, STATUS_NONE},
	{LIGHT_HSL_TARGET_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_target_get, LIGHT_HSL_TARGET_STATUS},
	{LIGHT_HSL_TARGET_STATUS, 1, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_hsl_target_status, STATUS_NONE},
	{LIGHT_HSL_HUE_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_HUE_S, mesh_cmd_sig_light_hue_get, LIGHT_HSL_HUE_STATUS},
	{LIGHT_HSL_HUE_SET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_HUE_S, mesh_cmd_sig_light_hue_set, LIGHT_HSL_HUE_STATUS},
	{LIGHT_HSL_HUE_SET_NOACK, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_HUE_S, mesh_cmd_sig_light_hue_set, STATUS_NONE},
	{LIGHT_HSL_HUE_STATUS, 1, SIG_MD_LIGHT_HSL_HUE_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_hue_status, STATUS_NONE},
	{LIGHT_HSL_SAT_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SAT_S, mesh_cmd_sig_light_sat_get, LIGHT_HSL_SAT_STATUS},
	{LIGHT_HSL_SAT_SET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SAT_S, mesh_cmd_sig_light_sat_set, LIGHT_HSL_SAT_STATUS},
	{LIGHT_HSL_SAT_SET_NOACK, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SAT_S, mesh_cmd_sig_light_sat_set, STATUS_NONE},
	{LIGHT_HSL_SAT_STATUS, 1, SIG_MD_LIGHT_HSL_SAT_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_sat_status, STATUS_NONE},
	{LIGHT_HSL_DEF_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_def_get, LIGHT_HSL_DEF_STATUS},
	{LIGHT_HSL_DEF_SET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SETUP_S, mesh_cmd_sig_light_hsl_def_set, LIGHT_HSL_DEF_STATUS},
	{LIGHT_HSL_DEF_SET_NOACK, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SETUP_S, mesh_cmd_sig_light_hsl_def_set, STATUS_NONE},
	{LIGHT_HSL_DEF_STATUS, 1, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_hsl_def_status, STATUS_NONE},
	{LIGHT_HSL_RANGE_GET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_S, mesh_cmd_sig_light_hsl_range_get, LIGHT_HSL_RANGE_STATUS},
	{LIGHT_HSL_RANGE_SET, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SETUP_S, mesh_cmd_sig_light_hsl_range_set, LIGHT_HSL_RANGE_STATUS},
	{LIGHT_HSL_RANGE_SET_NOACK, 0, SIG_MD_LIGHT_HSL_C, SIG_MD_LIGHT_HSL_SETUP_S, mesh_cmd_sig_light_hsl_range_set, STATUS_NONE},
	{LIGHT_HSL_RANGE_STATUS, 1, SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_C, mesh_cmd_sig_light_hsl_range_status, STATUS_NONE},
#endif
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
    {LIGHT_XYL_GET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_get, LIGHT_XYL_STATUS},
    {LIGHT_XYL_SET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_set, LIGHT_XYL_STATUS},
    {LIGHT_XYL_SET_NOACK, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_set, STATUS_NONE},
	{LIGHT_XYL_STATUS, 1, SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_C, mesh_cmd_sig_light_xyl_status, STATUS_NONE},
	{LIGHT_XYL_TARGET_GET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_target_get, LIGHT_XYL_TARGET_STATUS},
	{LIGHT_XYL_TARGET_STATUS, 1, SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_C, mesh_cmd_sig_light_xyl_target_status, STATUS_NONE},
	{LIGHT_XYL_DEF_GET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_def_get, LIGHT_XYL_DEF_STATUS},
	{LIGHT_XYL_DEF_SET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_SETUP_S, mesh_cmd_sig_light_xyl_def_set, LIGHT_XYL_DEF_STATUS},
	{LIGHT_XYL_DEF_SET_NOACK, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_SETUP_S, mesh_cmd_sig_light_xyl_def_set, STATUS_NONE},
	{LIGHT_XYL_DEF_STATUS, 1, SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_C, mesh_cmd_sig_light_xyl_def_status, STATUS_NONE},
	{LIGHT_XYL_RANGE_GET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_S, mesh_cmd_sig_light_xyl_range_get, LIGHT_XYL_RANGE_STATUS},
	{LIGHT_XYL_RANGE_SET, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_SETUP_S, mesh_cmd_sig_light_xyl_range_set, LIGHT_XYL_RANGE_STATUS},
	{LIGHT_XYL_RANGE_SET_NOACK, 0, SIG_MD_LIGHT_XYL_C, SIG_MD_LIGHT_XYL_SETUP_S, mesh_cmd_sig_light_xyl_range_set, STATUS_NONE},
	{LIGHT_XYL_RANGE_STATUS, 1, SIG_MD_LIGHT_XYL_S, SIG_MD_LIGHT_XYL_C, mesh_cmd_sig_light_xyl_range_status, STATUS_NONE},
#endif
#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
	{G_POWER_LEVEL_GET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_get, G_POWER_LEVEL_STATUS},
	{G_POWER_LEVEL_SET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_set, G_POWER_LEVEL_STATUS},
	{G_POWER_LEVEL_SET_NOACK, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_set, STATUS_NONE},
	{G_POWER_LEVEL_STATUS, 1, SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_C, mesh_cmd_sig_g_power_status, STATUS_NONE},
	{G_POWER_LEVEL_LAST_GET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_last_get, G_POWER_LEVEL_LAST_STATUS},
	{G_POWER_LEVEL_LAST_STATUS, 1, SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_C, mesh_cmd_sig_g_power_last_status, STATUS_NONE},
	
	{G_POWER_DEF_GET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_def_get, G_POWER_DEF_STATUS},
	{G_POWER_DEF_STATUS, 1, SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_C, mesh_cmd_sig_g_power_def_status, STATUS_NONE},
	{G_POWER_LEVEL_RANGE_GET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_S, mesh_cmd_sig_g_power_range_get, G_POWER_LEVEL_RANGE_STATUS},
	{G_POWER_LEVEL_RANGE_STATUS, 1, SIG_MD_G_POWER_LEVEL_S, SIG_MD_G_POWER_LEVEL_C, mesh_cmd_sig_g_power_range_status, STATUS_NONE},
	
	{G_POWER_DEF_SET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_SETUP_S, mesh_cmd_sig_g_power_def_set, G_POWER_DEF_STATUS},
	{G_POWER_DEF_SET_NOACK, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_SETUP_S, mesh_cmd_sig_g_power_def_set, STATUS_NONE},
	{G_POWER_LEVEL_RANGE_SET, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_SETUP_S, mesh_cmd_sig_g_power_range_set, G_POWER_LEVEL_RANGE_STATUS},
	{G_POWER_LEVEL_RANGE_SET_NOACK, 0, SIG_MD_G_POWER_LEVEL_C, SIG_MD_G_POWER_LEVEL_SETUP_S, mesh_cmd_sig_g_power_range_set, STATUS_NONE},
#endif

#if (MD_PROPERTY_EN)
    {G_USER_PROPERTIES_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_USER_PROP_S, mesh_cmd_sig_properties_get, G_USER_PROPERTIES_STATUS},
    {G_USER_PROPERTIES_STATUS, 1, SIG_MD_G_USER_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_properties_status, STATUS_NONE},
    {G_ADMIN_PROPERTIES_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_ADMIN_PROP_S, mesh_cmd_sig_properties_get, G_ADMIN_PROPERTIES_STATUS},
    {G_ADMIN_PROPERTIES_STATUS, 1, SIG_MD_G_ADMIN_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_properties_status, STATUS_NONE},
    {G_MFG_PROPERTIES_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_MFG_PROP_S, mesh_cmd_sig_properties_get, G_MFG_PROPERTIES_STATUS},
    {G_MFG_PROPERTIES_STATUS, 1, SIG_MD_G_MFG_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_properties_status, STATUS_NONE},
    {G_CLIENT_PROPERTIES_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_CLIENT_PROP_S, mesh_cmd_sig_properties_get, G_CLIENT_PROPERTIES_STATUS},
    {G_CLIENT_PROPERTIES_STATUS, 1, SIG_MD_G_CLIENT_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_properties_status, STATUS_NONE},
    {G_USER_PROPERTY_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_USER_PROP_S, mesh_cmd_sig_property_get, G_USER_PROPERTY_STATUS},
    {G_USER_PROPERTY_SET, 0, SIG_MD_G_PROP_C, SIG_MD_G_USER_PROP_S, mesh_cmd_sig_property_set, G_USER_PROPERTY_STATUS},
    {G_USER_PROPERTY_SET_NOACK, 0, SIG_MD_G_PROP_C, SIG_MD_G_USER_PROP_S, mesh_cmd_sig_property_set, STATUS_NONE},
    {G_USER_PROPERTY_STATUS, 1, SIG_MD_G_USER_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_property_status, STATUS_NONE},
    {G_ADMIN_PROPERTY_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_ADMIN_PROP_S, mesh_cmd_sig_property_get, G_ADMIN_PROPERTY_STATUS},
    {G_ADMIN_PROPERTY_SET, 0, SIG_MD_G_PROP_C, SIG_MD_G_ADMIN_PROP_S, mesh_cmd_sig_property_set, G_ADMIN_PROPERTY_STATUS},
    {G_ADMIN_PROPERTY_SET_NOACK, 0, SIG_MD_G_PROP_C, SIG_MD_G_ADMIN_PROP_S, mesh_cmd_sig_property_set, STATUS_NONE},
    {G_ADMIN_PROPERTY_STATUS, 1, SIG_MD_G_ADMIN_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_property_status, STATUS_NONE},
    {G_MFG_PROPERTY_GET, 0, SIG_MD_G_PROP_C, SIG_MD_G_MFG_PROP_S, mesh_cmd_sig_property_get, G_MFG_PROPERTY_STATUS},
    {G_MFG_PROPERTY_SET, 0, SIG_MD_G_PROP_C, SIG_MD_G_MFG_PROP_S, mesh_cmd_sig_property_set, G_MFG_PROPERTY_STATUS},
    {G_MFG_PROPERTY_SET_NOACK, 0, SIG_MD_G_PROP_C, SIG_MD_G_MFG_PROP_S, mesh_cmd_sig_property_set, STATUS_NONE},
    {G_MFG_PROPERTY_STATUS, 1, SIG_MD_G_MFG_PROP_S, SIG_MD_G_PROP_C, mesh_cmd_sig_property_status, STATUS_NONE},
#endif
};

/**************
mesh_search_model_id_by_op():
get model id, call back function, status command flag
*/
int mesh_search_model_id_by_op(mesh_op_resource_t *op_res, u16 op, u8 tx_flag)
{
    memset(op_res, 0, sizeof(mesh_op_resource_t));
    
    u8 op_type = GET_OP_TYPE(op);
    if((OP_TYPE_VENDOR != op_type) || DRAFT_FEAT_VD_MD_EN){ // some vd op of draft feat is defined in sig func.
        foreach_arr(i,mesh_cmd_sig_func){
            if(op == mesh_cmd_sig_func[i].op){
                op_res->cb = mesh_cmd_sig_func[i].cb;
                op_res->op_rsp = mesh_cmd_sig_func[i].op_rsp;
                op_res->sig = (OP_TYPE_VENDOR == op_type) ? 0 : 1;
                op_res->status_cmd = mesh_cmd_sig_func[i].status_cmd ? 1 : 0;
                if(tx_flag){
                    op_res->id = mesh_cmd_sig_func[i].model_id_tx;
                }else{
                    #if (MD_CLIENT_EN == 0)
                    if(TIME_STATUS == op){
                        op_res->id = SIG_MD_TIME_S; // both server and client model can support time status.
                    }else
                    #endif
                    {
                    op_res->id = mesh_cmd_sig_func[i].model_id_rx;
                    }
                }
                return 0;
            }
        }
    }

    if(OP_TYPE_VENDOR == op_type){
#if (DUAL_VENDOR_EN)
        if(VENDOR_ID_MI == g_vendor_id){
            return mi_mesh_search_model_id_by_op_vendor(op_res, op, tx_flag);
        }else{
            return mesh_search_model_id_by_op_vendor(op_res, op, tx_flag);
        }
#elif (VENDOR_MD_MI_EN)
        return mi_mesh_search_model_id_by_op_vendor(op_res, op, tx_flag);
#else
        return mesh_search_model_id_by_op_vendor(op_res, op, tx_flag);
#endif
    }

    return -1;
}

int is_cmd_with_tid(u8 *tid_pos_out, u16 op, u8 *par, u8 tid_pos_vendor_app)
{
    int cmd_with_tid = 0;

    if(IS_VENDOR_OP(op)){
#if (DUAL_VENDOR_EN)
        if(VENDOR_ID_MI == g_vendor_id){
            cmd_with_tid = is_mi_cmd_with_tid_vendor(tid_pos_out, op, tid_pos_vendor_app);
        }else{
            cmd_with_tid = is_cmd_with_tid_vendor(tid_pos_out, op, par, tid_pos_vendor_app);
        }
#elif (VENDOR_MD_MI_EN)
        cmd_with_tid = is_mi_cmd_with_tid_vendor(tid_pos_out, op, tid_pos_vendor_app);
#else
        cmd_with_tid = is_cmd_with_tid_vendor(tid_pos_out, op, par, tid_pos_vendor_app);
#endif
    }else{
        switch(op){
            default:
                break;
                
            case G_ONOFF_SET:
            case G_ONOFF_SET_NOACK:
            case LIGHT_LC_ONOFF_SET:
            case LIGHT_LC_ONOFF_SET_NOACK:
                cmd_with_tid = 1;
                *tid_pos_out = OFFSETOF(mesh_cmd_g_onoff_set_t, tid);
                break;
                
            case G_LEVEL_SET:
            case G_LEVEL_SET_NOACK:
            case G_MOVE_SET:
            case G_MOVE_SET_NOACK:
            case G_POWER_LEVEL_SET:
            case G_POWER_LEVEL_SET_NOACK:
            case SCENE_RECALL:
            case SCENE_RECALL_NOACK:
            case LIGHTNESS_SET:
            case LIGHTNESS_SET_NOACK:
            case LIGHTNESS_LINEAR_SET:
            case LIGHTNESS_LINEAR_SET_NOACK:
            case LIGHT_HSL_HUE_SET:
            case LIGHT_HSL_HUE_SET_NOACK:
            case LIGHT_HSL_SAT_SET:
            case LIGHT_HSL_SAT_SET_NOACK:
                cmd_with_tid = 1;
                *tid_pos_out = 2; // OFFSETOF(mesh_cmd_g_level_set_t, tid);
                break;
                
            case G_DELTA_SET:
            case G_DELTA_SET_NOACK:
            case LIGHT_CTL_TEMP_SET:
            case LIGHT_CTL_TEMP_SET_NOACK:
                cmd_with_tid = 1;
                *tid_pos_out = 4; // OFFSETOF(mesh_cmd_g_level_delta_t, tid);
                break;
                
            case LIGHT_CTL_SET:
            case LIGHT_CTL_SET_NOACK:
    		case LIGHT_HSL_SET:
    		case LIGHT_HSL_SET_NOACK:
    		case LIGHT_XYL_SET:
    		case LIGHT_XYL_SET_NOACK:
                cmd_with_tid = 1;
                *tid_pos_out = 6; // OFFSETOF(mesh_cmd_light_ctl_set_t, tid);
                break;
        }
    }
    
    return cmd_with_tid;
}

u32 get_op_rsp(u16 op)
{
    mesh_op_resource_t op_res;
    if(0 == mesh_search_model_id_by_op(&op_res, op, 1)){
        return op_res.op_rsp;
    }
    return STATUS_NONE;
}

int is_reliable_cmd(u16 op, u32 vd_op_rsp)
{
    #if VC_SUPPORT_ANY_VENDOR_CMD_EN
    if(IS_VENDOR_OP(op)){
        return ((vd_op_rsp & 0xff) != STATUS_NONE_VENDOR_OP_VC);
    }
    #endif
    
    return (STATUS_NONE != get_op_rsp(op));
}


// ----------
#define GET_ARRAR_MODEL_AND_COUNT(model, cb_pub)    (u8 *)&model, cb_pub, sizeof(model[0]), ARRAY_SIZE(model), 1
#define GET_SINGLE_MODEL_AND_COUNT(model, cb_pub)   (u8 *)&model, cb_pub, sizeof(model),    1,                 0

const mesh_model_resource_t MeshSigModelResource[] = {
    {SIG_MD_CFG_SERVER, GET_SINGLE_MODEL_AND_COUNT(model_sig_cfg_s, 0)},
#if MD_CFG_CLIENT_EN
    {SIG_MD_CFG_CLIENT, GET_SINGLE_MODEL_AND_COUNT(model_sig_cfg_c, 0)},
#endif
    {SIG_MD_HEALTH_SERVER, GET_SINGLE_MODEL_AND_COUNT(model_sig_health.srv, &mesh_health_cur_sts_publish)},  // change to multy element model later. 
    {SIG_MD_HEALTH_CLIENT, GET_SINGLE_MODEL_AND_COUNT(model_sig_health.clnt, 0)},  // change to multy element model later. 
#if MD_DF_EN
    #if MD_SERVER_EN
    {SIG_MD_DF_CFG_S, GET_SINGLE_MODEL_AND_COUNT(model_sig_g_df_sbr_cfg.df_cfg.srv, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_DF_CFG_C, GET_SINGLE_MODEL_AND_COUNT(model_sig_g_df_sbr_cfg.df_cfg.clnt, 0)},
    #endif
#endif

#if MD_SBR_EN
    #if MD_SERVER_EN
    {SIG_MD_BRIDGE_CFG_SERVER, GET_SINGLE_MODEL_AND_COUNT(model_sig_g_df_sbr_cfg.bridge_cfg.srv, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_BRIDGE_CFG_CLIENT, GET_SINGLE_MODEL_AND_COUNT(model_sig_g_df_sbr_cfg.bridge_cfg.clnt, 0)},
    #endif
#endif

#if MD_MESH_OTA_EN
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
    {SIG_MD_FW_DISTRIBUT_S, GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.fw_distr_srv, 0)},
    {SIG_MD_FW_DISTRIBUT_C, GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.fw_distr_clnt, 0)},
    {SIG_MD_FW_UPDATE_C,    GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.fw_update_clnt, 0)},
    {SIG_MD_BLOB_TRANSFER_C, GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.blob_trans_clnt, 0)},
    #endif
    #if MD_SERVER_EN
    {SIG_MD_FW_UPDATE_S, GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.fw_update_srv, 0)},
    {SIG_MD_BLOB_TRANSFER_S, GET_SINGLE_MODEL_AND_COUNT(model_mesh_ota.blob_trans_srv, 0)},
    #endif
#endif

#if MD_REMOTE_PROV
    #if MD_SERVER_EN
    {SIG_MD_REMOTE_PROV_SERVER, GET_ARRAR_MODEL_AND_COUNT(model_remote_prov.srv, &mesh_remote_prov_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_REMOTE_PROV_CLIENT, GET_ARRAR_MODEL_AND_COUNT(model_remote_prov.client, 0)},
    #endif
#endif

#if MD_PRIVACY_BEA
    #if MD_SERVER_EN
    {SIG_MD_PRIVATE_BEACON_SERVER, GET_ARRAR_MODEL_AND_COUNT(model_private_beacon.srv, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_PRIVATE_BEACON_CLIENT, GET_ARRAR_MODEL_AND_COUNT(model_private_beacon.client, 0)},
    #endif
#endif

#if MD_SERVER_EN
    {SIG_MD_G_ONOFF_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_onoff_level.onoff_srv, &mesh_g_onoff_st_publish)},
#endif
#if MD_CLIENT_EN
    {SIG_MD_G_ONOFF_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_onoff_level.onoff_clnt, 0)},
#endif

#if MD_LEVEL_EN
    #if MD_SERVER_EN
    {SIG_MD_G_LEVEL_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_onoff_level.level_srv, &mesh_g_level_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_LEVEL_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_onoff_level.level_clnt, 0)},
    #endif
#endif

#if MD_DEF_TRANSIT_TIME_EN
    #if MD_SERVER_EN
    {SIG_MD_G_DEF_TRANSIT_TIME_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_power_onoff.def_trans_time_srv, &mesh_def_trans_time_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_DEF_TRANSIT_TIME_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_power_onoff.def_trans_time_clnt, 0)},
    #endif
#endif

#if MD_POWER_ONOFF_EN
    #if MD_SERVER_EN
    {SIG_MD_G_POWER_ONOFF_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_power_onoff.pw_onoff_srv, &mesh_on_powerup_st_publish)},
    {SIG_MD_G_POWER_ONOFF_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_power_onoff.pw_onoff_setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_POWER_ONOFF_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_g_power_onoff.pw_onoff_clnt, 0)},
    #endif
#endif

#if MD_SCENE_EN
    #if MD_SERVER_EN
    {SIG_MD_SCENE_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_scene.srv, &mesh_scene_st_publish)},
    {SIG_MD_SCENE_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_scene.setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_SCENE_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_scene.clnt, 0)},
    #endif
#endif

#if MD_TIME_EN
    #if MD_SERVER_EN
    {SIG_MD_TIME_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.time_srv, &mesh_time_st_publish)},
    {SIG_MD_TIME_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.time_setup, &mesh_time_role_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_TIME_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.time_clnt, 0)},
    #endif
#endif

#if (MD_SCHEDULE_EN)
    #if MD_SERVER_EN
    {SIG_MD_SCHED_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.sch_srv, 0)},
    {SIG_MD_SCHED_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.sch_setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_SCHED_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_time_schedule.sch_clnt, 0)},
    #endif
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_POWER)
    #if MD_SERVER_EN
    {SIG_MD_G_POWER_LEVEL_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.srv, &mesh_g_power_st_publish)},
    {SIG_MD_G_POWER_LEVEL_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_POWER_LEVEL_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.clnt, 0)},
    #endif
#elif MD_LIGHTNESS_EN
    #if MD_SERVER_EN
    {SIG_MD_LIGHTNESS_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.srv, &mesh_lightness_st_publish)},
    {SIG_MD_LIGHTNESS_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_LIGHTNESS_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_lightness.clnt, 0)},
    #endif
#endif

#if MD_LIGHT_CONTROL_EN
    #if MD_SERVER_EN
    {SIG_MD_LIGHT_LC_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_lc.srv, &mesh_lc_onoff_st_publish)},
    {SIG_MD_LIGHT_LC_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_lc.setup, &mesh_lc_prop_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_LIGHT_LC_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_lc.clnt, 0)},
    #endif
#endif

#if (LIGHT_TYPE_CT_EN)
    #if MD_SERVER_EN
    {SIG_MD_LIGHT_CTL_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_ctl.srv, &mesh_light_ctl_st_publish)},
    {SIG_MD_LIGHT_CTL_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_ctl.setup, 0)},
    {SIG_MD_LIGHT_CTL_TEMP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_ctl.temp, &mesh_light_ctl_temp_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_LIGHT_CTL_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_ctl.clnt, 0)},
    #endif
#endif

#if (LIGHT_TYPE_HSL_EN)
    #if MD_SERVER_EN
    {SIG_MD_LIGHT_HSL_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_hsl.srv, &mesh_light_hsl_st_publish)},
    {SIG_MD_LIGHT_HSL_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_hsl.setup, 0)},
    {SIG_MD_LIGHT_HSL_HUE_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_hsl.hue, &mesh_light_hue_st_publish)},
    {SIG_MD_LIGHT_HSL_SAT_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_hsl.sat, &mesh_light_sat_st_publish)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_LIGHT_HSL_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_hsl.clnt, 0)},
    #endif
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
    #if MD_SERVER_EN
    {SIG_MD_LIGHT_XYL_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_xyl.srv, &mesh_light_xyl_st_publish)},
    {SIG_MD_LIGHT_XYL_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_xyl.setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_LIGHT_XYL_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_light_xyl.clnt, 0)},
    #endif
#endif

#if MD_SENSOR_EN
    #if MD_SENSOR_SERVER_EN
    {SIG_MD_SENSOR_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.sensor_srv, &mesh_sensor_st_publish)},
    {SIG_MD_SENSOR_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.sensor_setup, &mesh_sensor_setup_st_publish)},
    #endif
    #if MD_SENSOR_CLIENT_EN
    {SIG_MD_SENSOR_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.sensor_clnt, 0)},
    #endif
#endif

#if MD_BATTERY_EN
    #if MD_SERVER_EN
    {SIG_MD_G_BAT_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.battery_srv, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_BAT_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.battery_clnt, 0)},
    #endif
#endif

#if MD_LOCATION_EN
    #if MD_SERVER_EN
    {SIG_MD_G_LOCATION_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.location_srv, 0)},
    {SIG_MD_G_LOCATION_SETUP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.location_setup, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_LOCATION_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_sensor.location_clnt, 0)},
    #endif
#endif

#if MD_PROPERTY_EN
    #if MD_SERVER_EN
    {SIG_MD_G_ADMIN_PROP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_property.admin_srv, &mesh_property_st_publish_admin)},
    {SIG_MD_G_MFG_PROP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_property.mfg_srv, &mesh_property_st_publish_mfg)},
    {SIG_MD_G_USER_PROP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_property.user_srv, &mesh_property_st_publish_user)},
    {SIG_MD_G_CLIENT_PROP_S, GET_ARRAR_MODEL_AND_COUNT(model_sig_property.client_srv, 0)},
    #endif
    #if MD_CLIENT_EN
    {SIG_MD_G_PROP_C, GET_ARRAR_MODEL_AND_COUNT(model_sig_property.clnt, 0)},
    #endif
#endif
};

#if 0
const mesh_vendor_model_resource_t MeshVendorModelResource[] = {
    // because Vendor Model ID and publish is not fixed at some time, so not use array now.
};
#endif

#define MODEL_ELE_ADR_INIT(model)	\
{									\
    foreach_arr(i,model){			\
    	model[i].com.ele_adr = 0;	\
    }								\
}

#define MODEL_PUB_ST_CB_INIT(model, cb)	\
{									\
    foreach_arr(i,model){			\
    	model[i].com.cb_pub_st = cb;	\
    	model[i].com.cb_tick_ms = clock_time_ms();	\
        model[i].com.pub_trans_flag = 0;\
        model[i].com.pub_2nd_state = 0; \
    }								\
}

/*because there is no multipy element now*/
#define MODEL_PUB_ST_CB_INIT_HEALTH(model, cb)	\
{									\
	model.com.cb_pub_st = cb;	\
	model.com.cb_tick_ms = clock_time_ms();	\
	model.com.pub_trans_flag = 0;\
	model.com.pub_2nd_state = 0; \
}

// (0 == model[i].com.ele_adr) means init state.
#define CASE_BREAK_find_ele_resource(res,model_id,model)  \
    case model_id:\
        foreach_arr(i,model){\
            if((ele_adr == model[i].com.ele_adr)||(set_flag && (0 == model[i].com.ele_adr))){\
                res = (u8 *)(&model[i]);\
                *idx_out = i;\
                break;\
            }\
        }\
    break;

#define IF_find_ele_resource(res,model_id_in,model)  \
    if(model_id == model_id_in){\
        foreach_arr(i,model){\
            if((ele_adr == model[i].com.ele_adr)||(set_flag && (0 == model[i].com.ele_adr))){\
                res = (u8 *)(&model[i]);\
                *idx_out = i;\
                break;\
            }\
        }\
    }

u8* mesh_find_ele_resource_in_model(u16 ele_adr, u32 model_id, bool4 sig_model, u8 *idx_out, int set_flag)
{
    u8 *p_model = 0;
    *idx_out = 0;
    if(sig_model || DRAFT_FEAT_VD_MD_EN) // some vd op of draft feat is defined in sig func.
    {
        foreach_arr(m,MeshSigModelResource){
            const mesh_model_resource_t *p_source = &MeshSigModelResource[m];
            if(p_source->model_id == model_id){
                if(p_source->multy_flag){
                    foreach(i,p_source->model_cnt){
                        // member of 'com' always at the first place of p_source->p_model.
                        model_common_t *p_com = (model_common_t *)((u8 *)p_source->p_model + p_source->size * i);
                        if((ele_adr == p_com->ele_adr)||(set_flag && (0 == p_com->ele_adr))){
                            p_model = (u8 *)p_com;
                            *idx_out = i;
                            break;
                        }
                    }
                }else{
                    if(ele_adr == ele_adr_primary){
                        p_model = p_source->p_model;
                    }
                }
                break;
            }
        }
    }

    if(0 == sig_model){
#if WIN32
    #if MD_SERVER_EN
        IF_find_ele_resource(p_model,g_vendor_md_light_vc_s,model_vd_light.srv);
            #if MD_VENDOR_2ND_EN
        IF_find_ele_resource(p_model,g_vendor_md_light_vc_s2,model_vd_light.srv2);
            #endif
    #endif
    #if MD_CLIENT_VENDOR_EN
        IF_find_ele_resource(p_model,g_vendor_md_light_vc_c,model_vd_light.clnt);
    #endif
#else
    #if (VENDOR_MD_MI_EN)
        u32 model_vd_id_srv = MIOT_SEPC_VENDOR_MODEL_SER;
        u32 model_vd_id_srv2 = MIOT_VENDOR_MD_SER;
        #if MD_CLIENT_VENDOR_EN
        u32 model_vd_id_clnt = MIOT_SEPC_VENDOR_MODEL_CLI;
        #endif
        #if (DUAL_VENDOR_EN)
        if(DUAL_VENDOR_ST_ALI == provision_mag.dual_vendor_st){
            model_vd_id_srv = VENDOR_MD_LIGHT_S;
            model_vd_id_srv2 = VENDOR_MD_LIGHT_S2;
            #if MD_CLIENT_VENDOR_EN
            model_vd_id_clnt = VENDOR_MD_LIGHT_C;
            #endif
        }
        #endif
    #else
        #if MD_SERVER_EN
        u32 model_vd_id_srv = VENDOR_MD_LIGHT_S;
            #if MD_VENDOR_2ND_EN
        u32 model_vd_id_srv2 = VENDOR_MD_LIGHT_S2;
            #endif
        #endif
        #if MD_CLIENT_VENDOR_EN
        u32 model_vd_id_clnt = VENDOR_MD_LIGHT_C;
        #endif
    #endif
    
    #if MD_SERVER_EN
        IF_find_ele_resource(p_model,model_vd_id_srv,model_vd_light.srv);
        #if MD_VENDOR_2ND_EN
        IF_find_ele_resource(p_model,model_vd_id_srv2,model_vd_light.srv2);
        #endif
    #endif
    #if MD_CLIENT_VENDOR_EN
        IF_find_ele_resource(p_model,model_vd_id_clnt,model_vd_light.clnt);
    #endif
#endif
    }

    return p_model;
}

void mesh_model_ele_adr_init()
{
    foreach_arr(m,MeshSigModelResource){
        const mesh_model_resource_t *p_source = &MeshSigModelResource[m];
        if(p_source->multy_flag){
            foreach(i,p_source->model_cnt){
                // member of 'com' always at the first place of p_source->p_model.
                model_common_t *p_com = (model_common_t *)((u8 *)p_source->p_model + p_source->size * i);
                p_com->ele_adr = 0;
            }
        }else{
            // no need init
        }
    }
	
	#if MD_SERVER_EN
	MODEL_ELE_ADR_INIT(model_vd_light.srv);
        #if MD_VENDOR_2ND_EN
	MODEL_ELE_ADR_INIT(model_vd_light.srv2);
        #endif
	#endif
	#if MD_CLIENT_VENDOR_EN
	MODEL_ELE_ADR_INIT(model_vd_light.clnt);
	#endif
}

#if MD_SERVER_EN
    #if MD_LIGHT_CONTROL_EN
void model_pub_st_cb_re_init_lc_srv(cb_pub_st_t cb)
{
    foreach_arr(i,model_sig_light_lc.srv){
    	model_sig_light_lc.srv[i].com.cb_pub_st = cb;
    }
}
    #endif

    #if MD_SENSOR_SERVER_EN
void model_pub_st_cb_re_init_sensor_setup(cb_pub_st_t cb)
{
    foreach_arr(i,model_sig_sensor.sensor_setup){
    	model_sig_sensor.sensor_setup[i].com.cb_pub_st = cb;
    }
}
    #endif

void mesh_model_cb_pub_st_register()
{
    foreach_arr(m,MeshSigModelResource){
        const mesh_model_resource_t *p_source = &MeshSigModelResource[m];
        foreach(i,p_source->model_cnt){
            // member of 'com' always at the first place of p_source->p_model.
            model_common_t *p_com = (model_common_t *)((u8 *)p_source->p_model + p_source->size * i);
            p_com->cb_pub_st = p_source->cb_pub_st;
            p_com->cb_tick_ms = clock_time_ms();
            p_com->pub_trans_flag = 0;
            p_com->pub_2nd_state = 0;
        }
    }
    
#if MD_LIGHT_CONTROL_EN
	foreach_arr(i,model_sig_light_lc.lc_onoff_target){
        model_sig_light_lc.lc_onoff_target[i] = 0;    // init, not use data in flash. 
	}
#endif


    #if DEBUG_VENDOR_CMD_EN
    	#if LPN_VENDOR_SENSOR_EN
	MODEL_PUB_ST_CB_INIT(model_vd_light.srv, &cb_vd_lpn_sensor_st_publish);
    	#else
    	    #if (VENDOR_MD_MI_EN)
    MODEL_PUB_ST_CB_INIT(model_vd_light.srv, &vd_mi_proper_sts_publish);
    	    #else // (VENDOR_MD_NORMAL_EN)
    MODEL_PUB_ST_CB_INIT(model_vd_light.srv, &vd_light_onoff_st_publish);
            #endif
    	#endif
    	#if MD_VENDOR_2ND_EN
            #if (VENDOR_MD_MI_EN)
    MODEL_PUB_ST_CB_INIT(model_vd_light.srv2, &mi_vd_light_onoff_st_publish2);
            #else // (VENDOR_MD_NORMAL_EN)
	//MODEL_PUB_ST_CB_INIT(model_vd_light.srv2, &vd_light_onoff_st_publish2); // active later
	        #endif
    	#endif
    #endif
}

#if(DUAL_VENDOR_EN)
void vendor_md_cb_pub_st_set2ali()
{
    MODEL_PUB_ST_CB_INIT(model_vd_light.srv, &vd_light_onoff_st_publish);
    #if MD_VENDOR_2ND_EN
    MODEL_PUB_ST_CB_INIT(model_vd_light.srv2, 0); // active later: &vd_light_onoff_st_publish2
    #endif
}
#endif

void model_pub_check_set(int level_set_st, u8 *model, int priority)
{
    model_common_t *p_model = (model_common_t *)model;
#if MI_API_ENABLE
	if(!priority){// in the mi mode ,only priority==1 can allow to send pub cmd
		return ;
	}
#endif
    if(p_model->pub_adr){
        int pub_flag = 0;
        if(ST_G_LEVEL_SET_PUB_NOW == level_set_st){
            light_pub_trans_step = ST_PUB_TRANS_ALL_OK;
            pub_flag = 1;
        }else if(ST_G_LEVEL_SET_PUB_TRANS == level_set_st){
            light_pub_trans_step = ST_PUB_TRANS_ING;
            pub_flag = 1;
        }

        if(pub_flag){
            p_model->pub_trans_flag = 1;
            if(priority){
                light_pub_model_priority = model;
            }
        }
    }

    #if ONLINE_STATUS_EN
    if((ST_G_LEVEL_SET_PUB_NOW == level_set_st) || (ST_G_LEVEL_SET_PUB_TRANS == level_set_st)){
        int update_online_data_flag = 0;
        foreach(i,LIGHT_CNT){
            if(((u32)model == (u32)(&(model_sig_g_onoff_level.onoff_srv[i])))
            #if MD_LIGHTNESS_EN
            || ((u32)model == (u32)(&(model_sig_lightness.srv[i])))
            #endif
            #if LIGHT_TYPE_CT_EN
            || ((u32)model == (u32)(&(model_sig_light_ctl.srv[i])))
            || ((u32)model == (u32)(&(model_sig_light_ctl.temp[i])))
            #endif
            #if LIGHT_TYPE_HSL_EN
            || ((u32)model == (u32)(&(model_sig_light_hsl.srv[i])))
            || ((u32)model == (u32)(&(model_sig_light_hsl.hue[i])))
            || ((u32)model == (u32)(&(model_sig_light_hsl.sat[i])))
            #endif
            ){
                update_online_data_flag = 1;
                break;
            }
        }

        #if MD_LEVEL_EN
        if(!update_online_data_flag){
            foreach(i,g_ele_cnt){
                if((u32)model == (u32)(&(model_sig_g_onoff_level.level_srv[i]))){
                    update_online_data_flag = 1;
                    break;
                }
            }
        }
        #endif

        if(update_online_data_flag){
            device_status_update();
        }
    }
    #endif
}
#endif
//---------

/**
  * @}
  */
    
/**
  * @}
  */


//--model command interface-------------------
//-----------access command--------

//--model command interface end----------------


#endif

