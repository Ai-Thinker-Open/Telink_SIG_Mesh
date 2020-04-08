/********************************************************************************************************
 * @file     hk_hap2_opcode_wrapper.h
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

#ifndef HK_HAP2_OPCODE_WRAPPER_H_
#define HK_HAP2_OPCODE_WRAPPER_H_

/* ------------------------------------
    Macro Definitions
   ------------------------------------ */
// === HAP Status Code ===
enum {
    HAP_STCODE_SUCCESS          = 0,
    HAP_STCODE_UNSUPPORT_PDU    = 1,
    HAP_STCODE_MAX_PROC         = 2,
    HAP_STCODE_INSF_AUTHO       = 3,
    HAP_STCODE_INVALID_INST     = 4,
    HAP_STCODE_INSF_AUTHE       = 5,
    HAP_STCODE_INVALID_REQ      = 6,
//--    HAP_STCODE_SSR_INSTANCE0    = 7,
    HAP_STCODE_SERVICE_SIGN_READ_GOT_IID0    = 8,       // workaround: if got a iid=0, the accessory respone success with 00 00
    HAP_CHAR_ERR                = BIT(7),
};

// ===== parameter type ===
enum {
    HAPBLE2_PARAM_VALUE                     = 0x01,
    HAPBLE2_PARAM_ADDTIONAL_AUTHDATA        = 0x02,
    HAPBLE2_PARAM_ORIGIN                    = 0x03,
    HAPBLE2_PARAM_CHAR_TYPE                 = 0x04,
    HAPBLE2_PARAM_CHAR_INSTANCE_ID          = 0x05,
    HAPBLE2_PARAM_SERVICE_TYPE              = 0x06,
    HAPBLE2_PARAM_SERVICE_INSTANCE_ID       = 0x07,
    HAPBLE2_PARAM_TTL                       = 0x08,
    HAPBLE2_PARAM_RETURN_RESPONSE           = 0x09,
    HAPBLE2_PARAM_HAP_CHAR_PROP_DESCR       = 0x0A,
    HAPBLE2_PARAM_GATT_USER_DESCR_DESCR     = 0x0B,
    HAPBLE2_PARAM_GATT_PRESENT_FORMAT_DESCR = 0x0C,
    HAPBLE2_PARAM_GATT_VALID_RANGE          = 0x0D,
    HAPBLE2_PARAM_HAP_STEP_VALUE_DESCR      = 0x0E,
    HAPBLE2_PARAM_HAP_SERVICE_PROP          = 0x0F,
    HAPBLE2_PARAM_HAP_LINKED_SERVICES       = 0x10,
    HAPBLE2_PARAM_HAP_VALID_VALUE_DESCR     = 0x11,
    HAPBLE2_PARAM_HAP_VALID_VALUE_RANGE_DESCR   = 0x12,
};

// ===== characteristic configuration parameter types ===
enum {
    HAP_CHAR_CONFIG_PARAM_PROPERIES         = 0x01,
    HAP_CHAR_CONFIG_PARAM_BROADCAST_INTERVAL= 0x02,
};

// ===== Protocol configuration parameter types ===
enum {
    HAP_PROTOCOL_CONFIG_GEN_BRDCAST_ENCRYP_KEY  = 0x01,
    HAP_PROTOCOL_CONFIG_GEN_ALL_PARAMS          = 0x02,
    HAP_PROTOCOL_CONFIG_SET_ADV_IDENTIFIER      = 0x03,
};
#endif /* HK_HAP2_OPCODE_WRAPPER_H_ */
