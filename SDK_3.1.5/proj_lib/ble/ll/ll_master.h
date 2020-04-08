/********************************************************************************************************
 * @file     ll_master.h 
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

#ifndef LL_MASTER_H_
#define LL_MASTER_H_



#define			BLE_DATA_CHANNEL_EN				1
#define			SYS_LINK_ADV_INTERVAL			500000
#define			BLE_MASTER_CONNECTION_REQ		1
#define			BLM_CONN_HANDLE_CANCEL			BIT(8)
#define			BLM_CONN_MASTER_TERMINATE		BIT(4)
#define			BLM_CONN_SLAVE_TERMINATE		BIT(5)
#define			BLM_CONN_TERMINATE_SEND			BIT(0)

#define			BLM_CONN_ENC_CHANGE				BIT(9)
#define			BLM_CONN_ENC_REFRESH			BIT(10)
#define			BLM_CONN_ENC_REFRESH_T			BIT(11)



#define  BLM_WINSIZE			4
#define  BLM_WINOFFSET			5

#define  BLM_MID_WINSIZE		2500


#define CONN_REQ_WAIT_ACK_NUM				6



#if (LL_MASTER_MULTI_CONNECTION)

#define			BLM_TX_FIFO_NUM				4
#define			BLM_TX_FIFO_SIZE			40



typedef struct {
	u32		tx_fifo[BLM_TX_FIFO_NUM][BLM_TX_FIFO_SIZE>>2];
	u8		tx_wptr;
	u8		tx_rptr;
	u8		tx_num;
	u8		chn_idx;
	u8		chn_tbl[40];

	u8 		newRx;
	u8 		ll_remoteFeature; //not only one for BLE master, use connHandle to identify
	u8		remoteFeatureReq;
	u8 		adv_filterPolicy;

	u8		macAddress_public[6];
	u8		macAddress_random[6];   //host may set this

	u8		connState;
	u8		peer_adr_type;
	u8		peer_adr[6];

	u32		conn_access_code;
	u8		conn_sn;
	u8		conn_snnesn;
	u8		conn_chn;
	u8		conn_update;

//	u16		connHandle;
	u8 		smp_busy;
	u8 		host_pkt_hold;
	u16		rsvd3;

	u32		conn_receive_packet;
	u32		conn_missing;
	u32		conn_timeout;
	u32		conn_tick;
	u32		conn_interval;
	u16		conn_inst;
	u16		conn_latency;

	u32		conn_winsize_next;
	u32		conn_timeout_next;
	u32		conn_offset_next;
	u32		conn_interval_next;
	u16		conn_inst_next;
	u16		conn_latency_next;

	u8		conn_chn_hop;
	u8		conn_chn_map[5];
	u8		conn_chn_map_next[5];
	u8		connParaUpReq_pending;

	u32		conn_Req_noAck_timeout;
	u8		conn_Req_waitAck_enable;
	u8		conn_terminate_reason;
	u8		conn_terminate_pending;   // terminate_pending = master_terminate || slave_terminate
	u8		remote_version;

	u32		slot_idx;
	u16		slot_latency;		//8 (40ms) typical; 2 (10ms) for high throughput connection
	u16		slot_num;

	u16		slot_latency_next;
	u16		enc_ediv;

	u32		enc_ivs;
	u8		enc_random[8];
	u8 		enc_skds[8];
	ble_crypt_para_t	crypt;

} st_ll_conn_master_t;


ble_sts_t blm_ll_connectWhiteList (int en);
ble_sts_t blm_ll_readRemoteVersion (u16 handle);
ble_sts_t blm_ll_setScanEnable (u8 scan_enable, u8 filter_duplicate);
ble_sts_t blm_ll_readRemoteFeature (u16 handle);
ble_sts_t blm_ll_readChannelMap (u16 handle, u8 * map);
ble_sts_t blm_ll_setHostChannel (u16 handle, u8 * map);



ble_sts_t blm_ll_connectDevice (u8 adr_type, u8 *mac, u16 conn_min, u16 conn_max, u16 conn_latency, u16 timeout);
ble_sts_t blm_ll_createConnection (u16 scan_interval, u16 scan_window, u8 initiator_filter_policy,
							  u8 adr_type, u8 *mac, u8 own_adr_type,
							  u16 conn_min, u16 conn_max, u16 conn_latency, u16 timeout,
							  u16 ce_min, u16 ce_max );

ble_sts_t blm_ll_createConnectionCancel ();







#else


#define			BLM_TX_FIFO_NUM				8
#define 		STACK_FIFO_NUM				2  //user 6, stack 2

#define			BLM_TX_FIFO_SIZE			40

typedef struct {
	u32		tx_fifo[BLM_TX_FIFO_NUM][BLM_TX_FIFO_SIZE>>2];
	u8		tx_wptr;
	u8		tx_rptr;
	u8		tx_num;
	u8		chn_idx;
	u8		chn_tbl[40];

	u8 		newRx;
	u8 		ll_remoteFeature; //not only one for BLE master, use connHandle to identify
	u8		remoteFeatureReq;
	u8 		adv_filterPolicy;

	u8		macAddress_public[6];
	u8		macAddress_random[6];   //host may set this

	u8		rsvd;
	u8		peer_adr_type;
	u8		peer_adr[6];

	u32		conn_access_code;
	u8		conn_sn;
	u8		conn_snnesn;
	u8		conn_chn;
	u8		conn_update;

	u16		connHandle;
	u8 		conn_btx_not_working;
	u8		conn_rcvd_slave_pkt;

	u32		conn_receive_packet;
	u32		conn_missing;
	u32		conn_timeout;
	u32		conn_tick;
	u32		conn_interval;
	u16		conn_inst;
	u16		conn_latency;

	u32		conn_winsize_next;
	u32		conn_timeout_next;
	u32		conn_offset_next;
	u32		conn_interval_next;
	u16		conn_inst_next;
	u16		conn_latency_next;

	u8		conn_chn_hop;
	u8		conn_chn_map[5];
	u8		conn_chn_map_next[5];
	u8		connParaUpReq_pending;

	u32		conn_Req_noAck_timeout;
	u8		conn_Req_waitAck_enable;
	u8		conn_terminate_reason;
	u8		slave_terminate_conn_flag;
	u8	 	master_terminate_conn_flag;
	u8		rscd11;
	u8		rsvd22;
	u8		conn_terminate_pending;   // terminate_pending = master_terminate || slave_terminate
	u8		remote_version;

	u32		slot_idx;
	u16		slot_latency;		//8 (40ms) typical; 2 (10ms) for high throughput connection
	u16		slot_num;

	u16		slot_latency_next;
	u16		enc_ediv;

	u32		enc_ivs;
	u8		enc_random[8];
	u8 		enc_skds[8];
	ble_crypt_para_t	crypt;

} st_ll_conn_master_t;





/******************************* User Interface  ************************************/
void blc_ll_initMasterRoleSingleConn_module(void);


u8 blm_ll_isRfStateMachineBusy(void);

u8   blm_ll_getTxFifoNumber (u16 connHandle);
u8 blm_ll_isTxFifoAvailableForApp(u16 connHandle);



#endif   //end of LL_MASTER_SINGLE_CONNECTION





ble_sts_t blm_ll_disconnect (u16 handle, u8 reason);

ble_sts_t blm_ll_updateConnection (u16 connHandle,
							  u16 conn_min, u16 conn_max, u16 conn_latency, u16 timeout,
							  u16 ce_min, u16 ce_max );



extern 	int		blm_create_connection;




rf_packet_l2cap_t * blm_l2cap_packet_pack (u16 conn, u8 * raw_pkt);


st_ll_conn_master_t * blm_ll_getConnection (u16 h);

u8 blm_ll_deviceIsConnState (u8 addr_type, u8* mac_addr);

//------------- ATT client function -------------------------------

//------------- ATT service discovery function -------------------------------
int blm_att_discoveryService (u16 connHandle, att_db_uuid16_t *p16, int n16, att_db_uuid128_t *p128, int n128);
u16 blm_att_findHandleOfUuid16 (att_db_uuid16_t *p, u16 uuid, u16 ref);
u16 blm_att_findHandleOfUuid128 (att_db_uuid128_t *p, const u8 * uuid);
u16 blm_att_discoveryHandleOfUUID (u8 *l2cap_data, u8 *uuid128);

//------------	master function -----------------------------------
u8 blm_fifo_num (u16 h);


u8 	  blm_push_fifo (int connHandle, u8 *dat);

void 	blm_main_loop (void);


//------------	master security function -------------------
int  blm_ll_startEncryption (u8 connhandle ,u16 ediv, u8* random, u8* ltk);
void blm_ll_startDistributeKey (u8 connhandle );


int blm_l2cap_packet_receive (u16 conn, u8 * raw_pkt);

ble_sts_t	blm_hci_receiveHostACLData(u16 connHandle, u8 PB_Flag, u8 BC_Flag, u8 *pData );

#endif /* LL_MASTER_H_ */
