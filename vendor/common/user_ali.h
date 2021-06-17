/********************************************************************************************************
 * @file     user_ali.h 
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

#ifndef __USER_ALI_H
#define __USER_ALI_H
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/user_config.h"
#if !WIN32
extern const char num2char[17];
#endif
extern u32 con_product_id;// little endiness 
#if ((MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE) || (MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE))
extern const u8 con_mac_address[6];//small endiness
#else
extern u8  con_mac_address[6];//small endiness
#endif
extern u8 con_sec_data[16];

typedef struct{
	//u8 rev;
	u16 cid;
	union{
		u8 pid;
		struct{
		u8	adv_ver:4;
		u8 	ser_fun:1;
		u8  ota_en:1;
		u8  ble_ver:2;
		};
	};
	u8  product_id[4];
	u8 	mac[6];
	union{
		u8 feature_flag;
		struct{		
			u8 adv_flag:1;
			u8 uuid_ver:7;
		};
	};
	u8 	rfu[2];
}sha256_dev_uuid_str;

typedef struct{
	u8 auth_ok;
	u8 ble_key[16];
}ais_gatt_auth_t;

extern ais_gatt_auth_t ais_gatt_auth;
extern const u8 iv[];

void set_sha256_init_para_mode(u8 mode);
void set_dev_uuid_for_sha256();
void create_sha256_input_string(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret);
void create_sha256_input_string_node(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret);
void caculate_sha256_to_create_static_oob();
void caculate_sha256_to_create_pro_oob(u8 *pro_auth,u8 *random);
void caculate_sha256_node_auth_value(u8 *auth_value);
int ais_write_pipe(void *p);
void ais_gatt_auth_init();
void caculate_sha256_node_oob(u8 *p_oob ,u8 *p_random);
void mesh_ais_global_var_set();
void calculate_auth_value();

#endif

