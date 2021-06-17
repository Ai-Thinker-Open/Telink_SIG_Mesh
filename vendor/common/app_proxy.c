/********************************************************************************************************
 * @file     app_proxy.c 
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
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/app_provison.h"
#include "app_beacon.h"
#include "app_proxy.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "app_privacy_beacon.h"
#include "directed_forwarding.h"
#include "subnet_bridge.h"
proxy_config_mag_str proxy_mag;
mesh_proxy_protocol_sar_t  proxy_sar;
u8 proxy_filter_initial_mode = FILTER_WHITE_LIST;

//------------------------------att cb-----------------------------
#if !ATT_REPLACE_PROXY_SERVICE_EN
u8 proxy_Out_ccc[2]={0x00,0x00};
u8 proxy_In_ccc[2]=	{0x01,0x00};
#endif

u8  provision_In_ccc[2]={0x01,0x00};// set it can work enable 
u8  provision_Out_ccc[2]={0x00,0x00}; 

void reset_all_ccc()
{
	// wait for the whole dispatch 	
	memset(provision_Out_ccc,0,sizeof(provision_Out_ccc));
	#ifndef WIN32 
	memset(proxy_Out_ccc,0,sizeof(proxy_Out_ccc));
	#endif 
	return ;
}

int pb_gatt_provision_out_ccc_cb(void *p)
{
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
	provision_Out_ccc[0] = pw->dat[0];
	provision_Out_ccc[1] = pw->dat[1];
	beacon_send.conn_beacon_flag =1;
	return 1;	
}

int	pb_gatt_Write (void *p)
{
	if(provision_In_ccc[0]==0 && provision_In_ccc[1]==0){
		return 0;
	}
	#if ATT_REPLACE_PROXY_SERVICE_EN
	extern int proxy_gatt_Write(void *p);
	u8 service_uuid[] = SIG_MESH_PROXY_SERVICE;
	if(0 == memcmp(my_pb_gattUUID, service_uuid, sizeof(my_pb_gattUUID) )){
		return proxy_gatt_Write(p);
	}
	#endif
	#if FEATURE_PROV_EN
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
	// package the data 
	if(!pkt_pb_gatt_data(pw,L2CAP_PROVISON_TYPE,proxy_para_buf,&proxy_para_len)){
		return 0;
	}
	dispatch_pb_gatt(proxy_para_buf ,proxy_para_len);
	#endif 
	return 1;
}

int proxy_out_ccc_cb(void *p)
{
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
	proxy_Out_ccc[0] = pw->dat[0];
	proxy_Out_ccc[1] = pw->dat[1];
	beacon_send.conn_beacon_flag =1;
	beacon_send.tick = clock_time();

#if (MD_DF_EN&&MD_SERVER_EN&&!WIN32)
	proxy_mag.proxy_client_type = UNSET_CLIENT;
	for(int i=0; i<NET_KEY_MAX; i++){
		proxy_mag.directed_server[i].use_directed = (DIRECTED_PROXY_USE_DEFAULT_ENABLE == model_sig_g_df_sbr_cfg.df_cfg.directed_forward.subnet_state[i].directed_control.directed_proxy_use_directed_default);
		proxy_mag.directed_server[i].client_addr = ADR_UNASSIGNED;
		proxy_mag.directed_server[i].client_2nd_ele_cnt = 0;			
		mesh_directed_proxy_capa_report(i);
	}
#endif
	return 1;	
}

int proxy_gatt_Write(void *p)
{
	if(proxy_In_ccc[0]==0 && proxy_In_ccc[1]==0){
		return 0;
	}
	#if FEATURE_PROXY_EN
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
	pb_gatt_proxy_str *p_gatt = (pb_gatt_proxy_str *)(pw->dat);
	mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)proxy_para_buf;
	
	if(p_gatt->msgType == MSG_PROXY_CONFIG ){
		if(!pkt_pb_gatt_data(pw,L2CAP_PROXY_TYPE,(u8 *)&p_bear->nw,&proxy_para_len)){
			return 0;
		}
		p_bear->trans_par_val=TRANSMIT_DEF_PAR;
		p_bear->len=proxy_para_len+1;
		p_bear->type=MESH_ADV_TYPE_MESSAGE;
		// different dispatch 
		//send the data by the SIG MESH layer 
		if(0 == mesh_rc_data_cfg_gatt((u8 *)p_bear)){
		    proxy_config_dispatch((u8 *)&p_bear->nw,proxy_para_len);
		}
	}else if (p_gatt->msgType == MSG_MESH_BEACON){
		if(!pkt_pb_gatt_data(pw,L2CAP_BEACON_TYPE,(u8 *)&p_bear->beacon,&proxy_para_len)){
			return 0;
		}
		if(SECURE_BEACON == p_bear->beacon.type){
			p_bear->len =23;
			//mesh_bear_tx2mesh((u8 *)p_bear, TRANSMIT_DEF_PAR_BEACON);
			int err = mesh_rc_data_beacon_sec(&p_bear->len, 0);		
			if(err != 100){
                LOG_MSG_INFO(TL_LOG_IV_UPDATE,&p_bear->len, p_bear->len+1,"RX secure GATT beacon,nk arr idx:%d, new:%d, pkt:",mesh_key.netkey_sel_dec,mesh_key.new_netkey_dec);
			}
		}else if (PRIVACY_BEACON == p_bear->beacon.type){
			// no handle for other beacon now
			#if MD_PRIVACY_BEA
			p_bear->len =28;
			int err = mesh_rc_data_beacon_privacy(&p_bear->len, 0);		
			if(err != 100){
                LOG_MSG_INFO(TL_LOG_IV_UPDATE,&p_bear->len, p_bear->len+1,"RX prrivacy GATT beacon,nk arr idx:%d, new:%d, pkt:",mesh_key.netkey_sel_dec,mesh_key.new_netkey_dec);
			}
			#endif
		}else{
			// no handle for other beacon now
		}
	}else if(p_gatt->msgType == MSG_NETWORK_PDU){
		if(!pkt_pb_gatt_data(pw,L2CAP_NETWORK_TYPE,(u8 *)&p_bear->nw,&proxy_para_len)){
			return 0;
		}
		// and then how to use the data ,make a demo to turn on or turn off the light 
		p_bear->trans_par_val = TRANSMIT_DEF_PAR;
		p_bear->len=proxy_para_len+1;
		p_bear->type=MESH_ADV_TYPE_MESSAGE;
		mesh_nw_pdu_from_gatt_handle((u8 *)p_bear);
		#if DF_TEST_MODE_EN
		extern void cfg_led_event (u32 e);
		cfg_led_event(LED_EVENT_FLASH_2HZ_2S);
		#endif
	}
#if MESH_RX_TEST
	else if((p_gatt->msgType == MSG_RX_TEST_PDU)&&(p_gatt->data[0] == 0xA3) && (p_gatt->data[1] == 0xFF)){
		u8 par[10];
		memset(par,0x00,sizeof(par));
		u16 adr_dst = p_gatt->data[2] + (p_gatt->data[3]<<8);
		u8 rsp_max = p_gatt->data[4];	
		par[0] = p_gatt->data[6]&0x01;//on_off	
		u8 ack = p_gatt->data[5];
		u32 send_tick = clock_time();
		memcpy(par+4, &send_tick, 4);
		par[8] = p_gatt->data[6];// cur count
		u8 pkt_nums_send = p_gatt->data[7];
		par[3] = p_gatt->data[8];// pkt_nums_ack	
		u32 par_len = 9;
		if(p_gatt->data[7] > 1){// unseg:11  seg:8
			par_len = 12*pkt_nums_send-6;
		}
		extern u16 mesh_rsp_rec_addr;
		mesh_rsp_rec_addr = p_gatt->data[9] + (p_gatt->data[10]<<8);
		SendOpParaDebug(adr_dst, rsp_max, ack ? G_ONOFF_SET : G_ONOFF_SET_NOACK, 
						   (u8 *)&par, par_len);
	}
#endif
	else{
	}
#endif
	return 0;
}
//-----------------------------end att cb-----------------------------

void set_proxy_initial_mode(u8 special_mode)
{
	if(special_mode){
		proxy_filter_initial_mode =FILTER_BLACK_LIST;
	}else{
		proxy_filter_initial_mode =FILTER_WHITE_LIST;
	}
	mesh_proxy_sar_para_init();
	return;
}
	

void proxy_cfg_list_init_upon_connection()
{
	memset(&proxy_mag, 0x00, sizeof(proxy_mag));
	proxy_mag.filter_type = proxy_filter_initial_mode;

	return ;
}

int find_data_in_list(u16 dst)
{
	int idx = -1;
	foreach(i,MAX_LIST_LEN){
		if(dst == proxy_mag.addr_list[i]){
			idx = i;
			break;
		}
	}
	return idx;
}

int is_addr_in_proxy_list(u16 dst)
{
	return (-1 != find_data_in_list(dst));
}

int is_valid_adv_with_proxy_filter(u16 src)
{
	int valid = 1;
	if(src == PROXY_CONFIG_FILTER_DST_ADR){
	}else{
		valid = (FILTER_WHITE_LIST == proxy_mag.filter_type) ? is_addr_in_proxy_list(src):!is_addr_in_proxy_list(src);
		if(!valid){
			static u16 filter_error_cnt;	filter_error_cnt++;
		}
	}

	return valid;
}

int get_list_cnt()
{
	int cnt=0;
	foreach(i,MAX_LIST_LEN){
		if(0 == proxy_mag.addr_list[i]){
			break;
		}
		cnt++;
	}
	return cnt;
}

int add_data_to_list(u16 src)
{
    if(is_addr_in_proxy_list(src)){
        return 0;
    }

	u8 idx;
	for(idx =0;idx<MAX_LIST_LEN;idx++){
		if(0 == proxy_mag.addr_list[idx]){
			proxy_mag.addr_list[idx] = src;
			return 0;
		}
	}
	
	return -1;
}

int delete_data_from_list(u16 src)
{
	int idx = find_data_in_list(src);
	if((-1 == idx) || (get_list_cnt()>=MAX_LIST_LEN)){
        return -1;
	}

	proxy_mag.addr_list[idx] = 0;
	memcpy(&proxy_mag.addr_list[idx], &proxy_mag.addr_list[idx+1], 2*(MAX_LIST_LEN-(idx+1)));
	return 0;
}

int send_filter_sts(mesh_cmd_nw_t *p_nw)
{
	int err = -1;
	mesh_filter_sts_str mesh_filter_sts;
	memset(&mesh_filter_sts,0,sizeof(mesh_filter_sts_str));
	u8 filter_sts = PROXY_FILTER_STATUS; 
	mesh_filter_sts.fil_type = proxy_mag.filter_type;
	mesh_filter_sts.list_size = get_list_cnt();
	// swap the list size part 
	endianness_swap_u16((u8 *)(&mesh_filter_sts.list_size));
#if 1
	err = mesh_tx_cmd_layer_cfg_primary(filter_sts,(u8 *)(&mesh_filter_sts),sizeof(mesh_filter_sts),PROXY_CONFIG_FILTER_DST_ADR);
#else
#endif 
	return err;
}

#ifdef WIN32
void set_pair_login_ok(u8 val)
{
	pair_login_ok = val;
}
#endif

u8 proxy_proc_filter_mesh_cmd(u16 src)
{
    if(proxy_mag.filter_type == FILTER_WHITE_LIST){
        return add_data_to_list(src);
    }else if (proxy_mag.filter_type == FILTER_BLACK_LIST){
        return delete_data_from_list(src);
    }else{

    }
    return 0;
}

u8 proxy_config_dispatch(u8 *p,u8 len )
{
	mesh_cmd_nw_t *p_nw = (mesh_cmd_nw_t *)(p);
	proxy_config_pdu_sr *p_str = (proxy_config_pdu_sr *)(p_nw->data);
	u8 *p_addr = p_str->para;
	u8 para_len;
	u16 proxy_unicast=0;
	u8 i=0;
	// if not set ,use the white list 
	SET_TC_FIFO(TSCRIPT_PROXY_SERVICE, (u8 *)p_str, len-17);
	switch(p_str->opcode & 0x3f){
		case PROXY_FILTER_SET_TYPE:
			// switch the list part ,and if switch ,it should clear the certain list 
			LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"set filter type %d ",p_str->para[0]);
			proxy_mag.filter_type = p_str->para[0];
			memset(proxy_mag.addr_list, 0, sizeof(proxy_mag.addr_list));
			send_filter_sts(p_nw);
			pair_login_ok = 1;
			#if MD_DF_EN
			if(FILTER_BLACK_LIST ==  proxy_mag.filter_type){
				proxy_mag.proxy_client_type = BLACK_LIST_CLIENT;
				for(int i=0; i<NET_KEY_MAX; i++){
					proxy_mag.directed_server[i].use_directed = 0;
				}
			}
			#endif
			break;
		case PROXY_FILTER_ADD_ADR:
			// we suppose the num is 2
			// 18 means nid(1)ttl(1) sno(3) src(2) dst(2) opcode(1) encpryt(8)
			para_len = len-18;
			LOG_MSG_LIB(TL_LOG_NODE_SDK,p_addr,para_len,"add filter adr part ",0);
			for(i=0;i<para_len/2;i++){
				// swap the endiness part 
				endianness_swap_u16(p_addr+2*i);
				// suppose the data is little endiness 
				proxy_unicast = p_addr[2*i]+(p_addr[2*i+1]<<8);
				add_data_to_list(proxy_unicast);
			}
			send_filter_sts(p_nw);
			pair_login_ok = 1;
			break;
		case PROXY_FILTER_RM_ADR:
			//we suppose the num is 2
			para_len =len-18;
			LOG_MSG_LIB(TL_LOG_NODE_SDK,p_addr,para_len,"remove filter adr part ",0);
			for(i=0;i<para_len/2;i++){
				endianness_swap_u16(p_addr+2*i);
				proxy_unicast = p_addr[2*i]+(p_addr[2*i+1]<<8);
				delete_data_from_list(proxy_unicast);
			}
			send_filter_sts(p_nw);
			break;
		#if (MD_DF_EN&&!WIN32)
		case DIRECTED_PROXY_CONTROL:{
				directed_proxy_ctl_t *p_directed_ctl = (directed_proxy_ctl_t *)p_str->para;
				swap_addr_range2_little_endian((u8 *)&p_directed_ctl->addr_range);
				LOG_MSG_LIB(TL_LOG_NODE_SDK, (u8 *)p_directed_ctl, sizeof(directed_proxy_ctl_t),"directed proxy control ",0);
				int key_offset = get_mesh_net_key_offset(mesh_key.netkey_sel_dec);
				direct_proxy_server_t *p_direct_proxy = &proxy_mag.directed_server[key_offset];
				if(!((BLACK_LIST_CLIENT == proxy_mag.proxy_client_type) || (PROXY_CLIENT == proxy_mag.proxy_client_type)) && (p_directed_ctl->use_directed < USE_DIRECTED_PROHIBITED)){
					if(is_directed_proxy_en(key_offset)){
						if(UNSET_CLIENT == proxy_mag.proxy_client_type){					
							proxy_mag.proxy_client_type = DIRECTED_PROXY_CLIENT;
							for(int i=0; i<NET_KEY_MAX; i++){
								proxy_mag.directed_server[i].use_directed = 0;
							}						
						}
						
						p_direct_proxy->use_directed = p_directed_ctl->use_directed;
						if(p_directed_ctl->use_directed){
							p_direct_proxy->client_addr = p_directed_ctl->addr_range.range_start;
							if(p_directed_ctl->addr_range.length_present){
								p_direct_proxy->client_2nd_ele_cnt = p_directed_ctl->addr_range.range_length - 1;
							}
						}					
					}
					mesh_directed_proxy_capa_report(key_offset);
				}										
			}
			break;
		#endif
		default:
			break;
	}
	#if MD_DF_EN
	if(UNSET_CLIENT == proxy_mag.proxy_client_type){
		proxy_mag.proxy_client_type = PROXY_CLIENT; 
	}
	#endif
	return 1;

}
extern u8 aes_ecb_encryption(u8 *key, u8 mStrLen, u8 *mStr, u8 *result);
#if WIN32
void aes_win32(char *p, int plen, char *key);
#endif
int proxy_adv_calc_with_node_identity(u8 random[8],u8 node_key[16],u16 ele_adr,u8 hash[8])
{
	u8 adv_para[16];
	memset(adv_para,0,sizeof(adv_para));
	endianness_swap_u16((u8 *)&ele_adr);
	memset(adv_para,0,6);
	memcpy(adv_para+6,random,8);
	memcpy(adv_para+14,(u8 *)&ele_adr,2);
	#if WIN32
	aes_win32((char *)adv_para,sizeof(adv_para),(char *)node_key);
	memcpy(hash,adv_para+8,8);
	#else
	u8 adv_hash[16];
	aes_ecb_encryption(node_key,sizeof(adv_para),adv_para,adv_hash);
	memcpy(hash,adv_hash+8,8);
	#endif 
	return 1;
}

int proxy_adv_calc_with_private_net_id(u8 random[8],u8 net_id[8],u8 idk[16],u8 hash[8])
{
	u8 adv_para[16];
	memcpy(adv_para,net_id,8);
	memcpy(adv_para+8,random,8);
	#if WIN32
	aes_win32((char *)adv_para,sizeof(adv_para),(char *)idk);
	memcpy(hash,adv_para+8,8);
	#else
	u8 adv_hash[16];
	aes_ecb_encryption(idk,sizeof(adv_para),adv_para,adv_hash);
	memcpy(hash,adv_hash+8,8);
	#endif
	return 1;
}

int proxy_adv_calc_with_private_node_identity(u8 random[8],u8 node_key[16],u16 ele_adr,u8 hash[8])
{
	u8 adv_para[16];
	endianness_swap_u16((u8 *)&ele_adr);
	memset(adv_para,0,5);
	adv_para[5] = PRIVATE_NODE_IDENTITY_TYPE;
	memcpy(adv_para+6,random,8);
	memcpy(adv_para+14,(u8 *)&ele_adr,2);
	#if WIN32
	aes_win32((char *)adv_para,sizeof(adv_para),(char *)node_key);
	memcpy(hash,adv_para+8,8);
	#else
	u8 adv_hash[16];
	aes_ecb_encryption(node_key,sizeof(adv_para),adv_para,adv_hash);
	memcpy(hash,adv_hash+8,8);
	#endif
	return 1;

}

void caculate_proxy_adv_hash(mesh_net_key_t *p_netkey )
{
	proxy_adv_calc_with_node_identity(prov_para.random,p_netkey->idk,ele_adr_primary,p_netkey->ident_hash);	
	proxy_adv_calc_with_private_net_id(prov_para.random,p_netkey->nw_id,p_netkey->idk,p_netkey->priv_net_hash);
	proxy_adv_calc_with_private_node_identity(prov_para.random,p_netkey->idk,ele_adr_primary,p_netkey->priv_ident_hash);
	return;
}

void set_proxy_adv_header(proxy_adv_node_identity * p_proxy)
{
	p_proxy->flag_len = 0x02;
	p_proxy->flag_type = 0x01;
	p_proxy->flag_data = 0x06;
	p_proxy->uuid_len = 0x03;
	p_proxy->uuid_type = 0x03;
	p_proxy->uuid_data[0]= SIG_MESH_PROXY_SER_VAL &0xff;
	p_proxy->uuid_data[1]= (SIG_MESH_PROXY_SER_VAL>>8)&0xff;

}
#if MD_PRIVACY_BEA
	#if 0
u8 mesh_enable_private_identity_type(u8 en)
{
	for(int i=0;i<NET_KEY_MAX;i++){
		mesh_net_key_t *p_net = &mesh_key.net_key[i][0];
		if(p_net->valid){
			u8 key_phase = p_net->key_phase;
			if(KEY_REFRESH_PHASE2 == key_phase){
				p_net += 1;		// use new key
			}
			if(en){
				p_net->node_identity = NODE_IDENTITY_SUB_NET_STOP;
				p_net->priv_identity = PRIVATE_NODE_IDENTITY_ENABLE;
			}else{
				p_net->node_identity = NODE_IDENTITY_SUB_NET_STOP;
				p_net->priv_identity = PRIVATE_NODE_IDENTITY_DISABLE;
			}
		}	
	}
}
u32 A_debug_private_sts =0;

void mesh_private_identity_proc()
{
	if(A_debug_private_sts == 1){
		mesh_enable_private_identity_type(1);
		A_debug_private_sts =0;
	}else if (A_debug_private_sts == 2){
		mesh_enable_private_identity_type(0);
		A_debug_private_sts =0;
	}
}
	#endif


u8 mesh_get_identity_type(mesh_net_key_t *p_netkey)
{
#if MD_SERVER_EN
	u8 gatt_proxy_sts = model_sig_cfg_s.gatt_proxy;
	u8 priv_proxy_sts = model_private_beacon.srv[0].proxy_sts;
	u8 node_identi =NODE_IDENTITY_PROHIBIT;
	if(gatt_proxy_sts == GATT_PROXY_SUPPORT_DISABLE && priv_proxy_sts == PRIVATE_PROXY_ENABLE){
		node_identi = PRIVATE_NETWORK_ID_TYPE;
	}else if ( gatt_proxy_sts == GATT_PROXY_SUPPORT_ENABLE &&
			   (priv_proxy_sts == PRIVATE_PROXY_DISABLE ||priv_proxy_sts == PRIVATE_PROXY_NOT_SUPPORT)){
		node_identi = NETWORK_ID_TYPE;
	}else{}

	if(p_netkey->node_identity == NODE_IDENTITY_SUB_NET_STOP && p_netkey->priv_identity == PRIVATE_NODE_IDENTITY_ENABLE){
		node_identi = PRIVATE_NODE_IDENTITY_TYPE;
	}else if ( p_netkey->node_identity == NODE_IDENTITY_SUB_NET_RUN &&
			(p_netkey->priv_identity == PRIVATE_NODE_IDENTITY_NOT_SUPPORT || p_netkey->priv_identity == PRIVATE_NODE_IDENTITY_DISABLE)){
		node_identi = NODE_IDENTITY_TYPE;
	}
	return node_identi;
#else
	return 1;
#endif
}
#endif

u8 set_proxy_adv_pkt(u8 *p ,u8 *pRandom,mesh_net_key_t *p_netkey,u8 *p_len)
{
	// get the key part 
	u8 key_phase = p_netkey->key_phase;
	if(KEY_REFRESH_PHASE2 == key_phase){
		p_netkey += 1;		// use new key
	}
	// get_node_identity type part 
	u8 node_identity_type =0;
	#if MD_PRIVACY_BEA
	node_identity_type = mesh_get_identity_type(p_netkey);
	if(node_identity_type == NODE_IDENTITY_PROHIBIT){// in some condition it will not allow to send proxy adv
		return 0;
	}
	#else
	if(p_netkey->node_identity == NODE_IDENTITY_SUB_NET_RUN){
		node_identity_type = NODE_IDENTITY_TYPE;
	}else{
		node_identity_type = NETWORK_ID_TYPE;
	}
	#endif
	// set the base para for the proxy adv 
	proxy_adv_node_identity * p_proxy = (proxy_adv_node_identity *)p;
	set_proxy_adv_header(p_proxy);
	p_proxy->serv_len = 0x14;
	p_proxy->serv_type = 0x16;
	p_proxy->identify_type= node_identity_type;
	if(node_identity_type == NETWORK_ID_TYPE){
		proxy_adv_net_id *p_net_id = (proxy_adv_net_id *)p;
		p_net_id->serv_len = 0x0c;
		p_net_id->serv_type = 0x16;
		p_proxy->serv_uuid[0]= SIG_MESH_PROXY_SER_VAL &0xff;
		p_proxy->serv_uuid[1]= (SIG_MESH_PROXY_SER_VAL>>8)&0xff;
		memcpy(p_net_id->net_id,p_netkey->nw_id,8);
	}else if(node_identity_type == NODE_IDENTITY_TYPE){
		// calculate the demo part of the proxy adv 'hash
		p_proxy->serv_len = 0x14;
		p_proxy->serv_type = 0x16;
		p_proxy->serv_uuid[0]= SIG_MESH_PROXY_SER_VAL &0xff;
		p_proxy->serv_uuid[1]= (SIG_MESH_PROXY_SER_VAL>>8)&0xff;
		memcpy(p_proxy->random,pRandom,8);
		memcpy(p_proxy->hash,p_netkey->ident_hash,8);
	}else if (node_identity_type == PRIVATE_NETWORK_ID_TYPE){
		// calculate the demo part of the proxy adv 'hash
		p_proxy->serv_len = 0x14;
		p_proxy->serv_type = 0x16;
		p_proxy->serv_uuid[0]= SIG_MESH_PROXY_SER_VAL &0xff;
		p_proxy->serv_uuid[1]= (SIG_MESH_PROXY_SER_VAL>>8)&0xff;
		memcpy(p_proxy->random,pRandom,8);
		memcpy(p_proxy->hash,p_netkey->priv_net_hash,8);
	}else if (node_identity_type == PRIVATE_NODE_IDENTITY_TYPE){
		// calculate the demo part of the proxy adv 'hash
		p_proxy->serv_len = 0x14;
		p_proxy->serv_type = 0x16;
		p_proxy->serv_uuid[0]= SIG_MESH_PROXY_SER_VAL &0xff;
		p_proxy->serv_uuid[1]= (SIG_MESH_PROXY_SER_VAL>>8)&0xff;
		memcpy(p_proxy->random,pRandom,8);
		memcpy(p_proxy->hash,p_netkey->priv_ident_hash,8);
	}
	*p_len = p_proxy->serv_len + 8;
	return 1;
}

void push_proxy_raw_data(u8 *p_unseg){
	mesh_tx_cmd_add_packet(p_unseg);
}

u8 set_proxy_pdu_data(proxy_msg_str *p_proxy,u8 msg_type , u8 *para ,u8 para_len ){
	static u8 para_idx;
	p_proxy->type = msg_type;
	if(para_len<=MAX_PROXY_RSV_LEN){
		p_proxy->sar = COMPLETE_MSG ;
		memcpy(p_proxy->data ,para , para_len);
		push_proxy_raw_data((u8*)(p_proxy));
	}else{
		para_idx =0;
		para_len -= MAX_PROXY_RSV_LEN;
		p_proxy->sar = FIRST_SEG_MSG ;
		memcpy(p_proxy->data ,para , MAX_PROXY_RSV_LEN);
		para_idx += MAX_PROXY_RSV_LEN;
		while(para_len){
			u8 pkt_len;
			pkt_len =(para_len>=MAX_PROXY_RSV_LEN)?MAX_PROXY_RSV_LEN:para_len ;
			para_len -= pkt_len;
			if(pkt_len == MAX_PROXY_RSV_LEN){
				p_proxy->sar = CONTINUS_SEG_MSG;
				memcpy(p_proxy->data , para+para_idx,pkt_len);
				push_proxy_raw_data((u8*)(p_proxy));
			}else{
				// the last packet 
				p_proxy->sar = CONTINUS_SEG_MSG;
				memcpy(p_proxy->data , para+para_idx,pkt_len);
				push_proxy_raw_data((u8*)(p_proxy));
			}
			para_idx += pkt_len;
		}
	}
	return 1;
}
#if 0
void test_fun_private_node_identity()
{
	static u8 A_debug_hash[8];
	u8 random[8] = {0x34,0xae,0x60,0x8f,0xbb,0xc1,0xf2,0xc6};
	u8 identity_key[16] = {	0x84,0x39,0x6c,0x43,0x5a,0xc4,0x85,0x60,
							0xb5,0x96,0x53,0x85,0x25,0x3e,0x21,0x0c};
	u8 net_id[8]= {0x3e,0xca,0xff,0x67,0x2f,0x67,0x33,0x70};
	proxy_adv_calc_with_private_net_id(random,net_id,identity_key,A_debug_hash);
}

void test_fun_private_node_identity1()
{
	static u8 A_debug_hash1[8];
	u8 random[8] = {0x34,0xae,0x60,0x8f,0xbb,0xc1,0xf2,0xc6};
	u8 identity_key[16] = {	0x84,0x39,0x6c,0x43,0x5a,0xc4,0x85,0x60,
							0xb5,0x96,0x53,0x85,0x25,0x3e,0x21,0x0c};
	u16 ele_adr = 0x1201;
	proxy_adv_calc_with_private_node_identity(random,identity_key, ele_adr,A_debug_hash1);
}
void test_fun_private()
{
	test_fun_private_node_identity();
	test_fun_private_node_identity1();
	irq_disable();
	while(1);
}
#endif



