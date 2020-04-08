/********************************************************************************************************
 * @file     ll.h 
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


extern u8 adv_filter ;
extern u8 adv_mesh_en_flag ;
extern u8 mesh_kr_filter_flag ;
extern u8 mesh_provisioner_buf_enable ;
extern u8 ble_state;

extern u8					blt_state;
#ifndef VENDOR_ID
#if (WIN32 && (0 == DEBUG_VENDOR_CMD_EN))    // because app can't set value by macro
#define         VENDOR_ID                       0x0000
#else
	#if MI_API_ENABLE
#define         VENDOR_ID                       VENDOR_ID_MI
	#else
#define         VENDOR_ID                       0x0211
	#endif
#endif
#endif


void blt_set_bluetooth_version (u8 v);

void enable_mesh_provision_buf();
void disable_mesh_provision_buf();
void enable_mesh_kr_cfg_filter();
void disable_mesh_kr_cfg_filter();
void ota_fw_check_over_write (void);

#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258))
#include <stack/ble/ll/ll.h>
#include <stack/ble/ll/ll_conn/ll_conn.h>
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include <stack/ble_8278/ll/ll.h>
#include <stack/ble_8278/ll/ll_conn/ll_conn.h>
#else
#ifndef LL__H_
#define LL__H_

#define			BLUETOOTH_VER_4_0				6
#define			BLUETOOTH_VER_4_1				7
#define			BLUETOOTH_VER_4_2				8

#define			BLUETOOTH_VER					BLUETOOTH_VER_4_2

#if (BLUETOOTH_VER == BLUETOOTH_VER_4_2)
	#define			BLUETOOTH_VER_SUBVER			0x22BB
#else
	#define			BLUETOOTH_VER_SUBVER			0x4103
#endif

#include "../ble_common.h"
#include "../att.h"
#include "../uuid.h"
#include "../l2cap.h"
#include "../gap.h"
#include "../hci/hci_const.h"
#include "../hci/hci_event.h"
#include "../blt_config.h"

#include "ll_encrypt.h"
#include "ll_pm.h"
#include "ll_adv.h"
#include "ll_scan.h"
#include "ll_init.h"
#include "ll_whitelist.h"
#include "ll_slave.h"
#include "ll_master.h"


/////////////////////////////////////////////////////////////////////////////
#define		CLOCK_SYS_CLOCK_1250US			(1250 * sys_tick_per_us)
#define		CLOCK_SYS_CLOCK_10MS			(10000 * sys_tick_per_us)
#define		FLG_RF_CONN_DONE	(FLD_RF_IRQ_CMD_DONE | FLD_RF_IRQ_FSM_TIMEOUT | FLD_RF_IRQ_FIRST_TIMEOUT | FLD_RF_IRQ_RX_TIMEOUT)


/////////////////////////////////////////////////////////////////////////////
#define 				CONTROLLER_ONLY				0//1//

#define 				LL_ROLE_MASTER              0
#define 				LL_ROLE_SLAVE               1

#define					BLM_CONN_HANDLE				BIT(7)
#define					BLS_CONN_HANDLE				BIT(6)

#define					HANDLE_STK_FLAG				BIT(15)
/////////////////////////////////////////////////////////////////////////////
#define					LL_CONNECTION_UPDATE_REQ	0x00
#define					LL_CHANNEL_MAP_REQ			0x01
#define					LL_TERMINATE_IND			0x02

#define					LL_UNKNOWN_RSP				0x07
#define 				LL_FEATURE_REQ              0x08
#define 				LL_FEATURE_RSP              0x09

#define 				LL_VERSION_IND              0x0C
#define 				LL_REJECT_IND         		0x0D
#define 				LL_SLAVE_FEATURE_REQ        0x0E
#define 				LL_CONNECTION_PARAM_REQ		0x0F
#define 				LL_CONNECTION_PARAM_RSP		0x10
#define					LL_REJECT_IND_EXT			0x11
#define 				LL_PING_REQ					0x12
#define					LL_PING_RSP					0x13
#define 				LL_LENGTH_REQ				0x14
#define					LL_LENGTH_RSP				0x15

#define					LL_ENC_REQ					0x03
#define					LL_ENC_RSP					0x04
#define					LL_START_ENC_REQ			0x05
#define					LL_START_ENC_RSP			0x06

#define					LL_PAUSE_ENC_REQ			0x0A
#define					LL_PAUSE_ENC_RSP			0x0B

#define					SLAVE_LL_ENC_OFF			0
#define					SLAVE_LL_ENC_REQ			1
#define					SLAVE_LL_ENC_RSP_T			2
#define					SLAVE_LL_ENC_START_REQ_T	3
#define					SLAVE_LL_ENC_START_RSP		4
#define					SLAVE_LL_ENC_START_RSP_T	5
#define					SLAVE_LL_ENC_PAUSE_REQ		6
#define					SLAVE_LL_ENC_PAUSE_RSP_T	7
#define					SLAVE_LL_ENC_PAUSE_RSP		8
#define					SLAVE_LL_REJECT_IND_T		9

#define					MASTER_LL_ENC_OFF				0
#define					MASTER_LL_ENC_REQ				1
#define					MASTER_LL_ENC_RSP_T				2
#define					MASTER_LL_ENC_START_REQ_T		3
#define					MASTER_LL_ENC_START_RSP			4
#define					MASTER_LL_ENC_START_RSP_T		5
#define					MASTER_LL_ENC_PAUSE_REQ			6
#define					MASTER_LL_ENC_PAUSE_RSP_T		7
#define					MASTER_LL_ENC_PAUSE_RSP			8
#define					MASTER_LL_REJECT_IND_T			9
#define					MASTER_LL_ENC_SMP_INFO_S		10
#define					MASTER_LL_ENC_SMP_INFO_E		11

#define					LL_ENC_REQ					0x03
#define					LL_ENC_RSP					0x04
#define					LL_START_ENC_REQ			0x05
#define					LL_START_ENC_RSP			0x06





//ble link layer state
#define			BLS_LINK_STATE_IDLE				0
#define			BLS_LINK_STATE_ADV				BIT(0)
#define			BLS_LINK_STATE_SCAN				BIT(1)
#define			BLS_LINK_STATE_INIT				BIT(2)
#define 		BLS_LINK_STATE_CONN				BIT(3)

//#define 		BLS_LINK_STATE_CONN_SLAVE		BIT(3)
//#define 		BLS_LINK_STATE_CONN_MASTER		BIT(4)



#define				BLE_STATE_IDLE			0
#define				BLE_STATE_ADV			1
#define				BLE_STATE_SCAN			2
#define				BLE_STATE_INIT			3

#define				BLE_STATE_BTX_S			4
#define				BLE_STATE_BTX_E			5
#define				BLE_STATE_BRX_S			6
#define				BLE_STATE_BRX_E			7

#define			BLS_FLAG_ADV_IN_SLAVE_MODE				BIT(6)
#define			BLS_FLAG_SCAN_ENABLE					BIT(0)



#define			MAX_OCTETS_DATA_LEN_27					27
#define			MAX_OCTETS_DATA_LEN_EXTENSION			251


#define			LL_PACKET_OCTET_TIME(n)				((n) * 8 + 112)

#define 		DATA_LENGTH_REQ_PENDING				1
#define			DATA_LENGTH_REQ_DONE				2


extern my_fifo_t			blt_rxfifo;
extern u8					blt_rxfifo_b[];

extern my_fifo_t			blt_txfifo;
extern u8					blt_txfifo_b[];
//////////////////////////////////////


typedef struct {
	u8		macAddress_public[6];
	u8		macAddress_random[6];   //host may set this
}ll_mac_t;

#ifndef WIN32
ll_mac_t  bltMac;
#endif

typedef struct {
	u8		adv_en;
	u8		adv_extension_mask;
	u8		adv_scanReq_connReq;
	u8 		phy_en;


	u8		ll_recentAvgRSSI;
	u8		ll_localFeature;
	u8		tx_irq_proc_en;
	u8		conn_rx_num;  //slave: rx number in a new interval
} st_ll_conn_t;

#ifndef WIN32
st_ll_conn_t  bltParam;
#endif

typedef struct {
	u16		connEffectiveMaxRxOctets;
	u16		connEffectiveMaxTxOctets;
	u16 	connMaxRxOctets;
	u16 	connMaxTxOctets;
	u16		connRemoteMaxRxOctets;
	u16 	connRemoteMaxTxOctets;
	u16		supportedMaxRxOctets;
	u16		supportedMaxTxOctets;

	u16 	connInitialMaxTxOctets;
	u8		connMaxTxRxOctets_req;
}ll_data_extension_t;

#ifndef WIN32
ll_data_extension_t  bltData;
#endif





////////////////// Telink defined Event Callback  ////////////////////////
typedef void (*blt_event_callback_t)(u8 e, u8 *p, int n);

#define 		BLT_EV_MAX_NUM						20

#define			BLT_EV_FLAG_ADV						0
#define			BLT_EV_FLAG_ADV_DURATION_TIMEOUT	1
#define			BLT_EV_FLAG_SCAN_RSP				2
#define			BLT_EV_FLAG_CONNECT					3    //
#define			BLT_EV_FLAG_TERMINATE				4    //
#define			BLT_EV_FLAG_PAIRING_BEGIN			5
#define			BLT_EV_FLAG_PAIRING_END				6	 //success or fail(with fail reason)
#define			BLT_EV_FLAG_ENCRYPTION_CONN_DONE    7
#define			BLT_EV_FLAG_DATA_LENGTH_EXCHANGE	8
#define			BLT_EV_FLAG_GPIO_EARLY_WAKEUP		9
#define			BLT_EV_FLAG_CHN_MAP_REQ				10
#define			BLT_EV_FLAG_CONN_PARA_REQ			11
#define			BLT_EV_FLAG_CHN_MAP_UPDATE			12
#define			BLT_EV_FLAG_CONN_PARA_UPDATE		13
#define			BLT_EV_FLAG_SUSPEND_ENTER			14
#define			BLT_EV_FLAG_SUSPEND_EXIT			15
#define			BLT_EV_FLAG_READ_P256_KEY			16
#define			BLT_EV_FLAG_GENERATE_DHKEY			17
#define			BLT_EV_FLAG_ADV_REPORT				18



#define 		EVENT_MASK_ADV_DURATION_TIMEOUT		BIT(BLT_EV_FLAG_ADV_DURATION_TIMEOUT)
#define			EVENT_MASK_SCAN_RSP					BIT(BLT_EV_FLAG_SCAN_RSP)
#define			EVENT_MASK_CONNECT					BIT(BLT_EV_FLAG_CONNECT)
#define			EVENT_MASK_TERMINATE				BIT(BLT_EV_FLAG_TERMINATE)
#define			EVENT_MASK_CHN_MAP_REQ				BIT(BLT_EV_FLAG_CHN_MAP_REQ)
#define			EVENT_MASK_CONN_PARA_REQ			BIT(BLT_EV_FLAG_CONN_PARA_REQ)
#define			EVENT_MASK_CHN_MAP_UPDATE			BIT(BLT_EV_FLAG_CHN_MAP_UPDATE)
#define			EVENT_MASK_CONN_PARA_UPDATE			BIT(BLT_EV_FLAG_CONN_PARA_UPDATE)
#define			EVENT_MASK_READ_P256_KEY			BIT(BLT_EV_FLAG_READ_P256_KEY)
#define			EVENT_MASK_GENERATE_DHKEY			BIT(BLT_EV_FLAG_GENERATE_DHKEY)



typedef void (*ll_irq_tx_callback_t)(void);

typedef int (*ll_irq_rx_data_callback_t)(u8 *, u32);
typedef int (*ll_irq_rx_post_callback_t)(void);

typedef void (*ll_irq_systemTick_conn_callback_t)(void);


typedef int (*blc_main_loop_data_callback_t)(u8 *);
typedef int (*blc_main_loop_pre_callback_t)(void);
typedef int (*blc_main_loop_post_callback_t)(void);





typedef int (*blt_LTK_req_callback_t)(u16 handle, u8* rand, u16 ediv);



extern my_fifo_t		hci_tx_fifo;



/******************************* User Interface  ************************************/
void		irq_blt_sdk_handler ();;
int 		blt_sdk_main_loop(void);

void 		blc_ll_initBasicMCU (u8 *public_adr);

ble_sts_t 	blc_ll_setRandomAddr(u8 *randomAddr);


ble_sts_t 	blc_ll_readBDAddr(u8 *addr);

u8			blc_ll_getCurrentState(void);

u8 			blc_ll_getLatestAvgRSSI(void);

u16   		blc_ll_setInitTxDataLength (u16 maxTxOct);   //core4.2 long data packet







// application
void		bls_app_registerEventCallback (u8 e, blt_event_callback_t p);






/************************* Stack Interface, user can not use!!! ***************************/
//encryption
ble_sts_t 		blc_hci_ltkRequestNegativeReply (u16 connHandle);
ble_sts_t  		blc_hci_ltkRequestReply (u16 connHandle,  u8*ltk);

void 			blc_ll_setEncryptionBusy(u8 enc_busy);
u8 			blc_ll_isEncryptionBusy(void);
void 			blc_ll_registerLtkReqEvtCb(blt_LTK_req_callback_t* evtCbFunc);

void 			blc_ll_setIdleState(void);
ble_sts_t 		blc_hci_le_getLocalSupportedFeatures(u8 *features);

ble_sts_t 		blc_hci_le_readBufferSize_cmd(u8 *pData);


//core4.2 data extension
void 			blc_ll_initDataLengthExtension (void);
ble_sts_t   	blc_ll_exchangeDataLength (u8 opcode, u16 maxTxOct);
ble_sts_t 		blc_hci_setTxDataLength (u16 connHandle, u16 tx, u16 txtime);
ble_sts_t 		blc_hci_readSuggestedDefaultTxDataLength (u8 *tx, u8 *txtime);
ble_sts_t 		blc_hci_writeSuggestedDefaultTxDataLength (u16 tx, u16 txtime);




int blm_send_acl_to_btusb (u16 conn, u8 *p);


#ifndef WIN32
static inline u8  blc_ll_getTxFifoNumber (void)
{
	u8 r = irq_disable();
	u8 ret = ((reg_dma_tx_wptr - reg_dma_tx_rptr) & 15 )  +  ( (blt_txfifo.wptr - blt_txfifo.rptr) & 31 ) ;
	irq_restore(r);
	return ret;
}


static inline u8  blc_ll_getTxHardWareFifoNumber (void)
{
	return  ((reg_dma_tx_wptr - reg_dma_tx_rptr) & 15 );
}



static inline void blc_ll_resetInfoRSSI(void)
{
	bltParam.ll_recentAvgRSSI = 0;
}

static inline void blc_ll_recordRSSI(u8 rssi)
{
	if(bltParam.ll_recentAvgRSSI == 0) {
		bltParam.ll_recentAvgRSSI = rssi;
	}
	else {
		bltParam.ll_recentAvgRSSI = (bltParam.ll_recentAvgRSSI + rssi) >> 1;
	}
}
#endif



/************************************************************* RF DMA RX\TX data strcut ***************************************************************************************
----RF RX DMA buffer struct----:
byte0    byte3   byte4    byte5   byte6  byte7  byte8    byte11   byte12      byte13   byte14 byte(14+w-1) byte(14+w) byte(16+w) byte(17+w) byte(18+w) byte(19+w)    byte(20+w)
*-------------*----------*------*------*------*----------------*----------*------------*------------------*--------------------*---------------------*------------------------*
| DMA_len(4B) | Rssi(1B) |xx(1B)|yy(1B)|zz(1B)| Stamp_time(4B) | type(1B) | Rf_len(1B) |    payload(wB)   |      CRC(3B)       |   Fre_offset(2B)    |  jj(1B)     0x40(1B)   |
|             | rssi-110 |                    |                |         Header        |     Payload      |                    | (Fre_offset/8+25)/2 |          if CRC OK:0x40|
|             |                               |                |<--               PDU                  -->|                    |                     |                        |
*-------------*-------------------------------*----------------*------------------------------------------*--------------------*---------------------*------------------------*
|<------------------------------------------------------   DMA len  -------------------|------------------|------------------->|
|                                                                                      |                  |                    |
|<----------------------------------- 14 byte ---------------------------------------->|<---- Rf_len ---->|<------ 3 Byte ---->|
note: byte12 ->  type(1B):llid(2bit) nesn(1bit) sn(1bit) md(1bit)
we can see: DMA_len = w(Rf_len) + 17.
		    CRC_OK  = DMA_buffer[w(Rf_len) + 20] == 0x40 ? True : False.

----RF TX DMA buffer struct----:
byte0    byte3   byte4       byte5      byte6  byte(6+w-1)
*-------------*----------*------------*------------------*
| DMA_len(4B) | type(1B) | Rf_len(1B) |    payload(wB)   |
|             |         Header        |     Payload      |
|             |<--               PDU                  -->|
*-------------*------------------------------------------*
note: type(1B):llid(2bit) nesn(1bit) sn(1bit) md(1bit),实际向RF 硬件FIFO中压数据，type只表示llid,其他bit位为0！
*******************************************************************************************************************************************************************************/



#endif /* LL__H_ */
#endif
