/********************************************************************************************************
 * @file     homekit_inc.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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
//--#ifndef _HOMEKIT_INC_H_
//--#define _HOMEKIT_INC_H_

#pragma once

/* ------------------------------------
    Header Files
   ------------------------------------ */
//#include "stack/ble/attr/att.h"
#include "proj_lib/ble/att.h"
#include "hook_func.h"
//#include "../common/types.h"
#include "proj/common/types.h"
#include "homekit_def.h"
#include "hk_crc32_half.h"

/* ------------------------------------
    Macro Definitions
   ------------------------------------ */
#undef EXTERN

#ifdef _HOMEKIT_INC_C_
    #define EXTERN
#else
    #define EXTERN  extern
#endif

#define HK_UART_PRINT	0
#define SHOW_LogMsg     0
#define SHOW_FUNC       0
#if SHOW_FUNC
    #define LogFunc(x, y, z)  LogMsg((x), (y), (z))
#else
    #define LogFunc(x, y, z)
#endif

#if HK_UART_PRINT
	#define HK_UART_OUT		printf
#else
	#define HK_UART_OUT(x)
#endif

//////////////////////////////////////////////////////////////////
// additional supported MACRO.
/** @addtogroup GATT_CPF_Bits Characteristic Presentation Format bits
 * @{
 */
// FORMAT (1byte)
#define CLIENT_CHAR_CPF_BOOL				0x01
#define CLIENT_CHAR_CPF_UINT8				0x04
#define CLIENT_CHAR_CPF_UINT16				0x06
#define CLIENT_CHAR_CPF_UINT32				0x08
#define CLIENT_CHAR_CPF_UINT64				0x0A
//--#define CLIENT_CHAR_CPF_INT8				0x0C
//--#define CLIENT_CHAR_CPF_INT16				0x0E
#define CLIENT_CHAR_CPF_INT32				0x10
//--#define CLIENT_CHAR_CPF_INT64				0x12
#define CLIENT_CHAR_CPF_FLOAT32				0x14
//--#define CLIENT_CHAR_CPF_FLOAT64				0x15
//---#define CLIENT_CHAR_CPF_FLOAT				0x17
//--#define CLIENT_CHAR_CPF_UTF8S				0x19
#define CLIENT_CHAR_CPF_STRING              0x19
//--#define CLIENT_CHAR_CPF_UTF16S				0x1A
//--#define CLIENT_CHAR_CPF_OPAQUE				0x1B
#define CLIENT_CHAR_CPF_TLV8                0x1B
#define CLIENT_CHAR_CPF_DATA                0x1B
// EXPONENT (1byte)
//                                         Exponent
// Actual_Value = Characteristic_Value X 10
//
// UNIT (2bytes)
// NAMESPACE (1byte)
#define CLIENT_CHAR_CPF_NAMESPACE01			0x01
#define CLIENT_CHAR_CPF_NAMESPACE02			0x02
#define CLIENT_CHAR_CPF_NAMESPACE03			0x03
// DESCRIPTION (2bytes)
/** @} end of group GATT_CCCC_Bits */

//////////////////////////////////////////////////////////////////
#define HOMEKIT_PROTOCOL_SUPPORT 			1
/////////////////////////////////////////////////////////////////

#define USE_DEFAULT_MTU_SIZE				1	//default mtu_size = 0x17

#define BUFF_LEN_MAX		2304
#define BUFF_TX_LEN_MAX		2304


extern	u8			blt_buff_tx[BUFF_TX_LEN_MAX];
extern	u8			blt_buff_ll_pack[124];
#define PKT_READ_RSP    blt_buff_tx





#define UUID16_HOMEKIT_BASE     0xFE00
#define UUID96_HOMEKIT_BASE     0x91,0x52,0x76,0xBB,0x26,0x00,0x00,0x80,0x00,0x10,0x00,0x00

/////////////////////////////////////////////////////////////////
#define HAP_CHAR_PROP_READ              0x01
#define HAP_CHAR_PROP_WRITE             0x02
#define HAP_CHAR_PROP_AUTHOR_DATA       0x04
#define HAP_CHAR_PROP_TIMED_WRITE       0x08
#define HAP_CHAR_PROP_SECURE_READ       0x10
#define HAP_CHAR_PROP_SECURE_WRITE      0x20
#define HAP_CHAR_PROP_HIDDEN_FROM_USER  0x40
#define HAP_CHAR_PROP_NOTIFY_ENEVT_IN_CONNECTED_STATE       0x80
#define HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE    0x0100
#define HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY              0x0200

#define hap_readOnlyProp            (HAP_CHAR_PROP_READ)
#define hap_writeOnlyProp           (HAP_CHAR_PROP_WRITE)
#define hap_readWriteProp           (HAP_CHAR_PROP_WRITE | HAP_CHAR_PROP_READ)

#define hap_SecureW                 (HAP_CHAR_PROP_SECURE_WRITE)
#define hap_SecureW_and_writeProp   (HAP_CHAR_PROP_SECURE_WRITE | HAP_CHAR_PROP_WRITE)
#define hap_SecureR_and_readProp    (HAP_CHAR_PROP_SECURE_READ | HAP_CHAR_PROP_READ)
#define hap_SecureRW_Prop           (HAP_CHAR_PROP_SECURE_READ | HAP_CHAR_PROP_SECURE_WRITE)
#define hap_SecureRW_and_readWriteProp  (HAP_CHAR_PROP_WRITE | HAP_CHAR_PROP_READ | HAP_CHAR_PROP_SECURE_READ | HAP_CHAR_PROP_SECURE_WRITE)

#define hap_SecureR_10              (HAP_CHAR_PROP_SECURE_READ)
#define hap_SecureW_20              (HAP_CHAR_PROP_SECURE_WRITE)
#define hap_SecureRW_30             (HAP_CHAR_PROP_SECURE_WRITE | HAP_CHAR_PROP_SECURE_READ)
#define hap_SecureR_NOTIFY_90       (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_CONNECTED_STATE | HAP_CHAR_PROP_SECURE_READ)
#define hap_SecureW_NOTIFY_A0       (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_CONNECTED_STATE | HAP_CHAR_PROP_SECURE_WRITE)
#define hap_SecureRW_NOTIFY_B0      (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_CONNECTED_STATE | HAP_CHAR_PROP_SECURE_WRITE | HAP_CHAR_PROP_SECURE_READ)
#define hap_SecureR_NOTIFY_110      (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | HAP_CHAR_PROP_SECURE_READ)
#define hap_SecureRW_NOTIFY_130     (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | HAP_CHAR_PROP_SECURE_WRITE | HAP_CHAR_PROP_SECURE_READ)
#define hap_PRead_NOTIFY            (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | hap_SecureR_NOTIFY_90)
#define hap_PWrite_NOTIFY           (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | hap_SecureW_NOTIFY_A0)
#define hap_PReadWrite_NOTIFY       (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | hap_SecureRW_NOTIFY_B0)

#define hap_PR_PW_TW_NOTIFY         (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_TIMED_WRITE | hap_PReadWrite_NOTIFY)
#define hap_PReadWrite_AUTH_NOTIFY  (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_AUTHOR_DATA | hap_PReadWrite_NOTIFY)
#define hap_PR_PW_TW_AUTH_NOTIFY    (HAP_CHAR_PROP_SUPPORT_BROADCAST_NOTIFY | HAP_CHAR_PROP_AUTHOR_DATA | hap_PR_PW_TW_NOTIFY)

#define hap_PRead_NOTIFY_NO_BRC     (HAP_CHAR_PROP_NOTIFY_ENEVT_IN_DISCONNECTED_STATE | hap_SecureR_NOTIFY_90)

/* ------------------------------------
    Type Definitions
   ------------------------------------ */

#define HK_SETUP_CODE               "150-01-500"

#ifndef DEVICE_NAME
    #define DEVICE_NAME         "hkI6s"
#endif

#define accessoryPubkey         srpSecret           // share memory
#define controllerPubkey        (srpSecret + 32)


/* ------------------------------------
    Variables Definitions/Declarations
   ------------------------------------ */

enum{
    ADV_PKT_DATA_LEN            = 3,            // 0x16
    ADV_PKT_TYPE                = 7,            // 0x06
    ADV_PKT_SUBTYPE_AND_LEN     = 8,            // 0x31
    ADV_PKT_ADVERTISE_INTERVAL  = 8,            // ignore, since HAP R10
    ADV_PKT_PAIR_STATUS_FLAG    = 9,
    ADV_PKT_DEVICE_ID           = 10,
    ADV_PKT_CATEGORY_FIELD      = 16,
    ADV_PKT_GLOBAL_STATE_NUM    = 18,
    ADV_PKT_FW_UPDATE_CNT_CN    = 20,
    ADV_PKT_HAP_VERSION         = 21,
    ADV_PKT_SETUP_HASH          = 22,
    ADV_PKT_LOCAL_NAME          = 26,         // local short name
    ADV_PKT_END                 = 0xFF
}HAP_BLE_2_0;

enum{
    ADV_ENC_DATA_LEN            = 3,            // 0x1b
    ADV_ENC_TYPE                = 7,            // 0x11
    ADV_ENC_SUBTYPE_AND_LEN     = 8,            // 0x36
    ADV_ENC_DEVICE_ID           = 9,
    ADV_ENC_GLOBAL_STATE_NUM    = 15,
    ADV_ENC_INSTANCE_ID         = 17,
    ADV_ENC_CHAR_VAL            = 19,
    ADV_ENC_AUTH_TAG            = 27,
    ADV_ENC_END                 = 0xFF
}HAP_BLE_ENCRYPED_ADV;


// === HAP Opcode ===
enum {
    HAP_CHAR_SIGN_READ          = 1,
    HAP_CHAR_WRITE              = 2,
    HAP_CHAR_READ               = 3,
    HAP_CHAR_TIMED_WRITE        = 4,
    HAP_CHAR_EXEC_WRITE         = 5,
    HAP_SERVICE_SIGN_READ       = 6,
    HAP_CHAR_CONFIG             = 7,
    HAP_PROTOCOL_CONFIG         = 8,

    /* HAP software authentication opcode. */
    HAP_TOKEN_REQ_RSP			= 0x10,
    HAP_TOKEN_UPDATE_REQ_RSP	= 0x11,
	HAP_TOKEN_INFO_REQ_RSP		= 0x12,
};


typedef enum{
    kTLVType_Method         = 0,
    kTLVType_Identifier     = 1,
    kTLVType_Salt           = 2,
    kTLVType_PublicKey      = 3,
    kTLVType_Proof          = 4,
    kTLVType_EncryptedData  = 5,
    kTLVType_State          = 6,
    kTLVType_Error          = 7,
    kTLVType_RetryDelay     = 8,
    kTLVType_Certificate    = 9,
    kTLVType_Signature      = 0xA,
    kTLVType_Permissions    = 0xB,
    kTLVType_FragmentData   = 0xC,
    kTLVType_FragmentLast   = 0xD,
    kTLVType_SessionID      = 0xE,
    kTLVType_Flags			= 0x13,
    kTLVType_Separator      = 0xFF,
}kTLVType_t;

typedef enum{
	kPairingFlag_Transient	= (1 << 4),		//0x00000010
	kPairingFlag_Split		= (1 << 24),	//0x01000000
}kPairing_Flags_t;


typedef enum{
	Method_Pair_Setup       		= 0,
    Method_Pair_Setup_with_Auth 	= 1,
    Method_Pair_Verify      		= 2,
    Method_Pair_Add         		= 3,
    Method_Pair_Remove      		= 4,
    Method_Pair_List        		= 5,
    Method_Pair_Resume      		= 6,
    Method_Pair_MAX,
} PairMethod_t;

#define KEYPAIR_ADD             0
#define KEYPAIR_REMOVE          1
#define KEYPAIR_FIND            2
#define KEYPAIR_FIND_PUBLICKEY_CHECK_PERMISSION     55

#define SESSION_ID_NUM			8	//min = 8
#define PID_COMPARE_LEN			4	//just compare 4 bytes to decide if sessionid is valid, it can be set up to 40 when it is need.


typedef struct
{
	u8 permission;
	u8 pId[PID_COMPARE_LEN];
	u8 sessionID[8];
	u8 sessionShareKey[32];
}session_type_t;

typedef struct
{
	u8 session_cnts;
	session_type_t session_content[SESSION_ID_NUM];
}session_t;


typedef enum{
    State_PS_M1_SRPStartRequest      = 1,
    State_PS_M2_SRPStartRespond      = 2,
    State_PS_M3_SRPVerifyRequest     = 3,
    State_PS_M4_SRPVerifyRespond     = 4,
    State_PS_M5_ExchangeRequest      = 5,
    State_PS_M6_ExchangeRespond      = 6,
//-------------
    State_Pair_Setup_M1_SRPStartRequest     = 1,
    State_Pair_Setup_M2_SRPStartRespond     = 2,
    State_Pair_Setup_M3_SRPVerifyRequest    = 3,
    State_Pair_Setup_M4_SRPVerifyRespond    = 4,
    State_Pair_Setup_M5_ExchangeRequest     = 5,
    State_Pair_Setup_M6_ExchangeRespond     = 6,
} PairSetupState_t;

typedef enum{
    State_Pair_Verify_M1          = 1,
    State_Pair_Verify_M2          = 2,
    State_Pair_Verify_M3          = 3,
    State_Pair_Verify_M4          = 4,
} PairVerifyState_t;

typedef enum{
    State_Pair_Pair_M1          = 1,
    State_Pair_Pair_M2          = 2,
    State_Pair_Pair_M3          = 3,
    State_Pair_Pair_M4          = 4,
} PairPairState_t;

typedef enum{
    kTLVError_UnknownErr        = 1,    // Generic error to handle unexpected errors
    kTLVError_Authentication    = 2,    // Setup code or signature verfication failed
    kTLVError_Backoff           = 3,    // Client must look at the retry delay TLV item and wait that many seconds before retrying
    kTLVError_MaxPeers          = 4,    // Server cannot accept any more pairings
    kTLVError_MaxTries          = 5,    // Server reached its maximum number of authentication attempts
    kTLVError_Unavailable       = 6,    // Server pairing method is unavailable
    kTLVError_Busy              = 7,    // Server is busy and cannot accept a pairing request at this time
}PairErrorCode_t;

typedef enum{
    Type_Data_Without_Length      = 1,
    Type_Data_With_Length         = 2,
} Poly1305Type_t;


// === sw auth token set flag ===
enum{
	SW_AUTH_TOKEN_INIT			= 0x01,
	SW_AUTH_TOKEN_UPDATE		= 0x02,
};

// === sw auth uuid set flag ===
 enum{
	SW_AUTH_UUID_INIT			= 0x01,
	SW_AUTH_UUID_UPDATE 		= 0x02,
};


// ===== Software Authentication TLV types ===
enum {
	SW_TLV_UUID = 0x01,
	SW_TLV_SW_TOKEN = 0x02,
};

// ===== Software Authentication Info Parameter Types ===
enum {
	SW_HAP_PARAM_CURRENT_STATE_NUMBER	= 0x01,
	SW_HAP_PARAM_CURRENT_CONFIG_NUMBER	= 0x02,
	SW_HAP_PARAM_DEVICE_IDENTIFIER		= 0x03,
	SW_HAP_PARAM_FEATURE_FLAGS			= 0x04,
	SW_HAP_PARAM_MODEL_NAME				= 0x05,
	SW_HAP_PARAM_PROTOCOL_VERSION		= 0x06,
	SW_HAP_PARAM_STATUS_FLAG			= 0x07,
	SW_HAP_PARAM_CATEGORY_IDENTIFIER	= 0x08,
	SW_HAP_PARAM_SETUP_HASH				= 0x09,
};



#if (__PROJECT_HOMEKIT_EMULATOR__ == 1)
extern unsigned char decryptKey[32];
extern unsigned char encryptKey[32];
extern unsigned char srpSecret[64];
extern unsigned char accessoryLTPK[32];
#else
#if 0
unsigned char *decryptKey;      // unsigned char decryptKey[32];
unsigned char *encryptKey;      //unsigned char encryptKey[32];
unsigned char *srpSecret;       // unsigned char srpSecret[64];
unsigned char *accessoryLTPK;   // unsigned char accessoryLTPK[32];
#else
extern unsigned char decryptKey[32];
extern unsigned char encryptKey[32];
extern unsigned char srpSecret[64];
extern unsigned char accessoryLTPK[32];
#endif
#endif

#ifdef WIN32
unsigned char accessoryLTSK[32];
#endif

unsigned char sessionKey[32];

unsigned char accessoryPass[11];
unsigned char accessoryId[18];

extern unsigned char ReCheckADVStatusFlag;
extern const u8 model[24];
extern const u8 hap_ble_version[5];
extern const u8 my_pairFeature;
extern flash_adr_layout_def flash_adr_layout;

////////////  Apple defined Characteristic  //////////////////////////
extern const unsigned char charInstanceUUID[16];
extern const unsigned char serviceInstanceUUID[16];
extern const unsigned char HapBleInfoUUID[16];
extern const unsigned char HapServiceSignatureCharUUID[16];

// UUID of Information Service and Characteristics
extern const unsigned char informationServiceUUID[16];
extern const unsigned char identifyUUID[16];
extern const unsigned char manufacturerUUID[16];
extern const unsigned char modelUUID[16];
extern const unsigned char nameUUID[16];
extern const unsigned char serialNumberUUID[16];
extern const unsigned char FirmwareRevUUID[16];
extern const unsigned char HardwareRevUUID[16];
extern const unsigned char AccessoryFlagsUUID[16];

extern const hap_characteristic_desc_t hap_desc_ble_version;
//--extern const hap_characteristic_desc_t hap_desc_identify;
extern const hap_characteristic_desc_t hap_desc_identify;


// UUID of Pairing Service and Characteristics
extern const unsigned char pairingServiceUUID[16];
extern const unsigned char pairSetupUUID[16];
extern const unsigned char pairVerifyUUID[16];
extern const unsigned char pairFeatureUUID[16];
extern const unsigned char pairPairingUUID[16];

extern const hap_characteristic_desc_t hap_desc_pairSetup;
extern const hap_characteristic_desc_t hap_desc_pairVerify;
extern const hap_characteristic_desc_t hap_desc_pairFeature;
extern const hap_characteristic_desc_t hap_desc_pairPairing;


// UUID of Apple-defined Services
extern const unsigned char fanServiceUUID[16];
extern const unsigned char garageDoorServiceUUID[16];
extern const unsigned char lightBulbServiceUUID[16];
extern const unsigned char lockManagementServiceUUID[16];
extern const unsigned char lockMechanismServiceUUID[16];
extern const unsigned char outletServiceUUID[16];
extern const unsigned char switchServiceUUID[16];
extern const unsigned char thermostatServiceUUID[16];
extern const unsigned char sensor_air_qualityServiceUUID[16];
extern const unsigned char securityServiceUUID[16];
extern const unsigned char sensor_carbon_monoxideServiceUUID[16];
extern const unsigned char sensor_contactServiceUUID[16];
extern const unsigned char doorServiceUUID[16];
extern const unsigned char sensor_humidityServiceUUID[16];
extern const unsigned char sensor_leakServiceUUID[16];
extern const unsigned char sensor_lightServiceUUID[16];
extern const unsigned char sensor_motionServiceUUID[16];
extern const unsigned char sensor_occupancyServiceUUID[16];
extern const unsigned char sensor_smokeServiceUUID[16];
extern const unsigned char stateful_prog_swServiceUUID[16];     // Removed in HAP R9 Spec.
extern const unsigned char stateless_prog_swServiceUUID[16];
extern const unsigned char sensor_temperatureServiceUUID[16];
extern const unsigned char windowServiceUUID[16];
extern const unsigned char window_coveringServiceUUID[16];
extern const unsigned char batteryServiceUUID[16];
extern const unsigned char sensor_carbon_dioxideServiceUUID[16];
extern const unsigned char cam_rtp_str_managementServiceUUID[16];
extern const unsigned char microphoneServiceUUID[16];
extern const unsigned char speakerServiceUUID[16];
extern const unsigned char doorbellServiceUUID[16];
extern const unsigned char fan_v2ServiceUUID[16];
extern const unsigned char slatServiceUUID[16];
extern const unsigned char filter_maintenanceServiceUUID[16];
extern const unsigned char air_purifierServiceUUID[16];
extern const unsigned char heater_coolerServiceUUID[16];
extern const unsigned char humidifier_dehumidifierServiceUUID[16];
extern const unsigned char service_labelServiceUUID[16];
extern const unsigned char irrigation_systemServiceUUID[16];
extern const unsigned char valveServiceUUID[16];
extern const unsigned char faucetServiceUUID[16];


// UUID of Apple-defined Characteristics
extern const unsigned char charAdminOnlyAccessUUID[16];
extern const unsigned char charAudioFeedbackUUID[16];
extern const unsigned char charBrightnessUUID[16];
extern const unsigned char charCoolingThresholdTemperatureUUID[16];
extern const unsigned char charCurrentDoorStateUUID[16];
extern const unsigned char charCurrentHeatingCoolingStateUUID[16];
extern const unsigned char charCurrentRelativeHumidityUUID[16];
extern const unsigned char charCurrentTemperatureUUID[16];
extern const unsigned char charHeatingThresholdTemperatureUUID[16];
extern const unsigned char charHueUUID[16];
extern const unsigned char charLockControlPointUUID[16];
extern const unsigned char charAutoSecurityTimeoutUUID[16];
extern const unsigned char charLockLastKnownActionUUID[16];
extern const unsigned char charLockCurrentStateUUID[16];
extern const unsigned char charLockTargetStateUUID[16];
extern const unsigned char charLogsUUID[16];
extern const unsigned char charMotionDetectedUUID[16];
extern const unsigned char charObstructionDetectedUUID[16];
extern const unsigned char charOnUUID[16];
extern const unsigned char charOutletInUseUUID[16];
extern const unsigned char charRotationDirectionUUID[16];
extern const unsigned char charRotationSpeedUUID[16];
extern const unsigned char charSaturationUUID[16];
extern const unsigned char charTargetDoorStateUUID[16];
extern const unsigned char charTargetHeatingCoolingStateUUID[16];
extern const unsigned char charTargetRelativeHumidityUUID[16];
extern const unsigned char charTargetTemperatureUUID[16];
extern const unsigned char charTemperatureDisplayUnitsUUID[16];
extern const unsigned char charVersionUUID[16];
extern const unsigned char charAirParticulateDensityUUID[16];
extern const unsigned char charAirParticulateSizeUUID[16];
extern const unsigned char charSecurityCurrentStateUUID[16];
extern const unsigned char charSecurityTargetStateUUID[16];
extern const unsigned char charBatteryLevelUUID[16];
extern const unsigned char charCarbonMonoxideDetectedUUID[16];
extern const unsigned char charContactStateUUID[16];
extern const unsigned char charCurrentAmbientLightLevelUUID[16];
extern const unsigned char charCurrentHTA_UUID[16];
extern const unsigned char charCurrentPositionUUID[16];
extern const unsigned char charCurrentVTA_UUID[16];
extern const unsigned char charHoldPositionUUID[16];
extern const unsigned char charLeakDetectedUUID[16];
extern const unsigned char charOccupancyDetectedUUID[16];
extern const unsigned char charPositionStateUUID[16];
extern const unsigned char charProgSwEventUUID[16];
extern const unsigned char charProgSwOutputStateUUID[16];      // Removed in HAP R9 Spec.
extern const unsigned char charStatusActiveUUID[16];
extern const unsigned char charSmokeDetectedUUID[16];
extern const unsigned char charStatusFaultUUID[16];
extern const unsigned char charStatusLowBatteryUUID[16];
extern const unsigned char charStatusTamperedUUID[16];
extern const unsigned char charTargetHTA_UUID[16];
extern const unsigned char charTargetPositionUUID[16];
extern const unsigned char charTargetVTA_UUID[16];
extern const unsigned char charSecurityAlarmTypeUUID[16];
extern const unsigned char charChargingStateUUID[16];
extern const unsigned char charCarbonMonoxideLevelUUID[16];
extern const unsigned char charCarbonMonoxidePeakLevelUUID[16];
extern const unsigned char charCarbonDioxideDetectedUUID[16];
extern const unsigned char charCarbonDioxideLevelUUID[16];
extern const unsigned char charCarbonDioxidePeakLevelUUID[16];
extern const unsigned char charAirQualityIndicatorUUID[16];
extern const unsigned char charLockPhysicalControlsUUID[16];
extern const unsigned char charTargetAirPurifierStateUUID[16];
extern const unsigned char charCurrentAirPurifierStateUUID[16];
extern const unsigned char charCurrentSlatStateUUID[16];
extern const unsigned char charFilterLifeLevelUUID[16];
extern const unsigned char charFilterChangeIndicationUUID[16];
extern const unsigned char charResetFilterIndicationUUID[16];
extern const unsigned char charCurrentFanStateUUID[16];
extern const unsigned char charActiveUUID[16];
extern const unsigned char charCurrentHeaterCoolerStateUUID[16];
extern const unsigned char charTargetHeaterCoolerStateUUID[16];
extern const unsigned char charCurrentHumidifierDehumidifierStateUUID[16];
extern const unsigned char charTargetHumidifierDehumidifierStateUUID[16];
extern const unsigned char charWaterLevelUUID[16];
extern const unsigned char charSwingModeUUID[16];
extern const unsigned char charTargetFanStateUUID[16];
extern const unsigned char charSlatTypeUUID[16];
extern const unsigned char charCurrentTiltAngleUUID[16];
extern const unsigned char charTargetTiltAngleUUID[16];
extern const unsigned char charOzoneDensityUUID[16];
extern const unsigned char charNitrogenDioxideDensityUUID[16];
extern const unsigned char charSulphurDioxideDensityUUID[16];
extern const unsigned char charPM2_5DensityUUID[16];
extern const unsigned char charPM10DensityUUID[16];
extern const unsigned char charVOCDensityUUID[16];
extern const unsigned char charRelativeHumidityDehumidifierThresholdUUID[16];
extern const unsigned char charRelativeHumidityHumidifierThresholdUUID[16];
extern const unsigned char charServiceLabelIndexUUID[16];
extern const unsigned char charServiceLabelNamespaceUUID[16];
extern const unsigned char charColorTemperatureUUID[16];
extern const unsigned char charProgramModeUUID[16];
extern const unsigned char charInUseUUID[16];
extern const unsigned char charSetDurationUUID[16];
extern const unsigned char charRemainingDurationUUID[16];
extern const unsigned char charValveTypeUUID[16];
extern const unsigned char charIsConfiguredUUID[16];
extern const unsigned char charSupportedVideoStreamConfigurationUUID[16];
extern const unsigned char charSupportedAudioStreamConfigurationUUID[16];
extern const unsigned char charSupportedRTPConfigurationUUID[16];
extern const unsigned char charSelectedRTPStreamConfigurationUUID[16];
extern const unsigned char charSetupEndpointsUUID[16];
extern const unsigned char charVolumeUUID[16];
extern const unsigned char charMuteUUID[16];
extern const unsigned char charStreamingStatusUUID[16];


extern const hap_characteristic_desc_t hap_desc_char_ADMIN_ONLY_ACCESS;
extern const hap_characteristic_desc_t hap_desc_char_AUDIO_FEEDBACK;
extern const hap_characteristic_desc_t hap_desc_char_BRIGHTNESS;
extern const hap_characteristic_desc_t hap_desc_char_COOLING_TH_TEMPERATURE;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_DOOR_STATE;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_HEATING_COOLING_STATE;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_RELATIVE_HUMIDITY;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_TEMPERATURE;
extern const hap_characteristic_desc_t hap_desc_char_FIRMWARE_REVISION;
extern const hap_characteristic_desc_t hap_desc_char_HARDWARE_REVISION;
extern const hap_characteristic_desc_t hap_desc_char_HEATING_TH_TEMPERATURE;
extern const hap_characteristic_desc_t hap_desc_char_HUE;
extern const hap_characteristic_desc_t hap_desc_char_IDENTIFY;
extern const hap_characteristic_desc_t hap_desc_char_LOCK_CONTROL_POINT;
extern const hap_characteristic_desc_t hap_desc_char_LOCK_CURRENT_STATE;
extern const hap_characteristic_desc_t hap_desc_char_LOCK_LAST_KNOWN_ACTION;
extern const hap_characteristic_desc_t hap_desc_char_LOCK_MANAGE_AUTO_SECUR_TIMEOUT;
extern const hap_characteristic_desc_t hap_desc_char_LOCK_TARGET_STATE;
extern const hap_characteristic_desc_t hap_desc_char_LOGS;
extern const hap_characteristic_desc_t hap_desc_char_MANUFACTURER;
extern const hap_characteristic_desc_t hap_desc_char_MODEL;
extern const hap_characteristic_desc_t hap_desc_char_MOTION_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_NAME;
extern const hap_characteristic_desc_t hap_desc_char_OBSTRUCTION_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_ON;
extern const hap_characteristic_desc_t hap_desc_char_OUTLET_IN_USE;
extern const hap_characteristic_desc_t hap_desc_char_ROTATION_DIRECTION;
extern const hap_characteristic_desc_t hap_desc_char_ROTATION_SPEED;
extern const hap_characteristic_desc_t hap_desc_char_SATURATION;
extern const hap_characteristic_desc_t hap_desc_char_SERIAL_NUMBER;
extern const hap_characteristic_desc_t hap_desc_char_SOFTWARE_REVISION;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_DOOR_STATE;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_HEATING_COOLING_STATE;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_RELATIVE_HUMIDITY;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_TEMPERATURE;
extern const hap_characteristic_desc_t hap_desc_char_TEMPERATURE_DISPLAY_UNITS;
extern const hap_characteristic_desc_t hap_desc_char_VERSION;
extern const hap_characteristic_desc_t hap_desc_char_AIR_PARTICULATE_DENSITY;
extern const hap_characteristic_desc_t hap_desc_char_AIR_PARTICULATE_SIZE;
extern const hap_characteristic_desc_t hap_desc_char_SECUR_SYSTEM_CURRENT_STATE;
extern const hap_characteristic_desc_t hap_desc_char_SECUR_SYSTEM_TARGET_STATE;
extern const hap_characteristic_desc_t hap_desc_char_BATTERY_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_MONOXIDE_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_CONTACT_SENSOR_STATE;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_AMBIENT_LIGHT_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_HORIZONTAL_TILT_ANGLE;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_POSITION;
extern const hap_characteristic_desc_t hap_desc_char_CURRENT_VERTICAL_TILT_ANGLE;
extern const hap_characteristic_desc_t hap_desc_char_HOLD_POSITION;
extern const hap_characteristic_desc_t hap_desc_char_LEAK_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_OCCUPANCY_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_POSITION_STATE;
extern const hap_characteristic_desc_t hap_desc_char_PROG_SW_EVENT;
extern const hap_characteristic_desc_t hap_desc_char_PROG_SW_OUTPUT_STATE;
extern const hap_characteristic_desc_t hap_desc_char_STATUS_ACTIVE;
extern const hap_characteristic_desc_t hap_desc_char_SMOKE_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_STATUS_FAULT;
extern const hap_characteristic_desc_t hap_desc_char_STATUS_JAMMED;
extern const hap_characteristic_desc_t hap_desc_char_STATUS_LOW_BATTERY;
extern const hap_characteristic_desc_t hap_desc_char_STATUS_TAMPERED;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_H_TILT_ANGLE;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_POSITION;
extern const hap_characteristic_desc_t hap_desc_char_TARGET_V_TILT_ANGLE;
extern const hap_characteristic_desc_t hap_desc_char_SECUR_SYSTEM_ALARM_TYPE;
extern const hap_characteristic_desc_t hap_desc_char_CHARGING_STATE;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_MONOXIDE_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_MONOXIDE_PEAK_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_DIOXIDE_DETECTED;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_DIOXIDE_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_CARBON_DIOXIDE_PEAK_LEVEL;
extern const hap_characteristic_desc_t hap_desc_char_AIR_QUALITY;


extern const hap_characteristic_desc_t hap_desc_char_bool;
//--extern const hap_characteristic_desc_t hap_desc_char_utf8;
extern const hap_characteristic_desc_t hap_desc_char_string;


extern const unsigned char CFG_BOOL_UNITLESS[];
extern const unsigned char CFG_UINT8[];
extern const unsigned char CFG_UINT8_PERCENTAGE[];
extern const unsigned char CFG_UINT16[];
extern const unsigned char CFG_UINT32[];
extern const unsigned char CFG_UINT32_SECONDS[];
extern const unsigned char CFG_INT32[];
extern const unsigned char CFG_INT32_PERCENTAGE[];
extern const unsigned char CFG_INT32_CELSIUS[];
extern const unsigned char CFG_INT32_ARCDEGREE[];
//--extern const unsigned char CFG_UTF8S[];
extern const unsigned char CFG_STRING[];
//--extern const unsigned char CFG_OPAQUE[];
extern const unsigned char CFG_TLV8[];
extern const unsigned char CFG_DATA[];
extern const unsigned char CFG_FLOAT32[];
extern const unsigned char CFG_FLOAT32_ARCDEGREE[];
extern const unsigned char CFG_FLOAT32_PERCENTAGE[];
extern const unsigned char CFG_FLOAT32_CELSIUS[];
extern const unsigned char CFG_FLOAT32_LUX[];


/* ------------------------------------
    Function Prototypes
   ------------------------------------ */
int pairSetupWriteCallback(void *pp);
int pairSetupReadCallback(void *pp);
int pairVerifyReadCallback(void *pp);
int pairVerifyWriteCallback(void *pp);
int pairPairReadCallback(void *pp);
int pairPairWriteCallback(void *pp);

int HAPProtocolInfoWriteCallback(void *pp);
int HAPProtocolInfoReadCallback(void *pp);

int HAPCommonSetSessionTimeoutAndGSNWriteCB(void *pp);
int HAPCommonSetSessionTimeoutWriteCB(void *pp);
int HAPCommonSetSessionTimeoutReadCB(void *pp);
int ccc_GapAttServiceChangedWriteCB(void *pp);

unsigned char *searchTLVdeep(unsigned char *p, int len, unsigned char t, int *n);
unsigned char *searchTLV(unsigned char *p, int len, unsigned char t, int *n);
unsigned char *addTLV(unsigned char *pd, unsigned char *p, int len, unsigned char t);


int pairPaired ();
int pairInitID (void);

void srpInitKey();
void srpInitKey_2nd();
unsigned char hap_char_boundary_check (void);

int pairProcCommand (unsigned char * ps);
int test_srp();

void dev_device_id_set(void);
void dev_hash_deviceid_set(void);

//--#endif /* _HOMEKIT_INC_H_ */

/* %% End Of File %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
