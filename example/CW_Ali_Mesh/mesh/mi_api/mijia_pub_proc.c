#include "mijia_pub_proc.h"
#if MI_API_ENABLE
#define MI_PUB_MAX			180

#define PUB_POWER_ON_MODE		0
#define PUB_STS_CHANGE_MODE 	1

#define MAX_PUB_WITH_RSP_INTER	3600// 1 hour
#define MAX_PUB_WITHOUT_RSP_INTER	14400 // 4 hour 

typedef struct{
	u8 mode;
	u8 model_idx;
	u8 rfu[2];
	// the unit is 100ms
	u32 last_100ms;
	u32 inter_100ms;
	u32 max_pub_100ms;
	u32 sts_change_100ms;
	model_common_t * p_model;
	// max pub units is s;
	u32 mp_inter;
	u32 mp_last_sec;
	u8 mp_rsp_flag;
	u8 mp_send_flag;
	u8 retry_cnt;
}mi_pub_str_t;
mi_pub_str_t mi_pub_t;

u8 mi_random_1_10()
{
	//create a rand from 1~10
	return rand()%11;
}

u8 mi_random_10_20()
{
	
	//create a rand from 10~20
	return rand()%12 + 9;
}

u8 mi_random_36_50()
{
	return rand()%16+35; // (1~15)+35  
}

u8 mi_random_72_100()
{
	return rand()%30+71;//(1~29)+71
}


// max pub interval proc flow 
u32 get_pub_random_delay_1_1800()
{
	return rand()%1801;
}


void  mi_inter_sts_change_init()
{
	#if MI_SWITCH_LPN_EN
	mi_pub_t.inter_100ms = mi_random_36_50();
	#else
	mi_pub_t.inter_100ms = mi_random_10_20()*10;
	#endif
	mi_pub_t.last_100ms = clock_time_100ms();
}

u32 mi_get_pub_inter_normal( u32 inter)
{
	inter = inter*2+mi_random_1_10()*10;
	if(inter >= mi_pub_t.max_pub_100ms){
		return mi_pub_t.max_pub_100ms;
	}else{
		return inter;
	}
}

u32 mi_get_pub_inter_lpn(u32 inter)
{
	if(mi_pub_t.mode == PUB_STS_CHANGE_MODE){ // in the mi lpn mode is special 
		if(clock_time_100ms() - mi_pub_t.sts_change_100ms < 60*10){
			return mi_random_72_100();
		}else{ // after 1min ,it need to change step by step 
			return mi_get_pub_inter_normal(inter);
		}
	}else if (mi_pub_t.mode == PUB_POWER_ON_MODE){
		return mi_get_pub_inter_normal(inter);
	}

}
u32 mi_get_pub_inter(u32 inter)
{
	#if MI_SWITCH_LPN_EN
	return  mi_get_pub_inter_lpn(inter);
	#else
	return mi_get_pub_inter_normal(inter);
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
#endif
}

void mi_pub_para_init()
{
	mi_pub_t.inter_100ms = mi_random_1_10()*10;
	mi_pub_t.last_100ms = clock_time_100ms();
	mi_pub_t.max_pub_100ms = MI_PUB_MAX*10;
	mi_pub_t.mp_inter = get_pub_random_delay_1_1800();
	mi_pub_t.mp_last_sec = clock_time_s();
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
#endif

}


// proc the pub sending msg part 
void mi_pub_loop_proc(u8 sts_flag,model_common_t *p_model)
{
	if(sts_flag){
		// need to switch to publish single sts mode,and set the para part 
		mi_pub_t.mode = PUB_STS_CHANGE_MODE;
		mi_inter_sts_change_init();
		mi_pub_t.p_model = p_model;
		mi_pub_t.model_idx = 0;
		mi_pub_t.sts_change_100ms = clock_time_100ms();
		// clear the pub trans flag 
		p_model->pub_trans_flag =0;
	}
	if(clock_time_100ms() - mi_pub_t.last_100ms >= mi_pub_t.inter_100ms){
		mi_pub_t.last_100ms += mi_pub_t.inter_100ms;
		mi_pub_t.inter_100ms = mi_get_pub_inter(mi_pub_t.inter_100ms);
		if(mi_pub_t.inter_100ms >= mi_pub_t.max_pub_100ms){
			mi_pub_t.mode = PUB_POWER_ON_MODE;
		}
		// pub const msg 
		if(mi_pub_t.mode == PUB_POWER_ON_MODE){
			mi_pub_loopback_msg();			
		}else if (mi_pub_t.mode == PUB_STS_CHANGE_MODE){
            if((mi_pub_t.p_model)->cb_pub_st){
			    (mi_pub_t.p_model)->cb_pub_st(mi_pub_t.model_idx);
			}
		}	
	}
}

u8 is_model_suit_to_pub(u32 mode_id)
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	const u32 model_white_list[3]={SIG_MD_G_ONOFF_S,SIG_MD_LIGHTNESS_S,SIG_MD_LIGHT_CTL_TEMP_S};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	const u32 model_white_list[2]={SIG_MD_G_ONOFF_S,SIG_MD_LIGHTNESS_S};
#elif (	MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF ||\
		MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF ||\
		MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
	const u32 model_white_list[1]={SIG_MD_G_ONOFF_S};
#endif
	foreach_arr(i,model_white_list){
		if(mode_id == model_white_list[i] ){
			return 1;
		}
	}
	return 0;
}

// when receive the rsp from the gateway ,we will set the max inter loop 
void pub_max_inter_rcv_cb(u8 max_pub_min)
{
	mi_pub_t.mp_rsp_flag = 1;
	mi_pub_t.max_pub_100ms = max_pub_min*60*10;
}

void pub_max_inter_loop_proc()
{
	if(clock_time_s() - mi_pub_t.mp_last_sec > mi_pub_t.mp_inter){
		mi_pub_t.mp_last_sec += mi_pub_t.mp_inter;
		// send max pub inter request 
		u8 req[2];
		req[0] = MI_NET_PARA_REQ;
		req[1] = mi_pub_t.retry_cnt;
		mesh_tx_cmd2normal_primary(VD_MI_NODE_GW, req, sizeof(req),MI_MESH_PUB_ADR,0);
		mi_pub_t.retry_cnt++;
		// set the send flag ,and wait for the rsp
		mi_pub_t.mp_send_flag = 1;
		mi_pub_t.mp_rsp_flag = 0;
	}

	// refresh the flag part ,wait 3s to update the flag part 
	if(mi_pub_t.mp_send_flag && (clock_time_s() - mi_pub_t.mp_last_sec >3)){
		mi_pub_t.mp_send_flag = 0;
		if(mi_pub_t.mp_rsp_flag){
			mi_pub_t.mp_inter = MAX_PUB_WITH_RSP_INTER;
		}else{
			mi_pub_t.mp_inter = MAX_PUB_WITHOUT_RSP_INTER;
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
	pub_max_inter_loop_proc();
	return 1;
	#endif
}



