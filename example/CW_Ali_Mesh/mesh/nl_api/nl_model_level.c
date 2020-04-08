#include "proj/tl_common.h"
#include "../light.h"
#include "../user_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "../lighting_model.h"
#include "nl_model_level.h"

#if NL_API_ENABLE
/**
 * Callback function
 * @fn void level_state_changed(uint8_t element_index, int16_t old_level, int16_t new_level)
 * @param[in] element_idx is an index to the element in this device on which the level changed
 * @param[in] old_level is the previous value of the generic level state
 * @param[in] new_level is the new value of the generic level state
 *
 * Registering function
 * @brief     Registers callback that gets called whenever there is a level change in the model
 * @param[in] level_state_changed ptr to function callback
 *
 */
nl_level_state_changed_t	p_nl_level_state_changed;
void nl_register_level_server_state_changed_callback(void *p){
	p_nl_level_state_changed = p;
}

void nl_level_server_state_changed(uint8_t element_idx, int16_t pre_level, int16_t tar_level){\
	LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "%s, element_idx:%d, present_lv:%04x, target_lv:%04x", __FUNCTION__, element_idx, pre_level, tar_level);
	#if 1// for test
	nl_get_current_level_in_level_server(element_idx);
	#endif
}

/**
 * gets the current state from the level server
 * @param[in] element_idx index to the element in this device on which the level changed. 0-based index
 * @return the current level state in the level server
 */
int16_t nl_get_current_level_in_level_server(uint16_t element_idx){
	mesh_cmd_g_level_st_t rsp = {0};
    mesh_g_level_st_rsp_par_fill(&rsp, element_idx);
	LOG_MSG_INFO (TL_LOG_CMD_NAME, 0, 0, "%s, element_idx:%d, cur_lv:%04x", __FUNCTION__, element_idx, rsp.present_level);
	return rsp.present_level;
}
#endif
