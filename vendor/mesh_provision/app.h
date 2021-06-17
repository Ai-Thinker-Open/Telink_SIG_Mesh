/********************************************************************************************************
 * @file     app.h 
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
u8 gateway_cmd_from_host_ctl(u8 *p, u16 len );
u8 gateway_upload_mac_address(u8 *p_mac,u8 *p_adv);
void set_gateway_provision_sts(unsigned char en);
unsigned char get_gateway_provisison_sts();
u8 gateway_upload_provision_suc_event(u8 evt,u16 adr,u8 *p_mac,u8 *p_uuid);
void set_gateway_provision_para_init();
u8 gateway_upload_keybind_event(u8 evt);
u8 gateway_upload_provision_self_sts(u8 sts);
u8 gateway_upload_node_ele_cnt(u8 ele_cnt);
u8 gateway_upload_node_info(u16 unicast);
u8 gateway_upload_mesh_ota_sts(u8 *p_dat,int len);
u8 gateway_upload_mesh_sno_val();
u8 gateway_upload_cmd_rsp(u8 *p_rsp,u8 len);

u8 gateway_cmd_from_host_ota(u8 *p, u16 len );
u8 gateway_cmd_from_host_mesh_ota(u8 *p, u16 len );
u8 gateway_upload_ividx(u8 *p_ivi);
u8 gateway_upload_mesh_src_cmd(u16 op,u16 src,u8 *p_ac_par);
u8 gateway_upload_prov_cmd(u8 *p_cmd,u8 len);
u8 gateway_upload_prov_rsp_cmd(u8 *p_rsp,u8 len);
u8 gateway_upload_prov_cmd(u8 *p_cmd,u8 cmd);
u8 gateway_upload_prov_rsp_cmd(u8 *p_rsp,u8 cmd);
u8 gateway_upload_prov_link_cls(u8 *p_rsp,u8 len);
u8 gateway_upload_prov_link_open(u8 *p_cmd,u8 len);
u8 gateway_upload_log_info(u8 *p_buf,u8 len ,char *format,...); //gateway upload the print info to the vc

int mesh_tx_comm_cmd(u16 adr);
void mesh_ota_comm_test();
extern u32 comm_send_flag ;
extern u32 comm_send_tick ;