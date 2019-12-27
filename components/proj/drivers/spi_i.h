/********************************************************************************************************
 * @file     spi_i.h 
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
#include "proj/mcu/config.h"
#include "../config/user_config.h"
#include "../mcu/gpio.h"
#include "../mcu/register.h"
#ifndef WIN32
#if(__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/spi_i.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "drivers/8278/spi_i.h"
#else

// use static inline, because, spi flash code must reside in memory..
// these code may be embedd in flash code

static inline void mspi_wait(void){
	while(reg_master_spi_ctrl & FLD_MASTER_SPI_BUSY)
		;
}

static inline void mspi_high(void){
	reg_master_spi_ctrl = FLD_MASTER_SPI_CS;
}

static inline void mspi_low(void){
	reg_master_spi_ctrl = 0;
}

static inline u8 mspi_get(void){
	return reg_master_spi_data;
}

static inline void mspi_write(u8 c){
	reg_master_spi_data = c;
}

static inline void mspi_ctrl_write(u8 c){
	reg_master_spi_ctrl = c;
}

static inline u8 mspi_read(void){
	mspi_write(0);		// dummy, issue clock 
	mspi_wait();
	return mspi_get();
}
#endif
#endif
