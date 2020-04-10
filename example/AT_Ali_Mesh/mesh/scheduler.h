/********************************************************************************************************
 * @file     scheduler.h 
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
#define SCHD_ACTION_GET		        	0x4882
#define SCHD_ACTION_STATUS		        0x5F
#define SCHD_GET		        		0x4982
#define SCHD_STATUS		        		0x4A82
#define SCHD_ACTION_SET		        	0x60
#define SCHD_ACTION_SET_NOACK		    0x61

//------op parameters

//------------------vendor op end-------------------
enum{
    SCHD_YEAR_ANY = 0x64,
    SCHD_YEAR_MAX,
};

enum{
    SCHD_MONTH_ANY = 0,  // define by Examples of Table 5.14
};

enum{
    SCHD_DAY_ANY = 0,
    SCHD_DAY_MAX = 31,
};

enum{
    SCHD_WEEK_ANY = 0,  // define by Examples of Table 5.14 // SCHD_WEEK_ANY = BIT_MASK_LEN(7),    // just use as input parameter
};

enum{
    SCHD_HOUR_ANY       = 0x18,
    SCHD_HOUR_RANDOM    = 0x19,
    SCHD_HOUR_MAX,
};

enum{
    SCHD_MIN_ANY        = 0x3C,
    SCHD_MIN_EVERY15    = 0x3D, // 0, 15, 30, 45
    SCHD_MIN_EVERY20    = 0x3E, // 0, 20, 40, 
    SCHD_MIN_RANDOM     = 0x3F,
    SCHD_MIN_MAX,
};

enum{
    SCHD_SEC_ANY        = 0x3C,
    SCHD_SEC_EVERY15    = 0x3D, // 0, 15, 30, 45
    SCHD_SEC_EVERY20    = 0x3E, // 0, 20, 40, 
    SCHD_SEC_RANDOM     = 0x3F,
    SCHD_SEC_MAX,
};

enum{
	SCHD_ACTION_OFF     = 0,
	SCHD_ACTION_ON      = 1,
	SCHD_ACTION_SCENE   = 2,
	SCHD_ACTION_NONE    = 0x0F,
	SCHD_ACTION_MAX,
};

enum{
    RANDOM_REBUILD_HOUR = 0,
    RANDOM_REBUILD_MIN,
    RANDOM_REBUILD_SEC,
};

#define NO_NEAREST  (0xff)

typedef struct{
	u32 TAI_local;
	u16 entry_bit[LIGHT_CNT];
}schd_nearest_t;

int schd_random_rebuild_hour();
int schd_random_rebuild_min();
int schd_random_rebuild_sec();
void rebulid_schd_nearest_and_check_event(int rebuild, u32 TAI_local);
void mesh_scheduler_proc();

void test_schd_action_set_off(u16 adr);
void test_schd_action_set_on(u16 adr);
void test_schd_action_set_scene(u16 adr);


// -----------
#if MD_SERVER_EN
int mesh_cmd_sig_scheduler_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_schd_action_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_schd_action_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_scheduler_get              (0)
#define mesh_cmd_sig_schd_action_get            (0)
#define mesh_cmd_sig_schd_action_set            (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_scheduler_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_schd_action_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_scheduler_status           (0)
#define mesh_cmd_sig_schd_action_status         (0)
#endif

// ----------
int access_cmd_schd_get(u16 adr, u32 rsp_max);
int access_cmd_schd_action_get(u16 adr, u32 rsp_max, u8 index);
int access_cmd_schd_action_set(u16 adr, u32 rsp_max, scheduler_t *p_schd, int ack);


