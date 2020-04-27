/********************************************************************************************************
 * @file     mesh_common.h 
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
#include "app/app.h"
// #include "mesh_lpn/app.h"
// #include "mesh_provision/app.h"
// #include "mesh_switch/app.h"
#include "mesh_lpn.h"
#include "mesh_fn.h"
#include "time_model.h"
#include "scheduler.h"
#include "mesh_property.h"

/** @addtogroup Mesh_Common
  * @{
  */
  
/** @defgroup Mesh_Common
  * @brief Mesh Common Code.
  * @{
  */


#ifndef HCI_LOG_FW_EN
#define HCI_LOG_FW_EN   0
#endif

#if SPIRIT_PRIVATE_LPN_EN
#define MESH_RSP_BASE_DELAY_STEP			120  //unit:ADV_INTERVAL_MIN(10ms)
#else
#define MESH_RSP_BASE_DELAY_STEP			18  //unit:ADV_INTERVAL_MIN(10ms)
#endif

#define MESH_POWERUP_BASE_TIME				200

typedef struct{
	u8 len;
	u8 type;            // 0xFF: manufacture data
	u8 mac_adr[6];
	u16 adr_primary;
    u8 rsv_telink[10];  // not for user
    u8 rsv_user[11];
}mesh_scan_rsp_t;

typedef struct ais_pri_data{
	u8 length;
	u8 type;
	u16 cid;
	u8 vid;
	union{
		u8 fmsk;
		struct{
			u8 ble_version:2;//00£ºBLE4.0 01£ºBLE4.2 10£ºBLE5.0 11£ºBLE5.0 above
			u8 ota_support:1;
			u8 authen_en:1;
			u8 secret_type:1;// 0:one device type on key, 1:one device one key
			u8 prov_flag:1;
			u8 rfu:2;
		};
	};
	u32 pid;
	u8 mac[6];
} ais_pri_t;

//---------
extern u8 g_reliable_retry_cnt_def;
extern u16 g_reliable_retry_interval_min;
extern u16 g_reliable_retry_interval_max;
extern u8 pair_login_ok;
extern u8 mesh_need_random_delay;
extern const u8 UART_TX_LEN_MAX;

static inline int is_valid_val_100(u8 val_100)
{
	return (val_100 <= 100);
}

//---------
void mesh_ble_connect_cb(u8 e, u8 *p, int n);
void mesh_ble_disconnect_cb();
void mesh_conn_param_update_req();
int mesh_get_proxy_hci_type();
void vendor_id_check_and_update();
void mesh_global_var_init();
void mesh_tid_save(int ele_idx);
void mesh_vd_init();
void lpn_node_io_init();
void entry_ota_mode(void);
void set_mesh_ota_type();
void set_firmware_type_init();
void set_firmware_type_SIG_mesh();
void set_firmware_type_zb_with_factory_reset();
void set_ota_firmwaresize(int adr);
void ota_set_flag();
void mesh_ota_reboot_set(u8 type);
void mesh_ota_reboot_check_refresh();
void mesh_ota_reboot_proc();
void get_fw_id();
u8 proc_telink_mesh_to_sig_mesh(void);
u8 send_vc_fifo(u8 cmd,u8 *pfifo,u8 cmd_len);
void app_enable_scan_all_device ();
int	app_device_mac_match (u8 *mac, u8 *mask);
int app_l2cap_packet_receive (u16 handle, u8 * raw_pkt);
int chn_conn_update_dispatch(u8 *p);
void sim_tx_cmd_node2node();
int app_advertise_prepare_handler (rf_packet_adv_t * p);
void my_att_init(u8 mode);
void ble_mac_init();
void usb_id_init();
void uart_drv_init();
void adc_drv_init();
int blc_rx_from_uart (void);
int blc_hci_tx_to_uart ();
void mesh_scan_rsp_init();
int SendOpParaDebug(u16 adr_dst, u8 rsp_max, u16 op, u8 *par, int len);
int SendOpParaDebug_vendor(u16 adr_dst, u8 rsp_max, u16 op, u8 *par, int len, u8 rsp_op, u8 tid);
void share_model_sub_by_rx_cmd(u16 op, u16 ele_adr, u16 sub_adr, u16 dst_adr,u8 *uuid, u32 model_id, int sig_model);
void share_model_sub(u16 op, u16 sub_adr, u8 *uuid);
u32 get_all_online_node(u16 *list, u32 max_cnt);
void APP_reset_vendor_id(u16 vd_id);
int mesh_rc_data_layer_access_cb(u8 *params, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_tx_cmd2self_primary(u8 *ac, int len_ac);
u32 get_mesh_pub_interval_ms(u32 model_id, int sig_model, mesh_pub_period_t *period);
void publish_when_powerup();
int is_need_response_to_self(u16 adr_dst, u16 op);

extern u8 gatt_adv_send_flag;
extern u16 g_vendor_id;
extern u32 g_vendor_md_light_vc_s;
extern u32 g_vendor_md_light_vc_s2;
extern u32 g_vendor_md_light_vc_c;
extern u16 publish_powerup_random_ms;

extern u32 fw_id_local;
extern u16 sub_adr_onoff ;
void set_unprov_beacon_para(u8 *p_uuid ,u8 *p_info);
void set_provision_adv_data(u8 *p_uuid,u8 *oob_info);
void set_proxy_adv_data(u8 *p_hash,u8 *p_random);
void bls_set_adv_delay(u8 delay);	// unit : 625us
void bls_set_adv_retry_cnt(u8 cnt); // default :0
void set_random_adv_delay_normal_adv(int en );

void set_sec_req_send_flag(u8 flag);// set the sec req send or not 
ble_sts_t  blc_att_setServerDataPendingTime_upon_ClientCmd(u8 num_10ms);
void reliable_retry_cnt_def_set(u8 retry_cnt);
int mesh_rsp_handle_cb(mesh_rc_rsp_t *p_rsp);
int app_hci_cmd_from_usb (void);
int app_hci_cmd_from_usb_handle (u8 *buff, int n);

int mesh_send_cl_proxy_bv03(u16 node_adr);
int mesh_send_cl_proxy_bv04(u16 node_adr);
int mesh_send_cl_proxy_bv05(u16 node_adr);
int mesh_send_cl_proxy_bv06(u16 node_adr);
int mesh_send_cl_proxy_bv07(u16 node_adr);
u8 get_flash_data_is_valid(u8 *p_data,u16 len);
void erase_ecdh_sector_restore_info(u32 adr,u8 *p_data,u16 len);

#if WIN32
int SendOpParaDebug_VC(u16 adr_dst, u8 rsp_max, u16 op, u8 *par, int len);  // only for SIG command now
#endif
extern u8  mesh_user_define_mode ;

extern u8 PROVISION_ATT_HANDLE; // may use in library
extern u8 GATT_PROXY_HANDLE;    // may use in library
extern const u8 SERVICE_CHANGE_ATT_HANDLE_SLAVE;
extern const u8 ONLINE_ST_ATT_HANDLE_SLAVE;
extern u8 proc_homekit_pair;

enum{
	BLE_4_0 =0,
	BLE_4_2,
	BLE_5_0,
	BLE_5_0_HIGH,
};
typedef struct{
	mesh_page0_head_t cps_head;
	u8 	mac[6];
}simple_flow_dev_uuid_t;



// set dev key struct for gateway 
typedef struct{
    u16 unicast;
    u8 dev_key[16];
}mesh_gw_set_devkey_str;

typedef struct{
    u8 fastbind;
    u8 key_idx[2];
    u8 key[16];
}mesh_gw_appkey_bind_str;

typedef struct{
    u8 eve;
    u16 adr;
    u8 mac[6];
    u8 uuid[16];
}gateway_prov_event_t;
typedef struct{
    u16 op;
    u16 src;
    u8 ac_par[32];
}gateway_upload_mesh_src_t;

// gateway cmd event log part 
typedef struct{
			u16 src;
			u16 dst;
			u16 opcode;
			u8 para[20];
}gateway_upload_mesh_cmd_str;

typedef struct{
	u8 dsk[32];
	u8 dpk[64];
	u32 valid;
	u32 crc;
}mesh_ecdh_key_str;
#define ECDH_KEY_VALID_FLAG		0xa5

#define ECDH_KEY_STS_TWO_VALID	1
#define ECDH_KEY_STS_NONE_VALID	2
#define ECDH_KEY_STS_ONE_VALID	3

void provision_ecc_calc();
void set_dev_uuid_for_simple_flow( u8 *p_uuid);
u8 prov_uuid_fastbind_mode(u8 *p_uuid);
u8 is_homekit_pair_handle(u8 handle); 

u8 ota_condition_enable();
void create_sha256_input_string(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret);
extern u8 model_need_key_bind_whitelist(u16 *key_bind_list_buf,u8 *p_list_cnt,u8 max_cnt);
extern int LogMsgModuleDlg_and_buf(u8 *pbuf,int len,char *log_str,char *format, va_list list);
void mesh_node_prov_event_callback(u8 evt_code);
void wd_clear_lib();
void bls_ota_set_fwSize_and_fwBootAddr(int firmware_size_k, int boot_addr);
void mesh_mi_cfg_segmust_set(material_tx_cmd_t *p);
void mesh_secure_beacon_loop_proc();
u16 mi_share_model_sub(u16 op,u16 ele_adr,u16 sub_adr,u8 *uuid,u32 model_id);
int mesh_cmd_sig_cfg_model_sub_cb(u8 st,mesh_cfg_model_sub_set_t * p_sub_set,int sig_model,u16 adr_src);
void start_reboot(void);
void blc_l2cap_register_pre_handler(void *p);
#if!WIN32
uint32_t soft_crc32_telink(const void *buf, size_t size, uint32_t crc);
#endif
void vendor_md_cb_pub_st_set2ali();


/**
  * @}
  */

/**
  * @}
  */


