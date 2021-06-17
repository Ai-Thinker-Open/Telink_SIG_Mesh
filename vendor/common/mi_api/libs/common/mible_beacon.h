#ifndef MIBLE_BEACON_H__
#define MIBLE_BEACON_H__

#include "mible_type.h"
#include "mible_port.h"

typedef enum {
    MI_EVT_BASE          = 0x0000,
    MI_EVT_CONNECT       = 0x0001,
    MI_EVT_SIMPLE_PAIR   = 0x0002,
    MI_EVT_DOOR          = 0x0007,
    MI_EVT_LOCK          = 0x000B,

    MI_STA_BASE          = 0x1000,
    MI_STA_BUTTON        = 0x1001,
    MI_STA_SLEEP         = 0x1002,
    MI_STA_RSSI          = 0x1003,
    MI_STA_TEMPERATURE   = 0x1004,
    MI_STA_WATER_BOIL    = 0x1005,
    MI_STA_HUMIDITY      = 0x1006,
    MI_STA_LUMINA        = 0x1007,
    MI_STA_SOIL_PF       = 0x1008,
    MI_STA_SOIL_EC       = 0x1009,
    MI_STA_BATTERY       = 0x100A,
    MI_STA_LOCK          = 0x100E,
    MI_STA_DOOR          = 0x100F,

} mibeacon_obj_name_t;


typedef union {
    struct{
        uint16_t piid : 9;
        uint16_t flag : 1;
        uint16_t siid : 4;
        uint16_t head : 2;
    }prop;
    struct{
        uint16_t eiid : 9;
        uint16_t flag : 1;
        uint16_t siid : 4;
        uint16_t head : 2;
    }event;
    uint16_t all;
} spec_id_v2_t;


/**
 * @brief   Set adv data with mibeacon and user customized data.
 * @param   [in] is_solicited: set solicite bit.
 *          [in] mesh_stat   : set mesh status.
 *          [in] p_user_data : pointer to user customized data.
 *          [in] user_dlen   : user data length.
 * @return
 *          MI_SUCCESS:            Set successfully.
 *          MI_ERR_DATA_SIZE:      user data length excceed the maximun or 
 *                                 contains invalid adv struct.
 */
mible_status_t mibeacon_adv_data_set(bool is_solicited, uint8_t mesh_stat,
                                     uint8_t *p_user_data, uint8_t user_dlen);

/**
 * @brief   Start adv mibeacon.
 * @param   [in] adv_interval_ms:  adv interval in millsecond.
 *
 * @return
 * @return  MI_SUCCESS             Successfully initiated advertising procedure.
 *          MI_ERR_INVALID_PARAM   Invalid parameter(s) supplied.
 */
mible_status_t mibeacon_adv_start(uint16_t adv_interval_ms);

/**
 * @brief   Stop adv mibeacon.
 *
 * @return  MI_SUCCESS             Successfully stopped advertising procedure.
 *          MI_ERR_INVALID_STATE   Not in advertising state.
 */
mible_status_t mibeacon_adv_stop(void);

/**
 * @brief   Enqueue a object value into the mibeacon object tx queue.
 *
 * @param   [in] nm:  object id name
 *          [in] len: length of the object value
 *          [in] val: pointer to the object value
 *          [in] Bit mask for power saving:
 *               bit 0: Turn off BLE advertising after object sending completed.
 *               bit 1: Using NON CONNECTABLE advertising during object sending procedure.
 *
 * @return  MI_SUCCESS             Successfully enqueued a object into the object queue.
 *          MI_ERR_DATA_SIZE       Object value length is too long.
 *          MI_ERR_RESOURCES       Object queue is full. Please try again later.
 *          MI_ERR_INTERNAL        Can not invoke the sending handler.
 *
 * @note    This function ONLY works when the device has been registered and has restored the keys.
 *
 * The mibeacon object is an adv message contains the status or event. BLE gateway
 * can receive the beacon message (by BLE scanning) and upload it to server for
 * triggering customized home automation scene.
 *
 * OBJ_QUEUE_SIZE      : max num of objects can be concurrency advertising
 *                      ( actually, it will be sent one by one )
 * OBJ_ADV_INTERVAL    : the object adv interval
 * OBJ_ADV_TIMEOUT_MS  : the time one object will be continuously sent.
 *
 */
int mibeacon_obj_enque(uint16_t nm, uint8_t len, void *val, uint8_t stop_adv);


/**
 * @brief   Enqueue a SPEC property into the mibeacon object tx queue.
 *
 * @param   [in] siid: service id
 *          [in] piid: property id
 *          [in] len: length of the property value
 *          [in] val: pointer to the property value
 *          [in] stop_adv: When the object queue is sent out, it will SHUTDOWN BLE advertising
 *
 * @return  MI_SUCCESS             Successfully enqueued a object into the object queue.
 *          MI_ERR_DATA_SIZE       Object value length is too long.
 *          MI_ERR_RESOURCES       Object queue is full. Please try again later.
 *          MI_ERR_INTERNAL        Can not invoke the sending handler.
 *
 * @note    This function ONLY works when the device has been registered and has restored the keys.
 *
 * The mibeacon object is an adv message contains the status or event. BLE gateway
 * can receive the beacon message (by BLE scanning) and upload it to server for
 * triggering customized home automation scene.
 *
 * OBJ_QUEUE_SIZE      : max num of objects can be concurrency advertising
 *                      ( actually, it will be sent one by one )
 * OBJ_ADV_INTERVAL    : the object adv interval
 * OBJ_ADV_TIMEOUT_MS  : the time one object will be continuously sent.
 *
 */
int mibeacon_property_changed(uint8_t siid, uint16_t piid, uint8_t len, void* val, uint8_t stop_adv);
 
/**
 * @brief   Enqueue a SPEC event into the mibeacon object tx queue.
 *
 * @param   [in] siid: service id
 *          [in] eiid: event id
 *          [in] val: user should build properties value into parameter "val" in order
 *          [in] stop_adv: When the object queue is sent out, it will SHUTDOWN BLE advertising
 *
 * @return  MI_SUCCESS             Successfully enqueued a object into the object queue.
 *          MI_ERR_DATA_SIZE       Object value length is too long.
 *          MI_ERR_RESOURCES       Object queue is full. Please try again later.
 *          MI_ERR_INTERNAL        Can not invoke the sending handler.
 *
 * @note    This function ONLY works when the device has been registered and has restored the keys.
 *
 * @note    spec v2 do not support lock event, secure product (eg. lock) should use mibeacon_obj_enque
 *
 * @note    Users should build properties value into parameter "val" in order
 *
 * The mibeacon object is an adv message contains the status or event. BLE gateway
 * can receive the beacon message (by BLE scanning) and upload it to server for
 * triggering customized home automation scene.
 *
 * OBJ_QUEUE_SIZE      : max num of objects can be concurrency advertising
 *                      ( actually, it will be sent one by one )
 * OBJ_ADV_INTERVAL    : the object adv interval
 * OBJ_ADV_TIMEOUT_MS  : the time one object will be continuously sent.
 *
 */
int mibeacon_event_occurred(uint8_t siid, uint16_t eiid, uint8_t len, void* val, uint8_t stop_adv);

#endif
