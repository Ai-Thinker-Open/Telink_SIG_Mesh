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

// ------
#ifndef CEIL_DIV
#define CEIL_DIV(A, B)                  (((A) + (B) - 1) / (B))
#endif
#ifndef CHAR_BIT
#define CHAR_BIT                        8
#endif

#define CEIL_8(val)                     CEIL_DIV(val, 8)
#define GET_BLOCK_SIZE(log)             (1 << log)
// ------
#define BLOB_TRANSFER_WITHOUT_FW_UPDATE_EN  (PTS_TEST_OTA_EN || 1)

#define MESH_OTA_R04                    1
#define MESH_OTA_R06                    2

#define MESH_OTA_V_SEL                  (MESH_OTA_R06)

#if(PTS_TEST_OTA_EN)
#define SIG_MD_FW_UPDATE_S              0xBF44
#define SIG_MD_FW_UPDATE_C              0xBF45
#define SIG_MD_FW_DISTRIBUT_S           0xBF46
#define SIG_MD_FW_DISTRIBUT_C           0xBF47
#define SIG_MD_BLOB_TRANSFER_S        	0xBF42
#define SIG_MD_BLOB_TRANSFER_C         	0xBF43

//----------------------------------- op code
// op cmd 0xxxxxxx (SIG)
// op cmd 10xxxxxx xxxxxxxx (SIG)

#define FW_UPDATE_INFO_GET		        0x1BB7
#define FW_UPDATE_INFO_STATUS		    0x7C
#define FW_UPDATE_METADATA_CHECK	    0x7B
#define FW_UPDATE_METADATA_CHECK_STATUS 0x7A
#define FW_UPDATE_GET		        	0x1CB7
#define FW_UPDATE_START		        	0x1DB7
#define FW_UPDATE_CANCEL		        0x1EB7
#define FW_UPDATE_APPLY		        	0x1FB7
#define FW_UPDATE_STATUS		        0x70
#define FW_DISTRIBUT_GET		        0x0AB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_START		        0x0BB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_CANCEL		        0x0CB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_APPLY		        0x0DB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_STATUS		        0x0EB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_DETAIL_GET		    0x0FB6  // To be update, because INI not update now.
#define FW_DISTRIBUT_DETAIL_LIST		0x10B6  // To be update, because INI not update now.

#define BLOB_TRANSFER_GET		        0x01B7
#define BLOB_TRANSFER_START		        0x02B7
#define BLOB_TRANSFER_CANCEL		    0x03B7
#define BLOB_TRANSFER_STATUS		    0x04B7
#define BLOB_BLOCK_GET		            0x07B7
#define BLOB_BLOCK_START		        0x05B7
#define BLOB_BLOCK_STATUS		        0x7E
#define BLOB_PARTIAL_BLOCK_REPORT	    0x7F    // used for pull mode(LPN)
#define BLOB_CHUNK_TRANSFER		        0x7D
#define BLOB_INFO_GET		            0x0AB7
#define BLOB_INFO_STATUS		        0x0BB7
#elif DRAFT_FEAT_VD_MD_EN
#include "draft_feature_vendor.h"
#else
#define SIG_MD_FW_UPDATE_S              0xFE00
#define SIG_MD_FW_UPDATE_C              0xFE01
#define SIG_MD_FW_DISTRIBUT_S           0xFE02
#define SIG_MD_FW_DISTRIBUT_C           0xFE03
#define SIG_MD_BLOB_TRANSFER_S        	0xFF00
#define SIG_MD_BLOB_TRANSFER_C         	0xFF01

//----------------------------------- op code
// op cmd 0xxxxxxx (SIG)

// op cmd 10xxxxxx xxxxxxxx (SIG)
#define FW_UPDATE_INFO_GET		        0x01B6
#define FW_UPDATE_INFO_STATUS		    0x02B6
#define FW_UPDATE_METADATA_CHECK	    0x03B6
#define FW_UPDATE_METADATA_CHECK_STATUS 0x04B6
#define FW_UPDATE_GET		        	0x05B6
#define FW_UPDATE_START		        	0x06B6
#define FW_UPDATE_CANCEL		        0x07B6
#define FW_UPDATE_APPLY		        	0x08B6
#define FW_UPDATE_STATUS		        0x09B6
#define FW_DISTRIBUT_GET		        0x0AB6
#define FW_DISTRIBUT_START		        0x0BB6
#define FW_DISTRIBUT_CANCEL		        0x0CB6
#define FW_DISTRIBUT_APPLY		        0x0DB6
#define FW_DISTRIBUT_STATUS		        0x0EB6
#define FW_DISTRIBUT_DETAIL_GET		    0x0FB6
#define FW_DISTRIBUT_DETAIL_LIST		0x10B6

#define BLOB_TRANSFER_GET		        0x01B7
#define BLOB_TRANSFER_START		        0x02B7
#define BLOB_TRANSFER_CANCEL		    0x03B7
#define BLOB_TRANSFER_STATUS		    0x04B7
#define BLOB_BLOCK_GET		            0x07B7
#define BLOB_BLOCK_START		        0x05B7
#define BLOB_BLOCK_STATUS		        0x7E
#define BLOB_PARTIAL_BLOCK_REPORT	    0x7F    // used for pull mode(LPN)
#define BLOB_CHUNK_TRANSFER		        0x7D
#define BLOB_INFO_GET		            0x0AB7
#define BLOB_INFO_STATUS		        0x0BB7
#endif

//------op parameters
#if WIN32
#define MESH_OTA_UPDATE_NODE_MAX        (MESH_NODE_MAX_NUM) // max: (380 - head)/2
#else
    #if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
#define MESH_OTA_UPDATE_NODE_MAX        (MESH_ELE_MAX_NUM)  // set small just for save RAM
    #else
#define MESH_OTA_UPDATE_NODE_MAX        (MESH_NODE_MAX_NUM)
    #endif
#endif

#if 0 // WIN32 test
#define MESH_OTA_CHUNK_SIZE             (16)   // because fifo size is not enough now: must less than (72 - 11)
#define MESH_OTA_BLOCK_SIZE_LOG_MIN     (8)
#define MESH_OTA_BLOCK_SIZE_LOG_MAX     (8)
#else
#define MESH_OTA_CHUNK_SIZE             (208)   // =(((CONST_DELTA_EXTEND_AND_NORMAL + 11 - 3)/16)*16) // need 16 align 
#define MESH_OTA_BLOCK_SIZE_LOG_MIN     (18)    // for 4k erase // in order to check missing only once
#define MESH_OTA_BLOCK_SIZE_LOG_MAX     (18)    // for 4k erase // in order to check missing only once
#endif
#define MESH_OTA_BLOCK_SIZE_MIN         (1 << MESH_OTA_BLOCK_SIZE_LOG_MIN)
#define MESH_OTA_BLOCK_SIZE_MAX         (1 << MESH_OTA_BLOCK_SIZE_LOG_MAX)
#define MESH_OTA_CHUNK_SIZE_MAX         (MESH_OTA_CHUNK_SIZE)
#define MESH_OTA_CHUNK_NUM_MAX          CEIL_DIV(MESH_OTA_BLOCK_SIZE_MAX, MESH_OTA_CHUNK_SIZE)
#define MESH_OTA_CHUNK_NUM_MAX_CEIL     (CEIL_8(MESH_OTA_CHUNK_NUM_MAX))
// #define MESH_OTA_CHUNK_MISS_MAX         (4)     // for unsegment // TODO
#if PTS_TEST_OTA_EN
#define MESH_OTA_BLOB_SIZE_MAX        	(5*1024)
#else
#define MESH_OTA_BLOB_SIZE_MAX        	(FW_SIZE_MAX_K * 1024)
#endif

#define MESH_OTA_BLOCK_MAX 	            CEIL_DIV(MESH_OTA_BLOB_SIZE_MAX, MESH_OTA_BLOCK_SIZE_MIN)

#if PTS_TEST_OTA_EN
#define MESH_OTA_BLOB_START_TIMEOUT_MS  (8 * 1000) // PTS MMDL/SR/BT/BV-04-C check 10s ?
#endif


#if MD_MESH_OTA_EN
    #if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE ||MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)
#define FW_ADD_BYTE_EN      0
    #else
#define FW_ADD_BYTE_EN      0
    #endif
#endif

/*distribute model status code*/
enum{
	DISTRIBUT_ST_SUCCESS                = 0,
	DISTRIBUT_ST_OUTOF_RESOURCE         = 1,
	DISTRIBUT_ST_INVALID_APPKEY_IDX     = 2,
	DISTRIBUT_ST_NODE_LIST_EMPTY        = 3,
	DISTRIBUT_ST_INVALID_PHASE          = 4,
	DISTRIBUT_ST_FW_NOT_FOUND           = 5,
	DISTRIBUT_ST_BUSY_WITH_TRANSFER     = 6,
	DISTRIBUT_ST_URI_NOT_SUPPORT        = 7,
	DISTRIBUT_ST_URI_MALFORMED          = 8,
	DISTRIBUT_ST_DISTRIBUTOR_BUSY       = 9,
	DISTRIBUT_ST_INTERNAL_ERROR         = 0x0A,
	DISTRIBUT_ST_MAX,
};
/*distribute model status code end*/

enum{
	UPDATE_NODE_ST_SUCCESS          = 0,
	UPDATE_NODE_ST_IN_PROGRESS      = 1,
	UPDATE_NODE_ST_CANCEL           = 2,
};

typedef struct{
	u16 pid;
	u16 vid;
}fw_id_t;

typedef struct{
	u16 adr_group;
	u16 update_list[MESH_OTA_UPDATE_NODE_MAX];
}fw_distribut_start_t;

typedef struct{
    u8 st;
}fw_distribut_status_t;

typedef struct{
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
    UPDATE_PHASE_TRANSFER_ERR       = 1,
    UPDATE_PHASE_TRANSFER_ACTIVE    = 2,
    UPDATE_PHASE_VERIFYING_UPDATE   = 3,	// DUF_READY
    UPDATE_PHASE_VERIFYING_SUCCESS  = 4,
    UPDATE_PHASE_VERIFYING_FAIL     = 5,
    UPDATE_PHASE_APPLYING_UPDATE    = 6,    // means device is applying new firmware, and will reboot later
    UPDATE_PHASE_MAX,
};

enum{
    ADDITIONAL_CPS_NO_CHANGE                    = 0, // CPS: composition data
    ADDITIONAL_CPS_CHANGE_NO_REMOTE_PROVISIOIN  = 1,
    ADDITIONAL_CPS_CHANGE_REMOTE_PROVISIOIN     = 2,
    ADDITIONAL_NODE_UNPROVISIONED               = 3, // The node is unprovisioned after successful application of a verified firmware image.
    ADDITIONAL_MAX,
};

/*firmware update model status code*/
enum{
	UPDATE_ST_SUCCESS               = 0,
	UPDATE_ST_INSUFFICIENT_RESOURCE = 1,
	UPDATE_ST_WRONG_PHASE           = 2,
	UPDATE_ST_INTERNAL_ERROR        = 3,
	UPDATE_ST_WRONG_FW_INDEX        = 4,
	UPDATE_ST_METADATA_CHECK_FAIL   = 5,
	UPDATE_ST_TEMP_UNAVAILABLE      = 6,
	UPDATE_ST_BLOB_TRANSFER_BUSY    = 7,
	UPDATE_ST_MAX,
};
/*firmware update model status code end*/


typedef struct{
    u8 first_index;
    u8 entry_limit;
}fw_update_info_get_t;

typedef struct{
    u8 list_count;
    u8 first_index;
#if 1   // only one firmware for telink SDK
    u8 fw_id_len;
    fw_id_t fw_id;
    u8 uri_len; // set 0 now.
    // uri[];
#endif
}fw_update_info_status_t;

typedef struct{
    fw_id_t fw_id; // just for demo
    #if (!(WIN32 || DRAFT_FEAT_VD_MD_EN))
    u8 rsv[4];
    #endif
}fw_metadata_t;

typedef struct{
    u8 image_index; // Index of the firmware image in the Firmware information List state that is being updated
    fw_metadata_t metadata;
}fw_update_metadata_check_t;

typedef struct{
	u8 st               :3;
	u8 additional_info  :5;
	u8 image_index;
}fw_update_metadata_check_status_t;

typedef struct{
    u8 ttl;
    u16 timeout_base; // unit: 10 second
    u8 blob_id[8];
    u8 image_index;  // Index of the firmware image in the Firmware information List state that is being updated
    fw_metadata_t metadata;
}fw_update_start_t;

typedef struct{
	u8 st               :3;
	u8 rfu              :2;
	u8 update_phase     :3;
	// --below is optional
	u8 ttl;
	u8 additional_info  :5;
	u8 rfu2             :3;
	u16 timeout_base;
	u8 blob_id[8];
	u8 image_index;
}fw_update_status_t;

static inline int is_valid_metadata_len(u32 len)
{
    return (len >= 0 && len <= sizeof(fw_metadata_t));
}

/*BLOB transfer update model status code*/
enum{ // include BLOB transfer get and BLOB block get, etc.
	BLOB_TRANS_ST_SUCCESS              	= 0,
	BLOB_TRANS_ST_INVALID_BK_NUM       	= 1,        // invalid block number, no previous block
	BLOB_TRANS_ST_INVALID_BK_SIZE      	= 2,        // bigger then Block Size Log [Object Transfer Start]
	BLOB_TRANS_ST_INVALID_CHUNK_SIZE   	= 3,        // bigger then Block Size divided by Max Chunks Number [Object Information Status]
	BLOB_TRANS_ST_WRONG_PHASE        	= 4,
	BLOB_TRANS_ST_INVALID_PAR         	= 5,
	BLOB_TRANS_ST_WRONG_BLOB_ID        	= 6,
	BLOB_TRANS_ST_BLOB_TOO_LARGE       	= 7,
	BLOB_TRANS_ST_UNSUPPORT_TRANS_MODE 	= 8,
	BLOB_TRANS_ST_INTERNAL_ERROR       	= 9,
	BLOB_TRANS_ST_INFO_UNAVAILABLE      = 0x0A,
	BLOB_TRANS_ST_MAX
};
/*BLOB transfer update model status code end*/

typedef struct{
    u8 rfu              :6;
    u8 transfer_mode    :2;
	u8 blob_id[8];
	u32 blob_size;
	u8 bk_size_log;
	u16 client_mtu_size;
}blob_transfer_start_t;

typedef struct{
	u8 blob_id[8];
}blob_transfer_cancel_t;

enum{
    BLOB_TRANS_PHASE_INACTIVE           = 0,    // be same with Idle ?
    BLOB_TRANS_PHASE_WAIT_START         = 1,
    BLOB_TRANS_PHASE_WAIT_NEXT_BLOCK    = 2,
    BLOB_TRANS_PHASE_WAIT_NEXT_CHUNK    = 3,
    BLOB_TRANS_PHASE_COMPLETE           = 4,    // The BLOB was transferred successfully.
    BLOB_TRANS_PHASE_SUSPEND            = 5,
    BLOB_TRANS_PHASE_MAX,
};

typedef struct{
	u8 st               :4;
	u8 rfu              :2;
	u8 transfer_mode    :2;
	u8 transfer_phase;
	// -- below is optional
	u8 blob_id[8];      // C1
	u32 blob_size;      // C2
	u8 bk_size_log;
	u16 transfer_mtu_size;
	u8 bk_not_receive[CEIL_8(MESH_OTA_BLOCK_MAX)];   // variable: bit field
}blob_transfer_status_t;

enum{
	BLOB_BLOCK_CHECK_SUM_TYPE_CRC32        	= 0,
	BLOB_BLOCK_CHECK_SUM_TYPE_MAX
};

typedef struct{
	u16 block_num;
	u16 chunk_size;
}blob_block_start_t;

typedef struct{
	u16 chunk_num;
	u8 data[MESH_OTA_CHUNK_SIZE];
}blob_chunk_transfer_t;

enum{
	BLOB_BLOCK_FORMAT_ALL_CHUNK_MISS   	= 0,
	BLOB_BLOCK_FORMAT_NO_CHUNK_MISS     = 1,    // 0x40
	BLOB_BLOCK_FORMAT_SOME_CHUNK_MISS  	= 2,    // 0x80
	BLOB_BLOCK_FORMAT_ENCODE_MISS_CHUNK	= 3,    // 0xc0
};

typedef struct{
	u8 st       :4;
	u8 rfu      :2;
	u8 format   :2;
#if (MESH_OTA_V_SEL == MESH_OTA_R04)
	u8 transfer_phase;
#endif
	u16 block_num;
	u16 chunk_size;
	// optional for block get
	u8 miss_chunk[MESH_OTA_CHUNK_NUM_MAX_CEIL];
}blob_block_status_t;

#define MESH_OTA_TRANSFER_MODE_PUSH     BIT(0)
#define MESH_OTA_TRANSFER_MODE_PULL     BIT(1)      // for LPN

#define MESH_OTA_TRANSFER_MODE_SEL      (MESH_OTA_TRANSFER_MODE_PUSH)

typedef struct{
	u8 bk_size_log_min;
	u8 bk_size_log_max;
	u16 chunk_num_max;
	u16 chunk_size_max;
	u32 blob_size_max;  // max firmware size
	u16 server_mtu_size;
	#if 1
    u8 transfer_mode;
	#else
	union{
	    u8 transfer_mode;
	    struct{
        	u8 transfer_mode_push   :1;
        	u8 transfer_mode_pull   :1;
        	u8 transfer_mode_rsv    :6;
    	};
	};
	#endif
}blob_info_status_t;


//--- proc
#define OTA_WAIT_ACK_MASK                (BIT(7))

enum{
	MASTER_OTA_ST_IDLE                      = 0,
	MASTER_OTA_ST_DISTRIBUT_START,
	MASTER_OTA_ST_UPDATE_METADATA_CHECK,
	MASTER_OTA_ST_SUBSCRIPTION_SET,
	MASTER_OTA_ST_FW_UPDATE_INFO_GET,
	MASTER_OTA_ST_UPDATE_START,        		// 5
	MASTER_OTA_ST_BLOB_TRANSFER_GET,
	MASTER_OTA_ST_BLOB_INFO_GET,
	MASTER_OTA_ST_BLOB_TRANSFER_START,
	MASTER_OTA_ST_BLOB_BLOCK_START,
	MASTER_OTA_ST_BLOB_BLOCK_START_CHECK_RESULT, // 10
	MASTER_OTA_ST_BLOB_CHUNK_START,
	MASTER_OTA_ST_BLOB_BLOCK_GET,
	MASTER_OTA_ST_UPDATE_GET,
	MASTER_OTA_ST_UPDATE_APPLY,
	MASTER_OTA_ST_DISTRIBUT_STOP,           // 15
	MASTER_OTA_ST_MAX,
};

typedef struct{
	u16 adr;
	u8 st_block_start;
	u8 st_block_get;
    u8 update_phase     :3;
    u8 additional_info  :5;
    u8 skip_flag        :1;
    u8 apply_flag       :1;
    u8 rsv              :6;
}fw_detail_list_t;

typedef struct{
	u8 blob_id[8];
	u16 adr_group;
	fw_detail_list_t list[MESH_OTA_UPDATE_NODE_MAX];
#if WIN32
	u8 miss_mask[max2(MESH_OTA_CHUNK_NUM_MAX_CEIL, 32)];   // set enough RAM
#else
	u8 miss_mask[max2(MESH_OTA_CHUNK_NUM_MAX_CEIL, 8)];
#endif
	blob_block_start_t block_start;
	u8 bk_size_log;  // for 4 byte align
	u16 node_cnt;    // update node cnt
	u16 node_num;    // for check all
	u32 blob_size;
	u32 bk_size_current;
	u16 chunk_num;
	u16 adr_distr_node;
	u8 miss_chunk_test_flag;
	u8 percent_last;    // progress
	u8 policy;
	u8 pause_flag;  // pause mesh ota tx flow when GATT disconnect, untill APP resume.
	u8 st;
	u8 ota_rcv_flag;    // step 1: private method of receiving update address list,
	u8 adr_set_flag;    // step 2: have received address list before, so there is no list within distribute start command.
}fw_distribut_srv_proc_t;

typedef struct{
	u32 blob_size;
	u32 crc_total;
	fw_update_start_t start;            // 0x08
	blob_block_start_t block_start;     // 0x18:2 byte align here
	u32 blob_trans_start_tick;
	u32 bk_size_current;
	u16 client_mtu_size;                // 0x1C
	u8 blk_crc_tlk_mask[(MESH_OTA_BLOCK_MAX + 7)/8];
	u8 miss_mask[MESH_OTA_CHUNK_NUM_MAX_CEIL];
	u8 blob_block_rx_ok;                // 
	u8 blob_trans_busy;
	u8 blob_trans_rx_start_error;
	u8 blob_trans_phase;
	u8 blob_block_get_retry_flag;
	u8 transfer_mode;
	u8 bk_size_log;
    u8 update_phase     :3;
    u8 additional_info  :5;
    u8 bin_crc_type;
    u8 bin_crc_done;
    u8 reboot_flag_backup;
    u8 metadata_len;
	u8 busy;
}fw_update_srv_proc_t;

typedef struct{
    
}blob_trans_srv_proc_t;

enum{
    VC_MESH_OTA_ONLY_ONE_NODE_SELF = 1,
    VC_MESH_OTA_ONLY_ONE_NODE_CONNECTED = 2,
};

// -----------
extern fw_distribut_srv_proc_t fw_distribut_srv_proc;

static inline int is_apply_phase_success(u8 update_phase)
{
    return (UPDATE_PHASE_APPLYING_UPDATE == update_phase || UPDATE_PHASE_VERIFYING_SUCCESS == update_phase);
}

static inline int is_mesh_ota_tx_client_model(u32 id, bool4 sig_model)
{
    return (sig_model && ((SIG_MD_FW_UPDATE_C == id) || (SIG_MD_BLOB_TRANSFER_C == id)
                        || (SIG_MD_BLOB_TRANSFER_C == id)));
}

static inline int is_mesh_ota_master_wait_ack()
{
    return (fw_distribut_srv_proc.st & OTA_WAIT_ACK_MASK);
}

static inline void clr_mesh_ota_master_wait_ack()
{
    fw_distribut_srv_proc.st &= ~OTA_WAIT_ACK_MASK;
}

u32 set_bit_by_cnt(u8 *out, u32 len, u32 cnt);

static inline int is_buf_bit_set(u8 *buf, u32 num)
{
    return (buf[num / 8] & BIT(num % 8));
}

static inline int buf_bit_clear(u8 *buf, u32 num)
{
    return (buf[num / 8] &= (~ BIT(num % 8)));
}

int mesh_cmd_sig_fw_update_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_metadata_check(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_metadata_check_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

#if DISTRIBUTOR_UPDATE_CLIENT_EN
int mesh_cmd_sig_fw_distribut_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_cancel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_detail_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_distribut_detail_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_fw_distribut_get               0
#define mesh_cmd_sig_fw_distribut_start             0
#define mesh_cmd_sig_fw_distribut_cancel            0
#define mesh_cmd_sig_fw_distribut_status            0
#define mesh_cmd_sig_fw_distribut_detail_get        0
#define mesh_cmd_sig_fw_distribut_detail_list       0
#endif
int mesh_cmd_sig_fw_update_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_cancel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_apply(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_fw_update_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_transfer_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_transfer_handle(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_transfer_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_block_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_chunk_transfer(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_block_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_blob_block_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

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
u32 get_total_crc_type1_new_fw();
int is_valid_ota_check_type1();
u32 get_blk_crc_tlk_type1(u8 *data, u32 len, u32 addr);
int ota_file_check();
void APP_RefreshProgressBar(u16 bk_current, u16 bk_total, u16 chunk_cur, u16 chunk_total, u8 percent);
void APP_report_mesh_ota_apply_status(u16 adr_src, fw_update_status_t *p);
u16 APP_get_GATT_connect_addr();
void APP_set_mesh_ota_pause_flag(u8 val);
int mesh_ota_and_only_one_node_check();
void check_and_clear_mesh_ota_master_wait_ack();
u32 soft_crc32_ota_flash(u32 addr, u32 len, u32 crc_init,u32 *out_crc_type1_blk);
int mesh_ota_slave_need_ota(fw_metadata_t *p_metadata, int len);
unsigned short crc16(unsigned char *pD, int len);
void mesh_ota_read_data(u32 adr, u32 len, u8 * buf);
void mesh_ota_proc();
int is_blob_chunk_transfer_ready();
void mesh_fw_distibut_set(u8 en);

extern u32	ota_program_offset;


