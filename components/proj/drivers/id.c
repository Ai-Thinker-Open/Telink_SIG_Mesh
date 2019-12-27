/********************************************************************************************************
 * @file     id.c 
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

#include "../mcu/register.h"
#include "../common/assert.h"
#include "id.h"

static void id_set_magic_enable(){
	reg_id_wr_en = ID_WRITE_ENABLE_MAGIC;
}

void id_set_product_id(u8 function_id, u8 version_id, u16 production_id){
	id_set_magic_enable();
	reg_product_id = (production_id << 16 | version_id << 8 | function_id);
}

