/********************************************************************************************************
 * @file     app_beacon.h 
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
#ifndef APP_BEACON_H
#define APP_BEACON_H
#include "mesh/light.h"
#include "proj_lib/sig_mesh/app_mesh.h"

#if WIN32
#pragma pack(1)
#endif

typedef struct {
	u8 len;
	u8 type;
}mesh_beacon_header;
typedef struct {
	mesh_beacon_header header;
	u8 beacon_type;
	u8 case_id[4];
	u8 para[4];
}beacon_testcase_pk;
typedef struct {
	mesh_beacon_header header;
	u8 beacon_type;
	u8 device_uuid[16];
	u8 oob_info[2];
}beacon_dat_without_uri_pk;
typedef struct {
	mesh_beacon_header header;
	u8 beacon_type;
	u8 device_uuid[16];
	u8 oob_info[2];
	u8 uri_hash[4];
}beacon_data_pk;
typedef struct {
	mesh_beacon_header header;
	u8 beacon_type;
	u8 flags;
	u8 networkId[8];
	u8 iviIndex[4];
	u8 authValue[8];
}secure_net_pk;
typedef struct {
	u8 trans_par_val;
	union{
		beacon_dat_without_uri_pk bea_out_uri;
		beacon_data_pk bea_data;
		secure_net_pk  sur_net;
		beacon_testcase_pk bea_testcase_id;
	};
}beacon_str;
typedef struct{
	u32 start_time_s;
	u32  tick;
	u32  inter;
	u8  conn_beacon_flag;
	u8 	en;
}beacon_send_str;
#define MESH_UNPROVISION_BEACON_WITHOUT_URI	0
#define MESH_UNPROVISION_BEACON_WITH_URI	1



typedef enum{
	UNPROVISION_BEACON = 0,
	SECURE_BEACON ,

	BEACON_TESTCASE=0x80,
	BEACON_RFU,
}BEACON_ADV_TYPE;
typedef enum{
	OOB_OTHERS = 0,
	OOB_URI	,
	OOB_READ_CODE,
	OOB_BAR_CODE,
	OOB_NFC,
	OOB_NUM,
	OOB_STRING,
	OOB_RSV1,
	OOB_RSV2,
	OOB_RSV3,
	OOB_RSV4,
	OOB_ON_BOX,
	OOB_IN_BOX,
	OOB_ON_PAPER,
	OOB_IN_MANUL,
	OOB_ON_DEVICE,
}OOB_INFO_FIELD;
extern beacon_str  beaconData;

u8  beacon_data_init_without_uri(beacon_str *p_str ,u8 *p_uuid,u8 *p_info);
u8  beacon_data_init_uri(beacon_str *p_str ,u8 *p_uuid,u8 *p_info,u8 *p_hash);
u8  beacon_secure_init(beacon_str *p_str,u8 flags,u8 * p_netId,u8* p_ivIdx,u8* p_auth);
u8 unprov_beacon_send(u8 mode ,u8 blt_sts);
u8 beacon_test_case(u8*p_tc,u8 len );
void mesh_tx_sec_nw_beacon(mesh_net_key_t *p_nk_base, u8 blt_sts);
void mesh_tx_sec_nw_beacon_all_net(u8 blt_sts);

u8 mesh_beacon_send_proc();
void beacon_str_init();
int check_pkt_is_unprovision_beacon(u8 *dat);
extern beacon_send_str beacon_send;


#endif 

