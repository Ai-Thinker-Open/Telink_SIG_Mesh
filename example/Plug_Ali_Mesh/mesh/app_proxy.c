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
#include "mesh/app_provison.h"
#include "app_beacon.h"
#include "app_proxy.h"
#include "proj_lib/sig_mesh/app_mesh.h"

proxy_config_mag_str proxy_mag;
mesh_proxy_protocol_sar_t  proxy_sar;
u8 proxy_filter_initial_mode = FILTER_WHITE_LIST;

void proxy_cfg_list_init()
{
	memset((u8 *)&proxy_mag,0,sizeof(proxy_config_mag_str));
	return ;
}

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
	if(proxy_filter_initial_mode == FILTER_WHITE_LIST){
		proxy_mag.filter_type = FILTER_WHITE_LIST;
		memset((u8 *)&proxy_mag.white_list,0,sizeof(list_mag_str));	
	}else{
		proxy_mag.filter_type = FILTER_BLACK_LIST;
		memset((u8 *)&proxy_mag.black_list,0,sizeof(list_mag_str));	
	}
	return ;
}

u8 find_data_in_list(u16 dst, list_mag_str *p_list_dst)
{
	u8 i=0;
	for(i=0;i<MAX_LIST_LEN;i++){
		if(dst == p_list_dst->list_data[i]){
			return 1;
		}
	}
	return 0;
}

// use this function to filter the data 
u8 find_data_in_mag_list(u16 src,proxy_config_mag_str *p_mag_list)
{
	list_mag_str *p_list= NULL ;
	if(p_mag_list->filter_type == FILTER_WHITE_LIST){
		p_list = &(p_mag_list->white_list);
	}else if(p_mag_list->filter_type == FILTER_BLACK_LIST){
		p_list = &(p_mag_list->black_list);
	}else{}
	return find_data_in_list(src,p_list);
}

u8 filter_data_in_mag_list(u16 src,proxy_config_mag_str *p_mag_list)
{
	list_mag_str *p_list= NULL ;
	if(p_mag_list->filter_type == FILTER_WHITE_LIST){
		p_list = &(p_mag_list->white_list);
		if(find_data_in_list(src,p_list)){
			return 1;
		}else{
			return 0;
		}
	}else if(p_mag_list->filter_type == FILTER_BLACK_LIST){
		p_list = &(p_mag_list->black_list);
		if(find_data_in_list(src,p_list)){
			return 0;
		}else{
			return 1;
		}
	}
	return 0;
}

int is_valid_adv_with_proxy_filter(u16 src)
{
	int valid = 1;
	if(src == PROXY_CONFIG_FILTER_DST_ADR){
	}else{
		valid = filter_data_in_mag_list(src,&proxy_mag);
		if(!valid){
			static u16 filter_error_cnt;	filter_error_cnt++;
		}
	}

	return valid;
}

u8 get_list_cnt(list_mag_str *p_list_dst)
{
	u8 i;
	u8 cnt=0;
	u8 *p_list_mask = p_list_dst->list_idx;
	for(i=0;i<MAX_LIST_LEN;i++){
		if(BIT(i%8)&(p_list_mask[i/8])){
			cnt++;
		}
	}
	return cnt;
}

u8 add_data_to_list(u16 src ,list_mag_str *p_list_dst)
{
    u8 find_result = find_data_in_list(src,p_list_dst);
    u8 idx;
    if(find_result){
        return 2;
    }
	
	if(get_list_cnt(p_list_dst) >= MAX_LIST_LEN){
		return 0;
	}

	for(idx =0;idx<MAX_LIST_LEN;idx++){
		if(p_list_dst->list_data[idx] == 0){
			break;
		}
	}
	if(idx != MAX_LIST_LEN){
		p_list_dst->list_data[idx] = src;
		p_list_dst->list_idx[idx/8]|= BIT(idx%8);
		return 1;
	}
	return 0;
}

u8 delete_data_from_list(u16 src ,list_mag_str *p_list_dst)
{
	u8 idx;
	u8 find_result = find_data_in_list(src,p_list_dst);
	if(!find_result){
        return 2;
	}
	if(get_list_cnt(p_list_dst)<=0){
		return 0;
	}

	for(idx =0;idx<MAX_LIST_LEN;idx++){
		if(p_list_dst->list_data[idx] == src){
			break;
		}
	}
	if(idx != MAX_LIST_LEN){
		p_list_dst->list_data[idx] = 0 ;
		p_list_dst->list_idx[idx/8] &= ~(BIT(idx%8));
		return 1;
	}
	return 0;
}

u8 cpy_list2buf(u8 *p_buf,list_mag_str *p_list_dst)
{
	u8 idx ;
	u8 cnt=0;
	u8 len ;
	for(idx=0;idx<MAX_LIST_LEN;idx++){
		if(BIT(idx%8)& p_list_dst->list_idx[idx/8] ){
			memcpy(p_buf+cnt*2,p_list_dst->list_data+idx,2);
			cnt++;
		}
	}
	len = cnt*2;
	return len;
}

int send_filter_sts(list_mag_str *p_list_dst,mesh_cmd_nw_t *p_nw)
{
	mesh_filter_sts_str mesh_filter_sts;
	memset(&mesh_filter_sts,0,sizeof(mesh_filter_sts_str));
	u8 filter_sts = PROXY_FILTER_STATUS; 
	mesh_filter_sts.fil_type = proxy_mag.filter_type;
	mesh_filter_sts.list_size = get_list_cnt(p_list_dst);
	// swap the list size part 
	endianness_swap_u16((u8 *)(&mesh_filter_sts.list_size));
#if 1
	mesh_tx_cmd_layer_cfg_primary(filter_sts,(u8 *)(&mesh_filter_sts),sizeof(mesh_filter_sts),PROXY_CONFIG_FILTER_DST_ADR);
#else
// add the filter data part for debug 
	u8 tmp_dat[3+MAX_LIST_LEN*2];
	u8 tmp_dat_len =0;
	memcpy(tmp_dat,(u8 *)&mesh_filter_sts,sizeof(mesh_filter_sts));
	tmp_dat_len = cpy_list2buf(tmp_dat+3,p_list_dst)+sizeof(mesh_filter_sts);
	mesh_tx_cmd_layer_cfg_primary(PROXY_FILTER_STATUS,tmp_dat,tmp_dat_len,p_nw->dst);
#endif 
	return 0;
}

list_mag_str * get_filter_pointer(u8 filter_type)
{
	list_mag_str * p_list;
	if(filter_type == FILTER_WHITE_LIST){
		p_list = &(proxy_mag.white_list);
	}else {
		p_list = &(proxy_mag.black_list);
	}
	return p_list;
}

#ifdef WIN32
void set_pair_login_ok(u8 val)
{
	pair_login_ok = val;
}
#endif

u8 proxy_proc_filter_mesh_cmd(u16 src)
{
    list_mag_str *p_list_dst = get_filter_pointer(proxy_mag.filter_type);
    if(proxy_mag.filter_type == FILTER_WHITE_LIST){
        return add_data_to_list(src ,p_list_dst);
    }else if (proxy_mag.filter_type == FILTER_BLACK_LIST){
        return delete_data_from_list(src,p_list_dst);
    }else{

    }
    return 0;
}

u8 proxy_config_dispatch(u8 *p,u8 len )
{
	mesh_cmd_nw_t *p_nw = (mesh_cmd_nw_t *)(p);
	proxy_config_pdu_sr *p_str = (proxy_config_pdu_sr *)(p_nw->data);
	list_mag_str *p_list_dst;
	u8 *p_addr = p_str->para;
	u8 para_len;
	u8 proxy_sts =0;
	u16 proxy_unicast=0;
	u8 i=0;
	// if not set ,use the white list 
	p_list_dst = &(proxy_mag.white_list);
	p_list_dst = get_filter_pointer(proxy_mag.filter_type);
	SET_TC_FIFO(TSCRIPT_PROXY_SERVICE, (u8 *)p_str, len-17);
	switch(p_str->opcode & 0x3f){
		case PROXY_FILTER_SET_TYPE:
			// switch the list part ,and if switch ,it should clear the certain list 
			LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"set filter type %d ",p_str->para[0]);
			proxy_mag.filter_type = p_str->para[0];
			p_list_dst = get_filter_pointer(proxy_mag.filter_type);
			memset(p_list_dst,0,sizeof(list_mag_str));
			proxy_sts =1;
			pair_login_ok = 1;
			break;
		case PROXY_FILTER_ADDR_ADR:
			// we suppose the num is 2
			// 18 means nid(1)ttl(1) sno(3) src(2) dst(2) opcode(1) encpryt(8)
			para_len = len-18;
			LOG_MSG_LIB(TL_LOG_NODE_SDK,p_addr,para_len,"add filter adr part ",0);
			for(i=0;i<para_len/2;i++){
				// swap the endiness part 
				endianness_swap_u16(p_addr+2*i);
				// suppose the data is little endiness 
				proxy_unicast = p_addr[2*i]+(p_addr[2*i+1]<<8);
				add_data_to_list(proxy_unicast ,p_list_dst);
			}
			proxy_sts =1;
			pair_login_ok = 1;
			break;
		case PROXY_FILTER_RM_ADR:
			//we suppose the num is 2
			para_len =len-18;
			for(i=0;i<para_len/2;i++){
				endianness_swap_u16(p_addr+2*i);
				proxy_unicast = p_addr[2*i]+(p_addr[2*i+1]<<8);
				delete_data_from_list(proxy_unicast ,p_list_dst);
			}
			proxy_sts =1;
			break;
		default:
			break;
	}
	if(proxy_sts){
		send_filter_sts(p_list_dst,p_nw);
	}
	return 1;

}

int cal_proxy_adv_with_node_identity(u8 random[8],u8 node_key[16],u16 ele_adr,u8 hash[8])
{
	extern u8 aes_ecb_encryption(u8 *key, u8 mStrLen, u8 *mStr, u8 *result);
	u8 adv_hash[16];
	u8 adv_para[16];
	memset(adv_hash,0,sizeof(adv_hash));
	memset(adv_para,0,sizeof(adv_para));
	endianness_swap_u16((u8 *)&ele_adr);
	memset(adv_para,0,6);
	memcpy(adv_para+6,random,8);
	memcpy(adv_para+14,(u8 *)&ele_adr,2);
	#if WIN32
	#else
	aes_ecb_encryption(node_key,sizeof(adv_para),adv_para,adv_hash);
	#endif 
	memcpy(hash,adv_hash+8,8);
	return 1;
}

void caculate_proxy_adv_hash()
{
	foreach(i,NET_KEY_MAX){
		mesh_net_key_t *p_netkey = &mesh_key.net_key[i][0];
		u8 *p_node_identity_random = &mesh_key.node_identity_hash[i][0];
		if(p_netkey->valid){
			cal_proxy_adv_with_node_identity(prov_para.random,p_netkey->idk,ele_adr_primary,p_node_identity_random);
		}
	}
}

void caculate_proxy_adv_hash_task()
{
	static u32 tick_proxy_hash =0;
	if(clock_time_exceed(tick_proxy_hash,500*1000)){
		tick_proxy_hash = clock_time();
		caculate_proxy_adv_hash();
	}
}



u8 set_proxy_adv_pkt(u8 *p ,u8 flags,u8 *pHash,u8 *pRandom,mesh_net_key_t *p_netkey)
{
	u8 node_identity_flag = (p_netkey->node_identity == NODE_IDENTITY_SUBNET_SUPPORT_ENABLE);
	proxy_adv_node_identity * p_proxy ;
	u8 temp_uuid[2] = SIG_MESH_PROXY_SERVICE;
	p_proxy = (proxy_adv_node_identity *)p;
	p_proxy->flag_len = 0x02;
	p_proxy->flag_type = 0x01;
	p_proxy->flag_data = flags;
	p_proxy->uuid_len = 0x03;
	p_proxy->uuid_type = 0x03;
	p_proxy->uuid_data[0]= temp_uuid[0];
	p_proxy->uuid_data[1]= temp_uuid[1];
	p_proxy->serv_len = 0x14;
	p_proxy->serv_type = 0x16;
	p_proxy->serv_uuid[0]= temp_uuid[0];
	p_proxy->serv_uuid[1]= temp_uuid[1];
	p_proxy->identify_type= node_identity_flag;

	if(!node_identity_flag){
		proxy_adv_net_id *p_net_id;
		p_net_id = (proxy_adv_net_id *)p;
		p_net_id->serv_len = 0x0c;
		memcpy(p_net_id->net_id,p_netkey->nw_id,8);
	}else{
		// calculate the demo part of the proxy adv 'hash 
		u8 hash_tmp[8];
		u8 key_idx =0;
		foreach(i,NET_KEY_MAX){
			mesh_net_key_t *p_netkey_com = &mesh_key.net_key[i][0];
			if(p_netkey_com == p_netkey){
				key_idx = i;
				break;
			}
		}
		memcpy(p_proxy->random,pRandom,8);
		// calculate the hash part 
		//cal_proxy_adv_with_node_identity(pRandom,p_netkey->idk,ele_adr_primary,hash_tmp);
		memcpy(hash_tmp,mesh_key.node_identity_hash[key_idx],8);
		memcpy(p_proxy->hash,hash_tmp,8);
		// set the hash for the provision part 
		memcpy(pHash,hash_tmp,8);
	}
	return p_proxy->serv_len + 8;
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
