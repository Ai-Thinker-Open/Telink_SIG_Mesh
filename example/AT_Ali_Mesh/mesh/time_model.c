/********************************************************************************************************
 * @file     time_model.c 
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
#ifndef WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "lighting_model.h"
#include "time_model.h"

#define PTS_TEST_TIME_EN    0

static inline u8 is_leap_year(const u32 year){
    #if 0
    return (year%4==0); // because just calculate 2000 to 2099
    #else
    return ((year%4==0 && year%100!=0) || year%400==0);
    #endif
}

#define DAYS_400YEAR            ((u32)(400*365 + 400/4 - 4 + 1))   // = 146103
#define DAYS_100YEAR(leap)      ((u32)(100*365 + 100/4 - (leap ? 0 : 1)))       // = 36524 for no leap400
#define DAYS_4YEAR(leap)        ((u32)(4*365 + (leap ? 1 : 0)))                 // = 1461
#define DAYS_1YEAR(leap)        ((u32)(1*365 + (leap ? 1 : 0)))
const u8 days_by_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static inline u8 get_days_by_month(int leap_year, u8 month){
    if(leap_year && (2==month)){
        return 29;
    }else{
        return days_by_month[(month-1)];
    }
}

u8 get_days_one_month(u32 year, u8 month)
{
    return get_days_by_month(is_leap_year(year), month);
}

int is_valid_UTC(mesh_UTC_t *UTC)
{
    u8 days = get_days_by_month(is_leap_year(UTC->year), UTC->month);
    
    if((UTC->year < YEAR_BASE || UTC->year >= YEAR_MAX )
      ||(!UTC->month || (UTC->month > 12))
      ||(!UTC->day || (UTC->day > days))
      ||(UTC->hour >= 24)
      ||(UTC->minute >= 60)
      ||(UTC->second >= 60)){
        return 0;
    }

    return 1;
}

int is_valid_TAI_second(u32 second)
{
    return (second && second < TAI_SECOND_MAX);
}

static inline u8 get_week(mesh_UTC_t *UTC){
    u32 y = UTC->year;
    u32 m = UTC->month;
    u32 d = UTC->day;
    if((1 == m) || (2 == m)){
        y = y - 1;
        m = m + 12;
    }
    //monday is 0
    u8 week = ((d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)) % 7;
    return week;
}

int get_UTC(const u32 second, mesh_UTC_t *UTC)
{
    u32 count_days = (u32)(second / (3600*24));
    u32 y = YEAR_BASE, m = 1;
    u32 w = ((count_days + WEEK_BASE) % 7);

    #if 1
    int leap_100_flag = 1;  // leap year for 2000
    if(count_days >= DAYS_400YEAR){
        y+= (count_days / DAYS_400YEAR) * 400;
        count_days %= DAYS_400YEAR;
    }

    if(count_days >= DAYS_100YEAR(1)){
        u32 temp = (count_days - 1);
        y += (temp / DAYS_100YEAR(0)) * 100;
        count_days = temp % DAYS_100YEAR(0);
        leap_100_flag = 0;
    }
    
    int flag_0_3year = 1;
    if(count_days >=  DAYS_4YEAR(leap_100_flag)){
        u32 temp = (count_days + (leap_100_flag ? 0 : 1));
        y += (temp / DAYS_4YEAR(1)) * 4;
        count_days = (temp) % DAYS_4YEAR(1);
        flag_0_3year = 0;
    }

    int leap_4 = (leap_100_flag || (!flag_0_3year));
    if(count_days >= DAYS_1YEAR(leap_4)){
        if(leap_4){
            y++;
            count_days -= DAYS_1YEAR(1);
        }
        if(count_days >= DAYS_1YEAR(0)){
            y += (count_days / DAYS_1YEAR(0)) * 1;
            count_days = count_days % DAYS_1YEAR(0);
        }
    }
    
    while(count_days){
        u32 days_one_month = get_days_by_month(is_leap_year(y), m);
        if(count_days >= days_one_month){
            count_days -= days_one_month;
            m++;
        }else{
            break;
        }
    }

    //if(y < YEAR_MAX){
        UTC->year = (u16)y;
        UTC->month = (u8)(m);
        UTC->day = count_days + 1;
        UTC->hour = (second / 3600 % 24);
        UTC->minute = (second / 60 % 60);
        UTC->second = (second % 60);
        UTC->week = w;
        return 0;
    //}
    return -1;    
    
    #else
    for(y = YEAR_BASE ; y < YEAR_MAX; y++){
        u32 Leap_Year = is_leap_year(y);
        u32 days_one_year = 365 + Leap_Year;
        if(count_days >= days_one_year){
            count_days -= days_one_year;
        }else{
            for(m = 1 ; m <= 12; m++){
                u32 days_one_month = get_days_by_month(Leap_Year, m);
                if(count_days >= days_one_month){
                    count_days -= days_one_month;
                }else{
                    UTC->year = (u16)y;
                    UTC->month = (u8)(m);
                    UTC->day = count_days + 1;
                    UTC->hour = (second / 3600 % 24);
                    UTC->minute = (second / 60 % 60);
                    UTC->second = (second % 60);
                    UTC->week = w;
                    return 0;
                }
            }
        }
    }
    
    return -1;
    #endif
}

STATIC_ASSERT(YEAR_BASE == 2000);
/*******************************************************************
 */
u32 get_TAI_sec( mesh_UTC_t *UTC )
{
    u16 curyear;                    // current year
    u32 cday = 0;                   // total days from base year
    u32 curmonthday = 0;            // current month days

    curyear= UTC->year;
    if(!is_valid_UTC(UTC)){
        return 0;
    }
    
    u32 cyear= curyear - YEAR_BASE;
    #if 1
    u32 temp = 0;
    int leap_100_flag = 1;      // leap year for 2000
    if(cyear >= 400){
        temp = (cyear / 400);
        cyear -= temp * 400;
        cday += temp * DAYS_400YEAR;
    }

    if(cyear >= 100){
        temp = (cyear / 100);
        cyear -= temp * 100;
        cday += temp * DAYS_100YEAR(0) + 1;
        leap_100_flag = 0;
    }

    int flag_0_3year = 1;
    if(cyear >= 4){
        temp = (cyear / 4);
        cyear -= temp * 4;
        cday += temp * DAYS_4YEAR(1) - (leap_100_flag ? 0 : 1);
        flag_0_3year = 0;
    }
    
    if(cyear > 0){
        cday += (cyear)*DAYS_1YEAR(0) + 1 - ((!leap_100_flag && flag_0_3year) ? 1 : 0);
        cyear = 0;
    }
    #else
    u32 cnt_100year = 0;
    
    while(cyear){
        if(cyear >= 400){
            cyear -= 400;
            cday += DAYS_400YEAR;
        }else if(cyear >= 100){
            cyear -= 100;
            cday += DAYS_100YEAR(0 == (cnt_100year++ % 4));
        }else{
            cday += (cyear/4)*(DAYS_4YEAR(1));
            int cyear_mod = cyear%4;
            if(cyear_mod > 0){
                cday += (cyear_mod)*365 + 1;
            }
            cyear = 0;
            break;
        }
    }
    #endif

    // no days for current month
    switch(UTC->month ){
        case  2:  curmonthday = 31;  break;
        case  3:  curmonthday = 59;  break;
        case  4:  curmonthday = 90;  break;
        case  5:  curmonthday = 120; break;
        case  6:  curmonthday = 151; break;
        case  7:  curmonthday = 181; break;
        case  8:  curmonthday = 212; break;
        case  9:  curmonthday = 243; break;
        case 10:  curmonthday = 273; break;
        case 11:  curmonthday = 304; break;
        case 12:  curmonthday = 334; break;
        default:  curmonthday = 0;   break;
    }
    if(is_leap_year(curyear) && (UTC->month >= 3)){
        curmonthday+= 1;
    }
    cday += curmonthday; 
    cday += (UTC->day-1);	//no days for current day

    return (((cday*24+UTC->hour)*60+UTC->minute)*60+UTC->second);  // TAI second
}

#if (STRUCT_MD_TIME_SCHEDULE_EN)
model_time_schedule_t	model_sig_time_schedule;
u32 mesh_md_time_schedule_addr = FLASH_ADR_MD_TIME_SCHEDULE;
#endif

#if (MD_TIME_EN)
#if MD_SERVER_EN
mesh_time_t	mesh_time = {{0}};
u32 mesh_time_tick = 0;

int mesh_time_set(time_status_t *p_set)
{
    #if 1 //(!PTS_TEST_TIME_EN)   // beacuse PTS send test command should be 40bit in BV 01
    if(PTS_TEST_EN || (((0 == p_set->TAI_sec_rsv) || p_set->time_auth) && is_valid_TAI_second(p_set->TAI_sec)))
    #endif
    {
        memcpy(&mesh_time.time, p_set, sizeof(time_status_t));  // include g_TAI_sec = xxxx
        mesh_time_tick = clock_time();
        
        rebulid_schd_nearest_and_check_event(1, get_local_TAI());
        return 0;
    }
    return -1;
}

u32 get_local_TAI()
{
    return (g_TAI_sec + get_time_zone_offset_min(mesh_time.time.zone_offset)*60);
}

#define MESH_TIME_CHECK_INTERVAL    (1 * CLOCK_SYS_CLOCK_1S)

void mesh_time_proc()
{
    if(!g_TAI_sec){
        return ;
    }

    u32 clock_tmp = clock_time();
    u32 t_delta = (u32)(clock_tmp - mesh_time_tick);    // should be differrent from system_time_tick_
    if(t_delta >= MESH_TIME_CHECK_INTERVAL){
        u32 interval_cnt = t_delta / MESH_TIME_CHECK_INTERVAL;
        foreach(i,interval_cnt){
            g_TAI_sec++;
            
            #if (MD_SCHEDULE_EN)
            mesh_scheduler_proc();
            #endif
        }

        mesh_time_tick += interval_cnt * MESH_TIME_CHECK_INTERVAL;
    }
}

/*
	model command callback function ----------------
*/	

//--
int mesh_tx_cmd_time_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	time_status_t rsp = {0};
	u32 len_rsp = 5;
	if(g_TAI_sec || (PTS_TEST_EN && mesh_time.time.TAI_sec_rsv)){
		memcpy(&rsp, &mesh_time.time, sizeof(rsp));
		len_rsp = sizeof(rsp);
	}
	return mesh_tx_cmd_rsp(TIME_STATUS, (u8 *)&rsp, len_rsp, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_time_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_time_schedule.time_srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_time_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_time_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_time_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_time_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_time_st_rsp(cb_par);
}

int mesh_cmd_sig_time_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
    mesh_time_set((time_status_t *)par);
    
	mesh_time_st_rsp(cb_par);
	
    return err;
}

//--
int mesh_tx_cmd_time_zone_st(u8 idx, u16 ele_adr, u16 dst_adr)
{
	mesh_time_zone_status_t rsp = {0};
	rsp.zone_offset_current = mesh_time.time.zone_offset;
	rsp.zone_offset_new = mesh_time.zone_offset_new;
	
	memcpy(rsp.TAI_zone_change, mesh_time.TAI_zone_change, 5);
	return mesh_tx_cmd_rsp(TIME_ZONE_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, 0, 0);
}

int mesh_time_zone_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_time_zone_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src);
}

int mesh_cmd_sig_time_zone_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_time_zone_st_rsp(cb_par);
}

int mesh_cmd_sig_time_zone_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
    mesh_time_zone_set_t *p_set = (mesh_time_zone_set_t *)par;

	mesh_time.zone_offset_new = p_set->zone_offset_new;
	memcpy(mesh_time.TAI_zone_change, p_set->TAI_zone_change, 5);
    
	mesh_time_zone_st_rsp(cb_par);
	mesh_time.time.zone_offset = p_set->zone_offset_new;	// set later
	
    return err;
}

//--
int mesh_tx_cmd_time_TAI_UTC_delta_st(u8 idx, u16 ele_adr, u16 dst_adr)
{
	mesh_time_TAI_UTC_delta_status_t rsp = {0};
	rsp.delta_current = mesh_time.time.TAI_UTC_delta;
	rsp.delta_new = mesh_time.delta_new;
	
	memcpy(rsp.TAI_delta_change, mesh_time.TAI_delta_change, 5);
	return mesh_tx_cmd_rsp(TAI_UTC_DELTA_STATUS, (u8 *)&rsp, sizeof(rsp), ele_adr, dst_adr, 0, 0);
}

int mesh_time_TAI_UTC_delta_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_time_TAI_UTC_delta_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src);
}

int mesh_cmd_sig_time_TAI_UTC_delta_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_time_TAI_UTC_delta_st_rsp(cb_par);
}

int mesh_cmd_sig_time_TAI_UTC_delta_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
    mesh_time_TAI_UTC_delta_set_t *p_set = (mesh_time_TAI_UTC_delta_set_t *)par;

	mesh_time.delta_new = p_set->delta_new;
	memcpy(mesh_time.TAI_delta_change, p_set->TAI_delta_change, 5);
    
	mesh_time_TAI_UTC_delta_st_rsp(cb_par);
	mesh_time.time.TAI_UTC_delta = p_set->delta_new;	// set later
	
    return err;
}

//--
int mesh_tx_cmd_time_role_st(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md)
{
	return mesh_tx_cmd_rsp(TIME_ROLE_STATUS, &mesh_time.role, 1, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_time_role_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_time_schedule.time_setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_time_role_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}

int mesh_time_role_st_rsp(mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_time_role_st(cb_par->model_idx, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

int mesh_cmd_sig_time_role_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return mesh_time_role_st_rsp(cb_par);
}

int mesh_cmd_sig_time_role_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	u8 role = par[0];
	if(role < TIME_ROLE_MAX){
		mesh_time.role = role;    
		mesh_time_role_st_rsp(cb_par);
	}
	
    return err;
}
#endif

// both server and client support time status message 
int mesh_cmd_sig_time_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
#if MD_SERVER_EN
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
        #if (!(__PROJECT_MESH_PRO__ && (!DEBUG_SHOW_VC_SELF_EN)))   // don't update time for app
        time_status_t *p_set = (time_status_t *)par;
        if(p_set->TAI_sec && (!g_TAI_sec)){
            mesh_time_set((time_status_t *)par);
        }
        #endif
    }
    return err;
#else
    return 0;
#endif
}

#if MD_CLIENT_EN
int mesh_cmd_sig_time_zone_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

int mesh_cmd_sig_time_TAI_UTC_delta_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}

int mesh_cmd_sig_time_role_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
#endif

//--model command interface-------------------
//-----------access command--------
int access_cmd_time_get(u16 adr, u32 rsp_max)
{
	return SendOpParaDebug(adr, rsp_max, TIME_GET, 0, 0);
}

int access_cmd_time_set(u16 adr, u32 rsp_max, time_status_t *p_set)
{
	return SendOpParaDebug(adr, rsp_max, TIME_SET, (u8 *)p_set, sizeof(time_status_t));
}

#if 1 // just for test, no use now.
/*
tx_cmd_time_set_local_sample(): just for showing how to creat a time parameters.
in fact, we can get TAI_sec and zone_offset directly by some API of APP or PC.
*/
void tx_cmd_time_set_local_sample()
{
    // beijing: 2019/1/1  09:00:00 (time zone: east 8)
    s8 zone_hour = 8;   // Positive numbers are eastwards
    mesh_UTC_t UTC = {0};
    UTC.year = 2019;
    UTC.month = 12;
    UTC.day = 4;
    UTC.hour = 10 - zone_hour;  // translate to 0 time zone.
    UTC.minute = 0;
    UTC.second = 0;
    u32 TAI_sec = get_TAI_sec(&UTC);

    time_status_t time_set = {0};
    time_set.TAI_sec = TAI_sec;
    time_set.zone_offset = get_time_zone_offset(zone_hour*60);
    
    access_cmd_time_set(0xffff, 1, &time_set);
}
#endif

//--model command interface end----------------


#endif

