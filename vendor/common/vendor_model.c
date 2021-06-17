/********************************************************************************************************
 * @file     vendor_model.c 
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
#if !WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "vendor_model.h"
#include "fast_provision_model.h"
#if DU_ENABLE
#include "user_du.h"
#endif
#if ALI_MD_TIME_EN
#include "user_ali_time.h"
#endif

#if (VENDOR_MD_NORMAL_EN)
model_vd_light_t       	model_vd_light;

#if (DUAL_VENDOR_EN)
STATIC_ASSERT((VENDOR_MD_LIGHT_S && 0xffff) != VENDOR_ID_MI);
#if 1   // if not equal, please modify id value in traversal_cps_reset_vendor_id();
STATIC_ASSERT((VENDOR_MD_LIGHT_S && 0xffff0000) == (MIOT_SEPC_VENDOR_MODEL_SER && 0xffff0000));
STATIC_ASSERT((VENDOR_MD_LIGHT_C && 0xffff0000) == (MIOT_SEPC_VENDOR_MODEL_CLI && 0xffff0000));
STATIC_ASSERT((VENDOR_MD_LIGHT_S2 && 0xffff0000) == (MIOT_VENDOR_MD_SER && 0xffff0000));
#endif
#endif
/*
	vendor command callback function ----------------
*/
#if MD_SERVER_EN
#if DEBUG_VENDOR_CMD_EN
u8 vd_onoff_state[ELE_CNT] = {0};



// --------- vendor onoff --------
int vd_group_g_light_onoff(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int pub_flag = 0;
    vd_light_onoff_set_t *p_set = (vd_light_onoff_set_t *)par;
    int light_idx = cb_par->model_idx;
    int on = !!p_set->sub_op;   // make sure bool
    light_onoff_all(on);    // not must, just for LED indication, no adjust global var of lightness.
    if(vd_onoff_state[light_idx] != on){
        vd_onoff_state[light_idx] = on;
        pub_flag = 1;
    }else{
    }
    
    return pub_flag;
}

int vd_light_tx_cmd_onoff_st(u8 light_idx, u8 sub_op, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
    vd_light_onoff_st_t rsp;
    rsp.sub_op = !!vd_onoff_state[light_idx];
#if DEBUG_CFG_CMD_GROUP_AK_EN
	rsp.brx_num = blt_rxfifo.num;
#endif
    return mesh_tx_cmd_rsp(VD_GROUP_G_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, uuid, pub_md);
}

int vd_light_onoff_st_publish(u8 light_idx)
{
#if DUAL_VENDOR_EN
    return 0;   // not use now.
#else
	model_common_t *p_com_md = &model_vd_light.srv[light_idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
    return vd_light_tx_cmd_onoff_st(light_idx, 0, ele_adr, pub_adr, uuid, p_com_md);
#endif
}

// --------- vendor LPN GATT ota mode set  --------
#if FEATURE_LOWPOWER_EN
int vd_group_g_lpn_gatt_ota_mode_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    vd_lpn_gatt_ota_mode_set_t *p_set = (vd_lpn_gatt_ota_mode_set_t *)par;
    lpn_mode_set(p_set->mode);
    return 0;
}

int vd_group_g_tx_lpn_gatt_ota_mode_st(u8 light_idx, u8 sub_op, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
    vd_lpn_gatt_ota_mode_status_t rsp;
    rsp.sub_op = sub_op;
    rsp.mode = lpn_mode;
    return mesh_tx_cmd_rsp(VD_GROUP_G_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, uuid, pub_md);
}
#endif


// --------- vendor sub op code end  --------
int cb_vd_group_g_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    u8 sub_op = 0;
    if(par_len){    // compatible with legacy version that is no par.
        sub_op = par[0];
    }
    
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    cb_vd_group_g_sub_tx_st p_cb = (cb_vd_group_g_sub_tx_st)search_vd_group_g_func(sub_op, SEARCH_VD_GROUP_G_FUNC_TYPE_TX_ST);
    if(p_cb){
        return p_cb(cb_par->model_idx, sub_op, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
    }else{
        return -1;
    }
}

int cb_vd_group_g_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = -1;
	int pub_flag = 0;
    //model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    vd_group_g_set_t *p_set = (vd_group_g_set_t *)par;
    u8 sub_op = p_set->sub_op;

    if(!cb_par->retransaction){
        cb_vd_group_g_sub_set p_cb_set = (cb_vd_group_g_sub_set)search_vd_group_g_func(sub_op, SEARCH_VD_GROUP_G_FUNC_TYPE_SET);
        if(p_cb_set){
            pub_flag = p_cb_set(par, par_len, cb_par);
        }else{
            return -1;
        }
    }

#if DEBUG_CFG_CMD_GROUP_AK_EN
    if(par_len >= 3){
    	if(par[2] && (par[2]<=128) && BIT_IS_POW2(par[2])){
    		blt_rxfifo.num = par[2];
    	}
	}
#endif

    if(VD_GROUP_G_SET_NOACK != cb_par->op){
        err = cb_vd_group_g_get(par, par_len, cb_par);
    }else{
        err = 0;
    }

    if(!err && pub_flag){
        if(is_vd_onoff_op(sub_op)){ // only onoff need publish now
            model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 1);
        }
    }
    
    return err;
}

vd_group_g_func_t vd_group_g_func[] = {
    /* telink use sub op from 0x00 to 0x7f*/
    {VD_GROUP_G_OFF, vd_group_g_light_onoff, vd_light_tx_cmd_onoff_st},
    {VD_GROUP_G_ON, vd_group_g_light_onoff, vd_light_tx_cmd_onoff_st},
#if FEATURE_LOWPOWER_EN
    {VD_GROUP_G_LPN_GATT_OTA_MODE, vd_group_g_lpn_gatt_ota_mode_set, vd_group_g_tx_lpn_gatt_ota_mode_st},
#endif

    /* user use sub op from 0x80 to 0xff*/
    //{VD_GROUP_G_USER_START, , , },
};

u8 * search_vd_group_g_func(u32 sub_op, int type)
{
    foreach_arr(i,vd_group_g_func){
        if(sub_op == vd_group_g_func[i].sub_op){
            if(SEARCH_VD_GROUP_G_FUNC_TYPE_SET == type){
                return (u8*)vd_group_g_func[i].cb_set;
            }else if(SEARCH_VD_GROUP_G_FUNC_TYPE_TX_ST == type){
                return (u8*)vd_group_g_func[i].cb_tx_st;
            }else if(SEARCH_VD_GROUP_G_FUNC_TYPE_RX_STATUS == type){
                //return (u8*)vd_group_g_func[i].cb_rx_status;
            }
            break;
        }
    }

    return 0;
}
#endif

#if SPIRIT_VENDOR_EN
vd_msg_attr_t vd_msg_attr[ATTR_TYPE_MAX_NUM]={
	{ATTR_TARGET_TEMP},
	{ATTR_CURRENT_TEMP},
};

//note:there is 1.2s response delay after receive reliable command, refer to MESH_RSP_BASE_DELAY_STEP
//user should use bls_ll_setAdvParam(...) to set lseep time(adv interval) in soft timer mode, SLEEP_TIME_US is useless.
//user can set gatt_adv_send_flag = 0 to stop gatt adv and save current
#define SLEEP_TIME_US 		ADV_INTERVAL_MIN 
#define RUN_TIME_US			(20*1000)
#define INDICATE_RETRY_CNT 	6

mesh_tx_indication_t mesh_indication_retry;
mesh_sleep_pre_t		mesh_sleep_time={0, RUN_TIME_US, SLEEP_TIME_US};

void vd_msg_attr_indica_retry_start(u16 interval_ms)
{
	mesh_indication_retry.retry_cnt = INDICATE_RETRY_CNT;
	mesh_indication_retry.busy = 1;
    mesh_indication_retry.tick = clock_time();
	mesh_indication_retry.interval_tick = interval_ms*1000*sys_tick_per_us;
}

void vd_msg_attr_indica_retry_stop()
{
	mesh_indication_retry.busy = 0;
}

int mesh_tx_cmd_indica_retry(u16 op, u8 *par, u32 par_len, u16 adr_src, u16 adr_dst, int rsp_max)
{
	int ret = 0;
	u16 retry_interval = 0;
	
	material_tx_cmd_t mat;
	mesh_match_type_t match_type;
	u8 nk_array_idx = get_nk_arr_idx_first_valid();
	u8 ak_array_idx = get_ak_arr_idx_first_valid(nk_array_idx);
	set_material_tx_cmd(&mat, op, par, par_len, adr_src, adr_dst, g_reliable_retry_cnt_def, rsp_max, 0, nk_array_idx, ak_array_idx, 0, 1);
    mesh_match_group_mac(&match_type, mat.adr_dst, mat.op, 1, mat.adr_src);
	
	memcpy(&mesh_indication_retry.mat, &mat, sizeof(material_tx_cmd_t));
	memcpy(&mesh_indication_retry.match_type, &match_type,sizeof(mesh_indication_retry.match_type));
	memset(mesh_indication_retry.ac_par, 0, sizeof(mesh_indication_retry.ac_par));
    memcpy(mesh_indication_retry.ac_par, mat.par, mat.par_len);
    mesh_indication_retry.mat.par = mesh_indication_retry.ac_par;

	ret = mesh_tx_cmd(&mat);
	retry_interval = (par_len+7)/8 * CMD_INTERVAL_MS;
	vd_msg_attr_indica_retry_start(retry_interval);
	u8 tid_pos=0;
	if(is_cmd_with_tid_vendor(&tid_pos, op, par, 0)){
		if(mesh_indication_retry.ac_par[tid_pos] == 0){ //if tid par is 0,use internal tid value
			u32 ele_idx = get_ele_idx(adr_src);
			mesh_indication_retry.tid = mesh_tid.tx[ele_idx];
			mesh_indication_retry.ac_par[tid_pos] = mesh_indication_retry.tid;
		}
		else{
			mesh_indication_retry.tid = mesh_indication_retry.ac_par[tid_pos];
		}
	}
	
	return ret;
}

int access_cmd_attr_indication(u16 op, u16 adr_dst, u16 attr_type, u8 *attr_par, u8 par_len)
{
	u8 ret = -1;
	vd_msg_attr_set_t par;
	if(mesh_indication_retry.busy){
		return ret;
	}

	if(!is_unicast_adr(adr_dst)){
		mesh_tx_segment_finished();
	}
	
	par.attr_type = attr_type;
	par_len = par_len>ATTR_PAR_MAX_LEN?ATTR_PAR_MAX_LEN:par_len;
	par.tid = 0;// use internal tid
	memcpy(&par.attr_par, attr_par, par_len);
	par_len += OFFSETOF(vd_msg_attr_set_t,attr_par);
	
	ret = mesh_tx_cmd_indica_retry(op, (u8 *)&par, par_len, ele_adr_primary, adr_dst, 1);
	return ret;

}

void mesh_tx_indication_tick_refresh()
{
	mesh_indication_retry.tick = clock_time();
}

void mesh_tx_indication_proc()
{
 	if(mesh_indication_retry.busy){
	    if(clock_time_exceed(mesh_indication_retry.tick, mesh_indication_retry.interval_tick/sys_tick_per_us)){
			mesh_tx_indication_tick_refresh();
	        if(mesh_indication_retry.retry_cnt){
	            mesh_indication_retry.retry_cnt--;
				mesh_indication_retry.tick = clock_time();
				if(!is_unicast_adr(mesh_indication_retry.mat.adr_dst)){
					mesh_tx_segment_finished();
				}
	            mesh_tx_cmd2_access(&mesh_indication_retry.mat, 0, &mesh_indication_retry.match_type);
	        }else{            			
	        	mesh_indication_retry.busy = 0;
	        }
	    }
    }	
}

u32 get_attr_type_index(u16 attr_type)
{
	for(u8 i=0; i<ATTR_TYPE_MAX_NUM; i++){
		if(attr_type == vd_msg_attr[i].attr_type){
			return i;
		}
	}
	
	return ATTR_TYPE_NOT_EXIST;
}

u8 get_attr_para_len(u16 attr_type)
{
	u8 len = 0;
	switch(attr_type){
		case ATTR_TARGET_TEMP:
			len = 2;
			break;
		default:
			break;
	}

	return len;
}

u8 is_rsp_cmd_tid = 0;
int vd_msg_tx_attr_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u8 *par)
{
	u8 ret = -1;
	vd_msg_attr_sts_t rsp;
	vd_msg_attr_sts_t *p_attr = (vd_msg_attr_sts_t *) par;	
   	u8 attr_index = get_attr_type_index(p_attr->attr_type);
	u8 attr_len = get_attr_para_len(p_attr->attr_type);
	
	rsp.attr_type = p_attr->attr_type;
	rsp.tid = p_attr->tid;
	if(ATTR_TYPE_NOT_EXIST != attr_index){
		memcpy(rsp.attr_par, vd_msg_attr[attr_index].attr_par, attr_len);
	}
	else{
		attr_len = 1;
		rsp.attr_type = ATTR_ERR_CODE;
		rsp.err_code = ATTR_NOT_SUPPORT;
	}
	is_rsp_cmd_tid = 1;
    ret = mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *)&rsp, OFFSETOF(vd_msg_attr_sts_t, attr_par)+attr_len, ele_adr, dst_adr, uuid, pub_md);
	is_rsp_cmd_tid = 0;
	return ret;
}

int vd_msg_attr_st_rsp(mesh_cb_fun_par_t *cb_par, u8 *par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	
    return vd_msg_tx_attr_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, par);
}

int cb_vd_msg_attr_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	vd_msg_attr_get_t *p_get = (vd_msg_attr_get_t *)par;
	switch (p_get->attr_type) {
#if ALI_MD_TIME_EN
	case ALI_VD_MD_TIME_ATTR_TYPE_TIME:
		return ali_mesh_unix_time_get(par, par_len, cb_par);
	case ALI_VD_MD_TIME_ATTR_TYPE_TIME_ZONE:
		return ali_mesh_timezone_get(par, par_len, cb_par);
	case ALI_VD_MD_TIME_ATTR_TYPE_SNTP:
		return ali_mesh_sntp_get(par, par_len, cb_par);
	case ALI_VD_MD_TIME_ATTR_TYPE_TIMING:
		return ali_mesh_timing_get(par, par_len, cb_par);
#endif
	}
    return vd_msg_attr_st_rsp(cb_par, par);
}

int cb_vd_msg_attr_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
    vd_msg_attr_set_t *p_set = (vd_msg_attr_set_t *)par;
#if 0
    u16 attr_type = p_set->attr_type;
		
	if(par_len > OFFSETOF(vd_msg_attr_set_t, attr_par) + ATTR_PAR_MAX_LEN){
		par_len = ATTR_PAR_MAX_LEN;
	}

	u8 attr_index = get_attr_type_index(attr_type);
	u8 attr_len = get_attr_para_len(attr_type);
	
	if(!cb_par->retransaction){
		if(ATTR_TYPE_NOT_EXIST != attr_index){
			LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"attr_type exist",0);
			if(attr_type) // for err_code test
			memcpy(vd_msg_attr[attr_index].attr_par, p_set->attr_par,attr_len);
		}
	}
	   
    if(VD_MSG_ATTR_SET_NACK != cb_par->op){
        err = vd_msg_attr_st_rsp(cb_par, par);
    }else{
        err = 0;
    }
#else
    switch(p_set->attr_type) {
#if ALI_MD_TIME_EN
    case ALI_VD_MD_TIME_ATTR_TYPE_TIME:
    	return ali_mesh_unix_time_set(par, par_len, cb_par);
    case ALI_VD_MD_TIME_ATTR_TYPE_TIME_ZONE:
    	return ali_mesh_timezone_set(par, par_len, cb_par);
    case ALI_VD_MD_TIME_ATTR_TYPE_SNTP:
    	return ali_mesh_sntp_set(par, par_len, cb_par);
    case ALI_VD_MD_TIME_ATTR_TYPE_TIMING:
    	return ali_mesh_timing_set(par, par_len, cb_par);
    case ALI_VD_MD_TIME_ATTR_TYPE_CYCLE_TIMING:
    	return ali_mesh_cycle_timing_set(par, par_len, cb_par);
    case ALI_VD_MD_TIME_ATTR_TYPE_CYCLE_TIMING_DEL:
    	return ali_mesh_timing_del(par, par_len, cb_par);
#endif
    }
#endif
    return err;
}

int cb_vd_msg_attr_confirm(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
	
	if(!is_own_ele(cb_par->adr_src) && (mesh_indication_retry.tid == par[0])){
		vd_msg_attr_indica_retry_stop();
		mesh_tx_segment_finished();
	}
	
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

#if ALI_MD_TIME_EN

int cb_vd_msg_attr_upd_time_req(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}


int cb_vd_msg_attr_upd_time_rsp(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    vd_msg_attr_set_t *p_set = (vd_msg_attr_set_t *)par;
	switch(p_set->attr_type) {
	case ALI_VD_MD_TIME_ATTR_TYPE_TIME:
		return ali_mesh_unix_time_set(par, par_len, cb_par);
	}
	return 0;
}

#endif

#endif
#else
#define cb_vd_group_g_get           (0)
#define cb_vd_group_g_set           (0)
#define cb_vd_msg_attr_get          (0)
#define cb_vd_msg_attr_set          (0)
#define cb_vd_msg_attr_confirm      (0)
#if ALI_MD_TIME_EN
#define cb_vd_msg_attr_upd_time_req	(0)
#define cb_vd_msg_attr_upd_time_rsp	(0)
#endif
#endif

#if MD_CLIENT_EN
int cb_vd_group_g_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int cb_vd_msg_attr_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}
#else
#define cb_vd_group_g_status            (0)
#define cb_vd_msg_attr_status           (0)
#endif

// ------ both server or GW / APP support --------
#if SPIRIT_VENDOR_EN
int vd_msg_tx_attr_confirm(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 attr_type)
{
	vd_msg_attr_confirm_t rsp;
	
	rsp.attr_type = attr_type;
    return mesh_tx_cmd_rsp(VD_MSG_ATTR_CONFIRM, (u8 *)&rsp, sizeof(vd_msg_attr_confirm_t), ele_adr, dst_adr, uuid, pub_md);
}

int vd_msg_attr_confirm_rsp(mesh_cb_fun_par_t *cb_par, u16 attr_type)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	
    return vd_msg_tx_attr_confirm(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, attr_type);
}

int cb_vd_msg_attr_indication(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = -1;
	vd_msg_attr_set_t *p_set = (vd_msg_attr_set_t *)par;
	if(!is_own_ele(cb_par->adr_src)){
  		err = vd_msg_attr_confirm_rsp(cb_par, p_set->attr_type);
	}
    return err;
}
#endif

#if (DEBUG_VENDOR_CMD_EN)
int cb_vd_key_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    //model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    vd_rc_key_report_t *p_key = (vd_rc_key_report_t *)par;
    p_key = p_key;
	LOG_MSG_INFO(TL_LOG_MESH,0,0,"vendor model key report: %d", p_key->code);
	
#if (! (__PROJECT_MESH_SWITCH__))
	static u16 key_report_1_ON,  key_report_2_ON,  key_report_3_ON,  key_report_4_ON;
	static u16 key_report_1_OFF, key_report_2_OFF, key_report_3_OFF, key_report_4_OFF;
	static u16 key_report_UP,	 key_report_DN,    key_report_L,	 key_report_R;

	u8 lum_level = light_lum_get(0, 0);

	switch(p_key->code){
		case RC_KEY_1_ON:
			key_report_1_ON++;
			break;
		case RC_KEY_1_OFF:
			key_report_1_OFF++;
			break;
		case RC_KEY_2_ON:
			key_report_2_ON++;
			break;
		case RC_KEY_2_OFF:
			key_report_2_OFF++;
			break;
		case RC_KEY_3_ON:
			key_report_3_ON++;
			break;
		case RC_KEY_3_OFF:
			key_report_3_OFF++;
			break;
		case RC_KEY_4_ON:
			key_report_4_ON++;
			break;
		case RC_KEY_4_OFF:
			key_report_4_OFF++;
			break;
		case RC_KEY_UP:
			key_report_UP++;
			if(lum_level >= 100){
				lum_level = 20;
			}else{
				lum_level += 20;
			}
			
			access_set_lum(ADR_ALL_NODES, 0, lum_level, 0);
			break;
		case RC_KEY_DN:
			key_report_DN++;
			if(lum_level <= 20){
				lum_level = 100;
			}else{
				lum_level -= 20;
			}
			
			access_set_lum(ADR_ALL_NODES, 0, lum_level, 0);
			break;
		case RC_KEY_L:
			key_report_L++;
			break;
		case RC_KEY_R:
			key_report_R++;
			break;
		default:
			break;
	}
#endif

    return 0;
}
#endif
// ------ end --------

#if (LPN_VENDOR_SENSOR_EN && MD_SERVER_EN)
typedef struct{
    u16 hunmidity;
    u16 temp; 
}hx300t_sensor_t;
#define HX300T_MSG_RSP_MODE 0
#define HX300T_MSG_PUB_MODE 1
typedef struct{
    u8 hx300t_tick ;
    u8 msg_mode;
    u16 hx300t_src ;
    u8 pub_idx;
}hx300t_mag_st_t;

hx300t_mag_st_t hx300t_mag;
hx300t_sensor_t hx300t_sensor;

#define HX300_SENSOR_ID     (0x44<<1)
#define HX300_INTER_TIME    (35*1000)

#if ((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
#if(MCU_CORE_TYPE == MCU_CORE_8258)
#include "../../drivers/8258/i2c.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "../../drivers/8278/i2c.h"
#endif
void i2c_io_init()
{
    i2c_master_init(HX300_SENSOR_ID,8);//set clk to 500k
    i2c_gpio_set(I2C_GPIO_GROUP_C0C1);
}
#elif (MCU_CORE_TYPE == MCU_CORE_8269)
#include "../../proj/drivers/i2c.h"
void i2c_io_init()
{
      // i2c init
    gpio_set_func(PIN_I2C_SDA,AS_GPIO);
    gpio_set_func(PIN_I2C_SCL,AS_GPIO);
    gpio_setup_up_down_resistor(PIN_I2C_SDA, 2);//GPIO_PULL_UP_10K
    gpio_setup_up_down_resistor(PIN_I2C_SCL, 2);//GPIO_PULL_UP_10K    
}

#else
#endif


void sensor_read_start()
{
    u8 r = irq_disable();
    #if (MCU_CORE_TYPE == MCU_CORE_8269)
    i2c_write_start(HX300_SENSOR_ID);
    #else
    i2c_write_byte(HX300_SENSOR_ID,1,0);
    #endif
    irq_restore(r);
}

void sensor_read_fun(u16 *p_humi,u16 *p_temp)
{
    u8 val_buffer[4];
    u8 r = irq_disable();
    #if (MCU_CORE_TYPE == MCU_CORE_8269)
    i2c_read_start_buf(HX300_SENSOR_ID,val_buffer,4);
    #else
    i2c_read_series(0,0,val_buffer,4);
    #endif
    irq_restore(r);
    u16 tempreture,humidity;
    humidity=((val_buffer[0]<<8) | val_buffer[1])&0x3fff;
    tempreture=((val_buffer[2]<<8) | val_buffer[3])>>2;
    humidity    = humidity * 100/ 0x3fff;
    tempreture = tempreture  * 165/ 0x3fff - 40;
    *p_humi = humidity;
    *p_temp = tempreture;
}

void sensor_proc_loop()
{
    if(hx300t_mag.hx300t_tick && clock_time_exceed(hx300t_mag.hx300t_tick,HX300_INTER_TIME)){
        hx300t_mag.hx300t_tick =0;
        sensor_read_fun(&(hx300t_sensor.hunmidity),&(hx300t_sensor.temp));
        if(hx300t_mag.msg_mode == HX300T_MSG_RSP_MODE){
            mesh_tx_cmd_rsp(VD_LPN_SENSOR_STATUS, (u8 *)&hx300t_sensor, 
                               sizeof(hx300t_sensor), ele_adr_primary, hx300t_mag.hx300t_src, 0, 0);
        }else if (hx300t_mag.msg_mode == HX300T_MSG_PUB_MODE){
            vd_lp_sensor_st_send(hx300t_mag.pub_idx);
        }
     }
}

int cb_vd_lpn_sensor_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // trigger start sensor data send 
    sensor_read_start();
    hx300t_mag.hx300t_tick = clock_time()|1;
    hx300t_mag.msg_mode = HX300T_MSG_RSP_MODE;
    hx300t_mag.hx300t_src = cb_par->adr_src;
    return 0;
}
int cb_vd_lpn_sensor_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}
// same as the vd_light_onoff_st_publish
int vd_lp_sensor_st_send(u8 idx)
{
    model_common_t *p_com_md = &model_vd_light.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_rsp(VD_LPN_SENSOR_STATUS, (u8 *)&hx300t_sensor, sizeof(hx300t_sensor), ele_adr, pub_adr, uuid, p_com_md);
}
int cb_vd_lpn_sensor_st_publish(u8 idx)
{
    #if 0 // not stable 
    if(idx == 0){
        sensor_read_start();
        hx300t_mag.hx300t_tick = clock_time()|1;
        hx300t_mag.msg_mode = HX300T_MSG_PUB_MODE;
	    hx300t_mag.pub_idx = idx;
    }
	#else
	sensor_read_start();
	sleep_us(HX300_INTER_TIME);
	sensor_read_fun(&(hx300t_sensor.hunmidity),&(hx300t_sensor.temp));
	vd_lp_sensor_st_send(idx);
	#endif
	return 0;
}
void mesh_vd_lpn_pub_set()
{
    mesh_cfg_model_pub_set_t pub_set = {0};
    pub_set.ele_adr = ele_adr_primary;
    pub_set.pub_adr = 0xffff;  // publish all
    pub_set.pub_par.appkey_idx = 0;
	pub_set.pub_par.credential_flag = 0;
	pub_set.pub_par.rfu = 0;
	pub_set.pub_par.ttl = 0x3F;
	pub_set.pub_par.period.res= 1;
	pub_set.pub_par.period.steps = 0x1e;//30s interval
	pub_set.pub_par.transmit.count= 0;
	pub_set.pub_par.transmit.invl_steps = 0;
	pub_set.model_id = 0x00000211;    
	mesh_tx_cmd2normal_primary(CFG_MODEL_PUB_SET, (u8 *)&pub_set, sizeof(mesh_cfg_model_pub_set_t), ele_adr_primary, 0);
}

#else

#define cb_vd_lpn_sensor_get (0)
#define cb_vd_lpn_sensor_sts (0)
#endif



#if VC_SUPPORT_ANY_VENDOR_CMD_EN
int cb_app_vendor_all_cmd(mesh_cmd_ac_vd_t *ac, int ac_len, mesh_cb_fun_par_vendor_t *cb_par)
{
    return 0;
}
#endif

// use unsegment pkt for vendor command should be better.
// = sizeof(mesh_cmd_lt_unseg_t.data) - sizeof(MIC) - sizeof(op) = 15-4-3 = 8
STATIC_ASSERT(sizeof(vd_rc_key_report_t) <= 8);

#if DEBUG_CFG_CMD_GROUP_AK_EN
int cb_vd_trans_time_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // send back the current tick part .
    u32 tick_sts =0;
	tick_sts = clock_time();
    return mesh_tx_cmd_rsp(VD_MESH_TRANS_TIME_STS,(u8*)&tick_sts,4,ele_adr_primary,cb_par->adr_src,0,0);
    
}

u8 max_time_10ms = 0x20; 
int cb_vd_trans_time_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	max_time_10ms = par[0];
	u32 tick_sts =0;
	tick_sts = max_time_10ms;
	return mesh_tx_cmd_rsp(VD_MESH_TRANS_TIME_STS,(u8*)&tick_sts,4,ele_adr_primary,cb_par->adr_src,0,0);
}

int cb_vd_trans_time_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
	return err;
}
#endif

#if DU_ENABLE
int cb_vd_du_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 1;
}

#endif

#if !WIN32
const 
#endif
mesh_cmd_sig_func_t mesh_cmd_vd_func[] = {
#if (VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_SPIRIT)
    #if 0 // DEBUG_VENDOR_CMD_EN // just for sample, default disable, 
    {VD_GROUP_G_SET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_set, VD_GROUP_G_STATUS},
	{VD_GROUP_G_GET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_get, VD_GROUP_G_STATUS},
	{VD_GROUP_G_SET_NOACK, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_set, STATUS_NONE},
    {VD_GROUP_G_STATUS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_group_g_status, STATUS_NONE},
    #endif
    
    #if SPIRIT_VENDOR_EN
	{VD_MSG_ATTR_GET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_msg_attr_get, VD_MSG_ATTR_STS},
	{VD_MSG_ATTR_SET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_msg_attr_set, VD_MSG_ATTR_STS},
	{VD_MSG_ATTR_SET_NACK, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_msg_attr_set, STATUS_NONE},
    {VD_MSG_ATTR_STS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_msg_attr_status, STATUS_NONE},
    {VD_MSG_ATTR_INDICA, 0, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_msg_attr_indication, VD_MSG_ATTR_CONFIRM},
    {VD_MSG_ATTR_CONFIRM, 1, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_msg_attr_confirm, STATUS_NONE},
		#if ALI_MD_TIME_EN
    {VD_MSG_ATTR_UPD_TIME_REQ, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_msg_attr_upd_time_req, STATUS_NONE},
    {VD_MSG_ATTR_UPD_TIME_RSP, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_msg_attr_upd_time_rsp, STATUS_NONE},
		#endif
    #endif

	#if DU_ENABLE
	{VD_LPN_REPROT, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_du_report, VD_LPN_REPROT},
	#endif
    
#elif(VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_DEFAULT)
    #if (DRAFT_FEATURE_VENDOR_TYPE_SEL == DRAFT_FEATURE_VENDOR_TYPE_ONE_OP)
	{VD_EXTEND_CMD0, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, 0, STATUS_NONE},
    #elif DRAFT_FEAT_VD_MD_EN
    // add by user
    #elif (DEBUG_VENDOR_CMD_EN)
	{VD_RC_KEY_REPORT, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_key_report, STATUS_NONE},
	
    {VD_GROUP_G_SET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_set, VD_GROUP_G_STATUS},
	{VD_GROUP_G_GET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_get, VD_GROUP_G_STATUS},
	{VD_GROUP_G_SET_NOACK, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_group_g_set, STATUS_NONE},
    {VD_GROUP_G_STATUS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_group_g_status, STATUS_NONE},
    #endif
        
    #if (FAST_PROVISION_ENABLE)
	{VD_MESH_RESET_NETWORK, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_reset_network, STATUS_NONE},
	{VD_MESH_ADDR_GET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_get_addr, VD_MESH_ADDR_GET_STS},
	{VD_MESH_ADDR_GET_STS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_mesh_addr_sts, STATUS_NONE},
	{VD_MESH_ADDR_SET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_set_addr, VD_MESH_ADDR_SET_STS},
	{VD_MESH_ADDR_SET_STS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_mesh_addr_sts, STATUS_NONE},
    {VD_MESH_PROV_DATA_SET, 1, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_set_provision_data, STATUS_NONE},
    {VD_MESH_PROV_CONFIRM, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_provision_confirm, VD_MESH_PROV_CONFIRM_STS},
    {VD_MESH_PROV_CONFIRM_STS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_mesh_provison_data_sts, STATUS_NONE},
    {VD_MESH_PROV_COMPLETE, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_mesh_provision_complete, STATUS_NONE},
	#else
		#if DEBUG_CFG_CMD_GROUP_AK_EN
	{VD_MESH_TRANS_TIME_GET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_trans_time_get, VD_MESH_TRANS_TIME_STS},
	{VD_MESH_TRANS_TIME_SET, 0, VENDOR_MD_LIGHT_C, VENDOR_MD_LIGHT_S, cb_vd_trans_time_set, VD_MESH_TRANS_TIME_STS},
	{VD_MESH_TRANS_TIME_STS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_trans_time_sts, STATUS_NONE},
		#endif
    #endif
    
    #if (LPN_VENDOR_SENSOR_EN)
    {VD_LPN_SENSOR_GET,0,VENDOR_MD_LIGHT_C,VENDOR_MD_LIGHT_S,cb_vd_lpn_sensor_get,VD_LPN_SENSOR_STATUS},
    {VD_LPN_SENSOR_STATUS, 1, VENDOR_MD_LIGHT_S, VENDOR_MD_LIGHT_C, cb_vd_lpn_sensor_sts, STATUS_NONE},
    #endif
#endif

    USER_MESH_CMD_VD_ARRAY
};

#if WIN32
void APP_set_vd_id_mesh_cmd_vd_func(u16 vd_id)
{
    foreach_arr(i,mesh_cmd_vd_func){
        //SET_FLD_V(mesh_cmd_vd_func[i].model_id_tx, BIT_RNG(0,15), vd_id);
        //SET_FLD_V(mesh_cmd_vd_func[i].model_id_rx, BIT_RNG(0,15), vd_id);
        mesh_cmd_vd_func[i].model_id_tx = (mesh_cmd_vd_func[i].model_id_tx & 0xFFFF0000) | vd_id;
        mesh_cmd_vd_func[i].model_id_rx = (mesh_cmd_vd_func[i].model_id_rx & 0xFFFF0000) | vd_id;
    }
}
#endif

// don't modify mesh_search_model_id_by_op_vendor()
int mesh_search_model_id_by_op_vendor(mesh_op_resource_t *op_res, u16 op, u8 tx_flag)
{
    foreach_arr(i,mesh_cmd_vd_func){
        if(op == mesh_cmd_vd_func[i].op){
            op_res->cb = mesh_cmd_vd_func[i].cb;
            op_res->op_rsp = mesh_cmd_vd_func[i].op_rsp;
            op_res->sig = 0;
            op_res->status_cmd = mesh_cmd_vd_func[i].status_cmd ? 1 : 0;
            if(tx_flag){
                op_res->id = mesh_cmd_vd_func[i].model_id_tx;
            }else{
                op_res->id = mesh_cmd_vd_func[i].model_id_rx;
            }
            return 0;
        }
    }

    return -1;
}



//--vendor command interface-------------------
#if (VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_DEFAULT)
    #if DEBUG_VENDOR_CMD_EN
int vd_cmd_key_report(u16 adr_dst, u8 key_code)
{
	vd_rc_key_report_t key_report = {0};
	key_report.code = key_code;

	return SendOpParaDebug(adr_dst, 0, VD_RC_KEY_REPORT, (u8 *)&key_report, sizeof(key_report));
}

int vd_cmd_onoff(u16 adr_dst, u8 rsp_max, u8 onoff, int ack)
{
    if(!is_vd_onoff_op(onoff)){
        return -1;
    }
	
#if 0 // GATEWAY_ENABLE // use the way of INI to send command for gatway
	u8 par[32] = {0};
	mesh_bulk_vd_cmd_par_t *p_bulk_vd_cmd = (mesh_bulk_vd_cmd_par_t *)par;
	p_bulk_vd_cmd->nk_idx = 0;
    p_bulk_vd_cmd->ak_idx = 0;
	p_bulk_vd_cmd->retry_cnt = g_reliable_retry_cnt_def;
	p_bulk_vd_cmd->rsp_max = rsp_max;
	p_bulk_vd_cmd->adr_dst = adr_dst;
	p_bulk_vd_cmd->op = ack ? VD_GROUP_G_SET : VD_GROUP_G_SET_NOACK;
	p_bulk_vd_cmd->vendor_id = g_vendor_id;
	p_bulk_vd_cmd->op_rsp = VD_GROUP_G_STATUS;
	p_bulk_vd_cmd->tid_pos = 2;
	p_bulk_vd_cmd->par[0] = onoff;
	u8 par_len = OFFSETOF(mesh_bulk_vd_cmd_par_t, par) + (p_bulk_vd_cmd->tid_pos?2:1);
	
	return mesh_bulk_cmd((mesh_bulk_cmd_par_t*)p_bulk_vd_cmd, par_len);
#else // use default vendor id to send command, include gateway.
	vd_light_onoff_set_t par = {0};
	par.sub_op = onoff;
	par.tid = 0;

	return SendOpParaDebug(adr_dst, rsp_max, ack ? VD_GROUP_G_SET : VD_GROUP_G_SET_NOACK, 
						   (u8 *)&par, sizeof(vd_light_onoff_set_t));
#endif	
}
    #endif
#endif

//--vendor command interface end----------------


int is_cmd_with_tid_vendor(u8 *tid_pos_out, u16 op, u8 *par, u8 tid_pos_vendor_app)
{
    int cmd_with_tid = 0;
    #if DEBUG_VENDOR_CMD_EN
    u8 sub_op = par[0];
    #endif
    
    switch(op){
        case 0:             // fix compile warning, op will never be '0' here, and will be optimized.
        default:
            break;
    #if DEBUG_VENDOR_CMD_EN
		case VD_RC_KEY_REPORT:
			break;

        case VD_GROUP_G_SET:
        case VD_GROUP_G_SET_NOACK:{
            if(is_vd_onoff_op(sub_op)){
                cmd_with_tid = 1;
                *tid_pos_out = OFFSETOF(vd_light_onoff_set_t, tid);
            }
            break;
        }
    #endif

	#if SPIRIT_VENDOR_EN
		case VD_MSG_ATTR_SET:
		case VD_MSG_ATTR_SET_NACK:
		case VD_MSG_ATTR_INDICA:
		case VD_MSG_ATTR_CONFIRM: // just RX for server model
		#if ALI_MD_TIME_EN
		case VD_MSG_ATTR_UPD_TIME_REQ:
		case VD_MSG_ATTR_UPD_TIME_RSP:
		#endif
			cmd_with_tid = 1;
            *tid_pos_out = 0;
			break;
		case VD_MSG_ATTR_STS:
			if(!is_rsp_cmd_tid){
				cmd_with_tid = 1;
	            *tid_pos_out = 0;
			}
			break;
	#endif
    }

    #if VC_SUPPORT_ANY_VENDOR_CMD_EN
    if((!cmd_with_tid) && (tid_pos_vendor_app != TID_NONE_VENDOR_OP_VC)){
        if(tid_pos_vendor_app != 0){
            cmd_with_tid = 1;
            *tid_pos_out = tid_pos_vendor_app - 1;
        }
    }
    #endif

    #ifdef CB_USER_IS_CMD_WITH_TID_VENDOR
    if(0 == cmd_with_tid){
        cmd_with_tid = CB_USER_IS_CMD_WITH_TID_VENDOR(tid_pos_out, op, par);
    }
    #endif

    return cmd_with_tid;
}
#endif

