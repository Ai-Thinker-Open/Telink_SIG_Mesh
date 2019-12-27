/********************************************************************************************************
 * @file     compatibility.c 
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

int LoadTblCmdSet (	const TBLCMDSET * pt, int size){
	int l=0;

	while (l<size) {
		unsigned int  cadr = ((u32)0x800000) | pt[l].adr;
		unsigned char cdat = pt[l].dat;
		unsigned char ccmd = pt[l].cmd;
		unsigned char cvld =(ccmd & TCMD_UNDER_WR);
		ccmd &= TCMD_MASK;
		if (cvld) {
			if (ccmd == TCMD_WRITE) {
				write_reg8 (cadr, cdat);
			}
			else if (ccmd == TCMD_WAREG) {
				WriteAnalogReg (cadr, cdat);
			}
			else if (ccmd == TCMD_WAIT) {
				WaitUs (pt[l].adr*256 + cdat);
			}
		}
		l++;
	}
	return size;
	
}





