#include "telink_sdk_mible_api.h"
#include "proj_lib/ble/ll/ll_whitelist.h"
#if MI_API_ENABLE
#include "./certi/mijia_profiles/mi_service_server.h"
#include "./certi/common/mible_beacon.h"
#include "./certi/mesh_auth/mible_mesh_auth.h"
#include "common/mible_beacon_internal.h"
#include "mible_log.h"
#include "Mijia_pub_proc.h"
#include "mesh/blt_soft_timer.h"
MYFIFO_INIT(blt_gatt_event_buf, 0x30, 8);
telink_record_t telink_record;
// call back function ,will be call by the telink api 
u8 telink_ble_mi_app_event(u8 sub_code , u8 *p, int n)
{
	mible_gap_evt_t gap_evt_t = MIBLE_GAP_EVT_ADV_REPORT ;
	mible_gap_evt_param_t gap_para_t;
	gap_para_t.conn_handle =0;
	if(sub_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT){
		gap_evt_t = MIBLE_GAP_EVT_ADV_REPORT;
		mible_gap_adv_report_t *p_report = &(gap_para_t.report);
		event_adv_report_t *pa = (event_adv_report_t *)p;
		if(pa->adr_type){
			p_report->addr_type = MIBLE_ADDRESS_TYPE_RANDOM;
		}else{
			p_report->addr_type = MIBLE_ADDRESS_TYPE_PUBLIC;
		}
		if(pa->event_type == ADV_REPORT_EVENT_TYPE_NONCONN_IND){
			p_report->adv_type = ADV_DATA;
		}else if(pa->event_type == ADV_REPORT_EVENT_TYPE_SCAN_IND){
			p_report->adv_type = SCAN_RSP_DATA;
		}
		memcpy(p_report->peer_addr,pa->mac,sizeof(pa->mac));
		// memcpy adv data 
		p_report->data_len = pa->len;
		memcpy(p_report->data,pa->data,pa->len);
		// lost rssi;
	
	}else if(sub_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE){
		gap_evt_t = MIBLE_GAP_EVT_CONNECTED;
		mible_gap_connect_t *p_connect = &(gap_para_t.connect);
		event_connection_complete_t *pc = (event_connection_complete_t *)p;
		memcpy(p_connect->peer_addr,pc->mac,sizeof(pc->mac));
		if(pc->peer_adr_type){
			p_connect->type = MIBLE_ADDRESS_TYPE_RANDOM;
		}else{
			p_connect->type = MIBLE_ADDRESS_TYPE_PUBLIC;
		}
		if(pc->role == LL_ROLE_MASTER){
			p_connect->role = MIBLE_GAP_CENTRAL;
		}else if(pc->role == LL_ROLE_SLAVE){
			p_connect->role = MIBLE_GAP_PERIPHERAL;
		}
		p_connect->conn_param.min_conn_interval = pc->interval;
		p_connect->conn_param.max_conn_interval = pc->interval;
		p_connect->conn_param.slave_latency = pc->latency;
		p_connect->conn_param.conn_sup_timeout = pc->timeout;
	}else if (sub_code == HCI_EVT_DISCONNECTION_COMPLETE){
		gap_evt_t =  MIBLE_GAP_EVT_DISCONNECT;
		mible_gap_disconnect_t *p_dis = &(gap_para_t.disconnect);
		event_disconnection_t *pd = (event_disconnection_t *)p;
		if(pd->reason == HCI_ERR_CONN_TIMEOUT){
			p_dis->reason = CONNECTION_TIMEOUT;
		}else if (pd->reason == HCI_ERR_REMOTE_USER_TERM_CONN){
			p_dis->reason = REMOTE_USER_TERMINATED;
		}else if (pd->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){
			p_dis->reason = LOCAL_HOST_TERMINATED;
		}else{}
		telink_mible_ota_end();
	}else if (sub_code ==  HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE){
		gap_evt_t = MIBLE_GAP_EVT_CONN_PARAM_UPDATED;
		mible_gap_connect_update_t *p_up = &(gap_para_t.update_conn);
		event_connection_update_t *pu = (event_connection_update_t *)p;
		p_up->conn_param.conn_sup_timeout = pu->timeout;
		p_up->conn_param.max_conn_interval = pu->interval;
		p_up->conn_param.min_conn_interval = pu->interval;
		p_up->conn_param.slave_latency = pu->latency;
	}
	// set all the parameter part ,and call the function interface 
	mible_gap_event_callback(gap_evt_t,&gap_para_t);
	return 1;
}

// callback function ,will call by the telink part 
u8 telink_ble_mi_event_callback(u8 opcode,u8 *p)
{
	mible_gatts_evt_t evt = MIBLE_GATTS_EVT_WRITE;
	mible_gatts_evt_param_t evt_param_t;
	memset(&evt_param_t,0,sizeof(mible_gatts_evt_param_t));
	rf_packet_att_readBlob_t *p_r = (rf_packet_att_readBlob_t*)p;
	rf_packet_att_data_t *p_w = (rf_packet_att_data_t*)p;
	mible_gatts_write_t *p_write = &(evt_param_t.write);
	mible_gatts_read_t *p_read = &(evt_param_t.read);
	u16 handle_val=0;
	evt_param_t.conn_handle =0;
	if(opcode == ATT_OP_WRITE_REQ || opcode == ATT_OP_WRITE_CMD ){
		u8 mi_write_buf[64];
		handle_val = (p_w->hh<<8) + p_w->hl;
		if(*(gAttributes[handle_val].p_perm) & ATT_PERMISSIONS_AUTHOR_WRITE){
			evt = MIBLE_GATTS_EVT_WRITE_PERMIT_REQ;
		}else{
			evt = MIBLE_GATTS_EVT_WRITE;
		}
		p_write->value_handle = handle_val;
		p_write->offset = 0;// the offset of the buffer part 
		p_write->len = (p_w->l2cap)-3;
		p_write->data = mi_write_buf;
		memcpy((u8 *)(p_write->data+(p_write->offset)),p_w->dat,p_write->len);
	}else if (opcode == ATT_OP_READ_REQ){
		handle_val = (p_r->handle1 <<8)+ p_r->handle;
		if(*(gAttributes[handle_val].p_perm) & ATT_PERMISSIONS_READ){
			evt = MIBLE_GATTS_EVT_READ_PERMIT_REQ;
		}
		p_read->value_handle = handle_val;
	}else if (opcode == ATT_OP_HANDLE_VALUE_CFM){
		evt = MIBLE_GATTS_EVT_IND_CONFIRM;
	}else{
		return 0;
	}
	mible_gatts_event_callback(evt,&evt_param_t);	
	return 1;
}
int telink_ble_mi_event_cb_att(u16 conn, u8 * p)
{
	// push the data into the fifo part,and dispatch the data in the loop part
	rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;
	u8 opcode = req->opcode;
	conn++;
	if(opcode == ATT_OP_WRITE_REQ){
		my_fifo_push(&blt_gatt_event_buf,(u8*)req,10+req->l2capLen,0,0);
	}else {
		telink_ble_mi_event_callback(opcode,p);
	}
	return TRUE;
}
void telink_gatt_event_loop()
{
	my_fifo_buf_t *fifo = (my_fifo_buf_t *)my_fifo_get(&blt_gatt_event_buf);
	if(blt_state == BLS_LINK_STATE_CONN){
		if(fifo){
			rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)(fifo->data);
			u8 opcode = req->opcode;
			telink_ble_mi_event_callback(opcode,(u8 *)req);
			my_fifo_pop(&blt_gatt_event_buf);
		}
	}else{
		blt_gatt_event_buf.rptr = blt_gatt_event_buf.wptr;
	}
}

// callback function ,and will call by the telink part 
u8 telink_ble_mi_att_sync(u8 att_op,mible_gatts_srv_db_t *p)
{
	mible_arch_event_t evt = MIBLE_GATTS_EVT_WRITE;
	mible_arch_evt_param_t evt_param_t;
	mible_gatts_srv_db_t *p_gatts_db_ini = (evt_param_t.srv_init_cmp.p_gatts_db->p_srv_db);
	if(att_op == 0){
		evt_param_t.srv_init_cmp.status = MI_SUCCESS;
		evt_param_t.srv_init_cmp.p_gatts_db->srv_num =1;
		memcpy((u8 *)p_gatts_db_ini , (u8 *)p,sizeof(mible_gatts_srv_db_t));
		evt = MIBLE_ARCH_EVT_GATTS_SRV_INIT_CMP;
	}else if (att_op == 1){
		evt_param_t.record.status = MI_SUCCESS;
		evt_param_t.record.id = 1;
		evt = MIBLE_ARCH_EVT_RECORD_WRITE;
	}else{
		return 0;
	}
	mible_arch_event_callback(evt, &evt_param_t);
	return 1;
}

// user function ,and will call by the mi part 
mible_status_t telink_ble_mi_get_gap_address(u8 *p_mac)
{
	memcpy(p_mac,tbl_mac,sizeof(tbl_mac));	
	return MI_SUCCESS;
}
// user function ,and will call by the mi part
u8 gap_adv_param_valid(mible_gap_adv_param_t *p_param)
{
	u8 valid =1;
	if(p_param->adv_interval_min > p_param->adv_interval_max){
		valid =0;
	}else if(p_param->adv_interval_min<20 || p_param->adv_interval_min > 0x4000){
		valid =0;
	}else if(p_param->adv_interval_max<20 || p_param->adv_interval_max > 0x4000){
		valid =0;
	}else if(p_param->adv_type < MIBLE_ADV_TYPE_CONNECTABLE_UNDIRECTED || 
		p_param->adv_type > MIBLE_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED	){
		valid =0;
	}
	return valid;
}
mible_status_t telink_ble_mi_adv_start(mible_gap_adv_param_t *p_param)
{
	if(blt_state == BLS_LINK_STATE_CONN){
		return MI_ERR_INVALID_STATE;
	}else if (!gap_adv_param_valid(p_param)){
		return MI_ERR_INVALID_PARAM;
	}
	u8 status =0;
	u8 chn_mask=0;
	if(p_param->ch_mask.ch_37_off){
		chn_mask &=~(BIT(0));
	}else{
		chn_mask |= BIT(0);
	}
	if(p_param->ch_mask.ch_38_off){
		chn_mask &=~(BIT(1));
	}else{
		chn_mask |= BIT(1);
	}
	if(p_param->ch_mask.ch_39_off){
		chn_mask &=~(BIT(2));
	}else{
		chn_mask |= BIT(2);
	}
	status = bls_ll_setAdvParam( p_param->adv_interval_min, p_param->adv_interval_max, \
			 	 	 	 	 	     p_param->adv_type, OWN_ADDRESS_PUBLIC, \
			 	 	 	 	 	     0,  NULL,  chn_mask, ADV_FP_NONE);
	bls_ll_setAdvEnable(1);  //adv enable
	return MI_SUCCESS;
}
// user function ,and will call by the mi part 
rf_packet_adv_t mi_adv_packet;
void set_adv_mi_certify(rf_packet_adv_t * p,uint8_t const * p_data,
        uint8_t dlen) 
{
	p->header.type = LL_TYPE_ADV_IND;
	memcpy(p->advA,tbl_mac,6);
	memcpy(p->data, p_data, dlen);
	p->rf_len = 6 + dlen;
	p->dma_len = p->rf_len + 2;	
	return ;
}
void set_adv_mi_prehandler(rf_packet_adv_t *p)
{
	memcpy(p,&mi_adv_packet,sizeof(rf_packet_adv_t));
	return ;
}

mible_status_t telink_ble_mi_gap_adv_data_set(uint8_t const * p_data,
        uint8_t dlen, uint8_t const *p_sr_data, uint8_t srdlen)
{
	if (p_data == NULL || dlen>31 ){
		return MI_ERR_INVALID_PARAM;
	}
	//bls_ll_setAdvData((u8 *)p_data,dlen);
	set_adv_mi_certify(&mi_adv_packet,(u8*)p_data,dlen);
	// set the scanrsp for the provision part ;
	bls_ll_setScanRspData((u8 *)p_sr_data,srdlen);
	return MI_SUCCESS;
}
// user function ,and will call by the mi part 

mible_status_t telink_ble_mi_adv_stop()
{
	if(blt_state == BLS_LINK_STATE_CONN){
		return MI_ERR_INVALID_STATE;
	}
	bls_ll_setAdvEnable(0);
	return MI_SUCCESS;
}

u8 mi_update_conn_params_valid(mible_gap_conn_param_t conn_params)
{
	u8 valid =1;
	if(conn_params.max_conn_interval < conn_params.min_conn_interval){
		valid =0;
	}else if(conn_params.max_conn_interval > 0x0C80 || conn_params.max_conn_interval < 0x0006){
		valid =0;
	}else if(conn_params.min_conn_interval > 0x0C80 || conn_params.min_conn_interval < 0x0006){
		valid =0;
	}else if(conn_params.slave_latency > 0x01F3 || conn_params.slave_latency < 0){
		valid =0;
	}else if(conn_params.conn_sup_timeout > 0x0C80 || conn_params.conn_sup_timeout < 0x000A){
		valid =0;
	}
	return valid;
}

// user function ,and will call by the mi part 
mible_status_t telink_ble_mi_update_conn_params(uint16_t conn_handle,mible_gap_conn_param_t conn_params)
{
	if(blt_state != BLS_LINK_STATE_CONN){
		return MI_ERR_INVALID_STATE;
	}else if(!mi_update_conn_params_valid(conn_params)){
		return MI_ERR_INVALID_PARAM;
	}
	conn_handle =0;
	bls_l2cap_requestConnParamUpdate(conn_params.min_conn_interval,conn_params.max_conn_interval,
										conn_params.slave_latency,conn_params.conn_sup_timeout);
	return MI_SUCCESS;
}

// user function ,and will call by the mi api
u16 find_handle_by_uuid_char(u8 mode,u8 *p_uuid ,attribute_t *p_att)
{
	
	for(int i=0;i<p_att->attNum;i++){
		if(mode == MIBLE_UUID_16){
			if(!memcmp(p_uuid,p_att[i].uuid,2) && p_att[i].uuidLen == 2){
				return i;
			}
		}else if(mode == MIBLE_UUID_128){
			if(!memcmp(p_uuid,p_att[i].uuid,16) && p_att[i].uuidLen == 16){
				return i;
			}
		}
	}
	return 0;
}

u16 find_handle_by_uuid_service(u8 mode,u8 *p_uuid ,attribute_t *p_att)
{
	
	for(int i=0;i<p_att->attNum;i++){
		if(mode == MIBLE_UUID_16){
			if(!memcmp(p_uuid,p_att[i].pAttrValue,2) && p_att[i].attrLen == 2){
				return i;
			}
		}else if(mode == MIBLE_UUID_128){
			if(!memcmp(p_uuid,p_att[i].pAttrValue,16) && p_att[i].attrLen == 16){
				return i;
			}
		}
	}
	return 0;
}

mible_status_t telink_ble_mi_gatts_service_init(mible_gatts_db_t *p_server_db)
{
	mible_gatts_srv_db_t *p_srv = p_server_db->p_srv_db ;
	mible_gatts_char_db_t *p_char_db = p_srv->p_char_db ;
	mible_uuid_t *p_uuid_srv_str = &(p_srv->srv_uuid);
	mible_uuid_t *p_uuid_char_str = &(p_char_db->char_uuid);
	u16 handle;
	if(p_srv == NULL || p_char_db == NULL || p_uuid_srv_str == NULL){
		return MI_ERR_INVALID_ADDR;
	}
	
	handle = find_handle_by_uuid_service(p_uuid_srv_str->type,p_uuid_srv_str->uuid128,gAttributes);
	if(handle == 0){
		return MI_ERR_INVALID_PARAM;
	}
	p_srv->srv_handle = handle;
	for(int i=0;i<(p_srv->char_num);i++){
		p_uuid_char_str = &(p_char_db[i].char_uuid);
		handle = find_handle_by_uuid_char(p_uuid_char_str->type,p_uuid_char_str->uuid128,gAttributes);
		p_char_db[i].char_value_handle = handle;
		// set the permit part 
		
		if(p_char_db[i].wr_author ){
			*(gAttributes[handle].p_perm) |= (ATT_PERMISSIONS_AUTHOR_WRITE);
		}else{
			*(gAttributes[handle].p_perm) &= ~ ATT_PERMISSIONS_AUTHOR_WRITE;
		}
		if(p_char_db[i].rd_author ){
			*(gAttributes[handle].p_perm) |=(ATT_PERMISSIONS_AUTHOR_READ);
		}else{
			*(gAttributes[handle].p_perm) &=~ ATT_PERMISSIONS_AUTHOR_READ;
		}
		
	}
	return MI_SUCCESS;

}

// user function ,and wil call by the mi api
u8 get_uuid_is_character_or_not(u8 *p)
{
	u16 uuid=0;
	uuid = (p[1]<<8)+p[0];
	if(uuid == GATT_UUID_CHARACTER){
		return TRUE;
	}else{
		return FALSE;
	}
}

mible_status_t telink_ble_mi_gatts_value_set(uint16_t srv_handle, uint16_t value_handle,
    uint8_t offset, uint8_t* p_value,
    uint8_t len)
{
	attribute_t * p_att = gAttributes + value_handle;
	attribute_t * p_att_char = gAttributes + value_handle-1;
	srv_handle =0;
	if(p_value == NULL){
		return MI_ERR_INVALID_ADDR;
	}else if (len ==0 ||len > p_att->attrLen){
		return MI_ERR_INVALID_LENGTH;
	}else if (value_handle > gAttributes->attNum ){
		return MIBLE_ERR_ATT_INVALID_ATT_HANDLE;
	}else if (!get_uuid_is_character_or_not(p_att_char->uuid)){
		return MIBLE_ERR_GATT_INVALID_ATT_TYPE;
	}
	memcpy(p_att->pAttrValue,p_value,len);
	return MI_SUCCESS;
}

// user function ,and will call by the mi api 

mible_status_t telink_ble_mi_gatts_value_get(uint16_t srv_handle, uint16_t value_handle,
    uint8_t* p_value, uint8_t *p_len)
{
	attribute_t * p_att = gAttributes + value_handle;
	attribute_t * p_att_char = gAttributes + value_handle-1;
	srv_handle =0;
	if(p_value == NULL || p_value == NULL){
		return MI_ERR_INVALID_ADDR;
	}else if (value_handle > gAttributes->attNum ){
		return MIBLE_ERR_ATT_INVALID_ATT_HANDLE;
	}else if (!get_uuid_is_character_or_not(p_att_char->uuid)){
		return MIBLE_ERR_ATT_INVALID_ATT_HANDLE;
	}
	*p_len = p_att->attrLen;
	memcpy(p_value,p_att->pAttrValue,*p_len);
	return MI_SUCCESS;
}

// user function ,and will call by the mi api 

mible_status_t telink_ble_mi_gatts_notify_or_indicate(uint16_t conn_handle, uint16_t srv_handle,
    uint16_t char_value_handle, uint8_t offset, uint8_t* p_value,
    uint8_t len, uint8_t type)
{
	u8 notify_ret =0;
	conn_handle =0;
	srv_handle =0;
	offset =0;
	attribute_t * p_att_char = gAttributes + char_value_handle-1;
	offset =0;//??
	if(p_value == NULL){
		return MI_ERR_INVALID_ADDR;
	}else if (blt_state != BLS_LINK_STATE_CONN){
		return MI_ERR_INVALID_STATE;
	}else if (len ==0){
		return MI_ERR_INVALID_LENGTH;
	}else if (char_value_handle > gAttributes->attNum ){
		return MIBLE_ERR_ATT_INVALID_ATT_HANDLE;
	}else if (!get_uuid_is_character_or_not(p_att_char->uuid)){
		return MIBLE_ERR_GATT_INVALID_ATT_TYPE;
	}
	if(type == NOTIFY_CMD_CODE){// notification 
		if(blc_ll_getTxFifoNumber()+ 1 >= (blt_txfifo.num - 2 - 3 )){
			return MI_ERR_BUSY;
		}
		notify_ret = bls_att_pushNotifyData(char_value_handle,p_value,len);
	}else if (type ==INDICATION_CMD_CODE ){
		notify_ret = bls_att_pushIndicateData(char_value_handle,p_value,len);		
	}
	if(notify_ret == BLE_SUCCESS){
		return MI_SUCCESS;
	}else{
		return MI_ERR_BUSY;
	}
	
}

ev_time_event_t ev_evnt[EV_TIMER_MAX_CNT];

ev_time_event_t * find_empty_ev_event()
{
	ev_time_event_t *p_ev = NULL;
	for(int i=0;i<EV_TIMER_MAX_CNT;i++){
		if(ev_evnt[i].cb==NULL){
			p_ev = &(ev_evnt[i]);
			return p_ev;
		}
	}
	return p_ev;
}

ev_time_event_t * find_ev_event_by_cb(mible_timer_handler timeout_handler)
{
	ev_time_event_t *p_ev = NULL;
	for(int i=0;i<EV_TIMER_MAX_CNT;i++){
		if(ev_evnt[i].cb==timeout_handler){
			p_ev = &(ev_evnt[i]);
			return p_ev;
		}
	}
	return p_ev;
}

u8 find_timer_id_pointer(void *p_timer_id)
{
	for(int i=0;i<EV_TIMER_MAX_CNT;i++){
		if(&ev_evnt[i]== p_timer_id){
			return TRUE;
		}
	}
	return FALSE;
}

mible_status_t telink_mi_timer_create(void** p_timer_id,
    mible_timer_handler timeout_handler,
    mible_timer_mode mode)
{
	ev_time_event_t *p_ev = NULL;
	p_ev = find_ev_event_by_cb(timeout_handler);
	if(p_ev){// alredy exist 
		p_ev->interval = 0;
		p_ev->mode = mode;
		*p_timer_id = p_ev;
		return MI_SUCCESS;
	}
	p_ev = find_empty_ev_event();
	if(p_ev != NULL){
			p_ev->cb = timeout_handler;
			p_ev->interval = 0;
			p_ev->mode = mode;
			*p_timer_id = p_ev;
			return MI_SUCCESS;
	}else{
			return MI_ERR_NO_MEM;
		}
}

mible_status_t telink_mi_timer_delete(void *timer_id)
{
	// find the pointer , clear the content part first 
	if(find_timer_id_pointer(timer_id) != TRUE){
		return MI_ERR_INVALID_PARAM;
	}
	ev_unon_timer((ev_time_event_t *) timer_id);
	memset((u8 *)timer_id ,0,sizeof(ev_time_event_t));
	return MI_SUCCESS;
}

mible_status_t telink_mi_timer_start(void* timer_id, uint32_t timeout_value,
    void* p_context)
{
	// find the pointer , clear the content part first 
	if(find_timer_id_pointer(timer_id) != TRUE){
		return MI_ERR_INVALID_PARAM;
	}
	ev_time_event_t *p_ev = (ev_time_event_t *)timer_id;
	p_ev->data = p_context;
	ev_on_timer(p_ev,timeout_value);
	return MI_SUCCESS;
}

mible_status_t telink_mi_timer_stop(void* timer_id)
{
	// delete from the list table 
	if(find_timer_id_pointer(timer_id) != TRUE){
		return MI_ERR_INVALID_PARAM;
	}
	ev_unon_timer((ev_time_event_t *) timer_id);
	return MI_SUCCESS;
}

// record dispatch part 
u32 flash_idx_adr = FLASH_ADR_MI_RECORD;
u32 flash_idx_adr_cpy = FLASH_ADR_MI_RECORD_TMP;
void telink_record_part_init()
{
	
	uint8_t *p_buf = (u8 *)(&telink_record);
	u32 record_adr = FLASH_ADR_MI_RECORD;
	while(1){
		flash_read_page(record_adr,sizeof(telink_record_t),p_buf);
		if(telink_record.rec_id == 0xffff || record_adr >= FLASH_ADR_MI_RECORD_TMP){
			flash_idx_adr = record_adr;
			break;
		}
		record_adr += telink_record.len +3 ;
	}
}
u8 telink_write_flash(u32 *p_adr,u8 *p_buf,u8 len )
{
	u32 tmp_adr = *p_adr;
	flash_write_page(tmp_adr,len,p_buf);
	tmp_adr += len;
	*p_adr = tmp_adr;
	return TRUE;
}

u8 telink_record_clean_cpy()
{
	u32 record_adr = FLASH_ADR_MI_RECORD;
	u32 record_adr_cpy = FLASH_ADR_MI_RECORD_TMP;
	u32 cpy_cnt =0;
	uint8_t *p_buf = (u8 *)(&telink_record);
	flash_erase_sector(FLASH_ADR_MI_RECORD_TMP);
	while(1){//cpy the record to tmp first 
		flash_read_page(record_adr,sizeof(telink_record_t),p_buf);
		if(telink_record.rec_id == 0xffff || record_adr >= FLASH_ADR_MI_RECORD_MAX){
			break;
		}else if(telink_record.rec_id == 0){
			record_adr += telink_record.len +3 ;
		}else{
			// need to cpy
			u8 total_len =0;
			total_len = telink_record.len+3;
			while(total_len){
				if(total_len > sizeof(telink_record_t)){
					telink_write_flash(&record_adr_cpy,p_buf,sizeof(telink_record_t));
					total_len -= sizeof(telink_record_t);
					record_adr += sizeof(telink_record_t);
					flash_read_page(record_adr,sizeof(telink_record_t),p_buf);
				}else{
					telink_write_flash(&record_adr_cpy,p_buf,total_len);
					record_adr += total_len;
					total_len =0;
				}
			}
		}
	}
	flash_erase_sector(FLASH_ADR_MI_RECORD);
	cpy_cnt = record_adr_cpy - FLASH_ADR_MI_RECORD_TMP;
	flash_idx_adr = FLASH_ADR_MI_RECORD + cpy_cnt;// update the write adr part 
	u32 idx =0;
	while(cpy_cnt){
		if(cpy_cnt > sizeof(telink_record_t)){
			flash_read_page(FLASH_ADR_MI_RECORD_TMP+idx,sizeof(telink_record_t),p_buf);
			flash_write_page(FLASH_ADR_MI_RECORD+idx,sizeof(telink_record_t),p_buf);
			idx += sizeof(telink_record_t);
			cpy_cnt -= sizeof(telink_record_t);
		}else{
			flash_read_page(FLASH_ADR_MI_RECORD_TMP+idx,cpy_cnt,p_buf);
			flash_write_page(FLASH_ADR_MI_RECORD+idx,cpy_cnt,p_buf);
			cpy_cnt = 0;
		}
	}	
	return TRUE;
}

mible_status_t telink_record_create(uint16_t record_id, uint8_t len)
{
/*
	uint8_t total_len = len;
	uint8_t buf_idx =0;
	uint8_t *p_buf = (u8 *)(&telink_record);
	if(len > RECORD_MAX_LEN || len == 0){
		return MI_ERR_INVALID_LENGTH;
	}
	mible_status_t err_sts = MI_SUCCESS;
	// if find the record part ,clear the id part ,else return mi_suc;
	u32 record_adr =0;
	if(flash_idx_adr + (len+3)+RECORD_RESERVE_SPACE > FLASH_ADR_MI_RECORD_TMP){
		// need to clean the flash first .
		telink_record_clean_cpy();
		err_sts = MI_ERR_NO_MEM;
	}
	if(find_record_adr(record_id,&record_adr) == TRUE){
		return MI_SUCCESS;
	}else{
		telink_record_delete(record_id);
	}
	memset(p_buf,0,sizeof(telink_record_t));
	telink_record.rec_id = record_id;
	telink_record.len = total_len;
	//write the header part 
	if(total_len > sizeof(telink_record.dat)){
		telink_write_flash(&flash_idx_adr,p_buf,sizeof(telink_record_t));
		total_len -= sizeof(telink_record.dat);
		buf_idx += sizeof(telink_record.dat);
	}else{
		telink_write_flash(&flash_idx_adr,p_buf,total_len+3);
		total_len = 0;
	}
	//the continus packet 
	while(total_len >0){
		memset(p_buf,0,sizeof(telink_record_t));
		if(total_len>sizeof(telink_record_t)){
			telink_write_flash(&flash_idx_adr,p_buf,sizeof(telink_record_t));
			total_len -= sizeof(telink_record_t);
			buf_idx += sizeof(telink_record_t);
		}else{
			telink_write_flash(&flash_idx_adr,p_buf,total_len);
			total_len =0;
			buf_idx =0;
		}
	}
	return err_sts;
*/
	return MI_SUCCESS;
}


mible_status_t telink_record_delete(uint16_t record_id)
{
	u32 record_adr =0;
	uint8_t *p_buf = (u8 *)(&telink_record);
	if(!find_record_adr(record_id,&record_adr)){
		telink_record_eve_cb(record_id,MI_ERR_INVALID_PARAM,MIBLE_ARCH_EVT_RECORD_DELETE);
		return MI_ERR_INVALID_PARAM;
	}
	//flash_read_page(record_adr,sizeof(telink_record_t),p_buf);
	telink_record.rec_id =0;
	// set the record id to 0 
	flash_write_page(record_adr,2,p_buf);//just clear the rec id part 
	telink_record_eve_cb(record_id,MI_SUCCESS,MIBLE_ARCH_EVT_RECORD_DELETE);
	return MI_SUCCESS;
}
uint8_t buf_is_empty_or_not(uint8_t* p_data,uint8_t len)
{
	for(int i=0;i<len;i++){
		if(p_data[i] != 0){
			return 0;
		}
	}
	return 1;
}
mible_status_t telink_record_read(uint16_t record_id, uint8_t* p_data,uint8_t len)
{
	u32 record_adr =0;
	uint8_t *p_buf = (u8 *)(&telink_record);
	if(len > RECORD_MAX_LEN ||len ==0  ){
		return  MI_ERR_INVALID_LENGTH;
	}else if(p_data == NULL){
		return MI_ERR_INVALID_ADDR;
	}else if(!find_record_adr(record_id,&record_adr)){
		return MI_ERR_INVALID_PARAM;
	}
	flash_read_page(record_adr,sizeof(telink_record_t),p_buf); 
	if(len>telink_record.len){
		return MI_ERR_INVALID_LENGTH;
	}
	if(len<=sizeof(telink_record.dat)){
		memcpy(p_data,telink_record.dat,len);
		/*
		if(buf_is_empty_or_not(p_data,len)){
			return MIBLE_ERR_UNKNOWN;
		}*/
		return MI_SUCCESS;
	}else{
		// directly read all the buf part 
		flash_read_page(record_adr+3,len ,p_data);
		/*
		if(buf_is_empty_or_not(p_data,len)){
			return MIBLE_ERR_UNKNOWN;
		}*/
		return MI_SUCCESS;
	}
}
void telink_record_eve_cb(uint16_t record_id,mible_status_t sts,mible_arch_event_t eve)
{
	mible_arch_evt_param_t param;
	param.record.status =sts;
	param.record.id = record_id;
	mible_arch_event_callback(eve, &param);
	return ;
}
mible_status_t telink_record_write(uint16_t record_id, uint8_t* p_data,uint8_t len)
{
	uint8_t total_len = len;
	uint8_t buf_idx =0;
	uint8_t *p_buf = (u8 *)(&telink_record);
	if(len > RECORD_MAX_LEN || len == 0){
		telink_record_eve_cb(record_id,MI_ERR_INVALID_LENGTH,MIBLE_ARCH_EVT_RECORD_WRITE);
		return MI_ERR_INVALID_LENGTH;
	}else if((u32)p_data%4 != 0){
		telink_record_eve_cb(record_id,MI_ERR_INVALID_PARAM,MIBLE_ARCH_EVT_RECORD_WRITE);
		return MI_ERR_INVALID_PARAM;
	}
	// check the first record 
	telink_record_delete(record_id);
	if(flash_idx_adr + (len+3)+RECORD_RESERVE_SPACE > FLASH_ADR_MI_RECORD_TMP){
		// need to clean the flash first .
		telink_record_clean_cpy();
	}
	// write part 
	memset(p_buf,0,sizeof(telink_record_t));
	telink_record.rec_id = record_id;
	telink_record.len = total_len;
	// write the header part 
	if(total_len > sizeof(telink_record.dat)){
		memcpy(telink_record.dat,p_data,sizeof(telink_record.dat));
		telink_write_flash(&flash_idx_adr,p_buf,sizeof(telink_record_t));
		total_len -= sizeof(telink_record.dat);
		buf_idx += sizeof(telink_record.dat);
	}else{
		memcpy(telink_record.dat,p_data,total_len);
		telink_write_flash(&flash_idx_adr,p_buf,total_len+3);
		total_len = 0;
	}
	// write the continus part 
	while(total_len >0){
		memset(p_buf,0,sizeof(telink_record_t));
		if(total_len>sizeof(telink_record_t)){
			memcpy(p_buf,p_data+buf_idx,sizeof(telink_record_t));
			telink_write_flash(&flash_idx_adr,p_buf,sizeof(telink_record_t));
			total_len -= sizeof(telink_record_t);
			buf_idx += sizeof(telink_record_t);
		}else{
			memcpy(p_buf,p_data+buf_idx,total_len);
			telink_write_flash(&flash_idx_adr,p_buf,total_len);
			total_len =0;
			buf_idx =0;
		}
	}
	telink_record_eve_cb(record_id,MI_SUCCESS,MIBLE_ARCH_EVT_RECORD_WRITE);
	return MI_SUCCESS;
}

uint8_t find_record_adr(uint16_t record_id,u32 *p_adr)
{
	uint32_t record_adr = FLASH_ADR_MI_RECORD;
	uint8_t *p_buf = (u8 *)(&telink_record);
	while(1){
		flash_read_page(record_adr,sizeof(telink_record_t),p_buf);
		if(telink_record.rec_id == 0xffff || record_adr >= FLASH_ADR_MI_RECORD_TMP){
			return FALSE;
		}
		if(telink_record.rec_id == record_id){
			*p_adr = record_adr;
			return TRUE;
		}else{
			record_adr += telink_record.len + 3 ;
		}
	}
	return FALSE;
}



// user function ,and will call by the mi part 
mible_status_t telink_ble_mi_aes128_encrypt(const uint8_t* key,
    const uint8_t* plaintext, uint8_t plen,
    uint8_t* ciphertext)
{
	if(key==NULL || plaintext ==NULL || ciphertext== NULL){
		return MI_ERR_INVALID_ADDR;
	}
	#if 0
	int plen_aes =0;
	plen_aes = plen;
	tn_aes_cmac ( (u8 *)key, (u8 *)plaintext, plen_aes,ciphertext );
	#else
	// raw data aes part 
	u8 text_dat[16];
	memset(text_dat,0,sizeof(text_dat));
	memcpy(text_dat,plaintext,plen);
	tn_aes_128((u8*)key, text_dat, ciphertext);
	#endif 
	return MI_SUCCESS;
}


// task poll part 
MYFIFO_INIT(mi_task_api_func_t, 16, 8);
/*
 * @brief   Post a task to a task quene, which can be executed in a right place(maybe a task in RTOS or while(1) in the main function).
 * @param   [in] handler: a pointer to function 
 *          [in] param: function parameters 
 * @return  MI_SUCCESS              Successfully put the handler to quene.
 *          MI_ERR_NO_MEM           The task quene is full. 
 *          MI_ERR_INVALID_PARAM    Handler is NULL
 * */

mible_status_t telink_mi_task_post(mible_handler_t handler, void *arg)
{
	mi_task_fun_t task_fun;
	task_fun.cb = handler;
	task_fun.arg = arg;
	if(handler == NULL){
		return MI_ERR_INVALID_PARAM;
	}
	if(my_fifo_push(&mi_task_api_func_t,(u8*)(&task_fun),sizeof(mi_task_fun_t),0,0)==0){
		return MI_ERR_NO_MEM;
	}else{
		return MI_SUCCESS;
	}
}
mible_status_t telink_mi_task_exec()
{
	// pull the function pointer part 
	mi_task_fun_t task_fun;
	u8 *p_func;
	p_func = my_fifo_get(&mi_task_api_func_t);
	if(p_func!=NULL){
		my_fifo_pop(&mi_task_api_func_t);
		memcpy((u8 *)(&task_fun),p_func+2,sizeof(mi_task_fun_t));
		task_fun.cb(task_fun.arg);
	}
	return MI_SUCCESS;
}
void telink_mible_ota_start()
{}

void telink_mible_ota_end()
{}

void telink_mible_nvm_write(void * p_data, uint32_t length, uint32_t address)
{
	if(address == ota_program_offset){// means ota start 
		telink_mible_ota_start();
	}
	flash_write_page(address,length,(u8 *)p_data);
}

void telink_mible_upgrade_firmware(void)
{
	ota_set_flag ();// set flag and 
	start_reboot();
}


// xiaomi model set 
void xiaomi_publish_set_model(u32 model_id,u8 ele_idx,u8 sig_model)
{
	mesh_cfg_model_pub_set_t pub_set;
	pub_set.ele_adr = ele_adr_primary + ele_idx;
	pub_set.pub_adr = MI_MESH_PUB_ADR;
	pub_set.model_id = model_id;
	mesh_app_key_t *p_appkey = &(mesh_key.net_key[0][0].app_key[0]);
	pub_set.pub_par.appkey_idx = p_appkey->index;
	pub_set.pub_par.credential_flag =0;
	pub_set.pub_par.ttl = TTL_DEFAULT;
	pub_set.pub_par.period.steps = MI_MESH_PUB_STEP;// 60s interval 
	pub_set.pub_par.period.res = MI_MESH_PUB_VAL;//step is 10s 
	pub_set.pub_par.transmit.val = TRANSMIT_CNT_DEF|(TRANSMIT_INVL_STEPS_DEF<<3);
	u32 par_len =(sig_model)?sizeof(mesh_cfg_model_pub_set_t)-2:sizeof(mesh_cfg_model_pub_set_t);
	mesh_tx_cmd2normal_primary(CFG_MODEL_PUB_SET, (u8 *)&pub_set, par_len, ele_adr_primary, 0);
}

void xiaomi_light_sub_set_model(u32 model_id,u8 ele_idx,u8 sig_model)
{
	mesh_cfg_model_sub_set_t sub_set;
	sub_set.ele_adr = ele_adr_primary + ele_idx;
	sub_set.model_id = model_id;
	sub_set.sub_adr = MI_MESH_SUB_ADR;
	u32 par_len =(sig_model)?sizeof(mesh_cfg_model_sub_set_t)-2:sizeof(mesh_cfg_model_sub_set_t);
	mesh_tx_cmd2normal_primary(CFG_MODEL_SUB_ADD, (u8 *)&sub_set, par_len, ele_adr_primary, 0);
}

void xiaomi_publish_set_proc_onff(u8 ele_cnt)
{
	for(int i=0;i<ele_cnt;i++){
		xiaomi_publish_set_model(SIG_MD_G_ONOFF_S,i,1);
	}
	return ;
}

void xiaomi_publish_set_proc_ct()
{
	xiaomi_publish_set_model(SIG_MD_G_ONOFF_S,0,1);
	xiaomi_publish_set_model(SIG_MD_LIGHTNESS_S,0,1);
	xiaomi_publish_set_model(SIG_MD_LIGHT_CTL_TEMP_S,1,1);
}

void xiaomi_publish_set_proc_lamp()
{
	xiaomi_publish_set_model(SIG_MD_G_ONOFF_S,0,1);
	xiaomi_publish_set_model(SIG_MD_LIGHTNESS_S,0,1);
}

void xiaomi_publish_set_proc()
{
#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)
	xiaomi_publish_set_proc_ct();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
	xiaomi_publish_set_proc_lamp();
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF)
	xiaomi_publish_set_proc_onff(1);
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF)
	xiaomi_publish_set_proc_onff(2);
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
	xiaomi_publish_set_proc_onff(3);
#endif	
}


// set the button part 
void mi_testboard_init()
{
	gpio_set_func(GPIO_PD2,AS_GPIO);
	gpio_set_input_en(GPIO_PD2,1);// wait to detect
	gpio_setup_up_down_resistor(GPIO_PD2,PM_PIN_PULLUP_1M);
	// line on the led part 
	gpio_set_func(GPIO_PC4,AS_GPIO);
	gpio_set_input_en(GPIO_PC4,1);
	gpio_set_output_en(GPIO_PC4,1);
	gpio_write(GPIO_PC4,0);
}
void mi_dectect_reset_proc()
{
	// demo code to read the io part 
	if(!gpio_read(GPIO_PD2)){
		// reset status 
		kick_out();
	}else{
	}	
}
void mi_dispatch_led_status()
{
	if(is_provision_success()){
		gpio_write(GPIO_PC4,1);//turn off the yellow led
	}else{
		gpio_write(GPIO_PC4,0);// turn on the yellow led 
	}
}
#define MI_TEST_MODE_EN	1
u8 mi_api_loop_run()
{
	mi_dispatch_led_status();
	return TRUE;
}
#if XIAOMI_TEST_CODE_ENABLE
#define MI_RANDOM_DELAY_MASK 0x1FFFFF // it's about 2.097s
u8 mi_test_random_delay()
{
	u32 rand_us =0;
	rand_us = (rand() & 0xffff);
	rand_us |=(rand()<<16);
	rand_us &= MI_RANDOM_DELAY_MASK;
	sleep_us_clear_dog(rand_us);  // be care to watch dog reboot
	return 1;
}
#endif


#if MI_SWITCH_LPN_EN
#define MI_SLEEP_INTERVAL 	1140
#define MI_RUN_INTERVAL		60

#define MI_MESH_RUN_MODE 	0
#define MI_MESH_SLEEP_MODE 	1
typedef struct{
    u32 last_tick;
	u32 run_ms;
	u32 sleep_ms;
	u32 mode;
}mi_mesh_sleep_pre_t;
extern void blt_adv_expect_time_refresh(u8 en);
void mi_mesh_blt_send_adv2_scan_mode()
{
	#if MI_SWITCH_LPN_EN
	blt_adv_expect_time_refresh(0);
	blt_send_adv2scan_mode(1);
	blt_adv_expect_time_refresh(1);
	#endif
}

mi_mesh_sleep_pre_t	mi_mesh_sleep_time={0, MI_RUN_INTERVAL,MI_SLEEP_INTERVAL,MI_MESH_RUN_MODE};

int soft_timer_proc_mi_beacon(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;
	static u32 mi_beacon_cnt ;
	mi_beacon_cnt++;
	mi_mesh_blt_send_adv2_scan_mode();
	if(mi_beacon_cnt == 2){
		mi_beacon_cnt =0;
		return -1;
	}else{
		return 0;
	}
}

void mi_mesh_sleep_init()
{
	mi_mesh_sleep_time.last_tick = clock_time()|1;
	mi_mesh_sleep_time.mode = MI_MESH_RUN_MODE;
	blt_soft_timer_add(&soft_timer_proc_mi_beacon, 20*1000);
}

u8 mi_mesh_sleep_time_exceed_adv_iner()
{
	if(clock_time_exceed(mi_mesh_sleep_time.last_tick,(MI_RUN_INTERVAL+MI_SLEEP_INTERVAL)*1000)){//1.26s
		return 1;
	}else{
		return 0;
	}
}

int soft_timer_proc_mesh_cmd(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;
	mi_mesh_blt_send_adv2_scan_mode();
	if(my_fifo_data_cnt_get(&mesh_adv_cmd_fifo) == 0){
		return -1;
	}else{
		return 0;
	}
}

void mi_mesh_lowpower_loop()
{
	if(blt_state == BLS_LINK_STATE_CONN){ // in the ble connection mode ,it will not trigger the deep mode ,and stop the mesh adv sending part 
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
		blt_soft_timer_delete(&soft_timer_proc_mesh_cmd);
	}else if (blt_state == BLS_LINK_STATE_ADV){
		if( clock_time_exceed(mi_mesh_sleep_time.last_tick,mi_mesh_sleep_time.run_ms *1000) &&
			mi_mesh_sleep_time.mode == MI_MESH_RUN_MODE){
			mi_mesh_sleep_time.mode = MI_MESH_SLEEP_MODE;
			if(my_fifo_data_cnt_get(&mesh_adv_cmd_fifo) > 0){
				blt_soft_timer_add(&soft_timer_proc_mesh_cmd, 50*1000);
			}
			bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		}
	}
}
#endif




u8 test_mi_api_part()
{
#if XIAOMI_TEST_CODE_ENABLE
	mi_test_random_delay();
#endif
	mi_testboard_init();
	sleep_us(20);// wait for the io stable 
	mi_dectect_reset_proc();
	return 0;
}

static void advertising_start(void)
{
    mible_gap_adv_param_t adv_param = (mible_gap_adv_param_t ) {
                .adv_type = MIBLE_ADV_TYPE_CONNECTABLE_UNDIRECTED,
                .adv_interval_min = ADV_INTERVAL_MIN,
        		.adv_interval_max = ADV_INTERVAL_MAX,
    };
	adv_param.ch_mask.ch_37_off =0;
	adv_param.ch_mask.ch_38_off =0;
	adv_param.ch_mask.ch_39_off =0;
    if (MI_SUCCESS != mible_gap_adv_start(&adv_param)) {
        MI_LOG_ERROR("adv failed. \r\n");
    }
}
#define MI_REG_STS_RECORD_ID 	0x0180
void advertise_init()
{
	uint8_t sts_rec[1];
	if(mible_record_read(MI_REG_STS_RECORD_ID,sts_rec,sizeof(sts_rec))!= MI_SUCCESS){
		sts_rec[0] = mi_mesh_unreg;
		mible_record_create(MI_REG_STS_RECORD_ID,1);
		mible_record_write(MI_REG_STS_RECORD_ID,sts_rec,sizeof(sts_rec));
	}
	if(is_provision_success()){
		mibeacon_adv_data_set(0, MI_PROVED_STATE, 0, NULL);
	}else{
		mibeacon_adv_data_set(0, MI_UNPROV_STATE, 0, NULL);
	}
	advertising_start();
}

static u32 A_debug_schd_cnt =0;
u8 mi_schd_event_fail(u8 code)
{
    if( code == SCHD_EVT_REG_FAILED ||
        code == SCHD_EVT_ADMIN_LOGIN_FAILED ||
        code == SCHD_EVT_SHARE_LOGIN_FAILED ||
        code == SCHD_EVT_TIMEOUT||
        code == SCHD_EVT_KEY_NOT_FOUND||
        code == SCHD_EVT_KEY_DEL_FAIL){
            return 1;

    }else{
        return 0;
    }
	return 0;
}

void mi_schd_event_handler(schd_evt_t* p_evt_id)
{
    MI_LOG_INFO("USER CUSTOM CALLBACK RECV EVT ID %d\n", p_evt_id->id);
    if (p_evt_id->id == SCHD_EVT_MESH_REG_SUCCESS ) {
		A_debug_schd_cnt++;
		
		uint8_t sts_rec[1];
        sts_rec[0] = mi_mesh_avail;
		mible_record_write(MI_REG_STS_RECORD_ID,sts_rec,sizeof(sts_rec));
        //advertising_config((mi_mesh_stat_t)sts_rec[0]);
		mibeacon_adv_data_set(0, MI_PROVED_STATE, 0, NULL);
		u16 primary_adr =0;
		
        MI_LOG_INFO("starting unprovisioned beaconing...\r\n");
		mesh_config_t *p_cfg = &(p_evt_id->data.mesh_config);
		//dev key first 
		set_dev_key((u8 *)p_cfg->p_devkey);
		// provision self 
		provison_net_info_str *p_net =(provison_net_info_str *)p_cfg->p_prov_data;
		primary_adr = p_net->unicast_address;
		prov_para.provison_rcv_state = STATE_PRO_SUC;
		endianness_swap_u32(p_net->iv_index);
#if DUAL_VENDOR_EN
		provision_mag.dual_vendor_st = DUAL_VENDOR_ST_MI;
#endif
		mesh_provision_par_handle((u8 *)p_net);
		mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_SUC);
		//add appkey part ,only support two appkeys
		appkey_item_t *p_app_item = p_cfg->p_appkey_list->head;
		u8 app_sts =0;
		app_sts = mesh_app_key_set(APPKEY_ADD, p_app_item->appkey,
						p_app_item->app_idx,p_app_item->net_idx, 1);
		// bind model part 
		model_bind_list_t *p_bind_list = (p_cfg->p_bind_list); 
		for(u32 i=0;i<p_bind_list->size;i++){
			bind_item_t *p_bind_it = &(p_bind_list->head[i]);
			int sig_model =0;
			u32 model_id=0;
			if(p_bind_it->vendor == 0){
				sig_model = 1;
				model_id = p_bind_it->model;
			}else{
				sig_model = 0;
				model_id = (p_bind_it->vendor)|(p_bind_it->model <<16);
			}
			mesh_appkey_bind(MODE_APP_BIND, primary_adr+p_bind_it->elem_idx,model_id, 
											sig_model, p_bind_it->appkey_idx);
			// sub set all the models 
			xiaomi_light_sub_set_model(model_id,p_bind_it->elem_idx,sig_model);	
		}
		// pub all the sts on time .
		xiaomi_publish_set_proc();
		mi_pub_para_init();
		mi_pub_send_all_status();
    }else if (mi_schd_event_fail(p_evt_id->id)){
        mesh_node_prov_event_callback(EVENT_MESH_NODE_RC_LINK_TIMEOUT);
    }
}
#endif 
// user function ,and will call by the mi part 
mible_status_t telink_ble_mi_rand_num_generator(uint8_t* p_buf, uint8_t len)
{
	if(len > RAND_NUM_MAX_LEN){
		return MI_ERR_NO_MEM;
	}
	for(int i=0;i<len;i++){
		p_buf[i]=rand()&0xff;
	}
	return MI_SUCCESS;
}


