/********************************************************************************************************
 * @file     vendor_model.c
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
#include "drivers/8258/flash.h"
#include "user_ali_time.h"

#if ALI_MD_TIME_EN

#define ALI_MD_TIME_LOG_EN							0

static void ali_mesh_time_save();

// UNIX Time.
volatile u32 ali_unix_time_running = 0;
volatile u32 ali_unix_time = 0;
volatile u32 ali_mesh_time_tick = 0;

static u32 all_timing_info_report_timing = 0;
static u8 all_timing_info_report_flag = 0;
static u8 timing_info_report_mask[(ALI_TIMING_INFO_NUM+7)/8];
static u16 timing_info_report_src_addr;
static u16 timing_info_report_des_addr;


typedef struct {
	u16 busy;
	u16 period_count;
	u8 retry_delay_count;
	u8 retry_times_count;
} ali_unix_sntp_tmp_t;
ali_unix_sntp_tmp_t ali_unix_sntp_tmp = {
	.period_count = U16_MAX-1,			// request time update at power up.
};

// for storage in flash.
u32 mesh_md_vd_ali_time_addr;
model_vd_ali_time_t model_vd_ali_time = {
		.sntp_para = {
			.period_time = 180,
			.retry_delay = 10,
			.retry_times = 10,
			.timezone = 8,
		},
};

typedef struct {
	u8 tid;
	u16 attr_type;
} ali_msg_header_t;

// set time.
typedef struct {
	u8 tid;
	u16 attr_type;
	u32 time;
} ali_msg_time_set_t;

// get time.
typedef struct {
	u8 tid;
	u16 attr_type;
} ali_msg_time_get_t;

// time info.
typedef ali_msg_time_set_t ali_msg_time_info_t;

// set sntp download.
typedef struct {
	u8 tid;
	u16 attr_type;
	u16 period_time;
	u8 retry_delay;
	u8 retry_times;
} ali_msg_sntp_para_set_t;

// get sntp para.  download.
typedef ali_msg_time_get_t ali_msg_sntp_para_get_t;

// sntp para info. upload.
typedef ali_msg_sntp_para_set_t ali_msg_sntp_para_info_t;

// set timezone download.
typedef struct {
	u8 tid;
	u16 attr_type;
	s8 timezone;
} ali_msg_timezone_set_t;

// get timezone. download.
typedef ali_msg_time_get_t ali_msg_timezone_get_t;

// timezone info. upload.
typedef ali_msg_timezone_set_t ali_msg_timezone_info_t;

// update time request. upload.
typedef ali_msg_time_get_t ali_msg_upd_time_req_t;

// update time. download.
typedef ali_msg_time_set_t ali_msg_upd_time_t;

// get timing info(s). download.
typedef struct {
	u8 tid;
	u16 attr_type;
	u8 index[1];
} ali_msg_timing_get_t;

typedef struct {
	u8 index;
	u32 time;
	timing_op_group_t op_groups[1];
} ali_msg_timing_set_para_t;

// set timing info(s) download.
typedef struct {
	u8 tid;
	u16 attr_type;
	ali_msg_timing_set_para_t para[1];
} ali_msg_timing_set_t;

// timing info(s). upload
typedef ali_msg_timing_set_t ali_msg_timing_info_t;

typedef struct {
	u8 index;
	u16 time;
	u8 schedule;
	timing_op_group_t op_groups[1];
} ali_msg_cycle_timing_set_para_t;

// set cycle timing. download.
typedef struct {
	u8 tid;
	u16 attr_type;
	ali_msg_cycle_timing_set_para_t para[1];
} ali_msg_cycle_timing_set_t;

// get cycle timing. upload.
typedef ali_msg_cycle_timing_set_t ali_msg_cycle_timing_info_t;

// delete timing. download.
typedef ali_msg_timing_get_t ali_msg_timing_del_t;

// Timing completion. upload.
typedef struct {
	u8 tid;
	u16 attr_type;
	u8 event;
	u8 index;
} ali_msg_timing_completion_t;

// err report.
typedef struct {
	u8 tid;
	u16 attr_type1;
	u16 attr_type2;
	u8 err_code;
	u8 index;
} ali_msg_err_rsp_t;

static int ali_mesh_err_rsp(u16 attr_type, u8 err_code, u8 index, mesh_cb_fun_par_t *cb_par, u8 *par)
{
	model_common_t *p_model = (model_common_t *) cb_par->model;

	ali_msg_err_rsp_t rsp;
	rsp.tid = 0;//p_attr->tid;
	rsp.attr_type1 = 0x0000;
	rsp.attr_type2 = attr_type;
	rsp.err_code = err_code;
	rsp.index = index;
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *) &rsp, sizeof(ali_msg_err_rsp_t), p_model->ele_adr, cb_par->adr_src, 0, 0);
}

static int ali_mesh_timing_completion_report(u8 index)
{
	ali_msg_timing_completion_t report;
	report.event = 0x11;
	report.index = index;
	return access_cmd_attr_indication(VD_MSG_ATTR_INDICA, 0xFFFF, ATTR_EVENT, &report.event, sizeof(report.event)+sizeof(report.index));
}

static int ali_mesh_upd_time_request()
{
	return access_cmd_attr_indication(VD_MSG_ATTR_UPD_TIME_REQ, 0xFFFF, ALI_VD_MD_TIME_ATTR_TYPE_TIME, 0, 0);
}



/************************ Time stamp conversion **************************/
#define UTC_BASE_YEAR		1970
#define DAY_PER_YEAR		365
#define MONTH_PER_YEAR		12
#define SEC_PER_DAY			(24*60*60)
#define SEC_PER_HOUR		(60*60)
#define SEC_PER_MINUTE		(60)
#define MINUTES_PER_HOUR	(60)
#define DAYS_PER_WEEK		(7)

const u8 days_per_mon[MONTH_PER_YEAR] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Determine if it(parameter year) is a leap year */
int is_leap_year(int year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

typedef struct {
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 week;
} date_time_t;


/* Convert unix time to a structure of data_time */
void unix_time2date_time(u32 unix_time, date_time_t *date_time)
{
	int total_days = unix_time / SEC_PER_DAY;
	int total_seconds = unix_time % SEC_PER_DAY;

	int year = UTC_BASE_YEAR;
	int remain_days = total_days;
	int leap_year = 0;
	for (; remain_days > 0; year++) {
		leap_year = is_leap_year(year);
		int days = DAY_PER_YEAR + leap_year;
		if (remain_days >= days) {
			remain_days -= days;
		} else {
			break;
		}
	}
	date_time->year = year;
	int month = 1;
	for (; month <= MONTH_PER_YEAR; month++) {
		int days = days_per_mon[month - 1] + (month == 2 && leap_year);
		if (remain_days >= days) {
			remain_days -= days;
		}else {
			break;
		}
	}
	date_time->month = month;
	date_time->day = remain_days+1;
	date_time->hour = total_seconds / SEC_PER_HOUR;
	date_time->minute = (total_seconds % SEC_PER_HOUR) / SEC_PER_MINUTE;
	date_time->second = total_seconds % 60;
	date_time->week = (total_days + 3) % 7;		// Sun ~ Sat : 0 ~ 6
}


static u16 unix_time2minutes_of_day(u32 unix_time)
{
	int seconds_of_day = unix_time % SEC_PER_DAY;
	return seconds_of_day / SEC_PER_MINUTE;
}


static int get_week_from_unix_time(u32 unix_time)
{
	int total_days = unix_time / SEC_PER_DAY;
	return (total_days + 3) % DAYS_PER_WEEK;	// Mon ~ Sun : 0 ~ 6
}



static int ali_mesh_unix_time_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_common_t *p_model = (model_common_t *) cb_par->model;
	ali_msg_header_t *header = (ali_msg_header_t *) par;
	ali_msg_time_info_t rsp;
	rsp.tid = 0;//header->tid;
	rsp.attr_type = header->attr_type;
	rsp.time = ali_unix_time;
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *) &rsp, sizeof(rsp), p_model->ele_adr, cb_par->adr_src, 0, 0);
}

int ali_mesh_unix_time_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	ali_msg_time_set_t *set = (ali_msg_time_set_t *) par;
	ali_unix_time = set->time;
	ali_mesh_time_tick = clock_time();
	ali_unix_time_running = 1;

	ali_unix_sntp_tmp.busy = 0;
	ali_unix_sntp_tmp.period_count = 0;
	ali_unix_sntp_tmp.retry_delay_count = 0;
	ali_unix_sntp_tmp.retry_times_count = 0;

	if (cb_par->op != VD_MSG_ATTR_SET_NACK)
		err = ali_mesh_unix_time_sts(par, par_len, cb_par);

	return err;
}

int ali_mesh_unix_time_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	if (!ali_unix_time_running){
		ali_msg_header_t *header = (ali_msg_header_t *) par;
		return ali_mesh_err_rsp(header->attr_type, ERR_CODE_UNIX_TIME_NOT_SET, 0, cb_par, par);
	}
	return ali_mesh_unix_time_sts(par, par_len, cb_par);
}

int ali_mesh_sntp_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_common_t *p_model = (model_common_t *) cb_par->model;
	ali_msg_header_t *header = (ali_msg_header_t *) par;
	ali_msg_sntp_para_info_t rsp;
	rsp.tid = 0;//header->tid;
	rsp.attr_type = header->attr_type;
	rsp.period_time = model_vd_ali_time.sntp_para.period_time;
	rsp.retry_delay = model_vd_ali_time.sntp_para.retry_delay;
	rsp.retry_times = model_vd_ali_time.sntp_para.retry_times;
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *) &rsp, sizeof(rsp), p_model->ele_adr, cb_par->adr_src, 0, 0);
}

int ali_mesh_sntp_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	ali_msg_sntp_para_set_t *set = (ali_msg_sntp_para_set_t *) par;
	model_vd_ali_time.sntp_para.period_time = set->period_time;
	model_vd_ali_time.sntp_para.retry_delay = set->retry_delay;
	model_vd_ali_time.sntp_para.retry_times = set->retry_times;

	if (cb_par->op != VD_MSG_ATTR_SET_NACK)
		err = ali_mesh_sntp_sts(par, par_len, cb_par);
	ali_mesh_time_save();
	return err;
}

int ali_mesh_sntp_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return ali_mesh_sntp_sts(par, par_len, cb_par);
}

int ali_mesh_timezone_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	model_common_t *p_model = (model_common_t *) cb_par->model;
	ali_msg_header_t *header = (ali_msg_header_t *) par;
	ali_msg_timezone_info_t rsp;
	rsp.tid = 0;//header->tid;
	rsp.attr_type = header->attr_type;
	rsp.timezone = model_vd_ali_time.sntp_para.timezone;
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *) &rsp, sizeof(rsp), p_model->ele_adr, cb_par->adr_src, 0, 0);
}

int ali_mesh_timezone_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	ali_msg_timezone_set_t *set = (ali_msg_timezone_set_t *) par;
	if (set->timezone > 12 || set->timezone < -12) {
		// Todo: done!
		err = -1;
		ali_msg_header_t *header = (ali_msg_header_t *) par;
		return ali_mesh_err_rsp(header->attr_type, ERR_CODE_ATTR_OP_PARAM_ERR, 0, cb_par, par);
	}
	model_vd_ali_time.sntp_para.timezone = set->timezone;

	if (cb_par->op != VD_MSG_ATTR_SET_NACK)
		err = ali_mesh_timezone_sts(par, par_len, cb_par);
	ali_mesh_time_save();
	return err;
}

int ali_mesh_timezone_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return ali_mesh_timezone_sts(par, par_len, cb_par);
}

int ali_mesh_timing_sts(u8 *index, int index_num)
{
	u8 data_buf[64];
	memset (data_buf, 0xFF, 64);
//	model_common_t *p_model = (model_common_t *) cb_par->model;
//	ali_msg_header_t *header = (ali_msg_header_t *) par;
	ali_msg_timing_info_t *rsp = (ali_msg_timing_info_t *) data_buf;
	rsp->tid = 0;//header->tid;
	rsp->attr_type = ALI_VD_MD_TIME_ATTR_TYPE_TIMING;//header->attr_type;
	int rsp_len = sizeof(ali_msg_header_t);
	for (int i = 0; i < index_num; i++) {
		ali_timing_info_t *info = &model_vd_ali_time.timing_infos[index[i]];
		if (info->active_time > 0) {
			ali_msg_timing_set_para_t *para = (ali_msg_timing_set_para_t *) &data_buf[rsp_len];
			para->index = index[i];
			rsp_len += sizeof(para->index);
			para->time = info->time;
			rsp_len += sizeof(para->time);
			for (int j = 0; j < info->op_group_num; j++) {
				para->op_groups[j].attr_type = info->op_groups[j].attr_type;
				para->op_groups[j].attr_para = info->op_groups[j].attr_para;
				rsp_len += sizeof(timing_op_group_t);
			}
		}
	}
#if ALI_MD_TIME_LOG_EN
	my_printf_uart("%s: ", __func__);
	for(int i=0;i<index_num;i++){
		my_printf_uart("%d ", index[i]);
	}
	my_printf_uart("\r\n");
#endif
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *)data_buf, rsp_len,
			timing_info_report_src_addr, timing_info_report_des_addr, 0, 0);
}

int ali_mesh_timing_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int index_num = par_len - sizeof(ali_msg_header_t);
	if (index_num <= 0) {
		return -1;
	}
	u8 *index = par + sizeof(ali_msg_header_t);

	int check_all_flag = 0;
	// check 0xFF
	for (int i = 0; i < index_num; i++) {
		if ( 0xFF == index[i] ) {
			check_all_flag = 1;
			memset (&timing_info_report_mask, 0xFF, sizeof(timing_info_report_mask));
			break;
		}
	}

	if (!check_all_flag) {
		for (int i=0; i< index_num; i++) {
			timing_info_report_mask[i / 8] |= i % 8;
		}
	}

	model_common_t *p_model = (model_common_t *) cb_par->model;

	all_timing_info_report_flag = 1;
	all_timing_info_report_timing = clock_time();
	timing_info_report_src_addr = p_model->ele_adr;
	timing_info_report_des_addr = cb_par->adr_src;

/*
	for (int i = 0; i < index_num; i++) {
		u8 idx = index[i];
		if (idx >= ALI_TIMING_INFO_NUM || model_vd_ali_time.timing_infos[idx].active_time == 0 || model_vd_ali_time.timing_infos[idx].type
				!= TIMING_TYPE_SINGLE) {
			ali_msg_header_t *header = (ali_msg_header_t *) par;
			return ali_mesh_err_rsp(header->attr_type, ERR_CODE_TIMING_INDEX_NOT_FOUND, idx, cb_par, par);
		}
	}
	*/
//	return ali_mesh_timing_sts(par, par_len, cb_par, index, index_num);
	return 0;
}

int ali_mesh_timing_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 err = 0;
	u8 err_index = 0xFF;
	u8 indexs[8];
	int index_num = 0;
	ali_msg_timing_set_t *set = (ali_msg_timing_set_t *) par;
	int handled_len = sizeof(ali_msg_header_t);
	while (handled_len < par_len) {
		ali_msg_timing_set_para_t *para = (ali_msg_timing_set_para_t *) &par[handled_len];

		if (para->index >= ALI_TIMING_INFO_NUM) {
			err = ERR_CODE_TIMING_INDEX_NOT_FOUND;
		}
		if ((para->time/60*60) < ali_unix_time) {
			err = ERR_CODE_TIME_RUNNING_OUT;
		}
		if (err){
			err_index = para->index;
			goto EXIT;
		}else{
			ali_timing_info_t *info = &model_vd_ali_time.timing_infos[para->index];
			info->active_time = ali_unix_time;
			info->time = para->time;
			info->type = TIMING_TYPE_SINGLE;
			info->schedule = 0;
			info->op_group_num = para->time % 60;
			if (info->op_group_num == 0){
				info->op_group_num = 1;
			}
			handled_len += OFFSETOF(ali_msg_timing_set_para_t, op_groups);
			for (int i = 0; i < info->op_group_num; i++) {
				if (i < ALI_TIMING_OP_GROUP_MAX){
					info->op_groups[i].attr_type = para->op_groups[i].attr_type;
					info->op_groups[i].attr_para = para->op_groups[i].attr_para;
				}
				handled_len += sizeof(timing_op_group_t);
			}

#if ALI_MD_TIME_LOG_EN
			date_time_t date_time;
			unix_time2date_time(para->time + model_vd_ali_time.sntp_para.timezone * SEC_PER_HOUR,
					&date_time);
			my_printf_uart("Setup timing: ");
			my_printf_uart("%d : %04d-%02d-%02d %02d:%02d:%02d  ", para->index,
					date_time.year, date_time.month, date_time.day,
					date_time.hour, date_time.minute, date_time.second);
			for(int j=0;j<info->op_group_num;j++){
				my_printf_uart("%04X:%02X  ", info->op_groups[j].attr_type,
						info->op_groups[j].attr_para);
			}
			my_printf_uart("\r\n");
#endif

			indexs[index_num] = para->index;
			index_num++;
		}
	}
EXIT:
	if (cb_par->op == VD_MSG_ATTR_SET_NACK){
		return err ? -1 : 0;
	}
	if (err){
		return ali_mesh_err_rsp(set->attr_type, err, err_index, cb_par, par);
	}else{
		ali_mesh_time_save();
		model_common_t *p_model = (model_common_t *) cb_par->model;
		timing_info_report_src_addr = p_model->ele_adr;
		timing_info_report_des_addr = cb_par->adr_src;
		return ali_mesh_timing_sts(indexs, index_num);
	}
}

int ali_mesh_cycle_timing_sts(u8 *index, int index_num)
{
	u8 data_buf[64];
//	model_common_t *p_model = (model_common_t *) cb_par->model;
//	ali_msg_header_t *header = (ali_msg_header_t *) par;
	ali_msg_cycle_timing_info_t *rsp = (ali_msg_cycle_timing_info_t *) data_buf;
	rsp->tid = 0;//header->tid;
	rsp->attr_type = ALI_VD_MD_TIME_ATTR_TYPE_CYCLE_TIMING;//header->attr_type;
	int rsp_len = sizeof(ali_msg_header_t);
	for (int i = 0; i < index_num; i++) {
		ali_timing_info_t *info = &model_vd_ali_time.timing_infos[index[i]];
		if (info->active_time > 0) {
			ali_msg_cycle_timing_set_para_t *para = (ali_msg_cycle_timing_set_para_t *) &data_buf[rsp_len];
			para->index = index[i];
			rsp_len += sizeof(para->index);
			para->time = info->time;
			rsp_len += sizeof(para->time);
			para->schedule = info->schedule;
			rsp_len += sizeof(para->schedule);
			for (int j = 0; j < info->op_group_num; j++) {
				para->op_groups[j].attr_type = info->op_groups[j].attr_type;
				para->op_groups[j].attr_para = info->op_groups[j].attr_para;
				rsp_len += sizeof(para->op_groups);
			}
		}
	}
#if ALI_MD_TIME_LOG_EN
	my_printf_uart("%s: ", __func__);
	for(int i=0;i<index_num;i++){
		my_printf_uart("%d ", index[i]);
	}
	my_printf_uart("\r\n");
#endif
	return mesh_tx_cmd_rsp(VD_MSG_ATTR_STS, (u8 *) rsp, rsp_len,
			timing_info_report_src_addr, timing_info_report_des_addr, 0, 0);
}



int ali_mesh_cycle_timing_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u8 err = 0;
	u8 err_index = 0xFF;
	u8 indexs[8];
	int index_num = 0;
	ali_msg_cycle_timing_set_t *set = (ali_msg_cycle_timing_set_t *) par;
	int handled_len = sizeof(ali_msg_header_t);
	while (handled_len < par_len) {
		ali_msg_cycle_timing_set_para_t *para = (ali_msg_cycle_timing_set_para_t *) &par[handled_len];
		if (para->index >= ALI_TIMING_INFO_NUM) {
			err = ERR_CODE_TIMING_INDEX_NOT_FOUND;
			err_index = para->index;
			goto EXIT;
		}
		ali_timing_info_t *info = &model_vd_ali_time.timing_infos[para->index];
		info->active_time = ali_unix_time;
		info->time = para->time;
		info->type = TIMING_TYPE_CYCLE;
		info->schedule = para->schedule;
		info->op_group_num = (para->time & 0xF000) >> 12;
		if (info->op_group_num == 0){
			info->op_group_num = 1;
		}
		handled_len += OFFSETOF(ali_msg_cycle_timing_set_para_t, op_groups);
		for (int i = 0; i < info->op_group_num; i++) {
			if (i < ALI_TIMING_OP_GROUP_MAX){
				info->op_groups[i].attr_type = para->op_groups[i].attr_type;
				info->op_groups[i].attr_para = para->op_groups[i].attr_para;
			}
			handled_len += sizeof(timing_op_group_t);
		}

#if ALI_MD_TIME_LOG_EN
		my_printf_uart("Setup cycle timing: ");
		my_printf_uart("%d WeekMask:", indexs[index_num]);
		for(int j=0;j<7;j++){
			if (info->schedule & BIT(j)){
				my_printf_uart("%d", j+1);
			}
		}
		my_printf_uart(" %02d:%02d %d ops:",
				(info->time&0xFFF)/60, (info->time&0xFFF)%60, info->op_group_num);

		for(int j=0;j<info->op_group_num;j++){
			my_printf_uart("%04X:%02X  ", info->op_groups[j].attr_type,
					info->op_groups[j].attr_para);
		}
		my_printf_uart("\r\n");
#endif

		indexs[index_num] = para->index;
		index_num++;
	}
EXIT:
	if (cb_par->op == VD_MSG_ATTR_SET_NACK){
		return err ? -1 : 0;
	}
	if (err){
		return ali_mesh_err_rsp(set->attr_type, err, err_index, cb_par, par);
	}else{
		ali_mesh_time_save();
		model_common_t *p_model = (model_common_t *) cb_par->model;
		timing_info_report_src_addr = p_model->ele_adr;
		timing_info_report_des_addr = cb_par->adr_src;
		return ali_mesh_cycle_timing_sts(indexs, index_num);
	}
}

int ali_mesh_timing_del(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	ali_msg_header_t *header = (ali_msg_header_t *) par;
	int index_num = par_len - sizeof(ali_msg_header_t);
	if (index_num <= 0) {
		return -1;
	}

	u8 *index = par + sizeof(ali_msg_header_t);

#if ALI_MD_TIME_LOG_EN
	my_printf_uart("Delete timing: ");
	for(int i=0;i<index_num;i++){
		my_printf_uart("%d ", index[i]);
	}
	my_printf_uart("\r\n");
#endif

	for (int i = 0; i < index_num; i++) {
		u8 idx = index[i];
		if (idx >= ALI_TIMING_INFO_NUM) {
			if (cb_par->op != VD_MSG_ATTR_SET_NACK) {
				return ali_mesh_err_rsp(header->attr_type, ERR_CODE_TIMING_INDEX_NOT_FOUND, idx, cb_par, par);
			} else {
				return -1;
			}
		}
		model_vd_ali_time.timing_infos[idx].active_time = 0;
		ali_mesh_time_save();
	}
	return 0;
}


static void ali_mesh_timing_proc(u32 ali_unix_time)
{
#if ALI_MD_TIME_LOG_EN
	// debug info.
	date_time_t date_time;
	unix_time2date_time(ali_unix_time, &date_time);
	my_printf_uart("Time: %4d-%02d-%02d %02d:%02d:%02d %d\r\n",
			date_time.year, date_time.month, date_time.day,
			date_time.hour, date_time.minute, date_time.second,
			date_time.week);
#endif

	// do operation once per minute.
	if (ali_unix_time % SEC_PER_MINUTE == 0) {
		u8 time_up_timing[ALI_TIMING_INFO_NUM];
		int time_up_count = 0;
		memset (time_up_timing, 0xFF, sizeof(time_up_timing));
		for (int i = 0; i < ALI_TIMING_INFO_NUM; i++) {
			// active_time != 0 && active_time != U32_MAX
			if (model_vd_ali_time.timing_infos[i].active_time > 0 && model_vd_ali_time.timing_infos[i].active_time != U32_MAX) {
				if (model_vd_ali_time.timing_infos[i].type == TIMING_TYPE_SINGLE) {
					u32 now_time = ali_unix_time / SEC_PER_MINUTE;
					u32 timing = (model_vd_ali_time.timing_infos[i].time + model_vd_ali_time.sntp_para.timezone * SEC_PER_HOUR) / SEC_PER_MINUTE;
					if ( now_time == timing) {
						time_up_timing[time_up_count++] = i;
					} else if (now_time > timing) {
						// remove outdated timing.
						model_vd_ali_time.timing_infos[i].active_time = 0;	// delete timing.
					}
				} else {
					int minutes_of_day = unix_time2minutes_of_day(ali_unix_time);
					int week = get_week_from_unix_time(ali_unix_time);
					if ((model_vd_ali_time.timing_infos[i].time & 0x0FFF) == minutes_of_day
							&& (model_vd_ali_time.timing_infos[i].schedule & BIT(week))) {
						time_up_timing[time_up_count++] = i;
					}
				}
			}
		}

		// check whether clash exists.
		if (time_up_count > 1) {
			for (int i = 0; i < time_up_count; i++) {
				ali_timing_info_t *info0 = &model_vd_ali_time.timing_infos[time_up_timing[i]];
				for (int j = i + 1; j < time_up_count; j++) {
					ali_timing_info_t *info1 = &model_vd_ali_time.timing_infos[time_up_timing[j]];
					for (int x = 0; x < info0->op_group_num; x++) {
						for (int y = 0; y < info1->op_group_num; y++) {
							if (info0->op_groups[x].attr_type == info1->op_groups[y].attr_type) {
								if (info0->active_time > info1->active_time) {
									time_up_timing[j] = 0xFF;
								} else {
									time_up_timing[x] = 0xFF;
								}
							}
						}
					}
				}
			}
		}

		// Ö´ÐÐ¶¨Ê±
		for (int i = 0; i < time_up_count; i++) {
			if (time_up_timing[i] == 0xFF) {
				continue;
			}
			ali_timing_info_t *info = &model_vd_ali_time.timing_infos[time_up_timing[i]];
			for(int j = 0; j < info->op_group_num; j++) {
				//To do: do operates. info->op_groups[j]
				if (ATTR_ONOFF == info->op_groups[j].attr_type) {
					u8 buf[sizeof(u16) + sizeof(mesh_cmd_g_onoff_set_t)];
					buf[0] = G_ONOFF_SET & 0xFF;
					buf[1] = (G_ONOFF_SET >> 8) & 0xFF;
					mesh_cmd_g_onoff_set_t *set = (mesh_cmd_g_onoff_set_t *)&buf[2];
					set->tid = 0;
					set->onoff = info->op_groups[j].attr_para;
					set->transit_t = 10;
					set->delay = 10;
					mesh_tx_cmd2self_primary(buf, sizeof(buf));
				}
			}

			if (info->type == TIMING_TYPE_SINGLE) {
				info->active_time = 0;	// delete timing.
				ali_mesh_time_save();
			}

			// Report Timing Completion Event.
			ali_mesh_timing_completion_report(time_up_timing[i]);
		}

		// Time synchronization proecess.
		ali_unix_sntp_tmp.period_count ++;
		if (!ali_unix_sntp_tmp.busy) {
			if (ali_unix_sntp_tmp.period_count >= model_vd_ali_time.sntp_para.period_time) {
				ali_unix_sntp_tmp.period_count = 0;
				ali_unix_sntp_tmp.busy = 1;
				ali_mesh_upd_time_request();
				ali_unix_sntp_tmp.retry_times_count ++;
			}
		}else{
			ali_unix_sntp_tmp.retry_delay_count ++;
			if (ali_unix_sntp_tmp.retry_delay_count >= model_vd_ali_time.sntp_para.retry_delay) {
				ali_mesh_upd_time_request();
				ali_unix_sntp_tmp.retry_delay_count = 0;
				ali_unix_sntp_tmp.retry_times_count ++;
				if (ali_unix_sntp_tmp.retry_times_count >= model_vd_ali_time.sntp_para.retry_times) {
					ali_unix_sntp_tmp.retry_times_count = 0;
					ali_unix_sntp_tmp.busy = 0;
				}
			}
		}
	}
}


static u8 save_time_info_flag = 0;
static u32 save_time_info_timing = 0;
/* save sntp parameters and timing informations to flash */
static void ali_mesh_time_save()
{
	save_time_info_flag = 1;
	save_time_info_timing = clock_time();
}


static void ali_mesh_time_save_to_flash_process()
{
	if (!save_time_info_flag){
		return;
	}
	if (!clock_time_exceed(save_time_info_timing, 1000000)){
		return;
	}
	save_time_info_flag = 0;

	mesh_common_store(FLASH_ADR_VD_TIME_INFO);
}

/* called in main_loop. */
void user_ali_time_proc()
{
	if (!(is_provision_success())) {
		return;
	}
	u32 clock_tmp = clock_time();
	u32 t_delta = (u32) (clock_tmp - ali_mesh_time_tick); // should be differrent from system_time_tick_

	if (t_delta >= ALI_MESH_TIME_CHECK_INTERVAL) {
		u32 interval_cnt = t_delta / ALI_MESH_TIME_CHECK_INTERVAL;
		foreach(i,interval_cnt) {
			ali_unix_time++;
			ali_mesh_timing_proc(ali_unix_time + model_vd_ali_time.sntp_para.timezone * SEC_PER_HOUR);
		}

		ali_mesh_time_tick += interval_cnt * ALI_MESH_TIME_CHECK_INTERVAL;
	}


	ali_mesh_time_save_to_flash_process();

	if (all_timing_info_report_flag && clock_time_exceed_ms(all_timing_info_report_timing, 300)) {
		all_timing_info_report_timing = clock_time();

		u8 indexs[5] = {0};
		int count = 0;
		for (int i=0; i<ALI_TIMING_INFO_NUM; i++) {
			if (!(timing_info_report_mask[i/8] & BIT(i%8))) {
				continue;
			}
			if (model_vd_ali_time.timing_infos[i].active_time != 0
					&& model_vd_ali_time.timing_infos[i].active_time != U32_MAX &&
					model_vd_ali_time.timing_infos[i].type == TIMING_TYPE_SINGLE) {
				indexs[count++] = i;
				timing_info_report_mask[i/8] &= ~BIT(i%8);
			}
			if (count >= 5) {
				break;
			}
		}
		if (count > 0) {
			ali_mesh_timing_sts(indexs, count);
			return;
		}

		for (int i=0; i<ALI_TIMING_INFO_NUM; i++) {
			if (!(timing_info_report_mask[i/8] & BIT(i%8))) {
				continue;
			}
			if (model_vd_ali_time.timing_infos[i].active_time != 0
					&& model_vd_ali_time.timing_infos[i].active_time != U32_MAX &&
					model_vd_ali_time.timing_infos[i].type == TIMING_TYPE_CYCLE) {
				indexs[count++] = i;
				timing_info_report_mask[i/8] &= ~BIT(i%8);
			}
			if (count >= 5) {
				break;
			}
		}

		if (count > 0) {
			ali_mesh_cycle_timing_sts(indexs, count);
			return;
		}
		all_timing_info_report_flag = 0;
	}
}

#endif
