/********************************************************************************************************
 * @file     ll_slave.h 
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

#ifndef LL_SLAVE_H_
#define LL_SLAVE_H_
	


typedef struct {
	u8		time_update_st;
	u8 		last_rf_len;
	u8 		ll_remoteFeature; //not only one for BLE master, use connHandle to identify
	u8		remoteFeatureReq;

	u8 		brx_rx_num;  //rx number in a new interval  brx state machine
	u8		interval_level;
	u16		connHandle;


	u8		ll_recentAvgRSSI;
	u8		conn_sn_master;
	u8		conn_chn;
	u8		conn_update;


	u8		master_not_ack_slaveAckUpReq;
	u8		conn_rcvd_ack_pkt;
	u16		conn_update_inst_diff;


	u32		conn_access_code_revert;
	u32		conn_crc;
	u32		connExpectTime;
	int		conn_interval_adjust;
	u32		conn_timeout;
	u32		conn_tick;
	u32		conn_interval;
	u16		conn_inst;
	u16		conn_latency;
	u32		conn_duration;

	u8		conn_new_param;
	u8		conn_winsize_next;
	u16     conn_offset_next;
	u16		conn_inst_next;
	u16		conn_interval_next; //standard value,  not * 1.25ms
	u16		conn_latency_next;
	u16		conn_timeout_next;  //standard value,  not *10ms


	u8		conn_chn_hop;
	u8		rsvd1;
	u8		conn_chn_map[5];
	u8		conn_chn_map_next[5];


	u32		conn_start_tick;

	int		conn_tolerance_time;

	u32		tick_1st_rx;
	u32		conn_brx_tick;

	u8 		conn_master_terminate;
	u8		conn_terminate_reason;
	u8 		conn_slave_terminate;
	u8		conn_terminate_pending;   // terminate_pending = master_terminate || slave_terminate
	u32 	conn_slaveTerminate_time;

	u32		conn_pkt_rcvd;
	u32		conn_pkt_rcvd_no;
	u8 *	conn_pkt_dec_pending;
	int		conn_enc_dec_busy;
	int		conn_stop_brx;
} st_ll_conn_slave_t;







/******************************* User Interface  ************************************/
void 		blc_ll_initSlaveRole_module(void);

ble_sts_t  	bls_ll_terminateConnection (u8 reason);

u8		bls_ll_isConnectState (void);

u16			bls_ll_getConnectionInterval(void);  // if return 0, means not in connection state
u16			bls_ll_getConnectionLatency(void);	 // if return 0, means not in connection state
u16			bls_ll_getConnectionTimeout(void);	 // if return 0, means not in connection state


int			bls_ll_requestConnBrxEventDisable(void);
void		bls_ll_disableConnBrxEvent(void);
void		bls_ll_restoreConnBrxEvent(void);





//ble module event
ble_sts_t 	bls_hci_mod_setEventMask_cmd(u32 evtMask);  //eventMask: module special




/************************* Stack Interface, user can not use!!! ***************************/
u8		bls_ll_pushTxFifo (int hanlde, u8 *p);
ble_sts_t  	bls_hci_reset(void);

ble_sts_t	bls_hci_receiveHostACLData(u16 connHandle, u8 PB_Flag, u8 BC_Flag, u8 *pData );



#endif /* LL_SLAVE_H_ */
