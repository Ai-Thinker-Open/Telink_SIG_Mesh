#ifndef __USER_PROC_H
#define __USER_PROC_H
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "user_ali.h"
#include "nl_api/nl_common.h"

u8 ais_pri_data_set(u8 *p);
void user_sha256_data_proc();
void user_node_oob_set();
u8   user_mac_proc();
void user_power_on_proc();
void user_mesh_cps_init();
void user_set_def_sub_adr();
void user_system_time_proc();
void mesh_provision_para_init();
void user_prov_multi_oob();
void user_prov_multi_device_uuid();
int user_node_rc_link_open_callback();

#endif
