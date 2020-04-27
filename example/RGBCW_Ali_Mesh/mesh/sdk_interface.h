/********************************************************************************************************
 * @file     sdk_interface.h 
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
#include "proj_lib/sig_mesh/app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "vendor_model.h"
#if MI_API_ENABLE
#include "mesh/mi_api/mi_vendor/vendor_model_mi.h"
#endif

//--------------------------------------------------------------------------
/**************************tn_p256_keypair**************************
function : generate the ecc key pair ,use random to generate  
para:
	s: privacy key  
	x: low 32 bytes for the public key 
	y: high 32 bytes for the public key 
****************************************************************************/
void tn_p256_keypair (unsigned char *s, unsigned char *x, unsigned char *y);

//--------------------------------------------------------------------------
/**************************tn_p256_keypair_mac**************************
function : generate the ecc key pair ,use mac buf  to generate  
para:
	s: privacy key  
	x: low 32 bytes for the public key 
	y: high 32 bytes for the public key 
	mac: the buf input as the random for the public key generate 
	len : the len for the buf of the mac 
****************************************************************************/
void tn_p256_keypair_mac (unsigned char *s, unsigned char *x, unsigned char *y,u8 *mac,u8 len );
//--------------------------------------------------------------------------
/**************************set_provision_adv_data**************************
function : set the uuid and the info ,  
para:
	p_uuid: the length is 16 bytes .
	oob_info: the length is  2 bytes .
****************************************************************************/
void set_provision_adv_data(u8 *p_uuid,u8 *oob_info);
//--------------------------------------------------------------------------
/**************************mesh_set_oob_type**************************
function : config the oob type ,the initial is no oob type  
para:
	type: MESH_NO_OOB,MESH_STATIC_OOB
	p_oob: the static oob for the encryptiton buf 
	len : the length for the p_oob 
****************************************************************************/
void mesh_set_oob_type(u8 type, u8 *p_oob ,u8 len );
//--------------------------------------------------------------------------
/**************************mesh_tx_cmd2normal_primary**************************
function : send cmd to model in the mesh  
para:
	op: the opcode for the model 
	par: the parameter for the model  
	par_len :  the length for the parameter 
	adr_dst: the adr for the dst (can be group adr or the unicast adr)
	rsp_max: the rsp cnt for the cmd ,if the adr_dst is unicast adr ,it will be the 1,if it's the group adr
			 the cnt will be the cnt you need .
Descryption: for the example of the cmd sending part for the light on and other model operation ,can
			see the interface in the cmd_interface.h
****************************************************************************/
int mesh_tx_cmd2normal_primary(u16 op, u8 *par, u32 par_len, u16 adr_dst, int rsp_max);










