/********************************************************************************************************
 * @file     printf.h 
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
#if(!__PROJECT_8263_BLE_MODULE__)
#ifdef WIN32
#include <stdio.h>
#else
#include <stdarg.h>

int my_printf(const char *fmt, ...);
int my_sprintf(char* s, const char *fmt, ...);

int my_printf_uart(const char *format,...);
int my_printf_uart_hexdump(unsigned char *p_buf,int len );
int print(char **out, const char *format, va_list args) ;
int printf_Bin2Text (char *lpD, int lpD_len_max, char *lpS, int n);
u32 get_len_Bin2Text(u32 buf_len);


#define printf	my_printf
#define sprintf	my_sprintf

#endif
#endif

