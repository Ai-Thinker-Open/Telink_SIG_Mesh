#include "nl_common.h"
#include "nl_model_level.h"
#include "nl_model_schedules.h"
#include "../user_config.h"

#if NL_API_ENABLE
void register_nl_callback(){
	//nl_model_level
	nl_register_level_server_state_changed_callback(nl_level_server_state_changed);
	
	//nl_model_schedule
	nl_register_scene_server_state_recalled_callback(nl_scene_server_state_recalled);
	nl_register_scene_server_get_vendor_scene_data_callback(nl_scene_server_get_vendor_scene_data);
}
#else
#define register_nl_callback() 
#endif