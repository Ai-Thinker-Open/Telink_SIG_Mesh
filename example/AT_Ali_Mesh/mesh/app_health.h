/********************************************************************************************************
 * @file     app_health.h 
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
#ifndef APP_HEALTH_H_
#define APP_HEALTH_H_

#include "mesh/light.h"
#include "proj_lib/sig_mesh/Test_case.h"
#include "proj_lib/sig_mesh/app_mesh.h"

#define TEST_ID_STATIC 0
#define TEST_ID_VENDOR_BLE_ASSIGN	1
#define TEST_ID_VENDOR_USER_DEF		0x80

typedef enum{
	NO_FAULT =0,
	BAT_LOW_WARN,
	BAT_LOW_ERR,
	SUP_VOL_LOW_WARN,
	SUP_VOL_LOW_ERR,
	SUP_VOL_HIGH_WARN,
	SUP_VOL_HIHG_ERR,
	POW_SUP_INTER_WARN,
	POW_SUP_INTER_ERR,
	NO_LOAD_WARN,
	NO_LOAD_ERR,
	OVERLOAD_WARN,
	OVERLOAD_ERR,
	OVERHEAT_WARN,
	OVERHEAT_ERR,
	CONDENSAT_WARN,
	CONDENSAT_ERR,
	VIBRATION_WARN,
	VIBRATION_ERR,
	CONFIG_WARN,
	CONFIG_ERR,
	ELE_NOT_CALBRATE_WARN,
	ELE_NOT_CALBRATE_ERR,
	MEM_WARN,
	MEM_ERR=0x18,
	SELF_TEST_WARN,
	SELF_TEST_ERR,
	INPUT_TOO_LOW_WARN,
	INPUT_TOO_LOW_ERR,
	INPUT_TOO_HIGH_WARN,
	INPUT_TOO_HIGH_ERR,
	INPUT_NO_CHANGE_WARN,
	INPUT_NO_CHANGE_ERR,
	ACTUATOR_BLOCK_WARN,
	ACTUATOR_BLOCK_ERR,
	HOUSING_OPEN_WARN,
	HOUSING_OPEN_ERR,
	TAMPER_WARN,
	TAMPER_ERR,
	DEVICE_MOVED_WARN,
	DEVICE_MOVED_ERR,
	DEVICE_DROP_WARN,
	DEVICE_DROP_ERR,
	OVER_FLOW_WARN,
	OVER_FLOW_ERR,
	EMPTY_WARN,
	EMPTY_ERR,
	INTERNAL_BUS_WARN,
	INTERNAL_BUS_ERR,
	MECHANISM_WARN,
	MECHANISM_ERR=0x32,

	VENDOR_SPEC_WARN=0x80,
	
}mesh_fault_array_code_e;


typedef struct{
	u16 company_id;
}mesh_health_fault_get_t;
typedef struct{
	u16 company_id;
}mesh_health_clear_t;
typedef struct{
	u16 company_id;
}mesh_health_clear_unrel_t;
typedef struct{
	u16 company_id;
}mesh_health_clear_rel_t;
typedef struct{
	u8 test_id;
	u16 company_id;
}mesh_health_fault_test_t;
typedef struct{
	u8 test_id;
	u16 company_id;
}mesh_health_fault_status_unrel_t;

extern u8 init_health_para();
extern u8 mesh_health_poll_100ms();

extern int mesh_cmd_sig_health_cur_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_health_fault_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

extern int mesh_cmd_sig_health_cur_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_health_fault_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_fault_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_fault_clr(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_fault_clr_unrel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_fault_test(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_fault_test_unrel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_period_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_period_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_period_set_unrel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_period_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_attention_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_attention_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_attention_set_unrel(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
extern int mesh_cmd_sig_attention_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);


#endif 
