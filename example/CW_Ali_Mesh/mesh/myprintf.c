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
#if PRINT_DEBUG_INFO

#include "myprintf.h"

#define		BIT_INTERVAL_SYS_TICK	(CLOCK_SYS_CLOCK_1S/BAUD_USE)


_attribute_ram_code_ static void uart_put_char(u8 byte){
	u8 j = 0;
	u32 t1 = 0,t2 = 0;
#if (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
	gpio_set_func(DEBUG_INFO_TX_PIN, AS_GPIO);
	gpio_set_output_en(DEBUG_INFO_TX_PIN, 1);
	gpio_set_input_en(DEBUG_INFO_TX_PIN, 0);
#else
	REG_ADDR8(0x582+((DEBUG_INFO_TX_PIN>>8)<<3)) &= ~(DEBUG_INFO_TX_PIN & 0xff) ;//Enable output
#endif
	u32 pcTxReg = (0x583+((DEBUG_INFO_TX_PIN>>8)<<3));//register GPIO output
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

	//u8 r = irq_disable();
	t1 = read_reg32(0x740);
	for(j = 0;j<10;j++)
	{
		t2 = t1;
		while(t1 - t2 < BIT_INTERVAL_SYS_TICK){
			t1  = read_reg32(0x740);
		}
		write_reg8(pcTxReg,bit[j]);        //send bit0
	}
	//irq_restore(r);
}
_attribute_ram_code_ void uart_simu_send_bytes(u8 *p,int len)
{
    gpio_setup_up_down_resistor(DEBUG_INFO_TX_PIN,PM_PIN_PULLUP_1M);
    #if SIMU_UART_IRQ_EN
    u8 r = irq_disable();
    #endif
    while(len--){
        uart_put_char(*p++);
    }
    #if SIMU_UART_IRQ_EN
    irq_restore(r);
    #endif
}


#endif
