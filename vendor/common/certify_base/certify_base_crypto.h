/********************************************************************************************************
 * @file     certify_base_crypto.h 
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

#ifndef __CERTIFY_BASE_CRYPTO__
#define __CERTIFY_BASE_CRYPTO__

#include <stdint.h>
#include "../user_config.h"

#ifndef NULL
#define NULL 	0
#endif
#define URI_CERTIFY_ID	0x0000
#define DEV_CERTIFY_ID	0x0001


#define MAX_PEM_CERT_LEN 0x300
extern u8 der_cert_buf[MAX_PEM_CERT_LEN];
extern u32 der_buf_len;

		// suppose the max record id is 16
#define MAX_PROV_RECORD_CNT 	16

typedef enum{
	RECORD_PROV_SUC =0,
	RECORD_REQ_IS_NOT_PRESENT,
	RECORD_REQ_OFF_OUTBOUND,
	RECORD_RFU,
}RECORD_REQ_STS;

int  cert_base_func_init();
void cert_base_set_key(u8 *pk,u8 *sk);
void get_cert_id_list(u8 *p_list,u32 *p_len);
const char * get_cert_content_by_id(u16 id,u32* p_len);
void cert_id_get(u16 *p_id,u32 *p_cnt);

void prov_clear_all_rec();
void prov_set_rec_id(u16 *p_rec,u8 len);
int prov_set_buf_len(u16 rec_id,u16 offset,u8 *p_buf,u32 len,u32 total);
int prov_use_rec_id_get_pubkey(u16 rec_id,u8 *p_pubkey);
int cert_item_rsp(u16 id,u16 offset,u16 max_size,u8 *p_buf,u16 *p_len);
u32  use_cert_id_get_len(u8 id);

void record_mag_init();
void record_mag_set(u16 rec_id,u16 max_size,u16 offset);
void record_mag_get(u16 *p_rec_id,u16 *p_max_size,u16 *p_offset);
void record_mag_get_max_size(u16 *p_max_size);


#endif
