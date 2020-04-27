#ifndef __MI_CONFIG_H__
#define __MI_CONFIG_H__
#include <stdint.h>
#include "mesh/version.h"    // include mesh_config.h inside.

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
#define DEVELOPER_VERSION       0001
#endif
#define MI_MESH_ENABLED 1
#define STR_VAL(str)  #str
#define CONCAT_VERSION(x,y,z,u) STR_VAL(x) "." STR_VAL(y) "." STR_VAL(z) "_" STR_VAL(u)

#if defined(MI_BLE_ENABLED) && (!HAVE_MSC)
#define MIBLE_AUTH_MODE            2
#define MIBLE_LIB_MAJOR            1
#define MIBLE_LIB_MINOR            0
#define MIBLE_LIB_REVISION         1
#elif defined(MI_BLE_ENABLED) && (HAVE_MSC)
#define MIBLE_AUTH_MODE            1
#define MIBLE_LIB_MAJOR            2
#define MIBLE_LIB_MINOR            2
#define MIBLE_LIB_REVISION         6
#elif defined(MI_MESH_ENABLED)
#define MIBLE_AUTH_MODE            1
#define MIBLE_LIB_MAJOR            1
#define MIBLE_LIB_MINOR            3
#define MIBLE_LIB_REVISION         0
#else
#error "No MI_BLE_ENABLED or MI_MESH_ENABLED is defined. Should add one of them in the preprocesser symbols."
#endif

#define MIBLE_LIB_AND_DEVELOPER_VERSION CONCAT_VERSION(MIBLE_LIB_MAJOR, MIBLE_LIB_MINOR, MIBLE_LIB_REVISION, DEVELOPER_VERSION)


/**
 * @note Product identification got from xiaomi IoT developer platform.
 */
#define MI_MESH_LIGHT_SUB_ADR	0xfe00
#define MI_MESH_SWITCH_SUB_ADR	0xfe01
#define MI_MESH_PLUG_SUB_ADR	0xfe02
	
#define MI_MESH_PUB_ADR 		0xfeff


#define MI_YEELIGHT_CT_LIGHT_PID	0x3b4
#define MI_LESHI_CT_LAMP_PID		2091
#define MI_LEISHI_CT_LIGHT_PID		1369
#define MI_FULIAN_CT_LIGHT_PID		1527
#define MI_LESHI_ONE_ONFF_PID		2007
#define MI_LESHI_TWO_ONOFF_PID		2047
#define MI_LESSHI_THREE_ONFF_PID 	2048

#define MI_SINGLE_ONOFF_BATTERY_PID	889
#define MI_IOT_TELINK_MODE 		1
#if MI_IOT_TELINK_MODE
	#if MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT
#define PRODUCT_ID	MI_YEELIGHT_CT_LIGHT_PID
//#define PRODUCT_ID	MI_LEISHI_CT_LIGHT_PID
//#define PRODUCT_ID	MI_FULIAN_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_LIGHT_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP
#define PRODUCT_ID				MI_LESHI_CT_LAMP_PID
#define MI_MESH_SUB_ADR			MI_MESH_LIGHT_SUB_ADR	
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF 
		#if MI_SWITCH_LPN_EN
#define PRODUCT_ID				MI_SINGLE_ONOFF_BATTERY_PID
		#else
#define PRODUCT_ID				MI_LESHI_ONE_ONFF_PID
		#endif
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF 
#define PRODUCT_ID				MI_LESHI_TWO_ONOFF_PID
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF
#define PRODUCT_ID				MI_LESSHI_THREE_ONFF_PID
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_PLUG)
#define PRODUCT_ID				MI_YEELIGHT_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_PLUG_SUB_ADR
	#else
#define PRODUCT_ID				MI_YEELIGHT_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_PLUG_SUB_ADR
	#endif
	
#else
	#define PRODUCT_ID             0x0379//silicon lab pid
#endif

#define DEMO_CER_MODE			0
#define FLASH_CER_MODE			1 

#define MI_MESH_PUB_STEP	6
#define MI_MESH_PUB_VAL		2
#define MI_MESH_GATEWAY_ADR 	1

#if MI_IOT_TELINK_MODE
#define MI_CER_MODE 	DEMO_CER_MODE
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
#define OBJ_ADV_INTERVAL_MS    100
#endif

#ifndef OBJ_ADV_TIMEOUT_MS
#define OBJ_ADV_TIMEOUT_MS     3000
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

/* DEBUG */
#ifndef DEBUG_MIBLE
#define DEBUG_MIBLE            0
#define TIME_PROFILE           0
#endif

#ifndef DFU_NVM_START
#define DFU_NVM_START          0x4A000UL      /**< DFU Buffer Start Address */
#endif

#ifndef DFU_NVM_SIZE
#define DFU_NVM_SIZE           0x34800UL      /**< DFU Buffer Size in bytes */
#endif

typedef struct {
    uint16_t pid;
    uint16_t io;
    uint8_t  have_msc;
    uint8_t  have_reset_button:1;
    uint8_t  schd_in_mainloop :1;
    uint8_t  reserve          :6;
} mi_config_t;

extern const mi_config_t m_config;
#define DEV_SK_FLASH_ADR 	   0x7f000
#endif  /* __MI_CONFIG_H__ */ 


