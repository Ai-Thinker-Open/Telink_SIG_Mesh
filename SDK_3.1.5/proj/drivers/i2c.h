/********************************************************************************************************
 * @file     i2c.h 
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

#include "../common/types.h"

#define I2C_VENDOR_SEND_WAIT_MODE  1
#define I2C_VENDOR_SEND_DIRECT_MODE 2

#define PIN_I2C_SCL				GPIO_CK
#define PIN_I2C_SDA				GPIO_DI

void i2c_sim_init(void);
void i2c_sim_write(u8 id, u8 addr, u8 dat);
u8 i2c_sim_read(u8 id, u8 addr);
void i2c_sim_burst_read(u8 id, u8 addr, u8 *p, int n);
void i2c_sim_burst_write(u8 id, u8 addr,u8 *p,int n);

void i2c_init (void);
int i2c_burst_write(u8 id, u16 adr, u8 * buff, int len);
int i2c_burst_read(u8 id, u16 adr, u8 * buff, int len);
void i2c_write(u8 id, u16 adr, u8 dat);
u8 i2c_read(u8 id, u16 adr);

typedef void (*i2c_callback_func)(u8 *);
void i2c_slave_register(int pin, i2c_callback_func callback);
void i2c_send_response(u8*,int);

void i2c_vendor_slave_init(u32 rd_mem_addr, u32 wt_mem_addr);
void i2c_vendor_send_response(u8 *buf, int len);
void i2c_vendor_slave_register(u32 rd_mem_addr, u32 wt_mem_addr, u8 rd_buf_size, u8 wt_buf_size, int pin, i2c_callback_func callback);
void i2c_vendor_slave_deregister(void);
int i2c_vendor_rx_data_timerCb (void* arg);
void  i2c_vendor_reset_write_buf(void);
void i2c_write_start(u8 id);
void i2c_read_start_buf(u8 id,u8 *p_buff,u8 len);







