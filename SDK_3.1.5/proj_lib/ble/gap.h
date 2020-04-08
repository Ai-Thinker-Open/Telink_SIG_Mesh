/********************************************************************************************************
 * @file     gap.h 
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

#include "proj/tl_common.h"


/** @addtogroup  TELINK_BLE_STACK TELINK BLE Stack
 *  @{
 */

/** @addtogroup  GAP_Constants_Module GAP Layer Constant
 *  @{
 */
 
/** @addtogroup  GAP_Constants GAP Constants
 *  @{
 */

/** @addtogroup gap_adtype_t GAP ADTYPE
 * https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile
 * @{
 */
#define GAP_ADTYPE_FLAGS                        0x01 //!< Discovery Mode: @ref GAP_ADTYPE_FLAGS_MODES
#define GAP_ADTYPE_16BIT_INCOMPLETE             0x02 //!< Incomplete List of 16-bit Service Class UUIDs
#define GAP_ADTYPE_16BIT_COMPLETE               0x03 //!< Complete List of 16-bit Service Class UUIDs
#define GAP_ADTYPE_32BIT_INCOMPLETE             0x04 //!< Service: More 32-bit UUIDs available
#define GAP_ADTYPE_32BIT_COMPLETE               0x05 //!< Service: Complete list of 32-bit UUIDs
#define GAP_ADTYPE_128BIT_INCOMPLETE            0x06 //!< Service: More 128-bit UUIDs available
#define GAP_ADTYPE_128BIT_COMPLETE              0x07 //!< Service: Complete list of 128-bit UUIDs
#define GAP_ADTYPE_LOCAL_NAME_SHORT             0x08 //!< Shortened local name
#define GAP_ADTYPE_LOCAL_NAME_COMPLETE          0x09 //!< Complete local name
#define GAP_ADTYPE_TX_POWER_LEVEL               0x0A //!< TX Power Level: 0xXX: -127 to +127 dBm
#define GAP_ADTYPE_OOB_CLASS_OF_DEVICE          0x0D //!< Simple Pairing OOB Tag: Class of device (3 octets)
#define GAP_ADTYPE_OOB_SIMPLE_PAIRING_HASHC     0x0E //!< Simple Pairing OOB Tag: Simple Pairing Hash C (16 octets)
#define GAP_ADTYPE_OOB_SIMPLE_PAIRING_RANDR     0x0F //!< Simple Pairing OOB Tag: Simple Pairing Randomizer R (16 octets)
#define GAP_ADTYPE_DEVICE_ID                    0x10 //!< Device ID Profile v1.3 or later
#define GAP_ADTYPE_SM_TK                        0x10 //!< Security Manager TK Value
#define GAP_ADTYPE_SM_OOB_FLAG                  0x11 //!< Secutiry Manager OOB Flags
#define GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE    0x12 //!< Min and Max values of the connection interval (2 octets Min, 2 octets Max) (0xFFFF indicates no conn interval min or max)
#define GAP_ADTYPE_SERVICES_LIST_16BIT          0x14 //!< Service Solicitation: list of 16-bit Service UUIDs
#define GAP_ADTYPE_SERVICES_LIST_32BIT          0x1F //!< Service Solicitation: list of 32-bit Service UUIDs
#define GAP_ADTYPE_SERVICES_LIST_128BIT         0x15 //!< Service Solicitation: list of 128-bit Service UUIDs
#define GAP_ADTYPE_SERVICE_DATA                 0x16 //!< Service Data
#define GAP_ADTYPE_SERVICE_DATA_UUID_16BIT      0x16 //!< Service Data - 16-bit UUID
#define GAP_ADTYPE_SERVICE_DATA_UUID_32BIT      0x20 //!< Service Data - 32-bit UUID
#define GAP_ADTYPE_SERVICE_DATA_UUID_128BIT     0x21 //!< Service Data - 128-bit UUID
#define GAP_ADTYPE_TARGET_ADDR_PUBLIC           0x17 //!< Public Target Address
#define GAP_ADTYPE_TARGET_ADDR_RANDOM           0x18 //!< Random Target Address
#define GAP_ADTYPE_APPEARANCE                   0x19 //!< Appearance
#define GAP_ADTYPE_ADVERTISING_INTERVAL         0x1A //!< Advertising Interval
#define GAP_ADTYPE_LE_BLUETOOTH_DEVICE_ADDR     0x1B //!< ​LE Bluetooth Device Address
#define GAP_ADTYPE_LE_ROLE                      0x1C //!< LE Role
#define GAP_ADTYPE_SIMPLE_PAIRING_HASHC_256     0x1D //!< Simple Pairing Hash C-256
#define GAP_ADTYPE_SIMPLE_PAIRING_RAND_R256     0x1E //!< Simple Pairing Randomizer R-256
#define GAP_ADTYPE_3D_INFORMATION_DATA          0x3D //!< 3D Synchronization Profile, v1.0 or later
#define GAP_ADTYPE_MANUFACTURER_SPECIFIC        0xFF //!< Manufacturer Specific Data: first 2 octets contain the Company Identifier Code followed by the additional manufacturer specific data
/** @} end of group gap_adtype_t */


/** @addtogroup gap_adtype_bitmask Bitmask of ADTYPE
 * @{
 */
#define GAP_ADTYPE_LE_LIMITED_DISCOVERABLE_MODE_BIT       0x01
#define GAP_ADTYPE_LE_GENERAL_DISCOVERABLE_MODE_BIT       0x02
#define GAP_ADTYPE_LMP_BIT37_BIT                          0x04
/** @} end of group gap_adtype_bitmask */


/**
 *  @brief  Definition for default timer values
 */
#define T_GAP_GEN_DISC_ADV_MIN_DEFAULT         0       // mSec (0 = no timeout)
#define T_GAP_LIM_ADV_TIMEOUT_DEFAULT          180     // 180 seconds
#define T_GAP_GEN_DISC_SCAN_DEFAULT            10240   // mSec
#define T_GAP_LIM_DISC_SCAN_DEFAULT            10240   // mSec
#define T_GAP_CONN_EST_ADV_TIMEOUT_DEFAULT     10240   // mSec
#define T_GAP_CONN_PARAM_TIMEOUT_DEFAULT       30000   // mSec

// GAP Constants defaults
#if defined ( GAP_STANDARDS )
  // Defined as defaults in spec
  #define T_GAP_LIM_DISC_ADV_INT_MIN_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_ADV_INT_MAX_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_ADV_INT_MIN_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_ADV_INT_MAX_DEFAULT     2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_ADV_INT_MIN_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_ADV_INT_MAX_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_SCAN_INT_DEFAULT            2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_SCAN_WIND_DEFAULT           18      // 11.25 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_HIGH_SCAN_INT_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_HIGH_SCAN_WIND_DEFAULT      16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_ADV_DEFAULT             80      // 50 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_INT_MIN_DEFAULT         400     // 500 mSec (n * 1.25 mSec)
  #define T_GAP_CONN_EST_INT_MAX_DEFAULT         400     // 500 mSec (n * 1.25 mSec)
  #define T_GAP_CONN_EST_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_SUPERV_TIMEOUT_DEFAULT  2000    // 20 sec (n * 10 mSec)
  #define T_GAP_CONN_EST_LATENCY_DEFAULT         0       // (in number of connection events)
  #define T_GAP_CONN_EST_MIN_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
  #define T_GAP_CONN_EST_MAX_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
  #define T_GAP_PRIVATE_ADDR_INT_DEFAULT         15      // 15 minutes
#else
  // Actually works
  #define T_GAP_LIM_DISC_ADV_INT_MIN_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_ADV_INT_MAX_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_ADV_INT_MIN_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_ADV_INT_MAX_DEFAULT     160     // 100 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_ADV_INT_MIN_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_ADV_INT_MAX_DEFAULT         2048    // 1280 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_SCAN_INT_DEFAULT            480     // 300 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_SCAN_WIND_DEFAULT           240     // 150 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_HIGH_SCAN_INT_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_HIGH_SCAN_WIND_DEFAULT      16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_GEN_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_LIM_DISC_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_ADV_DEFAULT             80      // 50 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_INT_MIN_DEFAULT         80      // 100 mSec (n * 1.25 mSec)
  #define T_GAP_CONN_EST_INT_MAX_DEFAULT         80      // 100 mSec (n * 1.25 mSec)
  #define T_GAP_CONN_EST_SCAN_INT_DEFAULT        16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_SCAN_WIND_DEFAULT       16      // 10 mSec (n * 0.625 mSec)
  #define T_GAP_CONN_EST_SUPERV_TIMEOUT_DEFAULT  2000    // 20 sec (n * 10 mSec)
  #define T_GAP_CONN_EST_LATENCY_DEFAULT         0       // (in number of connection events)
  #define T_GAP_CONN_EST_MIN_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
  #define T_GAP_CONN_EST_MAX_CE_LEN_DEFAULT      0       // (n * 0.625 mSec)
  #define T_GAP_PRIVATE_ADDR_INT_DEFAULT         15      // 15 minutes
#endif


/**
 *  @brief  Definition for default maximum advertising data length
 */
#define MAX_GAP_ADVERTISING_DATA_LEN             30


#define GAP_APPEARE_UNKNOWN                     0x0000 //!< Unknown

#if(MCU_CORE_TYPE == MCU_CORE_8258)
#include "stack/ble/gap/gap.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "stack/ble_8278/gap/gap.h"
#endif
/** @} end of group GAP_Constants */

/** @} end of group GAP_Constants_Module */

/** @} end of group TELINK_BLE_STACK */