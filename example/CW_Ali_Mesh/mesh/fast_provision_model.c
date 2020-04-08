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
#include "mesh/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "fast_provision_model.h"
#include "version.h"

#define FAST_PROVISION_TIMEOUT 		60*1000*1000
#if FAST_PROVISION_ENABLE
//----------------------fast provision proc---------------------------
fast_prov_par_t fast_prov;
#endif

void mesh_gatt_adv_beacon_enable(u8 enable){
	#if !WIN32
	beacon_send.en = enable;
	gatt_adv_send_flag = enable;
	#endif
}

int mesh_reset_network(u8 provision_enable)
{
	if(!is_provision_success()){
		return 1;
	}
	u8 r = irq_disable ();
	factory_test_mode_en = 1;
	provision_mag.gatt_mode = GATT_PROVISION_MODE;
	cache_init(ADR_ALL_NODES);
	mesh_provision_para_reset();

//init iv idx
	u8 iv_idx_cur[4] = IV_IDX_CUR;
	memcpy(iv_idx_st.cur, iv_idx_cur, 4);
	mesh_iv_idx_init(iv_idx_st.cur, 0);
//init model info
	mesh_common_reset_all();
	if(!provision_enable){
		mesh_gatt_adv_beacon_enable(0);
	}else{
		mesh_gatt_adv_beacon_enable(1);	
		provision_mag.gatt_mode = GATT_PROVISION_MODE;// because retrive in mesh_common_reset_all
	}
//att table
	#if !WIN32
	my_att_init (provision_mag.gatt_mode);
	#endif

//set default key
	memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
	mesh_init_flag = 1;
	factory_test_key_bind(1);
	mesh_init_flag = 0;
//provision random
//	provision_random_data_init(); // will init in provisioning

// spirit mode init
	#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
	beacon_str_init();
	sha256_dev_uuid_str *p_uuid = (sha256_dev_uuid_str *)prov_para.device_uuid;
	p_uuid->adv_flag = 0;
	#endif	
	
	irq_restore(r);
	return 0;
}

void mesh_revert_network()
{
	node_need_store_misc = 1;
	if(provision_mag.gatt_mode == GATT_PROVISION_MODE){		
		#if FAST_PROVISION_ENABLE
		if((!fast_prov.not_need_prov)&&(mesh_fast_prov_sts_get() == FAST_PROV_COMPLETE)){
			mesh_provision_par_handle((u8 *)&fast_prov.net_info.pro_data);
			//set app_key
			mesh_appkey_set_t *p_set = (mesh_appkey_set_t *)&fast_prov.net_info.appkey_set;
			mesh_app_key_set(APPKEY_ADD, p_set->appkey, GET_APPKEY_INDEX(p_set->net_app_idx), GET_NETKEY_INDEX(p_set->net_app_idx), 1);

			if(get_all_appkey_cnt() == 1){
			u16 app_key_idx = GET_NETKEY_INDEX(p_set->net_app_idx);
			 ev_handle_traversal_cps(EV_TRAVERSAL_BIND_APPKEY, (u8 *)&app_key_idx);
			}

			rf_link_light_event_callback(LGT_CMD_SET_MESH_INFO);
		}
		else
		#endif
		{
			memset(mesh_key.net_key, 0, sizeof(mesh_key_t)-OFFSETOF(mesh_key_t,net_key));
			mesh_flash_retrieve();	
			mesh_provision_para_init();
		}

		#if WIN32
		App_key_bind_end_callback(MESH_APP_KEY_BIND_EVENT_SUC); 
		#endif
		mesh_gatt_adv_beacon_enable(1);
		mesh_node_init();
		
		#if FAST_PROVISION_ENABLE
		mesh_fast_prov_val_init();
		#endif
	}
}

#if !(ANDROID_APP_ENABLE || IOS_APP_ENABLE)
u8 mesh_fast_prov_get_ele_cnt_callback(u16 pid)
{
	u8 node_ele_cnt = 1;
	switch(pid){
	case LIGHT_TYPE_CT:
		node_ele_cnt = 2;
		break;
	case LIGHT_TYPE_HSL:
		node_ele_cnt = 3;
		break;
	case LIGHT_TYPE_XYL:
		node_ele_cnt = 3;
		break;
	case LIGHT_TYPE_CT_HSL:
		node_ele_cnt = 4;
		break;
	case LIGHT_TYPE_PANEL:
		break;
	default:
		break;
	}
	return node_ele_cnt;
}
#endif

#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
void mesh_fast_prov_start(u16 pid)
{
	fast_prov.pid = pid;
	fast_prov.cur_sts = fast_prov.last_sts = FAST_PROV_START;
	fast_prov.start_tick = clock_time()|1;
	fast_prov.pending = 0;

// set network info
	fast_prov.net_info.pro_data.flags = 0;
	memcpy(fast_prov.net_info.pro_data.iv_index, iv_idx_st.cur, 4);
	u8 nk_array_idx = get_nk_arr_idx_first_valid();
    u8 ak_array_idx = get_ak_arr_idx_first_valid(nk_array_idx);
	fast_prov.net_info.pro_data.key_index = mesh_key.net_key[nk_array_idx][0].index;
	memcpy(fast_prov.net_info.pro_data.net_work_key, mesh_key.net_key[nk_array_idx][0].key, 16);
	fast_prov.net_info.pro_data.unicast_address = ele_adr_primary;
	#if WIN32
	fast_prov.prov_addr = get_win32_prov_unicast_adr();
	#endif

	u32 net_app_idx = (fast_prov.net_info.pro_data.key_index&0x0FFF) | (mesh_key.net_key[nk_array_idx][0].app_key[ak_array_idx].index<<12);
	memcpy(fast_prov.net_info.appkey_set.net_app_idx, &net_app_idx, 3);
	memcpy(fast_prov.net_info.appkey_set.appkey, mesh_key.net_key[nk_array_idx][0].app_key[ak_array_idx].key, 16);

	cache_init(ADR_ALL_NODES);
}

#define CACHE_MAC_MAX_NUM 	32
u8 fast_prov_w_idx = 0;
u8 fast_prov_r_idx = 0;
u8 fast_prov_retry_cnt  = 0;
fast_prov_mac_st fast_prov_mac_buf[CACHE_MAC_MAX_NUM];

void mesh_fast_prov_reliable_finish_handle()
{
#if (FAST_PROVISION_ENABLE)
	switch(fast_prov.cur_sts){
		case FAST_PROV_IDLE:
			break;
		case FAST_PROV_GET_ADDR:
		case FAST_PROV_GET_ADDR_RETRY:
			if(mesh_tx_reliable.mat.op == VD_MESH_ADDR_GET){
				if(fast_prov_w_idx == 0){
					if(fast_prov.cur_sts == FAST_PROV_GET_ADDR){
						mesh_fast_prov_sts_set(FAST_PROV_GET_ADDR_RETRY);
					}
					else{
						mesh_fast_prov_sts_set(FAST_PROV_NET_INFO);
					}
				}
				else{
					mesh_fast_prov_sts_set(FAST_PROV_SET_ADDR);
				}
			}
			break;
		case FAST_PROV_SET_ADDR:
			if(mesh_tx_reliable.mat.op == VD_MESH_ADDR_SET){				
				// for default address conflict, assume set addr must succcess
				u8 device_key[16];
				memset(device_key, 0x00, sizeof(device_key));
				memcpy(device_key, fast_prov_mac_buf[fast_prov_r_idx-1].mac, OFFSETOF(fast_prov_mac_st,pid));
				
				#if  (ANDROID_APP_ENABLE || IOS_APP_ENABLE)
				mesh_fast_prov_node_info_callback(device_key, fast_prov.prov_addr, fast_prov_mac_buf[fast_prov_r_idx-1].pid);				
				#endif
				u8 node_ele_cnt = mesh_fast_prov_get_ele_cnt_callback(fast_prov_mac_buf[fast_prov_r_idx-1].pid);
				VC_node_dev_key_save(fast_prov.prov_addr, device_key, node_ele_cnt);
				fast_prov.prov_addr += node_ele_cnt;
				#if WIN32
				set_win32_prov_unicast_adr(fast_prov.prov_addr);
				#endif
				if(fast_prov_r_idx == fast_prov_w_idx){
					mesh_fast_prov_sts_set(fast_prov.last_sts);
				}
				else{
					mesh_fast_prov_sts_set(FAST_PROV_SET_ADDR);// just refresh start_tick
				}
			}
			break;
		case FAST_PROV_CONFIRM:
			if(mesh_tx_reliable.mat.op == VD_MESH_PROV_CONFIRM){
				mesh_fast_prov_sts_set(FAST_PROV_CONFIRM_OK);
			}
			break;
		default:
			break;
	}
#endif
}

void mesh_fast_prov_rsp_handle(mesh_rc_rsp_t *rsp)
{
#if FAST_PROVISION_ENABLE
	u16 op = rf_link_get_op_by_ac(rsp->data);
	switch(fast_prov.cur_sts){
		case FAST_PROV_GET_ADDR:
		case FAST_PROV_GET_ADDR_RETRY:
			if(op == VD_MESH_ADDR_GET_STS){
				mesh_fast_prov_add_mac_to_buf(rsp);
			}
			break;
		case FAST_PROV_CONFIRM:
			if(op == VD_MESH_PROV_CONFIRM_STS){
				mesh_fast_prov_sts_set(FAST_PROV_NET_INFO);
			}
			break;
		default:
			break;
	}
#endif
}

#if (FAST_PROVISION_ENABLE)
void mesh_fast_prov_mac_buf_init()
{
	fast_prov_w_idx = 0;
	fast_prov_r_idx = 0;
	memset(fast_prov_mac_buf, 0x00, sizeof(fast_prov_mac_buf));
}


int mesh_fast_prov_add_mac_to_buf(mesh_rc_rsp_t *rsp)
{
	u16 op = rf_link_get_op_by_ac(rsp->data);
	u32 size_op = SIZE_OF_OP(op);
	foreach(i, fast_prov_w_idx){
		if(fast_prov_mac_buf[i].default_addr == rsp->src){
			fast_prov_mac_st *p = &fast_prov_mac_buf[fast_prov_w_idx % CACHE_MAC_MAX_NUM];
			memcpy(p, rsp->data+size_op, OFFSETOF(fast_prov_mac_st, default_addr));
			return 0;
		}
	}

	if(fast_prov_w_idx < CACHE_MAC_MAX_NUM){
		fast_prov_mac_st *p = &fast_prov_mac_buf[fast_prov_w_idx % CACHE_MAC_MAX_NUM];
		memcpy(p, rsp->data+size_op, OFFSETOF(fast_prov_mac_st, default_addr));
		p->default_addr = rsp->src;
		fast_prov_w_idx++;
	}
		
	return 0;	
}

u8 *mesh_fast_prov_get_mac_from_buf()
{
	u8 *p = 0;

	if(fast_prov_r_idx < fast_prov_w_idx)
	{
	    p =(u8 *) &fast_prov_mac_buf[(fast_prov_r_idx % CACHE_MAC_MAX_NUM)];
        fast_prov_r_idx++;          
	}
	return p;
}
#endif
#endif





#if FAST_PROVISION_ENABLE
void mesh_device_key_set_default(){
	memset(mesh_key.dev_key, 0x00, sizeof(mesh_key.dev_key));
	memcpy(mesh_key.dev_key, tbl_mac, sizeof(tbl_mac));
}

void mesh_fast_prov_val_init()
{
	memset(&fast_prov, 0x00, sizeof(fast_prov));
	if(is_provision_success()){
		fast_prov.not_need_prov = 1;
	}
	else{
		mesh_device_key_set_default();
		fast_prov.get_mac_en = 1;
	}
}


int mesh_fast_prov_sts_set(u8 sts_set)
{
	if(FAST_PROV_IDLE == sts_set){
		fast_prov.start_tick = 0;
	}
	else{
		fast_prov.start_tick = clock_time()|1;
	}

	if(sts_set != fast_prov.cur_sts){
		fast_prov.last_sts = fast_prov.cur_sts;
		fast_prov.cur_sts = sts_set;
	}
	return 1;
}

int mesh_fast_prov_sts_get()
{
	return fast_prov.cur_sts;
}

int mesh_fast_prov_rcv_op(u16 rcv_op)
{
	fast_prov.rcv_op = rcv_op;

	return 0;
}

void mesh_fast_provision_timeout()
{
	if(fast_prov.start_tick && clock_time_exceed(fast_prov.start_tick,FAST_PROVISION_TIMEOUT)){
		LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0,"FAST_PROV_TIME_OUT",0);
		mesh_fast_prov_sts_set(FAST_PROV_TIME_OUT);
		LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"time out",0);
		fast_prov.start_tick = 0;
	}
}

void mesh_fast_prov_proc()
{
	if(is_busy_segment_flow()){
		return ;
	}

	mesh_fast_provision_timeout();
	if((mesh_fast_prov_sts_get() == FAST_PROV_RESET_NETWORK) && clock_time_exceed(fast_prov.start_tick, fast_prov.delay*1000)){
		fast_prov.delay = 0;
		mesh_reset_network(0);
		LOG_MSG_INFO(TL_LOG_COMMON, 0, 0,"FAST_PROV_RESET_NETWORK",0);
		#if(__PROJECT_MESH_PRO__)
		mesh_fast_prov_sts_set(FAST_PROV_GET_ADDR);
		#else
		mesh_fast_prov_sts_set(FAST_PROV_CONFIRM);
		#endif
		fast_prov.pending = 0;
	}

	if(((mesh_fast_prov_sts_get() == FAST_PROV_COMPLETE) && clock_time_exceed(fast_prov.start_tick, fast_prov.delay*1000)) ||
		(mesh_fast_prov_sts_get() == FAST_PROV_TIME_OUT)){
		fast_prov.delay = 0;
		mesh_adv_txrx_to_self_en(0);
		mesh_revert_network();	
		LOG_MSG_INFO(TL_LOG_COMMON, 0, 0,"FAST_PROV_REVERT_NETWORK",0);
		mesh_fast_prov_sts_set(FAST_PROV_IDLE);
		fast_prov.pending = 0;
	}
	
	#if(__PROJECT_MESH_PRO__)

	if(fast_prov.pending) return;

	switch(fast_prov.cur_sts){
		case FAST_PROV_IDLE:
			break;
		case FAST_PROV_START:{
			mesh_adv_txrx_to_self_en(1);
			u16 delay_ms = 1000;
			SendOpParaDebug_vendor(ADR_ALL_NODES, 0, VD_MESH_RESET_NETWORK, (u8 *)&delay_ms, 2, 0, 0);
			fast_prov.delay = delay_ms+500;
			mesh_fast_prov_sts_set(FAST_PROV_RESET_NETWORK);
			fast_prov.pending = 1;
			}
			break;
		case FAST_PROV_GET_ADDR:
			LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0,"FAST_PROV_GET_ADDR",0);
			g_reliable_retry_cnt_def = 0;
			mesh_fast_prov_mac_buf_init();
			SendOpParaDebug_vendor(ADR_ALL_NODES, CACHE_MAC_MAX_NUM, VD_MESH_ADDR_GET, (u8 *)&fast_prov.pid, sizeof(fast_prov.pid), VD_MESH_ADDR_GET_STS, 0);
			break;
		case FAST_PROV_GET_ADDR_RETRY:{
			LOG_MSG_INFO(TL_LOG_NODE_BASIC, 0, 0,"FAST_PROV_GET_ADDR_RETRY",0);
			g_reliable_retry_cnt_def = 0;
			mesh_fast_prov_mac_buf_init();
			mac_addr_get_t mac_get;
			mac_get.pid = fast_prov.pid;
			mac_get.ele_addr = fast_prov.prov_addr;
			SendOpParaDebug_vendor(ADR_ALL_NODES, CACHE_MAC_MAX_NUM, VD_MESH_ADDR_GET, (u8 *)&fast_prov.pid, sizeof(mac_addr_get_t), VD_MESH_ADDR_GET_STS, 0);
			}
			break;
		case FAST_PROV_SET_ADDR:{
			LOG_MSG_INFO(TL_LOG_COMMON,0,0,"FAST_PROV_SET_ADDR", 0);
			g_reliable_retry_cnt_def = RELIABLE_RETRY_CNT_DEF;
			mac_addr_set_t addr_set;
			//memcpy(addr_set.mac, fast_prov.mac_ele_info.mac, sizeof(addr_set.mac));
			u8 *p_mac = mesh_fast_prov_get_mac_from_buf();
			if(p_mac != 0){
				memcpy(addr_set.mac, p_mac, sizeof(addr_set.mac));		
				#if WIN32
				addr_set.ele_addr = fast_prov.prov_addr;
				#endif
				SendOpParaDebug_vendor(fast_prov_mac_buf[fast_prov_r_idx-1].default_addr, 1, VD_MESH_ADDR_SET,(u8 *)&addr_set, sizeof(mac_addr_set_t), VD_MESH_ADDR_SET_STS, 0);
			}
			}
			break;
		case FAST_PROV_NET_INFO:
			LOG_MSG_INFO(TL_LOG_COMMON,0,0,"FAST_PROV_NET_INFO", 0);
			g_reliable_retry_cnt_def = RELIABLE_RETRY_CNT_DEF;
			SendOpParaDebug_vendor(ADR_ALL_NODES, 0, VD_MESH_PROV_DATA_SET,(u8 *)&fast_prov.net_info, sizeof(fast_prov.net_info), 0, 0);
			mesh_fast_prov_sts_set(FAST_PROV_CONFIRM);
			break;
		case FAST_PROV_CONFIRM:
			LOG_MSG_INFO(TL_LOG_COMMON,0,0,"FAST_PROV_CONFIRM", 0);
			SendOpParaDebug_vendor(ADR_ALL_NODES, 1, VD_MESH_PROV_CONFIRM, 0, 0, VD_MESH_PROV_CONFIRM_STS, 0);
			break;
		case FAST_PROV_CONFIRM_OK:{
			u16 delay_ms = 1000;
			SendOpParaDebug_vendor(ADR_ALL_NODES, 0, VD_MESH_PROV_COMPLETE, (u8 *)&delay_ms, 2, 0, 0);
			mesh_fast_prov_sts_set(FAST_PROV_COMPLETE);
			LOG_MSG_INFO(TL_LOG_COMMON,0,0,"FAST_PROV_COMPLETE", 0);
			fast_prov.delay = delay_ms;
			fast_prov.pending = 1;
			}
			break;
		default:
			break;
	}
	
	#else

	switch(fast_prov.rcv_op){
		case VD_MESH_RESET_NETWORK:
			if(fast_prov.cur_sts == FAST_PROV_IDLE){
				mesh_adv_txrx_to_self_en(1);
				mesh_fast_prov_sts_set(FAST_PROV_RESET_NETWORK);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_RESET_NETWORK",0);
			}
			break;
		case VD_MESH_ADDR_GET:
			if(fast_prov.cur_sts == FAST_PROV_IDLE){
				mesh_adv_txrx_to_self_en(1);
				mesh_fast_prov_sts_set(FAST_PROV_GET_ADDR);
				mesh_gatt_adv_beacon_enable(0);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_ADDR_GET",0);
			}
			break;
		case VD_MESH_ADDR_SET:
			if(fast_prov.cur_sts == FAST_PROV_GET_ADDR){
				cache_init(ADR_ALL_NODES);
				mesh_fast_prov_sts_set(FAST_PROV_SET_ADDR);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_ADDR_SET",0);
			}
			break;
		case VD_MESH_PROV_DATA_SET:
			if(fast_prov.cur_sts == FAST_PROV_SET_ADDR){				
				mesh_fast_prov_sts_set(FAST_PROV_NET_INFO);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_PROV_DATA_SET",0);
			}
			break;
		case VD_MESH_PROV_CONFIRM:
			if(fast_prov.cur_sts == FAST_PROV_NET_INFO){
				mesh_fast_prov_sts_set(FAST_PROV_CONFIRM);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_PROV_CONFIRM",0);
			}
			break;
		case VD_MESH_PROV_COMPLETE:
			if(fast_prov.cur_sts == FAST_PROV_CONFIRM){
				mesh_fast_prov_sts_set(FAST_PROV_COMPLETE);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"VD_MESH_PROV_COMPLETE, delay:%x",fast_prov.delay);
			}
			break;
		default:
			break;
	}
	#endif
}

int cb_vd_mesh_reset_network(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	if(!fast_prov.not_need_prov){
		return -1;
	}
	mesh_fast_prov_rcv_op(cb_par->op);
	fast_prov.delay = par[0] + (par[1]<<8);

	return 0;
}

int mesh_tx_cmd_get_mac_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp)
{
    fast_prov_mac_st rsp;
   	memcpy(rsp.mac, tbl_mac, sizeof(rsp.mac));
	rsp.pid= MESH_PID_SEL;
    return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, OFFSETOF(fast_prov_mac_st, default_addr), ele_adr, dst_adr, uuid, pub_md);
}

int cb_vd_mesh_get_addr_st_rsp(mesh_cb_fun_par_t *cb_par)
{
    model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
    return mesh_tx_cmd_get_mac_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0, cb_par->op_rsp);
}

int cb_vd_mesh_get_addr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
	if(fast_prov.not_need_prov || (!fast_prov.get_mac_en)){
		return err;
	}

	mac_addr_get_t mac_get;
	memcpy(&mac_get, par, par_len);
	if((mac_get.pid == MESH_PID_SEL) || (mac_get.pid == 0xffff)){
		static u8 default_addr_random = 0;
		if((par_len >= sizeof(mac_addr_get_t)) && (!default_addr_random)){//for default ele_adr_primary conflict
			default_addr_random = 1;
			u16 tmp_ele_addr = mac_get.ele_addr+256+(u16)clock_time()%(0x8000-256-mac_get.ele_addr);
			tmp_ele_addr &= 0x7fff;
			mesh_set_ele_adr_ll(tmp_ele_addr, 0);
		}
		err = cb_vd_mesh_get_addr_st_rsp(cb_par);
	}
	mesh_fast_prov_rcv_op(cb_par->op);
	
	return err;
	
}

int cb_vd_mesh_set_addr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
	if(fast_prov.not_need_prov){
		return err;
	}
	
	if(!memcmp(par, tbl_mac, 6)){
		err = cb_vd_mesh_get_addr_st_rsp(cb_par);
		u16 addr = par[6] +(par[7]<<8);
		if(addr&&is_unicast_adr(addr)){
			mesh_fast_prov_rcv_op(cb_par->op);
			mesh_set_ele_adr_ll(addr, 0);
		}
		fast_prov.get_mac_en = 0;		
	}

	return err;
}

int cb_vd_mesh_set_provision_data(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	if(fast_prov.not_need_prov){
		return -1;
	}
	mesh_fast_prov_rcv_op(cb_par->op);
//par: provision data + app_key add
	memset(&fast_prov.net_info.pro_data, 0x00, sizeof(provison_net_info_str));
	memcpy(&fast_prov.net_info.pro_data, par, sizeof(provison_net_info_str));
	fast_prov.net_info.pro_data.unicast_address = ele_adr_primary;//must, provision data's dst addrs is broadcast.
	memcpy(&fast_prov.net_info.appkey_set,par+sizeof(provison_net_info_str),sizeof(mesh_appkey_set_t));

	return 0;
}

int cb_vd_mesh_provision_confirm(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = -1;
	if(fast_prov.not_need_prov){
		return err;
	}
	mesh_fast_prov_rcv_op(cb_par->op);
	if(fast_prov.cur_sts == FAST_PROV_SET_ADDR){
		err = mesh_tx_cmd_rsp(cb_par->op_rsp, 0, 0, ele_adr_primary, cb_par->adr_src, 0, 0);
	}
	
	return err;
}

int cb_vd_mesh_provision_complete(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	mesh_fast_prov_rcv_op(cb_par->op);
	fast_prov.delay = par[0] + (par[1]<<8);

	return 0;
}

//client
int cb_vd_mesh_addr_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int cb_vd_mesh_primary_addr_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int cb_vd_mesh_provison_data_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}

int cb_vd_mesh_provision_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        //model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
    }
    return err;
}
#endif


