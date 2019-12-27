/********************************************************************************************************
 * @file     hk_crc32_half.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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
#ifndef HK_CRC32_HALF_H_
#define HK_CRC32_HALF_H_
extern const unsigned long hk_crc32_half_tbl[16];
extern unsigned long hk_crc32_half_cal(unsigned long crc, unsigned char* input, unsigned long* table, int len);
#endif /* HK_CRC32_HALF_H_ */
