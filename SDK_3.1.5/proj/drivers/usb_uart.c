/********************************************************************************************************
 * @file     usb_uart.c 
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

#if USB_CDC_ENABLE

/**********************************************************************
 * INCLUDES
 */
#include "usb_uart.h"
#include "../app/usbcdc.h"



/**********************************************************************
 * LOCAL CONSTANTS
 */



/**********************************************************************
 * LOCAL TYPES
 */
typedef struct {
    u8  len;
    u8* txBuf;
} usb_uart_txPendingEvt_t;



/**********************************************************************
 * LOCAL VARIABLES
 */
uart_recvCb_t usb_uart_cbFn = NULL;
ev_queue_t    usb_uart_txPendingQ;


/**********************************************************************
 * LOCAL FUNCTIONS
 */



/*********************************************************************
  * @fn      usb_uart_rxHandler
  *
  * @brief   Internal rx handler from usb_cdc module. This function is 
  *          called from USB interrupt.
  *          In this function, the user reigsted handler will be called 
  *          through task mechnism, and then allocate a new buffer for 
  *          usb driver to continue receive.
  *
  * @param   pData - The received usb-uart packets
  *
  * @return  None
  */
static void usb_uart_rxHandler(u8* pData)
{
	u8* pBuf;

	/* Post task to handle the received command */
	EV_SCHEDULE_TASK(usb_uart_cbFn, pData);

	/* Set the USB RX buffer again */
    pBuf = ev_buf_allocate(LARGE_BUFFER);
	if (!pBuf) {
		while(1);
	}

    memset(pBuf, 0, 150);
	usbcdc_setRxBuf(pBuf);
}

/*********************************************************************
  * @fn      usb_uart_tXFinishCb
  *
  * @brief   This function is called by usb interrupt.
  *          In this function, we free the tx buffer first and then check
  *          is there any packets in the tx pending queue. If any, send it.
  *
  * @param   pData - Pointer to the already sent tx buffer.
  *
  * @return  None
  */
static void usb_uart_tXFinishCb(u8* pData)
{
	usb_uart_txPendingEvt_t* pEvt;
    u8 len;
    u8 *p;

    /* Free the TX buffer at first */
    ev_buf_free(pData);

    /* If there is pending data, send it again */
    if ( usb_uart_txPendingQ.curNum ) {
        pEvt = (usb_uart_txPendingEvt_t*)ev_queue_pop(&usb_uart_txPendingQ);
        p = pEvt->txBuf;
        len = pEvt->len;
        ev_buf_free((u8*)pEvt);
        
        usb_uart_write(p, len);
    }
}


/*********************************************************************
  * @fn      usb_uart_open
  *
  * @brief   API to open usb-uart.
  *
  * @param   baudrate - The baudrate for the uart connection
  *          cb       - The uart receive callback function
  *
  * @return  None
  */
void usb_uart_open(u32 baudrate, uart_recvCb_t cb)
{
	u8* pBuf;

	/* Initialize USB-UART */
    usbcdc_setCB(usb_uart_rxHandler, usb_uart_tXFinishCb);
	pBuf = ev_buf_allocate(LARGE_BUFFER);
	if (!pBuf) {
		while(1);
	}

	/* Set RX buffer to USB-CDC */
	usbcdc_setRxBuf(pBuf);

	/* Register callback to handle received command */
	usb_uart_cbFn = cb;

    /* Initialize USB tx pending Queue */
    ev_queue_init(&usb_uart_txPendingQ, NULL);
}


/*********************************************************************
  * @fn      usb_uart_write
  *
  * @brief   API to send usb-uart data.
  *
  * @param   buf - The buffer to be sent
  *          len - The length to be sent
  *
  * @return  status
  */
usbcdc_sts_t usb_uart_write(u8* buf, u8 len)
{
	usb_uart_txPendingEvt_t* pEvt;

    /* Check the usb status first. If it is idle, send data dirctly.
    If it is busy, put data to the pending queue and send it after
    current operation done. */
    if (usbcdc_isAvailable()) {
        return usbcdc_sendData(buf, len);
    } else {
        pEvt = (usb_uart_txPendingEvt_t*)ev_buf_allocate(sizeof(usb_uart_txPendingEvt_t));
        if (!pEvt) {
            while(1);
        }
        pEvt->len = len;
        pEvt->txBuf = buf;
        ev_queue_push(&usb_uart_txPendingQ, (u8*)pEvt);
        return USB_MULTIBLOCK;
    }
}


#endif  /* USB_CDC_ENABLE */
