/********************************************************************************************************
 * @file     ble.h 
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

#ifndef BLE_H_
#define BLE_H_


#include "proj_lib/ble/blt_config.h"
#include "ble_common.h"
#include "stack/ble_8278/ble_format.h"

#include "l2cap/l2cap.h"
//#include "phy/phy.h" 		// comment by weixiong in mesh
//#include "phy/phy_test.h" // comment by weixiong in mesh


#include "proj_lib/ble/att.h"
#include "attr/gatt.h"
#include "proj_lib/ble/uuid.h"



//#include "smp/smp.h" 				// comment by weixiong in mesh
//#include "smp/smp_const.h" 		// comment by weixiong in mesh
//#include "smp/smp_central.h" 		// comment by weixiong in mesh
//#include "smp/smp_peripheral.h" 	// comment by weixiong in mesh
//#include "smp/smp_storage.h" 		// comment by weixiong in mesh

#include "gap/gap.h"
#include "gap/gap_event.h"

//#include "crypt/aes_ccm.h" 		// comment by weixiong in mesh
//#include "crypt/le_crypto.h" 		// comment by weixiong in mesh
//#include "crypt/aes/aes_att.h" 	// comment by weixiong in mesh

//#include "hci/hci.h" 				// comment by weixiong in mesh
//#include "hci/hci_const.h" 		// comment by weixiong in mesh
//#include "hci/hci_event.h" 		// comment by weixiong in mesh
//#include "hci/usb_desc.h" 		// comment by weixiong in mesh

#include "proj_lib/ble/service/ble_ll_ota.h"
//#include "service/device_information.h" 	// comment by weixiong in mesh
//#include "service/hids.h" 				// comment by weixiong in mesh

#include "ll/ll.h"
#include "ll/ll_adv.h"
#include "ll/ll_encrypt.h"
#include "ll/ll_init.h"
#include "ll/ll_pm.h"
#include "ll/ll_scan.h"
#include "ll/ll_whitelist.h"
#include "ll/ll_conn/ll_conn.h"
#include "ll/ll_conn/ll_slave.h"
#include "ll/ll_conn/ll_master.h"
#include "ll/ll_conn/ll_conn_phy.h"
#include "ll/ll_conn/ll_conn_csa.h"

#include "ll/ll_ext.h"
//#include "ll/ll_ext_adv.h" 	// comment by weixiong in mesh
//#include "ll/ll_ext_scan.h" 	// comment by weixiong in mesh


//#include "bqb/bqb_ll.h" 	// comment by weixiong in mesh





volatile void  smemset(register char * dest,register int val,register unsigned int len);
volatile void * smemcpy(register char * out, register char * in, register unsigned int len);



#endif /* BLE_H_ */
