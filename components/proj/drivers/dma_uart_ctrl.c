/********************************************************************************************************
 * @file     dma_uart_ctrl.c 
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
#include "../tl_common.h"

#if 0

/**********************************************************************
 * INCLUDES
 */
#include "dma_uart_ctrl.h"
#include "dma_uart_hw.h"


/**********************************************************************
 * LOCAL CONSTANTS
 */

 
/**********************************************************************
 * LOCAL TYPES
 */

/**********************************************************************
 * LOCAL VARIABLES
 */
 
DmaUart_ctrl_t DmaUart_vs;

DmaUart_ctrl_t *DmaUart_v = &DmaUart_vs;

/**********************************************************************
 * GLOBAL VARIABLES
 */

extern u8* G_RxDestBuf;
extern u16 G_RxDestBufSize;
extern u8* G_TxSrcBuf;
extern u16 G_TxSrcBufSize;

/**********************************************************************
 * LOCAL FUNCTIONS
 */

uart_sts_t DmaUart_sendBulkData(void)
{
	
	u16 len;
	
	/* Get the length to send in this bulk transaction */
	len = (DmaUart_v->lenToSend > G_TxSrcBufSize) ? G_TxSrcBufSize : DmaUart_v->lenToSend;
	if (len == 0) 
	{
		return UART_SUCC;
	}

	//Every time before we send dma data, we need to check if the hardware is idle
	while(DmaUartTxHw_is_busy())
	{
		WaitMs(1);
	}
	
	DmaUart_v->lenToSend -= len;
	
	// Copy to DMA and start dma transmission
	G_TxSrcBuf[0] = len&(0xff);
	G_TxSrcBuf[1] = (len>>8)&(0xff);
	G_TxSrcBuf[2] = (len>>16)&(0xff);
	G_TxSrcBuf[3] = (len>>24)&(0xff);
	memcpy(&G_TxSrcBuf[4], &(DmaUart_v->txBuf[DmaUart_v->lastSendIndex]),len);
	DmaUart_v->lastSendIndex += len;
	StartDmaUartTx();

	return UART_SUCC;
}



uart_sts_t DmaUart_sendData(u8 *buf, u32 len)
{
	uart_sts_t SendBulkDataResult;

	/*If it is busy, return to add it to the queue*/
	if ((DmaUart_v->txBuf) || (DmaUartTxHw_is_busy())) 
	{
		return UART_BUSY;
	}


	/* Init the bulk transfer */
	DmaUart_v->lenToSend = len;
	DmaUart_v->txBuf = buf;
	DmaUart_v->lastSendIndex = 0;

	/* Send first bulk */
	SendBulkDataResult = DmaUart_sendBulkData();

	return SendBulkDataResult;
	
   
}

/*To check if DmaUart buffer (software) is available */
u8 DmaUart_isAvailable(void)
{
	return (DmaUart_v->txBuf == NULL);
}






#endif //END OF DMA_UART_ENABLE


