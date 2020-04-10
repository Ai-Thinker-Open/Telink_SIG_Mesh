#ifndef _MIJIA_PUB_PROC_C_
#define _MIJIA_PUB_PROC_C_
#include "mible_api.h"
#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "../app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
//#include "proj_lib/mesh_crypto/ecc_dsa.h"
#include "../mesh_common.h"
#include "../ev.h"
#include "./certi/third_party/micro-ecc/uECC.h"
#include "./certi/third_party/micro-ecc/types.h"

u8 pub_step_proc_cb(u8 sts_flag,model_common_t *p_model,u32 model_id);
void pub_max_inter_rcv_cb(u8 max_pub);
void mi_pub_clear_trans_flag();
void mi_pub_send_all_status();
void mi_pub_para_init();




#endif

