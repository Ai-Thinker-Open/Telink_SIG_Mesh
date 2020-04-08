/********************************************************************************************************
 * @file     scheduler.c 
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
#ifdef WIN32
#include <stdlib.h>
#else
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "lighting_model.h"
#include "scene.h"
#include "scheduler.h"


#if (MD_SCHEDULE_EN)
#if MD_SERVER_EN
u16 get_schd_entry(u8 ele_idx)
{
	u16 entry = 0;
	foreach_arr(i,g_schd_list[0]){
		if(g_schd_list[ele_idx][i].valid_flag_or_idx){	// read as valid flag
			entry |= BIT(i);
		}
	}
	return entry;
}

int is_valid_schd_par(scheduler_t *p_set)
{
	if((p_set->year > 0x64)
	|| (p_set->hour > 0x19)){
		return 0;
	}
	
	return 1;
}

static inline u8 get_random_min_or_sec()
{
    return rand() % 60;
}

static inline u8 get_random_hour()
{
    return rand() % 24;
}

//--
int mesh_cmd_sig_scheduler_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	u16 entry = get_schd_entry(cb_par->model_idx);
	return mesh_tx_cmd_rsp(SCHD_STATUS, (u8 *)&entry, 2, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
}

//--
int mesh_tx_cmd_schd_action_st(u8 ele_idx, u8 schd_idx, u16 ele_adr, u16 dst_adr)
{
	if(schd_idx >= 16){
		return 0;
	}
	
	scheduler_t *p_get = &g_schd_list[ele_idx][schd_idx];
	scheduler_t rsp;
	memcpy(&rsp, p_get, sizeof(rsp));
	rsp.valid_flag_or_idx = schd_idx;
	
	return mesh_tx_cmd_rsp(SCHD_ACTION_STATUS, (u8 *)&rsp, SIZE_SCHEDULER, ele_adr, dst_adr, 0, 0);
}

#if 0	// no need publish for schd_action
int mesh_schd_action_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_schd_action.setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cmd_schd_action_st(idx, ele_adr, pub_adr, uuid, p_com_md);
}
#endif

int mesh_schd_action_st_rsp(mesh_cb_fun_par_t *cb_par, u8 schd_idx)
{
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	return mesh_tx_cmd_schd_action_st(cb_par->model_idx, schd_idx, p_model->com.ele_adr, cb_par->adr_src);
}

int mesh_cmd_sig_schd_action_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    //  check index in mesh_tx_cmd_schd_action_st();
	return mesh_schd_action_st_rsp(cb_par, par[0]);
}

int mesh_cmd_sig_schd_action_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	scheduler_t *p_set = (scheduler_t *)par;
	u8 schd_idx = p_set->valid_flag_or_idx;
	scheduler_t *p_save = &g_schd_list[cb_par->model_idx][schd_idx];

	if(!is_valid_schd_par(p_set)){
		return 0;
	}

	memcpy(p_save, p_set, SIZE_SCHEDULER);
	p_save->rand_sec = (SCHD_SEC_RANDOM == p_save->second) ? get_random_min_or_sec() : 0;
	p_save->rand_min = (SCHD_MIN_RANDOM == p_save->minute) ? get_random_min_or_sec() : 0;
	p_save->rand_hour = (SCHD_HOUR_RANDOM == p_save->hour) ? get_random_hour() : 0;
	p_save->rsv = 0;
	p_save->valid_flag_or_idx = 1;	// save as valid flag
	mesh_model_store(1, SIG_MD_SCHED_SETUP_S);

	rebulid_schd_nearest_and_check_event(1, get_local_TAI());
	
	if(cb_par->op_rsp != STATUS_NONE){
		mesh_schd_action_st_rsp(cb_par, schd_idx);
	}
	
    return err;
}


//--model command interface end----------------
static inline int is_every_15_match(u32 val)
{
#if 1   // more efficient for MCU
    return ((0 == val)||(15 == val)||(30 == val)||(45 == val));
#else
    return (0 == (val % 15));
#endif
}

static inline int is_every_20_match(u32 val)
{
#if 1   // more efficient for MCU
    return ((0 == val)||(20 == val)||(40 == val));
#else
    return (0 == (val % 20));
#endif
}

static inline int is_schd_alive(scheduler_t *p_schd)
{
    if((0 == p_schd->valid_flag_or_idx)     // read as valid flag
    || (SCHD_ACTION_NONE == p_schd->action)){
        return 0;
    }
    return 1;
}

int schd_random_rebuild_hour()
{
    int update = 0;
	scheduler_t *p_schd;
    foreach_arr(i,g_schd_list){
        foreach_arr(k, g_schd_list[0]){
            p_schd = &g_schd_list[i][k];
            if(is_schd_alive(p_schd)){
                if(SCHD_HOUR_RANDOM == p_schd->hour){
                    update = 1;
                    p_schd->rand_hour = get_random_hour();
                    if(SCHD_MIN_RANDOM == p_schd->minute){
                        p_schd->rand_min = get_random_min_or_sec();
                    }
                    if(SCHD_SEC_RANDOM == p_schd->second){
                        p_schd->rand_sec = get_random_min_or_sec();
                    }
                }
            }

        }
    }
    return update;
}

int schd_random_rebuild_min()
{
    int update = 0;
	scheduler_t *p_schd;
    foreach_arr(i,g_schd_list){
        foreach_arr(k, g_schd_list[0]){
            p_schd = &g_schd_list[i][k];
            if(is_schd_alive(p_schd)){
                if((SCHD_MIN_RANDOM == p_schd->minute)&&(SCHD_HOUR_RANDOM != p_schd->hour)){
                    update = 1;
                    p_schd->rand_min = get_random_min_or_sec();
                    if(SCHD_SEC_RANDOM == p_schd->second){
                        p_schd->rand_sec = get_random_min_or_sec();
                    }
                }
            }
        }
    }
    return update;
}

int schd_random_rebuild_sec()
{
    int update = 0;
	scheduler_t *p_schd;
    foreach_arr(i,g_schd_list){
        foreach_arr(k, g_schd_list[0]){
            p_schd = &g_schd_list[i][k];
            if(is_schd_alive(p_schd)){
                if((SCHD_SEC_RANDOM == p_schd->second)&&(SCHD_HOUR_RANDOM != p_schd->hour)&&(SCHD_MIN_RANDOM != p_schd->minute)){
                    update = 1;
                    p_schd->rand_sec = get_random_min_or_sec();
                }
            }
        }
    }
    return update;
}


int is_schd_match_second_or_min(u32 val_now, u32 val_schd, u32 val_rand)
{
    if((val_now == val_schd)
    || (SCHD_MIN_ANY == val_schd)
    || ((SCHD_MIN_EVERY15 == val_schd)&&is_every_15_match(val_now))
    || ((SCHD_MIN_EVERY20 == val_schd)&&is_every_20_match(val_now))
    || ((SCHD_MIN_RANDOM == val_schd)&&(val_rand == val_now))){
        return 1;
    }
    return 0;
}

static inline int is_schd_match_hour(u32 val_now, u32 val_schd, u32 val_rand)
{
    if((val_now == val_schd)
    || (SCHD_HOUR_ANY == val_schd)
    || ((SCHD_HOUR_RANDOM == val_schd)&&(val_rand == val_now))){
        return 1;
    }
    return 0;
}

static inline void schd_action_cb(scheduler_t *p_schd, int ele_idx)
{
    transition_par_t trs_par = {{0}};
    trs_par.transit_t = p_schd->trans_t;
    switch(p_schd->action){
        case SCHD_ACTION_OFF:
            access_cmd_onoff(ele_adr_primary, 0, 0, 0, &trs_par);
            //LOG_MSG_INFO(TL_LOG_COMMON,0,0,"SCHD_ACTION_OFF", 0);
            break;
        case SCHD_ACTION_ON:
            access_cmd_onoff(ele_adr_primary, 0, 1, 0, &trs_par);
            //LOG_MSG_INFO(TL_LOG_COMMON,0,0,"SCHD_ACTION_ON", 0);
            break;
        case SCHD_ACTION_SCENE:
            #if MD_SCENE_EN
            access_cmd_scene_recall(ele_adr_primary, 0, p_schd->scene_id, 0, &trs_par);
			//LOG_MSG_INFO(TL_LOG_COMMON,0,0,"SCHD_ACTION_SCENE", 0);
            #endif
            break;
        default:
            break;
    }

    #if 0
    u32 t_hh = p_schd->hour;    // can't use bit-field as parameter in LOG_MSG_INFO()
    u32 t_mm = p_schd->minute;
    u32 t_ss = p_schd->second;
    
    LOG_MSG_INFO(TL_LOG_COMMON,0,0,"current HH:%2d, MM:%2d, SS:%2d", g_UTC.hour, g_UTC.minute, g_UTC.second);
    LOG_MSG_INFO(TL_LOG_COMMON,0,0,"random  HH:%2d, MM:%2d, SS:%2d", p_schd->rand_hour, p_schd->rand_min, p_schd->rand_sec);
    LOG_MSG_INFO(TL_LOG_COMMON,0,0,"schedul HH:%2d, MM:%2d, SS:%2d", t_hh, t_mm, t_ss);
    #endif
}

#if 1 // test
schd_nearest_t g_schd_nearest = {0};

u32 get_min_sec_step(u8 val)
{
    if(val < 60){
        return 0;
    }else if(SCHD_MIN_ANY == val){
        return 1;
    }else if(SCHD_MIN_EVERY15 == val){
        return 15;
    }else if(SCHD_MIN_EVERY20 == val){
        return 20;
    }else{
        return 0;
    }
}

/*
first_flag: get the first match value, when the upper unit is larger.
*/
u8 get_nearest_hour(u8 hour_schd, u8 hour_now, u8 hour_rand, int first_flag)
{
    if(hour_schd < 24){
        if(first_flag || (hour_now <= hour_schd)){
            return hour_schd;
        }else{
            return NO_NEAREST;
        }
    }else if(SCHD_HOUR_ANY == hour_schd){
        return hour_now;
    }else if(SCHD_HOUR_RANDOM == hour_schd){
        if(first_flag || (hour_now <= hour_rand)){
            return hour_rand;
        }else{
            return NO_NEAREST;
        }
    }else{
        return NO_NEAREST;
    }
}

/*
first_flag: get the first match value, when the upper unit is larger.
*/
u8 get_nearest_min_or_sec(u8 val_schd, u8 val_now, u8 val_rand, int first_flag)
{
    if(val_schd < 60){
        if(first_flag || (val_now <= val_schd)){
            return val_schd;
        }else{
            return NO_NEAREST;
        }
    }else if(SCHD_MIN_ANY == val_schd){
        return first_flag ? 0 : val_now;
    }else if(SCHD_MIN_RANDOM == val_schd){
        if(first_flag || (val_now <= val_rand)){
            return val_rand;
        }else{
            return NO_NEAREST;
        }
    }else if((SCHD_MIN_EVERY15 == val_schd)){
        if(first_flag){
            return 0;
        }else{
            foreach(i,4){
                if(val_now <= i*15){
                    return i*15;
                }
            }
            return NO_NEAREST;
        }
    }else if((SCHD_MIN_EVERY20 == val_schd)){
        if(first_flag){
            return 0;
        }else{
            foreach(i,3){
                if(val_now <= i*20){
                    return i*20;
                }
            }
            return NO_NEAREST;
        }
    }else{
        return NO_NEAREST;
    }
}

int update_schd_nearest_TAI_today(u32 TAI_local)
{
    if(!TAI_local){
        return -1;
    }
    memset(&g_schd_nearest, 0, sizeof(g_schd_nearest)); // init
    
    mesh_UTC_t UTC_now = {0};
    if(0 != get_UTC(TAI_local, &UTC_now)){ // action for current TAI
        return 0;
    }

    u32 days_max = get_days_one_month(UTC_now.year, UTC_now.month);
    u32 month_bit = (u32)BIT(UTC_now.month-1);
    u32 week_bit = (u32)BIT(UTC_now.week);

    u32 nearest_TAI = -1;
    foreach_arr(i,g_schd_list){
        foreach_arr(k, g_schd_list[0]){
            scheduler_t *p_schd = &g_schd_list[i][k];
            if((0 == p_schd->valid_flag_or_idx)	    // read as valid flag
            || (SCHD_ACTION_NONE == p_schd->action)){
                continue;
            }

            // rebuild nearest_TAI every day in mesh_scheduler_proc_()
            if(((SCHD_YEAR_ANY == p_schd->year)||((UTC_now.year-YEAR_BASE) == p_schd->year))
            && ((SCHD_MONTH_ANY == p_schd->month)||(month_bit & p_schd->month))
            && (((SCHD_DAY_ANY == p_schd->day)&&((SCHD_WEEK_ANY == p_schd->week)||(week_bit & p_schd->week)))
                 ||((UTC_now.day == p_schd->day)||((p_schd->day > days_max) && (UTC_now.day == days_max))))){ // single_day_match
                // today match
            }else{
                continue;
            }

            u32 step_hh = (SCHD_HOUR_ANY == p_schd->hour) ? 1 : 0;
            u32 step_mm = get_min_sec_step(p_schd->minute);

            u8 nearst_hh = NO_NEAREST, nearst_mm = NO_NEAREST, nearst_ss = NO_NEAREST;

            nearst_hh = get_nearest_hour(p_schd->hour, UTC_now.hour, p_schd->rand_hour, 0);
            if(NO_NEAREST == nearst_hh){
                continue;
            }else{
                int check_next_hour = 0;
                int big_hour = nearst_hh > UTC_now.hour;
                nearst_mm = get_nearest_min_or_sec(p_schd->minute, UTC_now.minute, p_schd->rand_min, big_hour);
                if(NO_NEAREST == nearst_mm){
                    check_next_hour = 1;
                }else{
                    int big_min = big_hour || (nearst_mm > UTC_now.minute);
                    nearst_ss = get_nearest_min_or_sec(p_schd->second, UTC_now.second, p_schd->rand_sec, big_min);
                    if(NO_NEAREST == nearst_ss){
                        if(step_mm){
                            nearst_mm += step_mm;
                            if(nearst_mm < 60){
                                nearst_ss = get_nearest_min_or_sec(p_schd->second, 0, p_schd->rand_sec, 1);
                            }else{
                                check_next_hour = 1;
                            }
                        }else{
                            check_next_hour = 1;
                        }
                    }
                }
                
                if(check_next_hour && step_hh){
                    nearst_hh += step_hh;
                    if(nearst_hh < 24){
                        nearst_mm = get_nearest_min_or_sec(p_schd->minute, 0, p_schd->rand_min, 1);
                        nearst_ss = get_nearest_min_or_sec(p_schd->second, 0, p_schd->rand_sec, 1);
                    }
                }
            }

            if((nearst_hh < 24)&&(nearst_mm < 60)&&(nearst_ss < 60)){
                u32 temp = nearst_hh * 3600 + nearst_mm * 60 + nearst_ss;
                if(temp < nearest_TAI){
                    memset(g_schd_nearest.entry_bit, 0, sizeof(g_schd_nearest.entry_bit));
                    g_schd_nearest.entry_bit[i] = BIT(k);
                    nearest_TAI = temp;
                }else if(temp == nearest_TAI){
                    g_schd_nearest.entry_bit[i] |= BIT(k);
                }
            }
        }
    }

    if(nearest_TAI != -1){
        u32 temp = (TAI_local / SECOND_ONE_DAY)*SECOND_ONE_DAY;
        nearest_TAI += temp;
        if(TAI_local <= nearest_TAI){
            g_schd_nearest.TAI_local = nearest_TAI;
            return 0;
        }
    }
    return -1;
}

int schd_event_check(u32 TAI_local)
{
    int act = 0;
    if(!g_schd_nearest.TAI_local){
        return 0;
    }

    if(g_schd_nearest.TAI_local == TAI_local){
        act = 1;
        foreach_arr(i,g_schd_list){
            u32 entry_bit = g_schd_nearest.entry_bit[i];
            foreach_arr(k, g_schd_list[0]){
                if(entry_bit & BIT(k)){
                    entry_bit &= ~BIT(k);
                    scheduler_t *p_schd = &g_schd_list[i][k];
                    schd_action_cb(p_schd, i);
                    if(0 == entry_bit){
                        break;
                    }
                }
            }
        }
        
        update_schd_nearest_TAI_today(TAI_local + 1);		// check next second
    }
    return act;
}

void rebulid_schd_nearest_and_check_event(int rebuild, u32 TAI_local)
{
    if(!g_TAI_sec){
        return ;
    }

    if(rebuild){
        update_schd_nearest_TAI_today(TAI_local);
    }
    
    schd_event_check(TAI_local); // must after:  get schd nearest TAI_today()
}

void mesh_scheduler_proc()
{
    u32 TAI_local = get_local_TAI();
    int rebuild_nearest = 0;
    if(0 == TAI_local % 60){
        rebuild_nearest = schd_random_rebuild_sec();
        if(0 == TAI_local % (60*60)){
            rebuild_nearest |= schd_random_rebuild_min();
            if(0 == TAI_local % (60*60*24)){
                schd_random_rebuild_hour();
                rebuild_nearest = 1;        // rebuild every day
            }
        }
    }

    rebulid_schd_nearest_and_check_event(rebuild_nearest, TAI_local);
}
#endif
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_scheduler_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
int mesh_cmd_sig_schd_action_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err = 0;
    if(cb_par->model){  // model may be Null for status message
    }
    return err;
}
#endif

//--model command interface-------------------
//-----------access command--------
int access_cmd_schd_get(u16 adr, u32 rsp_max)
{
	return SendOpParaDebug(adr, rsp_max, SCHD_GET, 0, 0);
}

int access_cmd_schd_action_get(u16 adr, u32 rsp_max, u8 index)
{
	return SendOpParaDebug(adr, rsp_max, SCHD_ACTION_GET, &index, 1);
}

int access_cmd_schd_action_set(u16 adr, u32 rsp_max, scheduler_t *p_schd, int ack)
{
	return SendOpParaDebug(adr, rsp_max, ack ? SCHD_ACTION_SET : SCHD_ACTION_SET_NOACK, (u8 *)p_schd, SIZE_SCHEDULER);
}

void test_schd_action_set_off(u16 adr)
{
    scheduler_t schd_test = {0};
    schd_test.valid_flag_or_idx = 0;
    schd_test.year = SCHD_YEAR_ANY; // 18
    schd_test.month = SCHD_MONTH_ANY; // BIT(10 - 1);
    schd_test.day = SCHD_DAY_ANY;
    schd_test.hour = 8;
    schd_test.minute = 0;
    schd_test.second = 5;
    schd_test.week = SCHD_WEEK_ANY; // BIT(0);
    schd_test.action = SCHD_ACTION_OFF;
    schd_test.trans_t = 0;//TRANSITION_TIME_DEFAULT_VAL;
    schd_test.scene_id = 0;
    access_cmd_schd_action_set(adr, 0, &schd_test, 1);
}

void test_schd_action_set_on(u16 adr)
{
    scheduler_t schd_test = {0};
    schd_test.valid_flag_or_idx = 1;
    schd_test.year = SCHD_YEAR_ANY; // 18
    schd_test.month = SCHD_MONTH_ANY; // BIT(10 - 1);
    schd_test.day = SCHD_DAY_ANY;
    schd_test.hour = 8;
    schd_test.minute = 0;
    schd_test.second = 8;
    schd_test.week = SCHD_WEEK_ANY; // BIT(0);
    schd_test.action = SCHD_ACTION_ON;
    schd_test.trans_t = 0;//TRANSITION_TIME_DEFAULT_VAL;
    schd_test.scene_id = 0;
    access_cmd_schd_action_set(adr, 0, &schd_test, 1);
}

void test_schd_action_set_scene(u16 adr)
{
    scheduler_t schd_test = {0};
    schd_test.valid_flag_or_idx = 2;
    schd_test.year = SCHD_YEAR_ANY;
    schd_test.month = SCHD_MONTH_ANY; // BIT(0);
    schd_test.day = SCHD_DAY_ANY;
    schd_test.hour = 8;
    schd_test.minute = 0;
    schd_test.second = 12;
    schd_test.week = SCHD_WEEK_ANY; // BIT(0);
    schd_test.action = SCHD_ACTION_SCENE;
    schd_test.trans_t = 0;//TRANSITION_TIME_DEFAULT_VAL;
    schd_test.scene_id = 1;
    access_cmd_schd_action_set(adr, 0, &schd_test, 1);
}
#endif

