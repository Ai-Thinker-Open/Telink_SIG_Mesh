/********************************************************************************************************
 * @file     myprintf.c 
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
#include "proj/tl_common.h"  
#include "proj_lib/ble/service/ble_ll_ota.h"

#if PRINT_DEBUG_INFO

#include "myprintf.h"

#define		BIT_INTERVAL_SYS_TICK	(CLOCK_SYS_CLOCK_1S/BAUD_USE)

_attribute_no_retention_bss_ static int tx_pin_initialed = 0;

/**
 * @brief  DEBUG_INFO_TX_PIN initialize. Enable 1M pull-up resistor,
 *   set pin as gpio, enable gpio output, disable gpio input.
 * @param  None
 * @retval None
 */
_attribute_no_inline_ void debug_info_tx_pin_init()
{
    gpio_setup_up_down_resistor(DEBUG_INFO_TX_PIN,PM_PIN_PULLUP_1M);
    gpio_set_func(DEBUG_INFO_TX_PIN, AS_GPIO);
	gpio_write(DEBUG_INFO_TX_PIN, 1);
    gpio_set_output_en(DEBUG_INFO_TX_PIN, 1);
    gpio_set_input_en(DEBUG_INFO_TX_PIN, 0);	
}

/* Put it into a function independently, to prevent the compiler from 
 * optimizing different pins, resulting in inaccurate baud rates.
 */
_attribute_ram_code_ 
_attribute_no_inline_ 
static void uart_do_put_char(u32 pcTxReg, u8 *bit)
{
	int j;
#if BAUD_USE == SIMU_BAUD_1M
	/*! Make sure the following loop instruction starts at 4-byte alignment */
	// _ASM_NOP_; 
	
	for(j = 0;j<10;j++) 
	{
	#if CLOCK_SYS_CLOCK_HZ == 16000000
		CLOCK_DLY_8_CYC;
	#elif CLOCK_SYS_CLOCK_HZ == 32000000
		CLOCK_DLY_7_CYC;CLOCK_DLY_7_CYC;CLOCK_DLY_10_CYC;
	#elif CLOCK_SYS_CLOCK_HZ == 48000000
		CLOCK_DLY_8_CYC;CLOCK_DLY_8_CYC;CLOCK_DLY_10_CYC;
		CLOCK_DLY_8_CYC;CLOCK_DLY_6_CYC;
	#else
	#error "error CLOCK_SYS_CLOCK_HZ"
	#endif
		write_reg8(pcTxReg, bit[j]); 	   //send bit0
	}
#else
	u32 t1 = 0, t2 = 0;
	t1 = read_reg32(0x740);
	for(j = 0;j<10;j++)
	{
		t2 = t1;
		while(t1 - t2 < BIT_INTERVAL_SYS_TICK){
			t1	= read_reg32(0x740);
		}
		write_reg8(pcTxReg,bit[j]); 	   //send bit0
	}
#endif
}


/**
 * @brief  Send a byte of serial data.
 * @param  byte: Data to send.
 * @retval None
 */
_attribute_ram_code_ static void uart_put_char(u8 byte){
	if (!tx_pin_initialed) {
	    debug_info_tx_pin_init();
		tx_pin_initialed = 1;
	}
	volatile u32 pcTxReg = (0x583+((DEBUG_INFO_TX_PIN>>8)<<3));//register GPIO output
	u8 tmp_bit0 = read_reg8(pcTxReg) & (~(DEBUG_INFO_TX_PIN & 0xff));
	u8 tmp_bit1 = read_reg8(pcTxReg) | (DEBUG_INFO_TX_PIN & 0xff);


	u8 bit[10] = {0};
	bit[0] = tmp_bit0;
	bit[1] = (byte & 0x01)? tmp_bit1 : tmp_bit0;
	bit[2] = ((byte>>1) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[3] = ((byte>>2) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[4] = ((byte>>3) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[5] = ((byte>>4) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[6] = ((byte>>5) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[7] = ((byte>>6) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[8] = ((byte>>7) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[9] = tmp_bit1;

	/*! Minimize the time for interrupts to close and ensure timely 
	    response after interrupts occur. */
	u8 r = 0;
	if(SIMU_UART_IRQ_EN){
		r = irq_disable();
	}
	uart_do_put_char(pcTxReg, bit);
	if(SIMU_UART_IRQ_EN){
		irq_restore(r);
	}
}

/**
 * @brief  Send serial datas.
 * @param  p: Data pointer to send.
 * @param  len: Data length to send.
 * @retval None
 * @note   Previously, the irq_disable and irq_restore functions were 
 *   placed at the beginning and end of this function, which caused the 
 *   interrupt to be turned off for a long time when sending long data, 
 *   causing some interrupted code to fail to execute in time.
 */
_attribute_ram_code_ void uart_simu_send_bytes(u8 *p,int len)
{
    while(len--){
        uart_put_char(*p++);
    }
}


#endif
