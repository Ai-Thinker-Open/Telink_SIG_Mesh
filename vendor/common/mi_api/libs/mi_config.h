#ifndef __MI_CONFIG_H__
#define __MI_CONFIG_H__
#include <stdint.h>

#if defined(CUSTOMIZED_MI_CONFIG_FILE)
#include CUSTOMIZED_MI_CONFIG_FILE
#endif

/**
 * @note MIBLE_LIB_AND_DEVELOPER_FIRMWARE_VERSION
 *
 * The MIBLE SDK major, minor and revision MUST NOT be changed.
 * And DEVELOPER_VERSION will identify developer firmware version.
 * e.g. x.y.z_d
 */

#ifndef DEVELOPER_VERSION
#define DEVELOPER_VERSION           0001
#endif

#define STR_VAL(str)  #str
#define CONCAT_DEVELOPER_VERSION(x) STR_VAL(x)
#define CONCAT_LIB_VERSION(x,y,z)   STR_VAL(x) "." STR_VAL(y) "." STR_VAL(z) "_"
#define CONCAT_VERSION(x,y,z,u)     STR_VAL(x) "." STR_VAL(y) "." STR_VAL(z) "_" STR_VAL(u)

#if defined(MI_BLE_ENABLED) && (!HAVE_MSC)
#define MIBLE_AUTH_MODE            2
#define MIBLE_LIB_MAJOR            1
#define MIBLE_LIB_MINOR            1
#define MIBLE_LIB_REVISION         4
#elif defined(MI_BLE_ENABLED) && (HAVE_MSC)
#define MIBLE_AUTH_MODE            1
#define MIBLE_LIB_MAJOR            2
#define MIBLE_LIB_MINOR            3
#define MIBLE_LIB_REVISION         3
#elif defined(MI_MESH_ENABLED)
#define MIBLE_AUTH_MODE            1
#define MIBLE_LIB_MAJOR            1
#define MIBLE_LIB_MINOR            4
#define MIBLE_LIB_REVISION         5
#if (HAVE_MSC==0)
#define HAVE_OTP_PKI               1
#else
#define HAVE_OTP_PKI               0
#endif
#else
//#error "No MI_BLE_ENABLED or MI_MESH_ENABLED is defined. Should add one of them in the preprocesser symbols."
#endif

#define MIBLE_DEVELOPER_VERSION         CONCAT_DEVELOPER_VERSION(DEVELOPER_VERSION)
#define MIBLE_LIB_VERSION               CONCAT_LIB_VERSION(MIBLE_LIB_MAJOR, MIBLE_LIB_MINOR, MIBLE_LIB_REVISION)
#define MIBLE_LIB_AND_DEVELOPER_VERSION CONCAT_VERSION(MIBLE_LIB_MAJOR, MIBLE_LIB_MINOR, MIBLE_LIB_REVISION, DEVELOPER_VERSION)

/**
 * @note Product identification got from xiaomi IoT developer platform.
 */
#ifndef PRODUCT_ID
#if defined(MI_BLE_ENABLED) && (HAVE_MSC)
#define PRODUCT_ID              463
#elif defined(MI_BLE_ENABLED)
#define PRODUCT_ID              156
#else
#define PRODUCT_ID              889
#endif
#endif

/**
 * @note Model name got from xiaomi IoT developer platform.
 */
#define DEFAULT_MODEL			"xiaomi.dev.ble"

#ifndef MODEL_NAME
#define MODEL_NAME              DEFAULT_MODEL
#endif


/**
 * @note mibeacon object advertising configuration
 *
 * The mibeacon object is an adv message contains the status or event. BLE gateway
 * can receive the beacon message (by active scanning) and upload it to server for
 * triggering customized home automation scene.
 *
 * OBJ_QUEUE_SIZE      : max num of objects can be concurrency advertising
 *                       ( actually, it will be sent one by one )
 * OBJ_ADV_INTERVAL_MS : the object adv interval
 * OBJ_ADV_TIMEOUT_MS  : the time one object will be continuously sent.
 */

#ifndef OBJ_QUEUE_SIZE
#define OBJ_QUEUE_SIZE         8
#endif

#ifndef OBJ_ADV_INTERVAL_MS
#define OBJ_ADV_INTERVAL_MS    50
#endif

#ifndef OBJ_ADV_TIMEOUT_MS
#define OBJ_ADV_TIMEOUT_MS     1500
#endif


/**
 * @note To support Secure Auth procedure MUST have mijia secure chip (MSC).
 *      NONE   : 0
 *      MJSC   : 1
 *      MJA1   : 2
 */
#ifndef HAVE_MSC
#define HAVE_MSC               0
#endif


/**
 * @note Device side has RESET button.
 */
#ifndef HAVE_RESET_BUTTON
#define HAVE_RESET_BUTTON      1
#endif

/**
 * @note Device side has bind confirm button.
 */
#ifndef HAVE_CONFIRM_BUTTON
#define HAVE_CONFIRM_BUTTON    1
#endif

/**
 * @note Which OOB methods device used in authentication procedure.
 *       OOB_USE_NUMPAD_INPUT  (bit 0): device has a numeric keypad to enter the pairing code.
 *       OOB_USE_QR_CODE_OUT   (bit 4): device provided with a QR code label.
 *       OOB_USE_DISPLAT_OUT   (bit 7): device displayed a six digit number.
 */
#ifndef OOB_USE_NUMPAD_INPUT
#define OOB_USE_NUMPAD_INPUT   0
#endif

#ifndef OOB_USE_QR_CODE_OUT
#define OOB_USE_QR_CODE_OUT    0
#endif

#ifndef OOB_USE_DISPLAT_OUT
#define OOB_USE_DISPLAT_OUT    0
#endif

#define OOB_IO_CAP             \
(OOB_USE_NUMPAD_INPUT | (OOB_USE_DISPLAT_OUT)<<4 | (OOB_USE_QR_CODE_OUT)<<7)


/**
 * @note Process mi scheduler in main loop (non-interrupt context).
 */
#ifndef MI_SCHD_PROCESS_IN_MAIN_LOOP
#define MI_SCHD_PROCESS_IN_MAIN_LOOP    1
#endif


/**
 * @note Use Mi BLE OTA Protocol.
 */
#ifndef USE_MIBLE_OTA
#define USE_MIBLE_OTA          1
#endif

/**
 * @note Use Mi BLE MCU OTA Protocol.
 */
#ifndef USE_MCU_OTA
#define USE_MCU_OTA           1
#endif

/**
 * @note Use Mi BLE WiFi Access Protocol.
 */
#ifndef USE_MIBLE_WAC
#define USE_MIBLE_WAC          0
#endif

/**
 * @note Use GATT MIoT-Spec.
 */
#ifndef USE_GATT_SPEC
#define USE_GATT_SPEC          1
#endif

#ifndef DFU_NVM_START
#define DFU_NVM_START          0x4A000UL      /**< DFU Buffer Start Address */
#endif

#ifndef DFU_NVM_SIZE
#define DFU_NVM_SIZE           0x34800UL      /**< DFU Buffer Size in bytes */
#endif

#ifndef MAX_ATT_MTU
#define MAX_ATT_MTU            247
#endif

#define MAX_ATT_PAYLOAD       (MAX_ATT_MTU-3)

#ifndef STORE_DFU_IDX_EVERY_N_FRAG
#define STORE_DFU_IDX_EVERY_N_FRAG 0
#endif

/* DEBUG */
#ifndef DEBUG_MIBLE
#define DEBUG_MIBLE            0
#define TIME_PROFILE           0
#define RXFER_VERBOSE          0
#endif


typedef struct {
    const char *   developer_version;
    const char *   model;
    uint16_t pid;
    uint16_t io;
    uint8_t  have_msc;
    uint8_t  have_reset_button  :1;
    uint8_t  have_confirm_button:1;
    uint8_t  schd_in_mainloop   :1;
    uint8_t  reserve            :5;
    uint8_t  max_att_payload;
    uint32_t dfu_start;
    uint32_t dfu_size;
} mi_config_t;

extern mi_config_t m_config;

#endif  /* __MI_CONFIG_H__ */ 


