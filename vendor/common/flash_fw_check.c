/********************************************************************************************************
 * @file     flash_fw_check.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 12, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
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

#include "flash_fw_check.h"
#include "drivers/8258/flash.h"

extern _attribute_data_retention_   u32		ota_program_offset;
extern _attribute_data_retention_	int		ota_program_bootAddr;
extern unsigned long crc32_half_cal(unsigned long crc, unsigned char* input, unsigned long* table, int len);

static const unsigned long fw_crc32_half_tbl[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

#define FW_READ_SIZE      256 ///16 ///256 require more stack space

u32 fw_crc_init = 0xFFFFFFFF;

/***********************************
 * this function must be called after the function cpu_wakeup_init.
 * cpu_wakeup_init will set the ota_program_offset value.
 */
bool flash_fw_check( u32 crc_init_value ){

	if(!crc_init_value){
		fw_crc_init = 0xFFFFFFFF;
	}else{
		fw_crc_init = crc_init_value;
	}

	/////find the real FW flash address
	u32 fw_flashAddr;
	if(!ota_program_offset){ ////zero, firmware is stored at flash 0x20000.
		fw_flashAddr = ota_program_bootAddr; ///NOTE: this flash offset need to set according to OTA offset
	}else{                   ////note zero, firmware is stored at flash 0x00000.
		fw_flashAddr = 0x00000;
	}

	u32 fw_size;
	flash_read_page( (fw_flashAddr+0x18), 4, (u8*)&fw_size); ///0x18 store bin size value

	u16 fw_Block;
	u16 fw_remainSizeByte;
	fw_Block = fw_size/FW_READ_SIZE;
	fw_remainSizeByte = fw_size%FW_READ_SIZE;


	int i = 0;
	u8 fw_tmpdata[FW_READ_SIZE]; ///
	u8 ota_dat[FW_READ_SIZE<<1];
	for(i=0; i < fw_Block; i++){ ///Telink bin must align 16 bytes.

		flash_read_page( (fw_flashAddr+i*FW_READ_SIZE), FW_READ_SIZE, fw_tmpdata);

		//将FW_READ_SIZE byte OTA data拆成32 个 half byte计算CRC
		for(int i=0;i<FW_READ_SIZE;i++){
			ota_dat[i*2] = fw_tmpdata[i]&0x0f;
			ota_dat[i*2+1] = fw_tmpdata[i]>>4;
		}
		fw_crc_init = crc32_half_cal(fw_crc_init, ota_dat, (unsigned long* )fw_crc32_half_tbl, (FW_READ_SIZE<<1));
	}

	//////////////////////////////
	if(fw_remainSizeByte != 4){
		flash_read_page( (fw_flashAddr+fw_size -fw_remainSizeByte), (fw_remainSizeByte-4), fw_tmpdata);
		for(int i=0;i<(fw_remainSizeByte-4);i++){
			ota_dat[i*2] = fw_tmpdata[i]&0x0f;
			ota_dat[i*2+1] = fw_tmpdata[i]>>4;
		}
		fw_crc_init = crc32_half_cal(fw_crc_init, ota_dat, (unsigned long* )fw_crc32_half_tbl, ((fw_remainSizeByte-4)<<1));
	}

	////////read crc value and compare
	u32 fw_check_value;
	flash_read_page( (fw_flashAddr+fw_size-4), 4, (u8*)&fw_check_value);

	if(fw_check_value != fw_crc_init){
		return 1;  ///CRC check fail
	}

	return 0; ///CRC check ok
}





















