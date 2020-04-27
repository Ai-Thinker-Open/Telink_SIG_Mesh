/********************************************************************************************************
 * @file     mesh_ota.h 
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

#include "proj/tl_common.h"
#include "proj_lib/sig_mesh/app_mesh.h"

//----------------------------------- op code
// op cmd 0xxxxxxx (SIG)

// op cmd 10xxxxxx xxxxxxxx (SIG)
#define FW_INFO_GET		        		0x01B6
#define FW_INFO_STATUS		        	0x02B6
#define FW_UPDATE_GET		        	0x03B6
#define FW_UPDATE_PREPARE		        0x04B6
#define FW_UPDATE_START		        	0x05B6
#define FW_UPDATE_ABORT		        	0x06B6
#define FW_UPDATE_APPLY		        	0x07B6
#define FW_UPDATE_STATUS		        0x08B6
#define FW_DISTRIBUT_GET		        0x09B6
#define FW_DISTRIBUT_START		        0x0AB6
#define FW_DISTRIBUT_STOP		        0x0BB6
#define FW_DISTRIBUT_STATUS		        0x0CB6
#define FW_DISTRIBUT_DETAIL_GET		    0x0DB6
#define FW_DISTRIBUT_DETAIL_LIST		0x0EB6

#define OBJ_TRANSFER_GET		        0x01B7
#define OBJ_TRANSFER_START		        0x02B7
#define OBJ_TRANSFER_ABORT		        0x03B7
#define OBJ_TRANSFER_STATUS		        0x04B7
#define OBJ_BLOCK_TRANSFER_START		0x05B7
#define OBJ_BLOCK_TRANSFER_STATUS		0x06B7
#define OBJ_CHUNK_TRANSFER		        0x7D
#define OBJ_BLOCK_GET		            0x7E
#define OBJ_BLOCK_STATUS		        0x09B7
#define OBJ_INFO_GET		            0x0AB7
#define OBJ_INFO_STATUS		            0x0BB7

//------op parameters
#if WIN32
#define MESH_OTA_UPDATE_NODE_MAX        (180)   // max: (380 - head)/2
#else
#define MESH_OTA_UPDATE_NODE_MAX        (64)     // set small just for save RAM
#endif

#if 0 // WIN32 test
#define MESH_OTA_CHUNK_SIZE             (16)   // because fifo size is not enough now: must less than (72 - 11)
#define MESH_OTA_BLOCK_SIZE_LOG_MIN     (8)
#define MESH_OTA_BLOCK_SIZE_LOG_MAX     (8)
#else
#define MESH_OTA_CHUNK_SIZE             (256)   // must 256
#define MESH_OTA_BLOCK_SIZE_LOG_MIN     (12)    // for 4k erase
#define MESH_OTA_BLOCK_SIZE_LOG_MAX     (12)    // for 4k erase
#endif
#define MESH_OTA_BLOCK_SIZE_MIN         (1 << MESH_OTA_BLOCK_SIZE_LOG_MIN)
#define MESH_OTA_BLOCK_SIZE_MAX         (1 << MESH_OTA_BLOCK_SIZE_LOG_MAX)
#define MESH_OTA_CHUNK_NUM_MAX          ((MESH_OTA_BLOCK_SIZE_MAX + MESH_OTA_CHUNK_SIZE - 1)/MESH_OTA_CHUNK_SIZE)
// #define MESH_OTA_CHUNK_MISS_MAX         (4)     // for unsegment // TODO
#define MESH_OTA_OBJ_SIZE_MAX           (FW_SIZE_MAX_K * 1024)

#define MESH_OTA_BLOCK_MAX  ((MESH_OTA_OBJ_SIZE_MAX + (MESH_OTA_BLOCK_SIZE_MIN - 1)) / MESH_OTA_BLOCK_SIZE_MIN)

#if MD_MESH_OTA_EN
    #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
#define FW_ADD_BYTE_EN      0
    #else
#define FW_ADD_BYTE_EN      0
    #endif
#endif
enum{
    OTA_CHECK_TYPE_NONE             = 0,
    OTA_CHECK_TYPE_TELINK_MESH      = 1,
};

enum{
	DISTRIBUT_ST_NOT_ACTIVE         = 0,
	DISTRIBUT_ST_ACTIVE             = 1,
	DISTRIBUT_ST_NO_SUCH_ID         = 2,
	DISTRIBUT_ST_BUSY_WITH_DIFF     = 3,
	DISTRIBUT_ST_UPDATE_NODE_LIST_TOO_LONG  = 4,
	DISTRIBUT_ST_MAX,
};

enum{
	UPDATE_NODE_ST_SUCCESS          = 0,
	UPDATE_NODE_ST_IN_PROGRESS      = 1,
	UPDATE_NODE_ST_CANCEL           = 2,
};

typedef struct{
	u16 cid;
	u32 fw_id;
}fw_cid_fwid_t;

typedef struct{
	fw_cid_fwid_t id;
}fw_info_status_t;

typedef struct{
	fw_cid_fwid_t id;
}fw_distribut_get_t;

typedef struct{
	fw_cid_fwid_t id;
	u16 adr_group;
	u16 update_list[MESH_OTA_UPDATE_NODE_MAX];
}fw_distribut_start_t;

typedef struct{
	fw_cid_fwid_t id;
}fw_distribut_stop_t;

typedef struct{
    u8 st;
	fw_cid_fwid_t id;
}fw_distribut_status_t;

typedef struct{
	fw_cid_fwid_t id;
}fw_distribut_detail_get_t;

typedef struct{
	u16 adr;
	u8 st;
}fw_distribut_node_t;

typedef struct{
	fw_distribut_node_t node[MESH_OTA_UPDATE_NODE_MAX];
}fw_distribut_detail_list_t;

enum{
	UPDATE_PHASE_IDLE               = 0,    // no DFU update in progress
	UPDATE_PHASE_PREPARE            = 1,
	UPDATE_PHASE_IN_PROGRESS        = 2,
	UPDATE_PHASE_DUF_READY          = 3,
	UPDATE_PHASE_APPLY_OK           = 7,    // just use internal
	UPDATE_PHASE_MAX,
};

enum{
	ADDITIONAL_PROV_NEED            = 0,
	ADDITIONAL_MAX,
};

enum{
	UPDATE_POLICY_NONE              = 0,
	UPDATE_POLICY_AUTO_UPDATE       = 1,
	UPDATE_POLICY_MAX,
};

enum{
	UPDATE_ST_SUCCESS                       = 0,
	UPDATE_ST_ID_COMBINATION_WRONG          = 1,
	UPDATE_ST_BUSY_WITH_DIFF_OBJ            = 2,
	UPDATE_ST_ID_COMBINATION_APPLY_FAIL     = 3,
	UPDATE_ST_ID_COMBINATION_ALWAYS_REJECT  = 4,    // newer firmware version present
	UPDATE_ST_ID_COMBINATION_TEMP_REJECT    = 5,    // node is not able to accept new firmware now, try again later
	UPDATE_ST_MAX,
};

typedef struct{
	fw_cid_fwid_t id;
}fw_update_get_t;

typedef struct{
	fw_cid_fwid_t id;
	u8 obj_id[8];
}fw_update_prepare_t;

typedef struct{
	u8 policy;
	fw_cid_fwid_t id;
}fw_update_start_t;

typedef struct{
	fw_cid_fwid_t id;
}fw_update_control_t;

typedef struct{
    u8 st;
    u8 phase            :3;
    u8 additional_info  :5;
	fw_cid_fwid_t id;
	u8 obj_id[8];
}fw_update_status_t;


enum{
	OBJ_TRANS_ST_READY                      = 0,
	OBJ_TRANS_ST_BUSY                       = 1,
	OBJ_TRANS_ST_BUSY_WITH_DIFF             = 2,
	OBJ_TRANS_ST_TOO_BIG                    = 3,
	OBJ_TRANS_ST_MAX
};

typedef struct{
	u8 obj_id[8];
}obj_transfer_get_t;

typedef struct{
	u8 obj_id[8];
	u32 obj_size;
	u8 bk_size_log;
}obj_transfer_start_t;

typedef struct{
	u8 st;
	u8 obj_id[8];
	u32 obj_size;
	u8 bk_size_log;
}obj_transfer_status_t;

enum{
	OBJ_BLOCK_TRANS_ST_ACCEPTED             = 0,
	OBJ_BLOCK_TRANS_ST_ALREADY_RX           = 1,
	OBJ_BLOCK_TRANS_ST_INVALID_BK_NUM       = 2,        // invalid block number, no previous block
	OBJ_BLOCK_TRANS_ST_WRONG_BK_SIZE_CURRENT    = 3,    // bigger then Block Size Log [Object Transfer Start]
	OBJ_BLOCK_TRANS_ST_WRONG_CHUNK_SIZE     = 4,        // bigger then Block Size divided by Max Chunks Number [Object Information Status]
	OBJ_BLOCK_TRANS_ST_UNKNOWN_CHECK_SUM_TYPE   = 5,
	OBJ_BLOCK_TRANS_ST_REJECT               = 0x0f,
	OBJ_BLOCK_TRANS_ST_MAX
};

enum{
	OBJ_BLOCK_CHECK_SUM_TYPE_CRC32          = 0,
	OBJ_BLOCK_CHECK_SUM_TYPE_MAX
};

typedef struct{
	u16 block_num;
	u16 chunk_size;
	u8 bk_check_sum_type;
	u32 bk_check_sum_val;
	u16 bk_size_current;    // optional
}block_transfer_start_par_t;

typedef struct{
	u8 obj_id[8];
	block_transfer_start_par_t par;
}obj_block_transfer_start_t;

typedef struct{
	u8 st;
}obj_block_transfer_status_t;

typedef struct{
	u16 chunk_num;
	u8 data[256];
}obj_chunk_transfer_t;

enum{
	OBJ_BLOCK_ST_ALL_CHUNK_RX               = 0,
	OBJ_BLOCK_ST_NOT_ALL_CHUNK_RX           = 1,
	OBJ_BLOCK_ST_WRONG_CHECK_SUM            = 2,
	OBJ_BLOCK_ST_WRONG_OBJ_ID               = 3,
	OBJ_BLOCK_ST_WRONG_BLOCK                = 4,
	OBJ_BLOCK_ST_MAX
};

typedef struct{
	u8 obj_id[8];
	u16 block_num;
}obj_block_get_t;

typedef struct{
	u8 st;
	u16 miss_chunk[MESH_OTA_CHUNK_NUM_MAX];
}obj_block_status_t;

typedef struct{
	u8 bk_size_log_min;
	u8 bk_size_log_max;
	u16 chunk_num_max;
}obj_info_status_t;


//--- proc
#define OTA_WAIT_ACK_MASK                (BIT(7))

enum{
	MASTER_OTA_ST_IDLE                      = 0,
	MASTER_OTA_ST_DISTRIBUT_START,
	MASTER_OTA_ST_SUBSCRIPTION_SET,
	MASTER_OTA_ST_FW_INFO_GET,
	MASTER_OTA_ST_OBJ_INFO_GET,
	MASTER_OTA_ST_UPDATE_PREPARE,
	MASTER_OTA_ST_UPDATE_START,                 // 10
	MASTER_OTA_ST_OBJ_TRANSFER_START,
	MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START,
	MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START_CHECK_RESULT,
	MASTER_OTA_ST_OBJ_CHUNK_START,
	MASTER_OTA_ST_OBJ_BLOCK_GET,
	MASTER_OTA_ST_UPDATE_GET,                   // 20
	MASTER_OTA_ST_UPDATE_APPLY,
	MASTER_OTA_ST_DISTRIBUT_STOP,
	MASTER_OTA_ST_MAX,
};

typedef struct{
	u16 adr;
	u8 st_block_trans_start;
	u8 st_block_get;
    u8 phase            :3;
    u8 additional_info  :5;
    u8 skip_flag        :1;
    u8 apply_flag       :1;
    u8 rsv              :6;
}fw_detail_list_t;

typedef struct{
	fw_cid_fwid_t id;
	u8 obj_id[8];
	u16 adr_group;
	fw_detail_list_t list[MESH_OTA_UPDATE_NODE_MAX];
	u32 miss_mask;
	block_transfer_start_par_t block_start_par;
	u8 bk_size_log;  // for 4 byte align
	u16 node_cnt;    // update node cnt
	u16 node_num;    // for check all
	u32 obj_size;
	u16 chunk_num;
	u16 adr_distr_node;
	u8 percent_last;    // progress
	u8 policy;
	u8 pause_flag;  // pause mesh ota tx flow when GATT disconnect, untill APP resume.
	u8 st;
}fw_distribut_srv_proc_t;

typedef struct{
	fw_cid_fwid_t id;
	u8 obj_id[8];
	u16 obj_block_trans_num_next;       // for 4 byte align
	block_transfer_start_par_t block_start_par;
	u8 obj_block_trans_accepted;        // for 4 byte align
	u32 miss_mask;
	u32 obj_size;
	u32 crc_total;
	u8 blk_crc_tlk_mask[(MESH_OTA_BLOCK_MAX + 7)/8];
	u8 obj_trans_busy;
	u8 bk_size_log;
	u8 policy;
    u8 phase            :3;
    u8 additional_info  :5;
    u8 bin_crc_done;
    u8 reboot_flag_backup;
	u8 busy;
}fw_update_srv_proc_t;

typedef struct{
    
}obj_trans_srv_proc_t;

// -----------
int mesh_cmd_sig_fw_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#if DISTRIBUTOR_UPDATE_CLIENT_EN
int mesh_cmd_sig_fw_distribut_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_stop(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_detail_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_detail_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_fw_distribut_get               0
#define mesh_cmd_sig_fw_distribut_start             0
#define mesh_cmd_sig_fw_distribut_stop              0
#define mesh_cmd_sig_fw_distribut_status            0
#define mesh_cmd_sig_fw_distribut_detail_get        0
#define mesh_cmd_sig_fw_distribut_detail_list       0
#endif
int mesh_cmd_sig_fw_update_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_prepare(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_control(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_transfer_handle(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_transfer_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_block_transfer_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_block_transfer_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_chunk_transfer(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_block_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_obj_block_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

// ---
int access_cmd_fw_update_control(u16 adr_dst, u16 op, u8 rsp_max);

// ---
int mesh_ota_master_rx (mesh_rc_rsp_t *rsp, u16 op, u32 size_op);
u32 new_fw_read(u8 *data_out, u32 max_len);
void new_fw_write_file(u8 *data_in, u32 len);
void mesh_ota_master_proc();
void get_fw_id();
u32 get_fw_len();
void mesh_ota_master_ack_timeout_handle();
u8 get_ota_check_type();
int is_valid_ota_check_type1();
u32 get_blk_crc_tlk_type1(u8 *data, u32 len, u32 addr);
int ota_file_check();
void APP_RefreshProgressBar(u16 bk_current, u16 bk_total, u16 chunk_cur, u16 chunk_total, u8 percent);
void APP_report_mesh_ota_apply_status(u16 adr_src, fw_update_status_t *p);
u16 APP_get_GATT_connect_addr();
void APP_set_mesh_ota_pause_flag(u8 val);
int is_mesh_ota_and_only_VC_update();
u32 soft_crc32_ota_flash(u32 addr, u32 len, u32 crc_init,u32 *out_crc_type1_blk);

