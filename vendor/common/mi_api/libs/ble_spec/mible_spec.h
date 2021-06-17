/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __MI_SPEC_MAIN_H__
#define __MI_SPEC_MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "mible_api.h"
#include "mible_log.h"
#include "mible_trace.h"
#include "common/crc32.h"
#include "common/mible_rxfer.h"
#include "common/queue.h"
#include "common/mible_crypto.h"


#define MIBLE_SPEC_TIMER_PERIOD         20
#define MIBLE_SPEC_VERSION              2

#define MIBLE_SPEC_OP_SET_PROPERTY      0x00
#define MIBLE_SPEC_OP_SET_PROPERTY_RSP  0x01
#define MIBLE_SPEC_OP_GET_PROPERTY      0x02
#define MIBLE_SPEC_OP_GET_PROPERTY_RSP  0x03
#define MIBLE_SPEC_OP_PROPERTY_CHANGED  0x04
#define MIBLE_SPEC_OP_ACTION            0x05
#define MIBLE_SPEC_OP_ACTION_RSP        0x06
#define MIBLE_SPEC_OP_EVENT_OCCURED     0x07


#if defined ( __CC_ARM )
struct spec_property_s{
    uint8_t        siid;
    uint16_t        piid;
    uint8_t        type;
    uint16_t        len;	  
    void*           val;
    uint16_t       code;
};
#elif defined ( __GNUC__ )
struct spec_property_s{
    uint8_t        siid;
    uint16_t        piid;
    uint8_t        type;
    uint16_t        len;	  
    void*           val;
    uint16_t       code;
} __PACKED;
#endif
typedef struct spec_property_s spec_property_t;

typedef struct {
    uint8_t*        addr;
    uint8_t         len;
 } mible_spec_data_t;

void spec_data_trans_init(uint16_t conn_handle, uint16_t srv_handle, uint16_t read_handle, uint16_t write_handle);
void spec_data_trans_deinit(void);
void spec_write_data_process(uint8_t *pdata, uint16_t len);
void spec_recv_data_process(uint8_t *pdata, uint16_t len);

mible_status_t mible_set_properties_rsp(uint16_t tid, uint8_t p_num, spec_property_t* array);
mible_status_t mible_get_properties_rsp(uint16_t tid, uint8_t p_num, spec_property_t* array);
mible_status_t mible_action_rsp(uint16_t tid, int16_t code);
mible_status_t mible_properties_changed(uint8_t p_num, spec_property_t* array);
mible_status_t mible_event_occured(uint8_t siid, uint16_t eiid, uint8_t p_num, spec_property_t* array);

typedef enum {
    MIBLE_SPEC_CMD_SET_PROPERTY,
    MIBLE_SPEC_CMD_GET_PROPERTY,
    MIBLE_SPEC_CMD_ACTION,
} mible_spec_command_t;

typedef void (* mible_spec_callback_t) (mible_spec_command_t state, uint16_t tid, uint8_t p_num, spec_property_t *param);
mible_status_t mible_spec_callback_register(mible_spec_callback_t cb);
mible_status_t mible_spec_callback_unregister(mible_spec_callback_t cb);

#endif
