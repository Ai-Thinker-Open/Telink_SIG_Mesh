/********************************************************************************************************
 * @file     mijia_pub_proc.h 
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


#ifndef _MIJIA_PUB_PROC_C_
#define _MIJIA_PUB_PROC_C_
#include "./mijia_ble_api/mible_api.h"
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/user_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "../app_health.h"
//#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
//#include "proj_lib/mesh_crypto/ecc_dsa.h"
#include "../mesh_common.h"
#include "../ev.h"
#include "./libs/third_party/micro-ecc/uECC.h"
#include "./libs/third_party/micro-ecc/types.h"

typedef struct {
	u8 mode;
	u8 model_idx;
	u8 sts_pub_inter;
	u8  sts_pub_1st;
	// the unit is 100ms
	u32 last_100ms;
	u32 inter_100ms;
	u32 max_pub_100ms;
	u32 sts_change_100ms;
	model_common_t * p_model;
	// max pub units is s;
	u32 mp_inter;
	u32 mp_last_sec;
	u8 mp_rsp_flag;
	u8 mp_send_flag;
	u8 retry_cnt;
}mi_pub_str_t;


extern mi_pub_str_t mi_pub;
u8 pub_step_proc_cb(u8 sts_flag,model_common_t *p_model,u32 model_id);
void pub_max_inter_rcv_cb(u8 max_pub);
void mi_pub_clear_trans_flag();
void mi_pub_send_all_status();
void mi_pub_para_init(mi_pub_str_t *p_mi_pub);
void mi_pub_vd_sig_para_init();
void mi_pub_sigmodel_inter(u8 trans,u8 delay,u8 mode);




#endif

