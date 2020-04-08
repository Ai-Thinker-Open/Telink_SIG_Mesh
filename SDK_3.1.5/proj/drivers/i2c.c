/********************************************************************************************************
 * @file     i2c.c 
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
#include "../mcu/clock.h"
#include "i2c.h"

#if(MCU_CORE_TYPE != MCU_CORE_8263)

//////////////////////////////////////////////////////////////////////
///// i2c_nb: non-blocking access
//////////////////////////////////////////////////////////////////////
#ifndef I2C_USE_SIMULATION
#define I2C_USE_SIMULATION	0
#endif
#ifndef I2C_SPEED
#define I2C_SPEED	2000
#endif
#ifndef I2C_16BIT_ADDR
#define I2C_16BIT_ADDR		0
#endif

static inline int i2c_busy(){
	return(reg_i2c_status & FLD_I2C_CMD_BUSY);
}

static inline void i2c_nb_write_adr8_dat(u8 adr, u8 dat){
	reg_i2c_dat_ctrl = adr | (dat << 16) |
			((FLD_I2C_CMD_START | FLD_I2C_CMD_STOP | FLD_I2C_CMD_ID |
				FLD_I2C_CMD_ADR | FLD_I2C_CMD_DI) << 24);
}

static inline void i2c_nb_write_adr16_dat(u16 adr, u8 dat){
	reg_i2c_dat_ctrl = (adr>>8) | ((adr&0xff)<<8) | (dat << 16) |
			((FLD_I2C_CMD_START | FLD_I2C_CMD_STOP | FLD_I2C_CMD_ID |
				FLD_I2C_CMD_ADR | FLD_I2C_CMD_DI | FLD_I2C_CMD_DO) << 24);
}

static inline void i2c_nb_write_start8(u8 adr, u8 stop){
	reg_i2c_dat_ctrl = adr |
			((FLD_I2C_CMD_START | (stop ? FLD_I2C_CMD_STOP : 0) |
				FLD_I2C_CMD_ID | FLD_I2C_CMD_ADR ) << 24);
}

static inline void i2c_nb_write_start16(u16 adr, u8 stop){
	reg_i2c_dat_ctrl = (adr>>8) | ((adr&0xff)<<8) |
			((FLD_I2C_CMD_START | (stop ? FLD_I2C_CMD_STOP : 0) |
				FLD_I2C_CMD_ID | FLD_I2C_CMD_ADR | FLD_I2C_CMD_DO) << 24);
}

static inline void i2c_nb_write_byte(u8 dat, u8 stop){
	reg_i2c_di_ctrl = dat | ((FLD_I2C_CMD_DI | (stop ? FLD_I2C_CMD_STOP : 0)) << 8);
}

///////////// for read command ///////////////////////////////////////////////
static inline void i2c_nb_read_byte(){
	reg_i2c_ctrl = FLD_I2C_CMD_START | FLD_I2C_CMD_ID | FLD_I2C_CMD_READ_ID |
			FLD_I2C_CMD_DI | FLD_I2C_CMD_STOP;
}

static inline void i2c_nb_read_start(u8 stop){
	reg_i2c_ctrl = FLD_I2C_CMD_START | FLD_I2C_CMD_ID | FLD_I2C_CMD_DI |
			FLD_I2C_CMD_READ_ID |(stop ? FLD_I2C_CMD_STOP | FLD_I2C_CMD_NAK : 0);
}

static inline void i2c_nb_read_next(u8 stop){
	reg_i2c_ctrl = FLD_I2C_CMD_DI | FLD_I2C_CMD_READ_ID |
			(stop ? FLD_I2C_CMD_STOP | FLD_I2C_CMD_NAK : 0);
}

#if 1
int i2c_nb_write(u16 adr, u8 * buff, int len){
	static int status = 0;
	// 0: idle
	// 1: start-id-address
	// 2+: data
	if(status && i2c_busy()){
		return 0;		// i2c busy
	}

	if(!status){	// start i2c write command
		if(I2C_16BIT_ADDR){
			i2c_nb_write_start16(adr, len == 0);
		}
		else {
			i2c_nb_write_start8(adr, len == 0);
		}
	}
	else {
		int offset = status - 1;
		if(offset >= len){
			status = 0;
			if(reg_i2c_status & FLD_I2C_NAK){
				return -1;
			}
			else {
				return 1;
			}
		}
		else {
			i2c_nb_write_byte(buff[offset], status == len);
		}
	}
	status ++;
	return 0;
}

int i2c_nb_read(u16 adr, u8 * buff, int len){
	static int status = 0;
	// 0: idle
	// 1: start-write_id-address-stop
	// 2: start-read_id
	// 3+: data
	if(status && i2c_busy()){
		return 0;		// i2c busy
	}

	if(!status){	// start i2c write address command
		if(I2C_16BIT_ADDR){
			i2c_nb_write_start16(adr, FLD_I2C_CMD_STOP);
		}
		else {
			i2c_nb_write_start8(adr, FLD_I2C_CMD_STOP);
		}
	}
	else if(status == 1){ // start i2c read command
		if(reg_i2c_status & FLD_I2C_NAK || !len){
			status = 0;
			return -1;
		}
		i2c_nb_read_start(len == 1);
	}
	else {
		buff[status - 2] = reg_i2c_di;
		if(status > len){
			status = 0;
			return 1;
		}
		else {
			i2c_nb_read_next(status == len);
		}
	}
	status ++;
	return 0;
}
#endif
void i2c_init(void){
#if(I2C_USE_SIMULATION)
	i2c_init();
	i2c_sim_init();
#else
	// ¡Á¡é¨°areg_rst_clk0 ¦Ì?I2C  clk ¡ä¨°?a¨¢??e?
	reg_gpio_pe_gpio &= ~ BIT(7);
	reg_gpio_pf_gpio &= ~ BIT(1);

	analog_write(20, analog_read(20) | (GPIO_PULL_UP_10K<<2) | (GPIO_PULL_UP_10K<<6));	//  CK, DI, pullup 10K

	reg_i2c_set =(CLOCK_SYS_CLOCK_1MS / (I2C_SPEED * 4)) |(FLD_I2C_MODE_MASTER << 24);	/*  don't know why   speed * 4   */
	reg_spi_sp = 0;		//force PADs act as I2C
#endif
}

int i2c_burst_write(u8 id, u16 adr, u8 * buff, int len){
#if(I2C_USE_SIMULATION)
	i2c_sim_burst_write(id, adr, buff, len);
	return 0;
#else
	reg_i2c_id = id;
	int ret;
	do {
		ret = i2c_nb_write(adr, buff, len);
	} while(ret == 0);
	return !ret;
#endif
}

int i2c_burst_read(u8 id, u16 adr, u8 * buff, int len){
#if(I2C_USE_SIMULATION)
	i2c_sim_burst_read(id, adr, buff, len);
	return 0;
#else
	reg_i2c_id = id;
	int ret;
	do {
		ret = i2c_nb_read(adr, buff, len);
	} while(ret == 0);
	return !ret;
#endif
}

void i2c_write(u8 id, u16 adr, u8 dat){
#if(I2C_USE_SIMULATION)
	i2c_sim_write(id, adr, dat);
#else
	i2c_burst_write(id, adr, &dat, 1);
#endif
}

u8 i2c_read(u8 id, u16 adr){
#if(I2C_USE_SIMULATION)
	return i2c_sim_read(id, adr);
#else
	u8 dat;
	i2c_burst_read(id, adr, &dat, 1);
	return dat;
#endif
}

///////////////  I2C slave driver /////////////////////////////

#define I2C_SLAVE_SELF_ID		0x6e
// To use I2c slave:
// 1, CK, DI must not be GPIO mode.
// 2, CK, DI must 10K pullup or stronger
// 3, mem_addr is a address equal to  (0x808000 + real_mem), such as 0x809ea0
// 4, mem_addr is the write buffer, whereas (mem_addr + 64) is the read buffer
// 5, i2c_slave_init() is called instead of i2c_init()
u8 *i2c_slave_write_buff, *i2c_slave_read_buff;
void i2c_slave_init(u32 mem_addr){
	// ¡Á¡é¨°areg_rst_clk0 ¦Ì?I2C  clk ¡ä¨°?a¨¢??e?
	reg_gpio_pe_gpio &= ~ BIT(7);
	reg_gpio_pf_gpio &= ~ BIT(1);

	analog_write(20, analog_read(20) | (GPIO_PULL_UP_10K<<2) | (GPIO_PULL_UP_10K<<6));	//  CK, DI, pullup 10K

	reg_i2c_set = (I2C_SLAVE_SELF_ID << 8) | ((FLD_I2C_ADDR_AUTO | FLD_I2C_MEM_MAP) << 24);
	reg_spi_sp = 0;														//force PADs act as I2C
	reg_i2c_mem_map = mem_addr;
	i2c_slave_write_buff = (u8*)mem_addr;
	i2c_slave_read_buff = (u8*)(mem_addr + 64);
}
// sample  receive and send  command handling, pseudo code
#if(0)
void i2c_slave_sample_recv_command_poll(void){
	if(i2c_slave_write_buff[0] == CMD_OK){
		gpio_write (HOST_INTERRUPT_PIN, !HOST_INTERRUPT_LEVEL);	// clear interrupt pin
		parse_cmd();
		do_cmd();
	}
}
void i2c_slave_sample_send_command(void){
	foreach(i, 10){
		i2c_slave_read_buff[i] = data[i];
	}
	gpio_write (HOST_INTERRUPT_PIN, HOST_INTERRUPT_LEVEL); // set interrupt, to notify host to read
}
#endif

static int i2c_irq_pin = 0;
static i2c_callback_func i2c_callback;
void i2c_check_cmd(){
	if(i2c_callback){
		if((reg_i2c_irq_status & (FLD_I2C_STATUS_WR | FLD_I2C_STATUS_RD)) == FLD_I2C_STATUS_WR){
			i2c_callback(i2c_slave_write_buff);
		}else if((reg_i2c_irq_status & (FLD_I2C_STATUS_WR | FLD_I2C_STATUS_RD)) == (FLD_I2C_STATUS_WR | FLD_I2C_STATUS_RD)){
			*(u32*)(i2c_slave_read_buff) = 0;
		}
		reg_i2c_clr_status = (FLD_I2C_STATUS_WR | FLD_I2C_STATUS_RD);
	}

}

int i2c_notify_data_ready(void){
	gpio_set_output_en(i2c_irq_pin, 1);
	sleep_us(100);
	gpio_set_output_en(i2c_irq_pin, 0);
	return 0;
}
void i2c_send_response(u8 *buf, int len){
	if(len > 64) return;
	memcpy4(&i2c_slave_read_buff[0], buf, len+3);
	i2c_notify_data_ready();
}

void i2c_slave_register(int pin, i2c_callback_func callback){
	i2c_slave_init(0x80c000 - 128);

	gpio_set_func(pin, AS_GPIO);
	gpio_write(pin, 0);
	gpio_set_input_en(pin, 1);
	i2c_irq_pin = pin;
	i2c_callback = callback;

#if (SP_SRC_I2C)
	ev_on_poll(EV_POLL_I2C_CMD, i2c_check_cmd);
#endif
}

void i2c_slave_deregister(void){
	i2c_callback = 0;
}


#endif

///////////////  I2C simulation ////////////////////////////////////

#ifndef PIN_I2C_SCL
#define PIN_I2C_SCL				GPIO_CK
#endif
#ifndef PIN_I2C_SDA
#define PIN_I2C_SDA				GPIO_DI
#endif

static inline void i2c_sim_wait(void){
}
void i2c_sim_long_wait(void){
	CLOCK_DLY_600NS;
}

// Pulling the line to ground is considered a logical zero while letting the line float is a logical one.   http://en.wikipedia.org/wiki/I%C2%B2C
static inline void i2c_sim_scl_out(int v){
	gpio_set_output_en(PIN_I2C_SCL,(!v));
}

static inline int i2c_sim_scl_in(void){
	return gpio_read(PIN_I2C_SCL);
}

// Pulling the line to ground is considered a logical zero while letting the line float is a logical one.   http://en.wikipedia.org/wiki/I%C2%B2C
static inline void i2c_sim_sda_out(int v){
	gpio_set_output_en(PIN_I2C_SDA,(!v));
}

static inline int i2c_sim_sda_in(void){
	return gpio_read(PIN_I2C_SDA);
}

static inline void i2c_sim_scl_init(void){
	gpio_set_func(PIN_I2C_SCL, AS_GPIO);
}

static inline void i2c_sim_sda_init(void){
	gpio_set_func(PIN_I2C_SDA, AS_GPIO);
	gpio_set_input_en(PIN_I2C_SDA, 1);
}

static inline void i2c_sim_scl_idle(void){
	gpio_set_output_en(PIN_I2C_SCL, 0);
	gpio_write(PIN_I2C_SCL, 0);
}

static inline void i2c_sim_sda_idle(void){
	gpio_set_output_en(PIN_I2C_SDA, 0);
	gpio_write(PIN_I2C_SDA, 0);
}


void i2c_sim_init(){}

/*
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\ void i2c_sim_start(void)
\\   Sets clock high, then data high.  This will do a stop if data was low.
\\   Then sets data low, which should be a start condition.
\\   After executing, data is left low, while clock is left high
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
*/
void i2c_sim_start(void)
{
	i2c_sim_scl_init();
	i2c_sim_sda_init();
	i2c_sim_sda_idle();
	i2c_sim_scl_idle();
	i2c_sim_sda_out(0);		//sda: 0
	i2c_sim_wait();

}

/*
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\ void i2c_sim_stop(void)
\\  puts data low, then clock low,
\\  then clock high, then data high.
\\  This should cause a stop, which
\\  should idle the bus, I.E. both clk and data are high.
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
*/
void i2c_sim_stop(void)
{
	i2c_sim_sda_out(0);
	i2c_sim_wait();
	i2c_sim_scl_out(0);
	i2c_sim_wait();
	i2c_sim_scl_out(1);
	i2c_sim_wait();
	i2c_sim_sda_out(1);
}

static void i2c_sim_wirte_bit(int bit)
{
	i2c_sim_scl_out(0);
	i2c_sim_sda_out(bit);
	i2c_sim_long_wait();
	i2c_sim_scl_out(1);
}

// Read a bit from I2C bus
static int i2c_sim_read_bit(void){
	i2c_sim_wirte_bit(1);
	return i2c_sim_sda_in();
}

int i2c_sim_write_byte(u8 dat){
	int i = 0x80;
	while(i){
		i2c_sim_wirte_bit((dat & i));
		i = i >> 1;
	}
	return i2c_sim_read_bit();
}

u8 i2c_sim_read_byte(int last){
	u8 dat = 0;
	foreach(i, 8){
		i2c_sim_wirte_bit(1);
		if(i2c_sim_sda_in()){
			dat =(dat << 1) | 0x01;
		}else{
			dat = dat << 1;
		}
	}
	i2c_sim_wirte_bit(last);
	return dat;
}

void i2c_sim_write(u8 id, u8 addr, u8 dat)
{
	i2c_sim_start();
	i2c_sim_write_byte(id);
	i2c_sim_write_byte(addr);
	i2c_sim_write_byte(dat);
	i2c_sim_stop();
}

u8 i2c_sim_read(u8 id, u8 addr)
{
	u8 dat;
	i2c_sim_burst_read(id, addr, &dat, 1);
	return dat;
}

void i2c_sim_burst_read(u8 id, u8 addr,u8 *p, int n)
{
	i2c_sim_start();

	i2c_sim_write_byte(id);
	i2c_sim_write_byte(addr);
	//i2c_sim_sda_out(1);
	//i2c_sim_scl_out(0);
	//i2c_sim_long_wait();
	//i2c_sim_scl_out(1);
	//i2c_sim_sda_out(0);
	i2c_sim_stop();
	i2c_sim_start();

	i2c_sim_write_byte(id | 1);

	for(int k = 0; k < n; ++k){
		*p++ = i2c_sim_read_byte( k ==(n-1) );
	}
	i2c_sim_stop();

}

void i2c_sim_burst_write(u8 id, u8 addr,u8 *p, int n)
{
	i2c_sim_start();
	i2c_sim_write_byte(id);
	i2c_sim_write_byte(addr);
	foreach(i, n){
		i2c_sim_write_byte(*p++);
	}
	i2c_sim_stop();
}
void i2c_sim_reset_e2prom(void ){
	i2c_sim_sda_out(1);
	for(u8 i=0;i<8;i++){
		i2c_sim_scl_out(0);
		i2c_sim_long_wait();
		i2c_sim_scl_out(1);
		i2c_sim_long_wait();
	}
	i2c_sim_sda_out(0);
}

void i2c_write_start(u8 id)
{
    i2c_sim_start();
    i2c_sim_write_byte(id);
    i2c_sim_stop();
}

void i2c_read_start_buf(u8 id,u8 *p_buff,u8 len)
{
    i2c_sim_start();
	i2c_sim_write_byte(id | 1);
	for(int k = 0; k < len; ++k){
		*p_buff++ = i2c_sim_read_byte( k ==(len-1) );
	}
	i2c_sim_stop();
}



