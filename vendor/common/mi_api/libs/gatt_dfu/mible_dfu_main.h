/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __MI_DFU_MAIN_H__
#define __MI_DFU_MAIN_H__

/* Includes ------------------------------------------------------------------*/

#include "mible_type.h"

/* Exported define -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint16_t conn_handle;
    uint16_t srv_handle;
    uint16_t ctrl_handle;
    uint16_t data_handle;
    uint16_t product_id;
    char *   model;
    uint32_t flash_start;
    uint32_t flash_size;
} mible_dfu_init_t;

typedef enum {
    MIBLE_DFU_STATE_START,
    MIBLE_DFU_STATE_VERIFY,
    MIBLE_DFU_STATE_SWITCH,
    MIBLE_DFU_STATE_CANCEL,
} mible_dfu_state_t;

typedef struct {
    uint16_t fragment_size;
} mible_dfu_start_param_t;

typedef struct {
    uint8_t value;
} mible_dfu_verify_param_t;

typedef struct {
    uint8_t up_to_app;
} mible_dfu_switch_param_t;

typedef union {
    mible_dfu_start_param_t start;
    mible_dfu_verify_param_t verify;
    mible_dfu_switch_param_t switch_plan;
} mible_dfu_param_t;

typedef void (* mible_dfu_callback_t) (mible_dfu_state_t state, mible_dfu_param_t *param);
typedef int (* pre_check_func_t)(void);

/* Exported functions --------------------------------------------------------*/

/* Internal API for mijia ble libs. Should not use in Application layer */
mible_status_t mible_dfu_init(mible_dfu_init_t * p_init);
mible_status_t mible_dfu_ctrl(uint8_t * buffer, uint16_t size);
mible_status_t mible_dfu_data(uint8_t * buffer, uint16_t size);
mible_status_t mible_dfu_upgrade(void);


/* User API */

/**@brief Register DFU callback function used to receive DFU event.
 *
 * @param[in] cb    Callback function
 */
mible_status_t mible_dfu_callback_register(mible_dfu_callback_t cb);
mible_status_t mible_dfu_callback_unregister(mible_dfu_callback_t cb);

/**@brief Register Pre Check function used to permit DFU or not.
 *
 * @param[in] fn    PreCheck function
 * @note  PreCheck function should return those codes:
 *        0  Permit DFU
 *        5  Forbid DFU, cause device is busy
 *        8  Forbid DFU, cause battery level is low
 */
void mible_dfu_pre_check_register(pre_check_func_t fn);
void mible_dfu_pre_check_unregister(pre_check_func_t fn);

#endif
