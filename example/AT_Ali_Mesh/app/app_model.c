#include "proj/tl_common.h"
#ifndef WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "mesh/app_health.h"
#include "mesh/app_heartbeat.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh/remote_prov.h"
#include "mesh/lighting_model.h"
#include "mesh/lighting_model_HSL.h"
#include "mesh/lighting_model_xyL.h"
#include "mesh/lighting_model_LC.h"
#include "mesh/scene.h"
#include "mesh/time_model.h"
#include "mesh/sensors_model.h"
#include "mesh/scheduler.h"
#include "mesh/mesh_ota.h"
#include "mesh/mesh_property.h"
#include "mesh/generic_model.h"

//接收到Mesh数据
int mesh_cmd_at_data(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    char buf[64] = {0};

    if((cb_par->adr_src == ele_adr_primary) && (cb_par->op != 0x0282)) //如果原地址是自己,且不是开关操作，直接返回
    {
        return 0;
    }
	u_sprintf(buf, "\r\n+DATA:%02X%02X,%d,",(cb_par->op)&0xff,(cb_par->op)>>8,par_len);
    at_print(buf);
    at_print_hexstr((char*)par,par_len);
    at_print("\r\n");
    return 0;
}