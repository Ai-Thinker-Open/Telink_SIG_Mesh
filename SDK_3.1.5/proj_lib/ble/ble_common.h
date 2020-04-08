/********************************************************************************************************
 * @file     ble_common.h 
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
#pragma once
#if !WIN32
#include "proj/mcu/config.h"
#include "proj/tl_common.h"
#endif
#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258))
#include "stack/ble/ble_common.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "stack/ble_8278/ble_common.h"
#else

/*********************************************************************
 * CONSTANTS
 */


/**
 *  @brief  Definition for BLE Address Constants
 */

#define BLE_ACCESS_ADDR_LEN              4

/**
 *  @brief  Definition for BLE Random Number Size
 */
#define BLE_RANDOM_NUM_SIZE              8

/**
 *  @brief  Definition for BLE Invalid connection handle value
 */
#define BLE_INVALID_CONNECTION_HANDLE    0xffff

#define IS_CONNECTION_HANDLE_VALID(handle)  ( handle != BLE_INVALID_CONNECTION_HANDLE )



/**
 *  @brief  Definition for Link Layer Feature Support
 */
#define LL_FEATURE_SIZE                                      8
#define LL_FEATURE_MASK_LL_ENCRYPTION                        0x01   //core_4.0
#define LL_FEATURE_MASK_CONNECTION_PARA_REQUEST_PROCEDURE  	 0x02
#define LL_FEATURE_MASK_EXTENDED_REJECT_INDICATION           0x04
#define LL_FEATURE_MASK_SLAVE_INITIATED_FEATURES_EXCHANGE    0x08
#define LL_FEATURE_MASK_LE_PING                              0x10   //core_4.1
#define LL_FEATURE_MASK_LE_DATA_PACKET_EXTENSION             0x20
#define LL_FEATURE_MASK_LL_PRIVACY                           0x40
#define LL_FEATURE_MASK_EXTENDED_SCANNER_FILTER_POLICIES     0x80   //core_4.2


#if (BLE_CORE42_DATA_LENGTH_EXTENSION_ENABLE)

	#define LL_FEATURE_MASK_DEFAULT		(  LL_FEATURE_MASK_LL_ENCRYPTION                      |   \
									   	   LL_FEATURE_MASK_SLAVE_INITIATED_FEATURES_EXCHANGE  |   \
									   	   LL_FEATURE_MASK_LE_PING							  |   \
									   	   LL_FEATURE_MASK_LE_DATA_PACKET_EXTENSION	)
#else

	#define LL_FEATURE_MASK_DEFAULT		(  LL_FEATURE_MASK_LL_ENCRYPTION                      |   \
									   	   LL_FEATURE_MASK_SLAVE_INITIATED_FEATURES_EXCHANGE  |   \
									   	   LL_FEATURE_MASK_LE_PING					)
#endif


typedef enum {
	SCAN_TYPE_PASSIVE = 0x00,
	SCAN_TYPE_ACTIVE,
} scan_type_t;






#define 		ADV_INTERVAL_3_125MS                        5
#define 		ADV_INTERVAL_3_75MS                         6
#define 		ADV_INTERVAL_10MS                           16
#define 		ADV_INTERVAL_15MS                           24
#define 		ADV_INTERVAL_20MS                           32
#define 		ADV_INTERVAL_24MS                           38
#define 		ADV_INTERVAL_25MS                           40
#define 		ADV_INTERVAL_30MS                           48
#define 		ADV_INTERVAL_35MS                           56
#define 		ADV_INTERVAL_40MS                           64
#define 		ADV_INTERVAL_45MS                           72
#define 		ADV_INTERVAL_50MS                           80
#define 		ADV_INTERVAL_55MS                           88

#define 		ADV_INTERVAL_100MS                          160
#define 		ADV_INTERVAL_105MS                          168
#define 		ADV_INTERVAL_200MS                          320
#define 		ADV_INTERVAL_205MS                          328
#define 		ADV_INTERVAL_300MS                          480
#define 		ADV_INTERVAL_305MS                          488
#define 		ADV_INTERVAL_400MS                          640
#define 		ADV_INTERVAL_405MS                          648
#define 		ADV_INTERVAL_500MS                          800
#define 		ADV_INTERVAL_505MS                          808

#define 		ADV_INTERVAL_1S                          	1600
#define 		ADV_INTERVAL_1_28_S                         0x0800
#define			ADV_INTERVAL_2_56_S							0x1000
#define 		ADV_INTERVAL_10_24S                         16384

#define 		ADV_LOW_LATENCY_DIRECT_INTERVAL             ADV_INTERVAL_10MS
#define 		ADV_HIGH_LATENCY_DIRECT_INTERVAL            ADV_INTERVAL_3_75MS


#define 		SCAN_INTERVAL_10MS                           16
#define 		SCAN_INTERVAL_30MS                           48
#define 		SCAN_INTERVAL_60MS                           96
#define 		SCAN_INTERVAL_90MS                           144
#define 		SCAN_INTERVAL_100MS                          160
#define 		SCAN_INTERVAL_200MS                          320
#define 		SCAN_INTERVAL_300MS                          480


#define 		CONN_INTERVAL_7P5MS                          6
#define 		CONN_INTERVAL_10MS                           8
#define 		CONN_INTERVAL_15MS                           12
#define 		CONN_INTERVAL_18P75MS                        15
#define 		CONN_INTERVAL_20MS                           16
#define 		CONN_INTERVAL_30MS                           24
#define 		CONN_INTERVAL_48P75MS                        39
#define 		CONN_INTERVAL_50MS                           40
#define 		CONN_INTERVAL_100MS                          80


#define 		CONN_TIMEOUT_500MS							 50
#define 		CONN_TIMEOUT_1S							 	 100
#define 		CONN_TIMEOUT_4S							 	 400
#define 		CONN_TIMEOUT_10S							 1000
#define 		CONN_TIMEOUT_20S							 2000


/*********************************************************************
 * ENUMS
 */
typedef enum {
    BLE_SUCCESS = 0,
    
    // HCI Status, Per the Bluetooth Core Specification, V4.0.0, Vol. 2, Part D.
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
    HCI_ERR_DIRECTED_ADV_TIMEOUT                                   = 0x3C,
    HCI_ERR_CONN_TERM_MIC_FAILURE                                  = 0x3D,
    HCI_ERR_CONN_FAILED_TO_ESTABLISH                               = 0x3E,
    HCI_ERR_MAC_CONN_FAILED                                        = 0x3F,
    

    //telink define
    HCI_ERR_CONTROLLER_TX_FIFO_NOT_ENOUGH						   = HCI_ERR_CONTROLLER_BUSY,  //0x3A
    HCI_ERR_CONN_NOT_ESTABLISH									   = HCI_ERR_CONN_FAILED_TO_ESTABLISH,  //0x3E
	HCI_ERR_CURRENT_STATE_NOT_SUPPORTED_THIS_CMD 				   = HCI_ERR_CONTROLLER_BUSY,
    


    LL_ERR_START = 0x50,
	LL_ERR_WHITE_LIST_PUBLIC_ADDR_TABLE_FULL,                        //!< The white list public addr table full
	LL_ERR_WHITE_LIST_PRIVATE_RESOLVABLE_IRK_TABLE_FULL,
	LL_EER_FEATURE_NOT_SUPPORTED,
	LL_ERR_SUPVERVISION_TIMEOUT,
	LL_ERR_IRK_NOT_FOUND_FOR_RANDOM_ADDR,
	LL_ERR_ADDR_NOT_EXIST_IN_WHITE_LIST,
	LL_ERR_ADDR_ALREADY_EXIST_IN_WHITE_LIST,
	LL_ERR_WHITE_LIST_NV_DISABLED,
	LL_ERR_CURRENT_DEVICE_ALREADY_IN_CONNECTION_STATE,
	LL_ERR_CURRENT_STATE_NOT_SUPPORTED_THIS_CMD,

    
    
    L2CAP_ERR_START = 0x60,
    L2CAP_ERR_MUX_EXCCED,                                //!< The AUTOPEND pending all is turned on
    L2CAP_ERR_INVALID_PACKET_LEN,                        //!< The AUTOPEND pending all is turned off
    L2CAP_ERR_BEACON_LOSS,                               //!< The beacon was lost following a synchronization request
    L2CAP_ERR_CHANNEL_ACCESS_FAILURE,                    //!< The operation or data request failed because of activity on the channel
    L2CAP_ERR_DENIED,                                    //!< The l2cap was not able to enter low power mode
    L2CAP_ERR_INVALID_HANDLE,                            //!< The purge request contained an invalid handle
    L2CAP_ERR_INVALID_PARAMETER,                         //!< The API function parameter is out of range
    L2CAP_ERR_UNSUPPORTED,                               //!< The operation is not supported in the current configuration
    L2CAP_ERR_BAD_STATE,                                 //!< The operation could not be performed in the current state
    L2CAP_ERR_NO_RESOURCES,                              //!< The operation could not be completed because no memory resources were available
    L2CAP_ERR_TIME_OUT,                                  //!< The operation is time out
    L2CAP_ERR_NO_HANDLER,                                //!< No handle
    L2CAP_ERR_LEN_NOT_MATCH,                             //!< length not match
    
    
    ATT_ERR_START = 0x70,
    ATT_ERR_INVALID_HANDLE,                              //!< The attribute handle given was not valid on this server
    ATT_ERR_READ_NOT_PERMITTED,                          //!< The attribute cannot be read
    ATT_ERR_WRITE_NOT_PERMITTED,                         //!< The attribute cannot be written
    ATT_ERR_INVALID_PDU,                                 //!< The attribute PDU was invalid
    ATT_ERR_INSUFFICIENT_AUTH,                           //!< The attribute requires authentication before it can be read or written
    ATT_ERR_REQ_NOT_SUPPORTED,                           //!< Attribute server does not support the request received from the client
    ATT_ERR_INVALID_OFFSET,                              //!< Offset specified was past the end of the attribute
    ATT_ERR_INSUFFICIENT_AUTHOR,                         //!< The attribute requires authorization before it can be read or written
    ATT_ERR_PREPARE_QUEUE_FULL,                          //!< Too many prepare writes have been queued
    ATT_ERR_ATTR_NOT_FOUND,                              //!< No attribute found within the given attribute handle range
    ATT_ERR_ATTR_NOT_LONG,                               //!< The attribute cannot be read or written using the Read Blob Request
    ATT_ERR_INSUFFICIENT_KEY_SIZE,                       //!< The Encryption Key Size used for encrypting this link is insufficient
    ATT_ERR_INVALID_ATTR_VALUE_LEN,                      //!< The attribute value length is invalid for the operation
    ATT_ERR_UNLIKELY_ERR,                                //!< The attribute request that was requested has encountered an error that was unlikely, and therefore could not be completed as requested
    ATT_ERR_INSUFFICIENT_ENCRYPT,                        //!< The attribute requires encryption before it can be read or written
    ATT_ERR_UNSUPPORTED_GRP_TYPE,                        //!< The attribute type is not a supported grouping attribute as defined by a higher layer specification
    ATT_ERR_INSUFFICIENT_RESOURCES,                      //!< Insufficient Resources to complete the request
    ATT_ERR_ATTR_NUMBER_INVALID,                         //!< The attr number is 0 or too large to register
    ATT_ERR_ENQUEUE_FAILED,                              //!< register service failed when enqueue
    ATT_ERR_PREVIOUS_INDICATE_DATA_HAS_NOT_CONFIRMED,
    ATT_ERR_INVALID_PARAMETER,
	ATT_ERR_SERVICE_DISCOVERY_TIEMOUT,
    ATT_ERR_NOTIFY_INDICATION_NOT_PERMITTED,
    ATT_ERR_DATA_PENDING_DUE_TO_SERVICE_DISCOVERY_BUSY,



    GAP_ERR_START = 0x90,
    GAP_ERR_INVALID_ROLE,
    GAP_ERR_MEMORY_ERROR,
    GAP_ERR_INVALID_STATE,
    GAP_ERR_INVALID_PARAMETER,
    GAP_ERR_LISTENER_FULL,
    GAP_ERR_ITEM_NOT_FOUND,

   	SERVICE_ERR_START,
    SERVICE_ERR_INVALID_PARAMETER,
	SERVICE_ERR_NOTI_NOT_PERMITTED,
    
    SMP_EER_MUX_EXCCED = 0xA0,                          //!< The AUTOPEND pending all is turned on
    SMP_EER_INVALID_PACKET_LEN,                         //!< The AUTOPEND pending all is turned off 
    SMP_EER_INVALID_STATE,                              //!< received cmd in invalid state 
    SMP_EER_USER_CANCEL,                                //!< user channcel status  
    SMP_EER_SEC_FAILED,                                 //!< The l2cap was not able to enter low power mode. 
    SMP_EER_INVALID_HANDLE,                             //!< The purge request contained an invalid handle 
    SMP_EER_INVALID_PARAMETER,                          //!< The API function parameter is out of range 
    SMP_EER_UNSUPPORTED,                                //!< The operation is not supported in the current configuration 
    SMP_EER_BAD_STATE,                                  //!< The operation could not be performed in the current state 
    SMP_EER_NO_RESOURCES,                               //!< The operation could not be completed because no memory resources were available 
    SMP_EER_TIME_OUT,                                   //!< The operation is time out 
    SMP_EER_NO_HANDLER,                                 //!< The operation is time out 
    SMP_EER_LEN_NOT_MATCH,                              //!< The operation is time out 
    SMP_EER_NOT_FOUND,                                  //!< The operation is time out 
    SMP_EER_LINK_IS_ENCY,
    SMP_EER_PAIRING_IS_GOING_ON,
    SMP_EER_SIG_VERIFY_FAIL,                            //!< The operation is time out 
    SMP_EER_SIG_FAIL,                                   //!< The singature is failed
    SMP_EER_NO_SIGN_KEY,
    SMP_EER_ADDR_RESOLVE_FAIL,                          //!< The operation is time out 

    SPP_ERR_START = 0xC0,
    SPP_ERR_NO_HANDLER,
    

	BLE_COMMON_ERR_START = 0xD0,
    BLE_ERR_DUPLICATE_PACKET,
	BLE_ERR_INVALID_STATE,
	BLE_ERR_INVALID_PARAMETER,
	BLE_ERR_NO_RESOURCE,

	NO_BONDED_MAC_ADDRESS_FOR_DIRCET_ADV,



	SLAVE_TERMINATE_CONN_ACKED = 0xF0,
	SLAVE_TERMINATE_CONN_TIMEOUT    = 0xF1,

} ble_sts_t;





#define BLE_ADDR_LEN                     6

//Definition for BLE Common Address Type
 /* Device Address Type */
#define BLE_ADDR_PUBLIC                  0
#define BLE_ADDR_RANDOM                  1
#define BLE_ADDR_STATIC                  1
#define BLE_ADDR_PRIVATE_NONRESOLVE      2
#define BLE_ADDR_PRIVATE_RESOLVE         3
#define BLE_ADDR_INVALID                 0xff

//static
#define IS_STATIC_ADDR(type, addr)  					( (type)==BLE_ADDR_RANDOM && (addr[5] & 0xC0) == 0xC0 )

//non-resolvable private
#define IS_NON_RESOLVABLE_PRIVATE_ADDR(type, addr)  	( (type)==BLE_ADDR_RANDOM && (addr[5] & 0xC0) == 0x00 )

//resolvable private
#define IS_RESOLVABLE_PRIVATE_ADDR(type, addr)  		( (type)==BLE_ADDR_RANDOM && (addr[5] & 0xC0) == 0x40 )



typedef enum{
	OWN_ADDRESS_PUBLIC = 0,
	OWN_ADDRESS_RANDOM = 1,
	OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC = 2,
	OWN_ADDRESS_RESOLVE_PRIVATE_RANDOM = 3,
}own_addr_type_t;


/*********************************************************************
 * TYPES
 */




/////////////////////////////  PARING HEAD /////////////////////////////////////////////
#define			BLE_GATT_OP_PAIR_REQ			1
#define			BLE_GATT_OP_PAIR_RSP			2
#define			BLE_GATT_OP_PAIR_REJECT			3
#define			BLE_GATT_OP_PAIR_NETWORK_NAME	4
#define			BLE_GATT_OP_PAIR_NETWORK_PASS	5
#define			BLE_GATT_OP_PAIR_NETWORK_LTK	6
#define			BLE_GATT_OP_PAIR_CONFIRM		7
#define			BLE_GATT_OP_PAIR_LTK_REQ		8
#define			BLE_GATT_OP_PAIR_LTK_RSP		9
#define			BLE_GATT_OP_PAIR_DELETE			10
#define			BLE_GATT_OP_PAIR_DEL_RSP		11
#define			BLE_GATT_OP_ENC_REQ				12
#define			BLE_GATT_OP_ENC_RSP				13
#define			BLE_GATT_OP_ENC_FAIL			14
#define			BLE_GATT_OP_ENC_READY			15

///////////////////////////// SMP ///////////////////////////////////
#define			SMP_OP_PAIRING_REQ					1
#define			SMP_OP_PAIRING_RSP					2
#define			SMP_OP_PAIRING_CONFIRM				3
#define			SMP_OP_PAIRING_RANDOM				4
#define			SMP_OP_PAIRING_FAIL					5
#define			SMP_OP_ENC_INFO						6
#define			SMP_OP_ENC_IDX						7
#define			SMP_OP_ENC_IINFO					8
#define			SMP_OP_ENC_IADR						9
#define			SMP_OP_ENC_SIGN						0x0a
#define			SMP_OP_SEC_REQ						0x0b
#define			SMP_OP_PARING_PUBLIC_KEY			0x0c
#define			SMP_OP_PARING_DHKEY					0x0d
#define			SMP_OP_KEYPRESS_NOTIFICATION		0x0e
#define			SMP_OP_WAIT							0x0f

#define			SMP_OP_ENC_END						0xFF



// Advertise channel PDU Type
typedef enum advChannelPDUType_e {
	LL_TYPE_ADV_IND = 0x00,
	LL_TYPE_ADV_DIRECT_IND = 0x01,
	LL_TYPE_ADV_NONCONN_IND = 0x02,
	LL_TYPE_SCAN_REQ = 0x03,
	LL_TYPE_SCAN_RSP = 0x04,
	LL_TYPE_CONNNECT_REQ = 0x05,
	LL_TYPE_ADV_SCAN_IND = 0x06,
} advChannelPDUType_t;


/* Advertisement Type */
typedef enum{
  ADV_TYPE_CONNECTABLE_UNDIRECTED             = 0x00,  // ADV_IND
  ADV_TYPE_CONNECTABLE_DIRECTED_HIGH_DUTY     = 0x01,  // ADV_INDIRECT_IND (high duty cycle)
  ADV_TYPE_SCANNABLE_UNDIRECTED               = 0x02 , // ADV_SCAN_IND
  ADV_TYPE_NONCONNECTABLE_UNDIRECTED          = 0x03 , // ADV_NONCONN_IND
  ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY      = 0x04,  // ADV_INDIRECT_IND (low duty cycle)
}advertising_type;



// Advertise report event type
typedef enum {
	ADV_REPORT_EVENT_TYPE_ADV_IND = 0x00,
	ADV_REPORT_EVENT_TYPE_DIRECT_IND = 0x01,
	ADV_REPORT_EVENT_TYPE_SCAN_IND = 0x02,
	ADV_REPORT_EVENT_TYPE_NONCONN_IND = 0x03,
	ADV_REPORT_EVENT_TYPE_SCAN_RSP = 0x04,
} advReportEventType_t;


typedef struct {
	u8 type   :4;
	u8 rfu1   :2;
	u8 txAddr :1;
	u8 rxAddr :1;
}rf_adv_head_t;


typedef struct {
    u8 llid   :2;
    u8 nesn   :1;
    u8 sn     :1;
    u8 md     :1;
    u8 rfu1   :3;
}rf_data_head_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
}rf_packet_head_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
}rf_packet_auto_reply_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	rf_adv_head_t  header;	//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)

	u8	advA[6];			//address
#if (TEST_LONG_ADV_PACKET)
	u8	data[80];
#else
	u8	data[31];			//0-31 byte
#endif
}rf_packet_adv_t;


typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	rf_adv_head_t  header;	//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)

	u8	advA[6];			//slave address
	u8	initA[6];			//master address
}rf_packet_direct_adv_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	rf_adv_head_t  header;	//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)

	u8	scanA[6];			//
	u8	advA[6];			//
}rf_packet_scan_req_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	rf_adv_head_t  header;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)

	u8	advA[6];			//address
	u8	data[31];			//0-31 byte
}rf_packet_scan_rsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4): connect request PDU
	u8  rf_len;				//LEN(6)_RFU(2)
	u8	scanA[6];			//
	u8	advA[6];			//
	u8	accessCode[4];		// access code
	u8	crcinit[3];
	u8	winSize;
	u16	winOffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u8	chm[5];
	u8	hop;				//sca(3)_hop(5)
}rf_packet_connect_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	rf_adv_head_t  header;				//RA(1)_TA(1)_RFU(2)_TYPE(4): connect request PDU
	u8  rf_len;				//LEN(6)_RFU(2)
	u8	scanA[6];			//
	u8	advA[6];			//
	u8	aa[4];				// access code
	u8	crcinit[3];
	u8	wsize;
	u16	woffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u8	chm[5];
	u8	hop;				//sca(3)_hop(5)
}rf_packet_ll_init_t;

typedef struct {
	u8	type;
	u8  rf_len;
	u8 	opcode;
	u8 	winSize;
	u16 winOffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u16 instant;
} rf_packet_ll_updateConnPara_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4): connect request PDU
	u8  rf_len;				//LEN(6)_RFU(2)

	u8	scanA[6];			//
	u8	advA[6];			//
	u8	aa[4];				// access code
	u8	crcinit[3];
	u8	wsize;
	u16	woffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u8	chm[5];
	u8	hop;				//sca(3)_hop(5)
}rf_packet_relay_t;


typedef struct {
	u16 intervalMin;      // Minimum advertising interval for non-directed advertising, time = N * 0.625ms
	u16 intervalMax;      // Maximum advertising interval for non-directed advertising, time = N * 0.625ms
	u8  advType;          // Advertising
	u8  ownAddrType;
	u8  peerAddrType;
	u8  peerAddr[BLE_ADDR_LEN];
	u8  advChannelMap;
	u8  advFilterPolicy;
} adv_para_t;

typedef struct {
	u16 connHandle;
	u16 connIntervalMin;
	u16 connIntervalMax;
	u16 connLatency;
	u16 supervisionTimeout;
	u16 minCELen;
	u16 maxCELen;
} conn_para_t;



/*
LLID(2) - NESN(1) - SN(1) - MD(1) - RFU(3) - Length(5) - RFU(3)
*/


typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u16	l2capLen;
	u16	chanId;
}rf_packet_l2cap_head_t;


typedef struct{
	rf_data_head_t	header;
	u8  rf_len;
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 data[1];
}rf_packet_l2cap_t;


typedef struct{
	rf_data_head_t	header;
	u8  rf_len;
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  handle0;
	u8  handle1;
	u8	dat[20];
}rf_packet_att_t;


typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 data[1];
}rf_packet_l2cap_req_t;


typedef struct{
	u32 dma_len;
	u8	type;
	u8  rf_len;
	u16	l2capLen;
	u16	chanId;
	u8  code;
	u8  id;
	u16 dataLen;
	u16  result;
}rf_pkt_l2cap_sig_connParaUpRsp_t;


typedef struct{
	u8	type;
	u8  rf_len;
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 data[1];
}rf_pkt_l2cap_req_t;


typedef struct{
	u8	llid;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8	id;
	u16 data_len;
	u16 min_interval;
	u16 max_interval;
	u16 latency;
	u16 timeout;
}rf_packet_l2cap_connParaUpReq_t;


typedef struct{
	u8	llid;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8	id;
	u16 data_len;
	u16 result;
}rf_packet_l2cap_connParaUpRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u8	data;
}rf_packet_l2cap_cust_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 	errOpcode;
	u16 errHandle;
	u8  errReason;
}rf_packet_att_errRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8	startingHandle;
	u8	startingHandle1;
	u8	endingHandle;
	u8	endingHandle1;
	u8	attType[2];				//
}rf_packet_att_readByType_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8	startingHandle;
	u8	startingHandle1;
	u8	endingHandle;
	u8	endingHandle1;
	u8	attType[2];
	u8  attValue[2];
}rf_packet_att_findByTypeReq_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u16 	data[1];
}rf_packet_att_findByTypeRsp_t;


typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 handle;
	u8 handle1;
}rf_packet_att_read_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 handle;
	u8 handle1;
	u8 offset;
	u8 offset1;
}rf_packet_att_readBlob_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 	value[22];
}rf_packet_att_readRsp_t;


typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  datalen;
	u8  data[1];			// character_handle / property / value_handle / value
}rf_pkt_att_readByTypeRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  datalen;
	u8  data[1];			// character_handle / property / value_handle / value
}rf_packet_att_readByTypeRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  datalen;
	u16  data[3];
}rf_packet_att_readByGroupTypeRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  format;
	u8  data[1];			// character_handle / property / value_handle / value
}rf_packet_att_findInfoReq_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 handle;
	u8 handle1;
	u8 value;
}rf_packet_att_write_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 handle[2];
	u8 data;
}rf_packet_att_notification_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u16	l2cap;				//0x17
	u16	chanid;				//0x04,

	u8	att;				//0x12 for master; 0x1b for slave
	u8	hl;					// assigned by master
	u8	hh;					//
	u8	sno;

	u8	ctype;
	u8	cmd[18];				//byte
}rf_packet_ll_write_data_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 mtu[2];
}rf_packet_att_mtu_t;

typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 mtu[2];
}rf_packet_att_mtu_exchange_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
}rf_packet_att_writeRsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u8 	opcode;
	u8	data[8];
}rf_packet_feature_rsp_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u8 	opcode;
	u8  mainVer;
	u16 vendor;
	u16 subVer;
}rf_packet_version_ind_t;

typedef struct {
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8 	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u8 	opcode;
	u8 	winSize;
	u16 winOffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u16 instant;
}rf_packet_connect_upd_req_t;

typedef struct {
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8 	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u8 	opcode;
	u8 	chm[5];
	u16 instant;
} rf_packet_chm_upd_req_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u8 	opcode;
	u8	data[1];
}rf_packet_ctrl_unknown_t;


typedef struct {
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RA(1)_TA(1)_RFU(2)_TYPE(4)
	u8  rf_len;				//LEN(6)_RFU(2)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  id;
	u16 siglen;
    u16 min_interval;
    u16 max_interval;
    u16 latency;
    u16 timeout;
}rf_packet_connParUpReq_t;

typedef struct {
	u8  valid;
	u8 	winSize;
	u16 winOffset;
	u16 interval;
	u16 latency;
	u16 timeout;
	u16 instant;
}connect_upd_data_t;

typedef struct {
	u8  valid;
	u8 	chm[5];
	u16 instant;
}connect_chm_upd_data_t;


typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u8	opcode;				//
	u8	reason;				//
}rf_packet_ll_terminate_t;


typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u8	opcode;				//
	u8	dat[1];				//
}rf_packet_ll_control_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2cap;
	u16	chanid;

	u8	att;
	u8	hl;					// assigned by master
	u8	hh;					//

	u8	dat[20];

}rf_packet_att_data_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	flag;

	u32	src_id;

	u8	att;				//0x12 for master; 0x1b for slave
	u8	hl;					// assigned by master
	u8	hh;					//
	u8	sno;

	u16	nid;				// network ID
	u16	group;

	u32	dst_id;

	u8	cmd[11];			//byte
							// 10 xx xx xx xx xx xx		=> light on
							// 11 xx xx xx xx xx xx		=> light off
							// 12 rr gg bb ww uu vv		=> set

	//u32	mic[4];			//optional
}rf_packet_ll_rc_data_t;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)

	u16	l2capLen;			// can be src_id
	u16	chanId;

	u8	att;				//0x12 for master; 0x1b for slave
	u8	hl;					// assigned by master
	u8	hh;					//
	u8	sno;

	u16	mic;				// network ID
	u16	group;

	u8	dst_id[4];

	u8	cmd[11];			//byte
							// 10 xx xx xx xx xx xx		=> light on
							// 11 xx xx xx xx xx xx		=> light off
							// 12 rr gg bb ww uu vv		=> set

	//u32	mic[4];			//optional
}rf_packet_mesh_data_phone_t;

typedef struct{
    u8 sno[3];
    u8 src[2];
    u8 dst[2];
    u8 op_para[13];
 //   u8 ttl;
 //   u8 hop;
}rf_packet_mesh_nwk_t;


typedef struct{
	u32 dma_len;            //29

	u8	rf_len;				//28
	u8  type;				//LEN(5)_RFU(3)

	u8	src_id[4];			// src_id

	u8	ttl;				// time to live
	u8	hl;					// assigned by master
	u8	hh;					//
	u8	sno;

	u16	mic;				// network ID
	u16	group;

	u8	dst_id[4];

	u8	cmd[11];			//byte
							// 10 xx xx xx xx xx xx		=> light on
							// 11 xx xx xx xx xx xx		=> light off
							// 12 rr gg bb ww uu vv		=> set

	//u32	mic[4];			//optional
}rf_packet_mesh_data_t1;

typedef struct{
	u32 dma_len;            //29

	u8	rf_len;				//28
	u8  type;				//LEN(5)_RFU(3)

	u16	l2cap;				// l2cap length
	u16	chan;				// channel ID

	u8	ttl;				// time to live
	u8	hl;					// assigned by master
	u8	hh;					//
	rf_packet_mesh_nwk_t	c;

}rf_packet_mesh_phone_t;

typedef struct{
	u32 dma_len;            //29

	u8	rf_len;				//28
	u8  type;				//LEN(5)_RFU(3)

	u16	l2cap;				// l2cap length
	u16	chan;				// channel ID

	u8	ttl;				// time to live
	u8	hl;					// assigned by master
	u8	hh;					//
	rf_packet_mesh_nwk_t	c;
	u8 rsv[6];

}rf_packet_mesh_data_t;

typedef struct{
	u32 dma_len;            //39

	u8	rf_len;				//38
	u8  type;				//LEN(5)_RFU(3)

	u16	l2cap;				// l2cap length
	u16	chan;				// channel ID

	u8	ttl;				// time to live
	u8	hl;					// assigned by master
	u8	hh;					//
	u8	dat[30];

}rf_packet_mesh_status_t;

typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  datalen;
	u8  data[1];			// character_handle / property / value_handle / value
}att_readByTypeRsp_t;

typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8  format;
	u8  data[1];			// character_handle / property / value_handle / value
}att_findInfoRsp_t;

typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 	value[22];
}att_readRsp_t;

typedef struct{
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8	handle;
	u8	hh;
	u8 	value[1];
}att_notify_t;

typedef struct {
	u8 type;
	u8 address[BLE_ADDR_LEN];
} addr_t;

typedef struct {
	u8 address[BLE_ADDR_LEN];
} public_addr_t;


////////////////////////////////////////////////////////////////////////////







typedef struct {
	u8	num;
	u8	property;
	u16	handle;
	u16	uuid;
	u16 ref;
} att_db_uuid16_t;			//8-byte


typedef struct {
	u8	num;
	u8	property;
	u16	handle;
	u8	uuid[16];
} att_db_uuid128_t;			//20-byte

//-------------   event --------------------------------
typedef struct {
	u8	status;
	u8	handle;
	u8	hh;
	u8	reason;
} event_disconnection_t;			//20-byte

typedef struct {
	u8	subcode;
	u8	nreport;
	u8	event_type;
	u8	adr_type;
	u8	mac[6];
	u8	len;
	u8	data[1];
} event_adv_report_t;			//20-byte

typedef struct {
	u8	subcode;
	u8	status;
	u16	handle;
	u8	role;
	u8	peer_adr_type;
	u8	mac[6];
	u16	interval;
	u16	latency;
	u16	timeout;
	u8	accuracy;
} event_connection_complete_t;			//20-byte

typedef struct {
	u8	subcode;
	u8	status;
	u16	handle;
	u16	interval;
	u16	latency;
	u16	timeout;
} event_connection_update_t;			//20-byte

typedef struct {
	u8	status;
	u16	handle;
	u8  enc_enable;
} event_enc_change_t;

typedef struct {
	u8	status;
	u16	handle;
} event_enc_refresh_t;
#endif

typedef struct {
	s8	rssi;       // have been -110
	s16	dc;
	u32	timeStamp;
} adv_report_extend_t;

#include "att.h"
#include "gap.h"
#include "uuid.h"
#include "hci/hci.h"

