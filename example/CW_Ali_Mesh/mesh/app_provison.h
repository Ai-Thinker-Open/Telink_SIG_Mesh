/********************************************************************************************************
 * @file     app_provison.h 
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
#ifndef __APP_PROVISON_H_CLIPSE
#define __APP_PROVISON_H_CLIPSE
#include "mesh/light.h"
#include "mesh/mesh_node.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/sig_mesh/Test_case.h"
#define PROVISION_GATT_ENABLE 1

extern u8 blt_state;
#define PROVISION_ELE_ADR 	0x7F00
#define PROVISION_CMD_TIMEOUT_MS	30*1000*1000
typedef enum{
	L2CAP_PROVISON_TYPE =1,
	L2CAP_PROXY_TYPE,	
	L2CAP_BEACON_TYPE,
	L2CAP_NETWORK_TYPE,
}L2CAP_HANDLE_TYPE;
typedef enum{
	TRANS_START = 	0,
	TRANS_ACK ,
	TRANS_CON ,
	BEARS_CTL ,
}TRANS_BEARER_TYPE;
// provison ack reason 
typedef enum{
	LINK_CLOSE_SUCC =0,
	LINK_CLOSE_TIMEOUT,
	LINK_CLOSE_FAIL,
	LINK_CLOSE_RFU,
}TRANS_LINK_STATE;

typedef enum{
	LINK_OPEN =0,
	LINK_ACK,
	LINK_CLOSE,
	LINK_RFU,
}TRANS_LINK_OPERATION;

typedef struct {
	u8 GPCF:2;
	u8 BearCtl:6;
}trans_bear_header;
typedef struct {
	trans_bear_header header;
	u8 DeviceUuid[16];
}bear_open_link;
typedef struct {
	trans_bear_header header;
}bear_ack_link;
typedef struct {
	trans_bear_header header;
	u8 reason;
}bear_close_link;

typedef struct {
	u8 GPCF:2;
	u8 SegN:6;
	u16 total_len;
	u8 FCS;
	u8 data[1];
}trans_start_pdu;
typedef struct {
	u8 GPCF:2;
	u8 rfu:6;
}trans_ack_pdu;
typedef struct {
	u8 GPCF:2;
	u8 SegIdx:6;
	u8 data[1];
}trans_con_pdu;
typedef struct {
	union {
		bear_open_link  bearOpen;
		bear_ack_link	bearAck;
		bear_close_link bearCls;
	};
}trans_bear_pdu;
typedef struct {
	u8 length;
	u8 ad_type;
	u8 link_ID[4];
	u8 trans_num;
	union {
		u8 pb_pdu[0x50];
		trans_start_pdu transStart;
		trans_ack_pdu 	transAck;
		trans_con_pdu	transCon;
		trans_bear_pdu	transBear;
	};
}pro_PB_ADV;

typedef struct{
	u32 seg_map;
	u8 trans_num;
	u8 seg_n;
	u8 seg_cnt;
	u8 fcs_tmp;
	u32 link_id;
}mesh_prov_seg_t;

typedef struct {
	u8 flag_len;
	u8 flag_type;
	u8 flag_content;
	u8 uuid_len;
	u8 uuid_type;
	u8 uuid_pb_uuid[2];
	u8 service_len;
	u8 service_type;
	u8 service_uuid[2];
	u8 service_data[16];
	u8 oob_info[2];
}PB_GATT_ADV_DAT;

// provison pdu part 
typedef enum{
	PRO_INVITE = 0,
	PRO_CAPABLI	,
	PRO_START,
	PRO_PUB_KEY,
	PRO_INPUT_COM,
	PRO_CONFIRM,
	PRO_RANDOM,
	PRO_DATA,
	PRO_COMPLETE,
	PRO_FAIL,
	PRO_COMMAND_ACK,
	PRO_BEARS_CTL,
	PRO_BEACON,
	PRO_RFU,
}PRO_CMD_ENUM;
// SAR enum
typedef enum{
	SAR_COMPLETE =0,
	SAR_START,
	SAR_CONTINUS,
	SAR_END,
}PROXY_SAR_ENUM;
// Msg Type
typedef enum{
	MSG_NETWORK_PDU =0 ,
	MSG_MESH_BEACON,
	MSG_PROXY_CONFIG,
	MSG_PROVISION_PDU,
	MSG_RX_TEST_PDU,
	MSG_COMPOSITE_WAIT = 0xfe,
}PROXY_MSG_TYPE_ENUM;

// output act enum
typedef enum{
	MESH_OUT_ACT_BLINK = 0,
	MESH_OUT_ACT_BEEP,
	MESH_OUT_ACT_VIBRATE,
	MESH_OUT_ACT_NUMBERIC,
	MESH_OUT_ACT_ALPHA,
	MESH_OUT_ACT_RFU,
}MESH_OUT_ACT_ENUM;

// input act enum
typedef enum{
	MESH_IN_ACT_PUSH=0,
	MESH_IN_ACT_TWIST,
	MESH_IN_ACT_NUMBER,
	MESH_IN_ACT_ALPHA,
	MESH_IN_ACT_RFU,
}MESH_IN_ACT_ENUM;

typedef struct {
	u8 type:6;
	u8 rfu:2;
}pro_trans_header ;

typedef struct {
	pro_trans_header header;
	u8 attentionDura;
}pro_trans_invite;
//capa structure 
typedef struct {
	u16 alg_fips:1;
	u16 alg_rfu:15;
}capa_alg_str;
typedef struct {
	u8 oobKey_en:1;
	u8 oobRfu:7;
}pub_keyType_str;
typedef struct {
	u8 sta_oob_en:1;
	u8 sta_oob_rfu:7;
}static_OOB_Type_str;
typedef struct {
	u16 blink:1;
	u16 beep:1;
	u16 vibriate:1;
	u16 out_num:1;
	u16 out_alpha:1;
	u16 out_rfu:11;
}out_OOB_act_str;
typedef struct {
	u16 push:1;
	u16 Twist:1;
	u16 Input_num:1;
	u16 Input_alpha:1;
	u16 Input_rfu:11;
}in_OOB_act_str;
typedef struct {
	pro_trans_header header;
	u8 ele_num;
	union{
		u16 alogrithms;
		capa_alg_str alg_str;
		};
	union{
		u8 pubKeyType;
		pub_keyType_str keyType_str;
		};
	union{
		u8 staticOOBType;
		static_OOB_Type_str sta_oob;
		};
	u8 outPutOOBSize;
	union{
		u16 outPutOOBAct;
		out_OOB_act_str out_act_str;
		};
	u8 inOOBSize;
	union{
		u16 inOOBAct;
		in_OOB_act_str in_act_str;
		};
}pro_trans_capa;

typedef struct {
	pro_trans_header header;
	u8 alogrithms;
	u8 pubKey;
	u8 authMeth;
	u8 authAct;
	u8 authSize;
}pro_trans_start;

typedef struct {
	pro_trans_header header;
	u8 pubKeyX[32];
	u8 pubKeyY[32];
}pro_trans_pubkey;

typedef struct {
	pro_trans_header header;
}pro_trans_incomplete;
typedef struct {
	pro_trans_header header;
	u8 comfirm[16];
}pro_trans_comfirm;

typedef struct {
	pro_trans_header header;
	u8 random[16];
}pro_trans_random;
typedef struct {
	pro_trans_header header;
	u8 encProData[25];
	u8 proDataMic[8];	
}pro_trans_data;
typedef struct {
	pro_trans_header header;
}pro_trans_complete;
typedef struct {
	pro_trans_header header;
	u8 err_code;
}pro_trans_fail;

typedef enum{
	PRO_LINK_CLOSE_SUC =0,
	PRO_LINK_CLOSE_TIMEOUT =1,
	PRO_LINK_CLOSE_FAIL =2,
	PRO_LINK_CLOSE_UNRECONG =3,
}PRO_LINK_CLOSE_ENUM;

typedef  enum{
	RFU = 0,
	INVALID_PDU,
	INVALID_FORMAT,
	UNEXPECTED_PDU,
	COMFIRM_FAILED,
	OUT_OF_SOURCE,
	DECRYPTION_FAIL,
	UNEXPECT_ERROR,
	CANNOT_ASSGIN_ADDR,
}TRANS_FAIL_CODE_ENUM;
#define ELE_LIGHT_MODEL_SIZE  (380-12)	
 typedef struct{
	u8 nums;
	u8 numv;
	u8 id[ELE_LIGHT_MODEL_SIZE];
}mesh_element_model_id;

typedef struct{
    u16 len_cps;
    #if 1   // must same with page0_local_t from start to numv
    mesh_page0_head_t page0_head;
    u16 local;
    mesh_element_model_id model_id;
    #endif
}VC_node_cps_t;

#define MODEL_NOT_FOUND         (0xff)

#if DONGLE_PROVISION_EN
typedef struct{
    u16 node_adr;    // primary address
    u8 element_cnt;
    u8 rsv;
    u8 dev_key[16];
}VC_node_info_t;
#else
typedef struct{
    u16 node_adr;    // primary address
    u8 element_cnt;
    u8 rsv;
    u8 dev_key[16];
    VC_node_cps_t cps;
}VC_node_info_t;    // size is 404(0x194)
#endif
typedef struct{
	VC_node_info_t node_info[MESH_NODE_MAX_NUM];
	mesh_key_t mesh_key;
}sdk_info_t;

typedef struct {
	u8 type;
	u8 rfu;
	union {
		u8 pro_pdu_buff[0x50];
		pro_trans_invite 	invite;
		pro_trans_capa 		capa;
		pro_trans_start   	start;
		pro_trans_pubkey	pubkey;
		pro_trans_incomplete inComp;
		pro_trans_comfirm   comfirm;
		pro_trans_random 	random;
		pro_trans_data		data;
		pro_trans_complete	complete;
		pro_trans_fail      fail;
	};
}mesh_pro_pdu_content;

typedef struct {
	union{
		pro_trans_invite 	invite;
		pro_trans_capa 		capa;
		pro_trans_start   	start;
		pro_trans_pubkey	pubkey;
		pro_trans_incomplete inComp;
		pro_trans_comfirm   comfirm;
		pro_trans_random 	random;
		pro_trans_data		data;
		pro_trans_complete	complete;
		pro_trans_fail      fail;
		};
}mesh_pro_data_structer;

typedef enum{
	MESH_PUB_KEY_WITHOUT_OOB =0,
	MESH_PUB_KEY_WITH_OOB,
}OOB_PUB_KEY_TYPES_STR;
typedef enum{
	MESH_NO_OOB =0,
	MESH_STATIC_OOB,
	MESH_OUTPUT_OOB,
	MESH_INPUT_OOB,
}OOB_AUTH_KEYS_TYPES_STR;

typedef struct {
	pro_trans_capa 		capa;
	pro_trans_start   	start;
	u8 prov_key; 
	u8 oob_outsize;
	u8 oob_outAct;
	u8 oob_insize;
	u8 oob_inAct;
	u32 oob_out_tick;
}mesh_prov_oob_str;
extern mesh_prov_oob_str prov_oob;

#define EVENT_MESH_NODE_RC_LINK_CLOSE	0
#define EVENT_MESH_NODE_RC_LINK_START	1
#define EVENT_MESH_NODE_RC_LINK_FAIL_CODE 	2
#define EVENT_MESH_NODE_RC_LINK_TIMEOUT 	3
#define EVENT_MESH_NODE_RC_LINK_SUC			4

#define EVENT_MESH_PRO_RC_LINK_START		5
#define EVENT_MESH_PRO_RC_LINK_TIMEOUT 		6
#define EVENT_MESH_PRO_RC_LINK_SUC		7

#define EVENT_MESH_NODE_RC_CONFIRM_FAILED   0x10
#define EVENT_MESH_NODE_CONNECT             0x11
#define EVENT_MESH_NODE_DISCONNECT          0x12
typedef  enum{
	DEV_LINK_UNPRO_STATE =0,
	DEV_LINK_OPENED,
	DEV_LINK_COMPLETE,
}PROVISON_STATE;
#if TESTCASE_FLAG_ENABLE
	typedef struct{
		u8 wait2sendcls;

	}LOWER_para_testcase;
	typedef struct{
		u8 link_estab_hold;


	}IUT_para_tesecase;
#endif

typedef struct{
	u8  net_work_key[16];
	u16  key_index;
	union{
		mesh_ctl_fri_update_flag_t prov_flags;
		u8  flags;
	};
	u8  iv_index[4];
	u16  unicast_address;
}provison_net_info_str;

typedef struct{
	u16 apk_idx;
	u8 app_key[16];
}provision_appkey_t;


typedef struct{
    u32 link_id;
	u8 link_id_filter;
	u8 trans_num;
	u8 trans_num_last;
	u8 last_segmentIdx;
	u8 rcv_max_idx;
	u8 segmentIdx;
	u8 provison_send_state;
	u8 provison_rcv_state;
	u8 initial_pro_roles;
	u8 trans_start_fcs;
	u8 trans_seg_total_len;
	u8 pro_cmd;
    u8 pro_bearerCode;
	u8 pro_fail_code;
	u8 attention_value;
	u32 cmd_send_tick;
	u32 cmd_send_start_tick;
	u8  cmd_retry_flag;
	u8  ack_retry_flag;
	u8  link_close_flag;
	u8  link_close_cnt;
	u8  rsp_ack_transnum;
	u8  hash[8];
	u8  random[8];
	u8  oob_info[2];
	u8  device_uuid[16];
	u8 ele_cnt;
	u8 key_bind_lock;
}prov_para_proc_t;
extern prov_para_proc_t prov_para;

#if (DUAL_VENDOR_EN)
enum{
    DUAL_VENDOR_ST_STANDBY      = 0,    // wait for select
    DUAL_VENDOR_ST_MI           = 1,
    DUAL_VENDOR_ST_ALI          = 2,
};
#endif

typedef struct{
    u8  rfu[0x41];
    u8  dual_vendor_st;    // only use for dual vendor
	u8 	gatt_mode; 
	u8  pro_stop_flag;
	provison_net_info_str pro_net_info;
	u8  unused_oob[16];
	u16 unicast_adr_last;
}pro_para_mag;
extern u8 prov_link_cls_code;

extern u8 prov_link_uuid[16];
enum{
	GATT_ADV_NORMAL_MODE =0,
	GATT_PROVISION_MODE ,
	GATT_PROXY_MODE,
};

enum{
    STATE_DEV_UNPROV =0,
    STATE_DEV_PROVING   =1,
    STATE_DEV_PROVED =2,
};


#define MESH_KEYBIND_EVE_START 0
#define MESH_KEYBIND_EVE_SUC   1
#define MESH_KEYBIND_EVE_TIMEOUT 2

#define FIPS_P256_Ellipti_CURVE 	1
#define MESH_KR_CFGCL_RETRY_TIMES_ERR	2
#define MESH_APP_KEY_BIND_EVENT_SUC	1
#define MESH_APP_KEY_BIND_EVENT_TIMEOUT 0


enum{
	AUTH_MESH_NO_OOB=0,
	AUTH_MESH_STATIC_OOB,
	AUTH_MESH_OUTPUT_OOB,
	AUTH_MESH_INPUT_OOB ,
};

typedef struct{
	u8 msgType:6;
	u8 sar:2;
	u8 data[19];
}pb_gatt_proxy_str;
typedef enum{
	MESH_INI_ROLE_NODE=0,
	MESH_INI_ROLE_PROVISIONER ,
}provision_roles_define ;
typedef enum{
	LINK_UNPROVISION_STATE =0,
	LINK_ESTABLISHED,
	LINK_ESTABLISHED_ACK,
	LINK_CLOSE_STATE,
	STATE_PRO_INVITE,
	STATE_PRO_INVITE_ACK,
	STATE_DEV_CAPA,
	STATE_DEV_CAPA_ACK,
	STATE_PRO_START,
	STATE_PRO_START_ACK,
	STATE_PRO_PUB_KEY,
	STATE_PRO_PUB_KEY_ACK,
	STATE_DEV_PUB_KEY_INPUT_OOB,
	STATE_DEV_PUB_KEY_INPUT_OOB_ACK,
	STATE_DEV_PUB_KEY_OUTPUT_OOB,
	STATE_DEV_PUB_KEY,
	STATE_DEV_PUB_KEY_ACK,
	STATE_PRO_IN_COMPLETE,
	STATE_PRO_CONFIRM,
	STATE_PRO_CONFIRM_ACK,
	STATE_DEV_CONFIRM,
	STATE_DEV_CONFIRM_ACK,
	STATE_PRO_RANDOM,
	STATE_PRO_RANDOM_ACK,
	STATE_DEV_RANDOM,
	STATE_DEV_RANDOM_ACK,
	STATE_PRO_DATA,
	STATE_PRO_DATA_ACK,
	STATE_PRO_COMPLETE,
	STATE_PRO_COMPLETE_ACK,
	STATE_PRO_SUC,
	STATE_PRO_FAILED,
	STATE_PRO_FAILED_ACK,
}PROVISON_SEND_STATE_UNION;
typedef enum{
	PROVISION_FAIL_PROHIBITED =0,
	PROVISION_FAIL_INVALID_PDU =1,
	PROVISION_FAIL_INVALID_FORMAT =2,
	PROVISION_FAIL_UNEXPT_PDU =3,
	PROVISION_FAIL_COMFIRM_FAIL =4,
	PROVISION_FAIL_OUT_OF_RES =5,
	PROVISION_FAIL_DECRYT_FAIL =6,
	PROVISION_FAIL_UNEXPT_ERR =7,
	PROVISION_FAIL_CANNOT_ASSIGN_ADR =8,
	PROVISION_FAIL_INVALID_DATA    =9,// add for the device key refresh part 
	PROVISION_FAIL_RFU,
}PROVISION_FAIL_ENUM;

enum{
	FAST_PROV_IDLE,
	FAST_PROV_START,
	FAST_PROV_RESET_NETWORK,
	FAST_PROV_GET_ADDR,
	FAST_PROV_GET_ADDR_RETRY,
	FAST_PROV_SET_ADDR,
	FAST_PROV_NET_INFO,
	FAST_PROV_CONFIRM,
	FAST_PROV_CONFIRM_OK,
	FAST_PROV_COMPLETE,
	FAST_PROV_TIME_OUT,
};

typedef struct{
	u8 mac[6];
	u16 pid;	
	u16 default_addr;
}fast_prov_mac_st;

typedef struct{
	provison_net_info_str pro_data;
	mesh_appkey_set_t appkey_set;
}fast_prov_net_info_t;

typedef struct{
	u16 pid;
	u16 prov_addr;
	u8 get_mac_en;
	u8 not_need_prov;
	u16 rcv_op;
	u8 cur_sts;
	u8 last_sts;
	u8 pending;
	u16 delay;
	u32 start_tick;
	fast_prov_net_info_t net_info;
} fast_prov_par_t;

extern fast_prov_par_t fast_prov;

// DEFINATION OF THE PROVISON PART ///
#define MESH_PRO_MAX_CNT 8
#define MESH_PRO_MAX_LENG 31

#define MAX_RETRY_INTERVAL	1000*1000
extern VC_node_info_t VC_node_info[MESH_NODE_MAX_NUM];
extern pro_PB_ADV rcv_pb;
#define PROVISION_GATT_MAX_LEN      (sizeof(rcv_pb.pb_pdu)) // 0x50

extern mesh_cmd_bear_unseg_t		pro_adv_pkt;
extern mesh_pro_data_structer		pro_data_str;
extern pro_para_mag  provision_mag;
extern u8 para_pro[PROVISION_GATT_MAX_LEN];
extern u8 para_len ;
extern u8 prov_net_key[16];

extern u8 dev_auth[];
extern u8 dev_edch[32];
extern u8 dev_input[0x91];

u8 set_provision_networkkey_self(u8 *p_key,u8 len );
void set_provisionee_para(u8 *p_net_key,u16 key_index,u8 flags,u8 *p_ivi,u16 unicast);
u8 get_ele_offset_by_model_VC_node_info(u16 obj_adr, u32 model_id, int sig_model);
VC_node_info_t * get_VC_node_info(u16 obj_adr, int is_must_primary);
void erase_vc_node_info();

extern void set_pb_gatt_adv(u8 *p,u8 flags);
extern void set_adv_provision(rf_packet_adv_t * p);
extern void set_adv_proxy(rf_packet_adv_t * p);


void set_provision_stop_flag_act(u8 stop_flag);
extern void mesh_provision_para_init();
extern void mesh_provision_para_reset();

extern void send_rcv_retry_set(u8 cmd, u8 bearerCode, u8 ack_retry_flag);

extern u8 swap_mesh_pro_capa(mesh_pro_data_structer *p_str);
extern u8 set_pro_invite(mesh_pro_data_structer *p_str,u8 att_dura);
extern u8 set_pro_capa(mesh_pro_data_structer *p_str,u8 ele_num,u16 alogr,u8 keytype,
				u8 outOOBsize,u16 outAct,u8 inOOBsize,u16 inOOBact);
extern u8 set_pro_start_simple(mesh_pro_data_structer *p_str ,pro_trans_start *p_start);

extern u8 set_pro_start(mesh_pro_data_structer *p_str,u8 alogri,u8 pubkey,
					u8 authmeth,u8 authact,u8 authsize );
extern u8 set_pro_pub_key(mesh_pro_data_structer *p_str,u8 *p_pubkeyx,u8 *p_pubkeyy);
extern u8 set_pro_input_complete(mesh_pro_data_structer *p_str);
extern u8 set_pro_comfirm(mesh_pro_data_structer *p_str,u8 *p_comfirm);
extern u8 set_pro_random(mesh_pro_data_structer *p_str,u8 *p_random);
extern u8 set_pro_data(mesh_pro_data_structer *p_str, u8 *p_data,u8 *p_mic);
extern u8 set_pro_complete(mesh_pro_data_structer *p_str);
extern u8 set_pro_fail(mesh_pro_data_structer *p_str ,u8 fail_code);

extern int mesh_provision_rcv_process (u8 *p_payload, u32 t);

extern u8 mesh_send_provison_data(u8 pdu_type,u8 bearCode,u8 *para,u8 para_len );
extern int mesh_provison_process(u8 ini_role,u8 *p_rcv);
extern void mesh_adv_provision_retry();

extern void dispatch_pb_gatt(u8 *p ,u8 len );
extern u8 get_provision_state();
extern u8 is_provision_success();
extern u8 is_provision_working();

extern u8 tc_set_fifo(u8 cmd,u8 *pfifo,u8 cmd_len);
extern int notify_pkts(u8 *p,u16 len,u16 handle,u8 proxy_type);

extern u8 mesh_provision_cmd_process();
extern u8 pkt_pb_gatt_data(rf_packet_att_data_t *p, u8 l2cap_type,u8 *p_rcv,u8 *p_rcv_len);
extern void mesh_proxy_sar_timeout_terminate();
extern void provision_mag_cfg_s_store();
#if !WIN32
void get_private_key(u8 *p_private_key);
void get_public_key( u8 *p_public_key);
#else //just for compile,win32 use gatt_pro_psk and gatt_pro_ppk
#define get_private_key(a)		
#define get_public_key(a)	
#endif
void cal_private_and_public_key();
extern u8 dispatch_start_cmd_reliable(mesh_pro_data_structer *p_rcv_str);


extern void provision_timeout_cb();
extern u8 set_pro_dat_part(u16 ele_adr);
extern void provision_set_ivi_para(u8 *para);

extern int mesh_prov_sec_msg_enc(unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length);
extern int	mesh_prov_sec_msg_dec (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length);

extern void enable_mesh_adv_filter();
extern void disable_mesh_adv_filter();
extern void set_pro_init_role(u8 role);

extern void set_provisioner_para(u8 *p_net_key,u16 key_index,u8 flags,u8 *p_ivi,u16 unicast);

extern u8 mesh_prov_oob_auth_data(mesh_prov_oob_str *p_prov_oob);
extern void check_oob_out_timeout();
extern void send_comfirm_no_pubkey_cmd();
extern u8 adv_provision_state_dispatch(pro_PB_ADV * p_adv);
extern u8 wait_and_check_complete_state();
void VC_cmd_clear_all_node_info(u16 adr);

// proxy sar function part 
void mesh_proxy_sar_para_init();
void mesh_proxy_sar_start();
void mesh_proxy_sar_continue();
void mesh_proxy_sar_end();
void mesh_proxy_sar_complete();
void mesh_proxy_sar_err_terminate();
void mesh_proxy_sar_timeout_terminate();
void mesh_loop_check_link_close_flag();

void mesh_proxy_master_terminate_cmd();
void set_node_prov_capa_oob_init();
void set_node_prov_capa_oob(mesh_prov_oob_str *p_prov_oob,u8 pub_key_type,
					u8 staticOOBType,u16 outPutOOBAct,u16 inPutOOBAct);

void set_node_prov_para_no_pubkey_no_oob();
void set_node_prov_para_no_pubkey_static_oob();
void set_node_prov_para_no_pubkey_input_oob();
void set_node_prov_para_no_pubkey_output_oob();
void set_node_prov_para_pubkey_no_oob();
void set_node_prov_para_pubkey_static_oob();
void set_node_prov_para_pubkey_input_oob();
void set_node_prov_para_pubkey_output_oob();
void check_mesh_node_out_oob_pub_key_send_time();
u8  mesh_loop_provision_end_process();
void mesh_prov_link_close_terminate();
void prov_set_link_close_code(u8 code);

extern int App_key_bind_end_callback(u8 event);
u8 mesh_cfg_keybind_end_event(u8 eve,u16 unicast);

void mesh_set_oob_type(u8 type, u8 *p_oob ,u8 len );
void mesh_set_pro_auth(u8 *p_auth, u8 len);
void mesh_set_dev_auth(u8 *p_auth, u8 len);
void set_node_prov_start_oob(mesh_pro_data_structer *p_rcv,mesh_prov_oob_str *p_oob);

u8 get_pubkey_oob_info_by_capa(mesh_prov_oob_str *p_prov_oob);
u8 set_start_para_by_capa(mesh_prov_oob_str *p_prov_oob);
u32 mesh_check_pubkey_valid(u8 *rcv_ppk);
void pro_random_init();
u8 win32_create_rand();
void dev_random_init();
void provision_random_data_init();
int upload_sig_mesh_para(u8 *buf,int *len);
int download_sig_mesh_para(u8 *buf,int len );
void reset_uuid_create_flag();
void check_inputoob_proc();

//gateway 
void set_gateway_adv_filter(u8 *p_mac);
void gateway_adv_filter_init();
void mesh_cfg_keybind_start_trigger_event(u8* p_idx,u8 *p_key,u16 unicast,u16 nk_idx,u8 fastbind);
int VC_node_dev_key_save(u16 adr, u8 *dev_key,u8 ele_cnt);
VC_node_info_t * get_VC_node_info(u16 obj_adr, int is_must_primary);
void mesh_prov_proc_loop();
void set_prov_timeout_tick(u32 tick);
void send_rcv_retry_clr();
u8 send_multi_type_data(u8 type,u8 *p_para);
int gatt_prov_notify_pkts(u8 *p,u16 len,u16 handle,u8 proxy_type);
void mesh_adv_prov_comfirm_cmd(mesh_pro_data_structer *p_send_str,u8 *p_comfirm);
void mesh_adv_prov_link_close();
void mesh_adv_prov_link_open(u8 *p_uuid);
void mesh_adv_prov_link_open_ack(pro_PB_ADV *p_adv);
void mesh_adv_prov_send_invite(mesh_pro_data_structer *p_send_str);
void mesh_adv_prov_invite_ack(pro_PB_ADV *p_adv);
void mesh_adv_prov_capa_rcv(pro_PB_ADV *p_adv);
void mesh_adv_prov_send_start_cmd(mesh_pro_data_structer *p_send_str,pro_trans_start *p_start);
void mesh_adv_prov_start_ack(pro_PB_ADV *p_adv);
void mesh_adv_prov_pubkey_send(mesh_pro_data_structer *p_send_str,u8 *ppk);
void mesh_adv_prov_pubkey_rsp(pro_PB_ADV *p_adv);
void mesh_adv_prov_comfirm_cmd(mesh_pro_data_structer *p_send_str,u8 *p_comfirm);
void mesh_adv_prov_comfirm_ack(pro_PB_ADV *p_adv);
void mesh_adv_prov_comfirm_rsp(pro_PB_ADV *p_adv);
void mesh_adv_prov_random_cmd(mesh_pro_data_structer *p_send_str,u8 *p_random);
void mesh_adv_prov_random_ack_cmd(pro_PB_ADV *p_adv);
void mesh_adv_prov_random_rsp(pro_PB_ADV *p_adv);
void mesh_adv_prov_data_send(mesh_pro_data_structer *p_send_str,u8 *p_data);
void mesh_adv_prov_data_ack(pro_PB_ADV *p_adv);
void mesh_adv_prov_complete_rsp(pro_PB_ADV *p_adv);
u8 get_mesh_pro_str_len(u8 type);
void mesh_prov_end_set_tick();
void mesh_rp_server_set_link_sts(u8 sts);
void del_vc_node_info_by_unicast(u16 unicast);
u8 win32_proved_state();
void mesh_rp_adv_prov_complete_rsp(pro_PB_ADV *p_adv);

#endif 

