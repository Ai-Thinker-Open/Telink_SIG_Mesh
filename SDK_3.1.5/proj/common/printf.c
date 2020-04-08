/********************************************************************************************************
 * @file     printf.c 
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

/*
 putchar is the only external dependency for this file,
 if you have a working putchar, leave it commented out.
 If not, uncomment the define below and
 replace outbyte(c) by your own function call.

 #define putchar(c) outbyte(c)
 */
#if(!__PROJECT_8263_BLE_MODULE__)
#ifndef WIN32

#include <stdarg.h>

#include "types.h"
#include "proj_lib/ble/hci/hci.h"
#include "../mcu/putchar.h"
#include "mesh/myprintf.h"
#include "mesh/user_config.h"


#define MAX_PRINT_STRING_CNT 60

static void printchar(char **str, int c) {
	if (str) {
		**str = c;
		++(*str);
	} else
		(void) putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad) {
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad,
		int letbase) {
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u) {
		t = u % b;
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar(out, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

int print(char **out, const char *format, va_list args) {
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if(pc> 512-2){
			return pc;
		}
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				register char *s = (char *) va_arg( args, int );
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd') {
				pc += printi(out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char) va_arg( args, int );
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
			out: printchar(out, *format);
			++pc;
		}
	}
	if (out)
		**out = '\0';
	va_end( args );
	return pc;
	
}
const char printf_arrb2t[] = "0123456789abcdef";

u32 get_len_Bin2Text(u32 buf_len)
{
    return (buf_len*3 + (((buf_len+15)/16)*(1+2))); // 1: space, 2: /r/n
}

int printf_Bin2Text (char *lpD, int lpD_len_max, char *lpS, int n)
{
    int i = 0;
	int m = n;
	int d = 0;

    #define LINE_MAX_LOG        (5)
	if(m > BIT(LINE_MAX_LOG)){
	    if(lpD_len_max > 2){
    		lpD[d++] = '\r';
    		lpD[d++] = '\n';
    		lpD_len_max -= 2;
		}
	}
	
	for (i=0; i<m; i++) {
	    if((d + 6 + 2) > lpD_len_max){
	        break;
	    }
	    
        if((0 == (i & BIT_MASK_LEN(LINE_MAX_LOG))) && i){
            lpD[d++] = '\r';
            lpD[d++] = '\n';
        }

		lpD[d++] = printf_arrb2t [(lpS[i]>>4) & 15];
		lpD[d++] = printf_arrb2t [lpS[i] & 15];

		if((i & BIT_MASK_LEN(LINE_MAX_LOG)) != BIT_MASK_LEN(LINE_MAX_LOG)){	// end of line
    		lpD[d++] = ' ';

            if(m > BIT(LINE_MAX_LOG)){
    		    if ((i&7)==7){
    		        lpD[d++] = ' ';
    		    }
    		}
		}
	}

	if(lpD_len_max >= d+2){
    	lpD[d++] = '\r';    // lpS is always ture here. so can't distinguish whether there is buffer or not.
    	lpD[d++] = '\n';
        // lpD[d++] = '\0';        // can't add 0, because some UART Tool will show error.
    }
    
	return d;
}

extern int my_fifo_push_hci_tx_fifo (unsigned char *p, unsigned short n, unsigned char *head, unsigned char head_len);
#if HCI_LOG_FW_EN
char hex_dump_buf[MAX_PRINT_STRING_CNT];
unsigned char printf_uart[512];
#endif
int my_printf_uart_hexdump(unsigned char *p_buf,int len )
{
    #if HCI_LOG_FW_EN
	
	int dump_len ;
	dump_len = printf_Bin2Text(hex_dump_buf, sizeof(hex_dump_buf), (char*)p_buf,len);
    uart_simu_send_bytes((unsigned char *)hex_dump_buf,dump_len);
    #endif
	return 1;
	#if 0
	u8 head[2] = {HCI_LOG};
	head[1]= dump_len+2;
	return	my_fifo_push_hci_tx_fifo((unsigned char *)hex_dump_buf,dump_len,head,2);
	#endif
}

int my_printf_uart(const char *format,...){
    #if HCI_LOG_FW_EN
	va_list args;
	unsigned char **pp_buf;
	unsigned char *p_buf;
	unsigned int len =0;
	p_buf = printf_uart;
	pp_buf = &(p_buf);
	va_start( args, format );
	len = print((char **)pp_buf, format, args);
	uart_simu_send_bytes(printf_uart,len);
	#endif
	return 1;
	
	#if 0
	unsigned int idx =0;
	u8 head[2] = {HCI_LOG};
	while(len){
		if(len>MAX_PRINT_STRING_CNT){
			head[1]= MAX_PRINT_STRING_CNT+2;
			my_fifo_push_hci_tx_fifo(buf+idx ,MAX_PRINT_STRING_CNT,head,2);
			idx+=MAX_PRINT_STRING_CNT;
			len-=MAX_PRINT_STRING_CNT;
		}else{
			head[1]= len+2;
			my_fifo_push_hci_tx_fifo(buf+idx ,len,head,2);
			len =0;
			idx =0;
		}
	}
	return	0;
	#endif

}

int my_printf(const char *format, ...) {
	va_list args;
	va_start( args, format );
	return print(0, format, args);
}

int my_sprintf(char *out, const char *format, ...) {
	va_list args;
	va_start( args, format );
	return print(&out, format, args);
}

#endif
#endif
