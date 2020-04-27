/********************************************************************************************************
 * @file     time_model.h 
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
#define TIME_GET		        		0x3782
#define TIME_SET		        		0x5C
#define TIME_STATUS		        		0x5D
#define TIME_ROLE_GET		        	0x3882
#define TIME_ROLE_SET		        	0x3982
#define TIME_ROLE_STATUS		        0x3A82
#define TIME_ZONE_GET		        	0x3B82
#define TIME_ZONE_SET		        	0x3C82
#define TIME_ZONE_STATUS		        0x3D82
#define TAI_UTC_DELTA_GET		        0x3E82
#define TAI_UTC_DELTA_SET		        0x3F82
#define TAI_UTC_DELTA_STATUS		    0x4082

//------op parameters

//------------------vendor op end-------------------

enum{
	TIME_ROLE_NONE		= 0,
	TIME_ROLE_AUTH		= 1,
	TIME_ROLE_RELAY		= 2,
	TIME_ROLE_CLIENT	= 3,
	TIME_ROLE_MAX,
};

typedef struct{
	u32 TAI_sec;     // 32bit is enough for 2000 ~ 2099 year
	u8 TAI_sec_rsv;
	u8 sub_sec;
	u8 uncertainty;
	u16 time_auth		:1;
	u16 TAI_UTC_delta	:15;
	u8 zone_offset;
}time_status_t;

typedef struct{
	time_status_t time;
	u8 zone_offset_new;
	u8 TAI_zone_change[5];
	u16 delta_new		:15;
	u16 rsv				:1;
	u8 TAI_delta_change[5];
	u8 role;
}mesh_time_t;

typedef struct{
	u8 zone_offset_new;
	u8 TAI_zone_change[5];
}mesh_time_zone_set_t;

typedef struct{
	u8 zone_offset_current;
	u8 zone_offset_new;
	u8 TAI_zone_change[5];
}mesh_time_zone_status_t;

typedef struct{
	u16 delta_new		:15;
	u16 rsv				:1;
	u8 TAI_delta_change[5];
}mesh_time_TAI_UTC_delta_set_t;

typedef struct{
	u16 delta_current	:15;
	u16 rsv1			:1;
	u16 delta_new		:15;
	u16 rsv2			:1;
	u8 TAI_delta_change[5];
}mesh_time_TAI_UTC_delta_status_t;


extern mesh_time_t	mesh_time;
extern u32          mesh_time_tick;
#define g_TAI_sec   mesh_time.time.TAI_sec

// internal -----------
#define YEAR_BASE       (2000)
#define WEEK_BASE       (6-1)  // 0 means monday.  2000/01/02: Saturday

#define YEAR_MAX        (YEAR_BASE + 100)   // (2000 -- 2099)
#define TAI_SECOND_MAX  (0xbc191380)        // (2100/01/01 00:00:00)
#define SECOND_ONE_DAY  (60*60*24)

#define OFFSET_1970_2000        (946684800)
//#define OFFSET_1970_2000_EAST8  (946684800 - 8*3600)
#define TIME_ZONE_0     (0x40)

typedef struct{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 week;    // 0 means monday
}mesh_UTC_t;


static inline s16 get_time_zone_offset_min(u8 zone_offset)
{
    return (s16)(((s32)zone_offset - 64)*15);
}

static inline u8 get_time_zone_offset(s32 zone_min) // Positive numbers are eastwards
{
    return (u8)((zone_min/15) + 64);
}

int get_UTC(const u32 second, mesh_UTC_t *UTC);
u32 get_TAI_sec( mesh_UTC_t *UTC );
u32 get_local_TAI();
u8 get_days_one_month(u32 year, u8 month);
u8 get_days_current_month();
void mesh_time_proc();
int mesh_time_set(time_status_t *p_set);
int mesh_time_st_publish(u8 idx);
int mesh_time_role_st_publish(u8 idx);

// -----------
int mesh_cmd_sig_time_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);

#if MD_SERVER_EN
int mesh_cmd_sig_time_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_zone_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_zone_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_TAI_UTC_delta_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_TAI_UTC_delta_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_role_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_role_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_time_get                   (0)
#define mesh_cmd_sig_time_set                   (0)
#define mesh_cmd_sig_time_zone_get              (0)
#define mesh_cmd_sig_time_zone_set              (0)
#define mesh_cmd_sig_time_TAI_UTC_delta_get     (0)
#define mesh_cmd_sig_time_TAI_UTC_delta_set     (0)
#define mesh_cmd_sig_time_role_get              (0)
#define mesh_cmd_sig_time_role_set              (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_time_zone_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_TAI_UTC_delta_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_time_role_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_time_zone_status           (0)
#define mesh_cmd_sig_time_TAI_UTC_delta_status  (0)
#define mesh_cmd_sig_time_role_status           (0)
#endif

// ----------
int access_cmd_time_get(u16 adr, u32 rsp_max);
int access_cmd_time_set(u16 adr, u32 rsp_max, time_status_t *p_set);


