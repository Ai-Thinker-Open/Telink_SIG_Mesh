#ifndef __USER_ALI_H
#define __USER_ALI_H
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"

typedef struct{
	//u8 rev;
	u16 cid;
	union{
		u8 pid;
		struct{
		u8	adv_ver:4;
		u8 	ser_fun:1;
		u8  ota_en:1;
		u8  ble_ver:2;
		};
	};
	u8  product_id[4];
	u8 	mac[6];
	union{
		u8 feature_flag;
		struct{		
			u8 adv_flag:1;
			u8 uuid_ver:7;
		};
	};
	u8 	rfu[2];
}sha256_dev_uuid_str;

typedef struct{
	u8 auth_ok;
	u8 ble_key[16];
}ais_gatt_auth_t;

extern ais_gatt_auth_t ais_gatt_auth;
extern const u8 iv[];

void set_sha256_init_para_mode(u8 mode);
void set_dev_uuid_for_sha256();
void create_sha256_input_string(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret);
void create_sha256_input_string_node(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret);
void caculate_sha256_to_create_static_oob();
void caculate_sha256_node_auth_value(u8 *auth_value);
int ais_write_pipe(void *p);
void ais_gatt_auth_init();
void caculate_sha256_node_oob(u8 *p_oob);
void mesh_ais_global_var_set();
void calculate_auth_value();

#endif

