/********************************************************************************************************
 * @file     telink_sdk_mible_api.h 
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

#ifndef _TELINK_SDK_MIBLE_API_C_
#define _TELINK_SDK_MIBLE_API_C_
#include "./mijia_ble_api/mible_api.h"
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "vendor/common/user_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "../app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
//#include "proj_lib/mesh_crypto/ecc_dsa.h"
#include "../mesh_common.h"
#include "../ev.h"
#include "./libs/third_party/micro-ecc/uECC.h"
#include "./libs/third_party/micro-ecc/types.h"

#if MI_API_ENABLE
#include "./libs/third_party/mbedtls/sha256_hkdf.h"
#include "./libs/mi_config.h"
#endif
extern attribute_t* gAttributes;


u8 telink_ble_mi_app_event(uint8_t sub_code , uint8_t *p, int n);
u8 telink_ble_mi_event_callback(uint8_t opcode,uint8_t *p);
int telink_ble_mi_event_cb_att(u16 conn, u8 * p);
u8 telink_ble_mi_att_sync(uint8_t att_op,mible_gatts_srv_db_t *p);

u8 telink_write_flash(u32 *p_adr,u8 *p_buf,u8 len );



mible_status_t telink_ble_mi_get_gap_address(uint8_t *p_mac);
mible_status_t telink_ble_mi_adv_start(mible_gap_adv_param_t *p_param);
mible_status_t telink_ble_mi_gap_adv_data_set(uint8_t const * p_data,
        uint8_t dlen, uint8_t const *p_sr_data, uint8_t srdlen);

mible_status_t telink_ble_mi_adv_stop();
mible_status_t telink_ble_mi_update_conn_params(uint16_t conn_handle,mible_gap_conn_param_t conn_params);
mible_status_t telink_ble_mi_gatts_service_init(mible_gatts_db_t *p_server_db);

mible_status_t telink_ble_mi_gatts_value_set(uint16_t srv_handle, uint16_t value_handle,
    uint8_t offset, uint8_t* p_value,
    uint8_t len);
mible_status_t telink_ble_mi_gatts_value_get(uint16_t srv_handle, uint16_t value_handle,
    uint8_t* p_value, uint8_t *p_len);
mible_status_t telink_ble_mi_gatts_notify_or_indicate(uint16_t conn_handle, uint16_t srv_handle,
    uint16_t char_value_handle, uint8_t offset, uint8_t* p_value,
    uint8_t len, uint8_t type);
mible_status_t telink_ble_mi_rand_num_generator(uint8_t* p_buf, uint8_t len);
mible_status_t telink_ble_mi_aes128_encrypt(const uint8_t* key,
    const uint8_t* plaintext, uint8_t plen,
    uint8_t* ciphertext);
mible_status_t telink_record_create(uint16_t record_id, uint8_t len);
mible_status_t telink_record_write(uint16_t record_id, uint8_t* p_data,uint8_t len);
u8 find_record_adr(uint16_t record_id,u32 *p_adr);
mible_status_t telink_record_read(uint16_t record_id, uint8_t* p_data,uint8_t len);
mible_status_t telink_record_delete(uint16_t record_id);

mible_status_t telink_mi_task_post(mible_handler_t handler, void *arg);
mible_status_t telink_mi_task_exec();
mible_status_t telink_mi_timer_create(void** p_timer_id,
    mible_timer_handler timeout_handler,
    mible_timer_mode mode);
mible_status_t telink_mi_timer_delete(void *timer_id);
mible_status_t telink_mi_timer_start(void* timer_id, uint32_t timeout_value,
    void* p_context);
mible_status_t telink_mi_timer_stop(void* timer_id);

void telink_record_eve_cb(uint16_t record_id,mible_status_t sts,mible_arch_event_t eve);
void mi_certify_part_init();
void set_adv_mi_prehandler(rf_packet_adv_t *p);
void advertise_init();
void telink_record_part_init();
uint8_t buf_is_empty_or_not(uint8_t* p_data,uint8_t len);
void telink_gatt_event_loop();
void mi_testboard_init();
void telink_mible_nvm_write(void * p_data, uint32_t length, uint32_t address);
void telink_mible_upgrade_firmware(void);
void telink_mible_ota_start();
void telink_mible_ota_end();
u8 mi_mesh_sleep_time_exceed_adv_iner();
void telink_mi_vendor_init();
void mi_reboot_proc();
unsigned char  mi_ota_is_busy();
u8 telink_record_clean_cpy();
void mi_mesh_state_set(u8 state);
u8 mi_mesh_get_state();
void mi_mesh_sleep_init();
void mi_mesh_lowpower_loop();




u8 mi_api_loop_run();

u8 test_mi_api_part();
#define RECORD_RESERVE_SPACE 	16
#define EV_TIMER_MAX_CNT 	4
typedef struct{
	uint16_t rec_id;
	uint8_t len ;
	uint8_t dat[13];
}telink_record_t;
extern telink_record_t telink_record;
#define RECORD_MAX_LEN 	250
#define RAND_NUM_MAX_LEN 	RECORD_MAX_LEN
extern my_fifo_t mi_task_api_func_t;
typedef struct{
	mible_handler_t cb;
	void *arg;
}mi_task_fun_t;
extern attribute_t* gAttributes;
typedef enum{
	NOTIFY_CMD_CODE =1,
	INDICATION_CMD_CODE =2,
}operation_cmd_enum_t;
typedef enum {
    mi_mesh_unreg       = 0x00,
    mi_mesh_unprov      = 0x01,
    mi_mesh_unconfig    = 0x02,
    mi_mesh_avail       = 0x03
} mi_mesh_stat_t;

#define MI_UNPROV_STATE	0
#define MI_PROVED_STATE 3

#define MI_MESH_PUB_STEP	6
#define MI_MESH_PUB_VAL		2
void mem_pool_init(void);

#endif

