/********************************************************************************************************
 * @file     app_provison.c 
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
#include "app_provison.h"
#include "app_beacon.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"

#if WIN32 
#include "../../../reference/tl_bulk/lib_file/app_config.h"
#include "../../../reference/tl_bulk/lib_file/hw_fun.h"
#include"../../../reference/tl_bulk/lib_file/gatt_provision.h"
#else 
#include "mesh/mi_api/telink_sdk_mible_api.h"
#endif
#include "mesh/remote_prov.h"

#define DEBUG_PROXY_SAR_ENABLE 	0
prov_para_proc_t prov_para;
#if TESTCASE_FLAG_ENABLE
_attribute_no_retention_bss_ mesh_cmd_bear_unseg_t		pro_adv_pkt;
#endif
pro_para_mag  provision_mag;
_attribute_no_retention_bss_ u8 prov_link_cls_code;
_attribute_no_retention_bss_ u8 prov_link_uuid[16];
_attribute_no_retention_bss_ pro_PB_ADV rcv_pb;
_attribute_no_retention_bss_ u8 para_pro[PROVISION_GATT_MAX_LEN]; //it's also used in proxy_gatt_Write(), but network payload is less then 31, because it will be relayed directly.
_attribute_no_retention_bss_ u8 para_len ;


#define OFFSET_START	0x1F
#define OFFSET_CON		0x17
_attribute_no_retention_bss_ mesh_prov_seg_t mesh_prov_seg;

const u8 pub_key_with_oob[64]={
					0xF4,0x65,0xE4,0x3F,0xF2,0x3D,0x3F,0x1B,
					0x9D,0xC7,0xDF,0xC0,0x4D,0xA8,0x75,0x81,
					0x84,0xDB,0xC9,0x66,0x20,0x47,0x96,0xEC,
					0xCF,0x0D,0x6C,0xF5,0xE1,0x65,0x00,0xCC,
					0x02,0x01,0xD0,0x48,0xBC,0xBB,0xD8,0x99,
					0xEE,0xEF,0xC4,0x24,0x16,0x4E,0x33,0xC2,
					0x01,0xC2,0xB0,0x10,0xCA,0x6B,0x4D,0x43,
					0xA8,0xA1,0x55,0xCA,0xD8,0xEC,0xB2,0x79};

// add the provision data part dispatch 

mesh_prov_oob_str prov_oob;

void provision_mag_cfg_s_store()
{
	mesh_common_store(FLASH_ADR_PROVISION_CFG_S);
}

u8 mesh_provision_cmd_process()
{
	// add the test part ,how to send ,and the test_script part		
	unprov_beacon_send(MESH_UNPROVISION_BEACON_WITH_URI,0);
    send_rcv_retry_set(PRO_BEACON,0,0);
	return 1;
}

// add the sar part logical 
void mesh_proxy_sar_para_init()
{
	proxy_sar.sar_start =0;
	proxy_sar.sar_end =1;
	proxy_sar.err_flag =0;
	proxy_sar.sar_tick =0;
}

void mesh_proxy_sar_start()
{	
	if(!proxy_sar.sar_start&&proxy_sar.sar_end){
		proxy_sar.sar_start = 1;
		proxy_sar.sar_end =0;
		proxy_sar.sar_tick = clock_time()|1;
	}else{
		#if DEBUG_PROXY_SAR_ENABLE
		static u32 A_debug_start_err =0;
		A_debug_start_err++;
		LOG_MSG_INFO(TL_LOG_PROXY,0, 0 ,"mesh_proxy_sar_start:debug printf sar start err");
		#endif 
		proxy_sar.err_flag =1;
	}	
}

void mesh_proxy_sar_continue()
{
	if(proxy_sar.sar_start && !proxy_sar.sar_end){
		proxy_sar.sar_tick = clock_time()|1;
	}else{
		#if DEBUG_PROXY_SAR_ENABLE
		static u32 A_debug_start_continu =0;
		A_debug_start_continu++;
		LOG_MSG_INFO(TL_LOG_PROXY,0, 0 ,"mesh_proxy_sar_continue:debug printf sar continue err");
		#endif 
		proxy_sar.err_flag =1;
	}	
}

void mesh_proxy_sar_end()
{
	if(proxy_sar.sar_start && !proxy_sar.sar_end){
		proxy_sar.sar_start =0;
		proxy_sar.sar_end = 1;
		proxy_sar.sar_tick = 0;
	}else{
		#if DEBUG_PROXY_SAR_ENABLE
		static u32 A_debug_start_end =0;
		A_debug_start_end++;
		LOG_MSG_INFO(TL_LOG_PROXY,0, 0 ,"mesh_proxy_sar_end:debug printf sar end err");
		#endif 
		proxy_sar.err_flag =1;
	}	
}

void mesh_proxy_sar_complete()
{
	// reset the status 
	if(!proxy_sar.sar_start&&proxy_sar.sar_end){
		proxy_sar.sar_tick =0;
	}else{
	#if DEBUG_PROXY_SAR_ENABLE
		static u32 A_debug_start_complete =0;
		A_debug_start_complete++;
		LOG_MSG_INFO(TL_LOG_PROXY,0, 0 ,"mesh_proxy_sar_complete:debug printf sar complete err");
	#endif 
		proxy_sar.err_flag =1;
	}
}

void mesh_proxy_sar_err_terminate()
{
	if(proxy_sar.err_flag){
	    LOG_MSG_ERR(TL_LOG_PROXY,0, 0 ,"TL_LOG_PROXY:sar complete err",0);
		//reset para part 
		mesh_proxy_sar_para_init();
		// send terminate ind cmd to the master part 
		#if WIN32 
		// the upper tester should reliaze the function 
		//mesh_proxy_master_terminate_cmd(); 
		#else
		#if DEBUG_PROXY_SAR_ENABLE
		irq_disable();
		while(1);
		#endif 
		//bls_ll_terminateConnection(0x13); 
		#endif 
	}	
}

void mesh_proxy_sar_timeout_terminate()
{
	if(proxy_sar.sar_tick&&clock_time_exceed(proxy_sar.sar_tick,PROXY_PDU_TIMEOUT_TICK)){
		mesh_proxy_sar_para_init();
		LOG_MSG_ERR(TL_LOG_PROXY,0, 0 ,"TL_LOG_PROXY:sar timeout terminate",0);
		#if WIN32 
		mesh_proxy_master_terminate_cmd();
		#else
		bls_ll_terminateConnection(0x13);
		#endif
	}
}

#define GATT_PROVISION_TIMEOUT 	20*1000*1000 // 10S
_attribute_no_retention_bss_ u32 provision_timeout_tick=0;

void set_prov_timeout_tick(u32 tick)
{
    provision_timeout_tick = tick;
}

void provision_timeout_cb()
{
	if(provision_timeout_tick && clock_time_exceed(provision_timeout_tick,GATT_PROVISION_TIMEOUT)){
		provision_timeout_tick =0;
		mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_TIMEOUT);
	}
}



u8 pkt_pb_gatt_data(rf_packet_att_data_t *p, u8 l2cap_type,u8 *p_rcv,u8 *p_rcv_len)
{
	//package the data str 
	static u8 idx_num =0;
	pb_gatt_proxy_str *p_gatt = (pb_gatt_proxy_str *)(p->dat);
	if(l2cap_type == L2CAP_PROVISON_TYPE){
		if(p_gatt->msgType != MSG_PROVISION_PDU){
			return 0;
		}
	}else if (l2cap_type == L2CAP_PROXY_TYPE){
		if(p_gatt->msgType != MSG_PROXY_CONFIG){
			return 0;
		}
	}else if (l2cap_type == L2CAP_BEACON_TYPE){
		if(p_gatt->msgType != MSG_MESH_BEACON){
			return 0;
		}
	}else if (l2cap_type == L2CAP_NETWORK_TYPE){
		if(p_gatt->msgType != MSG_NETWORK_PDU){
			return 0;
		}
	}else{
			return 0;
	}
	#if DEBUG_PROXY_SAR_ENABLE
	static u8 A_proxy_sar_debug[256];
	static u8 proxy_idx =0;
	A_proxy_sar_debug[proxy_idx++]= p_gatt->sar;
	if(proxy_idx ==255){
		memset(A_proxy_sar_debug,0,sizeof(A_proxy_sar_debug));
	}
	#endif 
	#if DEBUG_PROXY_SAR_ENABLE
	/*
	static u8 A_debug_record_last_lost[32][16];
	static u8 A_debug_record_last_idx =0;
	memcpy((u8*)A_debug_record_last_lost+A_debug_record_last_idx*16,p_gatt->data,(p->l2cap-4)>16?16:4);
	A_debug_record_last_idx++;
	if(A_debug_record_last_idx == 16){
		A_debug_record_last_idx = 0;
	}
	*/
	#endif 

    u8 len_payload = p->l2cap-4;
    u8 len_total = ((((p_gatt->sar == SAR_CONTINUS)||(p_gatt->sar == SAR_END))? idx_num : 0) + len_payload);
	if(len_total > sizeof(para_pro)){    // p_rcv point to para_pro, over folw.
	    static u8 para_pro_overflow_cnt;para_pro_overflow_cnt++;
		mesh_proxy_sar_para_init();
	    return 0;
	}
	
	if(p_gatt->sar == SAR_START){
		idx_num =0;
		memcpy(p_rcv,p_gatt->data,len_payload);
		idx_num +=len_payload;
		mesh_proxy_sar_start();
		return 0;
	}else if(p_gatt->sar == SAR_CONTINUS){
		memcpy(p_rcv+idx_num,p_gatt->data,len_payload);
		idx_num +=len_payload;
		mesh_proxy_sar_continue();
		return 0;
	}else if(p_gatt->sar == SAR_END){
		memcpy(p_rcv+idx_num,p_gatt->data,len_payload);
		idx_num +=len_payload;
		mesh_proxy_sar_end();
	}else{
		idx_num =0;
		memcpy(p_rcv+idx_num,p_gatt->data,len_payload);
		idx_num +=len_payload;
		mesh_proxy_sar_complete();
	} 
	#if DEBUG_PROXY_SAR_ENABLE
	if(proxy_sar.err_flag){
		static u8 A_debug_record_last_lost1[32];
		memcpy(A_debug_record_last_lost1,p_gatt->data,len_payload);
		LOG_MSG_ERR(TL_LOG_PROXY,p_gatt->data, len_payload ,"TL_LOG_PROXY:sar complete err");
	}
	#endif 
	mesh_proxy_sar_err_terminate();
	*p_rcv_len = idx_num ;
	return 1;
	// after package the data and the para 
}



void set_pb_gatt_adv(u8 *p,u8 flags)
{
	PB_GATT_ADV_DAT *p_pb_adv;
	u8 temp_uuid[2]=SIG_MESH_PROVISION_SERVICE;
	p_pb_adv = (PB_GATT_ADV_DAT *)(p);
	p_pb_adv->flag_len = 0x02;
	p_pb_adv->flag_type = 0x01;
	p_pb_adv->flag_content = flags;
	p_pb_adv->uuid_len= 0x03;
	p_pb_adv->uuid_type=0x03;
	p_pb_adv->uuid_pb_uuid[0] = temp_uuid[0];
	p_pb_adv->uuid_pb_uuid[1] = temp_uuid[1];
	p_pb_adv->service_len = 0x15;
	p_pb_adv->service_type = 0x16;
	p_pb_adv->service_uuid[0] = temp_uuid[0];
	p_pb_adv->service_uuid[1] = temp_uuid[1];
	memcpy(p_pb_adv->service_data,prov_para.device_uuid,16);
	memcpy(p_pb_adv->oob_info , prov_para.oob_info,2);
	return;
}

void set_adv_provision(rf_packet_adv_t * p) 
{
	set_pb_gatt_adv(p->data,6);
	p->header.type = LL_TYPE_ADV_IND;
	memcpy(p->advA,tbl_mac,6);
	memcpy(p->data, p->data, 29);
	p->rf_len = 6 + 29;
	p->dma_len = p->rf_len + 2;	
	return ;
} 

void set_adv_proxy(rf_packet_adv_t * p)
{
	u8 dat_len =0;
	foreach(i,NET_KEY_MAX){
		static u8 proxy_adv_idx = 0;
		proxy_adv_idx = proxy_adv_idx % NET_KEY_MAX;
		mesh_net_key_t *p_netkey = &mesh_key.net_key[proxy_adv_idx++][0];
		if(p_netkey->valid){
			dat_len = set_proxy_adv_pkt(p->data ,6,prov_para.hash,prov_para.random,p_netkey);
			break;
		}
	}
	p->header.type = LL_TYPE_ADV_IND;
	memcpy(p->advA,tbl_mac,6);
	//memcpy(p->data, p->data, dat_len);
	p->rf_len = 6 + dat_len;
	p->dma_len = p->rf_len + 2;	
}

void mesh_provision_para_reset()
{
	// test para init 
#if FEATURE_PROV_EN
    #if MD_REMOTE_PROV
    if(mesh_pr_sts_work_or_not()&&is_provision_success()){
        return ;// remote prov cannot clear the para for the remoteprov server part 
    }
    #endif

	memset((u8 *)(&prov_para),0,OFFSETOF(prov_para_proc_t, rsp_ack_transnum));
	prov_para.provison_send_state = LINK_UNPROVISION_STATE;
	prov_para.provison_rcv_state= LINK_UNPROVISION_STATE;
	#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
	prov_para.initial_pro_roles = MESH_INI_ROLE_PROVISIONER;
	#if(__PROJECT_MESH_GW_NODE__)
	if(!is_provision_success()){
		prov_para.initial_pro_roles = MESH_INI_ROLE_NODE;
	}
	#endif
	#else
	prov_para.initial_pro_roles = MESH_INI_ROLE_NODE;
	#endif
	if(provision_mag.gatt_mode != GATT_PROXY_MODE){
		#if PROVISION_GATT_ENABLE 
		provision_mag.gatt_mode = GATT_PROVISION_MODE;
		#else
		provision_mag.gatt_mode = GATT_ADV_NORMAL_MODE;
		#endif 
	}
#endif 
	return ;

}

void reset_uuid_create_flag()
{
	// save provision_mag struct part 
	provision_mag_cfg_s_store();
}

void link_open_init(pro_PB_ADV* p_pb_adv ,u8 transnum,u8 *p_uuid,u8 *p_link)
{
	p_pb_adv->length = 7+sizeof(bear_open_link);
	p_pb_adv->ad_type = MESH_ADV_TYPE_PRO;
	memcpy(p_pb_adv->link_ID,p_link,4);
	p_pb_adv->trans_num = transnum;
	p_pb_adv->transBear.bearOpen.header.BearCtl =LINK_OPEN;
	p_pb_adv->transBear.bearOpen.header.GPCF = BEARS_CTL;
	memcpy(p_pb_adv->transBear.bearOpen.DeviceUuid,p_uuid,16);
}

void link_ack_init(pro_PB_ADV* p_pb_adv,u8 transnum,u8 *p_link)
{
	p_pb_adv->length = 7+ sizeof(bear_ack_link);
	p_pb_adv->ad_type = MESH_ADV_TYPE_PRO;
	memcpy(p_pb_adv->link_ID,p_link,4);
	p_pb_adv->trans_num = transnum;
	p_pb_adv->transBear.bearOpen.header.BearCtl =LINK_ACK;
	p_pb_adv->transBear.bearOpen.header.GPCF = BEARS_CTL;
}

u8 set_pro_invite(mesh_pro_data_structer *p_str,u8 att_dura)
{
	pro_trans_invite *p_invite = &(p_str->invite);
	p_invite->header.type = PRO_INVITE;
	p_invite->attentionDura = att_dura ;
	return 1;
}
u8 swap_mesh_pro_capa(mesh_pro_data_structer *p_str)
{
	endianness_swap_u16((u8 *)(&p_str->capa.alogrithms));
	endianness_swap_u16((u8 *)(&p_str->capa.outPutOOBAct));
	endianness_swap_u16((u8 *)(&p_str->capa.inOOBAct));
	return 1;
}

u8 set_pro_capa_cpy(mesh_pro_data_structer *p_str,mesh_prov_oob_str *p_prov_oob)
{
	pro_trans_capa * p_capa = &(p_str->capa);
	p_capa->header.type = PRO_CAPABLI;
	p_capa->header.rfu= 0;
	memcpy((u8 *)&(p_capa->ele_num),(u8 *)&(p_prov_oob->capa.ele_num),sizeof(pro_trans_capa)-1);
	return 1;
}
u8 set_pro_capa(mesh_pro_data_structer *p_str,u8 ele_num,u16 alogr,u8 keytype,
				u8 outOOBsize,u16 outAct,u8 inOOBsize,u16 inOOBact)
{
	pro_trans_capa *p_capa =  &(p_str->capa);
	p_capa->header.type = PRO_CAPABLI;
	p_capa->ele_num = ele_num;
	p_capa->alogrithms = alogr;
	p_capa->pubKeyType = keytype;
	p_capa->staticOOBType= 0;
	p_capa->outPutOOBSize = outOOBsize;
	p_capa->outPutOOBAct = outAct ;
	p_capa->inOOBSize = inOOBsize;
	p_capa->inOOBAct = inOOBact ;
	return 1;
}
u8 set_pro_start_simple(mesh_pro_data_structer *p_str ,pro_trans_start *p_start)
{
	return set_pro_start(p_str,p_start->alogrithms,p_start->pubKey,
			p_start->authMeth,p_start->authAct,p_start->authSize);

}
u8 set_pro_start(mesh_pro_data_structer *p_str,u8 alogri,u8 pubkey,
					u8 authmeth,u8 authact,u8 authsize )
{
	pro_trans_start *p_start = &(p_str->start);
	p_start->header.type = PRO_START;
	p_start->alogrithms = alogri;
	p_start->pubKey = pubkey;
	p_start->authMeth = authmeth;
	p_start->authAct = authact;
	p_start->authSize = authsize;
	return 1;
}

u8 set_pro_pub_key(mesh_pro_data_structer *p_str,u8 *p_pubkeyx,u8 *p_pubkeyy){
	pro_trans_pubkey *p_pubkey = &(p_str->pubkey);
	p_pubkey->header.type = PRO_PUB_KEY;
	memcpy(p_pubkey->pubKeyX,p_pubkeyx,32);
	memcpy(p_pubkey->pubKeyY,p_pubkeyy,32);
	return 1;
}

u8 set_pro_input_complete(mesh_pro_data_structer *p_str)
{
	pro_trans_incomplete *p_com = &(p_str->inComp);
	p_com->header.type = PRO_INPUT_COM;
	return 1;
}
u8 set_pro_comfirm(mesh_pro_data_structer *p_str,u8 *p_comfirm)
{
	pro_trans_comfirm *p_comfirm_t = &(p_str->comfirm);	
	p_comfirm_t->header.type = PRO_CONFIRM;
	memcpy(p_comfirm_t->comfirm,p_comfirm,16);
	return 1;
}

u8 set_pro_random(mesh_pro_data_structer *p_str,u8 *p_random)
{
	pro_trans_random *p_random_t = &(p_str->random);
	p_random_t->header.type = PRO_RANDOM;
	memcpy(p_random_t->random,p_random,16);
	return 1;
}

u8 set_pro_data(mesh_pro_data_structer *p_str, u8 *p_data,u8 *p_mic)
{
	pro_trans_data *p_data_t = &(p_str->data);	
	p_data_t->header.type = PRO_DATA;
	memcpy(p_data_t->encProData,p_data,25);
	memcpy(p_data_t->proDataMic,p_mic,8);
	return 1;
}

u8 set_pro_complete(mesh_pro_data_structer *p_str)
{
	pro_trans_complete *p_complete = &(p_str->complete);
	p_complete->header.type =PRO_COMPLETE;
	return 1;
}

u8 set_pro_fail(mesh_pro_data_structer *p_str ,u8 fail_code)
{
	pro_trans_fail *p_fail = &(p_str->fail);	
	p_fail->header.type =PRO_FAIL;
	p_fail->err_code = fail_code;
	return 1;
}

u8 mesh_send_provison_data(u8 pdu_type,u8 bearCode,u8 *para,u8 para_len )
{
	#if FEATURE_PROV_EN
	if(para_len>31||pdu_type>BEARS_CTL){
		return 0;
	}
	// for the bearerctl
	u8 bearerCode=0;
	u8 bearerLen=0;
	u8 trans_ack_flag =0;
	u8 link_close_flag =0;
	pro_PB_ADV *p_pro_pb_adv;
	
	#if TESTCASE_FLAG_ENABLE
	p_pro_pb_adv = (pro_PB_ADV *)(&(pro_adv_pkt.len));
	#else
	mesh_cmd_bear_unseg_t cmd_bear_tmp = {0};
	p_pro_pb_adv = (pro_PB_ADV *)(&(cmd_bear_tmp.len));
	#endif
	p_pro_pb_adv->ad_type = MESH_ADV_TYPE_PRO;
	p_pro_pb_adv->transAck.GPCF = pdu_type;
	switch(pdu_type){
		case TRANS_START:
		    {
		    
			// initial the segment idx , the first cmd of the provisioner and device will not increase
			
			prov_para.segmentIdx=0;
			p_pro_pb_adv->length = 4+6+para_len;
			memcpy(p_pro_pb_adv->link_ID,(u8*)(&(prov_para.link_id)),4);
			p_pro_pb_adv->transStart.SegN = prov_para.last_segmentIdx;
			p_pro_pb_adv->transStart.total_len =prov_para.trans_seg_total_len;
			endianness_swap_u16((u8*)(&(p_pro_pb_adv->transStart.total_len)));
			p_pro_pb_adv->transStart.FCS =prov_para.trans_start_fcs;
			memcpy(p_pro_pb_adv->transStart.data,para,para_len);
			u8 prov_cmd = (p_pro_pb_adv->transStart.data[0])&0x3f;
			if(!prov_para.cmd_retry_flag && prov_cmd!= PRO_CAPABLI)
				prov_para.trans_num++;
			// after the sending ,it will change the transnum part 
		    }
			break;
		case TRANS_ACK:
			trans_ack_flag =1;
			p_pro_pb_adv->length = 1+6;// no para 
			memcpy(p_pro_pb_adv->link_ID,(u8*)(&(prov_para.link_id)),4);
			p_pro_pb_adv->transAck.rfu =0;
			break;
		case TRANS_CON:
			prov_para.segmentIdx++;
			p_pro_pb_adv->length = 1+6+para_len;// no para 
			memcpy(p_pro_pb_adv->link_ID,(u8*)(&(prov_para.link_id)),4);
			p_pro_pb_adv->transCon.SegIdx = prov_para.segmentIdx;
			memcpy(p_pro_pb_adv->transCon.data,para,para_len);
			break;
		case BEARS_CTL:
			bearerCode = bearCode;
			memcpy(p_pro_pb_adv->link_ID,(u8*)(&(prov_para.link_id)),4);
		//	bearerCode = p_pro_pb_adv->transBear.bearOpen.header.BearCtl;
			p_pro_pb_adv->transBear.bearOpen.header.BearCtl=bearerCode;
			if(bearerCode == LINK_OPEN){
				//transaction num should changes when send link open 
				if(!prov_para.cmd_retry_flag){
					prov_para.trans_num++; 
                    prov_para.link_id++;
				}
				prov_para.trans_num =0;
				bearerLen =17;
				memcpy(p_pro_pb_adv->link_ID,(u8 *)(&prov_para.link_id),4);
				memcpy(p_pro_pb_adv->transBear.bearOpen.DeviceUuid,para,16);               
			}else if(bearerCode == LINK_ACK){
				bearerLen = 1;
				trans_ack_flag =1;
			}else if(bearerCode == LINK_CLOSE){
				link_close_flag =1;
				// transaction num should changes when send link close
				p_pro_pb_adv->transBear.bearCls.reason = prov_link_cls_code;
				bearerLen = 2;
			}else{}
			//caculate the total length of the data 
			p_pro_pb_adv->length = 6+bearerLen;
			break;
		default:
			return 0;
			break;
	}
	if(prov_para.initial_pro_roles == MESH_INI_ROLE_PROVISIONER){
		prov_para.trans_num &= 0x7f;
		p_pro_pb_adv->trans_num = prov_para.trans_num;
	}else if(prov_para.initial_pro_roles == MESH_INI_ROLE_NODE){
		prov_para.trans_num |=  0x80;
		p_pro_pb_adv->trans_num = prov_para.trans_num;		
	}else{}
	// add the transaction num 
	if(trans_ack_flag){
		p_pro_pb_adv->trans_num = prov_para.rsp_ack_transnum;
	}
	if(link_close_flag){
		prov_para.trans_num =0;
	}
	// if the data can be successful dispatch ,add the data to the fifo 
	#if(TESTCASE_FLAG_ENABLE)
	pro_adv_pkt.trans_par_val = 0x21;
	mesh_tx_cmd_add_packet((u8 *)(&pro_adv_pkt));
	#else
	cmd_bear_tmp.trans_par_val = 0x02;
	mesh_tx_cmd_add_packet((u8 *)(&cmd_bear_tmp));
	#endif
//	LAYER_DATA_DEBUG(A_buf4_pro, (u8 *)(&pro_adv_pkt), sizeof(mesh_cmd_bear_unseg_t));

	#endif 
	return 1;
}

u8 mesh_send_multi_data(u8 *para , u8 para_len)
{
	u8 p_idx=0;
	u8 start_pkt_flag =1;
	prov_para.trans_seg_total_len = para_len;
	prov_para.last_segmentIdx=0;
	prov_para.trans_start_fcs = crc8_rohc(para,(u16)para_len);
	if(para_len<=20){
		prov_para.last_segmentIdx=0;
		mesh_send_provison_data(TRANS_START,0,para,para_len);
	}else{
		while(para_len){
			u8 data_len;
			if(start_pkt_flag){
				start_pkt_flag =0;
				para_len -=20;
				prov_para.last_segmentIdx=1+para_len/23;
				mesh_send_provison_data(TRANS_START,0,para,20);
				p_idx += 20; 
				continue;
			}
			data_len = (para_len>=23)?23:para_len;
			para_len -=data_len;
			mesh_send_provison_data(TRANS_CON,0,para+p_idx,data_len);
			p_idx += data_len;
		}
	}
	return 1;
}

int mesh_provision_rcv_process (u8 *p_payload, u32 t)
{
	// provison 's flow control
	#if FEATURE_PROV_EN
	mesh_provison_process(prov_para.initial_pro_roles ,(p_payload));
	#endif 
	return 1;
}

u8 get_mesh_pro_str_len(u8 type)
{
	u8 len=0;
	mesh_pro_pdu_content *p_content;
	mesh_pro_pdu_content pro_pdu_content;
	p_content = &pro_pdu_content;
	switch(type){
		case PRO_INVITE:
			len = sizeof(p_content->invite);
			break;
		case PRO_CAPABLI:
			len = sizeof(p_content->capa);
			break;
		case PRO_START:
			len = sizeof(p_content->start);
			break;
		case PRO_PUB_KEY:
			len = sizeof(p_content->pubkey);
			break;
		case PRO_INPUT_COM:
			len = sizeof(p_content->inComp);
			break;
		case PRO_CONFIRM:
			len = sizeof(p_content->comfirm);
			break;
		case PRO_RANDOM:
			len = sizeof(p_content->random);
			break;
		case PRO_DATA:
			len = sizeof(p_content->data);
			break;
		case PRO_COMPLETE:
			len = sizeof(p_content->complete);
			break;
		case PRO_FAIL:
			len = sizeof(p_content->fail);
			break;
		default:
			break;
	}
	return len ;
}

u8 send_multi_type_data(u8 type,u8 *p_para)
{
	u8 len;
	len = get_mesh_pro_str_len(type);
	p_para[0]=type;
	#if FEATURE_PROV_EN
	mesh_send_multi_data(p_para,len);
	#endif 
	return len;
}

//unsigned char r[16], r1[16];//, r2[16], salt[16];

const unsigned char psk[] = {0x06,0xa5,0x16,0x69,0x3c,0x9a,0xa3,0x1a, 0x60,0x84,0x54,0x5d,0x0c,0x5d,0xb6,0x41,
					   0xb4,0x85,0x72,0xb9,0x72,0x03,0xdd,0xff, 0xb7,0xac,0x73,0xf7,0xd0,0x45,0x76,0x63};

const unsigned char ppk[] = { 0x2c,0x31,0xa4,0x7b,0x57,0x79,0x80,0x9e, 0xf4,0x4c,0xb5,0xea,0xaf,0x5c,0x3e,0x43,
						0xd5,0xf8,0xfa,0xad,0x4a,0x87,0x94,0xcb, 0x98,0x7e,0x9b,0x03,0x74,0x5c,0x78,0xdd,
						0x91,0x95,0x12,0x18,0x38,0x98,0xdf,0xbe, 0xcd,0x52,0xe2,0x40,0x8e,0x43,0x87,0x1f,
						0xd0,0x21,0x10,0x91,0x17,0xbd,0x3e,0xd4, 0xea,0xf8,0x43,0x77,0x43,0x71,0x5d,0x4f};

const unsigned char dsk[] = { 0x52,0x9a,0xa0,0x67,0x0d,0x72,0xcd,0x64, 0x97,0x50,0x2e,0xd4,0x73,0x50,0x2b,0x03,
						0x7e,0x88,0x03,0xb5,0xc6,0x08,0x29,0xa5, 0xa3,0xca,0xa2,0x19,0x50,0x55,0x30,0xba};
const unsigned char dpk[] = { 0xf4,0x65,0xe4,0x3f,0xf2,0x3d,0x3f,0x1b, 0x9d,0xc7,0xdf,0xc0,0x4d,0xa8,0x75,0x81,
						0x84,0xdb,0xc9,0x66,0x20,0x47,0x96,0xec, 0xcf,0x0d,0x6c,0xf5,0xe1,0x65,0x00,0xcc,
						0x02,0x01,0xd0,0x48,0xbc,0xbb,0xd8,0x99, 0xee,0xef,0xc4,0x24,0x16,0x4e,0x33,0xc2,
						0x01,0xc2,0xb0,0x10,0xca,0x6b,0x4d,0x43, 0xa8,0xa1,0x55,0xca,0xd8,0xec,0xb2,0x79};

const unsigned char ecdh[] = {0xab,0x85,0x84,0x3a,0x2f,0x6d,0x88,0x3f, 0x62,0xe5,0x68,0x4b,0x38,0xe3,0x07,0x33,
						0x5f,0xe6,0xe1,0x94,0x5e,0xcd,0x19,0x60, 0x41,0x05,0xc6,0xf2,0x32,0x21,0xeb,0x69};
const unsigned char prn[] = { 0x8b,0x19,0xac,0x31,0xd5,0x8b,0x12,0x4c, 0x94,0x62,0x09,0xb5,0xdb,0x10,0x21,0xb9};
const unsigned char drn[] = { 0x55,0xa2,0xa2,0xbc,0xa0,0x4c,0xd3,0x2f, 0xf6,0xf3,0x46,0xbd,0x0a,0x0c,0x1a,0x3a};

const unsigned char input[] ={0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x2c,0x31,0xa4,0x7b,0x57,0x79,0x80, 0x9e,0xf4,0x4c,0xb5,0xea,0xaf,0x5c,0x3e,
						0x43,0xd5,0xf8,0xfa,0xad,0x4a,0x87,0x94, 0xcb,0x98,0x7e,0x9b,0x03,0x74,0x5c,0x78,
						0xdd,0x91,0x95,0x12,0x18,0x38,0x98,0xdf, 0xbe,0xcd,0x52,0xe2,0x40,0x8e,0x43,0x87,
						0x1f,0xd0,0x21,0x10,0x91,0x17,0xbd,0x3e, 0xd4,0xea,0xf8,0x43,0x77,0x43,0x71,0x5d,
						0x4f,0xf4,0x65,0xe4,0x3f,0xf2,0x3d,0x3f, 0x1b,0x9d,0xc7,0xdf,0xc0,0x4d,0xa8,0x75,
						0x81,0x84,0xdb,0xc9,0x66,0x20,0x47,0x96, 0xec,0xcf,0x0d,0x6c,0xf5,0xe1,0x65,0x00,
						0xcc,0x02,0x01,0xd0,0x48,0xbc,0xbb,0xd8, 0x99,0xee,0xef,0xc4,0x24,0x16,0x4e,0x33,
						0xc2,0x01,0xc2,0xb0,0x10,0xca,0x6b,0x4d, 0x43,0xa8,0xa1,0x55,0xca,0xd8,0xec,0xb2,
						0x79};

const unsigned char auth[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

const unsigned char r_salt[] = {0x5f,0xaa,0xbe,0x18,0x73,0x37,0xc7,0x1c, 0xc6,0xc9,0x73,0x36,0x9d,0xca,0xa7,0x9a};
const unsigned char r_ck[16] = {0xe3,0x1f,0xe0,0x46,0xc6,0x8e,0xc3,0x39, 0xc4,0x25,0xfc,0x66,0x29,0xf0,0x33,0x6f};
const unsigned char r_pcfm[16] = {0xb3,0x8a,0x11,0x4d,0xfd,0xca,0x1f,0xe1, 0x53,0xbd,0x2c,0x1e,0x0d,0xc4,0x6a,0xc2};
const unsigned char r_dcfm[16] = {0xee,0xba,0x52,0x1c,0x19,0x6b,0x52,0xcc, 0x2e,0x37,0xaa,0x40,0x32,0x9f,0x55,0x4e};
const unsigned char r_sk[16] = {0xc8,0x02,0x53,0xaf,0x86,0xb3,0x3d,0xfa, 0x45,0x0b,0xbd,0xb2,0xa1,0x91,0xfe,0xa3};
const unsigned char r_sn[16] = {0xda,0x7d,0xdb,0xe7,0x8b,0x5f,0x62,0xb8, 0x1d,0x68,0x47,0x48,0x7e,};
const unsigned char r_dat[40] = {0xef,0xb2,0x25,0x5e,0x64,0x22,0xd3,0x30, 0x08,0x8e,0x09,0xbb,0x01,0x5e,0xd7,0x07,	
						 0x05,0x67,0x00,0x01,0x02,0x03,0x04,0x0b, 0x0c};
const unsigned char r_enc[] = {0xd0,0xbd,0x7f,0x4a,0x89,0xa2,0xff,0x62, 0x22,0xaf,0x59,0xa9,0x0a,0x60,0xad,0x58,
						 0xac,0xfe,0x31,0x23,0x35,0x6f,0x5c,0xec, 0x29};
const unsigned char r_mic[] = {0x73,0xe0,0xec,0x50,0x78,0x3b,0x10,0xc7};

//unsigned char r_ecdh[32]; 
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
_attribute_no_retention_bss_ u8 pro_edch[32];
u8 pro_random[16]={0x8b,0x19,0xac,0x31,0xd5,0x8b,0x12,0x4c, 0x94,0x62,0x09,0xb5,0xdb,0x10,0x21,0xb9};
_attribute_no_retention_bss_ u8 pro_comfirm[16];
_attribute_no_retention_bss_ u8 pro_input[145]/*={		0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x2c,0x31,0xa4,0x7b,0x57,0x79,0x80, 0x9e,0xf4,0x4c,0xb5,0xea,0xaf,0x5c,0x3e,
						0x43,0xd5,0xf8,0xfa,0xad,0x4a,0x87,0x94, 0xcb,0x98,0x7e,0x9b,0x03,0x74,0x5c,0x78,
						0xdd,0x91,0x95,0x12,0x18,0x38,0x98,0xdf, 0xbe,0xcd,0x52,0xe2,0x40,0x8e,0x43,0x87,
						0x1f,0xd0,0x21,0x10,0x91,0x17,0xbd,0x3e, 0xd4,0xea,0xf8,0x43,0x77,0x43,0x71,0x5d,
						0x4f,0xf4,0x65,0xe4,0x3f,0xf2,0x3d,0x3f, 0x1b,0x9d,0xc7,0xdf,0xc0,0x4d,0xa8,0x75,
						0x81,0x84,0xdb,0xc9,0x66,0x20,0x47,0x96, 0xec,0xcf,0x0d,0x6c,0xf5,0xe1,0x65,0x00,
						0xcc,0x02,0x01,0xd0,0x48,0xbc,0xbb,0xd8, 0x99,0xee,0xef,0xc4,0x24,0x16,0x4e,0x33,
						0xc2,0x01,0xc2,0xb0,0x10,0xca,0x6b,0x4d, 0x43,0xa8,0xa1,0x55,0xca,0xd8,0xec,0xb2,
						0x79}*/;
_attribute_no_retention_bss_ u8  pro_auth[16]/* = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}*/;
_attribute_no_retention_bss_ u8  dev_sig_comfirm[16];

u8 dev_random[16];
_attribute_no_retention_bss_ u8 pro_session_key[16];
_attribute_no_retention_bss_ u8 pro_session_nonce_tmp[16];
_attribute_no_retention_bss_ u8 pro_session_nonce[16];

_attribute_no_retention_data_ u8 prov_net_key[16]/*={0x11,0x22,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf}*/;
const u8 const_prov_ivi[4]={0x11,0x22,0x33,0x44};

_attribute_no_retention_bss_ u8 pro_dat[40]/* = {0x11,0x22,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,	
						 0x00,0x00,0x00,0x11,0x22,0x33,0x44,0x04,0x00}*/;						 
u8 set_provision_networkkey_self(u8 *p_key,u8 len )
{
	if(len >16){
		return 0;
	}
	provison_net_info_str *p_str = (provison_net_info_str *)pro_dat;
	memcpy(prov_net_key,p_key,len);
	memcpy(p_str->net_work_key,prov_net_key,16);
	set_pro_dat_part(PROVISION_ELE_ADR);
	return 1;
}

void set_provisionee_para(u8 *p_net_key,u16 key_index,u8 flags,u8 *p_ivi,u16 unicast)
{
	provison_net_info_str *p_str = (provison_net_info_str *)pro_dat;
	if(p_net_key){
		memcpy(p_str->net_work_key,p_net_key,sizeof(p_str->net_work_key));
	}else{
		memcpy(p_str->net_work_key,prov_net_key,sizeof(prov_net_key));
	}
	p_str->key_index = key_index;
	p_str->flags = flags;
	if(p_ivi){
		memcpy(p_str->iv_index,p_ivi,sizeof(p_str->iv_index));
	}else{
		memcpy(p_str->iv_index,const_prov_ivi,sizeof(p_str->iv_index));
	}
	p_str->unicast_address = unicast;
	return ;
}
void set_provisioner_para(u8 *p_net_key,u16 key_index,u8 flags,u8 *p_ivi,u16 unicast)
{
	u8 pro_dat_tmp[25];
	set_provisionee_para(p_net_key,key_index,flags,p_ivi,unicast);
	memcpy(pro_dat_tmp,pro_dat,sizeof(pro_dat_tmp));
	mesh_provision_par_set_dir(pro_dat_tmp);
}
u8 set_pro_dat_part(u16 ele_adr)
{
	set_provisioner_para(0,0,0,0,ele_adr);
	return 0;
}

void set_pro_unicast_adr(u16 unicast)
{
	provison_net_info_str *p_str = (provison_net_info_str *)pro_dat;
	p_str->unicast_address = unicast;
}

#if WIN32
void pro_random_init()
{

	for(int i=0;i<sizeof(gatt_pro_random);i++){
		gatt_pro_random[i]= win32_create_rand()&0xff;
	}
}
#else
void pro_random_init()
{
	generateRandomNum(sizeof(pro_random), pro_random);
}
#endif
#endif

_attribute_bss_retention_ u8 dev_auth[16]/* = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}*/;
#if(!__PROJECT_MESH_PRO__)
_attribute_no_retention_bss_ u8 dev_edch[32];
u8 dev_random[16]={0x55,0xa2,0xa2,0xbc,0xa0,0x4c,0xd3,0x2f, 0xf6,0xf3,0x46,0xbd,0x0a,0x0c,0x1a,0x3a};
_attribute_no_retention_bss_ u8 dev_comfirm[16];
_attribute_no_retention_bss_ u8 dev_input[0x91]/*={		0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x2c,0x31,0xa4,0x7b,0x57,0x79,0x80, 0x9e,0xf4,0x4c,0xb5,0xea,0xaf,0x5c,0x3e,
						0x43,0xd5,0xf8,0xfa,0xad,0x4a,0x87,0x94, 0xcb,0x98,0x7e,0x9b,0x03,0x74,0x5c,0x78,
						0xdd,0x91,0x95,0x12,0x18,0x38,0x98,0xdf, 0xbe,0xcd,0x52,0xe2,0x40,0x8e,0x43,0x87,
						0x1f,0xd0,0x21,0x10,0x91,0x17,0xbd,0x3e, 0xd4,0xea,0xf8,0x43,0x77,0x43,0x71,0x5d,
						0x4f,0xf4,0x65,0xe4,0x3f,0xf2,0x3d,0x3f, 0x1b,0x9d,0xc7,0xdf,0xc0,0x4d,0xa8,0x75,
						0x81,0x84,0xdb,0xc9,0x66,0x20,0x47,0x96, 0xec,0xcf,0x0d,0x6c,0xf5,0xe1,0x65,0x00,
						0xcc,0x02,0x01,0xd0,0x48,0xbc,0xbb,0xd8, 0x99,0xee,0xef,0xc4,0x24,0x16,0x4e,0x33,
						0xc2,0x01,0xc2,0xb0,0x10,0xca,0x6b,0x4d, 0x43,0xa8,0xa1,0x55,0xca,0xd8,0xec,0xb2,
						0x79}*/;
u8 pro_random[16];

_attribute_no_retention_bss_ u8 dev_pro_comfirm[16];
void dev_random_init()
{
	generateRandomNum(sizeof(dev_random), dev_random);
}
#endif 
void provision_random_data_init()
{
	// use the initial data part 
	#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
	pro_random_init();
	#endif
	#if(!__PROJECT_MESH_PRO__)
	dev_random_init();
	#endif
}

void send_rcv_retry_set(u8 cmd, u8 bearerCode, u8 ack_retry_flag)
{
	prov_para.pro_cmd = cmd ;
    prov_para.pro_bearerCode = bearerCode;
	prov_para.cmd_send_tick = clock_time();
	prov_para.cmd_send_start_tick = clock_time();
	prov_para.cmd_retry_flag =1;
	prov_para.ack_retry_flag = ack_retry_flag;
}
void prov_set_link_close_code(u8 code)
{
	prov_link_cls_code = code;
}
void send_rcv_retry_clr()
{
	prov_para.cmd_send_tick = clock_time();
	prov_para.cmd_retry_flag =0;
	prov_para.ack_retry_flag =0;
	prov_link_cls_code = LINK_CLOSE_SUCC;
	my_fifo_reset(&mesh_adv_cmd_fifo);
}

// send multi packets 
// GATT part dispatch 
int notify_pkts(u8 *p,u16 len,u16 handle,u8 proxy_type)
{
	int err = -1;
#if !WIN32 
	u8 tmp[256];
	u8 pkt_no=0;
	u8 buf_idx =0;
	u8 total_len;
	u8 pkt_len =0;
	u8 valid_len = bltAtt.effective_MTU- 4; // opcode 1 + handle_id 2 + sar 1
	
	if(len ==0){
		pkt_len = 0;
	}else {
		pkt_len = 1+(len-1)/valid_len;
	}
	total_len =len;
	#if !WIN32 
	// reserve more fifo for the tx fifo part 
	if(blc_ll_getTxFifoNumber()+ pkt_len >= (blt_txfifo.num - 2 - 3 )){
		return err;
	}
	#endif 
	// dispatch the notification flag
	extern u8  provision_Out_ccc[2];
	if(	handle == PROVISION_ATT_HANDLE && 
		provision_Out_ccc[0]==0 && provision_Out_ccc[1]==0 ){
		#if (!DEBUG_MESH_DONGLE_IN_VC_EN)
		return err;
		#endif
	}
	pb_gatt_proxy_str *p_notify = (pb_gatt_proxy_str *)(tmp);
	//can send in one packet
	if(len==0){
		return err;
	}
	if(len>valid_len){
		while(len){			
			if(!pkt_no){
				//send the first pkt
				p_notify->sar = SAR_START;
				p_notify->msgType = proxy_type;					
				memcpy(p_notify->data,p,valid_len);
				err = bls_att_pushNotifyData(handle,tmp,valid_len+1);
				len = len-valid_len;
				buf_idx +=valid_len;
				pkt_no++;
			}else{
				// the last pkt 
				if(buf_idx+valid_len>=total_len){
					p_notify->sar = SAR_END;
					p_notify->msgType = proxy_type;
					memcpy(p_notify->data,p+buf_idx,total_len-buf_idx);
					err = bls_att_pushNotifyData(handle,tmp,total_len-buf_idx+1);
					len =0;
				}else{
				// send the continus pkt 
					p_notify->sar = SAR_CONTINUS;
					p_notify->msgType = proxy_type;
					memcpy(p_notify->data,p+buf_idx,valid_len); //
					err = bls_att_pushNotifyData(handle,tmp,valid_len+1);
					len = len-valid_len;
					buf_idx +=valid_len;
				}
			}

			if(err){
				break;
			}
		}

	}else{
	// send the complete pkt 
		p_notify->sar = SAR_COMPLETE;
		p_notify->msgType = proxy_type;
		memcpy(p_notify->data,p,len);
		err = bls_att_pushNotifyData(handle,tmp,len+1);
		
	}
	#endif
	return err;

}

int gatt_prov_notify_pkts(u8 *p,u16 len,u16 handle,u8 proxy_type)
{
    notify_pkts(p,len,handle,proxy_type);
    return 0;
}


u8 prov_auth_en_flag =0;
u8 prov_auth_val = 0;
u32 prov_oob_output_auth = 0;

void mesh_set_dev_auth(u8 *p_auth, u8 len)
{
	if(len > 16){
		return;
	}		
	memcpy(dev_auth, p_auth, len);
	return;
}

#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
int get_auth_value_by_uuid(u8 *uuid_in,u8 *oob_out);
const u8 con_prov_static_oob[16] ={ 0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
					0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f};

void mesh_set_pro_auth(u8 *p_auth, u8 len)
{
	if(len > 16){
		return;
	}
	memcpy(pro_auth, p_auth, len);
	return;
}

u8 mesh_prov_oob_auth_data(mesh_prov_oob_str *p_prov_oob)
{
	
	pro_trans_start *p_start = &(p_prov_oob->start);
	u8 ret =0;
	if(p_start->authMeth == MESH_NO_OOB) {
		memset(pro_auth ,0,sizeof(pro_auth));
		#if WIN32
		memset(gatt_pro_auth, 0x00, sizeof(gatt_pro_auth));
		#endif
		ret = 1;
	}else if (p_start->authMeth == MESH_STATIC_OOB){
	#if WIN32
        u8 prov_oob_static[32] = {0}; // must 32 byte for sha256 output.
		#if VC_APP_ENABLE
		int err = -1;
		err = get_auth_value_by_uuid(gatt_provision_mag.device_uuid, prov_oob_static);
		#endif		
		memcpy(gatt_pro_auth, prov_oob_static, 16);
	#else
		#if PTS_TEST_EN
		mesh_set_pro_auth(con_prov_static_oob,sizeof(con_prov_static_oob));
		#endif
	#endif 
		ret =1;
	}else if (p_start->authMeth == MESH_OUTPUT_OOB){
		// need to input the para part 
		if(p_prov_oob->oob_outAct == MESH_OUT_ACT_BLINK){

		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_BEEP){
			u8 tmp_auth[4];
			memcpy(tmp_auth,(u8 *)(&prov_oob_output_auth),4);
			endianness_swap_u32(tmp_auth);
			memcpy(pro_auth+12,tmp_auth,4);
			ret = 1;
		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_NUMBERIC){
			memset(pro_auth,0,sizeof(pro_auth));
			pro_auth[15]= prov_auth_val;
			ret = 1;
		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_ALPHA){
				
		}
	}else if (p_start->authMeth == MESH_INPUT_OOB){
		if(prov_oob.start.pubKey == MESH_PUB_KEY_WITH_OOB){
			const u8 const_mesh_auth_input[16]={0x32,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
			mesh_set_pro_auth((u8 *)const_mesh_auth_input,sizeof(const_mesh_auth_input));
		}else{
			const u8 const_mesh_auth_input_no_oob[16]={ 0,0,0,0, 0,0,0,0 ,0,0,0,0, 0,0,0,2};
			mesh_set_pro_auth((u8 *)const_mesh_auth_input_no_oob,sizeof(const_mesh_auth_input_no_oob));
		}
		
		ret =1;
	}
	return ret ;
}


u8 set_start_para_by_capa(mesh_prov_oob_str *p_prov_oob)
{
	pro_trans_start *p_start = &(p_prov_oob->start);
	p_start->alogrithms = 0;
	p_start->pubKey =  p_prov_oob->capa.pubKeyType;
	p_start->authMeth = p_prov_oob->prov_key;
	if(p_start->authMeth == MESH_NO_OOB || p_start->authMeth == MESH_STATIC_OOB){
		p_start->authAct =0;
		p_start->authSize = 0;
	}else if (p_start->authMeth == MESH_OUTPUT_OOB ){
		p_start->authAct = p_prov_oob->oob_outAct ;
		p_start->authSize = p_prov_oob->oob_outsize ;
	}else if (p_start->authMeth == MESH_INPUT_OOB ){
		p_start->authAct = p_prov_oob->oob_inAct ;
		p_start->authSize = p_prov_oob->oob_insize ;
	}
	return 1;
}

void send_comfirm_no_pubkey_cmd()
{
	mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
	u8 prov_private_key[32];
	get_private_key(prov_private_key);
	tn_p256_dhkey (pro_edch, prov_private_key,pro_input+0x11+0x40 ,pro_input+0x11+0x60);
	mesh_sec_prov_confirmation (pro_comfirm, pro_input, 
								145, pro_edch, pro_random, pro_auth);												
	set_pro_comfirm(p_send_str,pro_comfirm);
	send_multi_type_data(PRO_CONFIRM,para_pro);
	send_rcv_retry_set(PRO_CONFIRM,0,0);
	prov_para.trans_num_last = prov_para.trans_num;
	prov_para.provison_send_state = STATE_DEV_CONFIRM;	
}

void send_comfirm_no_pubkey_cmd_with_ack()
{
	mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
	u8 prov_private_key[32];
	get_private_key(prov_private_key);
	tn_p256_dhkey (pro_edch, prov_private_key,pro_input+0x11+0x40 ,pro_input+0x11+0x60);
	mesh_sec_prov_confirmation (pro_comfirm, pro_input, 
								145, pro_edch, pro_random, pro_auth);
	mesh_adv_prov_comfirm_cmd(p_send_str,pro_comfirm);
	prov_para.provison_send_state = STATE_DEV_CONFIRM;
	
	return ;

}
#endif

#if(!__PROJECT_MESH_PRO__) // the node part dispatch the oob part 
void mesh_set_oob_type(u8 type, u8 *p_oob ,u8 len )
{
	pro_trans_capa * p_capa  = 	&(prov_oob.capa);
	
	if(type == MESH_NO_OOB){
		p_capa->staticOOBType = MESH_NO_OOB;
	}else if (type == MESH_STATIC_OOB){
		p_capa->staticOOBType = MESH_STATIC_OOB;
		if(p_oob!=0){
			mesh_set_dev_auth(p_oob,len );
		}
	}
	p_capa->pubKeyType = 0;
	p_capa->ele_num =g_ele_cnt;
	p_capa->alogrithms = FIPS_P256_Ellipti_CURVE;
	p_capa->outPutOOBSize = 0;
	p_capa->outPutOOBAct = 0;
	p_capa->inOOBSize = 0;
	p_capa->inOOBAct = 0;
}

u8 mesh_node_oob_auth_data(mesh_prov_oob_str *p_prov_oob)
{
	
	pro_trans_start *p_start = &(p_prov_oob->start);
	u8 ret =0;
	if(p_start->authMeth == MESH_NO_OOB) {
		memset(dev_auth ,0,sizeof(dev_auth));
		ret = 1;
	}else if (p_start->authMeth == MESH_STATIC_OOB){
        #if DEBUG_EVB_EN
        uint8_t auth_value[16] = {0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
        mesh_set_dev_auth(auth_value,sizeof(auth_value));
		#elif PTS_TEST_EN
        uint8_t auth_value[16] = {0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
								  0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f};
        mesh_set_dev_auth(auth_value,sizeof(auth_value));
		#endif
		ret =1;
	}else if (p_start->authMeth == MESH_OUTPUT_OOB){
		// need to input the para part 
		if(p_prov_oob->oob_outAct == MESH_OUT_ACT_BLINK){
			// we suppose the key code is 0
			#if (PTS_TEST_EN || 1) // open mainly for the bluez test 
        	uint8_t auth_value[16] = {0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
									  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x02};
        	mesh_set_dev_auth(auth_value,sizeof(auth_value));
			#endif
		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_BEEP){
			
		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_NUMBERIC){
			
		}else if (p_prov_oob->oob_outAct == MESH_OUT_ACT_ALPHA){		
		}
		ret=1;
	}else if (p_start->authMeth == MESH_INPUT_OOB){
		const u8 const_mesh_auth_input_no_oob[16]={ 0,0,0,0, 0,0,0,0 ,0,0,0,0, 0,0,0,0};
		mesh_set_dev_auth((u8 *)const_mesh_auth_input_no_oob,sizeof(dev_auth));
		dev_auth[15]= prov_auth_val;
		if(prov_auth_en_flag ){
			prov_auth_en_flag =0;
		}
		ret =1;
	}
	return ret ;
}

#endif 

u8 change_capa_act_to_start_act(u16 capa_act,u8 out_flag)
{
	u8 act=0;
	if(out_flag){
		if(capa_act&BIT(MESH_OUT_ACT_BLINK)){
			act = MESH_OUT_ACT_BLINK;
		}else if (capa_act&BIT(MESH_OUT_ACT_BEEP)){
			act = MESH_OUT_ACT_BEEP;
		}else if (capa_act & BIT(MESH_OUT_ACT_VIBRATE)){
			act = MESH_OUT_ACT_VIBRATE;
		}else if (capa_act & BIT(MESH_OUT_ACT_NUMBERIC)){
			act = MESH_OUT_ACT_NUMBERIC;
		}else if (capa_act & BIT(MESH_OUT_ACT_ALPHA)){
			act = MESH_OUT_ACT_ALPHA;
		}else{
			act = MESH_OUT_ACT_RFU;
		}
	}else{
		if(capa_act&BIT(MESH_IN_ACT_PUSH)){
			act = MESH_IN_ACT_PUSH;
		}else if (capa_act &BIT(MESH_IN_ACT_TWIST )){
			act = MESH_IN_ACT_TWIST;
		}else if (capa_act &BIT(MESH_IN_ACT_NUMBER)){
			act = MESH_IN_ACT_NUMBER;
		}else if (capa_act &BIT(MESH_IN_ACT_ALPHA)){
			act = MESH_IN_ACT_ALPHA;
		}else {
			act = MESH_IN_ACT_RFU;
		}
	}
	return act ;
}


u8 get_pubkey_oob_info_by_capa(mesh_prov_oob_str *p_prov_oob)
{
	u8 pubkey =0;
	u8 oob_type =0;
	pro_trans_capa * p_capa = &(p_prov_oob->capa);
	pubkey = p_capa->pubKeyType;
	oob_type = p_capa->staticOOBType;
	if(oob_type){ // if the type is static oob type ,so we will use the oob type 
		p_prov_oob->prov_key = MESH_STATIC_OOB;
	}else{ // or it will be the input ouput or no 
		if(p_capa->outPutOOBSize>=1 && p_capa->outPutOOBSize <=8){
			p_prov_oob->oob_outsize = p_capa->outPutOOBSize;
			p_prov_oob->prov_key = MESH_OUTPUT_OOB;
			p_prov_oob->oob_outAct =change_capa_act_to_start_act(p_capa->outPutOOBAct,1);
		}else if (p_capa->inOOBSize>=1 && p_capa->inOOBSize <=8){
			p_prov_oob->oob_insize = p_capa->inOOBSize;
			p_prov_oob->prov_key = MESH_INPUT_OOB;
			p_prov_oob->oob_inAct =change_capa_act_to_start_act(p_capa->inOOBAct,0);
		}else if (p_capa->inOOBSize==0 && p_capa->outPutOOBSize==0){
			p_prov_oob->prov_key = MESH_NO_OOB;
		}
	}
	return 1;
}

u8 prov_cmd_is_valid(u8 op_code)
{
	if(op_code > PRO_FAIL){
		return 0;
	}else{
		return 1;
	}
}
u8 prov_fail_cmd_proc(mesh_pro_data_structer *p_notify,u8 err_code)
{
	set_pro_fail(p_notify,err_code);
	mesh_provision_para_reset();
	mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_FAIL_CODE);
	return sizeof(pro_trans_fail);
}

u8 prov_calc_cmd_len_chk(u8 op_code,u8 len)
{
	u8 prov_cmd_len =0;
	prov_cmd_len = get_mesh_pro_str_len(op_code);
	if(prov_cmd_len == len){
		return 1;
	}else{
		return 0;
	}
}

// PB-GATT data dispatch 
void dispatch_pb_gatt(u8 *p ,u8 len )
{
#if !__PROJECT_MESH_PRO__
	mesh_pro_data_structer *p_rcv_str = (mesh_pro_data_structer *)p;
	u8 gatt_send_buf[70];
	memset(gatt_send_buf,0,sizeof(gatt_send_buf));
	mesh_pro_data_structer *p_notify = (mesh_pro_data_structer *)gatt_send_buf;
	u16 notify_len=0;
	u8 rcv_prov_type;
	rcv_prov_type = p_rcv_str->invite.header.type;
	// dispatch the code is valid or not 
	if(!prov_cmd_is_valid(rcv_prov_type)){
	    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt prov cmd is invalid,%d",rcv_prov_type);
		notify_len = prov_fail_cmd_proc(p_notify,INVALID_PDU);
		gatt_prov_notify_pkts(gatt_send_buf,notify_len,PROVISION_ATT_HANDLE,MSG_PROVISION_PDU);
		return ;
	}
	if(!prov_calc_cmd_len_chk(rcv_prov_type,len)){
	    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt the length of the prov cmd is invalid,%d",rcv_prov_type);
		notify_len = prov_fail_cmd_proc(p_notify,INVALID_FORMAT);
		gatt_prov_notify_pkts(gatt_send_buf,notify_len,PROVISION_ATT_HANDLE,MSG_PROVISION_PDU);
		return ;
	}
	switch(prov_para.provison_rcv_state){
		case LINK_UNPROVISION_STATE:
			if(rcv_prov_type == PRO_INVITE){
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->invite), 
								sizeof(pro_trans_invite),"rcv provision invite ",0);
                mesh_provision_para_reset();
				if(is_provision_success()){
					// can not provision again ,and stop by the cmd 
					LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt the dev has already provisioned",0);
					return ;
				}
				dev_input[0]= p_rcv_str->invite.attentionDura;
				set_pro_capa_cpy(p_notify,&prov_oob);// set capability part 
				swap_mesh_pro_capa(p_notify);
				memcpy(dev_input+1,&(p_notify->capa.ele_num),11);
				notify_len = sizeof(pro_trans_capa);
				prov_para.provison_rcv_state =STATE_DEV_CAPA;
				blc_att_setServerDataPendingTime_upon_ClientCmd(1);
				LAYER_PARA_DEBUG(A_provision_invite);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_notify->capa), 
								sizeof(pro_trans_capa),"send capa cmd ",0);
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_invite));
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE,(u8 *)p_notify,notify_len);
				
			}else{
                LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err opcode in the LINK_UNPROVISION_STATE state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		case STATE_DEV_CAPA:
			if(rcv_prov_type == PRO_START){
				if(dispatch_start_cmd_reliable(p_rcv_str)){
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_START);
					LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->start), 
								sizeof(pro_trans_start),"rcv start cmd ",0);
					set_node_prov_start_oob(p_rcv_str,&prov_oob);//set the start cmd for the prov oob info 
					SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_start));
					memcpy(dev_input+12,&(p_rcv_str->start.alogrithms),5);
					prov_para.provison_rcv_state =STATE_PRO_START;
					LAYER_PARA_DEBUG(A_provision_start);
				}else{
					// send terminate ind
                    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt the start cmd is invalid",0);
					#if !WIN32 
					notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
					#endif 
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_FAIL_CODE);
				}
			}else{
			    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err opcode in the STATE_DEV_CAPA state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		case STATE_PRO_START:
			if(rcv_prov_type == PRO_PUB_KEY){
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->pubkey), 
								sizeof(pro_trans_pubkey),"rcv pubkey cmd ",0);
				if(!mesh_check_pubkey_valid(p_rcv_str->pubkey.pubKeyX)){
					notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
					break;
				}
				u8 dev_public_key[64];
				get_public_key(dev_public_key);
				memcpy(dev_input+0x11,p_rcv_str->pubkey.pubKeyX,0x40);
				memcpy(dev_input+0x11+0x40,dev_public_key,0x40);
				set_pro_pub_key(p_notify,dev_public_key,dev_public_key+32);
				notify_len = sizeof(pro_trans_pubkey);
				prov_para.provison_rcv_state =STATE_DEV_PUB_KEY;
				LAYER_PARA_DEBUG(A_provision_pubkey);
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_pubkey));
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE,(u8 *)p_notify,notify_len);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_notify->pubkey),
								sizeof(pro_trans_pubkey),"send pubkey cmd ",0);
			}else{
			    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err opcode in the STATE_PRO_START state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		case STATE_DEV_PUB_KEY:
			if(rcv_prov_type == PRO_CONFIRM){
				if(!mesh_node_oob_auth_data(&prov_oob)){// set the auth part ,only for the no oob ,and static oob
					// send terminate ind 
					LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt the start cmd can not suit to capa cmd",0);
					#if !WIN32 
					bls_ll_terminateConnection(0x13);
					#endif 
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_FAIL_CODE);
					return ;
				}
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->comfirm), 
								sizeof(pro_trans_comfirm),"rcv comfirm cmd ",0);
				pro_trans_comfirm *p_comfirm;
				u8 dev_private_key[32];
				p_comfirm = &(p_rcv_str->comfirm);
				memcpy(dev_pro_comfirm,p_comfirm->comfirm,16);
				get_private_key(dev_private_key);
				tn_p256_dhkey (dev_edch, dev_private_key, dev_input+0x11, dev_input+0x11+0x20);
				provision_random_data_init();
				
				#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
				u8 node_auth[16];
				caculate_sha256_node_auth_value(node_auth);
				mesh_sec_prov_confirmation (dev_comfirm, dev_input, 145, dev_edch, dev_random, node_auth);
				#else
				mesh_sec_prov_confirmation (dev_comfirm, dev_input, 145, dev_edch, dev_random, dev_auth);
				#endif
				
				set_pro_comfirm(p_notify,dev_comfirm);
				notify_len = sizeof(pro_trans_comfirm);
				prov_para.provison_rcv_state =STATE_DEV_CONFIRM;
				LAYER_PARA_DEBUG(A_provision_comfirm);
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_comfirm));
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE,(u8 *)p_notify,notify_len);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_notify->comfirm), 
								sizeof(pro_trans_comfirm),"send comfirm cmd ",0);
			}else{
			    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err cmd in the STATE_DEV_PUB_KEY state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		case STATE_DEV_CONFIRM:
			if(rcv_prov_type == PRO_RANDOM){
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->random), 
								sizeof(pro_trans_random),"rcv random cmd ",0);
				memcpy(pro_random,p_rcv_str->random.random,16);
				// use the provision random to caculate the provision comfirm 
				u8 pro_comfirm_tmp[16];
				mesh_sec_prov_confirmation (pro_comfirm_tmp, dev_input, 145, dev_edch, pro_random, dev_auth);
				if(memcmp(pro_comfirm_tmp,dev_pro_comfirm,16)){
				    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt the comfirm compare fail",0);
					notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
					break;
				}

				set_pro_random(p_notify,dev_random);		
				notify_len = sizeof(pro_trans_random);
				prov_para.provison_rcv_state =STATE_DEV_RANDOM;
				LAYER_PARA_DEBUG(A_provision_random);
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_random));
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE,(u8 *)p_notify,notify_len);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_notify->random),sizeof(pro_trans_random),
								"send random cmd ",0);
			}else{
			    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err cmd in the STATE_DEV_CONFIRM state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		case STATE_DEV_RANDOM:
			if(rcv_prov_type == PRO_DATA){
				// need to be checked 
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->data),
							sizeof(pro_trans_data),"rcv provision data cmd ",0);
				u8 *p_prov_net;
				p_prov_net = (u8 *)(p+1);
				u8 dev_key[16];
				u8 prov_salt[16];
				u8 dev_session_key[16];
				u8 dev_session_nonce[16];
				mesh_sec_prov_salt(prov_salt,dev_input,pro_random,dev_random);
				mesh_sec_prov_session_key (dev_session_key, dev_session_nonce, dev_input, 145, dev_edch, pro_random, dev_random);
				//calculate the dev_key part 
				mesh_sec_dev_key(dev_key,prov_salt,dev_edch);
				set_dev_key(dev_key);
				
				mesh_prov_sec_msg_dec (dev_session_key, dev_session_nonce+3, p_prov_net, 33, 8);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,p_prov_net,
							25,"provision net info is ",0);
				memcpy(&provision_mag.pro_net_info,p_prov_net,sizeof(provison_net_info_str));
				// add the info about the gatt mode provision ,should set the cfg data part into the node identity
				model_sig_cfg_s.node_identity_def = NODE_IDENTITY_SUBNET_SUPPORT_ENABLE;
				mesh_provision_par_handle((u8 *)&provision_mag.pro_net_info);
				#if !WIN32 
				mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_SUC);
				#endif 
				// send back the complete cmd 
				LAYER_PARA_DEBUG(A_provision_data);
				gatt_send_buf[0]= PRO_COMPLETE ;
				notify_len=1;
				cache_init(ADR_ALL_NODES);
				prov_para.provison_rcv_state = STATE_PRO_DATA;
				prov_para.provison_rcv_state = STATE_PRO_SUC;
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE|TSCRIPT_MESH_RX,(u8 *)p,sizeof(pro_trans_data));
				SET_TC_FIFO(TSCRIPT_PROVISION_SERVICE,(u8 *)p_notify,notify_len);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"provision suc! ",0);
			}else{
			    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"gatt rcv err cmd in the STATE_DEV_CONFIRM state",0);
				notify_len = prov_fail_cmd_proc(p_notify,UNEXPECTED_PDU);
			}
			break;
		default:
			return;
			break;
	}
	if(notify_len){
		gatt_prov_notify_pkts(gatt_send_buf,notify_len,PROVISION_ATT_HANDLE,MSG_PROVISION_PDU);
	}
#endif 
	return;
}

void mesh_adv_provision_retry()
{
	// if the retry time excced about 20s ,it will reset the states 
	if(prov_para.cmd_retry_flag &&clock_time_exceed(prov_para.cmd_send_start_tick,PROVISION_CMD_TIMEOUT_MS)){
		mesh_provision_para_reset();
		LOG_MSG_ERR(TL_LOG_NODE_SDK, 0, 0,"provision time_out",0);
		mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_TIMEOUT);
		prov_para.cmd_send_start_tick = clock_time();
	}
	if(	prov_para.cmd_retry_flag &&
		clock_time_exceed(prov_para.cmd_send_tick ,100*1000)){
		
		if(my_fifo_data_cnt_get(&mesh_adv_cmd_fifo)>2){//make sure enough buf
			return;
		}
		
		if(prov_para.pro_cmd == PRO_COMMAND_ACK){
			mesh_send_provison_data(TRANS_ACK,0,0,0);
			mesh_send_provison_data(TRANS_ACK,0,0,0);
		}else if(prov_para.pro_cmd == PRO_BEARS_CTL){
		    if(prov_para.pro_bearerCode == LINK_OPEN){
                mesh_send_provison_data(BEARS_CTL,prov_para.pro_bearerCode,prov_link_uuid,sizeof(prov_link_uuid));
		    }else{
                mesh_send_provison_data(BEARS_CTL,prov_para.pro_bearerCode,0,0);
		    }
		}
        else if(prov_para.pro_cmd == PRO_BEACON){
            unprov_beacon_send(MESH_UNPROVISION_BEACON_WITH_URI,0);
        }
        else{
			if(prov_para.ack_retry_flag){
				#if PTS_TEST_EN
				//mesh_send_provison_data(TRANS_ACK,0,0,0);
				#endif
				mesh_send_provison_data(TRANS_ACK,0,0,0);
			}
			send_multi_type_data(prov_para.pro_cmd,para_pro);
		}
		prov_para.cmd_send_tick =clock_time();
		if(prov_para.link_close_flag && prov_para.link_close_cnt){
			prov_para.link_close_cnt -- ;
			if(prov_para.link_close_cnt --){
				mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
			}else{
				prov_para.link_close_flag =0;
				prov_para.link_close_cnt =0;
			}
		}
	}
}

void set_rsp_ack_transnum(pro_PB_ADV *p_adv){
	if(	(p_adv->transBear.bearOpen.header.GPCF == BEARS_CTL && p_adv->transBear.bearAck.header.BearCtl == LINK_ACK	)){
		prov_para.rsp_ack_transnum = 0;
	}else if(p_adv->transBear.bearAck.header.GPCF!= TRANS_ACK){
		prov_para.rsp_ack_transnum = p_adv->trans_num;
	}
	else{
	}
}

void provision_set_ivi_para(u8 *para)
{
	provison_net_info_str *p_prov_net_info = (provison_net_info_str *)para;
	u8 iv_0[4] = {0};
	if(p_prov_net_info->prov_flags.IVUpdate
	&& memcmp(p_prov_net_info->iv_index, iv_0, sizeof(iv_0))){
		iv_update_set_with_update_flag_ture(p_prov_net_info->iv_index, 0);
	}else{
        int rst_sno = memcmp(p_prov_net_info->iv_index, iv_idx_st.cur, 4);
        mesh_iv_idx_init(p_prov_net_info->iv_index, rst_sno);
	}
}

u8 proc_start_cmd_reliable(pro_trans_start *p_start)
{
  if(p_start->alogrithms!=0){
		return 0;
	}else if(p_start->pubKey >= 2){
		return 0;
	}else if(p_start->authMeth >= 4){
		return 0;
	}else if(p_start->authMeth == AUTH_MESH_NO_OOB){
		if(p_start->authAct!=0 || p_start->authSize !=0){
			return 0;
		}
	}else if (p_start->authMeth == AUTH_MESH_STATIC_OOB){
		if(p_start->authAct!=0 || p_start->authSize !=0){
			return 0;
		}
	}else if (p_start->authMeth == AUTH_MESH_OUTPUT_OOB){
		if(p_start->authAct>4 || (p_start->authSize == 0 || p_start->authSize>8)){
			return 0;
		}
	}else if (p_start->authMeth == AUTH_MESH_INPUT_OOB){
		if(p_start->authAct>4 || (p_start->authSize == 0 || p_start->authSize>8)){
			return 0;
		}
	}else{
	}  
	return 1;

}

#define ENABLE_NO_OOB_IN_STATIC_OOB     0

u8 compare_capa_with_start(mesh_prov_oob_str *p_oob)
{
#if ENABLE_NO_OOB_IN_STATIC_OOB
    return 1;
#else
    pro_trans_capa *p_capa = &(p_oob->capa);
    pro_trans_start *p_start = &(p_oob->start);
    //compare the pubkey oob part 
    if(p_capa->pubKeyType != p_start->pubKey){
        return 0;
    }
    //compare the sec method 
    if(p_oob->prov_key != p_start->authMeth){
        return 0;
    }
    if( p_start->authMeth == MESH_NO_OOB ||
        p_start->authMeth == MESH_STATIC_OOB){
        if(p_start->authAct!=0 || p_start->authSize!=0){
           return 0;
        }
    }else if (p_start->authMeth == MESH_OUTPUT_OOB){
        if( p_oob->oob_outsize != p_start->authSize ||
            p_oob->oob_outAct  != p_start->authAct){
            return 0;
        }
    }else if (p_start->authMeth == MESH_INPUT_OOB){
        if( p_oob->oob_insize != p_start->authSize ||
            p_oob->oob_inAct  != p_start->authAct){
            return 0;
        }
    }else{}
    return 1;
#endif
}

u8 dispatch_start_cmd_reliable(mesh_pro_data_structer *p_rcv_str)
{
	pro_trans_start *p_start = &(p_rcv_str->start);
	if(!proc_start_cmd_reliable(p_start)){
        return 0;
	}
	get_pubkey_oob_info_by_capa(&prov_oob);
	set_node_prov_start_oob(p_rcv_str,&prov_oob);//set the start cmd for the prov oob info 
	if(!compare_capa_with_start(&prov_oob)){
        return 0;
	}
	return 1;
}

int mesh_prov_sec_msg_enc(unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length)
{
	provison_net_info_str *p_prov = (provison_net_info_str *)(dat);
	endianness_swap_u16((u8 *)(&p_prov->key_index));
	endianness_swap_u16((u8 *)(&p_prov->unicast_address));
	return mesh_sec_msg_enc(key,nonce,dat,n,mic_length);
}

int	mesh_prov_sec_msg_dec (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length)
{
	int err=-1;
	err = mesh_sec_msg_dec (key, nonce, dat, n, mic_length);
	provison_net_info_str *p_prov = (provison_net_info_str *)(dat);
	endianness_swap_u16((u8 *)(&p_prov->key_index));
	endianness_swap_u16((u8 *)(&p_prov->unicast_address));
	return err;
}
static u32 tick_check_complete=0;
// the ack should be trigged by the send cmd ,so we just need to dispatch in the receive state

void set_node_prov_capa_oob(mesh_prov_oob_str *p_prov_oob,u8 pub_key_type,
					u8 staticOOBType,u16 outPutOOBAct,u16 inPutOOBAct)
{
	pro_trans_capa *p_capa = &(p_prov_oob->capa);
	memset((u8 *)p_capa,0,sizeof(pro_trans_capa));
	p_capa->ele_num =g_ele_cnt;
	p_capa->alogrithms = FIPS_P256_Ellipti_CURVE;
	p_capa->pubKeyType = pub_key_type;
	p_capa->staticOOBType = staticOOBType;
	if(p_capa->staticOOBType == MESH_STATIC_OOB){
		p_capa->outPutOOBSize = 0;
		p_capa->outPutOOBAct = 0;
		p_capa->inOOBSize = 0;
		p_capa->inOOBAct = 0;
	}else{
		// no oob ,or the input oob and output oob part 
		if(outPutOOBAct!=0){ // output oob 
			p_capa->outPutOOBAct = BIT(MESH_OUT_ACT_BLINK);
			p_capa->outPutOOBSize = 1;
			p_capa->inOOBSize = 0;
			p_capa->inOOBAct = 0;
		}else if (inPutOOBAct!=0){ // input oob 
			p_capa->outPutOOBSize = 0;
			p_capa->outPutOOBAct = 0;
			p_capa->inOOBAct = BIT(MESH_IN_ACT_PUSH);
			p_capa->inOOBSize = 1;
		}else{// no oob 
			p_capa->outPutOOBSize = 0;
			p_capa->outPutOOBAct = 0;
			p_capa->inOOBSize = 0;
			p_capa->inOOBAct = 0;
		}
	}
}

void set_node_prov_capa_oob_init()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITHOUT_OOB,0,0,0);
}

void set_node_prov_para_no_pubkey_no_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITHOUT_OOB,0,0,0);
}

void set_node_prov_para_no_pubkey_static_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITHOUT_OOB,MESH_STATIC_OOB,0,0);
}

void set_node_prov_para_no_pubkey_input_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITHOUT_OOB,0,0,1);
}

void set_node_prov_para_no_pubkey_output_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITHOUT_OOB,0,1,0);
}

void set_node_prov_para_pubkey_no_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITH_OOB,0,0,0);
}

void set_node_prov_para_pubkey_static_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITH_OOB,MESH_STATIC_OOB,0,0);
}

void set_node_prov_para_pubkey_input_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITH_OOB,0,0,1);
}

void set_node_prov_para_pubkey_output_oob()
{
	set_node_prov_capa_oob(&prov_oob,MESH_PUB_KEY_WITH_OOB,0,1,0);
}


void set_node_prov_start_oob(mesh_pro_data_structer *p_rcv,mesh_prov_oob_str *p_oob)
{
	pro_trans_start *p_start = &(p_rcv->start);
	memcpy((u8 *)(&p_oob->start),(u8 *)p_start,sizeof(pro_trans_start));
}
u32 mesh_node_out_oob_pub_key_tick =0;
#define MESH_NODE_OUT_OOB_PUB_KEY_TIME_S	6*1000*1000
void check_mesh_node_out_oob_pub_key_send_time()
{
	if(clock_time_exceed(mesh_node_out_oob_pub_key_tick,MESH_NODE_OUT_OOB_PUB_KEY_TIME_S)&&mesh_node_out_oob_pub_key_tick){
		// stop send the pubkey 
		mesh_node_out_oob_pub_key_tick =0;
		send_rcv_retry_clr();
	}
}
u32 link_close_end_tick =0;
void mesh_prov_link_close_terminate()
{
	if(link_close_end_tick && clock_time_exceed(link_close_end_tick,2*1000*1000)){
		send_rcv_retry_clr();
		mesh_provision_para_reset();
		link_close_end_tick =0;
	}
}
void mesh_terminate_provision_link_reset(u8 code)
{	
	send_rcv_retry_clr();
	prov_set_link_close_code(code);
	mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
	send_rcv_retry_set(PRO_BEARS_CTL,LINK_CLOSE, 0); 
	LOG_MSG_LIB(TL_LOG_NODE_SDK, 0, 0,"send link close",0);
	link_close_end_tick = clock_time()|1;
}
int pub_swap_endian(const uint8_t *in, uint8_t *out, uint32_t size)
{
    for(u32 i = 0; i < size; i++)
        out[i] = in[size-1-i];

    return 0;
}

u32 mesh_check_pubkey_valid(u8 *rcv_ppk)
{
#if WIN32
	return 1;
#else
    const struct uECC_Curve_t * p_curve;
    p_curve = uECC_secp256r1();
	u8 ecc_ppk[64];
	pub_swap_endian(&rcv_ppk[0],&ecc_ppk[0],32);
    pub_swap_endian(&rcv_ppk[32],&ecc_ppk[32],32);
    return uECC_valid_public_key((const uint8_t *)ecc_ppk, p_curve);
#endif
}
void check_inputoob_proc()
{
#if !__PROJECT_MESH_PRO__
	mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
	if(prov_para.provison_rcv_state ==  STATE_DEV_PUB_KEY_INPUT_OOB){
		// need to send the cmd of the input complete			
		if(prov_auth_en_flag ){
			prov_auth_en_flag =0;
			send_rcv_retry_clr();
			set_pro_input_complete(p_send_str);
			send_multi_type_data(PRO_INPUT_COM,para_pro);
			send_rcv_retry_set(PRO_INPUT_COM,0,0);
			prov_para.trans_num_last = prov_para.trans_num;
			prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_ACK;
		}
	}
#endif
}
void mesh_node_rc_data_dispatch(pro_PB_ADV *p_adv){
	// unprovision device  
	#if !__PROJECT_MESH_PRO__	
	mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
	mesh_pro_data_structer *p_rcv_str = (mesh_pro_data_structer *)(p_adv->transStart.data);
	#if MD_REMOTE_PROV
    if(mesh_pr_sts_work_or_not()){
	    mesh_prov_server_rcv_cmd(p_adv);
        return ;
	}
	#endif
	// if the rcv pkt is a adv pkt ,we should remeber the transaction num ,then rsp with the same transaction num 
	if((!prov_para.link_id_filter) && (!(p_adv->transBear.bearOpen.header.GPCF == BEARS_CTL &&
		p_adv->transBear.bearAck.header.BearCtl == LINK_OPEN))){
		return;
	}
	if(p_adv->transBear.bearOpen.header.GPCF == BEARS_CTL &&
			   p_adv->transBear.bearAck.header.BearCtl == LINK_CLOSE){
		//when receive a link close cmd ,it will reset the state 
		SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(&(p_adv->length)),p_adv->length+1);            
		//reset the link id and others 
		LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"rcv link close cmd ",0);
		mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_CLOSE);
		if(p_adv->transBear.bearCls.reason != PRO_LINK_CLOSE_SUC){
			mesh_provision_para_reset();
		}
	}

	if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
		LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 
						0,"rcv transation ack(transation:0x%02x)",p_adv->trans_num);
		send_rcv_retry_clr();//stop send cmd when receive ack
		prov_para.trans_num_last = p_adv->trans_num + 2; // receive trans ack once
		if(prov_para.provison_rcv_state == STATE_PRO_COMPLETE){			
			adv_provision_state_dispatch(p_adv);
			mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_SUC);
		}
		else if(prov_para.provison_rcv_state == STATE_PRO_FAILED_ACK){
			mesh_terminate_provision_link_reset(LINK_CLOSE_FAIL);
		}
		return;
	}
	
	switch(prov_para.provison_rcv_state){
		case LINK_UNPROVISION_STATE:
			if(p_adv->transBear.bearOpen.header.GPCF == BEARS_CTL &&
			   p_adv->transBear.bearAck.header.BearCtl == LINK_OPEN){	
			    if(	prov_para.provison_rcv_state == LINK_UNPROVISION_STATE&&
					!memcmp(prov_para.device_uuid, p_adv->transBear.bearOpen.DeviceUuid,sizeof(prov_para.device_uuid))){
					if(is_provision_success()){
						// can not provision again ,and stop by the cmd
						LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"adv the dev has already provisioned",0);
						return ;
					}
					LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)(p_adv), 
								p_adv->length+1,"rcv link open cmd ",0);
					SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
					if(user_node_rc_link_open_callback() == 0){
						return;
					}
				   	memcpy((u8 *)(&prov_para.link_id),p_adv->link_ID,4); 
					set_rsp_ack_transnum(p_adv);
                    mesh_send_provison_data(BEARS_CTL,LINK_ACK,0,0);
					send_rcv_retry_set(PRO_BEARS_CTL,LINK_ACK, 0); 
					SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);					
					prov_para.provison_rcv_state = LINK_ESTABLISHED;
					prov_para.link_id_filter =1;
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_START);
			    }
			}
			break;
		case LINK_ESTABLISHED:           
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_INVITE){
				// add the rcv invite part 
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->invite), 
								sizeof(pro_trans_invite),"rcv invite cmd ",0);
				dev_input[0] = p_rcv_str->invite.attentionDura;
				SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
				set_rsp_ack_transnum(p_adv);
				prov_para.attention_value= p_rcv_str->invite.attentionDura;
				send_rcv_retry_clr();
				mesh_send_provison_data(TRANS_ACK,0,0,0);
				SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
				// need to set the add the prov oob init part 
				set_pro_capa_cpy(p_send_str,&prov_oob);
				// swap the send buf part 
				swap_mesh_pro_capa(p_send_str);
				// use the send message as the input msg 
				memcpy(dev_input+1,&(p_send_str->capa.ele_num),11);
				send_multi_type_data(PRO_CAPABLI,para_pro);
				SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
				send_rcv_retry_set(PRO_CAPABLI,0,1);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_send_str->capa), 
								sizeof(pro_trans_capa),"send capa cmd ",0);
				prov_para.provison_rcv_state = STATE_DEV_CAPA;
				prov_para.trans_num_last = prov_para.trans_num;
			}
			break;
		case STATE_DEV_CAPA:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_START){
					if(dispatch_start_cmd_reliable(p_rcv_str)){
						// get the provision start part 
						LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->start), 
								sizeof(pro_trans_start),"rcv start cmd ",0);
						set_node_prov_start_oob(p_rcv_str,&prov_oob);
						memcpy(dev_input+12,&(p_rcv_str->start.alogrithms),5);
						SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
						set_rsp_ack_transnum(p_adv);
						send_rcv_retry_clr();
						mesh_send_provison_data(TRANS_ACK,0,0,0); 
						SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
						send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
						prov_para.trans_num_last += 2; // trans_num_last must change when run into new state
						prov_para.provison_rcv_state = STATE_PRO_START_ACK;
						
					}else{
						LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"adv the start cmd is unreliable",0);
						set_rsp_ack_transnum(p_adv);
						send_rcv_retry_clr();
						mesh_send_provison_data(TRANS_ACK,0,0,0);
						set_pro_fail(p_send_str,PROVISION_FAIL_INVALID_FORMAT);
						send_multi_type_data(PRO_FAIL,para_pro);
						send_rcv_retry_set(PRO_FAIL,0,1);
						//prov_para.link_close_flag = 1;
						//prov_para.link_close_cnt = 0x20;
						prov_para.trans_num_last += 2;;
						prov_para.provison_rcv_state = STATE_PRO_FAILED_ACK;
						return ;
					}
				}
			break;			
		case STATE_PRO_START_ACK:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_PUB_KEY){
				if(!mesh_check_pubkey_valid(p_rcv_str->pubkey.pubKeyX)){
				    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"adv the pubkey value is unvalid",0);
					set_rsp_ack_transnum(p_adv);
					send_rcv_retry_clr();
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					set_pro_fail(p_send_str,PROVISION_FAIL_UNEXPT_PDU);
					send_multi_type_data(PRO_FAIL,para_pro);
					send_rcv_retry_set(PRO_FAIL,0,1);
					prov_para.trans_num_last = prov_para.trans_num;
					prov_para.provison_rcv_state = STATE_PRO_FAILED_ACK;
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_FAIL_CODE);
					return;
				}
				SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(&rcv_pb),rcv_pb.transStart.total_len+11);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->pubkey), 
								sizeof(pro_trans_pubkey),"rcv pubkey cmd ",0);
				u8 dev_public_key[64];
				get_public_key(dev_public_key);
				set_rsp_ack_transnum(p_adv);
				send_rcv_retry_clr();
				memcpy(dev_input+0x11,p_rcv_str->pubkey.pubKeyX,0x40);
				memcpy(dev_input+0x11+0x40,dev_public_key,0x40);
				if(prov_oob.start.pubKey == MESH_PUB_KEY_WITHOUT_OOB){
					set_pro_pub_key(p_send_str,dev_public_key,dev_public_key+32);
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
					send_multi_type_data(PRO_PUB_KEY,para_pro);
					#if TESTCASE_FLAG_ENABLE
					u8 tmp_data_len = get_mesh_pro_str_len(PRO_PUB_KEY);
					memcpy(tc_seg_buf, (u8 *)(&(pro_adv_pkt.len)),OFFSETOF(pro_PB_ADV,transStart.data));
					memcpy(tc_seg_buf+OFFSETOF(pro_PB_ADV,transStart.data), para_pro, tmp_data_len);
					SET_TC_FIFO(TSCRIPT_MESH_TX, tc_seg_buf,tmp_data_len+OFFSETOF(pro_PB_ADV,transStart.data));
					#endif
					send_rcv_retry_set(PRO_PUB_KEY,0,1);
					if(prov_oob.start.authMeth == MESH_OUTPUT_OOB){
						mesh_node_out_oob_pub_key_tick = clock_time()|1;
					}
					LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_send_str->pubkey), 
								sizeof(pro_trans_pubkey),"send pubkey cmd ",0);
					if(prov_oob.start.authMeth == MESH_NO_OOB || prov_oob.start.authMeth == MESH_STATIC_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_ACK;     
					}else if (prov_oob.start.authMeth == MESH_OUTPUT_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_ACK;
					}else if (prov_oob.start.authMeth == MESH_INPUT_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_INPUT_OOB;
					}        
					prov_para.trans_num_last = prov_para.trans_num;
				}else if (prov_oob.start.pubKey == MESH_PUB_KEY_WITH_OOB){
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
					prov_para.trans_num_last += 2;
					if(prov_oob.start.authMeth == MESH_NO_OOB || prov_oob.start.authMeth == MESH_STATIC_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_ACK;  
					}else if (prov_oob.start.authMeth == MESH_OUTPUT_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_ACK;
					}else if (prov_oob.start.authMeth == MESH_INPUT_OOB){
						prov_para.provison_rcv_state = STATE_DEV_PUB_KEY_INPUT_OOB;
					}									
				}
			}
			else if(p_adv->transAck.GPCF == TRANS_ACK || p_adv->transStart.data[0]== PRO_START){
			}else{
			}
			break;
		case STATE_DEV_PUB_KEY_ACK:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_CONFIRM){
					LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->comfirm), 
								sizeof(pro_trans_comfirm),"rcv comfirm cmd ",0);
					SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
					set_rsp_ack_transnum(p_adv);
					send_rcv_retry_clr();
					// wait for the cmd of the comfirm part 
					if(!mesh_node_oob_auth_data(&prov_oob)){
					    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"the prov_oob is invalid",0);
						return ;
					}
					pro_trans_comfirm *p_comfirm;
					u8 dev_private_key[32];
					p_comfirm = &(p_rcv_str->comfirm);
					memcpy(dev_pro_comfirm,p_comfirm->comfirm,16);
					get_private_key(dev_private_key);
					tn_p256_dhkey (dev_edch, dev_private_key, dev_input+0x11, dev_input+0x11+0x20);
					provision_random_data_init();
					
					#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
					u8 node_auth[16];
					caculate_sha256_node_auth_value(node_auth);
					mesh_sec_prov_confirmation (dev_comfirm, dev_input, 145, dev_edch, dev_random, node_auth);
					#else
					mesh_sec_prov_confirmation (dev_comfirm, dev_input, 145, dev_edch, dev_random, dev_auth);
					#endif
					
					set_pro_comfirm(p_send_str,dev_comfirm);
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
					send_multi_type_data(PRO_CONFIRM,para_pro);
					SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);					
					send_rcv_retry_set(PRO_CONFIRM,0,1);
					LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_send_str->comfirm), 
								sizeof(pro_trans_comfirm),"send comfirm cmd ",0);
					prov_para.provison_rcv_state = STATE_DEV_CONFIRM;
					prov_para.trans_num_last = prov_para.trans_num;
					
			}	
			break;
		case STATE_DEV_CONFIRM:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_RANDOM){
				SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->random), 
								sizeof(pro_trans_random),"rcv random cmd ",0);
				set_rsp_ack_transnum(p_adv);
				send_rcv_retry_clr();
				memcpy(pro_random,p_rcv_str->random.random,16);
				
				// use the provision random to caculate the provision comfirm 
				u8 pro_comfirm_tmp[16];
				mesh_sec_prov_confirmation (pro_comfirm_tmp, dev_input, 145, dev_edch, pro_random, dev_auth);
				if(memcmp(pro_comfirm_tmp,dev_pro_comfirm,16)){
				    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"provision confirm fail",0);
					set_rsp_ack_transnum(p_adv);
					send_rcv_retry_clr();
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					set_pro_fail(p_send_str,PROVISION_FAIL_COMFIRM_FAIL);
					send_multi_type_data(PRO_FAIL,para_pro);
					send_rcv_retry_set(PRO_FAIL,0,1);
					prov_para.trans_num_last = prov_para.trans_num;
					prov_para.provison_rcv_state = STATE_PRO_FAILED_ACK;
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_CONFIRM_FAILED);
					return;
				}
				
				set_pro_random(p_send_str,dev_random);	
				mesh_send_provison_data(TRANS_ACK,0,0,0);
				SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
				send_multi_type_data(PRO_RANDOM,para_pro);
				SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
				send_rcv_retry_set(PRO_RANDOM,0,1);			
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_send_str->random), 
								sizeof(pro_trans_random),"send random cmd ",0);
				prov_para.provison_rcv_state = STATE_DEV_RANDOM;
				prov_para.trans_num_last = prov_para.trans_num;
			}
			break;
		case STATE_DEV_RANDOM:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_DATA){
				LOG_MSG_LIB(TL_LOG_NODE_SDK,(u8 *)&(p_rcv_str->data), 
								sizeof(pro_trans_data),"rcv provision data cmd ",0);
				u8 *p_prov_net;
				u8 dev_key[16];
				u8 prov_salt[16];
				u8 dev_session_key[16];
				u8 dev_session_nonce[16];
				mesh_sec_prov_salt(prov_salt,dev_input,pro_random,dev_random);
				mesh_sec_prov_session_key (dev_session_key, dev_session_nonce, dev_input, 145, dev_edch, pro_random, dev_random);
				//calculate the dev_key part 
				mesh_sec_dev_key(dev_key,prov_salt,dev_edch);
				set_dev_key(dev_key);
				LOG_MSG_LIB(TL_LOG_NODE_SDK,dev_key, 16,"device key ",0);
				p_prov_net = (p_adv->transStart.data)+1;				
				set_rsp_ack_transnum(p_adv);
				send_rcv_retry_clr();
				// trans data 
				mesh_prov_sec_msg_dec (dev_session_key, dev_session_nonce+3, p_prov_net, 33, 8);
				SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(&rcv_pb),rcv_pb.transStart.total_len+11);
				// need to be checked 
				
				LOG_MSG_LIB(TL_LOG_NODE_SDK,p_prov_net, 
								25,"the provision net info is  ",0);
				memcpy(&provision_mag.pro_net_info,p_prov_net,sizeof(provison_net_info_str));
				mesh_send_provison_data(TRANS_ACK,0,0,0);
				send_multi_type_data(PRO_COMPLETE,para_pro);
				SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
				send_rcv_retry_set(PRO_COMPLETE,0,1);
				prov_para.trans_num_last = prov_para.trans_num;
				prov_para.provison_rcv_state = STATE_PRO_COMPLETE;
				tick_check_complete = clock_time()|1;
				cache_init(ADR_ALL_NODES);
			}
			break;
		default:             
			break;
	}
	#endif 
	return ;
}

u8 adv_provision_state_dispatch(pro_PB_ADV * p_adv)
{
	prov_para.provison_rcv_state = STATE_PRO_SUC;
	mesh_provision_par_handle((u8 *)&provision_mag.pro_net_info);
	SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	send_rcv_retry_clr();
	SET_RESULT_TESTCASE(0,0);
	return 1;
}

u8 wait_and_check_complete_state()
{
	if( provision_mag.gatt_mode == GATT_PROXY_MODE ){
		tick_check_complete =0;
	}
	if(tick_check_complete && clock_time_exceed(tick_check_complete,2000*1000)){
		tick_check_complete =0;
		mesh_provision_par_handle((u8 *)&provision_mag.pro_net_info);
		prov_para.provison_rcv_state = STATE_PRO_SUC;
		send_rcv_retry_clr();
		LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 
								0,"adv provision part suc! ",0);
		mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_SUC);
	}
	return 1;
}

u8 get_provision_state()
{
	#if TESTCASE_FLAG_ENABLE
	if((CFGCL_CFG_KR_PTS_1 & 0xFFFF0000) == (tc_par.tc_id & 0xFFFF0000)){
		return 1;	// comfirm later
	}
	#endif
	
	if(provision_mag.gatt_mode == GATT_PROXY_MODE){
		return STATE_DEV_PROVED;
	}else{
	    if(prov_para.provison_send_state || prov_para.provison_rcv_state){
    		if( prov_para.provison_send_state == STATE_PRO_SUC  || 
    		    prov_para.provison_rcv_state == STATE_PRO_SUC){
    			return STATE_DEV_PROVED;
    		}else{
    			return STATE_DEV_PROVING;
    		}
	    }else{
            return STATE_DEV_UNPROV;
	    }
	}
}

u8 is_provision_success()   // rename is_proved_state(), proved means "prov + ed"
{
    #if WIN32
    return win32_proved_state();
    #else
    return (get_provision_state() == STATE_DEV_PROVED);
    #endif
}

u8 is_provision_working()
{
    #if WIN32
    return win32_prov_working();
    #else
	return(get_provision_state() == STATE_DEV_PROVING);
	#endif
}
static u8 gateway_connect_filter[6];
void set_gateway_adv_filter(u8 *p_mac)
{
	memcpy(gateway_connect_filter,p_mac,sizeof(gateway_connect_filter));
}
void gateway_adv_filter_init()
{
	memset(gateway_connect_filter,0,sizeof(gateway_connect_filter));
}
void mesh_pro_rc_beacon_dispatch(pro_PB_ADV *p_adv,u8 *p_mac){
	//provision_mag.provison_send_state = LINK_ESTABLISHED;
#if BLE_SIG_MESH_CERTIFY_ENABLE
	if(	prov_para.provison_send_state != STATE_PRO_SUC &&
		prov_para.provison_send_state != LINK_UNPROVISION_STATE){
		return ;
	}
#else
	if(	(prov_para.provison_send_state != STATE_PRO_SUC &&
		prov_para.provison_send_state != LINK_UNPROVISION_STATE)||
		prov_para.key_bind_lock){
		return ;
	}
#endif 
	beacon_data_pk *p_beacon = (beacon_data_pk *)p_adv;
	if(p_beacon->beacon_type == SECURE_BEACON){
		return ;
	}
	
#if MD_REMOTE_PROV
    // remote prov raw pkt  
    remote_prov_report_raw_pkt_cb((u8 *)p_adv);
#endif

	#if !WIN32&&(__PROJECT_MESH_PRO__||__PROJECT_MESH_GW_NODE__)
	//SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)&(p_adv->length),sizeof(beacon_data_pk));

	
	
	// if the stop flag ==1 ,can not send link open cmd part 
	if(provision_mag.pro_stop_flag){
		return ;
	}
	// upload the adv part and the device uuid part 
	gateway_upload_mac_address(p_mac,(u8 *)p_adv);
	// dispatch the filter part ,if the filter is the same ,the gateway can send the link open cmd 
	if(memcmp(gateway_connect_filter,p_mac,sizeof(gateway_connect_filter))){
		return ;
	}
	// whether the provisioner interal para set or not 
	if(!is_provision_success()){
		return ;
	}
	// whether the provisionee para set or not 
	if(!get_gateway_provisison_sts()){
		return ;
	}
	mesh_provision_para_reset();
	memcpy(prov_link_uuid,p_beacon->device_uuid,sizeof(p_beacon->device_uuid));
    send_rcv_retry_set(PRO_BEARS_CTL,LINK_OPEN,0); //test case use start_tick in mesh_send_provison_data
    prov_para.link_id = clock_time();
	mesh_send_provison_data(BEARS_CTL,LINK_OPEN,prov_link_uuid,sizeof(prov_link_uuid));
	#if GATEWAY_ENABLE
	gateway_upload_prov_link_open(prov_link_uuid,sizeof(prov_link_uuid));
	#endif
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, p_mac, 6,"send link open,mac:",0);
	prov_para.provison_send_state = LINK_ESTABLISHED_ACK;	
	#endif
	return;
}
// use to set the auth enable part 

void check_oob_out_timeout()
{
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
	if((prov_oob.oob_out_tick && clock_time_exceed(prov_oob.oob_out_tick ,12*1000*1000))&&prov_auth_en_flag){
		prov_oob.oob_out_tick =0;
		prov_auth_en_flag=0;
		send_rcv_retry_clr();
		mesh_prov_oob_auth_data(&prov_oob);
		send_comfirm_no_pubkey_cmd();
	}
	return ;
#endif
	
}


u32 mesh_provision_end_tick =0;
void mesh_prov_end_set_tick()
{
    mesh_provision_end_tick = clock_time()|1;
}

u8  mesh_loop_provision_end_process()
{
	u32 prov_retry_time = 2*1000*1000;
	
	#if MD_REMOTE_PROV
	if(mesh_pr_sts_work_or_not()){
		prov_retry_time = 4000*1000;
	}
	#endif
	
	if(mesh_provision_end_tick && clock_time_exceed(mesh_provision_end_tick,prov_retry_time)){
	    #if MD_REMOTE_PROV
        mesh_rp_server_prov_end_cb();
        #endif
	    send_rcv_retry_clr();
		mesh_provision_end_tick = 0;
		prov_para.provison_send_state = STATE_PRO_SUC;
		#if MD_REMOTE_PROV
		if(!mesh_pr_sts_work_or_not())
		#endif
		{
			u16 adr_store ;
			adr_store = provision_mag.unicast_adr_last;
			provision_mag.unicast_adr_last += prov_para.ele_cnt;
			prov_para.link_id_filter =0;
			provision_mag_cfg_s_store();
			mesh_misc_store();
			#if GATEWAY_ENABLE
			gateway_upload_node_info(adr_store);
			// the gateway need the device's mac address and the uuid part 
	        gateway_upload_provision_suc_event(1,adr_store,
	                        gateway_connect_filter,prov_link_uuid);// update the suc event
			set_gateway_provision_para_init();// init the para part 
			#endif
		}
	}
	return 1;
}

u32 link_close_start_tick =0;
void mesh_loop_check_link_close_flag()
{
	if(link_close_start_tick&& clock_time_exceed(link_close_start_tick,6*1000*1000)){
		{
			mesh_terminate_provision_link_reset(LINK_CLOSE_FAIL);
			link_close_start_tick = 0;
		}
	}
}


void mesh_adv_prov_link_open_ack(pro_PB_ADV *p_adv)
{
    memset(&prov_oob,0,sizeof(prov_oob));
	SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv link open ack",0);
	set_rsp_ack_transnum(p_adv);    
}

void mesh_adv_prov_send_invite(mesh_pro_data_structer *p_send_str)
{
    set_pro_invite(p_send_str,0);
	send_multi_type_data(PRO_INVITE,para_pro);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"send invite",0);
	send_rcv_retry_set(PRO_INVITE,0,0);
	prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_invite_ack(pro_PB_ADV *p_adv)
{
    SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)p_adv,p_adv->length+1); 
	send_rcv_retry_clr();
}

void mesh_adv_prov_capa_rcv(pro_PB_ADV *p_adv)
{
    SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)p_adv,p_adv->length+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv capabilities",0);
	set_rsp_ack_transnum(p_adv);
	send_rcv_retry_clr();
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);    
}

void mesh_adv_prov_send_start_cmd(mesh_pro_data_structer *p_send_str,pro_trans_start *p_start)
{
    set_pro_start_simple(p_send_str,p_start);
	send_multi_type_data(PRO_START,para_pro);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"send prov start",0);
	send_rcv_retry_set(PRO_START,0,1);
	prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_start_ack(pro_PB_ADV *p_adv)
{
	SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	send_rcv_retry_clr();    
}

void mesh_adv_prov_pubkey_send(mesh_pro_data_structer *p_send_str,u8 *ppk)
{
	set_pro_pub_key(p_send_str,ppk,ppk+32);
	send_multi_type_data(PRO_PUB_KEY,para_pro);
	send_rcv_retry_set(PRO_PUB_KEY,0,0); 
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"send pub_key:",0);
	prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_pubkey_rsp(pro_PB_ADV *p_adv)
{
    send_rcv_retry_clr();
    set_rsp_ack_transnum(p_adv);
}

void mesh_adv_prov_comfirm_cmd(mesh_pro_data_structer *p_send_str,u8 *p_comfirm)
{
    set_pro_comfirm(p_send_str,p_comfirm);
    mesh_send_provison_data(TRANS_ACK,0,0,0);
    send_multi_type_data(PRO_CONFIRM,para_pro);
    SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);  
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"send prov confirm",0);
    send_rcv_retry_set(PRO_CONFIRM,0,1);
    prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_comfirm_ack(pro_PB_ADV *p_adv)
{
    SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	send_rcv_retry_clr();
}

void mesh_adv_prov_comfirm_rsp(pro_PB_ADV *p_adv)
{
	SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)p_adv,p_adv->length+1);
	set_rsp_ack_transnum(p_adv);
	send_rcv_retry_clr();
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);    
}

void mesh_adv_prov_random_cmd(mesh_pro_data_structer *p_send_str,u8 *p_random)
{
	set_pro_random(p_send_str,p_random);
	send_multi_type_data(PRO_RANDOM,para_pro);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
	LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"send prov random:",0);
	send_rcv_retry_set(PRO_RANDOM,0,1);
	prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_random_ack_cmd(pro_PB_ADV *p_adv)
{
	SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	send_rcv_retry_clr();    
}

void mesh_adv_prov_random_rsp(pro_PB_ADV *p_adv)
{
	set_rsp_ack_transnum(p_adv);
	send_rcv_retry_clr();

}

void mesh_adv_prov_data_send(mesh_pro_data_structer *p_send_str,u8 *p_data)
{
    set_pro_data(p_send_str,p_data,p_data+25);
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	SET_TC_FIFO(TSCRIPT_MESH_TX,(u8 *)(&(pro_adv_pkt.len)),pro_adv_pkt.len+1);
	send_multi_type_data(PRO_DATA,para_pro);
	send_rcv_retry_set(PRO_DATA,0,1);  
	prov_para.trans_num_last = prov_para.trans_num;
}

void mesh_adv_prov_data_ack(pro_PB_ADV *p_adv)
{
    SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
	send_rcv_retry_clr();
}

void mesh_rp_adv_prov_complete_rsp(pro_PB_ADV *p_adv)
{
	set_rsp_ack_transnum(p_adv);
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	mesh_send_provison_data(TRANS_ACK,0,0,0);
	send_rcv_retry_clr();
}

void mesh_adv_prov_complete_rsp(pro_PB_ADV *p_adv)
{
    SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)p_adv,p_adv->length+1);               
    set_rsp_ack_transnum(p_adv);
    send_rcv_retry_clr();
    mesh_send_provison_data(TRANS_ACK,0,0,0); 
    send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
    SET_RESULT_TESTCASE(0,0);
}
void mesh_adv_prov_link_close()
{
    mesh_provision_para_reset();
}	

void mesh_pro_rc_adv_dispatch(pro_PB_ADV *p_adv){
	//provision function 
#if (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)

	mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
	mesh_pro_data_structer *p_rcv_str = (mesh_pro_data_structer *)(p_adv->transStart.data);
	// if the rcv pkt is a adv pkt ,we should remeber the transaction num ,then rsp with the same transaction num 
	if(p_adv->transBear.bearOpen.header.GPCF == BEARS_CTL &&
			   p_adv->transBear.bearAck.header.BearCtl == LINK_CLOSE){            
		//reset the link id and others 
		
		if(p_adv->transBear.bearCls.reason != PRO_LINK_CLOSE_SUC){
		    #if GATEWAY_ENABLE
                gateway_upload_prov_link_cls(p_adv->pb_pdu,sizeof(bear_close_link));
		    #endif
			mesh_adv_prov_link_close();
			LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"prov rcv link close",0);
		}
	}
	else if((p_adv->transBear.bearAck.header.GPCF == TRANS_START) && (p_adv->transStart.data[0]== PRO_CAPABLI)){
		u8 rsp_ack_transnum = prov_para.rsp_ack_transnum;
		set_rsp_ack_transnum(p_adv);
	    mesh_send_provison_data(TRANS_ACK,0,0,0); 
		prov_para.rsp_ack_transnum = rsp_ack_transnum;
	}
	
	switch(prov_para.provison_send_state){
		case LINK_ESTABLISHED_ACK:
			if(p_adv->transBear.bearAck.header.GPCF == BEARS_CTL &&
				p_adv->transBear.bearAck.header.BearCtl == LINK_ACK){
					// if receive the establish ack 
					// init the prov_oob para meter 
					mesh_adv_prov_link_open_ack(p_adv);
					mesh_adv_prov_send_invite(p_send_str);
					#if GATEWAY_ENABLE
					gateway_upload_prov_cmd((u8 *)p_send_str,PRO_INVITE);
					#endif
					pro_input[0] = p_send_str->invite.attentionDura;	
					prov_para.provison_send_state = STATE_DEV_CAPA_ACK;
					prov_para.link_id_filter =1;
					mesh_node_prov_event_callback(EVENT_MESH_PRO_RC_LINK_START);
				}
			break;
			/*
		case STATE_PRO_INVITE_ACK:
			//wait for the ack 
			if(p_adv->transAck.GPCF == TRANS_ACK && p_adv->trans_num >= prov_para.trans_num_last){
					mesh_adv_prov_invite_ack(p_adv);
					provision_mag.provison_send_state = STATE_DEV_CAPA_ACK;
				}
			break;
			*/
		case STATE_DEV_CAPA_ACK:
			//wait for the capa and then rsp the ack,and then send the start cmd 
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_CAPABLI){
				    // dispatch the the capbility part 
				    #if GATEWAY_ENABLE
					gateway_upload_prov_rsp_cmd((u8 *)p_rcv_str,PRO_CAPABLI);
					#endif
	                memcpy(pro_input+1,&(p_rcv_str->capa.ele_num),11);
	                prov_para.ele_cnt = p_rcv_str->capa.ele_num;
	                swap_mesh_pro_capa((mesh_pro_data_structer *)p_adv->transStart.data);// swap the endiness for the capa data
					mesh_adv_prov_capa_rcv(p_adv);
					// get the capabilty para
	                memcpy(&prov_oob.capa , &p_rcv_str->capa,sizeof(p_rcv_str->capa));
	                get_pubkey_oob_info_by_capa(&prov_oob);
	                set_start_para_by_capa(&prov_oob);
					mesh_adv_prov_send_start_cmd(p_send_str,&(prov_oob.start));
					#if GATEWAY_ENABLE
					gateway_upload_prov_cmd((u8 *)p_send_str,PRO_START);
					#endif
					memcpy(pro_input+12,&(p_send_str->start.alogrithms),5);
					prov_para.provison_send_state = STATE_PRO_START_ACK;
				}
			break;
		case STATE_PRO_START_ACK:
			//ack and then send the public key cmd 
			if(p_adv->transAck.GPCF == TRANS_ACK&& p_adv->trans_num >= prov_para.trans_num_last){
					u8 prov_public_key[64];
					get_public_key(prov_public_key);
					mesh_adv_prov_start_ack(p_adv);
					mesh_adv_prov_pubkey_send(p_send_str,prov_public_key);
					#if GATEWAY_ENABLE
					gateway_upload_prov_cmd((u8 *)p_send_str,PRO_PUB_KEY);
					#endif
					#if TESTCASE_FLAG_ENABLE
					u8 tmp_data_len = get_mesh_pro_str_len(PRO_PUB_KEY);
					memcpy(tc_seg_buf, (u8 *)(&(pro_adv_pkt.len)),OFFSETOF(pro_PB_ADV,transStart.data));
					memcpy(tc_seg_buf+OFFSETOF(pro_PB_ADV,transStart.data), para_pro, tmp_data_len);
					SET_TC_FIFO(TSCRIPT_MESH_TX, tc_seg_buf,tmp_data_len+OFFSETOF(pro_PB_ADV,transStart.data));
					#endif
					prov_para.provison_send_state = STATE_PRO_PUB_KEY_ACK;
			}
			break;
		case STATE_PRO_PUB_KEY_ACK:
			//wait the ack 
			if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
				SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
				if(prov_oob.start.pubKey == MESH_PUB_KEY_WITH_OOB){
					// not need to exchange the part of the pubkey ,just need to send the pubkey
					// only need to send the comfirm command 
					if(!mesh_prov_oob_auth_data(&prov_oob)){
					    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"prov oob is invalid",0);
						return ;
					}
					u8 prov_public_key[64] = {0};					
					get_public_key(prov_public_key);
					send_rcv_retry_clr();
					
					memcpy(pro_input+0x11,prov_public_key,sizeof(prov_public_key));
					memcpy(pro_input+0x11+64,pub_key_with_oob,64);
					if(prov_oob.start.authMeth == MESH_OUTPUT_OOB){
						prov_oob.oob_out_tick = clock_time()|1;
						prov_para.provison_send_state = STATE_DEV_PUB_KEY_OUTPUT_OOB ;
					}else if (prov_oob.start.authMeth == MESH_INPUT_OOB){
						prov_para.provison_send_state = STATE_DEV_PUB_KEY_INPUT_OOB;
					}else{
						send_comfirm_no_pubkey_cmd();
					}
				}else{
					prov_para.provison_send_state = STATE_DEV_PUB_KEY;
				}	
			}
			break;
		case STATE_DEV_PUB_KEY_INPUT_OOB:
			 // how to input the numbers ,wait for the complete cmd 
			 if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_INPUT_COM){
					set_rsp_ack_transnum(p_adv);
					send_rcv_retry_clr();
					send_comfirm_no_pubkey_cmd_with_ack();
			 }
			 break;
		case STATE_PRO_FAILED_ACK:
			// send the link close cmd 
			mesh_terminate_provision_link_reset(LINK_CLOSE_FAIL);
			break;
		case STATE_DEV_PUB_KEY_OUTPUT_OOB:
				// switch to mainloop process part
			 break;
		
		case STATE_DEV_PUB_KEY:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_PUB_KEY){
				    #if GATEWAY_ENABLE
					gateway_upload_prov_rsp_cmd((u8 *)p_rcv_str,PRO_PUB_KEY);
					#endif
					SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(&rcv_pb),rcv_pb.transStart.total_len+11);
					LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv dev pub key:",0);
					if(!mesh_prov_oob_auth_data(&prov_oob)){
					    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"prov oob info is invalid",0);
						return ;
					}
					u8 prov_public_key[64] = {0};
					get_public_key(prov_public_key);
					mesh_adv_prov_pubkey_rsp(p_adv);
					memcpy(pro_input+0x11,prov_public_key,sizeof(prov_public_key));
					memcpy(pro_input+0x11+64,p_rcv_str->pubkey.pubKeyX,64);
					provision_random_data_init();
					if(prov_oob.start.authMeth == MESH_OUTPUT_OOB)	{
						prov_oob.oob_out_tick = clock_time()|1;
						mesh_send_provison_data(TRANS_ACK,0,0,0);
						send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
						prov_para.provison_send_state = STATE_DEV_PUB_KEY_OUTPUT_OOB ;

					}else if (prov_oob.start.authMeth == MESH_INPUT_OOB) {
						// need to send the ack retry first ,then jump the state to the STATE_DEV_PUB_KEY_INPUT_OOB and wait the incomplete cmd 
						mesh_send_provison_data(TRANS_ACK,0,0,0);
						send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
						prov_para.provison_send_state = STATE_DEV_PUB_KEY_INPUT_OOB;
					}else{
						// should check whether the pubkey is valid or not 
						if(!mesh_check_pubkey_valid(p_rcv_str->pubkey.pubKeyX)){
							// send the link close cmd 
							mesh_send_provison_data(TRANS_ACK,0,0,0);
							send_rcv_retry_set(PRO_COMMAND_ACK,0,0);
							link_close_start_tick = clock_time()|1;
							prov_para.provison_send_state = STATE_DEV_CONFIRM;
							return;
						}
						send_comfirm_no_pubkey_cmd_with_ack();
						#if GATEWAY_ENABLE
					    gateway_upload_prov_cmd((u8 *)p_send_str,PRO_CONFIRM);
					    #endif
					}				
				}else if(p_adv->transStart.data[0]!= PRO_PUB_KEY && p_adv->transAck.GPCF != TRANS_ACK){
                    #if 0
					set_rsp_ack_transnum(p_adv);
					send_rcv_retry_clr();
					mesh_send_provison_data(TRANS_ACK,0,0,0);
					set_pro_fail(p_send_str,PROVISION_FAIL_UNEXPT_PDU);
					send_multi_type_data(PRO_FAIL,para_pro);
					send_rcv_retry_set(PRO_FAIL,0,1);
					prov_para.trans_num_last = provision_mag.trans_num;
					provision_mag.provison_rcv_state = STATE_PRO_FAILED_ACK;
					mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_FAIL_CODE);
					#endif
				}
			break;
			/*
		case STATE_PRO_CONFIRM_ACK:
			if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
					mesh_adv_prov_comfirm_ack(p_adv);
					provision_mag.provison_send_state = STATE_DEV_CONFIRM;
				}
			break;
			*/
		case STATE_DEV_CONFIRM:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_CONFIRM){
					// get device comfirm part 
					#if GATEWAY_ENABLE
					gateway_upload_prov_rsp_cmd((u8 *)p_rcv_str,PRO_CONFIRM);
					#endif
					pro_trans_comfirm *p_comfirm = &(p_rcv_str->comfirm);
					// store the dev comfrim part 
					memcpy(dev_sig_comfirm,p_comfirm->comfirm,sizeof(dev_sig_comfirm));
					LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv prov confirm:",0);
					mesh_adv_prov_comfirm_rsp(p_adv);
					mesh_adv_prov_random_cmd(p_send_str,pro_random);
					#if GATEWAY_ENABLE
					gateway_upload_prov_cmd((u8 *)p_send_str,PRO_RANDOM);
					#endif
					prov_para.provison_send_state = STATE_DEV_RANDOM;
					
				}
			break;
			/*
		case STATE_PRO_RANDOM_ACK:
			if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
					mesh_adv_prov_random_ack_cmd(p_adv);
					provision_mag.provison_send_state = STATE_DEV_RANDOM;
				}
			break;
			*/
		case STATE_DEV_RANDOM:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_RANDOM){
				    #if GATEWAY_ENABLE
					gateway_upload_prov_rsp_cmd((u8 *)p_rcv_str,PRO_RANDOM);
					#endif
					u8 pro_dat_tmp[40];
					SET_TC_FIFO(TSCRIPT_MESH_RX,(u8 *)(p_adv),p_adv->length+1);
					mesh_adv_prov_random_rsp(p_adv);
					memcpy(dev_random,p_rcv_str->random.random,16);
					LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv dev random:",0);
					u8 dev_comfirm_tmp[16];
					mesh_sec_prov_confirmation (dev_comfirm_tmp, pro_input, 
												145, pro_edch, dev_random, pro_auth);
					if(memcmp(dev_comfirm_tmp,dev_sig_comfirm,sizeof(dev_sig_comfirm)))
					{
					    LOG_MSG_ERR(TL_LOG_PROVISION,0, 0 ,"prov comfirm fail",0);
						static u32 A_debug_dev_comfirm_err =0;
						A_debug_dev_comfirm_err++;
						// send the link close cmd 
						mesh_terminate_provision_link_reset(LINK_CLOSE_FAIL);
					}
					mesh_sec_prov_session_key (pro_session_key, pro_session_nonce_tmp, pro_input, 145, pro_edch, pro_random, dev_random);
					memcpy(pro_session_nonce,pro_session_nonce_tmp+3,13);
					//save the src pro_dat
					memcpy(pro_dat_tmp,pro_dat,sizeof(pro_dat));

					u8 dev_key[16];
					u8 prov_salt[16];
					mesh_sec_prov_salt(prov_salt,pro_input,pro_random,dev_random);
					//calculate the dev_key part 
					mesh_sec_dev_key(dev_key,prov_salt,pro_edch);
					// should not set the mesh key info into the mesh_key ,the devkey in the mesh_key is self
					// set_dev_key(dev_key);
					LOG_MSG_INFO(TL_LOG_PROVISION, dev_key, 16,"dev key is:",0);
					set_pro_unicast_adr(provision_mag.unicast_adr_last);
					
					#if (DONGLE_PROVISION_EN)
					gateway_provision_rsp_cmd(provision_mag.unicast_adr_last);
					VC_node_dev_key_save(provision_mag.unicast_adr_last, dev_key,prov_para.ele_cnt);
					#endif
					LOG_MSG_INFO(TL_LOG_PROVISION, pro_dat, 25,"send prov data:",0);
					mesh_prov_sec_msg_enc (pro_session_key, pro_session_nonce, pro_dat, 25, 8);
                    mesh_adv_prov_data_send(p_send_str,pro_dat);
					//back the src pro dat
					#if GATEWAY_ENABLE
					gateway_upload_prov_cmd((u8 *)p_send_str,PRO_DATA);
					#endif
					memcpy(pro_dat,pro_dat_tmp,sizeof(pro_dat));
					#if TESTCASE_FLAG_ENABLE
					u8 tmp_data_len = get_mesh_pro_str_len(PRO_DATA);
					memcpy(tc_seg_buf, (u8 *)(&(pro_adv_pkt.len)),OFFSETOF(pro_PB_ADV,transStart.data));
					memcpy(tc_seg_buf+OFFSETOF(pro_PB_ADV,transStart.data), para_pro, tmp_data_len);
					SET_TC_FIFO(TSCRIPT_MESH_TX, tc_seg_buf,tmp_data_len+OFFSETOF(pro_PB_ADV,transStart.data));					
					#endif
					
					prov_para.provison_send_state = STATE_PRO_COMPLETE;
				}
			break;
		/*
		case STATE_PRO_DATA_ACK:
			if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
				mesh_adv_prov_data_ack(p_adv);
				provision_mag.provison_send_state = STATE_PRO_COMPLETE;
			}
			break;
		*/
		case STATE_PRO_COMPLETE:
			if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_COMPLETE){
				// return the test case result 
				    #if GATEWAY_ENABLE
					gateway_upload_prov_rsp_cmd((u8 *)p_rcv_str,PRO_COMPLETE);
					#endif
					LOG_MSG_INFO(TL_LOG_PROVISION, 0, 0,"rcv prov complete",0);
				    mesh_adv_prov_complete_rsp(p_adv);
					mesh_prov_end_set_tick();
					mesh_node_prov_event_callback(EVENT_MESH_PRO_RC_LINK_SUC);
				}
			break;
		default:
			break;
	}
#endif 
	return ;
}

u8 filter_prov_link_id(pro_PB_ADV *p_adv)
{
	u8 invalid =0;
#if !WIN32
	if(prov_para.link_id_filter){
		if(memcmp(p_adv->link_ID,(u8 *)(&prov_para.link_id),4)){
			invalid = 1;
		}
	}
#endif 
	return invalid;
}

int mesh_provision_seg_check(u8 seg_idx, pro_PB_ADV * p_adv, u8 * rcv_pb_buf)
{
	int ret = 1;
	u16 rcv_pb_idx = 0;

	if(seg_idx){
		rcv_pb_idx = (seg_idx-1)*OFFSET_CON+ OFFSET_START;
	}

	if((p_adv->trans_num != mesh_prov_seg.trans_num) || (mesh_prov_seg.link_id != prov_para.link_id)){
		memset(&mesh_prov_seg, 0x00, sizeof(mesh_prov_seg));
		mesh_prov_seg.seg_n = 0x80; // first pkt maybe not start PDU
		mesh_prov_seg.trans_num = p_adv->trans_num;
		mesh_prov_seg.link_id = prov_para.link_id;
	}
	
	if(!(mesh_prov_seg.seg_map & BIT(seg_idx))){
		mesh_prov_seg.seg_map |= BIT(seg_idx);
		if(seg_idx == 0){
			memcpy(rcv_pb_buf,p_adv,p_adv->length+1);
			mesh_prov_seg.seg_n = p_adv->transStart.SegN;
			mesh_prov_seg.fcs_tmp = p_adv->transStart.FCS;
		}
		else{
			memcpy(rcv_pb_buf+rcv_pb_idx,p_adv->transCon.data ,p_adv->length - 7);
		}
		mesh_prov_seg.seg_cnt++;
	}
	
	if(mesh_prov_seg.seg_cnt == (mesh_prov_seg.seg_n+1)){
		if(mesh_prov_seg.fcs_tmp == crc8_rohc(rcv_pb.transStart.data,rcv_pb.transStart.total_len)){
			ret = 0;
		}
	}
	
	return ret;
}

int mesh_provison_process(u8 ini_role,u8 *p_rcv)
{	
	pro_PB_ADV * rcv_pb_buf = (&rcv_pb);
	pro_PB_ADV * p_adv = (pro_PB_ADV *) p_rcv;
	if(p_adv->ad_type == MESH_ADV_TYPE_BEACON ){
	    pro_PB_ADV rcv_beacon;
		memcpy((u8*)&rcv_beacon,p_adv,p_adv->length+1+7);
		event_adv_report_t *pa;
		pa = (event_adv_report_t *)(p_rcv-11);
		mesh_pro_rc_beacon_dispatch(&rcv_beacon,pa->mac);
		return 1;
	}
	if(filter_prov_link_id((pro_PB_ADV *)p_adv)){
		return 1;
	}
	// if the send pdu is not a start pdu ,we need to pack it 
	if(p_adv->transStart.GPCF == TRANS_START ){
		// if receive the trans start pdu ,it need to swap the length 
		endianness_swap_u16((u8 *)(&(p_adv->transStart.total_len)));
		// it means it 's a single start packet 
		if(p_adv->transStart.SegN == 0){
			// check the fcs is right or not 
			u8 start_fcs = p_adv->transStart.FCS;
			if(start_fcs!= crc8_rohc(p_adv->transStart.data,p_adv->transStart.total_len)){
				return 0;
			}
			rcv_pb_buf = p_adv;
		}else{
			if(mesh_provision_seg_check(0, p_adv, (u8 *)rcv_pb_buf)){
				return 1;
			}
		}
	}else if (p_adv->transCon.GPCF == TRANS_CON ){
		if(mesh_provision_seg_check(p_adv->transCon.SegIdx, p_adv,(u8 *) rcv_pb_buf)){
			return 1;
		}
	}else if(p_adv->transStart.GPCF == TRANS_ACK || p_adv->transStart.GPCF == BEARS_CTL){
		// bearer ctl or ack 
		rcv_pb_buf = p_adv;
	}

	if(ini_role == MESH_INI_ROLE_NODE){
		mesh_node_rc_data_dispatch(rcv_pb_buf);
	}else if(ini_role == MESH_INI_ROLE_PROVISIONER && p_adv->ad_type == MESH_ADV_TYPE_PRO){
		if(provision_mag.pro_stop_flag){
			return 1;
		}
		mesh_pro_rc_adv_dispatch(rcv_pb_buf);
	}else{
	}
	return 1;
}

void mesh_prov_proc_loop()
{
    #if FEATURE_PROV_EN
	mesh_adv_provision_retry();
	check_oob_out_timeout();
	wait_and_check_complete_state();
	check_inputoob_proc();
	mesh_loop_provision_end_process();
	check_mesh_node_out_oob_pub_key_send_time();
	mesh_loop_check_link_close_flag();
	mesh_prov_link_close_terminate();
	#if !WIN32
	provision_timeout_cb();
	#endif
	#endif
}
 
#if (IS_VC_PROJECT_MASTER || DONGLE_PROVISION_EN)
 /*************hci_tx_fifo_poll ***********************
 device key list save and read
 ******************************************************/ 

 #if DONGLE_PROVISION_EN
 VC_node_info_t VC_node_info[MESH_NODE_MAX_NUM];
 VC_node_cps_t VC_node_cps;
 #else
 VC_node_info_t VC_node_info[MESH_NODE_MAX_NUM];        // 1000*(20+384) = 404000
 #endif
 
#if WIN32 
STATIC_ASSERT(sizeof(VC_node_info) <= 4096*128);
#else
STATIC_ASSERT(sizeof(VC_node_info) <= 4096);	// because only one sector for it now
#endif

void save_vc_node_info_all()
{
    #if WIN32
    // erase all the store vc node info part 
    // erase_vc_node_info(); // no need erase, write directly later
    #else
    flash_erase_sector(FLASH_ADR_VC_NODE_INFO);
    #endif
    flash_write_page(FLASH_ADR_VC_NODE_INFO, sizeof(VC_node_info), (u8 *)VC_node_info);
}

#if WIN32
void save_vc_node_info_single(VC_node_info_t *p_info)
{
    // erase all the store vc node info part 
    // erase_vc_node_info(); // no need erase, write directly later
    u32 adr_save = FLASH_ADR_VC_NODE_INFO + (u8 *)p_info - (u8 *)&VC_node_info;
    flash_write_page(adr_save, sizeof(VC_node_info_t), (u8 *)p_info);
}
#endif

void VC_cmd_clear_all_node_info(u16 adr)
{
	if(adr == 0xffff){
		provision_mag.unicast_adr_last =1;// reset the adr part 
		provision_mag_cfg_s_store();// avoid restore part 
		memset((u8 *)(VC_node_info),0xff,sizeof(VC_node_info));
	}else if (is_unicast_adr(adr)){
		VC_node_info_t *p_info = get_VC_node_info(adr, 1);
		if(p_info){
            memset(p_info,0xff,sizeof(VC_node_info_t));
		}
	}else{
		return;
	}
	
    save_vc_node_info_all();
}

VC_node_info_t * get_VC_node_info(u16 obj_adr, int is_must_primary)
{
	if(obj_adr && is_unicast_adr(obj_adr)){
        foreach(i,MESH_NODE_MAX_NUM){
            VC_node_info_t *p_info = &VC_node_info[i];
            if(p_info->node_adr){
                if(is_must_primary){
                    if(p_info->node_adr == obj_adr){    
                        return p_info;
                    }
                }else if(is_ele_in_node(obj_adr, p_info->node_adr, p_info->element_cnt)){
                    return p_info;
                }
            }
        }
	}
	
#if WIN32
    LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"obj_adr 0x%04x, not found VC node info", obj_adr);
#endif

    return 0;
}

void del_vc_node_info_by_unicast(u16 unicast)
{
    VC_node_info_t * p_node_info = get_VC_node_info(unicast,1);        
    if(p_node_info!=0){
        // clear the vc node info part 
        #if WIN32
        memset(p_node_info,0,sizeof(VC_node_info_t));
        #else
        memset(p_node_info,0xff,sizeof(VC_node_info_t));
        #endif
    }
    save_vc_node_info_all();//save the vc node info part 
}

#if IS_VC_PROJECT_MASTER
void erase_vc_node_info()
{
    flash_erase_sector_VC(0, sizeof(VC_node_info));
}

/**
* function: get offset between object addr and element addr which include the model.
*/
u8 get_ele_offset_by_model_VC_node_info(u16 obj_adr, u32 model_id, int sig_model)
{
    VC_node_info_t *p_info = get_VC_node_info(obj_adr, 0);
    if(p_info){
        return get_ele_offset_by_model((mesh_page0_t *)(&p_info->cps.page0_head), p_info->cps.len_cps, p_info->node_adr, obj_adr, model_id, sig_model);
    }
    LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "VC node info NOT_FOUND!........................",0);
    return MODEL_NOT_FOUND;
}
#endif
#if WIN32
static u32 mesh_vc_node_addr = FLASH_ADR_VC_NODE_INFO;
#endif

void VC_node_info_retrieve()
{
#if WIN32
    int err = mesh_par_retrieve((u8 *)VC_node_info, &mesh_vc_node_addr, FLASH_ADR_VC_NODE_INFO, sizeof(VC_node_info));
#else    
	flash_read_page(FLASH_ADR_VC_NODE_INFO, sizeof(VC_node_info), (u8 *)VC_node_info);
#endif
}
 
int VC_node_dev_key_save(u16 adr, u8 *dev_key,u8 ele_cnt)
{
    if(!is_unicast_adr(adr) && (!adr)){
        return -1;
    }

    VC_node_info_t *p_info = 0;    // save
    VC_node_info_t *p_info_1st_empty = 0;
    foreach(i,MESH_NODE_MAX_NUM){
        VC_node_info_t *p_info_tmp = &VC_node_info[i];
        if(p_info_tmp->node_adr == adr){    
            p_info = p_info_tmp;  // existed
            break;
        }else if((!p_info_1st_empty) && ((0 == p_info_tmp->node_adr)||(p_info_tmp->node_adr >= 0x8000))){
            p_info_1st_empty = p_info_tmp;
        }
    }

    if(0 == p_info){
        p_info = p_info_1st_empty;
    }
    if(p_info){
        if(!((adr == p_info->node_adr)&&(!memcmp(p_info->dev_key, dev_key, 16)))){
            p_info->node_adr = adr;
			p_info->element_cnt = ele_cnt;
            memcpy(p_info->dev_key, dev_key, 16);

            #if DONGLE_PROVISION_EN
            save_vc_node_info_all();
            #else
            save_vc_node_info_single(p_info);
            #endif
        }
        return 0;
    }

    return -1; 	 // full
}
 
u8* VC_master_get_other_node_dev_key(u16 adr)
{
     if(is_actived_factory_test_mode()){
         return mesh_key.dev_key;
     }
     
	 VC_node_info_t *p_info = get_VC_node_info(adr, 1);
	 if(p_info){
	    return p_info->dev_key;
	 }
 
	 return 0;
}

u8 VC_node_cps_save(mesh_page0_t * p_page0,u16 unicast, u32 len_cps)
{
    VC_node_info_t *p_info = get_VC_node_info(unicast, 1); // have been sure no same node address in node info
    if(p_info){
        #if DONGLE_PROVISION_EN
        VC_node_cps_t *p_cps = &VC_node_cps;
        #else
		VC_node_cps_t *p_cps = &p_info->cps;
		if(unicast == ele_adr_primary){
            p_info->element_cnt = g_ele_cnt;
        }else{
        	u8 cps_ele_cnt =0;
            cps_ele_cnt = get_ele_cnt_by_traversal_cps(p_page0, len_cps);
			if(p_info->element_cnt == 0){
				p_info->element_cnt = cps_ele_cnt;
			}else if (cps_ele_cnt != p_info->element_cnt){
    			p_info->element_cnt =0;
    		}
		}
        #endif
        p_cps->len_cps = len_cps;
        memcpy(&p_cps->page0_head, p_page0, len_cps);
        
        #if WIN32
        save_vc_node_info_single(p_info);
        #endif
        
        return 0;
    }
    // to save later
    return -1;
}
void mesh_cfg_keybind_start_trigger_event(u8* p_idx,u8 *p_key,u16 unicast,u16 nk_idx,u8 fastbind)
{
	prov_para.key_bind_lock = 1;
	u16 key_idx =0;
	key_idx = (p_idx[0]<<8)+p_idx[1];
	mesh_kc_cfgcl_mode_para_set(key_idx,p_key,unicast,nk_idx,fastbind);
	#if !WIN32 
	enable_mesh_kr_cfg_filter();
	#endif 
}
void set_provision_stop_flag_act(u8 stop_flag)
{
	provision_mag.pro_stop_flag = stop_flag;
	if(stop_flag){
		mesh_provision_para_reset();		
		#if !WIN32
		prov_para.link_id_filter =0;
		disable_mesh_adv_filter();
		gateway_adv_filter_init();
		#endif
		rf_link_light_event_callback(PROV_END_LED_CMD);
	}else{
		rf_link_light_event_callback(PROV_START_LED_CMD);	
	}
}

u8 mesh_cfg_keybind_end_event(u8 eve,u16 unicast)
{
	key_refresh_cfgcl_proc_t *p_cfgcl =(key_refresh_cfgcl_proc_t *) &key_refresh_cfgcl_proc;
	if(eve == MESH_KEYBIND_EVE_START){
		// set the lock flag of the key_bind_lock
		prov_para.key_bind_lock = 1;
		// gateway will not enter ,wait for the adv-provision for the keybind part 
		mesh_kc_cfgcl_mode_para_set(0,(u8 *)p_cfgcl->ak,unicast,0,0);
		#if !WIN32 
		enable_mesh_kr_cfg_filter();
		#endif 
	}else if (eve == MESH_KEYBIND_EVE_SUC){
		p_cfgcl->st = KR_CFGCL_NORMAL;
		p_cfgcl->timeout = 0;
		#if GATEWAY_ENABLE
		//set_provision_stop_flag_act(1);//lock the start flag part 
		gateway_upload_keybind_event(MESH_KEYBIND_EVE_SUC);
		#endif
		#if  WIN32
		App_key_bind_end_callback(MESH_APP_KEY_BIND_EVENT_SUC); 
		#else
		disable_mesh_kr_cfg_filter();
		prov_para.key_bind_lock =0;
		gateway_keybind_rsp_cmd(0);
		rf_link_light_event_callback(LGT_CMD_SET_MESH_INFO);
		#endif 
		LOG_MSG_INFO(TL_LOG_KEY_BIND,0,0,"SEND: mesh keybind event success",0);
		
	}else if (eve == MESH_KEYBIND_EVE_TIMEOUT){
		LOG_MSG_INFO(TL_LOG_GATT_PROVISION,(u8 *)p_cfgcl, sizeof(key_refresh_cfgcl_proc_t),"appkey bind terminate\r\n",0);
		
		// need to reset the timeout tick 
		p_cfgcl->timeout = 0;
		// reset the status of the states of the key_refresh_cfgcl_proc
		p_cfgcl->st = KR_CFGCL_NORMAL;
		// reset the status of the provision part 
		mesh_provision_para_reset();
		// reset the mode of the provision mag part 
		// cancel the part of the bind lock flag 
		prov_para.key_bind_lock =0;
		#if  WIN32
		App_key_bind_end_callback(MESH_APP_KEY_BIND_EVENT_TIMEOUT);	
		#else 
		disable_mesh_kr_cfg_filter();
		#endif 
		#if GATEWAY_ENABLE
		gateway_upload_keybind_event(MESH_KEYBIND_EVE_TIMEOUT);
		gateway_keybind_rsp_cmd(1);
		#endif 
		LOG_MSG_INFO(TL_LOG_KEY_BIND,0,0,"SEND: mesh keybind event timeout",0);
	}
	return 1;

}
u8 VC_search_and_bind_model()
{
    key_refresh_cfgcl_proc_t *p =(key_refresh_cfgcl_proc_t *) &key_refresh_cfgcl_proc;
    VC_node_info_t *p_info = get_VC_node_info(p->node_adr, 1);
    if(p_info){
        #if DONGLE_PROVISION_EN
        VC_node_cps_t *p_cps = &VC_node_cps;
        #else
        VC_node_cps_t *p_cps = &p_info->cps;
        #endif

        mesh_element_head_t *p_ele = (mesh_element_head_t *)(&p_cps->local);
        foreach(i,p->ele_bind_index){
            p_ele = (mesh_element_head_t *)((u8 *)p_ele + get_cps_ele_len(p_ele));
        }
        u16 ele_adr = p->node_adr + p->ele_bind_index;
        mesh_element_model_id *p_model_id = (mesh_element_model_id *)(&p_ele->nums);

        if(p_info->element_cnt && (p->model_bind_index < p_model_id->nums)){//nums
            u16 model_id;
            memcpy(&model_id, p_model_id->id+p->model_bind_index*2, 2);
            if(is_cfg_model(model_id, 1)){
                p->model_bind_index++;
            }else{
                LOG_MSG_INFO(TL_LOG_KEY_BIND,0,0,"SEND: appkey bind addr: 0x%04x,sig model id: 0x%04x ",ele_adr,model_id);
                cfg_cmd_ak_bind(p->node_adr, ele_adr, p->ak_idx, model_id, 1);
            }
        }
        else if(p_info->element_cnt && (p->model_bind_index < p_model_id->nums+ p_model_id->numv)){
            u32 model_id;
            memcpy(&model_id, p_model_id->id+p_model_id->nums*2+(p->model_bind_index-p_model_id->nums)*4, 4);
            LOG_MSG_INFO(TL_LOG_KEY_BIND,0,0,"SEND: appkey bind addr: 0x%04x,vendor model id: 0x%08x ",ele_adr,model_id);
            cfg_cmd_ak_bind(p->node_adr, ele_adr, p->ak_idx, model_id, 0);
        }
        else{
            if((++p->ele_bind_index) < p_info->element_cnt){
                p->model_bind_index = 0;
            }else{
                mesh_cfg_keybind_end_event(MESH_KEYBIND_EVE_SUC,0);
            }
        }
        return 0;
    }
    return -1;
}
 //-------------------NET KEY INDEX / APP KEY INDEX
#if 0
 STATIC_ASSERT(sizeof(VC_key_idx_list_t) % 16 == 0);
 
 VC_key_idx_list_t VC_key_idx_list[NET_KEY_LIST_MAX_PROV] = {
	 {
		 KEY_VALID, NET0, {0}, NETKEY_A,
		 {
			 {KEY_VALID, NET0_APP0, {0}, APPKEY_A},
			 {KEY_VALID, NET0_APP1, {0}, APPKEY_B},
			 {KEY_VALID, NET0_APP2, {0}, APPKEY_C},
			 {KEY_VALID, NET0_APP3, {0}, APPKEY_D},
		 },
	 },
 };
#endif
#endif

#if WIN32 
int upload_sig_mesh_para(u8 *buf,int *len)
{
	sdk_info_t *p_info = (sdk_info_t *)(buf);
	memcpy(p_info->node_info,VC_node_info,sizeof(VC_node_info));
	memcpy(&p_info->mesh_key,&mesh_key,sizeof(mesh_key));
	*len = sizeof(sdk_info_t);
	return 1;
}

int download_sig_mesh_para(u8 *buf,int len )
{
	if(len<sizeof(sdk_info_t)){
		return -1;
	}
	sdk_info_t *p_info = (sdk_info_t *)buf;
	memcpy(&mesh_key,&p_info->mesh_key,sizeof(mesh_key));
	flash_erase_sector(FLASH_ADR_MESH_KEY);
	flash_write_page(FLASH_ADR_MESH_KEY,sizeof(mesh_key),(const u8 *)&mesh_key);
	
	// erase all the store vc node info part 
	memcpy(VC_node_info,p_info->node_info,sizeof(VC_node_info));
	save_vc_node_info_all();
	return 0;
}
#endif



