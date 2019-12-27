/********************************************************************************************************
 * @file     spi.c 
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
#include "spi.h"

#if(MCU_CORE_TYPE != MCU_CORE_8263)

///////////////  SPI master driver /////////////////////////////
void spi_select(int i){
	reg_spi_ctrl = FLD_SPI_MASTER_MODE_EN;
}

void spi_no_select(int i){
	reg_spi_ctrl = FLD_SPI_CS | FLD_SPI_MASTER_MODE_EN;
}

static inline void spi_wait () {
	while(reg_spi_ctrl & FLD_SPI_BUSY);
}

void spi_init (int divider, int mode) {
	reg_spi_ctrl = FLD_SPI_MASTER_MODE_EN;
	reg_spi_sp = FLD_SPI_ENABLE | divider;		//force PADs act as spi

}

int spi_read_write (u8 * buff_w, u8 * buff_r, int len) {
	for (int i=0; i<len; i++) {
		reg_spi_data = buff_w[i];
		spi_wait();
		buff_r[i] = reg_spi_data;
		//sleep_us (20);
	}
	return len;
}

void spi_write(u8 d){
	reg_spi_data = d;
	spi_wait();
}

u8 spi_read(){
	reg_spi_data = 0;
	spi_wait();
	u8 d = reg_spi_data;
	return d;
}

///////////////  SPI slave driver /////////////////////////////
u8 *spi_slave_write_buff, *spi_slave_read_buff;

static int spi_irq_pin = 0;
static spi_callback_func spi_callback;
void spi_slave_init(int divider, u8 *buff){
	reg_gpio_pe_gpio &= ~ BITS(6,7);
	reg_gpio_pf_gpio &= ~ BITS(0,1);

	reg_spi_ctrl = FLD_SPI_ADDR_AUTO;
	reg_spi_sp = FLD_SPI_ENABLE;			//force PADs act as spi

	spi_slave_write_buff = buff;
	spi_slave_read_buff = buff + 64;

}

void spi_irq_callback(){
	if(spi_callback && gpio_read(GPIO_CN)){
		if(*(u32*)(spi_slave_write_buff) != 0){		// check flag
			spi_callback(spi_slave_write_buff);
			*(u32*)(spi_slave_write_buff) = 0;			// clear flag
		}else{
			*(u32*)(spi_slave_read_buff) = 0;			// clear flag
		}
	}
}

int spi_notify_data_ready(void){
	if(spi_irq_pin){
		gpio_set_output_en(spi_irq_pin, 1);
		sleep_us(100);
		gpio_set_output_en(spi_irq_pin, 0);
	}
	return 0;
}

void spi_send_response(u8 *buf, int len){
	if(len > 64) return;
	memcpy4(&spi_slave_read_buff[0], buf, len+3);
	spi_notify_data_ready();
}

void spi_slave_register(int pin, spi_callback_func callback){
	spi_slave_init(32, (u8*)0x80bf80);

	gpio_set_func(pin, AS_GPIO);
	gpio_write(pin, 0);
	gpio_set_input_en(pin, 1);
	spi_irq_pin = pin;
	spi_callback = callback;

//	gpio_set_func(GPIO_CN, AS_GPIO);
	gpio_set_input_en(GPIO_CN, 1);
	gpio_set_interrupt(GPIO_CN, 0);	// rising edge

	reg_irq_mask |= FLD_IRQ_GPIO_RISC2_EN;

}
void spi_slave_deregister(void){
	gpio_clr_interrupt(GPIO_CN);
	spi_callback = 0;
}

#endif

