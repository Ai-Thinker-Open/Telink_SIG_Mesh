/********************************************************************************************************
 * @file     sensors_model.c 
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
#include "sensors_model.h"
u32 mesh_md_sensor_addr = FLASH_ADR_MD_SENSOR;
model_sensor_t			model_sig_sensor;

#if SENSOR_LIGHTING_CTRL_EN
// MD_CLIENT_EN, SENSOR_GPIO_PIN and MD_SENSOR_EN are must be opened.
STATIC_ASSERT(MD_CLIENT_EN && MD_SENSOR_EN && SENSOR_GPIO_PIN && SENSOR_LIGHTING_CTRL_ON_MS);
#endif

#if(MD_SENSOR_EN)
#if MD_SERVER_EN
STATIC_ASSERT((MD_LOCATION_EN == 0) || (MD_PROPERTY_EN == 0));   // because use same flash sector to save
u32 sensor_measure_ms = 0;
u32 sensure_measure_quantity = 0;

mesh_cmd_sensor_descript_st_t sensor_descrip[SENSOR_NUMS] = { 
								{PROP_ID, 0x347, 0x256, 0x02, 0x40, 0x4B},
							};

u32 get_prop_id_index(u16 prop_id)
{
	if(prop_id == PROHIBITED){
		return ID_PROHIBITED;
	}
	for(u8 i=0; i<SENSOR_NUMS; i++){
		if(prop_id == model_sig_sensor.sensor_states[i].prop_id){
			return i;
		}
	}
	
	return ID_UNKNOWN;
}

u8 get_prop_id_format_type(u16 prop_id)
{
	u8 ret = 0;
	switch(prop_id){
		case 0x004E:
			ret = 3;
			break;
		default:
			break;
	}

	return ret;
}

void mesh_global_var_init_sensor_descrip()
{	
	foreach_arr(i,sensor_descrip){	
		model_sig_sensor.sensor_states[i].prop_id = sensor_descrip[i].prop_id;
		model_sig_sensor.sensor_states[i].cadence.fast_period_div = 2;
		model_sig_sensor.sensor_states[i].cadence.trig_type = 1;
        model_sig_sensor.sensor_states[i].cadence.cadence_unit.delta_down= 0x00;
        model_sig_sensor.sensor_states[i].cadence.cadence_unit.delta_up= 0x00;
        model_sig_sensor.sensor_states[i].cadence.cadence_unit.min_interval = 0x04;
        model_sig_sensor.sensor_states[i].cadence.cadence_unit.cadence_low = 0x00;
        model_sig_sensor.sensor_states[i].cadence.cadence_unit.cadence_hight = 0x01;
		for(u8 j=0; j<SENSOR_SETTINGS_NUMS; j++){
			model_sig_sensor.sensor_states[i].setting[j].setting_id = i+1;// 0:prohibited
			model_sig_sensor.sensor_states[i].setting[j].setting_access = READ_WRITE;
		}
	}
}

u32 mesh_sensor_possible_positive_error()
{
	return 100*sensor_descrip[0].positive_tolerance/4095;
}

u32 mesh_sensor_possible_negative_error()
{
	return 100*sensor_descrip[0].negative_tolerance/4095;
}

int mesh_cmd_sig_sensor_descript_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	
	model_g_light_s_t *p_model = (model_g_light_s_t *)cb_par->model;
	mesh_cmd_sensor_descript_st_t rsp[SENSOR_NUMS];
	u32 len = sizeof(rsp);
	u16 prop_id = par[0] + (par[1]<<8);
	
	if(par_len){
		len = sizeof(mesh_cmd_sensor_descript_st_t);
		rsp[0].prop_id = prop_id;
		u32 index = get_prop_id_index(prop_id);
		if(index == ID_PROHIBITED){
			return -1;
		}
		else if(index !=  ID_UNKNOWN){ 
			memcpy(rsp, &sensor_descrip[index], len);
		}
		else{
			len = 2;
		}
	}
	else{
		memcpy(rsp, sensor_descrip, sizeof(rsp));
	}
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_tx_cmd_rsp(cb_par->op_rsp, (u8 *)&rsp, len, p_model->com.ele_adr, cb_par->adr_src, 0, 0);
	}
	
	return err;
}

int mesh_tx_sensor_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id, int par_len)
{
	sensor_mpid_b_t rsp[SENSOR_NUMS];


	u8 len = 0;
	
	rsp[0].format = 1;
	rsp[0].prop_id = prop_id;
	rsp[0].length = ARRAY_SIZE(rsp[0].raw_value)-1;     // in PTS, length value decreace 1, confirm later
	if(par_len){
		u32 id_index = get_prop_id_index(prop_id);
		if(id_index == ID_PROHIBITED){
			return -1;
		}
		else if(id_index != ID_UNKNOWN){
			len = sizeof(sensor_mpid_b_t);
			memcpy(rsp[0].raw_value,&model_sig_sensor.sensor_states[id_index].sensor_data,ARRAY_SIZE(rsp[0].raw_value));
		}
		else{
			len = OFFSETOF(sensor_mpid_b_t,raw_value);
			memset(rsp, 0xff, len);
		}
	}
	else{// get all property value
		len = sizeof(rsp);
		for(u8 i=0; i<SENSOR_NUMS; i++){
			rsp[i].format = 1;
			rsp[i].length = ARRAY_SIZE(rsp[0].raw_value)-1;
			rsp[i].prop_id = model_sig_sensor.sensor_states[i].prop_id;			
			memcpy(rsp[i].raw_value,&model_sig_sensor.sensor_states[i].sensor_data, ARRAY_SIZE(rsp[0].raw_value));
		}
	}
	sensor_measure_ms = clock_time_ms();
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_sensor_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id, int par_len)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_sensor_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model, cb_par->op_rsp, prop_id, par_len);
}

int mesh_cmd_sig_sensor_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u16 prop_id = 0;
	if(par_len){
		prop_id = par[0] + (par[1]<<8);
	}
	
	return mesh_sensor_st_rsp(cb_par, prop_id, par_len);
}

int mesh_tx_cadence_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id)
{
	static sensor_cadence_st_t rsp;
	u8 len = OFFSETOF(sensor_cadence_st_t, cadence);
	
	rsp.prop_id = prop_id;
	u32 id_index = get_prop_id_index(prop_id);
	u8 delta_len = get_prop_id_format_type(prop_id);
	u8 cadence_len = delta_len;
	if(id_index == ID_PROHIBITED){
		return -1;
	}
	else if(id_index != ID_UNKNOWN){
		sensor_cadence_t *p_cadence = (sensor_cadence_t *)&model_sig_sensor.sensor_states[id_index].cadence;
		rsp.cadence.trig_type = p_cadence->trig_type;
		rsp.cadence.fast_period_div = p_cadence->fast_period_div;
		if(p_cadence->trig_type){
			delta_len = 2;
		}
		memcpy(rsp.cadence.par, &p_cadence->cadence_unit.delta_down, delta_len);
		memcpy(rsp.cadence.par+delta_len, &p_cadence->cadence_unit.delta_up, delta_len);
		rsp.cadence.par[2*delta_len] = p_cadence->cadence_unit.min_interval;
		memcpy(rsp.cadence.par+2*delta_len+1, &p_cadence->cadence_unit.cadence_low, cadence_len);
		memcpy(rsp.cadence.par+2*delta_len+1+cadence_len, &p_cadence->cadence_unit.cadence_hight, cadence_len);
		len = delta_len*2 + cadence_len*2 + 4;
	}

	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}
 
int mesh_sensor_cadence_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_cadence_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model, cb_par->op_rsp, prop_id);
}

int mesh_cmd_sig_sensor_cadence_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	sensor_cadence_st_t *p_cadence = (sensor_cadence_st_t *)par;

	return mesh_sensor_cadence_st_rsp(cb_par, p_cadence->prop_id);
}

int mesh_sensor_cadence_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_sensor.sensor_setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_cadence_st_rsp(idx, ele_adr, pub_adr, uuid, p_com_md, SENSOR_CANDECE_STATUS, model_sig_sensor.sensor_states[0].prop_id);
}

u8 mesh_cadence_para_check(u8 *par, int par_len)
{
	u8 err = 0;
	sensor_cadence_st_t *p_cadence = (sensor_cadence_st_t *)(par);
	u8 var_len = get_prop_id_format_type(p_cadence->prop_id);
	if(p_cadence->cadence.fast_period_div >= 15){
		err = 1;
	}
	if(p_cadence->cadence.trig_type){
		if((par_len != var_len*2 + OFFSETOF(sensor_cadence_st_t,cadence.cadence_unitless.min_interval)+1) || (p_cadence->cadence.cadence_unitless.min_interval >= 26)){
			err = 1;
		}
	}
	else{
		if((par_len!= 4+var_len*4) || (p_cadence->cadence.par[var_len*2] >= 26)){
			err = 1;
		}
	}

	return err;
}

int mesh_cmd_sig_sensor_cadence_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;

	sensor_cadence_st_t *p_cadence_set = (sensor_cadence_st_t *)(par);
	u16 prop_id =p_cadence_set->prop_id;
	u32 id_index = get_prop_id_index(prop_id);
	u8 par_err =  mesh_cadence_para_check(par, par_len);
	
	if(id_index == ID_UNKNOWN){

	}
	else if(par_err){
		prop_id = PROHIBITED;
	}
	else if(id_index < ID_UNKNOWN){
		u8 cadence_len = get_prop_id_format_type(prop_id);
		u8 delta_len = cadence_len;
		if(p_cadence_set->cadence.trig_type){
			delta_len = 2;
		}
		
		if((delta_len <= 4) && (cadence_len <= 4)){
			sensor_cadence_t *p_cadence = (sensor_cadence_t *)&model_sig_sensor.sensor_states[id_index].cadence;
			memset(p_cadence, 0x00, sizeof(sensor_cadence_t));
			p_cadence->trig_type = p_cadence_set->cadence.trig_type;
			p_cadence->fast_period_div = p_cadence_set->cadence.fast_period_div;
			memcpy(&p_cadence->cadence_unit.delta_down, p_cadence_set->cadence.par, delta_len);
			memcpy(&p_cadence->cadence_unit.delta_up, p_cadence_set->cadence.par+delta_len, delta_len);
			memcpy(&p_cadence->cadence_unit.min_interval, p_cadence_set->cadence.par+2*delta_len, 1);
			memcpy(&p_cadence->cadence_unit.cadence_low, p_cadence_set->cadence.par+2*delta_len+1, cadence_len);
			memcpy(&p_cadence->cadence_unit.cadence_hight, p_cadence_set->cadence.par+2*delta_len+cadence_len+1, cadence_len);
			mesh_model_store(1, SIG_MD_SENSOR_SETUP_S);

		}
	}
	
	if(cb_par->op_rsp != STATUS_NONE){
		err = mesh_sensor_cadence_st_rsp(cb_par, prop_id);
	}
	
    model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 0);
    model_pub_st_cb_re_init_sensor_setup(&mesh_sensor_cadence_st_publish); 
	return err;
}

int mesh_tx_settings_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id)
{
	sensor_settings_st_t rsp;
	u8 len = sizeof(rsp);
	
	rsp.prop_id = prop_id;
	u32 id_index = get_prop_id_index(prop_id);
	
	if(id_index == ID_PROHIBITED){
		return -1;
	}
	else if(id_index != ID_UNKNOWN){
		for(u8 i=0; i<SENSOR_SETTINGS_NUMS; i++){
			rsp.setting_prop_id[i] = model_sig_sensor.sensor_states[id_index].setting[i].setting_id;
		}
	}
	else{
		len = OFFSETOF(sensor_settings_st_t, setting_prop_id[0]);
	}
	
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_sensor_settings_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_settings_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model, cb_par->op_rsp, prop_id);
}

int mesh_cmd_sig_sensor_settings_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	u16 prop_id = par[0] + (par[1]<<8);
	return mesh_sensor_settings_st_rsp(cb_par, prop_id);
}

int mesh_tx_setting_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id, u16 setting_id)
{
	sensor_setting_st_t rsp;
	sensor_setting_t *p_setting = 0;
	u8 len = sizeof(rsp);
	
	rsp.prop_id = prop_id;
	rsp.setting_id = setting_id;

	u32 id_index = 	get_prop_id_index(prop_id);
	if(id_index == ID_PROHIBITED){
		return -1;
	}
	else if(id_index != ID_UNKNOWN){
		for(u8 i=0; i<SENSOR_SETTINGS_NUMS; i++){
			if(model_sig_sensor.sensor_states[id_index].setting[i].setting_id== setting_id){
				p_setting = (sensor_setting_t *)&model_sig_sensor.sensor_states[id_index].setting[i];
			}
		}
	}

	if(p_setting){
		memcpy(&rsp.setting_id, p_setting, sizeof(sensor_setting_t));
	}
	else{
		len = OFFSETOF(sensor_setting_st_t, setting_access);
	}
	
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_sensor_setting_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id, u16 setting_id)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_setting_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model,cb_par->op_rsp, prop_id, setting_id);
}

int mesh_cmd_sig_sensor_setting_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	sensor_setting_get_t * p_setting_get = (sensor_setting_get_t *) par;
	return mesh_sensor_setting_st_rsp(cb_par, p_setting_get->prop_id, p_setting_get->setting_id);
}

int mesh_sensor_setting_st_publish_ll(u8 idx)
{
	model_common_t *p_com_md = &model_sig_sensor.sensor_setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_setting_st_rsp(idx, ele_adr, pub_adr, uuid, p_com_md, SENSOR_SETTING_STATUS, model_sig_sensor.sensor_states[0].prop_id, model_sig_sensor.sensor_states[0].setting[0].setting_id);
}

int mesh_cmd_sig_sensor_setting_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	u16 prop_id = par[0] + (par[1]<<8);
	u16 setting_id = par[2] + (par[3]<<8);
	
	sensor_setting_t * p_setting =0;
	u32 id_index = 	get_prop_id_index(prop_id);
	
	if(id_index < ID_PROHIBITED){
		for(u8 i=0; i<SENSOR_SETTINGS_NUMS; i++){
			if(model_sig_sensor.sensor_states[id_index].setting[i].setting_id== setting_id){
				p_setting = (sensor_setting_t *)&model_sig_sensor.sensor_states[id_index].setting[i];
			}
		}
	}
	
	if(p_setting){
		if(p_setting->setting_access == READ_WRITE){
			memcpy(&p_setting->setting_raw, par+4, sizeof(p_setting->setting_raw));
			mesh_model_store(1, SIG_MD_SENSOR_SETUP_S);
		}
	}

	model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, cb_par->model, 0);
    model_pub_st_cb_re_init_sensor_setup(&mesh_sensor_setting_st_publish_ll);
	if(cb_par->op_rsp != STATUS_NONE){	
	 	err = mesh_sensor_setting_st_rsp(cb_par, prop_id, setting_id);
	}

	return err;
}

int mesh_tx_column_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id)
{
	sensor_col_st_t rsp;
	u8 len = sizeof(rsp);
	
	rsp.prop_id = prop_id;

	u32 id_index = 	get_prop_id_index(prop_id);
	if(id_index == ID_PROHIBITED){
		return -1;
	}
	else if(id_index != ID_UNKNOWN){
		
	}
	else{
		memset(&rsp.raw_value.raw_val_X, 0x00, sizeof(rsp.raw_value.raw_val_X));
		len = OFFSETOF(sensor_col_st_t, raw_value);
	}
	
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_sensor_column_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_column_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model, cb_par->op_rsp, prop_id);
}

int mesh_cmd_sig_sensor_column_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	sensor_col_get_t * p_col_get = (sensor_col_get_t *) par;
	return mesh_sensor_column_st_rsp(cb_par, p_col_get->prop_id);
}

int mesh_tx_series_st_rsp(u8 idx, u16 ele_adr, u16 dst_adr, u8 *uuid, model_common_t *pub_md, u16 op_rsp, u16 prop_id)
{
	sensor_series_st_t rsp;
	u8 len = sizeof(rsp);
	
	rsp.prop_id = prop_id;
	u32 id_index = 	get_prop_id_index(prop_id);
	
	if(id_index == ID_PROHIBITED){
		return -1;
	}
	else if(id_index != ID_UNKNOWN){
		memcpy(&rsp.series_raw, &model_sig_sensor.sensor_states[id_index].series_raw, sizeof(rsp.series_raw));
	}
	else{
		memset(&rsp.series_raw, 0x00, sizeof(rsp.series_raw));
		len = OFFSETOF(sensor_series_st_t, series_raw);
	}
	
	return mesh_tx_cmd_rsp(op_rsp, (u8 *)&rsp, len, ele_adr, dst_adr, uuid, pub_md);
}

int mesh_sensor_series_st_rsp(mesh_cb_fun_par_t *cb_par, u16 prop_id)
{
	model_common_t *p_model = (model_common_t *)cb_par->model;

	return mesh_tx_series_st_rsp(cb_par->model_idx, p_model->ele_adr, cb_par->adr_src, 0, p_model, cb_par->op_rsp, prop_id);
}

int mesh_cmd_sig_sensor_series_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	sensor_series_get_t *p_series = (sensor_series_get_t *) par;
	
	return mesh_sensor_series_st_rsp(cb_par, p_series->prop_id);
}

int mesh_sensor_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_sensor.sensor_srv[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	return mesh_tx_sensor_st_rsp(idx, ele_adr, pub_adr, uuid, p_com_md, SENSOR_STATUS, 0, 0);
}

int mesh_sensor_setup_st_publish(u8 idx)
{
	model_common_t *p_com_md = &model_sig_sensor.sensor_setup[idx].com;
	u16 ele_adr = p_com_md->ele_adr;
	u16 pub_adr = p_com_md->pub_adr;
	if(!pub_adr){
		return -1;
	}
	u8 *uuid = get_virtual_adr_uuid(pub_adr, p_com_md);
	
	return mesh_tx_cadence_st_rsp(idx, ele_adr, pub_adr, uuid, p_com_md, SENSOR_CANDECE_STATUS, PROP_ID);
}

u32 sensor_measure_proc()
{
	u32 min_interval = 1<<model_sig_sensor.sensor_states[0].cadence.cadence_unit.min_interval;
	if(clock_time_exceed_ms(sensor_measure_ms, min_interval)){// 
		sensor_measure_ms = clock_time_ms();
		u8 pub_flag = 0;
		//update sensure_measure_quantity here
#if !WIN32 && SENSOR_LIGHTING_CTRL_EN
        gpio_set_input_en(SENSOR_GPIO_PIN, 1);
        gpio_set_output_en(SENSOR_GPIO_PIN, 0);
        sleep_us(100);
        sensure_measure_quantity = gpio_read(SENSOR_GPIO_PIN) ? 0 : 1;
#endif
		
		if(sensure_measure_quantity < model_sig_sensor.sensor_states[0].sensor_data){
			if((model_sig_sensor.sensor_states[0].sensor_data - sensure_measure_quantity) > model_sig_sensor.sensor_states[0].cadence.cadence_unit.delta_down){
				pub_flag = 1;
			}
		}
		else{
			if((sensure_measure_quantity - model_sig_sensor.sensor_states[0].sensor_data) > model_sig_sensor.sensor_states[0].cadence.cadence_unit.delta_up){
				pub_flag = 1;
			}
		}

		if(pub_flag){			
			model_pub_check_set(ST_G_LEVEL_SET_PUB_NOW, (u8 *)&model_sig_sensor.sensor_srv[0].com, 0);
		}
		model_sig_sensor.sensor_states[0].sensor_data = sensure_measure_quantity;					
	}

	return 1;
}
#endif

#if SENSOR_LIGHTING_CTRL_EN

static u32 keep_on_timer = 0;
static volatile bool sensor_set_light_on = false;

void sensor_lighting_ctrl_set_light_on()
{
    keep_on_timer = clock_time();
    sensor_set_light_on = true;
}

void sensor_lighting_ctrl_proc()
{
    static mesh_cmd_g_level_st_t level_st;
    light_g_level_get((u8 *)&level_st, 0, ST_TRANS_LIGHTNESS);

    //u8 target_onoff = get_onoff_from_level(level_st.target_level);
    u8 target_onoff = get_onoff_from_level(level_st.present_level);

    if (sensor_set_light_on) {
        if (!target_onoff) {
            u8 buf[sizeof(u16) + sizeof(mesh_cmd_g_onoff_set_t)];
            buf[0] = G_ONOFF_SET & 0xFF;
            buf[1] = (G_ONOFF_SET >> 8) & 0xFF;
            mesh_cmd_g_onoff_set_t *set = (mesh_cmd_g_onoff_set_t *)&buf[2];
            set->tid = 0;
            set->onoff = 1;
            set->transit_t = 10;
            set->delay = 10;
            mesh_tx_cmd2self_primary(buf, sizeof(buf));
        }else{
            if (clock_time_exceed(keep_on_timer, SENSOR_LIGHTING_CTRL_ON_MS*1000)) {
                u8 buf[sizeof(u16) + sizeof(mesh_cmd_g_onoff_set_t)];
                buf[0] = G_ONOFF_SET & 0xFF;
                buf[1] = (G_ONOFF_SET >> 8) & 0xFF;
                mesh_cmd_g_onoff_set_t *set = (mesh_cmd_g_onoff_set_t *)&buf[2];
                set->tid = 0;
                set->onoff = 0;
                set->transit_t = 10;
                set->delay = 10;
                mesh_tx_cmd2self_primary(buf, sizeof(buf));

                sensor_set_light_on = false;
            }
        }
    }else{
        keep_on_timer = clock_time();
    }
}
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_sensor_descript_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err = 0;
	if(cb_par->model){	// model may be Null for status message
		//model_client_common_t *p_model = (model_client_common_t *)(cb_par->model);
	}
	return err;
}

int mesh_cmd_sig_sensor_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
#if SENSOR_LIGHTING_CTRL_EN
    sensor_mpid_b_t *sts = (sensor_mpid_b_t *)par;

    if (sts->raw_value[0]) {
        sensor_lighting_ctrl_set_light_on();
    }
#endif
	return 0;
}

int mesh_cmd_sig_sensor_cadence_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}

int mesh_cmd_sig_sensor_settings_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}

int mesh_cmd_sig_sensor_setting_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}

int mesh_cmd_sig_sensor_column_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}

int mesh_cmd_sig_sensor_series_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	return 0;
}
#endif
#endif
