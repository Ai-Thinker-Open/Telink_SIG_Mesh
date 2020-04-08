/********************************************************************************************************
 * @file     rf_pa.h 
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

#ifndef BLT_PA_H_
#define BLT_PA_H_

#include "../tl_common.h"


#ifndef PA_ENABLE
#define PA_ENABLE                           0
#endif


// for not 826x,PA pin use integrated RF synthesizer, refer to RF_LNARxPinDef and RF_PATxPinDef for pa pin!!!
#ifndef PA_TXEN_PIN
#define PA_TXEN_PIN                         GPIO_PB3
#endif

#ifndef PA_RXEN_PIN
#define PA_RXEN_PIN                         GPIO_PB2
#endif



#define PA_TYPE_OFF							0
#define PA_TYPE_TX_ON						1
#define PA_TYPE_RX_ON						2


typedef void (*rf_pa_callback_t)(int type);
extern rf_pa_callback_t  blc_rf_pa_cb;



void rf_pa_init(void);


#endif /* BLT_PA_H_ */
