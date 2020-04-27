/********************************************************************************************************
 * @file     mesh_ota.c 
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
#include "proj/tl_common.h"
#if !WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "mesh_ota.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#if WIN32
u32 	ota_firmware_size_k = FW_SIZE_MAX_K;	// same with pm_8269.c
#endif

STATIC_ASSERT(FW_SIZE_MAX_K % 4 == 0);  // because ota_firmware_size_k is must 4k aligned.
STATIC_ASSERT(ACCESS_WITH_MIC_LEN_MAX >= (MESH_OTA_CHUNK_SIZE + 1 + SZMIC_TRNS_SEG64 + 7)); // 1: op code, 7:margin


u32 fw_id_local = 0;

void get_fw_id()
{
#if !WIN32
    #if FW_START_BY_BOOTLOADER_EN
    u32 fw_adr = DUAL_MODE_FW_ADDR_SIGMESH;
    #else
    u32 fw_adr = ota_program_offset ? 0 : 0x40000;
    #endif
    flash_read_page(fw_adr+2, sizeof(fw_id_local), (u8 *)&fw_id_local); // == BUILD_VERSION
#endif
}

void mesh_ota_read_data(u32 adr, u32 len, u8 * buf){
#if WIN32
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
    extern u8 fw_ota_data_rx[];
    memcpy(buf, fw_ota_data_rx + adr, len);
    #endif
#else
	flash_read_page(ota_program_offset + adr, len, buf);
#endif
}

u32 get_fw_len()
{
	u32 fw_len = 0;
	mesh_ota_read_data(0x18, 4, (u8 *)&fw_len);	// use flash read should be better
	return fw_len;
}

u8 get_ota_check_type()
{
    u8 ota_type[2] = {0};
    mesh_ota_read_data(6, sizeof(ota_type), ota_type);
	if(ota_type[0] == 0x5D){
		return ota_type[1];
	}
	return OTA_CHECK_TYPE_NONE;
}

u32 get_total_crc_type1_new_fw()
{
	u32 crc = 0;
	u32 len = get_fw_len();
	mesh_ota_read_data(len - 4, 4, (u8 *)&crc);
    return crc;
}

#define OTA_DATA_LEN_1      (16)    

int is_valid_ota_check_type1()
{	
	u32 crc_org = 0;
	u32 len = get_fw_len();
	mesh_ota_read_data(len - 4, 4, (u8 *)&crc_org);

    u8 buf[2 + OTA_DATA_LEN_1];
    u32 num = (len - 4 + (OTA_DATA_LEN_1 - 1))/OTA_DATA_LEN_1;
	u32 crc_new = 0;
    for(u32 i = 0; i < num; ++i){
    	buf[0] = i & 0xff;
    	buf[1] = (i>>8) & 0xff;
        mesh_ota_read_data((i * OTA_DATA_LEN_1), OTA_DATA_LEN_1, buf+2);
        if(!i){     // i == 0
             buf[2+8] = 0x4b;	// must
        }
        
        crc_new += crc16(buf, sizeof(buf));
        if(0 == (i & 0x0fff)){
			// about take 88ms for 10k firmware;
			#if (MODULE_WATCHDOG_ENABLE&&!WIN32)
			wd_clear();
			#endif
        }
    }
    
    return (crc_org == crc_new);
}

u32 get_blk_crc_tlk_type1(u8 *data, u32 len, u32 addr)
{	
    u8 buf[2 + OTA_DATA_LEN_1];
    u32 num = len / OTA_DATA_LEN_1; // sizeof firmware data which exclude crc, is always 16byte aligned.
    //int end_flag = ((len % OTA_DATA_LEN_1) != 0);
	u32 crc = 0;
    for(u32 i = 0; i < num; ++i){
        u32 line = (addr / 16) + i;
    	buf[0] = line & 0xff;
    	buf[1] = (line>>8) & 0xff;
    	memcpy(buf+2, data + (i * OTA_DATA_LEN_1), OTA_DATA_LEN_1);
        
        crc += crc16(buf, sizeof(buf));
    }
    return crc;
}

#if MD_MESH_OTA_EN
model_mesh_ota_t        model_mesh_ota;
u32 mesh_md_mesh_ota_addr = FLASH_ADR_MD_MESH_OTA;

//--- common
int is_cid_fwid_match(fw_cid_fwid_t *id1, fw_cid_fwid_t *id2)
{
    return (0 == memcmp(id1, id2, sizeof(fw_cid_fwid_t)));
}

int is_obj_id_match(u8 *obj_id1, u8 *obj_id2)
{
    return (0 == memcmp(obj_id1, obj_id2, 8));
}

inline u16 get_fw_block_cnt(u32 obj_size, u8 bk_size_log)
{
    u32 bk_size = (1 << bk_size_log);
    return (obj_size + bk_size - 1) / bk_size;
}

inline u16 get_block_size(u32 obj_size, u8 bk_size_log, u16 block_num)
{
    u16 bk_size = (1 << bk_size_log);
    u16 bk_cnt = get_fw_block_cnt(obj_size, bk_size_log);
    if(block_num + 1 < bk_cnt){
        return bk_size;
    }else{
        return (obj_size - block_num * bk_size);
    }
}

inline u16 get_fw_chunk_cnt(u16 bk_size_current, u16 chunk_size_max)
{
    return (bk_size_current + chunk_size_max - 1) / chunk_size_max;
}

inline u16 get_chunk_size(u16 bk_size_current, u16 chunk_size_max, u16 chunk_num)
{
    u16 chunk_cnt = get_fw_chunk_cnt(bk_size_current, chunk_size_max);
    if(chunk_num + 1 < chunk_cnt){
        return chunk_size_max;
    }else{
        return (bk_size_current - chunk_num * chunk_size_max);
    }
}

inline u32 get_fw_data_position(u16 block_num, u8 bk_size_log, u16 chunk_num, u16 chunk_size_max)
{
    return (block_num * (1 << bk_size_log) + chunk_num * chunk_size_max);
}

int is_mesh_ota_cid_match(u16 cid)
{
    #if WIN32 
    return 1;
    #else
    return (cid == cps_cid);
    #endif
}

/***************************************** 
------- for distributor node
******************************************/
#if DISTRIBUTOR_UPDATE_CLIENT_EN
// const u32 fw_id_new     = 0xff000021;   // set in distribution start
const u8  obj_id_new[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
#define TEST_CHECK_SUM_TYPE     (OBJ_BLOCK_CHECK_SUM_TYPE_CRC32)
#define TEST_BK_SIZE_LOG        (MESH_OTA_BLOCK_SIZE_LOG_MIN)

#define NEW_FW_MAX_SIZE     (FLASH_ADR_AREA_FIRMWARE_END) // = (192*1024)
u32 new_fw_size = 0;

#if GATEWAY_ENABLE
u8* fw_ota_data_tx;
#elif WIN32
u8 fw_ota_data_tx[NEW_FW_MAX_SIZE];
u8 fw_ota_data_rx[NEW_FW_MAX_SIZE] = {1,2,3,4,5,};
#endif

STATIC_ASSERT(MESH_OTA_BLOCK_SIZE_MAX <= MESH_OTA_CHUNK_SIZE * 32);
STATIC_ASSERT(MESH_OTA_CHUNK_NUM_MAX <= 32);  // max bit of miss_mask


fw_distribut_srv_proc_t fw_distribut_srv_proc = {{0}};      // for distributor (server + client) + updater client

#define master_ota_current_node_adr     (fw_distribut_srv_proc.list[fw_distribut_srv_proc.node_num].adr)


inline void mesh_ota_master_next_st_set(u8 st)
{
    fw_distribut_srv_proc.st = st;
}

inline int is_only_get_fw_info_fw_distribut_srv()
{
    return (0 == fw_distribut_srv_proc.adr_group);
}

void mesh_ota_master_next_block()
{
    fw_distribut_srv_proc.block_start_par.block_num++;
    fw_distribut_srv_proc.node_num = fw_distribut_srv_proc.chunk_num = fw_distribut_srv_proc.miss_mask = 0;
    mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START);
}

u32 distribut_get_not_apply_cnt()
{
    u32 cnt = 0;
    foreach(i, fw_distribut_srv_proc.node_cnt){
        if(0 == fw_distribut_srv_proc.list[i].apply_flag){
            cnt++;
        }
    }

    return cnt;
}

inline u16 distribut_get_fw_block_cnt()
{
    return get_fw_block_cnt(fw_distribut_srv_proc.obj_size, fw_distribut_srv_proc.bk_size_log);
}

inline u16 distribut_get_block_size(u16 block_num)
{
    return get_block_size(fw_distribut_srv_proc.obj_size, fw_distribut_srv_proc.bk_size_log, block_num);
}

inline u16 distribut_get_fw_chunk_cnt()
{
    block_transfer_start_par_t *bk_start = &fw_distribut_srv_proc.block_start_par;
    return get_fw_chunk_cnt(bk_start->bk_size_current, bk_start->chunk_size);
}

inline u16 distribut_get_chunk_size(u16 chunk_num)
{
    block_transfer_start_par_t *bk_start = &fw_distribut_srv_proc.block_start_par;
    return get_chunk_size(bk_start->bk_size_current, bk_start->chunk_size, chunk_num);
}

inline u32 distribut_get_fw_data_position(u16 chunk_num)
{
    block_transfer_start_par_t *bk_start = &fw_distribut_srv_proc.block_start_par;
    return get_fw_data_position(bk_start->block_num, fw_distribut_srv_proc.bk_size_log, chunk_num, bk_start->chunk_size);
}

#if ((ANDROID_APP_ENABLE || IOS_APP_ENABLE))
void APP_set_mesh_ota_pause_flag(u8 val)
{
    if(fw_distribut_srv_proc.st){
        fw_distribut_srv_proc.pause_flag = val;
    }
}
#else
void APP_RefreshProgressBar(u16 bk_current, u16 bk_total, u16 chunk_cur, u16 chunk_total, u8 percent)
{
    LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "OTA,block total:%2d,cur:%2d,chunk total:%2d,cur:%2d, Progress:%d%%", bk_total, bk_current, chunk_total, chunk_cur, percent);
}

void APP_report_mesh_ota_apply_status(u16 adr_src, fw_update_status_t *p)
{
    // nothing for VC now
}

u16 APP_get_GATT_connect_addr()
{
    #if GATEWAY_ENABLE
    return ele_adr_primary;
    #else
    return connect_addr_gatt;
    #endif
}

#endif

#if WIN32
void APP_print_connected_addr()
{
    u16 connect_addr = APP_get_GATT_connect_addr();
    LOG_MSG_INFO(TL_LOG_CMD_NAME, 0, 0, "connected addr 0x%04x", connect_addr);
}
#else
#define APP_print_connected_addr()      
#endif

/*
	model command callback function ----------------
*/	
void distribut_srv_proc_init()
{
    memset(&fw_distribut_srv_proc, 0, sizeof(fw_distribut_srv_proc));
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
    new_fw_size = 0;
    #endif
}

void distribut_srv_proc_init_keep_id(fw_cid_fwid_t *id)
{
    distribut_srv_proc_init();
    memcpy(&fw_distribut_srv_proc.id, id, sizeof(fw_distribut_srv_proc.id));
}


int mesh_tx_cmd_fw_distribut_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st, fw_cid_fwid_t *p_id)
{
	fw_distribut_status_t rsp = {0};
	rsp.st = st;
	memcpy(&rsp.id, p_id, sizeof(rsp.id));

	return mesh_tx_cmd_rsp(FW_DISTRIBUT_STATUS, (u8 *)&rsp, sizeof(fw_distribut_status_t), ele_adr, dst_adr, 0, 0);
}

int mesh_fw_distribut_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st, fw_cid_fwid_t *p_id)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_fw_distribut_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st, p_id);
}

int mesh_cmd_sig_fw_distribut_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    fw_distribut_get_t *p_get = (fw_distribut_get_t *)par;
	u8 st;
    if(fw_distribut_srv_proc.st){
        if(is_cid_fwid_match(&p_get->id, &fw_distribut_srv_proc.id)){
    	    st = DISTRIBUT_ST_ACTIVE;
        }else{
    	    st = DISTRIBUT_ST_NO_SUCH_ID;
    	}
    }else{
        st = DISTRIBUT_ST_NO_SUCH_ID;
    }
	return mesh_fw_distribut_st_rsp(cb_par, st, &p_get->id);
}

int read_ota_file2buffer(fw_cid_fwid_t *id)
{
#if VC_APP_ENABLE
    if(0 != ota_file_check()){
        return -1;
    }
    else
#endif
    {
#if DISTRIBUTOR_UPDATE_CLIENT_EN
    #if GATEWAY_ENABLE
        fw_ota_data_tx = (u8*)(ota_program_offset);//reflect to the flash part 
        new_fw_size = get_fw_len();
    #else
        new_fw_size = new_fw_read(fw_ota_data_tx, sizeof(fw_ota_data_tx));
    #endif
        
        if((0 == new_fw_size) || (-1 == new_fw_size)){
            return -1;
        }
#endif
    }

    return 0;
}

int mesh_cmd_sig_fw_distribut_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 st = DISTRIBUT_ST_NOT_ACTIVE;
    fw_distribut_start_t *p_start = (fw_distribut_start_t *)par;
    u32 update_node_cnt = (par_len - OFFSETOF(fw_distribut_start_t,update_list)) / 2;
    
    if(!is_mesh_ota_cid_match(p_start->id.cid)){
    	st = DISTRIBUT_ST_NO_SUCH_ID;
    }else if(update_node_cnt > MESH_OTA_UPDATE_NODE_MAX ||
    		 update_node_cnt == 0){
    	st = DISTRIBUT_ST_UPDATE_NODE_LIST_TOO_LONG;
    }else if(fw_distribut_srv_proc.st){   // comfirm later
        int retransmit = (0 == memcmp(p_start,&fw_distribut_srv_proc.id, OFFSETOF(fw_distribut_start_t,update_list)));
        if(retransmit){
            foreach(i,update_node_cnt){
                if(fw_distribut_srv_proc.list[i].adr != p_start->update_list[i]){
                    retransmit = 0;
                    break;
                }
            }
    	}
    	
    	st = retransmit ? DISTRIBUT_ST_ACTIVE : DISTRIBUT_ST_BUSY_WITH_DIFF;
    }else{
        APP_print_connected_addr();
        distribut_srv_proc_init_keep_id(&p_start->id);
        fw_distribut_srv_proc.adr_group = p_start->adr_group;
        fw_distribut_srv_proc.adr_distr_node = ele_adr_primary;
        if(0 != read_ota_file2buffer(&p_start->id)){
            distribut_srv_proc_init_keep_id(&p_start->id);
            return 0;   // error
        }
        
        if(update_node_cnt){
            foreach(i,update_node_cnt){
                fw_distribut_srv_proc.list[i].adr = p_start->update_list[i];
                fw_distribut_srv_proc.list[i].st_block_trans_start = UPDATE_NODE_ST_IN_PROGRESS;
            }
            fw_distribut_srv_proc.node_cnt = update_node_cnt;
        }

        #if WIN32 
        if(is_only_get_fw_info_fw_distribut_srv()){
            mesh_ota_master_next_st_set(MASTER_OTA_ST_FW_INFO_GET);
        }else
        #endif
        {
            mesh_ota_master_next_st_set(MASTER_OTA_ST_DISTRIBUT_START);
        }
        
	    st = DISTRIBUT_ST_ACTIVE;
	}
	
	return mesh_fw_distribut_st_rsp(cb_par, st, &p_start->id);
}

int mesh_cmd_sig_fw_distribut_stop(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    fw_distribut_stop_t *p_stop = (fw_distribut_stop_t *)par;
    int abort_flag = 0;
	u8 st = DISTRIBUT_ST_NO_SUCH_ID;
    if(is_cid_fwid_match(&p_stop->id, &fw_distribut_srv_proc.id)){
        if(fw_distribut_srv_proc.st != MASTER_OTA_ST_MAX){
            abort_flag = 1;
        }
        distribut_srv_proc_init_keep_id(&p_stop->id);
        // fw_distribut_srv_proc.st = 0;
        
        st = DISTRIBUT_ST_NOT_ACTIVE;
    }

	int err = mesh_fw_distribut_st_rsp(cb_par, st, &p_stop->id);
	if(abort_flag){
        access_cmd_fw_update_control(ADR_ALL_NODES, FW_UPDATE_ABORT, 0xff);
	}else{
        #if VC_APP_ENABLE
        extern int disable_log_cmd;
        disable_log_cmd = 1;   // mesh OTA finished
        #endif
	}
	
	return err;
}

int mesh_cmd_sig_fw_distribut_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

// -------
int mesh_cmd_sig_fw_distribut_detail_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    fw_distribut_get_t *p_get = (fw_distribut_get_t *)par;
    fw_distribut_detail_list_t rsp = {{{0}}};
    u32 rsp_size = 0;
    if(fw_distribut_srv_proc.st){
        if(is_cid_fwid_match(&p_get->id, &fw_distribut_srv_proc.id)){
            u32 node_cnt = fw_distribut_srv_proc.node_cnt;
            if(node_cnt > MESH_OTA_UPDATE_NODE_MAX){
                node_cnt = MESH_OTA_UPDATE_NODE_MAX;
            }
            
            rsp_size = node_cnt * sizeof(fw_distribut_node_t);
            foreach(i,node_cnt){
                rsp.node[i].adr = fw_distribut_srv_proc.list[i].adr;
                rsp.node[i].st = fw_distribut_srv_proc.list[i].st_block_trans_start;
            }
    	}
    }
    
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_rsp(FW_DISTRIBUT_DETAIL_LIST, (u8 *)&rsp, rsp_size, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_fw_distribut_detail_list(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//--model command interface-------------------
//-----------access command--------
#if 0
int access_cmd_fw_distribut_start(u16 adr_dst)
{
	fw_distribut_start_t cmd;
	cmd.id
	cmd.adr_group
	foreach(i,n){
	    cmd.update_list[i] = ;
	}
	return SendOpParaDebug(adr_dst, 1, FW_DISTRIBUT_START, (u8 *)&cmd, sizeof(cmd));
}
#endif

int access_cmd_fw_distribut_stop(u16 adr_dst)
{
    LOG_MSG_FUNC_NAME();
	fw_distribut_stop_t cmd;
    memcpy(&cmd.id, &fw_distribut_srv_proc.id, sizeof(cmd.id));
	return SendOpParaDebug(adr_dst, 1, FW_DISTRIBUT_STOP, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_fw_info_get(u16 adr_dst)
{
    LOG_MSG_FUNC_NAME();
	u8 par[1] = {0};
	return SendOpParaDebug(adr_dst, 1, FW_INFO_GET, par, 0);
}

int access_cmd_obj_info_get(u16 adr_dst)
{
    LOG_MSG_FUNC_NAME();
	u8 par[1] = {0};
	return SendOpParaDebug(adr_dst, 1, OBJ_INFO_GET, par, 0);
}

int access_cmd_fw_update_prepare(u16 adr_dst, const u8 *obj_id)
{
    LOG_MSG_FUNC_NAME();
    fw_update_prepare_t cmd;
    memcpy(&cmd.id, &fw_distribut_srv_proc.id, sizeof(cmd.id));
    memcpy(cmd.obj_id, obj_id, sizeof(cmd.obj_id));
    memcpy(fw_distribut_srv_proc.obj_id, obj_id, sizeof(fw_distribut_srv_proc.obj_id));   // back up
	return SendOpParaDebug(adr_dst, 1, FW_UPDATE_PREPARE, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_fw_update_start(u16 adr_dst, u8 policy)
{
    LOG_MSG_FUNC_NAME();
    fw_update_start_t cmd;
    cmd.policy = fw_distribut_srv_proc.policy = policy;
    memcpy(&cmd.id, &fw_distribut_srv_proc.id, sizeof(cmd.id));
	return SendOpParaDebug(adr_dst, 1, FW_UPDATE_START, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_fw_update_get(u16 adr_dst)
{
    LOG_MSG_FUNC_NAME();
    fw_update_get_t cmd;
    memcpy(&cmd.id, &fw_distribut_srv_proc.id, sizeof(cmd.id));
	return SendOpParaDebug(adr_dst, 1, FW_UPDATE_GET, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_fw_update_control(u16 adr_dst, u16 op, u8 rsp_max)
{
    if(FW_UPDATE_APPLY == op){
        LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "access_cmd_fw_update_apply ",0);
    }else if(FW_UPDATE_ABORT == op){
        LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "access_cmd_fw_update_abort ",0);
    }else{
        LOG_MSG_FUNC_NAME();
    }
    fw_update_control_t cmd;
    memcpy(&cmd.id, &fw_distribut_srv_proc.id, sizeof(cmd.id));
	return SendOpParaDebug(adr_dst, rsp_max, op, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_obj_transfer_start(u16 adr_dst, u32 obj_size, u8 bk_size_log)
{
    LOG_MSG_FUNC_NAME();
    obj_transfer_start_t cmd;
    memcpy(&cmd.obj_id, fw_distribut_srv_proc.obj_id, sizeof(cmd.obj_id));
    cmd.obj_size = fw_distribut_srv_proc.obj_size = obj_size;
    cmd.bk_size_log = fw_distribut_srv_proc.bk_size_log = bk_size_log;
	return SendOpParaDebug(adr_dst, 1, OBJ_TRANSFER_START, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_obj_block_transfer_start(u16 adr_dst, u16 block_num, u32 check_sum_val)
{
    LOG_MSG_FUNC_NAME();
    block_transfer_start_par_t *p_bk_par = &fw_distribut_srv_proc.block_start_par;  // record parameters
    p_bk_par->block_num = block_num;
    p_bk_par->chunk_size = MESH_OTA_CHUNK_SIZE;
    p_bk_par->bk_check_sum_type = TEST_CHECK_SUM_TYPE;
    p_bk_par->bk_check_sum_val = check_sum_val;
    p_bk_par->bk_size_current = distribut_get_block_size(block_num);

    obj_block_transfer_start_t cmd;
    memcpy(cmd.obj_id, fw_distribut_srv_proc.obj_id, sizeof(cmd.obj_id));
    memcpy(&cmd.par, p_bk_par, sizeof(cmd.par));
	return SendOpParaDebug(adr_dst, 1, OBJ_BLOCK_TRANSFER_START, (u8 *)&cmd, sizeof(cmd));
}

int access_cmd_obj_chunk_transfer(u16 adr_dst, u8 *cmd, u32 len)
{
	return SendOpParaDebug(adr_dst, 0, OBJ_CHUNK_TRANSFER, cmd, len);
}

int access_cmd_obj_block_get(u16 adr_dst, u16 block_num)
{
    LOG_MSG_FUNC_NAME();
    obj_block_get_t cmd;
    memcpy(&cmd.obj_id, fw_distribut_srv_proc.obj_id, sizeof(cmd.obj_id));
    cmd.block_num = block_num;
	return SendOpParaDebug(adr_dst, 1, OBJ_BLOCK_GET, (u8 *)&cmd, sizeof(cmd));
}

//--model command interface end----------------

//--mesh ota master proc
fw_detail_list_t * get_fw_node_detail_list(u16 node_adr)
{
    foreach(i,fw_distribut_srv_proc.node_cnt){
        fw_detail_list_t *p_list = &fw_distribut_srv_proc.list[i];
        if(p_list->adr == node_adr){
            return p_list;
        }
    }
    return 0;
}

u32 is_need_block_transfer()
{
    foreach(i,fw_distribut_srv_proc.node_cnt){
        fw_detail_list_t *p_list = &fw_distribut_srv_proc.list[i];
        if(OBJ_BLOCK_TRANS_ST_ACCEPTED == p_list->st_block_trans_start){
            return 1;
        }
    }
    return 0;
}

#if DISTRIBUTOR_UPDATE_CLIENT_EN
void mesh_ota_master_wait_ack_st_set()
{
    mesh_ota_master_next_st_set(fw_distribut_srv_proc.st | OTA_WAIT_ACK_MASK);
}

void mesh_ota_master_wait_ack_st_return(int success)
{
    if(!success){
        fw_distribut_srv_proc.list[fw_distribut_srv_proc.node_num].skip_flag = 1;
    }
    fw_distribut_srv_proc.node_num++;
    mesh_ota_master_next_st_set(fw_distribut_srv_proc.st & (~OTA_WAIT_ACK_MASK));
}

int mesh_ota_check_skip_current_node()
{
    fw_distribut_srv_proc_t *distr_proc = &fw_distribut_srv_proc;
    if(distr_proc->list[distr_proc->node_num].skip_flag){
        if(MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START == distr_proc->st){
            LOG_MSG_INFO(TL_LOG_COMMON,0,0,"access_cmd_obj_block_transfer_start, XXXXXX Skip addr:0x%04x", distr_proc->list[distr_proc->node_num].adr);
        }
        distr_proc->node_num++;
        return 1;
    }
    return 0;
}

#if DEBUG_SHOW_VC_SELF_EN
int is_mesh_ota_and_only_VC_update()
{
    fw_distribut_srv_proc_t *distr_proc = &fw_distribut_srv_proc;
    if(MASTER_OTA_ST_OBJ_CHUNK_START == distr_proc->st){
        if((1 == distr_proc->node_cnt) && (distr_proc->list[0].adr == ele_adr_primary)){
            return 1;
        }
    }
    return 0;
}
#endif

void mesh_ota_master_ack_timeout_handle()
{
    if(fw_distribut_srv_proc.st & OTA_WAIT_ACK_MASK){
        mesh_ota_master_wait_ack_st_return(0);
    }
}

int mesh_ota_master_rx (mesh_rc_rsp_t *rsp, u16 op, u32 size_op)
{
    int op_handle_ok = 0;
    u8 *par = rsp->data + size_op;
    u16 par_len = GET_PAR_LEN_FROM_RSP(rsp->len, size_op);
    fw_distribut_srv_proc_t *distr_proc = &fw_distribut_srv_proc;
    int adr_match = (rsp->src == master_ota_current_node_adr);
    int next_st = 0;
    
    if(FW_INFO_STATUS == op){
        if(adr_match && ((MASTER_OTA_ST_FW_INFO_GET | OTA_WAIT_ACK_MASK) == distr_proc->st)){
            //fw_info_status_t *p = (fw_info_status_t *)par;
            next_st = 1;
        }
        op_handle_ok = 1;
    }else if(FW_DISTRIBUT_STATUS == op){
        fw_distribut_status_t *p = (fw_distribut_status_t *)par;
        if(DISTRIBUT_ST_ACTIVE == p->st){
        }else if(DISTRIBUT_ST_NOT_ACTIVE == p->st){
        }

        if(distr_proc->adr_group){  // distribute start
            if(DISTRIBUT_ST_ACTIVE != p->st){
                LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "fw distribution status error:%d ", p->st);
            }
        }else{                      // distribute stop
            LOG_MSG_INFO(TL_LOG_CMD_NAME, 0, 0, "mesh OTA completed or get info ok!", 0);
        }
        op_handle_ok = 1;
    }else if(FW_DISTRIBUT_DETAIL_LIST == op){
        op_handle_ok = 1;
    }else if(FW_UPDATE_STATUS == op){
        if(adr_match){
            fw_update_status_t *p = (fw_update_status_t *)par;
            p->st = p->st;  // TODO
            fw_detail_list_t * p_list = get_fw_node_detail_list(rsp->src);
            p_list->phase = p->phase;
            p_list->additional_info = p->additional_info;
            
            if((MASTER_OTA_ST_UPDATE_PREPARE | OTA_WAIT_ACK_MASK) == distr_proc->st){
                next_st = 1;
            }else if((MASTER_OTA_ST_UPDATE_START | OTA_WAIT_ACK_MASK) == distr_proc->st){
                next_st = 1;
            }else if((MASTER_OTA_ST_UPDATE_GET | OTA_WAIT_ACK_MASK) == distr_proc->st){
                next_st = 1;
            }else if((MASTER_OTA_ST_UPDATE_APPLY | OTA_WAIT_ACK_MASK) == distr_proc->st){
                next_st = 1;
				if(UPDATE_ST_SUCCESS != p->st){
					fw_distribut_srv_proc.list[fw_distribut_srv_proc.node_num].skip_flag = 0;
				}
				APP_report_mesh_ota_apply_status(rsp->src, p);
            }
        
            if(UPDATE_ST_SUCCESS != p->st){
                LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "fw update status error:%d ", p->st);
            }
        }
        op_handle_ok = 1;
    }else if(OBJ_TRANSFER_STATUS == op){
        if(adr_match){
            obj_transfer_status_t *p = (obj_transfer_status_t *)par;
            p->st = p->st;  // TODO
            if((MASTER_OTA_ST_OBJ_TRANSFER_START | OTA_WAIT_ACK_MASK) == distr_proc->st){
                next_st = 1;
            }
        
            if(OBJ_TRANS_ST_BUSY != p->st){
                LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "object transfer status error:%d ", p->st);
            }
        }
        op_handle_ok = 1;
    }else if(OBJ_BLOCK_TRANSFER_STATUS == op){
        if(adr_match){
            obj_block_transfer_status_t *p = (obj_block_transfer_status_t *)par;
            p->st = p->st;  // TODO
            if((MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START | OTA_WAIT_ACK_MASK) == distr_proc->st){
                distr_proc->list[distr_proc->node_num].st_block_trans_start = p->st;
                next_st = 1;
            }
        
            if(p->st > OBJ_BLOCK_TRANS_ST_ALREADY_RX){
                LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "object block transfer status error:%d ", p->st);
            }
        }
        op_handle_ok = 1;
    }else if(OBJ_BLOCK_STATUS == op){
        if(adr_match){
            obj_block_status_t *p = (obj_block_status_t *)par;
            if((MASTER_OTA_ST_OBJ_BLOCK_GET | OTA_WAIT_ACK_MASK) == distr_proc->st){
                distr_proc->list[distr_proc->node_num].st_block_get = p->st;
                if(OBJ_BLOCK_ST_NOT_ALL_CHUNK_RX == p->st){
                    u32 cnt = (par_len - OFFSETOF(obj_block_status_t,miss_chunk))/2;
                    foreach(i,cnt){
                        if(p->miss_chunk[i] < 32){  // max bit of miss_mask
                            distr_proc->miss_mask |= BIT(p->miss_chunk[i]);
                        }
                    }
                }
                next_st = 1;
            }
        
            if(OBJ_BLOCK_ST_ALL_CHUNK_RX != p->st){
                LOG_MSG_ERR (TL_LOG_COMMON, 0, 0, "object block status error:%d ", p->st);
            }
        }
        op_handle_ok = 1;
    }else if(OBJ_INFO_STATUS == op){
        if(adr_match && ((MASTER_OTA_ST_OBJ_INFO_GET | OTA_WAIT_ACK_MASK) == distr_proc->st)){
            //obj_info_status_t *p = (obj_info_status_t *)par;
            next_st = 1;
        }
        op_handle_ok = 1;
    }else if(CFG_MODEL_SUB_STATUS == op){
        if(adr_match && ((MASTER_OTA_ST_SUBSCRIPTION_SET | OTA_WAIT_ACK_MASK) == distr_proc->st)){
            mesh_cfg_model_sub_status_t *p = (mesh_cfg_model_sub_status_t *)par;
            if(SIG_MD_OBJ_TRANSFER_S == (p->set.model_id & 0xffff)){
                if(ST_SUCCESS == p->status){
                }else{
                    LOG_MSG_ERR(TL_LOG_COMMON,0, 0,"set group failed %x",p->set.ele_adr);
                }
                next_st = 1;
                op_handle_ok = 1;
            }
        }
    }

    if(next_st){
        mesh_ota_master_wait_ack_st_return(1);
    }
    
    return op_handle_ok;
}

void mesh_ota_master_proc()
{
    fw_distribut_srv_proc_t *distr_proc = &fw_distribut_srv_proc;
	if(0 == distr_proc->st || is_busy_segment_flow()){
		return ;
	}
	
#if (WIN32 && (PROXY_HCI_SEL == PROXY_HCI_GATT))
    extern unsigned char connect_flag;
    if(!(pair_login_ok || DEBUG_SHOW_VC_SELF_EN) || distr_proc->pause_flag){
        return ;
    }
#endif

	static u32 tick_ota_master_proc;
	if((distr_proc->st != MASTER_OTA_ST_DISTRIBUT_START)
	&& (distr_proc->st != MASTER_OTA_ST_OBJ_CHUNK_START)){
	    if(clock_time_exceed(tick_ota_master_proc, 3000*1000)){
    	    tick_ota_master_proc = clock_time();
            LOG_MSG_INFO(TL_LOG_COMMON,0, 0,"mesh_ota_master_proc state: %d",distr_proc->st);
        }
	}else{
	    tick_ota_master_proc = clock_time();
	}

    if(distr_proc->st & OTA_WAIT_ACK_MASK){
        return ;
    }
	
	switch(distr_proc->st){
		case MASTER_OTA_ST_DISTRIBUT_START:
		    // FW_DISTRIBUT_START was send by VC
		    distr_proc->node_num = 0;
		    if (!is_only_get_fw_info_fw_distribut_srv()){
    		    mesh_ota_master_next_st_set(MASTER_OTA_ST_SUBSCRIPTION_SET);
                APP_RefreshProgressBar(0, 0, 0, 0, 0);
            }else{
    		    mesh_ota_master_next_st_set(MASTER_OTA_ST_FW_INFO_GET);
            }
			break;
			
		case MASTER_OTA_ST_SUBSCRIPTION_SET:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == cfg_cmd_sub_set(CFG_MODEL_SUB_ADD, master_ota_current_node_adr, master_ota_current_node_adr, 
    	                        fw_distribut_srv_proc.adr_group, SIG_MD_OBJ_TRANSFER_S, 1)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_FW_INFO_GET);
	        }
			break;
			
		case MASTER_OTA_ST_FW_INFO_GET:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_fw_info_get(master_ota_current_node_adr)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
	            if(is_only_get_fw_info_fw_distribut_srv()){
    	            distribut_srv_proc_init();  // stop
	            }else{
                    distr_proc->node_num = 0;
                    mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_INFO_GET);
    	        }
	        }
			break;
			
		case MASTER_OTA_ST_OBJ_INFO_GET:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_obj_info_get(master_ota_current_node_adr)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_UPDATE_PREPARE);
	        }
			break;
			
		case MASTER_OTA_ST_UPDATE_PREPARE:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_fw_update_prepare(master_ota_current_node_adr, obj_id_new)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_UPDATE_START);
	        }
			break;
			
		case MASTER_OTA_ST_UPDATE_START:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_fw_update_start(master_ota_current_node_adr, UPDATE_POLICY_NONE)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_TRANSFER_START);
	        }
			break;
			
		case MASTER_OTA_ST_OBJ_TRANSFER_START:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_obj_transfer_start(master_ota_current_node_adr, new_fw_size, TEST_BK_SIZE_LOG)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START);
	        }
			break;
			
		case MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START:
		{
            u16 block_num_current = fw_distribut_srv_proc.block_start_par.block_num;
		    if(block_num_current < distribut_get_fw_block_cnt()){
    		    if(distr_proc->node_num < distr_proc->node_cnt){
                    if(mesh_ota_check_skip_current_node()){ break;}
                    
    		        u32 adr = distribut_get_fw_data_position(0);
    		        u16 size = distribut_get_block_size(block_num_current);
					u32 crc =0;
					#if !WIN32
					if(block_num_current == 0){
						u8 crc_buf[16];
						flash_read_page(ota_program_offset,sizeof(crc_buf),crc_buf);
						crc_buf[8]= get_fw_ota_value();
						crc = soft_crc32_telink(crc_buf ,sizeof(crc_buf), 0);
						crc = soft_crc32_ota_flash(sizeof(crc_buf),size-16,crc,0);
					}else
					#endif
					{
						#if GATEWAY_ENABLE
						crc = soft_crc32_ota_flash(adr,size,0,0);
						#else
						crc = soft_crc32_telink(fw_ota_data_tx + adr, size, 0);
						#endif
					}
					
        	        if(0 == access_cmd_obj_block_transfer_start(master_ota_current_node_adr, block_num_current, crc)){
                        mesh_ota_master_wait_ack_st_set();
        	        }
    	        }else{
        	        mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START_CHECK_RESULT);
    	        }
	        }else{
                distr_proc->node_num = 0;
                mesh_ota_master_next_st_set(MASTER_OTA_ST_UPDATE_GET);
	        }
			break;
		}
			
		case MASTER_OTA_ST_OBJ_BLOCK_TRANSFER_START_CHECK_RESULT:
            if(is_need_block_transfer()){
                distr_proc->chunk_num = 0;
                distr_proc->miss_mask = BIT_MASK_LEN(distribut_get_fw_chunk_cnt()); // all send
                
    	        mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_CHUNK_START);
	        }else{
	            mesh_ota_master_next_block();
	        }
			break;
			
		case MASTER_OTA_ST_OBJ_CHUNK_START:
		{
		    u32 chunk_cnt = distribut_get_fw_chunk_cnt();
		    if(distr_proc->chunk_num < chunk_cnt){
		        if(distr_proc->chunk_num >= 32 || (BIT(distr_proc->chunk_num) & distr_proc->miss_mask)){
                    obj_chunk_transfer_t cmd = {0};
                    cmd.chunk_num = distr_proc->chunk_num;
                    u16 size = distribut_get_chunk_size(cmd.chunk_num);
                    if(size > MESH_OTA_CHUNK_SIZE){
                        size = MESH_OTA_CHUNK_SIZE;
                    }

                    u32 fw_pos = 0;
					u8 *data =0;
					u16 block_num_current = fw_distribut_srv_proc.block_start_par.block_num;
					
					#if !WIN32
					if(block_num_current == 0 && cmd.chunk_num == 0){
						u8 first_chunk[MESH_OTA_CHUNK_SIZE];
						flash_read_page(ota_program_offset,sizeof(first_chunk),first_chunk);
						first_chunk[8] = get_fw_ota_value();
						data = first_chunk;
						memcpy(cmd.data, data, size);
					}else
					#endif
					{
						fw_pos = distribut_get_fw_data_position(cmd.chunk_num);
						#if GATEWAY_ENABLE
						flash_read_page(ota_program_offset+fw_pos,sizeof(cmd.data),cmd.data);
						#else
						data = &fw_ota_data_tx[fw_pos];
						memcpy(cmd.data, data, size);
						#endif
					}
					
                    u16 bk_total = distribut_get_fw_block_cnt();
                    u8 percent = 1 + (fw_pos+size)*98/distr_proc->obj_size;
                    if(percent > distr_proc->percent_last){
                        distr_proc->percent_last = percent;
                        APP_RefreshProgressBar(block_num_current, bk_total, distr_proc->chunk_num, chunk_cnt, percent);
                    }
    		        if(0 == access_cmd_obj_chunk_transfer(fw_distribut_srv_proc.adr_group, (u8 *)&cmd, size+2)){
    		            distr_proc->chunk_num++;
    		        }
		        }else{
		            distr_proc->chunk_num++;
		        }
	        }else{
	            distr_proc->node_num = distr_proc->chunk_num = distr_proc->miss_mask = 0;
	            mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_BLOCK_GET);
	        }
	    }
			break;
			
		case MASTER_OTA_ST_OBJ_BLOCK_GET:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_obj_block_get(master_ota_current_node_adr, distr_proc->block_start_par.block_num)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
	            if(distr_proc->miss_mask){
                    distr_proc->chunk_num = 0;
                    LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "access_cmd_obj_chunk_transfer retry",0);
                    mesh_ota_master_next_st_set(MASTER_OTA_ST_OBJ_CHUNK_START);
	            }else{
                    mesh_ota_master_next_block();
    	        }
	        }
			break;
			
		case MASTER_OTA_ST_UPDATE_GET:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        if(mesh_ota_check_skip_current_node()){ break;}
		        
    	        if(0 == access_cmd_fw_update_get(master_ota_current_node_adr)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
                mesh_ota_master_next_st_set(MASTER_OTA_ST_UPDATE_APPLY);
	        }
			break;
			
		case MASTER_OTA_ST_UPDATE_APPLY:
		    if(distr_proc->node_num < distr_proc->node_cnt){
		        //if(mesh_ota_check_skip_current_node()){ break;}
		        fw_detail_list_t *p_list = &distr_proc->list[distr_proc->node_num];
		        if(p_list->apply_flag
		         || ((p_list->adr == APP_get_GATT_connect_addr()) && (distribut_get_not_apply_cnt() > 1))){
		            distr_proc->node_num++;
		            break ;
		        }
		        
		        p_list->apply_flag = 1;
		        
		        u16 op;
		        if(UPDATE_PHASE_DUF_READY == p_list->phase){
		            op = FW_UPDATE_APPLY;
		        }else{
		            op = FW_UPDATE_ABORT;
		        }
		        
    	        if(0 == access_cmd_fw_update_control(master_ota_current_node_adr, op, 1)){
    	            mesh_ota_master_wait_ack_st_set();
    	        }
	        }else{
                distr_proc->node_num = 0;
                if(distribut_get_not_apply_cnt() == 0){
                    mesh_ota_master_next_st_set(MASTER_OTA_ST_DISTRIBUT_STOP);
                    APP_print_connected_addr();
                }else{
                    mesh_ota_master_next_st_set(MASTER_OTA_ST_UPDATE_APPLY);
                }
	        }
			break;
			
		case MASTER_OTA_ST_DISTRIBUT_STOP:
			#if GATEWAY_ENABLE
			{
				fw_detail_list_t *p_list = fw_distribut_srv_proc.list;
				u8 mesh_ota_sts[1+2*MESH_OTA_UPDATE_NODE_MAX];
				u8 mesh_ota_idx =0;
				mesh_ota_sts[0]=0;
				for(int i=0;i<fw_distribut_srv_proc.node_cnt;i++){
					p_list = &(fw_distribut_srv_proc.list[i]);
					if(p_list->skip_flag){
						mesh_ota_sts[0]++;
						memcpy(mesh_ota_sts+1+2*mesh_ota_idx,(u8 *)&(p_list->adr),2);
						mesh_ota_idx++;
					}
				}
				gateway_upload_mesh_ota_sts(mesh_ota_sts,1+(mesh_ota_sts[0])*2);
			}
			#endif
			{
				u32 st_back = distr_proc->st;
				mesh_ota_master_next_st_set(MASTER_OTA_ST_MAX);	// must set before tx cmd, because gateway use it when rx this command.
		        if(0 == access_cmd_fw_distribut_stop(distr_proc->adr_distr_node)){
		            // no need, ota flow have been stop in mesh_cmd_sig_fw_distribut_stop(),
	                APP_RefreshProgressBar(0, 0, 0, 0, 100);
		        }else{
		        	mesh_ota_master_next_st_set(st_back);
		        }
			}
			break;

		default :
		    distr_proc->node_num = distr_proc->node_num;
			break;
	}
}
#else
int mesh_ota_master_rx (mesh_rc_rsp_t *rsp, u16 op, u32 size_op){return 0;}
void mesh_ota_master_proc(){}
void mesh_ota_master_ack_timeout_handle(){}
#endif
#endif


/***************************************** 
------- for updater node
******************************************/
#if 1
fw_update_srv_proc_t    fw_update_srv_proc = {{0}};         // for updater

void mesh_ota_save_data(u32 adr, u32 len, u8 * data){
#if WIN32
    #if DISTRIBUTOR_UPDATE_CLIENT_EN
	if (adr == 0){
	    fw_update_srv_proc.reboot_flag_backup = data[8];
		data[8] = 0xff;					//FW flag invalid
	}
	#if VC_APP_ENABLE
    memcpy(fw_ota_data_rx + adr, data, len);
	#endif
    #endif
#else
    #if(__TL_LIB_8267__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8267) || \
	    __TL_LIB_8269__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8269) ||	\
	    __TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258) || \
	    (MCU_CORE_TYPE == MCU_CORE_8278))
	if (adr == 0){
	    fw_update_srv_proc.reboot_flag_backup = data[8];
		data[8] = 0xff;					//FW flag invalid
	}
    #endif
	flash_write_page(ota_program_offset + adr, len, data);
#endif
}

u32 soft_crc32_ota_flash(u32 addr, u32 len, u32 crc_init,u32 *out_crc_type1_blk)
{
    u32 crc_type1_blk = 0;
    u8 buf[64];      // CRC_SIZE_UNIT
    while(len){
        u32 len_read = (len > sizeof(buf)) ? sizeof(buf) : len;
        #if 1
        mesh_ota_read_data(addr, len_read, buf);
        #else
        flash_read_page(addr, len_read, buf);
        #endif
        if(0 == addr){
            buf[8] = fw_update_srv_proc.reboot_flag_backup;
        }
        crc_init = soft_crc32_telink(buf, len_read, crc_init);
		if(out_crc_type1_blk){
        	crc_type1_blk += get_blk_crc_tlk_type1(buf, len_read, addr);	// use to get total crc of total firmware.
		}
        
        len -= len_read;
        addr += len_read;
    }
    if(out_crc_type1_blk){
		*out_crc_type1_blk = crc_type1_blk;
	}
    return crc_init;
}

int is_valid_mesh_ota_len(u32 fw_len)
{
    #if FW_ADD_BYTE_EN
    u32 len_org = get_fw_len();
	return ((fw_len >= len_org) && (fw_len <= len_org + 20));
	#else
	return (fw_len == get_fw_len());
	#endif
}

int is_valid_telink_fw_flag()
{
    u8 fw_flag_telink[4] = {0x4B,0x4E,0x4C,0x54};
    u8 fw_flag[4] = {0};
    mesh_ota_read_data(8, sizeof(fw_flag), fw_flag);
    fw_flag[0] = fw_update_srv_proc.reboot_flag_backup;
	if(!memcmp(fw_flag,fw_flag_telink, 4) && is_valid_mesh_ota_len(fw_update_srv_proc.obj_size)){
		return 1;
	}
	return 0;
}

enum{
    BIN_CRC_DONE_NONE       = 0,    // must 0
    BIN_CRC_DONE_OK         = 1,
    BIN_CRC_DONE_FAIL       = 2,
};

int is_valid_mesh_ota_calibrate_val()
{
    // eclipse crc32 calibrate
    #if (0 == DEBUG_EVB_EN)
    if(!is_valid_telink_fw_flag()){
        return 0;
    }
    #endif

	if(OTA_CHECK_TYPE_TELINK_MESH == get_ota_check_type()){
	    if(0 == fw_update_srv_proc.bin_crc_done){
    	    u32 len = fw_update_srv_proc.obj_size;
    		int crc_ok = (is_valid_mesh_ota_len(len) 
    		          && (fw_update_srv_proc.crc_total == get_total_crc_type1_new_fw()));  // is_valid_ota_check_type1()
    		          
    		fw_update_srv_proc.bin_crc_done = crc_ok ? BIN_CRC_DONE_OK : BIN_CRC_DONE_FAIL;
		}
        return (BIN_CRC_DONE_OK == fw_update_srv_proc.bin_crc_done);
	}
	return 1;
}


inline u16 updater_get_fw_block_cnt()
{
    return get_fw_block_cnt(fw_update_srv_proc.obj_size, fw_update_srv_proc.bk_size_log);
}

inline u16 updater_get_block_size(u16 block_num)
{
    return get_block_size(fw_update_srv_proc.obj_size, fw_update_srv_proc.bk_size_log, block_num);
}

inline u16 updater_get_fw_chunk_cnt()
{
    block_transfer_start_par_t *bk_start = &fw_update_srv_proc.block_start_par;
    return get_fw_chunk_cnt(bk_start->bk_size_current, bk_start->chunk_size);
}

inline u16 updater_get_chunk_size(u16 chunk_num)
{
    block_transfer_start_par_t *bk_start = &fw_update_srv_proc.block_start_par;
    return get_chunk_size(bk_start->bk_size_current, bk_start->chunk_size, chunk_num);
}

inline u32 updater_get_fw_data_position(u16 chunk_num)
{
    block_transfer_start_par_t *bk_start = &fw_update_srv_proc.block_start_par;
    return get_fw_data_position(bk_start->block_num, fw_update_srv_proc.bk_size_log, chunk_num, bk_start->chunk_size);
}

int is_updater_obj_id_match(u8 *obj_id)
{
    return is_obj_id_match(fw_update_srv_proc.obj_id, obj_id);
}

void fw_update_srv_proc_init_keep_id(fw_cid_fwid_t *id)
{
    u8 obj_id_backup[8];
    memcpy(obj_id_backup, &fw_update_srv_proc.obj_id, sizeof(obj_id_backup));
    
    memset(&fw_update_srv_proc, 0, sizeof(fw_update_srv_proc));
    
    memcpy(&fw_update_srv_proc.id, id, sizeof(fw_update_srv_proc.id)); // don't clear to handle retransmit here 
    memcpy(fw_update_srv_proc.obj_id, obj_id_backup, sizeof(fw_update_srv_proc.obj_id)); // don't clear to handle retransmit here 
}

void obj_block_erase(u16 block_num)
{
    // attention: block size may not integral multiple of 4K,
}

//---------
int mesh_cmd_sig_fw_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	fw_info_status_t rsp = {{0}};
	rsp.id.cid = cps_cid;
	#if DEBUG_EVB_EN
	rsp.id.fw_id = 0xff000020;
	#else
	rsp.id.fw_id = fw_id_local;
	#endif
	
	return mesh_tx_cmd_rsp(FW_INFO_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_fw_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

// -------
int mesh_tx_cmd_fw_update_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st, fw_cid_fwid_t *p_id)
{
	fw_update_status_t rsp = {0};
	rsp.st = st;
    rsp.phase = fw_update_srv_proc.phase;
    if(UPDATE_PHASE_APPLY_OK == rsp.phase){
        rsp.phase = UPDATE_PHASE_IDLE;
    }
    rsp.additional_info = fw_update_srv_proc.additional_info;
	memcpy(&rsp.id, p_id,sizeof(rsp.id));
	memcpy(&rsp.obj_id, fw_update_srv_proc.obj_id,sizeof(rsp.obj_id));
	u32 rsp_len = sizeof(fw_update_status_t);
	if(!((UPDATE_ST_SUCCESS == st)/* && ((UPDATE_PHASE_PREPARE == rsp.phase)
	                                ||(UPDATE_PHASE_IN_PROGRESS == rsp.phase)
	                                ||(UPDATE_PHASE_DUF_READY == rsp.phase))*/)){
	    rsp_len -= 8;   // no obj_id
	}

	return mesh_tx_cmd_rsp(FW_UPDATE_STATUS, (u8 *)&rsp, rsp_len, ele_adr, dst_adr, 0, 0);
}

int mesh_fw_update_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st, fw_cid_fwid_t *p_id)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_fw_update_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st, p_id);
}

int mesh_cmd_sig_fw_update_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    fw_update_get_t *p_get = (fw_update_get_t *)par;
	u8 st = UPDATE_ST_SUCCESS;
	if(fw_update_srv_proc.busy){
	    if(is_cid_fwid_match(&fw_update_srv_proc.id, &p_get->id)){
	        if(fw_update_srv_proc.obj_block_trans_num_next == updater_get_fw_block_cnt()){// all block rx ok
	            fw_update_srv_proc.phase = UPDATE_PHASE_DUF_READY;
	        }
	        st = UPDATE_ST_SUCCESS;
	    }else{
	        st = UPDATE_ST_BUSY_WITH_DIFF_OBJ;
	    }
	}else{
	    st = UPDATE_ST_ID_COMBINATION_WRONG;
	}
	return mesh_fw_update_st_rsp(cb_par, st, &p_get->id);
}

int mesh_ota_slave_need_ota(fw_update_prepare_t *p_prepare)
{
    return 1;   // TODO
}

int mesh_cmd_sig_fw_update_prepare(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    fw_update_prepare_t *p_prepare = (fw_update_prepare_t *)par;
	u8 st = UPDATE_ST_ID_COMBINATION_WRONG;
	if(!is_mesh_ota_cid_match(p_prepare->id.cid)){
	    st = UPDATE_ST_ID_COMBINATION_WRONG;
	}else if(fw_update_srv_proc.busy){
	    if(is_cid_fwid_match(&fw_update_srv_proc.id, &p_prepare->id)
	    && is_updater_obj_id_match(p_prepare->obj_id)){
	        st = UPDATE_ST_SUCCESS;  // retransmit
	    }else{
	        st = UPDATE_ST_BUSY_WITH_DIFF_OBJ;
	    }
	}else{
	    if(mesh_ota_slave_need_ota(p_prepare)){
            #if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
            dual_mode_disable();
            // bls_ota_clearNewFwDataArea(); // may disconnect
            #endif
    	    fw_update_srv_proc_init_keep_id(&p_prepare->id);
            memcpy(fw_update_srv_proc.obj_id, p_prepare->obj_id, sizeof(fw_update_srv_proc.obj_id));
    	    fw_update_srv_proc.phase = UPDATE_PHASE_PREPARE;
    	    fw_update_srv_proc.busy = 1;
    	    st = UPDATE_ST_SUCCESS;
	    }else{
    	    st = UPDATE_ST_ID_COMBINATION_WRONG;
	    }
	}
	return mesh_fw_update_st_rsp(cb_par, st, &p_prepare->id);
}

int mesh_cmd_sig_fw_update_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 st;
    fw_update_start_t *p_start = (fw_update_start_t *)par;
    if(fw_update_srv_proc.busy){
        if(is_cid_fwid_match(&fw_update_srv_proc.id, &p_start->id)){
            if(UPDATE_PHASE_PREPARE == fw_update_srv_proc.phase){
                fw_update_srv_proc.policy = p_start->policy;
            	fw_update_srv_proc.phase = UPDATE_PHASE_IN_PROGRESS;
            }
        	st = UPDATE_ST_SUCCESS;
    	}else{
    	    st = UPDATE_ST_BUSY_WITH_DIFF_OBJ;
    	}
    }else{       
	    st = UPDATE_ST_ID_COMBINATION_WRONG;    // not receive prepare before
	}
	
	return mesh_fw_update_st_rsp(cb_par, st, &p_start->id);
}

int mesh_cmd_sig_fw_update_control(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 st = UPDATE_ST_ID_COMBINATION_WRONG;
    fw_update_control_t *p_control = (fw_update_control_t *)par;
    int id_match = is_cid_fwid_match(&fw_update_srv_proc.id, &p_control->id);
    if(fw_update_srv_proc.busy){
        if(id_match){
            if(FW_UPDATE_ABORT == cb_par->op){
                fw_update_srv_proc_init_keep_id(&p_control->id);
                // fw_update_srv_proc.busy = 0;
                mesh_ota_reboot_set(OTA_DATA_CRC_ERR);
                st = UPDATE_ST_SUCCESS;
            }else if(FW_UPDATE_APPLY == cb_par->op){
                if((UPDATE_PHASE_DUF_READY == fw_update_srv_proc.phase)
                || (UPDATE_PHASE_APPLY_OK == fw_update_srv_proc.phase)){
                    if(is_valid_mesh_ota_calibrate_val()){
                        #if DISTRIBUTOR_UPDATE_CLIENT_EN
						#if VC_APP_ENABLE
                        fw_ota_data_rx[8] = fw_update_srv_proc.reboot_flag_backup;
                        new_fw_write_file(fw_ota_data_rx, fw_update_srv_proc.obj_size);
						#endif
						#else
                        mesh_ota_reboot_set((fw_update_srv_proc.obj_size > 256) ? OTA_SUCCESS : OTA_SUCCESS_DEBUG);
                        #endif
                        st = UPDATE_ST_SUCCESS;
                    }else{
                        mesh_ota_reboot_set(OTA_DATA_CRC_ERR);
                        st = UPDATE_ST_ID_COMBINATION_WRONG;    // comfirm later
                    }
                    fw_update_srv_proc_init_keep_id(&p_control->id);
                    fw_update_srv_proc.phase = UPDATE_PHASE_APPLY_OK;
                }else{
                    st = UPDATE_ST_ID_COMBINATION_APPLY_FAIL;
                }
            }
    	}else{
    	    st = UPDATE_ST_BUSY_WITH_DIFF_OBJ;
    	}
    }else{
	    st = id_match ? UPDATE_ST_SUCCESS : UPDATE_ST_ID_COMBINATION_WRONG;    // retransmit or not receive prepare before
	    mesh_ota_reboot_check_refresh();
	}
	
	return mesh_fw_update_st_rsp(cb_par, st, &p_control->id);
}

int mesh_cmd_sig_fw_update_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//------
int mesh_tx_cmd_obj_transfer_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st)
{
	obj_transfer_status_t rsp = {0};
	rsp.st = st;
	memcpy(&rsp.obj_id, &fw_update_srv_proc.obj_id, sizeof(rsp.obj_id));
	rsp.obj_size = fw_update_srv_proc.obj_size;
	rsp.bk_size_log = fw_update_srv_proc.bk_size_log;

	return mesh_tx_cmd_rsp(OBJ_TRANSFER_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, 0, 0);
}

int mesh_obj_transfer_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_obj_transfer_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st);
}

int mesh_cmd_sig_obj_transfer_handle(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 st = OBJ_TRANS_ST_BUSY_WITH_DIFF;
    obj_transfer_start_t *p_start = (obj_transfer_start_t *)par;
    if(fw_update_srv_proc.busy){
        if(UPDATE_PHASE_IN_PROGRESS == fw_update_srv_proc.phase){
            if(is_updater_obj_id_match(p_start->obj_id)){
                if(OBJ_TRANSFER_GET == cb_par->op){
                    st = fw_update_srv_proc.obj_trans_busy ? OBJ_TRANS_ST_BUSY : OBJ_TRANS_ST_READY;
                }else if(OBJ_TRANSFER_START == cb_par->op){
                    if((p_start->obj_size <= MESH_OTA_OBJ_SIZE_MAX)
                    && (p_start->bk_size_log >= MESH_OTA_BLOCK_SIZE_LOG_MIN)
                    && (p_start->bk_size_log <= MESH_OTA_BLOCK_SIZE_LOG_MAX)){
                        fw_update_srv_proc.obj_size = p_start->obj_size;
                        fw_update_srv_proc.bk_size_log = p_start->bk_size_log;
                        // fw_update_srv_proc.obj_block_trans_num_next = 0;    // init, no need, because continue OTA
                        fw_update_srv_proc.obj_trans_busy = 1;
                        st = OBJ_TRANS_ST_BUSY;
                    }else{
                        st = OBJ_TRANS_ST_TOO_BIG;
                    }
                }else if(OBJ_TRANSFER_ABORT == cb_par->op){
                    fw_update_srv_proc.obj_trans_busy = 0;
                    st = OBJ_TRANS_ST_READY;
                }
            }else{
                st = OBJ_TRANS_ST_BUSY_WITH_DIFF;
            }
    	}else{
    	    st = OBJ_TRANS_ST_BUSY_WITH_DIFF;    // TODO
    	}
    }else{       
	    st = OBJ_TRANS_ST_BUSY_WITH_DIFF;    // TODO
	}
	
	return mesh_obj_transfer_st_rsp(cb_par, st);
}

int mesh_cmd_sig_obj_transfer_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//------
int mesh_tx_cmd_obj_block_transfer_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 st)
{
	obj_block_transfer_status_t rsp = {0};
	rsp.st = st;
	return mesh_tx_cmd_rsp(OBJ_BLOCK_TRANSFER_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, 0, 0);
}

int mesh_obj_block_transfer_st_rsp(mesh_cb_fun_par_t *cb_par, u8 st)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_obj_block_transfer_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, st);
}

u8 obj_block_transfer_start_par_check(obj_block_transfer_start_t *p_start)
{
	u8 st;// = OBJ_BLOCK_TRANS_ST_REJECT;
    if(fw_update_srv_proc.busy){
        if((UPDATE_PHASE_IN_PROGRESS == fw_update_srv_proc.phase)
        && fw_update_srv_proc.obj_trans_busy && is_updater_obj_id_match(p_start->obj_id)){
            u16 bk_size = (1 << fw_update_srv_proc.bk_size_log);
            if(p_start->par.bk_size_current <= bk_size){
                if(get_fw_chunk_cnt(bk_size, p_start->par.chunk_size) <= MESH_OTA_CHUNK_NUM_MAX){ // TODO
                    if(p_start->par.bk_check_sum_type == OBJ_BLOCK_CHECK_SUM_TYPE_CRC32){
                        if(p_start->par.block_num == fw_update_srv_proc.obj_block_trans_num_next){
                            st = OBJ_BLOCK_TRANS_ST_ACCEPTED;
                        }else if(p_start->par.block_num < fw_update_srv_proc.obj_block_trans_num_next){
                            st = OBJ_BLOCK_TRANS_ST_ALREADY_RX;
                        }else{
                            st = OBJ_BLOCK_TRANS_ST_INVALID_BK_NUM;
                        }
                    }else{
                        st = OBJ_BLOCK_TRANS_ST_UNKNOWN_CHECK_SUM_TYPE;
                    }
                }else{
                    st = OBJ_BLOCK_TRANS_ST_WRONG_CHUNK_SIZE;
                }
            }else{
                st = OBJ_BLOCK_TRANS_ST_WRONG_BK_SIZE_CURRENT;
            }
    	}else{
    	    st = OBJ_BLOCK_TRANS_ST_REJECT;    // TODO
    	}
    }else{       
	    st = OBJ_BLOCK_TRANS_ST_REJECT;    // TODO
	}

	return st;
}

int mesh_cmd_sig_obj_block_transfer_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    obj_block_transfer_start_t start;
    memcpy(&start, par, sizeof(start));
    obj_block_transfer_start_t *p_start = &start;
    if(par_len < sizeof(start)){
        p_start->par.bk_size_current = (1 << fw_update_srv_proc.bk_size_log);   // because bk_size_current is optional
    }
    
    u8 st = obj_block_transfer_start_par_check(p_start);
    if(OBJ_BLOCK_TRANS_ST_ACCEPTED == st){
        memcpy(&fw_update_srv_proc.block_start_par, &p_start->par, sizeof(fw_update_srv_proc.block_start_par));
        obj_block_erase(p_start->par.block_num);
        fw_update_srv_proc.miss_mask = BIT_MASK_LEN(updater_get_fw_chunk_cnt());
        fw_update_srv_proc.obj_block_trans_accepted = 1;
    }
	
	return mesh_obj_block_transfer_st_rsp(cb_par, st);
}

int mesh_cmd_sig_obj_block_transfer_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//------
int is_obj_chunk_transfer_ready()
{
    return (fw_update_srv_proc.busy && (UPDATE_PHASE_IN_PROGRESS == fw_update_srv_proc.phase)
            && fw_update_srv_proc.obj_trans_busy && fw_update_srv_proc.obj_block_trans_accepted);
}

int mesh_cmd_sig_obj_chunk_transfer(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    obj_chunk_transfer_t *p_chunk = (obj_chunk_transfer_t *)par;
    int fw_data_len = par_len - 2;
    
    if(is_obj_chunk_transfer_ready() && ((fw_data_len > 0) && (fw_data_len <= MESH_OTA_CHUNK_SIZE))){
        if(fw_update_srv_proc.miss_mask){
            u32 bit_chunk = BIT(p_chunk->chunk_num);

            #if 1 // VC_DISTRIBUTOR_UPDATE_CLIENT_EN
            #if 0   // test
            static u8 skip_test = 1;
            if(skip_test && (p_chunk->chunk_num == 1)){
                skip_test = 0;
                return 0;
            }
            #endif
            
            if(fw_update_srv_proc.miss_mask & bit_chunk){
                fw_update_srv_proc.miss_mask &= ~bit_chunk;
                u32 pos = updater_get_fw_data_position(p_chunk->chunk_num);
                mesh_ota_save_data(pos, fw_data_len, p_chunk->data);
                fw_update_srv_proc.bin_crc_done = 0;
            }
            #endif
        }else{
        }
    }
    return 0;
}

//------
int block_crc32_check_current(u32 check_val)
{
    u32 adr = updater_get_fw_data_position(0);
    u32 crc_type1_blk = 0;
    if(check_val == soft_crc32_ota_flash(adr, fw_update_srv_proc.block_start_par.bk_size_current, 0,&crc_type1_blk)){
        // for telink mesh crc
        u8 *mask = fw_update_srv_proc.blk_crc_tlk_mask;
        u16 blk_num = fw_update_srv_proc.block_start_par.block_num; // have make sure blk_num is valid before.
        if(!is_array_mask_en(mask, blk_num)){
            fw_update_srv_proc.crc_total += crc_type1_blk;
            set_array_mask_en(mask, blk_num);
        }
        
        return 1;
    }
    return 0;
}

int mesh_cmd_sig_obj_block_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    obj_block_get_t *p_get = (obj_block_get_t *)par;
    u32 miss_cnt = 0;
	obj_block_status_t rsp = {0};
	u8 st;
	if(is_obj_chunk_transfer_ready() && is_updater_obj_id_match(p_get->obj_id)){
	    if(p_get->block_num == fw_update_srv_proc.obj_block_trans_num_next){
	        // TODO fill missing chunk
	        if(fw_update_srv_proc.miss_mask){
	            foreach(i,ARRAY_SIZE(rsp.miss_chunk)){
	                if(fw_update_srv_proc.miss_mask & BIT(i)){
	                    rsp.miss_chunk[miss_cnt++] = i;
	                }
	            }
                st = OBJ_BLOCK_ST_NOT_ALL_CHUNK_RX;
            }else{                
                if(block_crc32_check_current(fw_update_srv_proc.block_start_par.bk_check_sum_val)){  // SIG block crc32 ok
                    st = OBJ_BLOCK_ST_ALL_CHUNK_RX;
                    fw_update_srv_proc.obj_block_trans_num_next++;  // receive ok
                }else{
                    st = OBJ_BLOCK_ST_WRONG_CHECK_SUM;
                }
            }
	    }else if(p_get->block_num < fw_update_srv_proc.obj_block_trans_num_next){
            st = OBJ_BLOCK_ST_ALL_CHUNK_RX;
	    }else{
            st = OBJ_BLOCK_ST_WRONG_BLOCK;
	    }
	}else{
	    st = OBJ_BLOCK_ST_WRONG_OBJ_ID;
	}
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	rsp.st = st;
	
	return mesh_tx_cmd_rsp(OBJ_BLOCK_STATUS, (u8 *)&rsp, 1 + miss_cnt*2, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_obj_block_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

//------
int mesh_cmd_sig_obj_info_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	obj_info_status_t rsp = {0};
	rsp.bk_size_log_min = MESH_OTA_BLOCK_SIZE_LOG_MIN;
	rsp.bk_size_log_max = MESH_OTA_BLOCK_SIZE_LOG_MAX;
	rsp.chunk_num_max = MESH_OTA_CHUNK_NUM_MAX;
	
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_rsp(OBJ_INFO_STATUS, (u8 *)&rsp, sizeof(rsp), p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_obj_info_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
#endif
#else
void mesh_ota_master_proc(){}
#endif

