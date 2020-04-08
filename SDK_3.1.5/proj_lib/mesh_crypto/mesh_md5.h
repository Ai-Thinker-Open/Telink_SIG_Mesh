/********************************************************************************************************
 * @file     mesh_md5.h 
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
#ifndef __MESH_MD6_H
#define __MESH_MD6_H

#include "le_crypto.h"
#include "mesh_crypto.h"
#include "proj_lib/sig_mesh/app_mesh.h"


typedef int 			S32;
typedef unsigned int 	U32;
typedef unsigned char 	U8;

typedef U32 		  UINT4;
typedef int 	      INT4;

typedef U8		*POINTER;
typedef unsigned long uint32;

/* MD5 context. */
struct MD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

typedef struct {
    u32  time_low;
    u16  time_mid;
    u16  time_hi_and_version;
    u8   clock_seq_hi_and_reserved;
    u8   clock_seq_low;
    u8   node[6];
} uuid_mesh_t;
void uuid_create_md5_from_name(uuid_mesh_t *uuid, uuid_mesh_t nsid, void *name,
                               int namelen);
void uuid_create_by_mac(u8 *mac,u8 *uuid);
#endif 
