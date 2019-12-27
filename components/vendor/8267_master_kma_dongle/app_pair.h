/********************************************************************************************************
 * @file     app_pair.h 
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

#ifndef APP_PAIR_H_
#define APP_PAIR_H_


int user_tbl_slave_mac_add(u8 adr_type, u8 *adr);
int user_tbl_slave_mac_search(u8 adr_type, u8 * adr);

int user_tbl_slave_mac_delete_by_adr(u8 adr_type, u8 *adr);
void user_tbl_salve_mac_unpair_proc(void);

void user_master_host_pairing_management_init(void);


#endif /* APP_PAIR_H_ */
