/********************************************************************************************************
 * @file     ll_scan.h 
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
#include "stack/ble/ll_scan.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278) 
#include "stack/ble_8278/ll_scan.h"
#else

#ifndef LL_SCAN_H_
#define LL_SCAN_H_



#define 		BLC_SCAN_DISABLE						0
#define 		BLC_SCAN_ENABLE							1

#define			FILTER_DUP_DISABLE						0
#define 		FILTER_DUP_ENABLE						1


#define			BLS_FLAG_SCAN_ENABLE					BIT(0)


#define			BLS_FLAG_SCAN_IN_ADV_MODE				BIT(5)
#define			BLS_FLAG_SCAN_IN_SLAVE_MODE				BIT(6)




typedef struct {
	u8		scan_en;
	u8		scan_type;
	u8		scan_filterPolicy;
	u8		filter_dup;

	u8		scanDevice_num;
	u8		scanRspDevice_num;
	u8		scan_extension_mask;
	s8		T_SCAN_REQ_INTVL;



	//u32		scan_interval;
}st_ll_scan_t;
#ifndef WIN32
st_ll_scan_t  blts;
#endif
#ifndef WIN32
u32		blts_scan_interval;
#endif
extern 			rf_packet_scan_req_t	pkt_scan_req;


typedef int (*ll_procScanPkt_callback_t)(u8 *, u8 *, u32);
typedef int (*ll_procScanDat_callback_t)(u8 *);

extern ll_procScanDat_callback_t  blc_ll_procScanDatCb;
extern ll_procScanPkt_callback_t  blc_ll_procScanPktCb;





/******************************* User Interface  ************************************/
void 		blc_ll_initScanning_module(u8 *public_adr);


ble_sts_t   blc_ll_setScanParameter (u8 scan_type, u16 scan_interval, u16 scan_window, u8  ownAddrType, u8 filter_policy);
ble_sts_t 	blc_ll_setScanEnable (u8 scan_enable, u8 filter_duplicate);



ble_sts_t    blc_ll_addScanningInAdvState(void);
ble_sts_t    blc_ll_removeScanningFromAdvState(void);
ble_sts_t    blc_ll_addScanningInConnSlaveRole(void);
ble_sts_t    blc_ll_removeScanningFromConnSLaveRole(void);



/************************* Stack Interface, user can not use!!! ***************************/
int			blc_ll_filterAdvDevice (u8 type, u8 * mac);
int 		blc_ll_addScanRspDevice(u8 type, u8 *mac);
u8 		blc_ll_isScanRspReceived(u8 type, u8 *mac);
void 		blc_ll_clearScanRspDevice(void);


int  		blc_ll_procScanPkt(u8 *raw_pkt, u8 *new_pkt, u32 tick_now);
int  		blc_ll_procScanData(u8 *raw_pkt);

void 		blc_ll_switchScanChannel (int, int);


#endif /* LL_SCAN_H_ */
#endif
