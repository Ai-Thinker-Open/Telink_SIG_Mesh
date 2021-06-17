/********************************************************************************************************
 * @file     mesh_property.h 
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
#define G_MFG_PROPERTIES_GET		    0x2A82
#define G_MFG_PROPERTIES_STATUS		    0x43
#define G_MFG_PROPERTY_GET		        0x2B82
#define G_MFG_PROPERTY_SET		        0x44
#define G_MFG_PROPERTY_SET_NOACK		0x45
#define G_MFG_PROPERTY_STATUS		    0x46

#define G_ADMIN_PROPERTIES_GET		    0x2C82
#define G_ADMIN_PROPERTIES_STATUS		0x47
#define G_ADMIN_PROPERTY_GET		    0x2D82
#define G_ADMIN_PROPERTY_SET		    0x48
#define G_ADMIN_PROPERTY_SET_NOACK		0x49
#define G_ADMIN_PROPERTY_STATUS		    0x4A

#define G_USER_PROPERTIES_GET		    0x2E82
#define G_USER_PROPERTIES_STATUS		0x4B
#define G_USER_PROPERTY_GET		        0x2F82
#define G_USER_PROPERTY_SET		        0x4C
#define G_USER_PROPERTY_SET_NOACK		0x4D
#define G_USER_PROPERTY_STATUS		    0x4E

#define G_CLIENT_PROPERTIES_GET		    0x4F
#define G_CLIENT_PROPERTIES_STATUS		0x50

//------op parameters

//------------------vendor op end-------------------

#define INVALID_PROP_ID         (0)

enum{
    MESH_ACCESS_NOT_USER        = 0,
    MESH_ACCESS_READ            = 1,
    MESH_ACCESS_WRITE           = 2,
    MESH_ACCESS_RW              = 3,
    MESH_ACCESS_MAX,
};

enum{
    MESH_USER_ACCESS_READ       = 1,
    MESH_USER_ACCESS_WRITE      = 2,
    MESH_USER_ACCESS_RW         = 3,
};

enum{
    MESH_ADMIN_ACCESS_NOT_USER  = 0,
    MESH_ADMIN_ACCESS_READ      = 1,
    MESH_ADMIN_ACCESS_WRITE     = 2,
    MESH_ADMIN_ACCESS_RW        = 3,
};

enum{
    MESH_MFG_ACCESS_NOT_USER    = 0,
    MESH_MFG_ACCESS_READ        = 1,
};

typedef struct{
	u16 id;
	u8 data[MESH_PROPERTY_STR_SIZE_MAX];
}mesh_property_set_user_t;

typedef struct{
	u16 id;
	u8 access;
	u8 data[MESH_PROPERTY_STR_SIZE_MAX];
}mesh_property_set_admin_t;

typedef struct{
	u16 id;
	u8 access;
}mesh_property_set_mfg_t;

//---
extern model_property_t	model_sig_property;
extern u32 mesh_md_property_addr;

//---
void mesh_property_global_init();

//--- rx
int mesh_property_st_publish_user(u8 idx);
int mesh_property_st_publish_admin(u8 idx);
int mesh_property_st_publish_mfg(u8 idx);

#if MD_SERVER_EN
int mesh_cmd_sig_properties_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_property_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_property_set(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_properties_get         (0)
#define mesh_cmd_sig_property_get           (0)
#define mesh_cmd_sig_property_set           (0)
#endif

#if MD_CLIENT_EN
int mesh_cmd_sig_properties_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_property_status(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
#else
#define mesh_cmd_sig_properties_status      (0)
#define mesh_cmd_sig_property_status        (0)
#endif

//--- tx

