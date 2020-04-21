/********************************************************************************************************
 * @file     myprintf.h 
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
#ifndef MYPRINTF_H
#define MYPRINTF_H

#define SIMU_BAUD_115200    115200
#define SIMU_BAUD_230400    230400
#define SIMU_BAUD_1M        1000000

#define BAUD_USE    SIMU_BAUD_115200
#define SIMU_UART_IRQ_EN    1


extern  void uart_simu_send_bytes(u8 *p,int len);
//#define debugBuffer (*(volatile unsigned char (*)[40])(0x8095d8))
#endif
