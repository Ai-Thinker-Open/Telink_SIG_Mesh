/********************************************************************************************************
 * @file     ll_adv.h 
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
#if (!WIN32)
#include "proj/mcu/config.h"
#endif
#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258))
#include "stack/ble/ll_adv.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "stack/ble_8278/ll_adv.h"
#else

#ifndef LL_ADV_H_
#define LL_ADV_H_


/* Advertising Maximum data length */
#define 		ADV_MAX_DATA_LEN                    31

#define			BLT_ENABLE_ADV_37			BIT(0)
#define			BLT_ENABLE_ADV_38			BIT(1)
#define			BLT_ENABLE_ADV_39			BIT(2)
#define			BLT_ENABLE_ADV_ALL			(BLT_ENABLE_ADV_37 | BLT_ENABLE_ADV_38 | BLT_ENABLE_ADV_39)




#define 		BLC_ADV_DISABLE						0
#define 		BLC_ADV_ENABLE						1


#define			BLS_FLAG_ADV_IN_SLAVE_MODE			BIT(6)

#define 		BLC_FLAG_STK_ADV					BIT(24)


typedef struct {
	u8		adv_chn_mask;
	u8		adv_duraton_en;
	u8		adv_type;

	u8 		adv_filterPolicy;

	u8		T_SCAN_RSP_INTVL;
	u8		advInt_rand;
	u16		advInt_min;

	u32		adv_interval;     // system tick
	u32		adv_duration_us;
	u32		adv_begin_tick;
}st_ll_adv_t;







extern rf_packet_adv_t	pkt_adv;


typedef int (*ll_adv2conn_callback_t)(u8 *);   //rcvd conn_req, adv state to conn state
extern 	ll_adv2conn_callback_t ll_adv2conn_cb;


typedef int  (*ll_module_adv_callback_t)(void);



typedef int (*advertise_prepare_handler_t) (rf_packet_adv_t * p);



/******************************* User Interface  ************************************/
void 		blc_ll_initAdvertising_module(u8 *public_adr);;


ble_sts_t	bls_ll_setAdvData(u8 *data, u8 len);
ble_sts_t 	bls_ll_setScanRspData(u8 *data, u8 len);
ble_sts_t   bls_ll_setAdvEnable(int adv_enable);

u8 			blt_set_adv_direct_init_addrtype(u8* cmdPara);

ble_sts_t   bls_ll_setAdvParam( u16 intervalMin, u16 intervalMax, u8 advType, 	   u8 ownAddrType,  \
							     u8 peerAddrType, u8 *peerAddr,   u8 adv_channelMap, u8 advFilterPolicy);


ble_sts_t 	bls_ll_setAdvInterval(u16 intervalMin, u16 intervalMax);
ble_sts_t 	bls_ll_setAdvChannelMap(u8 adv_channelMap);
ble_sts_t 	bls_ll_setAdvFilterPolicy(u8 advFilterPolicy);

ble_sts_t   bls_ll_setAdvDuration (u32 duration_us, u8 duration_en);


void 		blc_ll_setAdvCustomedChannel (u8 chn0, u8 chn1, u8 chn2);


void 		bls_ll_adjustScanRspTiming( s8 t_us );
void        blt_send_adv2scan_mode(int tx_adv);




ble_sts_t   blc_ll_addAdvertisingInConnSlaveRole(void);
ble_sts_t   blc_ll_removeAdvertisingFromConnSLaveRole(void);
ble_sts_t 	blc_ll_setAdvParamInConnSlaveRole( u8 *adv_data, u8 advData_len, u8 *scanRsp_data, u8 scanRspData_len,
											 u8 advType,   u8 ownAddrType, u8 adv_channelMap, u8 advFilterPolicy);


/************************* Stack Interface, user can not use!!! ***************************/
ble_sts_t 	bls_hci_le_setAdvParam(adv_para_t *para);
ble_sts_t 	bls_hci_le_readChannelMap(u16 connHandle, u8 *returnChannelMap);







#endif /* LL_ADV_H_ */
#endif

