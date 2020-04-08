/********************************************************************************************************
 * @file     lighting_model.h 
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


typedef struct{
	u8 mac[6];
	u16 ele_addr;
}mac_addr_set_t;

typedef struct{
	u16 pid;
	u16 ele_addr;
}mac_addr_get_t;
extern my_fifo_t fast_prov_mac_fifo;

void mesh_device_key_set_default();
void mesh_fast_prov_start(u16 pid);
void mesh_fast_prov_rsp_handle(mesh_rc_rsp_t *rsp);
int mesh_fast_prov_sts_set(u8 sts_set);
void mesh_fast_prov_val_init();
int mesh_reset_network(u8 provision_enable);
void mesh_fast_prov_proc();
void mesh_fast_prov_reliable_finish_handle();
int is_fast_prov_mode();
int mesh_fast_prov_sts_get();
int mesh_fast_prov_add_mac_to_buf(mesh_rc_rsp_t *rsp);

int cb_vd_mesh_reset_network(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_get_addr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_get_addr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_set_addr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_set_provision_data(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_provision_confirm(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_provision_complete(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

int cb_vd_mesh_addr_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_primary_addr_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_provison_data_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int cb_vd_mesh_provision_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
u16 get_win32_prov_unicast_adr();
int set_win32_prov_unicast_adr(u16 adr);
u8 get_win32_ele_cnt(u8 pid);
void mesh_fast_prov_node_info_callback(u8 *dev_key, u16 node_addr, u16 pid);
u8 mesh_fast_prov_get_ele_cnt_callback(u16 pid);

