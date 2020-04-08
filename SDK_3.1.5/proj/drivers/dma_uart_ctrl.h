/********************************************************************************************************
 * @file     dma_uart_ctrl.h 
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

#include "../common/types.h"
#include "../common/static_assert.h"
#include "../common/bit.h"
#include "uart.h"
//#include "../usbstd/CDCClassCommon.h"
//#include "../usbstd/CDCClassDevice.h"

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
    extern "C" {
#endif

typedef struct {
	u8 *rxBuf;
	u8 *txBuf;

	/* Following variables are used in the RX more than DMA_MAX_SIZE */
	u16 lenToSend;
	u16 lastSendIndex;
	
} DmaUart_ctrl_t;

uart_sts_t DmaUart_sendData(u8* buf, u32 len);
uart_sts_t DmaUart_sendBulkData(void);
u8   DmaUart_isAvailable(void);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
    }
#endif

