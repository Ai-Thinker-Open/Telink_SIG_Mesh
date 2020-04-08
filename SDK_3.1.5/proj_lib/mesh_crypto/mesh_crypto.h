/********************************************************************************************************
 * @file     mesh_crypto.h 
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

#ifndef BLE_MESH_CRYPTO_H
#define BLE_MESH_CRYPTO_H

#include "proj_lib/ble/ll/ll.h"
//#include <memory.h>

int test_mesh_sec_func ();
int test_mesh_sec_key ();
int test_mesh_sec_provision ();
int test_mesh_sec_message ();

int mesh_sec_func_s1m (unsigned char *s1, char * m);
int mesh_sec_func_s1 (unsigned char *s1, unsigned char * m, int n);
int mesh_sec_func_k1 (unsigned char *k1, unsigned char * n, int nn, unsigned char *s, unsigned char * p, int np);
int mesh_sec_func_k1_id (unsigned char *k1, unsigned char n[16]);
int mesh_sec_func_k1_beacon (unsigned char *k1, unsigned char n[16]);

int mesh_sec_func_k2 (unsigned char *nid, unsigned char *ek, unsigned char *pk, unsigned char * n, unsigned char * p, int np);
int mesh_sec_func_k3 (unsigned char *k3, unsigned char n[16]);
int mesh_sec_func_k4 (unsigned char *k4, unsigned char n[16]);

int mesh_sec_beacon_auth (unsigned char *key, unsigned char *m, int dec);
int mesh_sec_beacon_dec (unsigned char *m);

int mesh_sec_prov_confirmation_key (unsigned char *key, unsigned char *input, int n, unsigned char ecdh[32]);
int mesh_sec_prov_confirmation (unsigned char *cfm, unsigned char *input, int n, unsigned char ecdh[32],
									unsigned char random[16], unsigned char auth[16]);
int mesh_sec_prov_session_key (unsigned char sk[16], unsigned char *sn, unsigned char *input, int n, unsigned char ecdh[32],
									unsigned char randomProv[16], unsigned char randomDev[16]);
void mesh_sec_prov_salt(unsigned char prov_salt[16],unsigned char *input,unsigned char randomProv[16], unsigned char randomDev[16]);

void mesh_sec_prov_salt(unsigned char prov_salt[16],unsigned char *input,unsigned char randomProv[16], unsigned char randomDev[16]);

int	mesh_sec_msg_enc (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length);
int	mesh_sec_msg_dec (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, int mic_length);
int	mesh_sec_msg_enc_ll (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, u8 *aStr, u8 aStrLen, int mic_length);
int	mesh_sec_msg_dec_ll (unsigned char key[16], unsigned char nonce[13], unsigned char* dat, int n, u8 *aStr, u8 aStrLen, int mic_length);

int	mesh_sec_msg_obfuscation (unsigned char key[16], unsigned char iv[4], unsigned char* p_ctl);
int  mesh_sec_msg_enc_nw(u8 *nw, u8 len_lt, u8 swap_type_lt, int use_friend_key, u8 len_nw, u8 adv_type,u8 cfg_filter, u8 nk_array_idx);
int  mesh_sec_msg_enc_nw_rf_buf(u8 *nw, u8 len_lt, int use_friend_key,u8 cfg_filter, u8 nk_array_idx);
int  mesh_sec_msg_enc_apl(u8 *mat, u8 *bear, int mic_length);
int  mesh_sec_msg_dec_nw(u8 *nw, int len_dec_nw_int, u8 nid, u8 *friend_key_flag,u8 cfg_filter, int src_type);
int  mesh_sec_msg_dec_apl(u8 *ac, u16 len_ut, const u8 *nw_little);
u8 mesh_sec_get_aid(u8 *key);
int mesh_sec_dev_key (unsigned char dev_key[16], unsigned char *salt, unsigned char ecdh[32]);
void mesh_sec_get_nid_ek_pk_master(u8 *nid, u8 *ek, u8 *pk, u8 *nk);
void mesh_sec_get_nid_ek_pk_friend(u8 lpn_idx, u8 *nid, u8 *ek, u8 *pk, u8 *nk);
void mesh_sec_get_network_id(u8 *nw_id, u8 *nk);
void mesh_sec_get_identity_key(u8 *id_key, u8 *nk);
void mesh_sec_get_beacon_key(u8 *bc_key, u8 *nk);
void endianness_swap_u32(u8 *data);
void endianness_swap_u16(u8 *data);
void endianness_swap_u24(u8 *data);
void endianness_swap_u48(u8 *data);
void endianness_swap_u64(u8 *data);
void endianness_swap(u8 *nw, u8 swap_type);
void mesh_swap_nw_lt(u8 *nw, u8 swap_type_lt);
int test_proxy_adv_with_node_identity();

u8 crc8_rohc(u8 *data,u32 len);
void test_virtual_address();
u16 cala_vir_adr_by_uuid(u8 *p_label_uuid);
void encode_password_ll(u8 *sk_user, u8 *pd, u32 pd_len, int mic_len); // pd_len exclude mic
int decode_password_ll(u8 *sk_user, u8 *pd, u32 pd_len, int mic_len); // pd_len include mic


#define SECURITY_ENABLE     (1) // use in library, if 0, it must be provisioned before.  //extern u8 security_enable;

//------------------ define----
#define NONCE_TYPE_NETWORK      0x00
#define NONCE_TYPE_APPLICATION  0x01
#define NONCE_TYPE_DEVICE       0x02
#define NONCE_TYPE_PROXY        0x03

enum{
    ERR_NO_NW_DEC_ERR               = 1,    // must 1 for decryption error
    ERR_NO_NW_DEC_ADR_INVALID       = 2,
    ERR_NO_NW_DEC_CACHE_OLD         = 3,
};


#endif 
