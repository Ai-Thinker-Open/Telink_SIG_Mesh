/********************************************************************************************************
 * @file     uart.h 
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
#include "proj/mcu/config.h"
#if(MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/uart.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "../../drivers/8278/uart.h"
#else
#ifndef 	_UART_H_
#define 	_UART_H_

#include "../mcu/register.h"
#include "../common/compatibility.h"
#include "../common/tutility.h"

enum UARTIRQSOURCE{
	UARTRXIRQ,
	UARTTXIRQ,
	UARTNOIRQ,
};

enum{
	UARTRXIRQ_MASK  = BIT(0),
	UARTTXIRQ_MASK  = BIT(1),
	UARTIRQ_MASK    = UARTRXIRQ_MASK | UARTTXIRQ_MASK,
};

/**
 *  @brief  Define UART RTS mode
 */
enum {
    UART_RTS_MODE_AUTO = 0,
    UART_RTS_MODE_MANUAL,
};

#define UART_115200RX_TIMEOUT_2MS_EN        1

#define CLK32M_UART9600			uart_Init(237,13,1,1)
#if (UART_115200RX_TIMEOUT_2MS_EN)
#define	CLK32M_UART115200		uart_Init(69,3,1,1)
#define	CLK16M_UART115200		uart_Init(34,3,1,1)
#else
#define	CLK32M_UART115200		uart_Init(19,13,1,1)
#define	CLK16M_UART115200		uart_Init(9,13,1,1)
#endif
#define	CLK16M_UART9600			uart_Init(103,15,1,1)


//UART_TX/UART_RX gpio pin config
#define	   UART_GPIO_CFG_PA6_PA7()  do{\
										*(volatile unsigned char  *)0x8005b0 |= 0x80;\
										*(volatile unsigned char  *)0x800586 &= 0x3f;\
										gpio_set_input_en(GPIO_PA7, 1);				 \
								    }while(0)
#define	   UART_GPIO_CFG_PB2_PB3()  do{\
										*(volatile unsigned char  *)0x8005b1 |= 0x0c;\
										*(volatile unsigned char  *)0x80058e &= 0xf3;\
										gpio_set_input_en(GPIO_PB3, 1);				 \
								    }while(0)
#define	   UART_GPIO_CFG_PC2_PC3()  do{\
										*(volatile unsigned char  *)0x8005b2 |= 0x0c;\
										*(volatile unsigned char  *)0x800596 &= 0xf3;\
										gpio_set_input_en(GPIO_PC3, 1);				 \
								    }while(0)


#define UART_GPIO_8266              1

#define UART_GPIO_8267_PA6_PA7      1
#define UART_GPIO_8267_PC2_PC3      2
#define UART_GPIO_8267_PB2_PB3      3


/**********************************************************
*
*	@brief	reset uart module
*
*	@param	none
*
*	@return	none
*/
extern void uart_Reset(void );




/*******************************************************
*
*	@brief	uart initiate, set uart clock divider, bitwidth and the uart work mode
*
*	@param	uartCLKdiv - uart clock divider
*			bwpc - bitwidth, should be set to larger than 2
*			en_rx_irq - '1' enable rx irq; '0' disable.
*			en_tx_irq - enable tx irq; '0' disable.
*
*	@return	'1' set success; '0' set error probably bwpc smaller than 3.
*
*		BaudRate = sclk/((uartCLKdiv+1)*(bwpc+1))
*		SYCLK = 16Mhz
		115200		9			13
		9600		103			15
*
*		SYCLK = 32Mhz
*		115200		19			13
		9600		237			13
*/
extern unsigned char uart_Init(unsigned short uartCLKdiv, unsigned char bwpc,unsigned char en_rx_irq,unsigned char en_tx_irq);




/********************************************************************
*
*	@brief	uart receive function, call this function to get the UART data
*
*	@param	userDataBuff - data buffer to store the uart data
*
*	@return	'0' rx error; 'rxLen' received data length
*/
//extern unsigned short uart_Rec(unsigned char* addr);

/******************************************************************************
*
*	@brief		get the uart IRQ source and clear the IRQ status, need to be called in the irq process function
*
*	@return		uart_irq_src- enum variable of uart IRQ source, 'UARTRXIRQ' or 'UARTTXIRQ'
*
*/
extern u8 uart_IRQSourceGet(void);

extern enum UARTIRQSOURCE uart_IRQSourceGet_kma(void);
/****************************************************************************************
*
*	@brief	data receive buffer initiate function. DMA would move received uart data to the address space, uart packet length
*			needs to be no larger than (recBuffLen - 4).
*
*	@param	*recAddr:	receive buffer's address info.
*			recBuffLen:	receive buffer's length, the maximum uart packet length should be smaller than (recBuffLen - 4)
*
*	@return	none
*/

extern void uart_RecBuffInit(unsigned char *recAddr, unsigned short recBuffLen);

extern void uart_BuffInit(unsigned char *recAddr, unsigned short recBuffLen, unsigned char *txAddr);

void uart_clr_tx_busy_flag();
unsigned char uart_tx_busy_check();
void uart_tx_busy_clear();

void uart_set_tx_done_delay (u32 t);		//for 8266 only

unsigned char uart_tx_is_busy(void);
extern void uart_gpio_set(u32 tx_pin,u32 rx_pin);
#endif
#endif

typedef struct{
	unsigned int len;        // data max 252
	unsigned char data[1];
}uart_data_t;

/**********************************************************
*
*	@brief	clear error state of uart rx, maybe used when application detected UART not work
*
*	@parm	none
*
*	@return	'1' RX error flag rised and cleard success; '0' RX error flag not rised
*
*/
unsigned char uart_ErrorCLR(void);

/********************************************************************************
*	@brief	uart send data function, this  function tell the DMA to get data from the RAM and start
*			the DMA send function
*
*	@param	sendBuff - send data buffer
*
*	@return	'1' send success; '0' DMA busy
*/
extern unsigned char uart_Send(unsigned char* data, unsigned int len);
extern unsigned char uart_Send_kma(unsigned char* addr);

void uart_io_init(unsigned char uart_io_sel);
