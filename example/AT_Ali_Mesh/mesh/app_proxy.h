/********************************************************************************************************
 * @file     app_proxy.h 
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
#ifndef APP_PROXY_H
#define APP_PROXY_H
#include "mesh/light.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#define MAX_LIST_LEN 	16

#define ADV_PROXY_TYPE_NET_ID	0
#define ADV_PROXY_TYPE_NODE_IDENTTITY	1
typedef struct {
	u8 flag_len;
	u8 flag_type;
	u8 flag_data;
	u8 uuid_len;
	u8 uuid_type;
	u8 uuid_data[2];
	u8 serv_len;
	u8 serv_type;
	u8 serv_uuid[2];
	u8 identify_type;
	u8 net_id[8];
}proxy_adv_net_id;

typedef struct {
	u8 flag_len;
	u8 flag_type;
	u8 flag_data;
	u8 uuid_len;
	u8 uuid_type;
	u8 uuid_data[2];
	u8 serv_len;
	u8 serv_type;
	u8 serv_uuid[2];
	u8 identify_type;
	u8 hash[8];
	u8 random[8];
}proxy_adv_node_identity;
typedef enum{
	COMPLETE_MSG = 0,
	FIRST_SEG_MSG,
	CONTINUS_SEG_MSG,
	LAST_SEG_MSG,
}PROXY_DATA_SAR_TYPE;
typedef enum{
	PROXY_NET_WORK=0,
	PROXY_MESH_BEACON,
	PROXY_CONFIG,	
	PROXY_PROVISION,
	PROXY_RFU,
}PROXY_MSG_TYPE;
#define MAX_PROXY_RSV_LEN 	30
typedef struct{
	u8 trans_par_val;
	u8 type:6;
	u8 sar:2;
	u8 data[MAX_PROXY_RSV_LEN];
}proxy_msg_str;
typedef struct{
	u8 opcode;
	u8 para[50];
}proxy_config_pdu_sr;
typedef enum{
	PROXY_FILTER_SET_TYPE =0,
	PROXY_FILTER_ADDR_ADR,
	PROXY_FILTER_RM_ADR,
	PROXY_FILTER_STATUS,
}proxy_opcode_str;
typedef enum{
	FILTER_WHITE_LIST = 0,
	FILTER_BLACK_LIST,
	FILTER_RFU,
}filter_type_str;
typedef struct{
	u16 list_data[MAX_LIST_LEN];
	u8 list_idx[(MAX_LIST_LEN+7)/8];
}list_mag_str;
typedef struct{
	u8 filter_type;
	list_mag_str white_list;
	list_mag_str black_list;
}proxy_config_mag_str;
typedef struct{
	u8 fil_type;
	u16 list_size;
}mesh_filter_sts_str;

typedef struct{
	u8 sar_start;
	u8 sar_end;
	u8 err_flag ;
	u32 sar_tick ;
}mesh_proxy_protocol_sar_t;
extern mesh_proxy_protocol_sar_t  proxy_sar;

extern void proxy_cfg_list_init();
extern u8 proxy_config_dispatch(u8 *p,u8 len );
extern u8 proxy_filter_change_by_mesh_net(u16 unicast_adr);

extern u8 set_proxy_adv_pkt(u8 *p ,u8 flags,u8 *pHash,u8 *pRandom,mesh_net_key_t *p_netkey);

extern u8 find_data_in_mag_list(u16 src,proxy_config_mag_str *p_mag_list);
extern proxy_config_mag_str proxy_mag;
extern int is_valid_adv_with_proxy_filter(u16 src);
extern void proxy_cfg_list_init_upon_connection();
extern void set_proxy_initial_mode(u8 special_mode);
extern void caculate_proxy_adv_hash_task();
extern void set_pair_login_ok(u8 val);
extern u8 proxy_proc_filter_mesh_cmd(u16 src);

#endif 

