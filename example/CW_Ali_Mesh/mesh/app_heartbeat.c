/********************************************************************************************************
 * @file     app_heartbeat.c 
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
#include "app_heartbeat.h"

u8 heartbeat_en =1;
u8 hb_sts_change = 0;
u32 hb_pub_100ms =0;
u32 hb_sub_100ms =0;

void mesh_cmd_sig_lowpower_heartbeat()
{
	if(model_sig_cfg_s.hb_pub.feature & BIT(MESH_HB_LOWPOWER_BIT)){
		hb_sts_change = 1;
		hb_pub_100ms = clock_time_100ms();
	}
}

// heart beat part dispatch 
int mesh_cmd_sig_heart_pub_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	// reserve to diaptch the status 
	int err = -1;
	if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

int mesh_cmd_sig_heartbeat_pub_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
	mesh_heartbeat_pub_status_t heart_pub_status;
	if(mesh_net_key_find(model_sig_cfg_s.hb_pub.netkeyidx) == NET_KEY_MAX){
		heart_pub_status.status = ST_INVALID_NETKEY;
	}else{
		heart_pub_status.status = ST_SUCCESS;
	}
	heart_pub_status.dst = model_sig_cfg_s.hb_pub.dst_adr;
	heart_pub_status.countlog = model_sig_cfg_s.hb_pub.cnt_log;
	heart_pub_status.periodlog = model_sig_cfg_s.hb_pub.per_log;
	heart_pub_status.ttl = model_sig_cfg_s.hb_pub.ttl;
	heart_pub_status.feature = model_sig_cfg_s.hb_pub.feature;
	//heart_pub_status.netkeyIndex = mesh_key.net_key[mesh_key.netkey_sel_dec][0].index;
	heart_pub_status.netkeyIndex = model_sig_cfg_s.hb_pub.netkeyidx;
	err = mesh_tx_cmd_rsp_cfg_model(HEARTBEAT_PUB_STATUS,(u8 *)(&heart_pub_status.status),sizeof(heart_pub_status),cb_par->adr_src);
    return err;
}
u16 get_cnt_log_to_val(u8 val)
{
	if((val == 0)||(val>0x11 && val <0xff)){
		return 0;
	}else if(val == 0x11){
		return 0xfffe;
	}else if(val == 0xff){
		return 0xffff;
	}else{
		return 1<<(val-1);
	}

}
int mesh_cmd_sig_heartbeat_pub_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
	mesh_heartbeat_pub_str *p_pub = &(model_sig_cfg_s.hb_pub);
	mesh_cfg_model_heartbeat_pub_get_t *p_pub_get = (mesh_cfg_model_heartbeat_pub_get_t *)(par);
	p_pub->dst_adr = p_pub_get->dst;
	p_pub->cnt_log = p_pub_get->count_log;
	p_pub->cnt_val = get_cnt_log_to_val(p_pub_get->count_log);
	p_pub->per_log = p_pub_get->period_log;
	p_pub->ttl = p_pub_get->ttl;
	p_pub->feature = (p_pub_get->features)&0x000f;
	p_pub->netkeyidx = p_pub_get->netkeyindex;
	// leave the key index settings .
	//rsp the status 
	mesh_cmd_sig_heartbeat_pub_get(par,par_len,cb_par);
#if TESTCASE_FLAG_ENABLE
	hb_pub_100ms = clock_time_100ms(); // test case required, because count in response of get command, should be same with set command.
#else
	hb_pub_100ms = clock_time_100ms()-BIT(31);  // Ali required, send once at once after received command.
#endif
	mesh_common_store(FLASH_ADR_MD_CFG_S);
	return err;
}

int mesh_cmd_sig_heartbeat_sub_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
	mesh_cfg_model_heartbeat_sub_status_t sub_status;
	mesh_heartbeat_sub_str *p_cfg_hb_sub =&(model_sig_cfg_s.hb_sub);
	sub_status.status = ST_SUCCESS;

	memcpy((u8 *)(&sub_status)+1,(u8 *)(p_cfg_hb_sub),sizeof(mesh_cfg_model_heartbeat_sub_status_t)-1);
	if(	p_cfg_hb_sub->adr == 0||
		p_cfg_hb_sub->dst == 0){
		if(cb_par->op == HEARTBEAT_SUB_GET){
			memset((u8 *)(&sub_status)+1,0,8);
		}
	}
	err = mesh_tx_cmd_rsp_cfg_model(HEARTBEAT_SUB_STATUS,(u8 *)(&sub_status.status),sizeof(sub_status),cb_par->adr_src);
	return err;
}
int mesh_cmd_sig_heartbeat_sub_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
	mesh_cfg_model_heartbeat_sub_set_t *p_sub_set = (mesh_cfg_model_heartbeat_sub_set_t *)(par);
	mesh_heartbeat_sub_str *p_cfg_hb_sub =&(model_sig_cfg_s.hb_sub);
	if(is_unicast_adr(p_sub_set->src)||p_sub_set->src == 0){
		p_cfg_hb_sub->adr = p_sub_set->src;
	}else{
		return err;
	}
	
	if((p_sub_set->dst == ele_adr_primary) || (is_group_adr(p_sub_set->dst) || (p_sub_set->dst==0))){
		p_cfg_hb_sub->dst = p_sub_set->dst;
	}
	else{
		return err;
	}
	p_cfg_hb_sub->per_log = p_sub_set->period_log;
	
	if((p_sub_set->src==0) || (p_sub_set->dst==0)){
		p_cfg_hb_sub->adr = 0;
		p_cfg_hb_sub->dst = 0;
		p_cfg_hb_sub->per_log = 0;
		p_cfg_hb_sub->cnt_log =0;
		p_cfg_hb_sub->cnt_val =0;
	}
	if( is_unicast_adr(p_sub_set->src)&&
		(is_group_adr(p_sub_set->dst)||is_unicast_adr(p_sub_set->dst))&&
		(p_sub_set->period_log!=0)){
		p_cfg_hb_sub->cnt_log =0;
		p_cfg_hb_sub->cnt_val =0;
		p_cfg_hb_sub->min_hops = 0x7f;
		p_cfg_hb_sub->max_hops = 0;
	}
	
	p_cfg_hb_sub->per_val = get_cnt_log_to_val(p_cfg_hb_sub->per_log);
	
	// how to rsp 
	mesh_common_store(FLASH_ADR_MD_CFG_S);
	mesh_cmd_sig_heartbeat_sub_get(par,par_len,cb_par);
	hb_sub_100ms = clock_time_100ms();
	return err;
}

int mesh_cmd_sig_heartbeat_sub_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
	if(cb_par->model){  // model may be Null for status message
    }
	return err;
}

void mesh_process_hb_sub(mesh_cmd_bear_unseg_t *p_bear)
{
	mesh_cmd_nw_t *p_nw = &(p_bear->nw);
	mesh_heartbeat_sub_str *p_sub = &(model_sig_cfg_s.hb_sub);
	#if WIN32 
	mesh_heartbeat_cb_data(p_nw->src, p_nw->dst,(u8 *)&(p_bear->lt_ctl_unseg));
	#endif
	#if GATEWAY_ENABLE
	mesh_hb_msg_cb_t hb_par;
	hb_par.src = p_nw->src;
	hb_par.dst = p_nw->dst;
	memcpy(&hb_par.hb_msg, p_bear->lt_ctl_unseg.data, sizeof(mesh_hb_msg_t));
	gateway_heartbeat_cb((u8 *)&hb_par,sizeof(mesh_hb_msg_cb_t));
	#endif
	if(p_sub->dst == 0 || p_sub->adr == 0 || (p_sub->per_log == 0)){
		return ;
	}
	if(p_sub->adr != p_nw->src ){
		return ;
	}
	// increase the count log part 
	mesh_hb_msg_t *p_hb =(mesh_hb_msg_t *)(p_bear->lt_ctl_unseg.data);
	// swap the ttl part 
	endianness_swap_u16((u8 *)(&p_hb->fea));
	if(p_hb->iniTTL){
		p_sub->min_hops = 1;
	}else{
		p_sub->min_hops = 0;
	}
	p_sub->max_hops = p_hb->iniTTL- (p_bear->nw.ttl)+1;
	p_sub->cnt_val++;
	p_sub->cnt_log = cal_heartbeat_count_log(p_sub->cnt_val);
}

u16 mesh_heartbeat_cal_feature_part(u16 fea)
{
	u16 ret =0;
	if(model_sig_cfg_s.hb_pub.feature & BIT(MESH_HB_RELAY_BIT)){
		if(model_sig_cfg_s.relay){
			ret |= BIT(MESH_HB_RELAY_BIT);
		}
	}
	if(model_sig_cfg_s.hb_pub.feature & BIT(MESH_HB_PROXY_BIT)){
		if(model_sig_cfg_s.gatt_proxy){
			ret |= BIT(MESH_HB_PROXY_BIT);
		}
	}
	if(model_sig_cfg_s.hb_pub.feature & BIT(MESH_HB_FRI_BIT)){
		if(model_sig_cfg_s.frid){
			ret |= BIT(MESH_HB_FRI_BIT);
		}
	}
	#if FEATURE_LOWPOWER_EN
	if(model_sig_cfg_s.hb_pub.feature & BIT(MESH_HB_LOWPOWER_BIT)){
		if(mesh_lpn_par.link_ok){
			ret |=  BIT(MESH_HB_LOWPOWER_BIT);
		}else{
			ret &= ~(BIT(MESH_HB_LOWPOWER_BIT));
		}
	}
	#endif
	return ret;
}

void heartbeat_cmd_send_conf(u8 ttl,u16 feature,u16 dst)
{
    mesh_hb_msg_t hb_msg;
	hb_msg.rfu = 0;
	hb_msg.iniTTL = ttl;
	hb_msg.fea = mesh_heartbeat_cal_feature_part(feature);
	
	
	mesh_tx_cmd_layer_upper_ctl(CMD_CTL_HEARTBEAT, (u8 *)(&hb_msg), sizeof(hb_msg), ele_adr_primary, dst,0);
	LOG_MSG_INFO(TL_LOG_MESH, 0, 0,"send heartbeat ",0);
	return ;
}

// mesh send heartbeat message 
void mesh_heartbeat_sub_poll()
{
	// dispatch the heartbeat subscription part 
	mesh_heartbeat_sub_str *p_sub = &(model_sig_cfg_s.hb_sub);
	if(p_sub->per_val){
    	u32 heartbeat_sub_per =0;
    	if(p_sub->per_log == 0){
    		heartbeat_sub_per = 0;
    	}else if (p_sub->per_log <=0x11){
    		heartbeat_sub_per = (1<<(p_sub->per_log-1));
    	}else{
    		return ;
    	}
    	
    	if(clock_time_exceed_100ms(hb_sub_100ms,heartbeat_sub_per*10)){
    		hb_sub_100ms = clock_time_100ms();
    		p_sub->per_val-- ;
    		p_sub->per_log = cal_heartbeat_count_log(p_sub->per_val);
    	}
	}
}

void mesh_heartbeat_pub_poll()
{
	// dispatch the heartbeat publication part 
	mesh_heartbeat_pub_str *p_pub = &(model_sig_cfg_s.hb_pub);
	if(p_pub->dst_adr){
    	if(mesh_net_key_find(p_pub->netkeyidx)== NET_KEY_MAX){
    		return ;
    	}

    	u32 heartbeat_pb_per =0;
    	if(p_pub->per_log == 0){
    		heartbeat_pb_per = 1;
    	}else if (p_pub->per_log <=0x11){
    		heartbeat_pb_per = (1<<(p_pub->per_log-1));
    	}else{
    		return ;
    	}
    	if(p_pub->cnt_val && (!hb_sts_change)
    	&& clock_time_exceed_100ms(hb_pub_100ms,heartbeat_pb_per*10)){
    		hb_pub_100ms = clock_time_100ms();
    		if(p_pub->cnt_log == 0xff){
    			// infinite 
    		}else if (p_pub->cnt_log <= 0x11){
    			p_pub->cnt_val--;
    			p_pub->cnt_log = cal_heartbeat_count_log(p_pub->cnt_val);
    		}else{
    			//prohibit
    			return ;
    		}
    		friend_cmd_send_fn(0, CMD_CTL_HEARTBEAT);
    	}	
    	
    	if(hb_sts_change && clock_time_exceed(hb_pub_100ms,1)){
    		hb_pub_100ms = clock_time_100ms();
    		hb_sts_change--;
    		friend_cmd_send_fn(0, CMD_CTL_HEARTBEAT);
    	}
	}
}

void mesh_heartbeat_poll_100ms()
{
    if(!heartbeat_en){
        return ;
    }
	mesh_heartbeat_sub_poll();
	mesh_heartbeat_pub_poll();
}

#define HEARTBEAT_TTL_VALUE		2
#define HEARTBEAT_MIN_HOPS		0x01
#define HEARTBEAT_MAX_HOPS		0x7f

void set_heartbeat_feature()
{
	//set the heartbeat feature enable by the feature 
	mesh_page_feature_t *p_ft = (mesh_page_feature_t *)&model_sig_cfg_s.hb_pub.feature;
	p_ft->relay = is_relay_support_and_en;
	p_ft->proxy = is_proxy_support_and_en;
	p_ft->frid = is_fn_support_and_en;
	p_ft->low_power = is_lpn_support_and_en;
}
u8 get_high_bits(u16 val)
{
	u8 i;
	for(i=0;i<16;i++){
		if(val<BIT(i)){
			return i-1;
		}
	}
	return 15;
}
u8 cal_heartbeat_count_log(u16 val)
{
	if(val ==0){
		return 0;
	}else if(val ==0xffff){
		return 0xff;
	}else{
		return (get_high_bits(val)+1);
	}
}

u8 disptch_heartbeat_count_log(u8 val)
{
	if(val>=12 && val<=0xfe){
		return 0;
	}else{
		return 1;
	}
	return 0;
}

u8 dispatch_heartbeat_pub_ttl(u8 val)
{
	if(val>=0x80){
		return 0;
	}else{
		return 1;
	}
}
void init_heartbeat_str()
{
	// need to trigger the hearbeat msg ,at the first time 
	hb_pub_100ms = clock_time_100ms()- BIT(31);//reserve 32767s

	// heartbeat publication 
	/*
	model_sig_cfg_s.hb_pub.dst_adr = 0xffff;
	model_sig_cfg_s.hb_pub.cnt_val = 0x0000;
	model_sig_cfg_s.hb_pub.per_log = 3;
	model_sig_cfg_s.hb_pub.cnt_log = cal_heartbeat_count_log(model_sig_cfg_s.hb_pub.cnt_val);
	model_sig_cfg_s.hb_pub.ttl = HEARTBEAT_TTL_VALUE;
	set_heartbeat_feature();
	// heartbeat subscription 
	model_sig_cfg_s.hb_sub.adr = 0;
	model_sig_cfg_s.hb_sub.dst = ele_adr_primary;
	model_sig_cfg_s.hb_sub.cnt_val= 0;
	model_sig_cfg_s.hb_sub.cnt_log =cal_heartbeat_count_log(model_sig_cfg_s.hb_sub.cnt_val); 
	model_sig_cfg_s.hb_sub.per_val = 0;
	model_sig_cfg_s.hb_sub.per_log = cal_heartbeat_count_log(model_sig_cfg_s.hb_sub.per_val);
	model_sig_cfg_s.hb_sub.min_hops = 0;
	model_sig_cfg_s.hb_sub.max_hops = 0;

	model_sig_cfg_s.hb_sub.state=1;
	model_sig_cfg_s.hb_sub.tick =clock_time();
	*/
	return ;
}

