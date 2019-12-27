/********************************************************************************************************
 * @file     e2prom.c 
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
#include "../../proj/tl_common.h"
#include "e2prom.h"
#include "../../proj/drivers/i2c.h"

#define						EEPROM_I2C_ID				0xa0
#define						EEPROM_SIZE					256
void e2prom_init(){
	//i2c_sim_reset_e2prom();
	u8 eraseByte[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	for(u8 i=0;i<=EEPROM_SIZE/8;i++)
		e2prom_write(i*8,eraseByte,8);
}
void e2prom_write (int adr, u8 *p, int len)
{
	int i = 0;
	while (len > 0)
	{
		int l = len >= 8 ? 8 : len;				//page write: 8-byte
		len -= l;
		i2c_sim_burst_write (EEPROM_I2C_ID, adr + i, p + i, l);
		i += l;
		sleep_us (5000);
	}
}
void e2prom_read (int adr, u8 *p, int len)
{
	i2c_sim_burst_read (EEPROM_I2C_ID, adr, p, len);
}
