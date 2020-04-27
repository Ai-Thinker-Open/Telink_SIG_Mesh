/********************************************************************************************************
 * @file     lighting_model_LC.h 
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
#define LIGHT_LC_MODE_GET		        0x9182
#define LIGHT_LC_MODE_SET		        0x9282
#define LIGHT_LC_MODE_SET_NOACK		    0x9382
#define LIGHT_LC_MODE_STATUS		    0x9482
#define LIGHT_LC_OM_GET		        	0x9582
#define LIGHT_LC_OM_SET		        	0x9682
#define LIGHT_LC_OM_SET_NOACK		    0x9782
#define LIGHT_LC_OM_STATUS		    	0x9882
#define LIGHT_LC_ONOFF_GET		        0x9982
#define LIGHT_LC_ONOFF_SET		        0x9A82
#define LIGHT_LC_ONOFF_SET_NOACK		0x9B82
#define LIGHT_LC_ONOFF_STATUS		    0x9C82
#define LIGHT_LC_PROPERTY_GET		    0x9D82
#define LIGHT_LC_PROPERTY_SET		    0x62
#define LIGHT_LC_PROPERTY_SET_NOACK		0x63
#define LIGHT_LC_PROPERTY_STATUS		0x64

//------op parameters

//------------------vendor op end-------------------

enum{
    LC_MODE_OFF = 0,
    LC_MODE_ON,
    LC_MODE_MAX,
};

enum{
    LC_OM_DISABLE = 0,
    LC_OM_EN,
    LC_OM_MAX,
};

// -----------
int is_light_lc_op(u16 op);
void scene_get_lc_par(scene_data_t *p_scene, int light_idx);
void scene_load_lc_par(scene_data_t *p_scene, int light_idx);
void LC_property_tick_set(int idx);
void LC_property_proc();
void light_LC_global_init();
int mesh_lc_prop_st_publish(u8 idx);
int mesh_lc_mode_st_publish(u8 idx);
int mesh_lc_onoff_st_publish(u8 idx);

#if MD_SERVER_EN
int mesh_cmd_sig_lc_mode_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_mode_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_om_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_om_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_prop_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_prop_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_lc_mode_get                (0)
#define mesh_cmd_sig_lc_mode_set                (0)
#define mesh_cmd_sig_lc_om_get                  (0)
#define mesh_cmd_sig_lc_om_set                  (0)
#define mesh_cmd_sig_lc_prop_get                (0)
#define mesh_cmd_sig_lc_prop_set                (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_lc_mode_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_om_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_lc_prop_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_lc_mode_status             (0)
#define mesh_cmd_sig_lc_om_status               (0)
#define mesh_cmd_sig_lc_prop_status             (0)
#endif


