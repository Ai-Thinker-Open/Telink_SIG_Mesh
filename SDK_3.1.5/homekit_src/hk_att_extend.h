/********************************************************************************************************
 * @file     hk_att_extend.h
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
#ifndef HK_ATT_EXTEND_H_
#define HK_ATT_EXTEND_H_

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "homekit_inc.h"
#include "homekit_def.h"


extern attribute_t *               	blt_pAtt;
extern rf_packet_att_write_t *     	blt_p_att_wr;
extern rf_packet_att_write_t *     	blt_p_att_wr_prep;
extern rf_packet_att_readBlob_t *  	blt_p_att_rd;
extern u8                          	blt_pair_ok;
extern u8 *                        	blt_write_response;
extern u8                         	blt_att_rd_buff[16];
extern u32 						   	blt_conn_wait_for_write;

extern u32						   	nonce_enc[2];
extern u32 							nonce_dec[2];

extern u32 							hap_timed_write_tick;
extern u32							hap_timed_write_timeout;
extern u32 							hap_transationID_valid_timeout;
extern u32 							hap_procedure_valid_timeout;
extern u32 							hap_remove_pairing_timeout;
extern u16 							hap_body_length;
extern u8	 						hap_write_ongoing;

extern void hk_l2cap_att_init ();
extern void hk_l2cap_att_main_loop ();
#endif /* HK_ATT_EXTEND_H_ */
