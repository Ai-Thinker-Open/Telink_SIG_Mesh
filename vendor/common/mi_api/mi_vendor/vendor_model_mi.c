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
#include "../../app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "vendor_model_mi.h"
#include "../mijia_pub_proc.h"
#if (VENDOR_MD_MI_EN)
#if (0 == DUAL_VENDOR_EN)
model_vd_light_t       	model_vd_light;
#endif

/*
	vendor command callback function ----------------
*/
#if MD_SERVER_EN
#if DEBUG_VENDOR_CMD_EN
u8 mi_vd_onoff_state[ELE_CNT] = {0};

int mi_vd_light_onoff_idx(int idx, int on)
{
    light_onoff_all(on);     // not must, just for indication
    if(mi_vd_onoff_state[idx] != on){
        mi_vd_onoff_state[idx] = on;
        return 1;
    }else{
        return 0;
    }
}

void mi_vd_light_onoff_st_rsp_par_fill(vd_mi_light_onoff_st_t *rsp, u8 idx)
{
    rsp->present_onoff = mi_vd_onoff_state[idx];
}

int mi_vd_light_tx_cmd_onoff_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
    vd_mi_light_onoff_st_t rsp;
    mi_vd_light_onoff_st_rsp_par_fill(&rsp, idx);
    return mesh_tx_cmd_rsp(VD_MI_LIGHT_ONOFF_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, uuid, pub_md);
}

int mi_vd_light_onoff_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mi_vd_light_tx_cmd_onoff_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mi_vd_light_onoff_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_vd_light.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
    return mi_vd_light_tx_cmd_onoff_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mi_cb_vd_light_onoff_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mi_vd_light_onoff_st_rsp(cb_par);
}

int mi_cb_vd_light_onoff_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = -1;
	int pub_flag = 0;
    //model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    vd_mi_light_onoff_set_t *p_set = (vd_mi_light_onoff_set_t *)par;

    if(!cb_par->retransaction){
        if(p_set->onoff < G_ONOFF_RSV){
            pub_flag = mi_vd_light_onoff_idx(cb_par->model_idx, p_set->onoff);
        }
    }
    
    if(VD_MI_LIGHT_ONOFF_SET_NOACK != cb_par->op){
        err = mi_vd_light_onoff_st_rsp(cb_par);
    }else{
        err = 0;
    }

    if(pub_flag){
        model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 1);
    }
    
    return err;
}

// vendor server model 2
    #if MD_VENDOR_2ND_EN
u8 vd_onoff_state2[ELE_CNT] = {0};

int mi_vd_light_onoff_idx2(int idx, int on)
{
    light_onoff_all(on);     // not must, just for indication
    if(vd_onoff_state2[idx] != on){
        vd_onoff_state2[idx] = on;
        return 1;
    }else{
        return 0;
    }
}

void mi_vd_light_onoff_st_rsp_par_fill2(vd_mi_light_onoff_st_t *rsp, u8 idx)
{
    rsp->present_onoff = vd_onoff_state2[idx];
}

int mi_vd_light_tx_cmd_onoff_st2(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
    vd_mi_light_onoff_st_t rsp;
    mi_vd_light_onoff_st_rsp_par_fill2(&rsp, idx);
    return mesh_tx_cmd_rsp(VD_MI_LIGHT_ONOFF_STATUS2, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, uuid, pub_md);
}

int mi_vd_light_onoff_st_rsp2(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mi_vd_light_tx_cmd_onoff_st2(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mi_vd_light_onoff_st_publish2(u8 idx)
{
	model_common_t *p_com_md = &model_vd_light.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
    return mi_vd_light_tx_cmd_onoff_st2(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mi_cb_vd_light_onoff_get2(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mi_vd_light_onoff_st_rsp2(cb_par);
}

int mi_cb_vd_light_onoff_set2(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = -1;
	int pub_flag = 0;
    //model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    vd_mi_light_onoff_set_t *p_set = (vd_mi_light_onoff_set_t *)par;

    if(!cb_par->retransaction){
        if(p_set->onoff < G_ONOFF_RSV){
            pub_flag = mi_vd_light_onoff_idx2(cb_par->model_idx, p_set->onoff);
        }
    }
    
    if(VD_MI_LIGHT_ONOFF_SET_NOACK2 != cb_par->op){
        err = mi_vd_light_onoff_st_rsp2(cb_par);
    }else{
        err = 0;
    }

    if(pub_flag){
        model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 1);
    }
    
    return err;
}
    #endif
#endif


mi_proper_str mi_proper;
mi_proper_str *p_mi_proper;


u8 mi_pub_ssid_piid_is_valid(u8 ssid,u8 piid)
{
	if(ssid ==0 && piid == 0){
		return 0;
	}else{
		return 1;
	}
}

void mi_set_pub_ssid_piid_now(u8 ssid,u8 piid)
{
	p_mi_proper->piid_now = piid;
	p_mi_proper->ssid_now = ssid;
}

mi_pub_str_t *get_mi_pub_by_ssid_piid()
{
	u8 ssid,piid;
	ssid = p_mi_proper->ssid_now;
	piid = p_mi_proper->piid_now;
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		vd_mi_property_changed_str *p_pro = &(p_mi_proper->proper_data[i]);
		if(ssid == p_pro->mi_head.ssid && piid == p_pro->mi_head.piid){
			return &(p_mi_proper->pub[i]);
		}
	}
	return NULL;
}

int  get_ssid_piid_idx(u8 ssid,u8 piid)
{
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		vd_mi_property_changed_str *p_pro = &(p_mi_proper->proper_data[i]);
		if(ssid == p_pro->mi_head.ssid && piid == p_pro->mi_head.piid){
			return i;
		}
	}
	return -1;
}


vd_mi_property_changed_str * get_ssid_piid_pointer(u8 ssid,u8 piid)
{
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		vd_mi_property_changed_str *p_pro = &(p_mi_proper->proper_data[i]);
		if(ssid == p_pro->mi_head.ssid && piid == p_pro->mi_head.piid){
			return p_pro;
		}
	}
	return NULL;
}

vd_mi_property_changed_str * get_ssid_piid_pointer_internal()
{
	return get_ssid_piid_pointer(p_mi_proper->ssid_now,p_mi_proper->piid_now);
}

int  set_mi_proper_data(vd_mi_property_changed_str * property_data) 
{
	vd_mi_property_changed_str *p_pro = get_ssid_piid_pointer(property_data->mi_head.ssid ,property_data->mi_head.piid);
	if(p_pro != NULL){
		
	}else{
		// need to find a empty buf to store 
		p_pro = get_ssid_piid_pointer(0,0);
		if(p_pro == NULL){// the buffer is full
			return 0;
		}
	}
	p_mi_proper->ssid_now = property_data->mi_head.ssid;
	p_mi_proper->piid_now= property_data->mi_head.piid;
	memcpy(p_pro,property_data,sizeof(vd_mi_property_changed_str));
	return 1;
}

void init_mi_ssid_piid_val()
{
	vd_mi_property_changed_str *p_proper = &(p_mi_proper->proper_data[0]);
	p_proper->mi_head.ssid = 0x03;
	p_proper->mi_head.piid = 0x01;
}

void init_mi_ssid_sn()
{
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		p_mi_proper->ssid_sn[i] = rand()&0xff;
	}
}

void init_mi_proper_data()
{
	p_mi_proper = &mi_proper;
	init_mi_ssid_sn();
	p_mi_proper->pub_mode =1;
	init_mi_ssid_piid_val();
}


u8 set_new_mi_ssid(u8 ssid,u8 piid)
{
	u8 idx = get_ssid_piid_idx(ssid,piid);
	if(idx >=0){
		return ++(p_mi_proper->ssid_sn[idx]);
	}else{
		return 0;
	}
	
}
u8 get_new_mi_ssid(u8 ssid,u8 piid)
{
	u8 idx = get_ssid_piid_idx(ssid,piid);
	if(idx >=0){
		return (p_mi_proper->ssid_sn[idx]);
	}else{
		return 0;
	}
}
vd_mi_property_changed_str property_ivi ;
static u32 ivi_sts_cnt =0;

void mi_ivi_event_loop()
{
	int err =-1;
	static u32 ivi_sts_tick =0;
	if(ivi_sts_cnt &&!is_busy_tx_segment_or_reliable_flow()&&clock_time_exceed(ivi_sts_tick,500*1000)){
		ivi_sts_tick = clock_time();
		err = mesh_tx_cmd2normal_primary(VD_MI_PROPERTY_STS, (u8 *)&property_ivi, sizeof(property_ivi), 0xfeff, 0);
		if(err == 0){
			ivi_sts_cnt--;
		}
	}
}

int mi_cb_ivi_event_send(u8 event_type,u8 *p_buf)
{
	static u8 ivi_tid =0;
	ivi_tid++;
	property_ivi.mi_head.ssid = 3;
	property_ivi.mi_head.piid = 1;
	property_ivi.type = 0;
	property_ivi.tid = ivi_tid;
	property_ivi.value[0] = event_type;
	memcpy(property_ivi.value+1,p_buf,3);
	// segment msg ,so it can received
	ivi_sts_cnt =3;
	return mesh_tx_cmd2normal_primary(VD_MI_PROPERTY_STS, (u8 *)&property_ivi, sizeof(property_ivi), 0xfeff, 0);
}

int mi_cb_vd_property_change(u16 ele_adr, u16 dst_adr,u8 ssid,u8 piid,model_common_t *p_com_md)
{
	int err = -1;
	vd_mi_property_changed_str property_sts ;
	memset((u8 *)&property_sts ,0,sizeof(property_sts));
	vd_mi_property_changed_str *p_pro = get_ssid_piid_pointer(ssid,piid);
	if(p_pro == NULL){
		return err;
	}
	memcpy((u8 *)&property_sts,p_pro,sizeof(vd_mi_property_changed_str));// excpet the tid ,others are the same 
	if(p_mi_proper->ver_new){
		property_sts.tid = get_new_mi_ssid(ssid,piid);
		err = mesh_tx_cmd_rsp(VD_MI_PROPERTY_STS, (u8 *)&property_sts, sizeof(property_sts), ele_adr, 
								dst_adr,0, p_com_md);
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"send property new protocol",0);
	}else{
		err = mesh_tx_cmd_rsp(VD_MI_PROPERTY_STS, (u8 *)&property_sts, 6, ele_adr, 
								dst_adr,0, p_com_md);
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"send property old protocol",0);
	}
	return err;
}

int vd_mi_proper_sts_publish(u8 idx)
{
#if DUAL_VENDOR_EN
    return 0;   // not use now.
#else
	LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"mi vendor status publish ",0);
	model_common_t *p_com_md = &model_vd_light.srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
    return mi_cb_vd_property_change(ele_adr,pub_adr,p_mi_proper->ssid_now,p_mi_proper->piid_now,p_com_md);
#endif
}


int mi_cb_vd_get_property(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_common_t *p_com_md = (model_common_t *)cb_par->model;
	vd_mi_get_property_str *p_proper = (vd_mi_get_property_str *)par;
	mi_cb_vd_property_change(p_com_md->ele_adr,cb_par->adr_src,
							p_proper->mi_head.ssid,p_proper->mi_head.piid,0);
	return 1;
}

int soft_timer_proper_sts_proc(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;
	static u32 mi_beacon_cnt ;
	mi_beacon_cnt++;
	vd_mi_proper_sts_publish(0);// send publish cmd for about 3 times
	if(mi_beacon_cnt == 3){
		mi_beacon_cnt =0;
		return -1;
	}else{
		return 0;
	}
}

int mi_cb_vd_set_property(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	LOG_MSG_INFO(TL_LOG_NODE_SDK,par,par_len,"mi_cb_vd_set_property ",0);
	if(mi_cb_vd_set_property_noack(par,par_len,cb_par)== 0){
		LOG_MSG_INFO(TL_LOG_NODE_SDK,par,par_len,"mi_cb_vd_set_property_noack ERR ",0);
		return 0;
	}
	model_common_t *p_com_md = (model_common_t *)cb_par->model;
	vd_mi_set_property_str *p_proper = (vd_mi_set_property_str *)par;
	set_new_mi_ssid(p_proper->mi_head.ssid,p_proper->mi_head.piid);
	mi_cb_vd_property_change(p_com_md->ele_adr,cb_par->adr_src,
				p_proper->mi_head.ssid,p_proper->mi_head.piid,0);
	return 1;
}


int mi_cb_vd_set_property_noack(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_common_t *p_com_md = (model_common_t *)cb_par->model;
	vd_mi_set_property_str *p_proper = (vd_mi_set_property_str *)par;
	if(par_len == sizeof(vd_mi_set_property_str)){
		p_mi_proper->ver_new =1;
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"use mi version new ",0);
	}else{
		p_mi_proper->ver_new =0;
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"use mi version old ",0);
	}
	if(p_mi_proper->ver_new){
		if( is_group_adr(cb_par->adr_dst)||
			is_unicast_adr(cb_par->adr_dst)||
			is_virtual_adr(cb_par->adr_dst)){
			// almost all the adr will proc the tid part .
			if(p_mi_proper->last_tid== p_proper->tid){
				LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"tid is the same",0);
				return 0;
			}else{
				p_mi_proper->last_tid = p_proper->tid;
			}
		}
	}
	if(set_mi_proper_data(p_proper)==0){
		return 0;
	}
	if(p_mi_proper->pub_mode){
		st_pub_list_t pub_list = {{0}};
		pub_list.st[ST_TRANS_MI_VENDOR_STS] = ST_G_LEVEL_SET_PUB_NOW;
    	model_pub_check_set_bind_all(&pub_list, cb_par, 0);
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"check bind and send sts quickly  ",0);
	}else{
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"use const fast inter to send ",0);
		#if BLT_SOFTWARE_TIMER_ENABLE
		blt_soft_timer_add(&soft_timer_proper_sts_proc, 300*1000);
		#endif
	}
	return 1;
}

int mi_cb_vd_action(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return 1;
}

int mi_cb_vd_reply_action(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return 1;
}

int mi_cb_vd_event_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return 1;
}


int mi_cb_vd_vendor(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 type = par[0];
	if(type == MI_NODE_FOUND_REQ){
		u8 rsp = MI_DEV_FOUND_RSP;
		mesh_tx_cmd2normal_primary(VD_MI_NODE_GW, (u8 *)&rsp, sizeof(rsp),cb_par->adr_src,0);
	}else if (type == MI_NET_PARA_RSP){
		pub_max_inter_rcv_cb(par[2]);
	}
	return 1;
}

int mi_cb_vd_vendor_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 1;
}

#else
#define cb_vd_light_onoff_get       (0)
#define cb_vd_light_onoff_set       (0)
#define cb_vd_light_onoff_get2      (0)
#define cb_vd_light_onoff_set2      (0)

#define mi_cb_vd_get_property       (0)
#define mi_cb_vd_set_property       (0)
#define mi_cb_vd_set_property_noack (0)
#define mi_cb_vd_action             (0)
#define mi_cb_vd_reply_action       (0)
#define mi_cb_vd_event_report       (0)
#define mi_cb_vd_vendor			(0)
#define mi_cb_vd_vendor_sts		(0)

#endif

#if MD_CLIENT_EN
int mi_cb_vd_light_onoff_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int mi_cb_vd_light_onoff_status2(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int mi_cb_vd_property_change_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return 1;
}
#else
#define mi_cb_vd_light_onoff_status         (0)
#define mi_cb_vd_light_onoff_status2        (0)
#define mi_cb_vd_property_change_status     (0)
#endif


// ------ both server or GW / APP support --------
int mi_cb_vd_key_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    //model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    vd_mi_rc_key_report_t *p_key = (vd_mi_rc_key_report_t *)par;
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
// ------ end --------


// use unsegment pkt for vendor command should be better.
// = sizeof(mesh_cmd_lt_unseg_t.data) - sizeof(MIC) - sizeof(op) = 15-4-3 = 8
STATIC_ASSERT(sizeof(vd_mi_rc_key_report_t) <= 8);

#if !WIN32
const 
#endif
mesh_cmd_sig_func_t mi_mesh_cmd_vd_func[] = {
	{VD_MI_GET_PROPERTY, 		0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_get_property, 	   	VD_MI_PROPERTY_STS},
    {VD_MI_SET_PROPERTY, 		0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_set_property, 		VD_MI_PROPERTY_STS},
	{VD_MI_SET_PROPERTY_NO_ACK, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_set_property_noack,VD_MI_PROPERTY_STS},
	{VD_MI_PROPERTY_STS, 		1, MIOT_SEPC_VENDOR_MODEL_SER, MIOT_SEPC_VENDOR_MODEL_CLI, mi_cb_vd_property_change_status, STATUS_NONE},
	{VD_MI_ACTION, 				0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_action, STATUS_NONE},
	{VD_MI_RELAY_ACTION, 		0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_reply_action, STATUS_NONE},
	{VD_MI_EVENT_REPORT, 		0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_event_report, STATUS_NONE},
		
	{VD_MI_GW_NODE, 			0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_vendor, STATUS_NONE},
	{VD_MI_NODE_GW,				1, MIOT_SEPC_VENDOR_MODEL_SER, MIOT_SEPC_VENDOR_MODEL_CLI, mi_cb_vd_vendor_sts,STATUS_NONE},


#if 0   // just for sample, default disable, 
  //{VD_RC_KEY_REPORT, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_key_report, STATUS_NONE},
#if DEBUG_VENDOR_CMD_EN
    {VD_MI_LIGHT_ONOFF_SET, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_light_onoff_set, VD_MI_LIGHT_ONOFF_STATUS},
	{VD_MI_LIGHT_ONOFF_GET, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_light_onoff_get, VD_MI_LIGHT_ONOFF_STATUS},
	{VD_MI_LIGHT_ONOFF_SET_NOACK, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_SEPC_VENDOR_MODEL_SER, mi_cb_vd_light_onoff_set, STATUS_NONE},
    {VD_MI_LIGHT_ONOFF_STATUS, 1, MIOT_SEPC_VENDOR_MODEL_SER, MIOT_SEPC_VENDOR_MODEL_CLI, mi_cb_vd_light_onoff_status, STATUS_NONE},
    #if MD_VENDOR_2ND_EN
    {VD_MI_LIGHT_ONOFF_SET2, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_VENDOR_MD_SER, mi_cb_vd_light_onoff_set2, VD_MI_LIGHT_ONOFF_STATUS2},
	{VD_MI_LIGHT_ONOFF_GET2, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_VENDOR_MD_SER, mi_cb_vd_light_onoff_get2, VD_MI_LIGHT_ONOFF_STATUS2},
	{VD_MI_LIGHT_ONOFF_SET_NOACK2, 0, MIOT_SEPC_VENDOR_MODEL_CLI, MIOT_VENDOR_MD_SER, mi_cb_vd_light_onoff_set2, STATUS_NONE},
    {VD_MI_LIGHT_ONOFF_STATUS2, 1, MIOT_VENDOR_MD_SER, MIOT_SEPC_VENDOR_MODEL_CLI, mi_cb_vd_light_onoff_status2, STATUS_NONE},
    #endif
#endif
#endif
};

// don't modify mesh_search_model_id_by_op_vendor()
int mi_mesh_search_model_id_by_op_vendor(mesh_op_resource_t *op_res, u16 op, u8 tx_flag)
{
    foreach_arr(i,mi_mesh_cmd_vd_func){
        if(op == mi_mesh_cmd_vd_func[i].op){
            op_res->cb = mi_mesh_cmd_vd_func[i].cb;
            op_res->op_rsp = mi_mesh_cmd_vd_func[i].op_rsp;
            op_res->sig = 0;
            op_res->status_cmd = mi_mesh_cmd_vd_func[i].status_cmd ? 1 : 0;
            if(tx_flag){
                op_res->id = mi_mesh_cmd_vd_func[i].model_id_tx;
            }else{
                op_res->id = mi_mesh_cmd_vd_func[i].model_id_rx;
            }
            return 0;
        }
    }

    return -1;
}



//--vendor command interface-------------------

int mi_vd_cmd_key_report(u16 adr_dst, u8 key_code)
{
	vd_mi_rc_key_report_t key_report = {0};
	key_report.code = key_code;

	return SendOpParaDebug(adr_dst, 0, VD_MI_RC_KEY_REPORT, (u8 *)&key_report, sizeof(key_report));
}

#if DEBUG_VENDOR_CMD_EN
int mi_vd_cmd_onoff(u16 adr_dst, u8 rsp_max, u8 onoff, int ack)
{
	vd_mi_light_onoff_set_t par = {0};
	par.onoff = onoff;
	par.tid = 0;

	return SendOpParaDebug(adr_dst, rsp_max, ack ? VD_MI_LIGHT_ONOFF_SET : VD_MI_LIGHT_ONOFF_SET_NOACK, 
						   (u8 *)&par, sizeof(vd_mi_light_onoff_set_t));
}
#endif

//--vendor command interface end----------------


int is_mi_cmd_with_tid_vendor(u8 *tid_pos_out, u16 op, u8 tid_pos_vendor_app)
{
    int cmd_with_tid = 0;
    switch(op){
        default:
            break;
            
		case VD_MI_RC_KEY_REPORT:
			break;
            
	#if DEBUG_VENDOR_CMD_EN
        case VD_MI_LIGHT_ONOFF_SET:
        case VD_MI_LIGHT_ONOFF_SET_NOACK:
        #if MD_VENDOR_2ND_EN
        case VD_MI_LIGHT_ONOFF_SET2:
        case VD_MI_LIGHT_ONOFF_SET_NOACK2:
        #endif
            cmd_with_tid = 1;
            *tid_pos_out = OFFSETOF(vd_mi_light_onoff_set_t, tid);
            break;
    #endif
    }

    return cmd_with_tid;
}
#endif

