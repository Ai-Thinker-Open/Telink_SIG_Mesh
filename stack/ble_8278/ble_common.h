/********************************************************************************************************
 * @file     ble_common.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
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
#ifndef BLE_COMMON_H
#define BLE_COMMON_H

#include "proj/mcu/config.h"
#include "proj/tl_common.h"
/*********************************************************************
 * CONSTANTS
 */


/**
 *  @brief  Definition for Link Layer Feature Support
 */
#define LL_FEATURE_SIZE                                      8
#define LL_FEATURE_MASK_LL_ENCRYPTION                        (0x00000001)   //core_4.0
#define LL_FEATURE_MASK_CONNECTION_PARA_REQUEST_PROCEDURE  	 (0x00000002)	//core_4.1
#define LL_FEATURE_MASK_EXTENDED_REJECT_INDICATION           (0x00000004)	//core_4.1
#define LL_FEATURE_MASK_SLAVE_INITIATED_FEATURES_EXCHANGE    (0x00000008)	//core_4.1
#define LL_FEATURE_MASK_LE_PING                              (0x00000010)   //core_4.1
#define LL_FEATURE_MASK_LE_DATA_LENGTH_EXTENSION             (0x00000020)	//core_4.2
#define LL_FEATURE_MASK_LL_PRIVACY                           (0x00000040)	//core_4.2
#define LL_FEATURE_MASK_EXTENDED_SCANNER_FILTER_POLICIES     (0x00000080)   //core_4.2
#define LL_FEATURE_MASK_LE_2M_PHY         					 (0x00000100)	//core_5.0
#define LL_FEATURE_MASK_STABLE_MODULATION_INDEX_TX 			 (0x00000200)	//core_5.0
#define LL_FEATURE_MASK_STABLE_MODULATION_INDEX_RX 			 (0x00000400)	//core_5.0
#define LL_FEATURE_MASK_LE_CODED_PHY     					 (0x00000800)	//core_5.0
#define LL_FEATURE_MASK_LE_EXTENDED_ADVERTISING          	 (0x00001000)	//core_5.0
#define LL_FEATURE_MASK_LE_PERIODIC_ADVERTISING     		 (0x00002000)	//core_5.0
#define LL_FEATURE_MASK_CHANNEL_SELECTION_ALGORITHM2         (0x00004000)	//core_5.0
#define LL_FEATURE_MASK_LE_POWER_CLASS_1 					 (0x00008000)	//core_5.0
#define LL_FEATURE_MASK_MIN_USED_OF_USED_CHANNELS   	     (0x00010000)	//core_5.0

/////////////////////////////////////////////////////////////////////////////

//#define         VENDOR_ID                       0x0211 // comment by weixiong in mesh
#define         VENDOR_ID_HI_B                  U16_HI(VENDOR_ID)
#define         VENDOR_ID_LO_B                  U16_LO(VENDOR_ID)

#define			BLUETOOTH_VER_4_0				6
#define			BLUETOOTH_VER_4_1				7
#define			BLUETOOTH_VER_4_2				8
#define			BLUETOOTH_VER_5_0				9

#define			BLUETOOTH_VER					BLUETOOTH_VER_4_2 // add by weixiong in mesh

#ifndef 		BLUETOOTH_VER
#define			BLUETOOTH_VER					BLUETOOTH_VER_5_0
#endif


#if (BLUETOOTH_VER == BLUETOOTH_VER_4_2)
	#define			BLUETOOTH_VER_SUBVER			0x22BB
#elif (BLUETOOTH_VER == BLUETOOTH_VER_5_0)
	#define			BLUETOOTH_VER_SUBVER			0x1C1C
#else
	#define			BLUETOOTH_VER_SUBVER			0x4103
#endif




#if (BLUETOOTH_VER == BLUETOOTH_VER_4_0)
	#define LL_FEATURE_ENABLE_LE_ENCRYPTION								1
	#define LL_CMD_MAX						   							LL_REJECT_IND

#elif (BLUETOOTH_VER == BLUETOOTH_VER_4_1)
	#define LL_FEATURE_ENABLE_LE_ENCRYPTION								1
	#define	LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION				1
	#define	LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE			1
	#define	LL_FEATURE_ENABLE_LE_PING									1

	#define LL_CMD_MAX						   							LL_PING_RSP

#elif (BLUETOOTH_VER == BLUETOOTH_VER_4_2)

	#define LL_FEATURE_ENABLE_LE_ENCRYPTION								0 // modify by weixiong
	#define	LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION				0 // modify by weixiong
	#define	LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE			0 // modify by weixiong
	#define	LL_FEATURE_ENABLE_LE_PING									0 // modify by weixiong
	#define	LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION					BLE_CORE42_DATA_LENGTH_EXTENSION_ENABLE

	#define LL_CMD_MAX						  							LL_LENGTH_RSP

#elif (BLUETOOTH_VER == BLUETOOTH_VER_5_0)

	#define LL_FEATURE_ENABLE_LE_ENCRYPTION								1
	#define	LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION				1
	#define	LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE			1
	#define	LL_FEATURE_ENABLE_LE_PING									1
	#define	LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION					BLE_CORE42_DATA_LENGTH_EXTENSION_ENABLE

	#define	LL_FEATURE_ENABLE_LE_2M_PHY									LL_FEATURE_SUPPORT_LE_2M_PHY
	#define	LL_FEATURE_ENABLE_LE_CODED_PHY								LL_FEATURE_SUPPORT_LE_CODED_PHY
	#define	LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING					LL_FEATURE_SUPPORT_LE_EXTENDED_ADVERTISING
	#define	LL_FEATURE_ENABLE_LE_PERIODIC_ADVERTISING					LL_FEATURE_SUPPORT_LE_PERIODIC_ADVERTISING
	#define	LL_FEATURE_ENABLE_CHANNEL_SELECTION_ALGORITHM2				LL_FEATURE_SUPPORT_CHANNEL_SELECTION_ALGORITHM2

	#define LL_CMD_MAX						   							LL_MIN_USED_CHN_IND
#else


#endif


#ifndef		 LL_FEATURE_ENABLE_LE_ENCRYPTION
#define		 LL_FEATURE_ENABLE_LE_ENCRYPTION							0
#endif

#ifndef		 LL_FEATURE_ENABLE_CONNECTION_PARA_REQUEST_PROCEDURE
#define		 LL_FEATURE_ENABLE_CONNECTION_PARA_REQUEST_PROCEDURE		0
#endif

#ifndef		 LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION
#define		 LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION				0
#endif

#ifndef		 LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE
#define		 LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE		0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_PING
#define		 LL_FEATURE_ENABLE_LE_PING									0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION
#define		 LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION					0
#endif

#ifndef		 LL_FEATURE_ENABLE_LL_PRIVACY
#define		 LL_FEATURE_ENABLE_LL_PRIVACY								0
#endif

#ifndef		 LL_FEATURE_ENABLE_EXTENDED_SCANNER_FILTER_POLICIES
#define		 LL_FEATURE_ENABLE_EXTENDED_SCANNER_FILTER_POLICIES			0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_2M_PHY
#define		 LL_FEATURE_ENABLE_LE_2M_PHY								0
#endif

#ifndef		 LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_TX
#define		 LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_TX				0
#endif

#ifndef		 LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_RX
#define		 LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_RX				0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_CODED_PHY
#define		 LL_FEATURE_ENABLE_LE_CODED_PHY								0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING
#define		 LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING					0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_PERIODIC_ADVERTISING
#define		 LL_FEATURE_ENABLE_LE_PERIODIC_ADVERTISING					0
#endif

#ifndef		 LL_FEATURE_ENABLE_CHANNEL_SELECTION_ALGORITHM2
#define		 LL_FEATURE_ENABLE_CHANNEL_SELECTION_ALGORITHM2				0
#endif

#ifndef		 LL_FEATURE_ENABLE_LE_POWER_CLASS_1
#define		 LL_FEATURE_ENABLE_LE_POWER_CLASS_1							0
#endif

#ifndef		 LL_FEATURE_ENABLE_MIN_USED_OF_USED_CHANNELS
#define		 LL_FEATURE_ENABLE_MIN_USED_OF_USED_CHANNELS				0
#endif




//BIT<0:31>
#if 1

// feature below is conFiged by application layer
// LL_FEATURE_ENABLE_LE_2M_PHY
// LL_FEATURE_ENABLE_LE_CODED_PHY
// LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING
// LL_FEATURE_ENABLE_CHANNEL_SELECTION_ALGORITHM2

#define LL_FEATURE_MASK_BASE0											(	LL_FEATURE_ENABLE_LE_ENCRYPTION 					<<0		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_CONNECTION_PARA_REQUEST_PROCEDURE <<1		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION 	   	<<2		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE <<3		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_PING 						   	<<4		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION 		   	<<5		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LL_PRIVACY 					   	<<6		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_EXTENDED_SCANNER_FILTER_POLICIES  <<7		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_TX 	   	<<9		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_RX  	   	<<10	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_PERIODIC_ADVERTISING 		   	<<13	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_POWER_CLASS_1 				   	<<15	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_MIN_USED_OF_USED_CHANNELS 		<<16 	)
#else
#define LL_FEATURE_MASK_0												(	LL_FEATURE_ENABLE_LE_ENCRYPTION 					<<0		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_CONNECTION_PARA_REQUEST_PROCEDURE <<1		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_EXTENDED_REJECT_INDICATION 	   	<<2		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_SLAVE_INITIATED_FEATURES_EXCHANGE <<3		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_PING 						   	<<4		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_DATA_LENGTH_EXTENSION 		   	<<5		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LL_PRIVACY 					   	<<6		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_EXTENDED_SCANNER_FILTER_POLICIES  <<7		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_2M_PHY 						<<8		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_TX 	   	<<9		|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_STABLE_MODULATION_INDEX_RX  	   	<<10	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_CODED_PHY 					   	<<11	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_EXTENDED_ADVERTISING  		   	<<12	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_PERIODIC_ADVERTISING 		   	<<13	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_CHANNEL_SELECTION_ALGORITHM2 	   	<<14	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_LE_POWER_CLASS_1 				   	<<15	|  \
		   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	   	LL_FEATURE_ENABLE_MIN_USED_OF_USED_CHANNELS 		<<16 	)

#endif


extern u32 LL_FEATURE_MASK_0;

//BIT<32:63>
#define LL_FEATURE_MASK_1												0


#define LL_FEATURE_BYTE_0												U32_BYTE0(LL_FEATURE_MASK_0)
#define LL_FEATURE_BYTE_1												U32_BYTE1(LL_FEATURE_MASK_0)
#define LL_FEATURE_BYTE_2												U32_BYTE2(LL_FEATURE_MASK_0)
#define LL_FEATURE_BYTE_3												U32_BYTE3(LL_FEATURE_MASK_0)
#define LL_FEATURE_BYTE_4												U32_BYTE0(LL_FEATURE_MASK_1)
#define LL_FEATURE_BYTE_5												U32_BYTE1(LL_FEATURE_MASK_1)
#define LL_FEATURE_BYTE_6												U32_BYTE2(LL_FEATURE_MASK_1)
#define LL_FEATURE_BYTE_7												U32_BYTE3(LL_FEATURE_MASK_1)






typedef enum {
    BLE_SUCCESS = 0,

//// HCI Status, See the Core_v5.0(Vol 2/Part D/1.3 "list of Error Codes") for more information)
    HCI_ERR_UNKNOWN_HCI_CMD                                        = 0x01,
    HCI_ERR_UNKNOWN_CONN_ID                                        = 0x02,
    HCI_ERR_HW_FAILURE                                             = 0x03,
    HCI_ERR_PAGE_TIMEOUT                                           = 0x04,
    HCI_ERR_AUTH_FAILURE                                           = 0x05,
    HCI_ERR_PIN_KEY_MISSING                                        = 0x06,
    HCI_ERR_MEM_CAP_EXCEEDED                                       = 0x07,
    HCI_ERR_CONN_TIMEOUT                                           = 0x08,
    HCI_ERR_CONN_LIMIT_EXCEEDED                                    = 0x09,
    HCI_ERR_SYNCH_CONN_LIMIT_EXCEEDED                              = 0x0A,
    HCI_ERR_ACL_CONN_ALREADY_EXISTS                                = 0x0B,
    HCI_ERR_CMD_DISALLOWED                                         = 0x0C,
    HCI_ERR_CONN_REJ_LIMITED_RESOURCES                             = 0x0D,
    HCI_ERR_CONN_REJECTED_SECURITY_REASONS                         = 0x0E,
    HCI_ERR_CONN_REJECTED_UNACCEPTABLE_BDADDR                      = 0x0F,
    HCI_ERR_CONN_ACCEPT_TIMEOUT_EXCEEDED                           = 0x10,
    HCI_ERR_UNSUPPORTED_FEATURE_PARAM_VALUE                        = 0x11,
    HCI_ERR_INVALID_HCI_CMD_PARAMS                                 = 0x12,
    HCI_ERR_REMOTE_USER_TERM_CONN                                  = 0x13,
    HCI_ERR_REMOTE_DEVICE_TERM_CONN_LOW_RESOURCES                  = 0x14,
    HCI_ERR_REMOTE_DEVICE_TERM_CONN_POWER_OFF                      = 0x15,
    HCI_ERR_CONN_TERM_BY_LOCAL_HOST                                = 0x16,
    HCI_ERR_REPEATED_ATTEMPTS                                      = 0x17,
    HCI_ERR_PAIRING_NOT_ALLOWED                                    = 0x18,
    HCI_ERR_UNKNOWN_LMP_PDU                                        = 0x19,
    HCI_ERR_UNSUPPORTED_REMOTE_FEATURE                             = 0x1A,
    HCI_ERR_SCO_OFFSET_REJ                                         = 0x1B,
    HCI_ERR_SCO_INTERVAL_REJ                                       = 0x1C,
    HCI_ERR_SCO_AIR_MODE_REJ                                       = 0x1D,
    HCI_ERR_INVALID_LMP_PARAMS                                     = 0x1E,
    HCI_ERR_UNSPECIFIED_ERROR                                      = 0x1F,
    HCI_ERR_UNSUPPORTED_LMP_PARAM_VAL                              = 0x20,
    HCI_ERR_ROLE_CHANGE_NOT_ALLOWED                                = 0x21,
    HCI_ERR_LMP_LL_RESP_TIMEOUT                                    = 0x22,
    HCI_ERR_LMP_ERR_TRANSACTION_COLLISION                          = 0x23,
    HCI_ERR_LMP_PDU_NOT_ALLOWED                                    = 0x24,
    HCI_ERR_ENCRYPT_MODE_NOT_ACCEPTABLE                            = 0x25,
    HCI_ERR_LINK_KEY_CAN_NOT_BE_CHANGED                            = 0x26,
    HCI_ERR_REQ_QOS_NOT_SUPPORTED                                  = 0x27,
    HCI_ERR_INSTANT_PASSED                                         = 0x28,
    HCI_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED                    = 0x29,
    HCI_ERR_DIFFERENT_TRANSACTION_COLLISION                        = 0x2A,
    HCI_ERR_RESERVED1                                              = 0x2B,
    HCI_ERR_QOS_UNACCEPTABLE_PARAM                                 = 0x2C,
    HCI_ERR_QOS_REJ                                                = 0x2D,
    HCI_ERR_CHAN_ASSESSMENT_NOT_SUPPORTED                          = 0x2E,
    HCI_ERR_INSUFFICIENT_SECURITY                                  = 0x2F,
    HCI_ERR_PARAM_OUT_OF_MANDATORY_RANGE                           = 0x30,
    HCI_ERR_RESERVED2                                              = 0x31,
    HCI_ERR_ROLE_SWITCH_PENDING                                    = 0x32,
    HCI_ERR_RESERVED3                                              = 0x33,
    HCI_ERR_RESERVED_SLOT_VIOLATION                                = 0x34,
    HCI_ERR_ROLE_SWITCH_FAILED                                     = 0x35,
    HCI_ERR_EXTENDED_INQUIRY_RESP_TOO_LARGE                        = 0x36,
    HCI_ERR_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST                   = 0x37,
    HCI_ERR_HOST_BUSY_PAIRING                                      = 0x38,
    HCI_ERR_CONN_REJ_NO_SUITABLE_CHAN_FOUND                        = 0x39,
    HCI_ERR_CONTROLLER_BUSY                                        = 0x3A,
    HCI_ERR_UNACCEPTABLE_CONN_INTERVAL                             = 0x3B,
    HCI_ERR_ADVERTISING_TIMEOUT                                    = 0x3C,
    HCI_ERR_CONN_TERM_MIC_FAILURE                                  = 0x3D,
    HCI_ERR_CONN_FAILED_TO_ESTABLISH                               = 0x3E,
    HCI_ERR_MAC_CONN_FAILED                                        = 0x3F,
    HCI_ERR_COARSE_CLOCK_ADJUSTMENT_REJECT						   = 0x40,
    HCI_ERR_TYPE0_SUBMAP_NOT_DEFINED							   = 0x41,
    HCI_ERR_UNKNOWN_ADV_INDENTIFIER								   = 0x42,
    HCI_ERR_LIMIT_REACHED										   = 0x43,
    HCI_ERR_OP_CANCELLED_BY_HOST								   = 0x44,
    




///////////////////////// TELINK define status /////////////////////////////

    //LL status
	LL_ERR_CONNECTION_NOT_ESTABLISH 							   = 0x80,
	LL_ERR_TX_FIFO_NOT_ENOUGH,
	LL_ERR_ENCRYPTION_BUSY,
	LL_ERR_CURRENT_STATE_NOT_SUPPORTED_THIS_CMD,

	//L2CAP status
    L2CAP_ERR_INVALID_PARAMETER 								   = 0x90,

    //SMP status
	SMP_ERR_INVALID_PARAMETER 									   = 0xA0,
	SMP_ERR_PAIRING_BUSY,

	//GATT status
	GATT_ERR_INVALID_PARAMETER 									   = 0xB0,
	GATT_ERR_PREVIOUS_INDICATE_DATA_HAS_NOT_CONFIRMED,
	GATT_ERR_SERVICE_DISCOVERY_TIEMOUT,
	GATT_ERR_NOTIFY_INDICATION_NOT_PERMITTED,
	GATT_ERR_DATA_PENDING_DUE_TO_SERVICE_DISCOVERY_BUSY,
	GATT_ERR_DATA_LENGTH_EXCEED_MTU_SIZE,

	//GAP status
	GAP_ERR_INVALID_PARAMETER 								   	   = 0xC0,

	SLAVE_TERMINATE_CONN_ACKED = 0xF0, 			// add by weixiong in mesh
	SLAVE_TERMINATE_CONN_TIMEOUT    = 0xF1, 	// add by weixiong in mesh
} ble_sts_t;













/******************************************** Link Layer **************************************************************/


/////////////////////////////// BLE  MAC ADDRESS //////////////////////////////////////////////
#define BLE_ADDR_PUBLIC                  0
#define BLE_ADDR_RANDOM                  1
#define BLE_ADDR_INVALID                 0xff
#define BLE_ADDR_LEN                     6


//Definition for BLE Common Address Type
/*
 *
 *				  |--public  ..................................................... BLE_DEVICE_ADDRESS_PUBLIC
 *                |
 * Address Type --|		      |-- random static  ................................. BLE_DEVICE_ADDRESS_RANDOM_STATIC
 *           	  |           |
 *    			  |--random --|
 * 			   				  |				       |-- non_resolvable private  ... BLE_DEVICE_ADDRESS_NON_RESOLVABLE_PRIVATE
 * 			 				  |-- random private --|
 *           					                   |-- resolvable private  ....... BLE_DEVICE_ADDRESS_RESOLVABLE_PRIVATE
 *
 */

#define	BLE_DEVICE_ADDRESS_PUBLIC							1
#define BLE_DEVICE_ADDRESS_RANDOM_STATIC					2
#define BLE_DEVICE_ADDRESS_NON_RESOLVABLE_PRIVATE			3
#define BLE_DEVICE_ADDRESS_RESOLVABLE_PRIVATE				4



#define IS_PUBLIC_ADDR(Type, Addr)  					( (Type)==BLE_ADDR_PUBLIC) )
#define IS_RANDOM_STATIC_ADDR(Type, Addr)  				( (Type)==BLE_ADDR_RANDOM && (Addr[5] & 0xC0) == 0xC0 )
#define IS_NON_RESOLVABLE_PRIVATE_ADDR(Type, Addr)  	( (Type)==BLE_ADDR_RANDOM && (Addr[5] & 0xC0) == 0x00 )
#define IS_RESOLVABLE_PRIVATE_ADDR(Type, Addr)  		( (Type)==BLE_ADDR_RANDOM && (Addr[5] & 0xC0) == 0x40 )








///////////////// ADV Interval/Scan Interval/Scan Window/Connection Interval  /////////////////
#define 		ADV_INTERVAL_3_125MS                        5
#define 		ADV_INTERVAL_3_75MS                         6
#define 		ADV_INTERVAL_10MS                           16
#define 		ADV_INTERVAL_15MS                           24
#define 		ADV_INTERVAL_20MS                           32
#define 		ADV_INTERVAL_25MS                           40
#define 		ADV_INTERVAL_30MS                           48
#define 		ADV_INTERVAL_35MS                           56
#define 		ADV_INTERVAL_40MS                           64
#define 		ADV_INTERVAL_45MS                           72
#define 		ADV_INTERVAL_50MS                           80
#define 		ADV_INTERVAL_55MS                           88
#define 		ADV_INTERVAL_60MS                           96
#define 		ADV_INTERVAL_70MS                           112
#define 		ADV_INTERVAL_80MS                           128
#define 		ADV_INTERVAL_90MS							144
#define 		ADV_INTERVAL_100MS                          160
#define 		ADV_INTERVAL_105MS                          168
#define 		ADV_INTERVAL_160MS                          256
#define 		ADV_INTERVAL_200MS                          320
#define 		ADV_INTERVAL_205MS                          328
#define 		ADV_INTERVAL_300MS                          480
#define 		ADV_INTERVAL_305MS                          488
#define 		ADV_INTERVAL_360MS                          576
#define 		ADV_INTERVAL_400MS                          640
#define 		ADV_INTERVAL_405MS                          648
#define 		ADV_INTERVAL_500MS                          800
#define 		ADV_INTERVAL_505MS                          808

#define 		ADV_INTERVAL_1S                          	1600
#define 		ADV_INTERVAL_1S5                          	2400
#define 		ADV_INTERVAL_2S                          	3200
#define 		ADV_INTERVAL_1_28_S                         0x0800
#define 		ADV_INTERVAL_10_24S                         16384



#define 		SCAN_INTERVAL_10MS                          16
#define 		SCAN_INTERVAL_30MS                          48
#define 		SCAN_INTERVAL_50MS                          80
#define 		SCAN_INTERVAL_60MS                          96
#define 		SCAN_INTERVAL_90MS                          144
#define 		SCAN_INTERVAL_100MS                         160
#define 		SCAN_INTERVAL_200MS                         320
#define 		SCAN_INTERVAL_300MS                         480
#define 		SCAN_INTERVAL_400MS                         640
#define 		SCAN_INTERVAL_500MS                         800


#define 		CONN_INTERVAL_7P5MS                          6
#define 		CONN_INTERVAL_10MS                           8
#define 		CONN_INTERVAL_11P25MS                        9
#define 		CONN_INTERVAL_15MS                           12
#define 		CONN_INTERVAL_16P25MS                        13
#define 		CONN_INTERVAL_18P75MS                        15
#define 		CONN_INTERVAL_20MS                           16
#define 		CONN_INTERVAL_30MS                           24
#define 		CONN_INTERVAL_31P25MS                        25
#define 		CONN_INTERVAL_38P75MS                        31
#define 		CONN_INTERVAL_40MS                           32
#define 		CONN_INTERVAL_48P75MS                        39
#define 		CONN_INTERVAL_50MS                           40
#define 		CONN_INTERVAL_100MS                          80


#define 		CONN_TIMEOUT_500MS							 50
#define 		CONN_TIMEOUT_1S							 	 100
#define 		CONN_TIMEOUT_4S							 	 400
#define 		CONN_TIMEOUT_10S							 1000
#define 		CONN_TIMEOUT_20S							 2000



/////////////////////////////////////////////////////////////////////////////

#define 				LLID_RESERVED				0x00
#define 				LLID_DATA_CONTINUE			0x01
#define 				LLID_DATA_START				0x02
#define 				LLID_CONTROL				0x03

/////////////////////////////////////////////////////////////////////////////
																				// rf_len without MIC
#define					LL_CONNECTION_UPDATE_REQ	0x00							// 12
#define					LL_CHANNEL_MAP_REQ			0x01							//	8
#define					LL_TERMINATE_IND			0x02							//	2

#define					LL_ENC_REQ					0x03	// encryption			// 23
#define					LL_ENC_RSP					0x04	// encryption			// 13
#define					LL_START_ENC_REQ			0x05	// encryption			//	1
#define					LL_START_ENC_RSP			0x06	// encryption			//	1

#define					LL_UNKNOWN_RSP				0x07							//	2
#define 				LL_FEATURE_REQ              0x08							//	9
#define 				LL_FEATURE_RSP              0x09							//	9

#define					LL_PAUSE_ENC_REQ			0x0A	// encryption			//	1
#define					LL_PAUSE_ENC_RSP			0x0B	// encryption			//	1

#define 				LL_VERSION_IND              0x0C							//	6
#define 				LL_REJECT_IND         		0x0D							//	2
#define 				LL_SLAVE_FEATURE_REQ        0x0E	//core_4.1				//	9
#define 				LL_CONNECTION_PARAM_REQ		0x0F	//core_4.1				// 24
#define 				LL_CONNECTION_PARAM_RSP		0x10	//core_4.1				// 24
#define					LL_REJECT_IND_EXT			0x11	//core_4.1				//	3
#define 				LL_PING_REQ					0x12    //core_4.1				//	1
#define					LL_PING_RSP					0x13    //core_4.1				//	1
#define 				LL_LENGTH_REQ				0x14    //core_4.2				//	9
#define					LL_LENGTH_RSP				0x15    //core_4.2				//	9
#define 				LL_PHY_REQ					0x16	//core_5.0				//	3
#define 				LL_PHY_RSP					0x17	//core_5.0				//	3
#define 				LL_PHY_UPDATE_IND			0x18	//core_5.0				//	5
#define 				LL_MIN_USED_CHN_IND			0x19	//core_5.0				//	3

#define 				LL_CTE_REQ					0x1A	//core_5.1				//	2
#define 				LL_CTE_RSP					0x1B	//core_5.1				//	2
#define 				LL_PERIODIC_SYNC_IND		0x1C	//core_5.1				// 35
#define 				LL_CLOCK_ACCURACY_REQ		0x1D	//core_5.1				//	2
#define 				LL_CLOCK_ACCURACY_RSP		0x1E	//core_5.1				//	2


#define 				LL_CIS_REQ					0x1F	//core_5.2				//	36
#define 				LL_CIS_RSP					0x20	//core_5.2				//	9
#define 				LL_CIS_IND					0x21	//core_5.2				//  16
#define 				LL_CIS_TERMINATE_IND		0x22	//core_5.2				//	4
#define 				LL_POWER_CONTROL_REQ		0x23	//core_5.2				//	4
#define 				LL_POWER_CONTROL_RSP		0x24	//core_5.2				//	5
#define 				LL_POWER_CHANGE_IND			0x25	//core_5.2				//	5

/////////////////////////////// Link Layer: ADV //////////////////////////////////////////////

// Advertise channel PDU Type
typedef enum advChannelPDUType_e {
	LL_TYPE_ADV_IND 		 = 0x00,
	LL_TYPE_ADV_DIRECT_IND 	 = 0x01,
	LL_TYPE_ADV_NONCONN_IND  = 0x02,
	LL_TYPE_SCAN_REQ 		 = 0x03,		LL_TYPE_AUX_SCAN_REQ 	 = 0x03,
	LL_TYPE_SCAN_RSP 		 = 0x04,
	LL_TYPE_CONNNECT_REQ 	 = 0x05,		LL_TYPE_AUX_CONNNECT_REQ = 0x05,
	LL_TYPE_ADV_SCAN_IND 	 = 0x06,

	LL_TYPE_ADV_EXT_IND		 = 0x07,		LL_TYPE_AUX_ADV_IND 	 = 0x07,	LL_TYPE_AUX_SCAN_RSP = 0x07,	LL_TYPE_AUX_SYNC_IND = 0x07,	LL_TYPE_AUX_CHAIN_IND = 0x07,
	LL_TYPE_AUX_CONNNECT_RSP = 0x08,
} advChannelPDUType_t;











/******************************************** HCI **************************************************************/
/**
 *  @brief  Definition for HCI request type
 */
#if 0 // comment by qifa in mesh: define in hci.h
typedef enum hci_type_e {
	HCI_TYPE_CMD = 0x01,
	HCI_TYPE_ACL_DATA,
	HCI_TYPE_SCO_DATA,
	HCI_TYPE_EVENT,
} hci_type_t;
#endif


#define 		HCI_FIRST_NAF_PACKET             			0x00	//LE Host to Controller
#define 		HCI_CONTINUING_PACKET             			0x01	//LE Host to Controller / Controller to Host
#define 		HCI_FIRST_AF_PACKET               			0x02	//LE 					  Controller to Host














/******************************************** L2CAP **************************************************************/



// l2cap pb flag type
#define L2CAP_FRIST_PKT_H2C              0x00
#define L2CAP_CONTINUING_PKT             0x01
#define L2CAP_FIRST_PKT_C2H              0x02


// define the l2cap CID for BLE
#define L2CAP_CID_NULL                   0x0000
#define L2CAP_CID_ATTR_PROTOCOL          0x0004
#define L2CAP_CID_SIG_CHANNEL            0x0005
#define L2CAP_CID_SMP                    0x0006
#define L2CAP_CID_GENERIC                0x0007
#define L2CAP_CID_DYNAMIC                0x0040


//signal packet formats of code define
#define L2CAP_CMD_REJECT                 0x01
#define L2CAP_CMD_DISC_CONN_REQ          0x06
#define L2CAP_CMD_DISC_CONN_RESP         0x07
#define L2CAP_CMD_CONN_UPD_PARA_REQ      0x12
#define L2CAP_CMD_CONN_UPD_PARA_RESP     0x13
#define L2CAP_CMD_CONN_REQ               0x14
#define L2CAP_CMD_CONN_RESP              0x15
#define L2CAP_CMD_FLOW_CTRL_CRED         0x16


#define L2CAP_SIGNAL_MSG_TYPE            1
#define L2CAP_DATA_MSG_TYPE              2
#define L2CAP_REJECT_CMD_NOT_UNDERSTOOD  0
#define L2CAP_REJECT_SIGNAL_MTU_EXCEED   1
#define L2CAP_REJECT_INVALID_CID         2






/******************************************** ATT ***************************************************************/









/******************************************** GATT **************************************************************/






/******************************************** GAP ***************************************************************/

// https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/
// EIR Data Type, Advertising Data Type (AD Type) and OOB Data Type Definitions

typedef enum {
	DT_FLAGS								= 0x01,		//	Flag
	DT_INCOMPLT_LIST_16BIT_SERVICE_UUID		= 0x02,		//	Incomplete List of 16-bit Service Class UUIDs
	DT_COMPLETE_LIST_16BIT_SERVICE_UUID	    = 0x03,		//	Complete List of 16-bit Service Class UUIDs
	DT_INCOMPLT_LIST_32BIT_SERVICE_UUID    	= 0x04,		//	Incomplete List of 32-bit Service Class UUIDs
	DT_COMPLETE_LIST_32BIT_SERVICE_UUID		= 0x05,		//	Complete List of 32-bit Service Class UUIDs
	DT_INCOMPLT_LIST_128BIT_SERVICE_UUID   	= 0x06,		//	Incomplete List of 128-bit Service Class UUIDs
	DT_COMPLETE_LIST_128BIT_SERVICE_UUID	= 0x07,		//	Complete List of 128-bit Service Class UUIDs
	DT_SHORTENED_LOCAL_NAME					= 0x08,		//	Shortened Local Name
	DT_COMPLETE_LOCAL_NAME					= 0x09,		//	Complete Local Name
	DT_TX_POWER_LEVEL						= 0x0A,		//	Tx Power Level

	DT_CLASS_OF_DEVICE						= 0x0D,		//	Class of Device

	DT_APPEARANCE							= 0x19,		//	Appearance

	DT_BIGINFO								= 0x2C,		//	BIGInfo
	DT_BROADCAST_CODE						= 0x2D,		// 	Broadcast_Code
	DT_3D_INFORMATION_DATA					= 0x3D,		//	3D Information Data

	DATA_TYPE_MANUFACTURER_SPECIFIC_DATA 	= 0xFF,     //	Manufacturer Specific Data
}data_type_t;


#endif
