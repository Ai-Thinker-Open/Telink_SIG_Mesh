/********************************************************************************************************
 * @file     mijia_pub_proc.c 
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

//#include "mijia_pub_proc.h"
#include "./mi_vendor/vendor_model_mi.h"
#if MI_API_ENABLE
#define MI_PUB_MAX			180

#define PUB_POWER_ON_MODE		0
#define PUB_STS_CHANGE_MODE 	1

#define MAX_PUB_WITH_RSP_INTER	3600// 1 hour
#define MAX_PUB_WITHOUT_RSP_INTER	14400 // 4 hour 


mi_pub_str_t mi_pub;

u8 mi_random_1_10()
{
	//create a rand from 1~10
	return rand()%10+1;//(0~9)+1
}

u8 mi_random_10_20()
{
	
	//create a rand from 10~20
	return rand()%11 + 10;//(0~10)+10
}

u8 mi_random_36_50()
{
	return rand()%15+35; // (0~14)+36  
}

u8 mi_random_72_100()
{
	return rand()%29+71;//(0~28)+72
}


// max pub interval proc flow 
u32 get_pub_random_delay_1_1800()
{
	return (rand()%1800)+1;
}


void  mi_inter_sts_change_init(mi_pub_str_t *p_mi_pub)
{
	#if MI_SWITCH_LPN_EN
	p_mi_pub->inter_100ms = mi_random_36_50();
	#else
	p_mi_pub->inter_100ms = mi_random_10_20()*10;
	#endif
	p_mi_pub->last_100ms = clock_time_100ms();
}

u32 mi_get_pub_inter_normal( u32 inter,mi_pub_str_t *p_mi_pub)
{
	inter = inter*2+mi_random_1_10()*10;
	if(inter >= p_mi_pub->max_pub_100ms){
		return p_mi_pub->max_pub_100ms;
	}else{
		return inter;
	}
}

u32 mi_get_pub_inter_lpn(u32 inter,mi_pub_str_t *p_mi_pub)
{
	if(p_mi_pub->mode == PUB_STS_CHANGE_MODE){ // in the mi lpn mode is special 
		if(clock_time_100ms() - p_mi_pub->sts_change_100ms < 60*10){
			return mi_random_72_100();
		}else{ // after 1min ,it need to change step by step 
			return mi_get_pub_inter_normal(inter,p_mi_pub);
		}
		return mi_get_pub_inter_normal(inter,p_mi_pub);
	}else if (p_mi_pub->mode == PUB_POWER_ON_MODE){
		return mi_get_pub_inter_normal(inter,p_mi_pub);
	}
	return 0;
}
u32 mi_get_pub_inter(u32 inter,mi_pub_str_t *p_mi_pub)
{
	#if MI_SWITCH_LPN_EN
	return  mi_get_pub_inter_lpn(inter,p_mi_pub);
	#else
	return mi_get_pub_inter_normal(inter,p_mi_pub);
	#endif
}
void mi_pub_send_onoff_sts(u8 ele_cnt)
{
	model_common_t *p_model;
	for(u8 i=0;i<ele_cnt;i++){
		p_model = &(model_sig_g_onoff_level.onoff_srv[i].com);
		if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}
	return ;
}

void mi_pub_send_ct_sts()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	model_common_t *p_model;
	p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
	if(p_model->cb_pub_st){
	    p_model->cb_pub_st(0);
	}
	p_model = &(model_sig_lightness.srv[0].com);
	if(p_model->cb_pub_st){
	    p_model->cb_pub_st(0);
	}
	p_model = &(model_sig_light_ctl.temp[0].com);
	if(p_model->cb_pub_st){
	    p_model->cb_pub_st(0);
	}
#endif
}

void mi_pub_send_lamp_sts()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	model_common_t *p_model;
	p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
	if(p_model->cb_pub_st){
	    p_model->cb_pub_st(0);
	}
	p_model = &(model_sig_lightness.srv[0].com);
	p_model->cb_pub_st{
	    p_model->cb_pub_st(0);
	}
#endif
}

void mi_pub_send_all_status()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	mi_pub_send_ct_sts();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	mi_pub_send_lamp_sts();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF)
	mi_pub_send_onoff_sts(1);
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF)
	mi_pub_send_onoff_sts(2);
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
	mi_pub_send_onoff_sts(3);
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
	mi_pub_send_onoff_sts(1);
#endif
}
void mi_pub_vd_sig_para_init()
{
	mi_pub_para_init(&mi_pub);
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		mi_pub_str_t *p_pub = &(mi_proper.pub[i]);
		mi_pub_para_init(p_pub);
	}	
}

void mi_pub_update_max_pub(u32 max)
{
	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		mi_pub_str_t *p_pub = &(mi_proper.pub[i]);
		p_pub->max_pub_100ms = max;
	}
}


void mi_pub_para_init(mi_pub_str_t *p_mi_pub)
{
	p_mi_pub->inter_100ms = mi_random_1_10()*10;
	p_mi_pub->last_100ms = clock_time_100ms();
	p_mi_pub->max_pub_100ms = MI_PUB_MAX*10;
	p_mi_pub->mp_inter = get_pub_random_delay_1_1800();
	p_mi_pub->mp_last_sec = clock_time_s();
}

void mi_pub_sigmodel_inter(u8 trans,u8 delay,u8 mode)// only the sigmodel have the delay and trans para 
{
	if(mode == 1){
		u32 remain_ms = 100*get_transition_100ms((trans_time_t *)&trans);
		remain_ms += delay*5;
		
		if(remain_ms > 200){
			mi_pub.sts_pub_inter = (remain_ms/100);
		}else{
			mi_pub.sts_pub_inter = 2;// use 200ms 
		}
	}else{
		mi_pub.sts_pub_inter = 2;// use 200ms 
	}
}


u8 pub_mode_idx =0;
void mi_pub_loopback_onff_msg(u8 ele_cnt)
{
	static u8 pub_model_idx =0;
	model_common_t *p_model;
	p_model = &(model_sig_g_onoff_level.onoff_srv[pub_model_idx%ele_cnt].com);
	if(p_model->cb_pub_st){
	    p_model->cb_pub_st(0);
	}
	pub_model_idx++;
	return;
}
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
void mi_pub_loopback_ct_msg()
{
	model_common_t *p_model;
	if(pub_mode_idx%3 == 0){
		p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
        if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}else if (pub_mode_idx%3 == 1){
		p_model = &(model_sig_lightness.srv[0].com);
        if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}else if (pub_mode_idx%3 == 2){
		p_model = &(model_sig_light_ctl.temp[0].com);
        if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}
	pub_mode_idx++;
}
#endif

#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
void mi_pub_loop_lamp_msg()
{
	model_common_t *p_model;
	if(pub_mode_idx%2 == 0){
		p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
        if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}else if (pub_mode_idx%2 == 1){
		p_model = &(model_sig_lightness.srv[0].com);
        if(p_model->cb_pub_st){
		    p_model->cb_pub_st(0);
		}
	}
	pub_mode_idx++;
}
#endif
void mi_pub_loopback_msg()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	mi_pub_loopback_ct_msg();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	mi_pub_loop_lamp_msg();
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF)
	mi_pub_loopback_onff_msg(1);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF)
	mi_pub_loopback_onff_msg(2);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
	mi_pub_loopback_onff_msg(3);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
	mi_pub_loopback_onff_msg(1);
#endif
}
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
void mi_pub_clear_trans_flag_ct()
{
	model_common_t *p_model;
	p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
	p_model->pub_trans_flag =0;
	p_model = &(model_sig_lightness.srv[0].com);
	p_model->pub_trans_flag =0;
	p_model = &(model_sig_light_ctl.temp[0].com);
	p_model->pub_trans_flag =0;
}
#endif

#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
void mi_pub_clear_trans_flag_lamp()
{
	model_common_t *p_model;
	p_model = &(model_sig_g_onoff_level.onoff_srv[0].com);
	p_model->pub_trans_flag =0;
	p_model = &(model_sig_lightness.srv[0].com);
	p_model->pub_trans_flag =0;
}
#endif

void mi_pub_clear_trans_flag_onff(u8 ele_cnt)
{
	for(u8 i=0;i<ele_cnt;i++){
		model_common_t *p_model = &(model_sig_g_onoff_level.onoff_srv[i].com);
		p_model->pub_trans_flag =0;
	}
	return ;
}

void mi_pub_clear_trans_flag()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	mi_pub_clear_trans_flag_ct();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	mi_pub_clear_trans_flag_lamp();
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF)
	mi_pub_clear_trans_flag_onff(1);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF)
	mi_pub_clear_trans_flag_onff(2);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
	mi_pub_clear_trans_flag_onff(3);
#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
	mi_pub_clear_trans_flag_onff(1);
#endif

}

void mi_vd_sts_change_proc(u8 sts_flag,model_common_t *p_model,mi_pub_str_t *p_mi_pub)
{
	if(sts_flag){
		// need to switch to publish single sts mode,and set the para part 
		p_mi_pub->mode = PUB_STS_CHANGE_MODE;
		mi_inter_sts_change_init(p_mi_pub);
		p_mi_pub->p_model = p_model;
		p_mi_pub->model_idx = 0;
		p_mi_pub->sts_change_100ms = clock_time_100ms();
		p_mi_pub->sts_pub_1st = 1;
		// clear the pub trans flag 
		p_model->pub_trans_flag =0;
		
	}
}

void mi_pub_sts_proc(model_common_t *p_model,mi_pub_str_t *p_mi_pub)
{
	if(clock_time_100ms() - p_mi_pub->last_100ms >= p_mi_pub->inter_100ms){
		// pub const msg 
		if (p_mi_pub->mode == PUB_STS_CHANGE_MODE){// in the vendor mode ,we will only proc the sts change mode
            if((p_mi_pub->p_model)->cb_pub_st){
			    (p_mi_pub->p_model)->cb_pub_st(p_mi_pub->model_idx);
			}
		}	
		p_mi_pub->last_100ms += p_mi_pub->inter_100ms;
		p_mi_pub->inter_100ms = mi_get_pub_inter(p_mi_pub->inter_100ms,p_mi_pub);
		if(p_mi_pub->inter_100ms >= p_mi_pub->max_pub_100ms){
			p_mi_pub->mode = PUB_POWER_ON_MODE;
		}
		
	}
}

void mi_pub_loop_proc_vendormodel(u8 sts_flag,model_common_t *p_model)
{
	mi_pub_str_t *p_mi_pub; 
	static mi_pub_str_t *p_mi_pub_1st; 
	if(sts_flag){
		p_mi_pub = get_mi_pub_by_ssid_piid();
		if(p_mi_pub){
			p_mi_pub_1st = p_mi_pub;
			mi_vd_sts_change_proc(sts_flag,p_model,p_mi_pub);
			mi_proper.ssid_sts_change = mi_proper.ssid_now;
			mi_proper.piid_sts_change = mi_proper.piid_now;
		}
	}
	if(p_mi_pub_1st->sts_pub_1st &&
		(clock_time_100ms() - p_mi_pub_1st->sts_change_100ms >=2))// wait for about 200ms ,need to send sts 
	{
		mi_set_pub_ssid_piid_now(mi_proper.ssid_sts_change,mi_proper.piid_sts_change);
		(p_mi_pub_1st->p_model)->cb_pub_st(p_mi_pub_1st->model_idx);
		p_mi_pub_1st->sts_pub_1st =0;
	}

	for(int i=0;i<MI_MAX_PROPER_CNT;i++){
		p_mi_pub = &(mi_proper.pub[i]);
		// we need to change ssid and piid when pub the sts 
		vd_mi_head_str *p_mi_head = (vd_mi_head_str *)&(mi_proper.proper_data[i].mi_head);
		if(mi_pub_ssid_piid_is_valid(p_mi_head->ssid,p_mi_head->piid)){
			mi_set_pub_ssid_piid_now(p_mi_head->ssid,p_mi_head->piid);
			mi_pub_sts_proc(p_model,p_mi_pub);
		}
	}
}
	
void mi_pub_loop_proc_sigmodel(u8 sts_flag,model_common_t *p_model,mi_pub_str_t *p_mi_pub)
{
	if(sts_flag){
		// need to switch to publish single sts mode,and set the para part 
		p_mi_pub->mode = PUB_STS_CHANGE_MODE;
		mi_inter_sts_change_init(p_mi_pub);
		p_mi_pub->p_model = p_model;
		p_mi_pub->model_idx = 0;
		p_mi_pub->sts_change_100ms = clock_time_100ms();
		p_mi_pub->sts_pub_1st = 1;
		// clear the pub trans flag 
		p_model->pub_trans_flag =0;
	}
	if(p_mi_pub->sts_pub_1st &&
		(clock_time_100ms() - p_mi_pub->sts_change_100ms >=p_mi_pub->sts_pub_inter))// wait for about 200ms ,need to send sts 
	{
		(p_mi_pub->p_model)->cb_pub_st(p_mi_pub->model_idx);
		p_mi_pub->sts_pub_1st =0;
	}
	
	if(clock_time_100ms() - p_mi_pub->last_100ms >= p_mi_pub->inter_100ms){
		// pub const msg 
		if(p_mi_pub->mode == PUB_POWER_ON_MODE){
			mi_pub_loopback_msg();			
		}else if (p_mi_pub->mode == PUB_STS_CHANGE_MODE){
            if((p_mi_pub->p_model)->cb_pub_st){
			    (p_mi_pub->p_model)->cb_pub_st(p_mi_pub->model_idx);
			}
		}
		p_mi_pub->last_100ms += p_mi_pub->inter_100ms;
		p_mi_pub->inter_100ms = mi_get_pub_inter(p_mi_pub->inter_100ms,p_mi_pub);
		if(p_mi_pub->inter_100ms >= p_mi_pub->max_pub_100ms){
			p_mi_pub->mode = PUB_POWER_ON_MODE;
		}		
	}
}




// proc the pub sending msg part 
void mi_pub_loop_proc(u8 sts_flag,model_common_t *p_model)
{
#if 1
	if(p_model == &(model_vd_light.srv[0].com)){
		mi_pub_loop_proc_vendormodel(sts_flag,p_model);
	}else{
		mi_pub_loop_proc_sigmodel(sts_flag,p_model,&mi_pub);
	}
#else
	mi_pub_loop_proc_sigmodel(sts_flag,p_model,&mi_pub);
#endif
}

u8 is_model_suit_to_pub(u32 mode_id)
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	const u32 model_white_list[3]={SIG_MD_G_ONOFF_S,SIG_MD_LIGHTNESS_S,SIG_MD_LIGHT_CTL_TEMP_S};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	const u32 model_white_list[2]={SIG_MD_G_ONOFF_S,SIG_MD_LIGHTNESS_S};
#elif (	MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF ||\
		MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF ||\
		MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF ||\
		MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
	const u32 model_white_list[1]={SIG_MD_G_ONOFF_S};
#endif
	foreach_arr(i,model_white_list){
		if(mode_id == model_white_list[i] || mode_id ==  MIOT_SEPC_VENDOR_MODEL_SER){// modify to allow vendor model pub
			return 1;
		}
	}
	return 0;
}

// when receive the rsp from the gateway ,we will set the max inter loop 
void pub_max_inter_rcv_cb(u8 max_pub_min)
{
	mi_pub.mp_rsp_flag = 1;
	mi_pub.max_pub_100ms = max_pub_min*60*10;
	mi_pub_update_max_pub(mi_pub.max_pub_100ms);
}

void pub_max_inter_loop_proc(mi_pub_str_t *p_mi_pub)
{
	if(clock_time_s() - p_mi_pub->mp_last_sec > p_mi_pub->mp_inter){
		p_mi_pub->mp_last_sec += p_mi_pub->mp_inter;
		// send max pub inter request 
		static u8 mi_pub_net_para_tid = 0;
		u8 req[3];
		req[0] = MI_NET_PARA_REQ;
		req[1] = p_mi_pub->retry_cnt;
		req[2] = ++mi_pub_net_para_tid;
		mesh_tx_cmd2normal_primary(VD_MI_NODE_GW, req, sizeof(req),MI_MESH_PUB_ADR,0);
		p_mi_pub->retry_cnt++;
		// set the send flag ,and wait for the rsp
		p_mi_pub->mp_send_flag = 1;
		p_mi_pub->mp_rsp_flag = 0;
	}

	// refresh the flag part ,wait 3s to update the flag part 
	if(p_mi_pub->mp_send_flag && (clock_time_s() - p_mi_pub->mp_last_sec >3)){
		p_mi_pub->mp_send_flag = 0;
		if(p_mi_pub->mp_rsp_flag){
			p_mi_pub->mp_inter = MAX_PUB_WITH_RSP_INTER;
		}else{
			p_mi_pub->mp_inter = MAX_PUB_WITHOUT_RSP_INTER;
		}
	}
}


#endif
// return 0 means can proc normally ,return 1 means can do step pub proc 
u8 pub_step_proc_cb(u8 sts_flag,model_common_t *p_model,u32 model_id)
{
	#if !STEP_PUB_MODE_EN
		return 0;
	#else
	// only proved we will allow to send the publish cmd 
	if(!is_provision_success()){
		return 0;
	}
	if(!is_model_suit_to_pub(model_id)){
		return 0;
	}
	mi_pub_loop_proc(sts_flag,p_model);
	pub_max_inter_loop_proc(&mi_pub);
	return 1;
	#endif
}



