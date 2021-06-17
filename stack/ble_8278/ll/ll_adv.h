/********************************************************************************************************
 * @file     ll_adv.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2015
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
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
/*
 * ll_adv.h
 *
 *  Created on: 2017-3-7
 *      Author: Administrator
 */

#ifndef LL_ADV_H_
#define LL_ADV_H_

#include "../ble_format.h"
#include "ll_whitelist.h"

/* Advertising Maximum data length */
#define 		ADV_MAX_DATA_LEN                    31


#define			BLS_FLAG_ADV_IN_SLAVE_MODE			BIT(6)

#define 		BLC_FLAG_STK_ADV					BIT(24)









typedef struct {
	u8		adv_chn_mask;
	u8		adv_duraton_en;
	u8		adv_type;
	u8 		adv_filterPolicy;

	u8		T_SCAN_RSP_INTVL;
	u8      own_addr_type;     //own_addr_type
	u8		rsvd;
	u8		advInt_rand;

	u16		advInt_min;
	u16 	advInt_max;

	u32		adv_interval;     // system tick
	u32		adv_duration_us;
	u32		adv_begin_tick;

}st_ll_adv_t;



extern _attribute_aligned_(4) st_ll_adv_t  blta;


extern rf_packet_adv_t	pkt_adv;


typedef int (*ll_adv2conn_callback_t)(u8 *, bool);   //rcvd conn_req, adv state to conn state
extern 	ll_adv2conn_callback_t ll_adv2conn_cb;


typedef int  (*ll_module_adv_callback_t)(void);



typedef int (*advertise_prepare_handler_t) (rf_packet_adv_t * p);


extern u32  blc_rcvd_connReq_tick;



/******************************* User Interface  ************************************/
void 		blc_ll_initAdvertising_module(u8 *public_adr);;


ble_sts_t	bls_ll_setAdvData(u8 *data, u8 len);
ble_sts_t 	bls_ll_setScanRspData(u8 *data, u8 len);
ble_sts_t   bls_ll_setAdvEnable(int adv_enable);

u8 			blt_set_adv_direct_init_addrtype(u8* cmdPara);

ble_sts_t   bls_ll_setAdvParam( u16 intervalMin,  u16 intervalMax,  adv_type_t advType,  		 	  own_addr_type_t ownAddrType,  \
							     u8 peerAddrType, u8  *peerAddr,    adv_chn_map_t 	adv_channelMap,   adv_fp_type_t   advFilterPolicy);

ble_sts_t 	bls_ll_setAdvInterval(u16 intervalMin, u16 intervalMax);
ble_sts_t 	bls_ll_setAdvChannelMap(adv_chn_map_t adv_channelMap);
ble_sts_t 	bls_ll_setAdvFilterPolicy(adv_fp_type_t advFilterPolicy);

ble_sts_t   bls_ll_setAdvDuration (u32 duration_us, u8 duration_en);


void 		blc_ll_setAdvCustomedChannel (u8 chn0, u8 chn1, u8 chn2);
void        blt_send_adv2scan_mode(int tx_adv); // add by weixiong in mesh




ble_sts_t   blc_ll_addAdvertisingInConnSlaveRole(void);
ble_sts_t   blc_ll_removeAdvertisingFromConnSLaveRole(void);
ble_sts_t 	blc_ll_setAdvParamInConnSlaveRole( u8 		  *adv_data,  u8              advData_len, u8 *scanRsp_data,  			 u8 scanRspData_len,
											   adv_type_t  advType,   own_addr_type_t ownAddrType, adv_chn_map_t adv_channelMap, adv_fp_type_t advFilterPolicy);


static inline u32 	bls_ll_getConnectionCreateTime(void)
{
	return blc_rcvd_connReq_tick;
}

/************************* Stack Interface, user can not use!!! ***************************/
ble_sts_t 	bls_hci_le_setAdvParam(adv_para_t *para);
ble_sts_t 	bls_hci_le_readChannelMap(u16 connHandle, u8 *returnChannelMap);


ble_sts_t   bls_ll_setAdvType(u8 advType);
ble_sts_t 	blt_set_adv_addrtype(u8* cmdPara);

static inline u8 	blt_ll_getOwnAddrType(void)
{
	return blta.own_addr_type;
}


#endif /* LL_ADV_H_ */
