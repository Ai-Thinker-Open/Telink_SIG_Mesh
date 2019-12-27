/********************************************************************************************************
 * @file     e2prom.h 
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

/***************************
 * @brief	EEPROM initiate, call this function to initiate the whole storage space
 *
 * @param	none
 *
 * @return	none
 */
void e2prom_init();

/***************************
 * @brief	write variable length data to the e2prom
 *
 * @param	adr:	destination e2prom address
 * 			p:		stored data space address
 * 			len:	data length need to be stored
 *
 * @return	none
 */
void e2prom_write (int adr, u8 *p, int len);

/***************************
 * @brief	read variable length data to from e2prom
 *
 * @param	adr:	destination address
 * 			p:		address used to store read bytes
 * 			len:	read byte length
 *
 * @return	none
 */
void e2prom_read (int adr, u8 *p, int len);
