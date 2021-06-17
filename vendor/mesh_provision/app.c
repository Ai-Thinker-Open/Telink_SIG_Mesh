/********************************************************************************************************
 * @file     app.c 
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
#include "proj_lib/rf_drv.h"
#include "proj_lib/pm.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ll/ll_whitelist.h"
#include "proj_lib/ble/trace.h"
#include "proj/mcu/pwm.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj/drivers/adc.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ble_smp.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"

#include "proj_lib/sig_mesh/app_mesh.h"
#include "../common/app_provison.h"
#include "../common/app_beacon.h"
#include "../common/app_proxy.h"
#include "../common/app_health.h"
#include "../common/mesh_ota.h"
#include "proj/drivers/keyboard.h"
#include "app.h"
#include "stack/ble/gap/gap.h"
#include "vendor/common/blt_soft_timer.h"
#include "proj/drivers/rf_pa.h"

#if (HCI_ACCESS==HCI_USE_UART)
#include "proj/drivers/uart.h"
#endif

#if(MCU_CORE_TYPE == MCU_CORE_8269) // 8269 ram limited
MYFIFO_INIT(blt_rxfifo, 64, 8);
MYFIFO_INIT(blt_txfifo, 40, 4);
#else
#define BLT_RX_FIFO_SIZE        (MESH_DLE_MODE ? DLE_RX_FIFO_SIZE : 64)
#define BLT_TX_FIFO_SIZE        (MESH_DLE_MODE ? DLE_TX_FIFO_SIZE : 40)

MYFIFO_INIT(blt_rxfifo, BLT_RX_FIFO_SIZE, 16);
MYFIFO_INIT(blt_txfifo, BLT_TX_FIFO_SIZE, 32);
#endif

//u8		peer_type;
//u8		peer_mac[12];

#if MD_SENSOR_EN
STATIC_ASSERT(FLASH_ADR_MD_SENSOR != FLASH_ADR_VC_NODE_INFO);   // address conflict
#endif
//////////////////////////////////////////////////////////////////////////////
//	Initialization: MAC address, Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////

//----------------------- UI ---------------------------------------------
#if (BLT_SOFTWARE_TIMER_ENABLE)
/**
 * @brief   This function is soft timer callback function.
 * @return  <0:delete the timer; ==0:timer use the same interval as prior; >0:timer use the return value as new interval. 
 */
int soft_timer_test0(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;
	static u32 soft_timer_cnt0 = 0;
	soft_timer_cnt0++;
	return 0;
}
#endif


//----------------------- handle BLE event ---------------------------------------------
int app_event_handler (u32 h, u8 *p, int n)
{
	static u32 event_cb_num;
	event_cb_num++;
	int send_to_hci = 1;

	if (h == (HCI_FLAG_EVENT_BT_STD | HCI_EVT_LE_META))		//LE event
	{
		u8 subcode = p[0];

	//------------ ADV packet --------------------------------------------
		if (subcode == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
		{
			event_adv_report_t *pa = (event_adv_report_t *)p;
			if(LL_TYPE_ADV_NONCONN_IND != (pa->event_type & 0x0F)){
				return 0;
			}

			#if 0 // TESTCASE_FLAG_ENABLE
			u8 mac_pts[] = {0xDA,0xE2,0x08,0xDC,0x1B,0x00};	// 0x001BDC08E2DA
			if(memcmp(pa->mac, mac_pts,6)){
				return 0;
			}
			#endif
			
			#if DEBUG_MESH_DONGLE_IN_VC_EN
			send_to_hci = mesh_dongle_adv_report2vc(pa->data, MESH_ADV_PAYLOAD);
			#else
			send_to_hci = app_event_handler_adv(pa->data, MESH_BEAR_ADV, 1);
			#endif
		}

	//------------ connection complete -------------------------------------
		else if (subcode == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
		{
			event_connection_complete_t *pc = (event_connection_complete_t *)p;
			if (!pc->status)							// status OK
			{
				//app_led_en (pc->handle, 1);

				//peer_type = pc->peer_adr_type;
				//memcpy (peer_mac, pc->mac, 6);
			}
			#if DEBUG_BLE_EVENT_ENABLE
			rf_link_light_event_callback(LGT_CMD_BLE_CONN);
			#endif

			#if DEBUG_MESH_DONGLE_IN_VC_EN
			debug_mesh_report_BLE_st2usb(1);
			#endif
			proxy_cfg_list_init_upon_connection();
			mesh_service_change_report();
		}

	//------------ connection update complete -------------------------------
		else if (subcode == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
		{
		}
	}

	//------------ disconnect -------------------------------------
	else if (h == (HCI_FLAG_EVENT_BT_STD | HCI_EVT_DISCONNECTION_COMPLETE))		//disconnect
	{

		event_disconnection_t	*pd = (event_disconnection_t *)p;
		//app_led_en (pd->handle, 0);
		//terminate reason
		if(pd->reason == HCI_ERR_CONN_TIMEOUT){

		}
		else if(pd->reason == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

		}
		else if(pd->reason == SLAVE_TERMINATE_CONN_ACKED || pd->reason == SLAVE_TERMINATE_CONN_TIMEOUT){

		}
		#if DEBUG_BLE_EVENT_ENABLE
		rf_link_light_event_callback(LGT_CMD_BLE_ADV);
		#endif 

		#if DEBUG_MESH_DONGLE_IN_VC_EN
		debug_mesh_report_BLE_st2usb(0);
		#endif

		mesh_ble_disconnect_cb();
	}

	if (send_to_hci)
	{
		//blc_hci_send_data (h, p, n);
	}

	return 0;
}
u8 prov_end_status=0;
void proc_ui()
{
	static u32 tick, scan_io_interval_us = 40000;
	if (!clock_time_exceed (tick, scan_io_interval_us))
	{
		return;
	}
	tick = clock_time();

	#if 0
	static u8 st_sw1_last;	
	u8 st_sw1 = !gpio_read(SW1_GPIO);
	static u8 st_sw2_last;	
	u8 st_sw2 = !gpio_read(SW2_GPIO);
	if(st_sw1_last != st_sw1 || st_sw2_last != st_sw2 ) {
		if(st_sw1 && st_sw2){
			provision_mag.pro_stop_flag = ~ provision_mag.pro_stop_flag;
			set_provision_stop_flag_act(0);
			
		}else if(st_sw1){
			#if 1
			access_cmd_onoff(0xffff, 0, G_ON, CMD_NO_ACK, 0);
			#else
			 mesh_cfg_model_relay_set_t relay_set;
			relay_set.relay= 0x01;//relay = 0x01;
			relay_set.transmit.count = 0x00;//relay_retransmit=relay_retransmit=0;
			relay_set.transmit.invl_steps= 0x00;
			mesh_tx_cmd2normal_primary(CFG_RELAY_SET, (u8 *)&relay_set, sizeof(mesh_cfg_model_relay_set_t), 0x01, 0x01);
			#endif 
			scan_io_interval_us = 100*1000; // fix dithering
		}else if (st_sw2){
			access_cmd_onoff(0xffff, 0, G_OFF, CMD_NO_ACK, 0);
	    	scan_io_interval_us = 100*1000; // fix ditherin
		}
		st_sw1_last = st_sw1;
		st_sw2_last = st_sw2;
	}


	
	/*
	if((!(st_sw1_last)&&st_sw1) && (!(st_sw2_last)&&st_sw2)){
		prov_press_cnt++;
		prov_end_status = prov_end_status?0:1;
		st_sw1_last = st_sw1;
		st_sw2_last = st_sw2;
		return ;
	}
	if(!(st_sw1_last)&&st_sw1){
		access_cmd_onoff(0xffff, 0, G_ON, CMD_NO_ACK, 0);
	    scan_io_interval_us = 100*1000; // fix dithering
	}
	st_sw1_last = st_sw1;
	
	if(!(st_sw2_last)&&st_sw2){ // dispatch just when you press the button 
		access_cmd_onoff(0xffff, 0, G_OFF, CMD_NO_ACK, 0);
	    scan_io_interval_us = 100*1000; // fix dithering
	}
	st_sw2_last = st_sw2;
	*/
	#endif
}
#if GATEWAY_ENABLE

static u8 gateway_provision_para_enable=0;
void set_gateway_provision_sts(unsigned char en)
{
	gateway_provision_para_enable =en;
	return ;
}
unsigned char get_gateway_provisison_sts()
{
	unsigned char ret;
	ret = gateway_provision_para_enable;
	return ret;
}
void set_gateway_provision_para_init()
{
	gateway_adv_filter_init();
	set_provision_stop_flag_act(1);
	set_gateway_provision_sts(0);//disable the provision sts part 

}
u8 mesh_get_hci_tx_fifo_cnt()
{
#if (HCI_ACCESS == HCI_USE_USB)
	return hci_tx_fifo.size;
#elif (HCI_ACCESS == HCI_USE_UART)
	return hci_tx_fifo.size-0x10;
#else
	return 0;
#endif
}

int gateway_common_cmd_rsp(u8 code,u8 *p_par,u16 len )
{
	u8 head[2] = {TSCRIPT_GATEWAY_DIR_RSP};
	u8 head_len = 2;
	head[1] = code;
	u16 valid_fifo_size = mesh_get_hci_tx_fifo_cnt()-2; // 2: length
	if(len+head_len > valid_fifo_size){
		return gateway_sar_pkt_segment(p_par, len, valid_fifo_size, head, 2);
	}
	else{
		return my_fifo_push_hci_tx_fifo(p_par,len, head, 2); 
	}
}

u8 gateway_provision_rsp_cmd(u16 unicast_adr)
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_RSP_UNICAST , (u8*)(&unicast_adr),2);
}
u8 gateway_keybind_rsp_cmd(u8 opcode )
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_KEY_BIND_RSP , (u8*)(&opcode),1);
}

u8 gateway_model_cmd_rsp(u8 *para,u16 len )
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_RSP_OP_CODE , para,len);
}

u8 gateway_heartbeat_cb(u8 *para,u8 len )
{
	//para reference to struct:  mesh_hb_msg_cb_t 
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_HEARTBEAT , para,len);
}

u8 gateway_upload_mac_address(u8 *p_mac,u8 *p_adv)
{
	u8 para[40];//0~5 mac,adv ,6,rssi ,7~8 dc
	u8 len;
	len = p_adv[0];
	memcpy(para,p_mac,6);
	memcpy(para+6,p_adv,len+4);
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_UPDATE_MAC,para,len+10);
}

u8 gateway_upload_provision_suc_event(u8 evt,u16 adr,u8 *p_mac,u8 *p_uuid)
{
    gateway_prov_event_t prov;
    prov.eve = evt;
    prov.adr = adr;
    memcpy(prov.mac,p_mac,6);
    memcpy(prov.uuid,p_uuid,16);
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_PROVISION_EVT,(u8*)&prov,sizeof(prov));
}

u8 gateway_upload_keybind_event(u8 evt)
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_KEY_BIND_EVT,&evt,1);
}

u8 gateway_upload_node_ele_cnt(u8 ele_cnt)
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_ELE_CNT,&ele_cnt,1);
}
u8 gateway_upload_node_info(u16 unicast)
{
	VC_node_info_t * p_info;
	p_info = get_VC_node_info(unicast,1);
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_NODE_INFO,(u8 *)p_info,sizeof(VC_node_info_t));
}

u8 gateway_upload_provision_self_sts(u8 sts)
{
	u8 buf[26];
	buf[0]=sts;
	if(sts){
		memcpy(buf+1,(u8 *)(&provision_mag.pro_net_info),25);
	}
	provison_net_info_str* p_net = (provison_net_info_str*)(buf+1);
	p_net->unicast_address = provision_mag.unicast_adr_last;
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_PRO_STS_RSP,buf,sizeof(buf));
}

u8 gateway_upload_mesh_ota_sts(u8 *p_dat,int len)
{
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_MESH_OTA_STS,p_dat,len);
}

u8 gateway_upload_mesh_sno_val()
{
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_SNO_RSP,
                        (u8 *)&mesh_adv_tx_cmd_sno,sizeof(mesh_adv_tx_cmd_sno));

}
u8 gateway_upload_dev_uuid(u8 *p_uuid,u8 *p_mac)
{
    u8 uuid_mac[22];
    memcpy(uuid_mac,p_uuid,16);
    memcpy(uuid_mac+16,p_mac,6);
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_UUID,
                        (u8 *)uuid_mac,sizeof(uuid_mac));
}

u8 gateway_upload_ividx(u8 *p_ivi)
{
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_IVI,
                        p_ivi,4);
}

u8 gateway_upload_mesh_src_cmd(u16 op,u16 src,u8 *p_ac_par)
{
    gateway_upload_mesh_src_t cmd;
    cmd.op = op;
    cmd.src = src;
    memcpy(cmd.ac_par,p_ac_par,sizeof(cmd.ac_par));
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_SRC_CMD,
                            (u8*)&cmd,sizeof(cmd));
}
#define GATEWAY_MAX_UPLOAD_CNT 0x20
u8 gateway_upload_prov_cmd(u8 *p_cmd,u8 cmd)
{
    u8 len =0;
    len = get_mesh_pro_str_len(cmd);
    if(len){
        if(len>GATEWAY_MAX_UPLOAD_CNT){
            len = GATEWAY_MAX_UPLOAD_CNT;
        }
        return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND,
                            (u8*)p_cmd,len);
    }
    return 0;
}

u8 gateway_upload_prov_rsp_cmd(u8 *p_rsp,u8 cmd)
{
    u8 len =0;
    len = get_mesh_pro_str_len(cmd);
    if(len){
        if(len>GATEWAY_MAX_UPLOAD_CNT){
            len = GATEWAY_MAX_UPLOAD_CNT;
        }
        return gateway_common_cmd_rsp(HCI_GATEWAY_DEV_RSP,
                            (u8*)p_rsp,len);
    }
    return 0;
}

u8 gateway_upload_prov_link_open(u8 *p_cmd,u8 len)
{
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_LINK_OPEN,
                            (u8*)p_cmd,len);
}

u8 gateway_upload_prov_link_cls(u8 *p_rsp,u8 len)
{
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_LINK_CLS,
                            (u8*)p_rsp,len);
}

u8 gateway_upload_mesh_cmd_back_vc(material_tx_cmd_t *p)
{
	gateway_upload_mesh_cmd_str gateway_cmd;
	u8 len ;
	gateway_cmd.src = p->adr_src;
	gateway_cmd.dst = p->adr_dst;
	gateway_cmd.opcode = p->op;
	if(p->par_len >sizeof(gateway_cmd.para)){
		len = sizeof(gateway_cmd.para);
	}else{
		len = p->par_len;
	}
	memcpy(gateway_cmd.para , p->par , len);
	len+=6;
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_BACK_VC,
                            (u8*)(&gateway_cmd),len);
}
u8 gateway_upload_log_info(u8 *p_data,u8 len ,char *format,...) //gateway upload the print info to the vc
{
	// get the info part 
	char log_str[60];
	va_list list;
	va_start( list, format );
	char *p_buf;
	char **pp_buf;
	
	p_buf = log_str;
	pp_buf = &(p_buf);
	u32 head_len = print(pp_buf,format,list);	// log_dst[] is enough ram.
	if(head_len > sizeof(log_str)){
		return 0;
	}
	gateway_common_cmd_rsp(HCI_GATEWAY_CMD_LOG_BUF,p_data,len);
	return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_LOG_STRING,(u8 *)log_str,head_len);
}

#if DEBUG_CFG_CMD_GROUP_AK_EN
u8 comm_send_cnt = 0;
u16 comm_adr_dst = 0;
u32 comm_send_flag = 0;
u32 comm_send_tick = 0;

int mesh_tx_comm_cmd(u16 adr)
{
	comm_send_tick = clock_time();
	comm_send_flag = 0;
	u8 par[32] = {0};
	mesh_bulk_vd_cmd_par_t *p_bulk_vd_cmd = (mesh_bulk_vd_cmd_par_t *)par;
	p_bulk_vd_cmd->nk_idx = 0;
    p_bulk_vd_cmd->ak_idx = 0;
	p_bulk_vd_cmd->retry_cnt = g_reliable_retry_cnt_def;
	p_bulk_vd_cmd->rsp_max = 1;
	p_bulk_vd_cmd->adr_dst = adr;
	p_bulk_vd_cmd->op = VD_MESH_TRANS_TIME_GET;
	p_bulk_vd_cmd->vendor_id = g_vendor_id;
	p_bulk_vd_cmd->op_rsp = VD_MESH_TRANS_TIME_STS;
	p_bulk_vd_cmd->tid_pos = 0;
	u8 par_len = OFFSETOF(mesh_bulk_vd_cmd_par_t, par) + (p_bulk_vd_cmd->tid_pos?2:1);
	return mesh_bulk_cmd((mesh_bulk_cmd_par_t*)p_bulk_vd_cmd, par_len);
}

void mesh_ota_comm_test()
{
	int err =-1;
	if(comm_send_flag && comm_send_cnt>0){
		err = mesh_tx_comm_cmd(comm_adr_dst);
		comm_send_cnt--;
	}
}
#endif

u8 gateway_upload_extend_adv_option(u8 option_val)
{
    return gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_EXTEND_ADV_OPTION,(u8 *)&option_val,sizeof(option_val));
}

u8 gateway_cmd_from_host_ctl(u8 *p, u16 len )
{
	if(len<=0){
		return 0;
	}
	u8 op_code = p[0];
	if(op_code == HCI_GATEWAY_CMD_START){
		set_provision_stop_flag_act(0);
	}else if (op_code == HCI_GATEWAY_CMD_STOP){
		set_provision_stop_flag_act(1);
	}else if (op_code == HCI_GATEWAY_CMD_RESET){
        factory_reset();
        light_ev_with_sleep(4, 100*1000);	//10hz for about the 1s 
        start_reboot();
	}else if (op_code == HCI_GATEWAY_CMD_CLEAR_NODE_INFO){
		// clear the provision store  information 
		VC_cmd_clear_all_node_info(0xffff);// clear all node
	}else if (op_code == HCI_GATEWAY_CMD_SET_ADV_FILTER){
		set_gateway_adv_filter(p+1);
	}else if (op_code == HCI_GATEWAY_CMD_SET_PRO_PARA){
		// set provisioner net info para 
		provison_net_info_str *p_net;
		p_net = (provison_net_info_str *)(p+1);
		set_provisioner_para(p_net->net_work_key,p_net->key_index,
								p_net->flags,p_net->iv_index,p_net->unicast_address);
		// use the para (node_unprovision_flag) ,and the flag will be 0 
		
	}else if (op_code == HCI_GATEWAY_CMD_SET_NODE_PARA){
		// set the provisionee's netinfo para 
		provison_net_info_str *p_net;
		p_net = (provison_net_info_str *)(p+1);
		// set the pro_data infomation 
		set_provisionee_para(p_net->net_work_key,p_net->key_index,
								p_net->flags,p_net->iv_index,p_net->unicast_address);
		provision_mag.unicast_adr_last = p_net->unicast_address;
		set_gateway_provision_sts(1);

	}else if (op_code == HCI_GATEWAY_CMD_START_KEYBIND){
		extern u8 pro_dat[40];
		provison_net_info_str *p_str = (provison_net_info_str *)pro_dat;
		mesh_gw_appkey_bind_str *p_bind = (mesh_gw_appkey_bind_str *)(p+1);
		mesh_cfg_keybind_start_trigger_event(p_bind->key_idx,p_bind->key,
			p_str->unicast_address,p_str->key_index,p_bind->fastbind);
	}else if (op_code == HCI_GATEWAY_CMD_SET_DEV_KEY){
        mesh_gw_set_devkey_str *p_set_devkey = (mesh_gw_set_devkey_str *)(p+1);
        set_dev_key(p_set_devkey->dev_key);
        #if (DONGLE_PROVISION_EN)
			VC_node_dev_key_save(p_set_devkey->unicast,p_set_devkey->dev_key,2);
	    #endif
	}else if (op_code == HCI_GATEWAY_CMD_GET_SNO){
        gateway_upload_mesh_sno_val();
	}else if (op_code == HCI_GATEWAY_CMD_SET_SNO){
        u32 sno;
        memcpy((u8 *)&sno,p+1,4);
        mesh_adv_tx_cmd_sno = sno;
        mesh_misc_store();
	}else if (op_code == HCI_GATEWAY_CMD_GET_PRO_SELF_STS){
		gateway_upload_provision_self_sts(is_provision_success());
		gateway_upload_node_ele_cnt(g_ele_cnt);
	}else if (op_code == HCI_GATEWAY_CMD_STATIC_OOB_RSP){
		if(len-1>16){
			return 1;
		}
		mesh_set_pro_auth(p+1,len-1);
	}else if (op_code == HCI_GATEWAY_CMD_GET_UUID_MAC){
        // rsp the uuid part 
        gateway_upload_dev_uuid(prov_para.device_uuid,tbl_mac);
	}else if (op_code == HCI_GATEWAY_CMD_DEL_VC_NODE_INFO){
        u16 unicast = p[1]|(p[2]<<8);
        del_vc_node_info_by_unicast(unicast);
	}else if (op_code == HCI_GATEWAY_CMD_SEND_VC_NODE_INFO){
		VC_node_info_t *p_info = (VC_node_info_t *)(p+1);
		VC_node_dev_key_save(p_info->node_adr,p_info->dev_key,p_info->element_cnt);
		gateway_common_cmd_rsp(HCI_GATEWAY_CMD_SEND_VC_NODE_INFO, NULL, 0);
	}else if (op_code == HCI_GATEWAY_CMD_MESH_OTA_ADR_SEND){
		#if MD_MESH_OTA_EN
		mesh_fw_distibut_set(1);
		mesh_cmd_sig_fw_distribut_start(p+1,len-1, 0);
		#endif
	}
	#if MESH_RX_TEST
	else if (op_code == HCI_GATEWAY_CMD_MESH_RX_TEST) {
		u8 par[10];
		u8 *data = &p[1];
		memset(par,0x00,sizeof(par));
		u16 adr_dst = data[2] + (data[3]<<8);
		u8 rsp_max = data[4];	
		par[0] = data[6]&0x01;//on_off	
		u8 ack = data[5];
		u32 send_tick = clock_time();
		memcpy(par+4, &send_tick, 4);
		par[8] = data[6];// cur count
		u8 pkt_nums_send = data[7];
		par[3] = data[8];// pkt_nums_ack	
		u32 par_len = 37-(31-12)-6-4;
		if(data[7] > 1){// unseg:11  seg:8
			par_len = (37-(31-12)-4)*pkt_nums_send-6;
		}
		extern u16 mesh_rsp_rec_addr;
		mesh_rsp_rec_addr = data[9] + (data[10]<<8);
		SendOpParaDebug(adr_dst, rsp_max, ack ? G_ONOFF_SET : G_ONOFF_SET_NOACK, 
						   (u8 *)&par, par_len);
	}
	#endif
	#if DEBUG_CFG_CMD_GROUP_AK_EN
	else if (op_code == HCI_GATEWAY_CMD_MESH_COMMUNICATE_TEST){
	    comm_adr_dst = p[1]|(p[2]<<8);
		comm_send_cnt = p[3];
		comm_send_flag = 1;
		mesh_tx_comm_cmd(comm_adr_dst);
	}
	#endif
	else if (op_code == HCI_GATEWAY_CMD_SET_EXTEND_ADV_OPTION){
	    u8 option_val = p[1];
	    #if EXTENDED_ADV_ENABLE
	    g_gw_extend_adv_option = option_val;
	    #else
	    if(option_val){
	        LOG_MSG_ERR(TL_LOG_NODE_BASIC,0,0,"not support extend adv option:%d",option_val);
	    }
	    option_val = EXTEND_ADV_OPTION_NONE;
	    #endif
	    LOG_MSG_LIB(TL_LOG_NODE_BASIC,0,0,"set extend adv option:%d",option_val);
	    gateway_upload_extend_adv_option(option_val);
	}
	else if(op_code == HCI_GATEWAY_CMD_FAST_PROV_START ){
		u16 pid = p[1] + (p[2]<<8);
		mesh_fast_prov_start(pid);
	}
	return 1;
}

u8 gateway_cmd_from_host_ota(u8 *p, u16 len )
{
	rf_packet_att_data_t local_ota;
	u8 dat_len ;
	dat_len = p[0];
	local_ota.dma_len = dat_len+9;
	local_ota.type = 0;
	local_ota.rf_len = dat_len+7;
	local_ota.l2cap = dat_len+3;
	local_ota.chanid = 4;
	local_ota.att = 0;
	local_ota.hl = 0;
	local_ota.hh = 0;
	memcpy(local_ota.dat,p+1,dat_len);
	// enable ota flag 
	pair_login_ok = 1;
	u16 ota_adr =  local_ota.dat[0] | (local_ota.dat[1]<<8);
	if(ota_adr == CMD_OTA_START){
		u8 irq_en = irq_disable();
		//reserve for about the 70ms*48 = 3.4s
		for(int i=0;i<0x30;i++) //erase from end to head
		{  //4K/16 = 256
			flash_erase_sector(ota_program_offset + i*0x1000);
		}
		irq_restore(irq_en);
	}
	otaWrite((u8 *)&local_ota);
	return 1;
}

u8 gateway_cmd_from_host_mesh_ota(u8 *p, u16 len )
{
	u8 op_type =0;
	op_type = p[0];
	if(op_type == MESH_OTA_SET_TYPE){
		set_ota_reboot_flag(p[1]);
	}else if(op_type == MESH_OTA_ERASE_CTL){
		// need to erase the ota part 
		bls_ota_clearNewFwDataArea();
	}else{}
	return 1;
}
#endif


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
void main_loop ()
{
	static u32 tick_loop;

	tick_loop ++;
#if (BLT_SOFTWARE_TIMER_ENABLE)
	blt_soft_timer_process(MAINLOOP_ENTRY);
#endif
	mesh_loop_proc_prior();
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop ();


	////////////////////////////////////// UI entry /////////////////////////////////
	//  add spp UI task:
#if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(1);
#endif
	proc_ui();
	proc_led();
	factory_reset_cnt_check();
	
	mesh_loop_process();
#if ADC_ENABLE
	static u32 adc_check_time;
	if(clock_time_exceed(adc_check_time, 1000*1000)){
		adc_check_time = clock_time();
		static u16 T_adc_val;
	#if(MCU_CORE_TYPE == MCU_CORE_8269)     
		T_adc_val = adc_BatteryValueGet();
	#else
		T_adc_val = adc_sample_and_get_result();
	#endif
	}  
#endif

	#if (TESTCASE_FLAG_ENABLE && (!__PROJECT_MESH_PRO__))
	test_case_key_refresh_patch();
	#endif
}

void user_init()
{
    #if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(0); //battery check must do before OTA relative operation
    #endif
	enable_mesh_provision_buf();
	mesh_global_var_init();
	set_blc_hci_flag_fun(0);// disable the hci part of for the lib .
	proc_telink_mesh_to_sig_mesh();		// must at first
	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value and tp value

	usb_id_init();
	usb_log_init ();
	// need to have a simulate insert
	usb_dp_pullup_en (0);  //open USB enum
	gpio_set_func(GPIO_DP,AS_GPIO);
	gpio_set_output_en(GPIO_DP,1);
	gpio_write(GPIO_DP,0);
	sleep_us(20000);
	gpio_set_func(GPIO_DP,AS_USB);
	usb_dp_pullup_en (1);  //open USB enum

	////////////////// BLE stack initialization ////////////////////////////////////
	ble_mac_init();    

	//link layer initialization
	//bls_ll_init (tbl_mac);
#if(MCU_CORE_TYPE == MCU_CORE_8269)
	blc_ll_initBasicMCU(tbl_mac);   //mandatory
#elif((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(tbl_mac);				//mandatory
#endif

#if (EXTENDED_ADV_ENABLE)
	mesh_blc_ll_initExtendedAdv();
#endif
	blc_ll_initAdvertising_module(tbl_mac); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
#if BLE_REMOTE_PM_ENABLE
	blc_ll_initPowerManagement_module();        //pm module:      	 optional
#endif

	//l2cap initialization
	//blc_l2cap_register_handler (blc_l2cap_packet_receive);
	blc_l2cap_register_handler (app_l2cap_packet_receive);
	///////////////////// USER application initialization ///////////////////

#if EXTENDED_ADV_ENABLE
	/*u8 status = */mesh_blc_ll_setExtAdvParamAndEnable();
#endif
	u8 status = bls_ll_setAdvParam( ADV_INTERVAL_MIN, ADV_INTERVAL_MAX, \
			 	 	 	 	 	     ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, \
			 	 	 	 	 	     0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);

	if(status != BLE_SUCCESS){  //adv setting err
		write_reg8(0x8000, 0x11);  //debug
		while(1);
	}
	
	// normally use this settings 
	blc_ll_setAdvCustomedChannel (37, 38, 39);

	bls_ll_setAdvEnable(1);  //adv enable

	rf_set_power_level_index (MY_RF_POWER_INDEX);
	bls_pm_setSuspendMask (SUSPEND_DISABLE);//(SUSPEND_ADV | SUSPEND_CONN)
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_ADVERTISING_REPORT|HCI_LE_EVT_MASK_CONNECTION_COMPLETE);

	////////////////// SPP initialization ///////////////////////////////////
#if (HCI_ACCESS != HCI_USE_NONE)
	#if (HCI_ACCESS==HCI_USE_USB)
	//blt_set_bluetooth_version (BLUETOOTH_VER_4_2);
	//bls_ll_setAdvChannelMap (BLT_ENABLE_ADV_ALL);
	usb_bulk_drv_init (0);
	blc_register_hci_handler (app_hci_cmd_from_usb, blc_hci_tx_to_usb);
	#elif (HCI_ACCESS == HCI_USE_UART)	//uart
	uart_drv_init();
	blc_register_hci_handler (blc_rx_from_uart, blc_hci_tx_to_uart);		//default handler
	//blc_register_hci_handler(rx_from_uart_cb,tx_to_uart_cb);				//customized uart handler
	#endif
#endif
#if ADC_ENABLE
	adc_drv_init();
#endif
	rf_pa_init();
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, (blt_event_callback_t)&mesh_ble_connect_cb);
	blc_hci_registerControllerEventHandler(app_event_handler);		//register event callback
	//bls_hci_mod_setEventMask_cmd(0xffff);			//enable all 15 events,event list see ble_ll.h
	bls_set_advertise_prepare (app_advertise_prepare_handler);
	//bls_set_update_chn_cb(chn_conn_update_dispatch);	
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);

	app_enable_scan_all_device ();

	// mesh_mode and layer init
	mesh_init_all();
	// OTA init
	bls_ota_clearNewFwDataArea(); //must

	//blc_ll_initScanning_module(tbl_mac);
	#if((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	blc_gap_peripheral_init();    //gap initialization
	#endif
	
	mesh_scan_rsp_init();
	my_att_init (provision_mag.gatt_mode);
	blc_att_setServerDataPendingTime_upon_ClientCmd(10);
	extern u32 system_time_tick;
	system_time_tick = clock_time();
#if (BLT_SOFTWARE_TIMER_ENABLE)
	blt_soft_timer_init();
	//blt_soft_timer_add(&soft_timer_test0, 1*1000*1000);
#endif
}

