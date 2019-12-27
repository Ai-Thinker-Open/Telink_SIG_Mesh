/********************************************************************************************************
 * @file     attributes.h
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

#pragma once

#include "../../ble/attr/gatt_server.h"

extern attribute_t gAttributes[];

#if(HOMEKIT_ENABLE)

typedef enum{
    kTLVType_Method     = 0,
	kTLVType_Identifier = 1,
	kTLVType_Salt 		= 2,
	kTLVType_PublicKey	= 3,
	kTLVType_Proof		= 4,
	kTLVType_EncryptedData	= 5,
	kTLVType_State		= 6,
	kTLVType_Error		= 7,
	kTLVType_RetryDelay	= 8,
	kTLVType_Certificate = 9,
	kTLVType_Signature	= 0xa,
	kTLVType_Permissions = 0xb,
	kTLVType_FragmentData = 0xc,
	kTLVType_FragmentLast = 0xd,
	kTLVType_Flags		  = 0x13,
	kTLVType_Separator	= 0xff,
}kTLVType_t;


typedef enum{
    State_M1_SRPStartRequest      = 1,
    State_M2_SRPStartRespond      = 2,
    State_M3_SRPVerifyRequest     = 3,
    State_M4_SRPVerifyRespond     = 4,
    State_M5_ExchangeRequest      = 5,
    State_M6_ExchangeRespond      = 6,
} PairSetupState_t;

typedef enum{
    State_Pair_Verify_M1          = 1,
    State_Pair_Verify_M2          = 2,
    State_Pair_Verify_M3          = 3,
    State_Pair_Verify_M4          = 4,
} PairVerifyState_t;

typedef enum{
    Type_Data_Without_Length      = 1,
    Type_Data_With_Length         = 2,
} Poly1305Type_t;

#define HomeKitLog 						0
#define HomeKitReplyHeaderLog 			0

//Number of client
/*
 * BEWARE: Never set the number of client to 1
 * iOS HomeKit pair setup socket will not release until the pair verify stage start
 * So you will never got the pair corrected, as it is incomplete (The error require manually reset HomeKit setting
 */
#define numberOfClient 20
//Number of notifiable value
/*
 * Count how many notifiable value exist in your set
 * For dynamic add/drop model, please estimate the maximum number (Too few->Buffer overflow)
 */
#define numberOfNotifiableValue 1

//If you compiling this to microcontroller, set it to 1
#define MCU 					1


#endif


