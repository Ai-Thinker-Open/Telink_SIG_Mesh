/********************************************************************************************************
 * @file     uart_simu.h 
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
#ifndef _DIGGER0_SERIAL_API_H
#define _DIGGER0_SERIAL_API_H

#include "../common/types.h"
//#include <windows.h>

#pragma pack(1)

/** @addtogroup  Digger0_Host_API
 *  @{
 */


/** @addtogroup  Digger0_Serial_API
 *  @{
 */


 /** @defgroup Digger0_Serial_Public_Constants
 *  @{
 */

#define ERR_NONE         0

#ifdef __CYGWIN
#define  ERR_SERIAL_ID   -1
#else
#define  ERR_SERIAL_ID   0
#endif

enum {
    ERR_SERIAL_START = 1,
	ERR_SERIAL_INVALID_PORT,
	ERR_SERIAL_INVALID_BAUD_RATE,
	ERR_SERIAL_CONFIG_FAIL,
	ERR_SERIAL_SEND_OVERFLOW,
	ERR_SERIAL_RECV_OVERFLOW,
	ERR_SERIAL_SEND_FRAME,
	ERR_SERIAL_RECV_FRAME,
	ERR_SERIAL_RECV_PARITY,
	ERR_SERIAL_RECV_EMPTY,
	ERR_SERIAL_RECV_OVERRUN,
	ERR_SERIAL_END,
};


/*@} end of group Digger0_Serial_Public_Constants */

#ifdef __CYGWIN
typedef int32_t SERIAL_ID;
#else
typedef void*  SERIAL_ID;
#endif

/** @defgroup Digger0_Serial_Public_Types Digger0_Serial_Public_Types
 *  @brief uart instance structure type definition
 *  @{
 */

/**
 *  @brief uart instance type definition
 */
enum Parity {
    PARITY_UART_NONE,
    PARITY_UART_ODD,
    PARITY_UART_EVEN,
};
typedef enum Parity parity_t;


/**
 *  @brief uart instance type definition
 */
enum SerialMode {
    UART_POLLING,
    UART_ASYN,
};


/**
 *  @brief uart instance type definition
 */
typedef struct {
    u8       port;          /*!< UART mode */
    u32      baudRate;      /*!< UART baud rate */
    u8       stopBits;      /*!< UART stop bits */
    parity_t parity;        /*!< UART parity mode */
    u8       mode;          /*!< UART Polling mode or not */
    SERIAL_ID fd;           /*!< UART file description */
} uart_inst_t;

/*@} end of group Digger0_Serial_Public_Types definitions */


/** @defgroup Digger0_Serial_API_Function_Declaration
 *  @brief Digger0_Serial_API functions statement
 *  @{
 */


/*API for Serial */
#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************//**
 * @fn SERIAL_ID halUartInit(uint8_t port, uint32_t baudRate, parity_t parity, uint8_t stopBits, uint8_t mode);
 *
 * @brief Initialize specified uart part with specified parameter.
 *
 * @param port specify the uart port to open
 *
 * @param baudRate specify the uart baud rate to transfer
 *
 * @param parity specifiy ODD/EVEN/NONE parity
 *
 * @param stopBits specifiy the stop bits
 *
 * @param mode specifiy the transfer mode
 *
 * @return length field and header only
 *
 *******************************************************************************/
SERIAL_ID  halUartInit(u8 port, u32 baudRate, parity_t parity, u8 stopBits, u8 mode);


/****************************************************************************//**
 * @fn uint32_t halUartWrite(SERIAL_ID fd, uint8_t* data, uint32_t length);
 *
 * @brief send data through specified uart.
 *
 * @param fd specify the uart id to transfer
 *
 * @param data the value(s) to send
 *
 * @param length the length of data
 *
 * @return status status of the Tx
 *
 *******************************************************************************/
u32 halUartWrite(SERIAL_ID fd, u8* data, u32 length);

/****************************************************************************//**
 * @fn uint32_t halUartRead(SERIAL_ID fd, uint8_t* data, uint32_t length);
 *
 * @brief receive data through specified uart.
 *
 * @param fd specify the uart id to receive
 *
 * @param data the value(s) received from uart
 *
 * @param length the length of data to receive
 *
 * @return status status of the Rx
 *
 *******************************************************************************/
u32 halUartRead(SERIAL_ID fd, u8* data, u32 length);
u32 halUartRead_nonBlock_byte(SERIAL_ID fd, u8* data);

/****************************************************************************//**
 * @fn void     halUartClose(SERIAL_ID fd);
 *
 * @brief close specified uart.
 *
 * @param fd specify the uart id to close
 *
 * @return None
 *
 *******************************************************************************/
void     halUartClose(SERIAL_ID fd);

#ifdef __cplusplus
}
#endif


/*@} end of group Digger0_Serial_API_Function_Declaration */

/*@} end of group Digger0_Serial_API  */

/*@} end of group Digger0_Host_API */


#endif /* _DIGGER0_SERIAL_API_H */
