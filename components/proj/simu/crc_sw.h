/********************************************************************************************************
 * @file     crc_sw.h 
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

/**************************** Public Macro Definitions ***********************/

#define CRC_MODE0_POLY  0x1021
#define CRC_MODE1_POLY  0x8005
#define CRC_MODE2_POLY  0x8bb7
#define CRC_MODE3_POLY  0x04c11db7

enum {
    CRC_MODE_0 = 0,
    CRC_MODE_1,
    CRC_MODE_2,
    CRC_MODE_3,
};

/**************************** Public Type Definitions ************************/

/* None */

/**************************** Public Enum Definitions ************************/

/* None */

/**************************** Public Variable Declaration ********************/

extern u16 crc16_mode_poly[3];

/**************************** Public Constant Declaration ********************/

/* None */

/**************************** Public Function Prototypes *********************/

void crc32_buildTable(u8 endian);
u32 crc32_getCrc(const u8 *str, u32 count, u8 endian);
u32 crc_reverseBitU32(const u32 src);
u8 reverseBitU8(const u8 src);
u32 crc32_bitwise(u8 *p_stream, u32 len_m1, u32 crc32_poly);
u16 crc16_bitwise(u8 *p_stream, u32 len_m1, u16 crc16_poly);

u16 crc16_calc(u8* crcArray, u8 len);



