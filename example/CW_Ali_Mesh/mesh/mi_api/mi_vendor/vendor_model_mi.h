/********************************************************************************************************
 * @file     vendor_model.h 
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
#include "../../mesh_node.h"

#if (VENDOR_MD_MI_EN)
// op cmd 11xxxxxx yyyyyyyy yyyyyyyy (vendor)
// ---------------------------------from 0xF0 to 0xFF
#define VD_MI_RC_KEY_REPORT				0xF0
#if DEBUG_VENDOR_CMD_EN
#define VD_MI_LIGHT_ONOFF_GET			0xF1
#define VD_MI_LIGHT_ONOFF_SET			0xF2
#define VD_MI_LIGHT_ONOFF_SET_NOACK		0xF3
#define VD_MI_LIGHT_ONOFF_STATUS		0xF4
    #if MD_VENDOR_2ND_EN
#define VD_MI_LIGHT_ONOFF_GET2			0xF5
#define VD_MI_LIGHT_ONOFF_SET2			0xF6
#define VD_MI_LIGHT_ONOFF_SET_NOACK2	0xF7
#define VD_MI_LIGHT_ONOFF_STATUS2		0xF8
    #endif
#endif

// MI VENDOR OPCODE 
#define VD_MI_GET_PROPERTY			0xc1
#define VD_MI_SET_PROPERTY			0xc3
#define VD_MI_SET_PROPERTY_NO_ACK	0xc4
#define VD_MI_PROPERTY_CHANGED		0xc5
#define VD_MI_ACTION				0xc6
#define VD_MI_RELAY_ACTION 			0xc7
#define VD_MI_EVENT_REPORT			0xc8

#define VD_MI_GW_NODE				0xfe
	#define MI_GW_FOUND_REQ				2
	#define MI_NODE_FOUND_REQ			3
	#define MI_NET_PARA_RSP				128

#define VD_MI_NODE_GW				0xff
	#define MI_SIMPLE_ROW_RSP 			1
	#define MI_GW_FOUND_RSP				2
	#define MI_DEV_FOUND_RSP			3
	#define MI_NET_PARA_REQ				128
typedef struct{
	u8 ssid;
	u8 piid;
}vd_mi_head_str;

typedef struct{
	vd_mi_head_str mi_head;
	u8 buf[4];
}vd_mi_get_property_str;


typedef struct{
	vd_mi_head_str mi_head;
	u8 value[4];
}vd_mi_set_property_str;

typedef struct{
	vd_mi_head_str mi_head;
	u8 value[4];
}vd_mi_set_noack_property_str;

typedef struct{
	vd_mi_head_str mi_head;
	u8 value[4];
}vd_mi_property_changed_str;


typedef struct{
	vd_mi_head_str mi_head;
	u8 value1;
	u8 value2;
	u8 value3;
	u8 value4;
	u8 value5;
	u8 value6;
}vd_mi_action_str;

typedef struct{
	vd_mi_head_str mi_head;
	u8 code;
	u8 value[5];
}vd_mi_reply_action_str;

typedef struct{
	vd_mi_head_str mi_head;
	u8 value1;
	u8 value2;
	u8 value3;
	u8 value4;
	u8 value5;
	u8 value6;
}vd_mi_event_report_str;

#define MIOT_SEPC_VENDOR_MODEL_SER	((0x0000<<16) | (VENDOR_ID_MI))
#define MIOT_SEPC_VENDOR_MODEL_CLI	((0x0001<<16) | (VENDOR_ID_MI)) 
#define MIOT_VENDOR_MD_SER			((0x0002<<16) | (VENDOR_ID_MI))	

//------0xE0 ~ 0xFF for customer


//------op parameters
typedef struct{
	u8 code;
	u8 rsv[7];
}vd_mi_rc_key_report_t;

typedef struct{
	u8 onoff;
	u8 tid;
}vd_mi_light_onoff_set_t;

typedef struct{
	u8 present_onoff;
}vd_mi_light_onoff_st_t;

// ------------------


//------------------vendor op end-------------------

int mi_vd_cmd_key_report(u16 adr_dst, u8 key_code);
int mi_vd_cmd_onoff(u16 adr_dst, u8 rsp_max, u8 onoff, int ack);
int mi_vd_light_onoff_st_publish(u8 idx);
int mi_vd_light_onoff_st_publish2(u8 idx);


int mi_mesh_search_model_id_by_op_vendor(mesh_op_resource_t *op_res, u16 op, u8 tx_flag);
int is_mi_cmd_with_tid_vendor(u8 *tid_pos_out, u16 op, u8 tid_pos_vendor_app);
#endif

