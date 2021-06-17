#pragma once
#include "vendor/common/version.h"    // include mesh_config.h inside.


#define USE_MIBLE_OTA          1
#define USE_GATT_SPEC          0
#define MI_MESH_ENABLED 1
// mi mode enable 
	#define MSC_NONE			   0
	#define MSC_MJSC               1
	#define MSC_MJA1               2
	#define MSC_TYPE 			   MSC_NONE
#if (MSC_TYPE == MSC_MJA1 || MSC_TYPE == MSC_MJSC)
#define HAVE_MSC	MSC_TYPE
#else
#define HAVE_MSC	0
#endif

#define MI_MESH_LIGHT_SUB_ADR	0xfe00
#define MI_MESH_SWITCH_SUB_ADR	0xfe01
#define MI_MESH_PLUG_SUB_ADR	0xfe02
#define MI_MESH_FANS_SUB_ADR	0xfe03
#define MI_MESH_PUB_ADR 		0xfeff
#define MI_YEELIGHT_CT_LIGHT_PID	0x3b4
#define MI_FANS_CTL_PID				0x3b5
#define MI_LESHI_CT_LAMP_PID		2091
#define MI_LEISHI_CT_LIGHT_PID		1369
#define MI_FULIAN_CT_LIGHT_PID		1527
#define MI_LESHI_ONE_ONFF_PID		2007
#define MI_LESHI_TWO_ONOFF_PID		2047
#define MI_LESSHI_THREE_ONFF_PID 	2048
#define MI_LEISHI_TEST_PID			3639
#define MI_SINGLE_ONOFF_BATTERY_PID	889
#define MI_IOT_TELINK_MODE 		1
#if MI_IOT_TELINK_MODE
	#if MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT
#define TL_CUSTOMER_PRODUCT_ID	MI_LEISHI_TEST_PID
	//#define PRODUCT_ID	MI_LEISHI_CT_LIGHT_PID
	//#define PRODUCT_ID	MI_FULIAN_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_LIGHT_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP
#define TL_CUSTOMER_PRODUCT_ID				MI_LESHI_CT_LAMP_PID
#define MI_MESH_SUB_ADR			MI_MESH_LIGHT_SUB_ADR	
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF 
		#if MI_SWITCH_LPN_EN
#define TL_CUSTOMER_PRODUCT_ID				MI_SINGLE_ONOFF_BATTERY_PID
		#else
#define TL_CUSTOMER_PRODUCT_ID				MI_LESHI_ONE_ONFF_PID
		#endif
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF 
#define TL_CUSTOMER_PRODUCT_ID				MI_LESHI_TWO_ONOFF_PID
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF
#define TL_CUSTOMER_PRODUCT_ID				MI_LESSHI_THREE_ONFF_PID
#define MI_MESH_SUB_ADR			MI_MESH_SWITCH_SUB_ADR
	#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_PLUG)
#define TL_CUSTOMER_PRODUCT_ID				MI_YEELIGHT_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_PLUG_SUB_ADR
	#elif (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_FANS)
#define TL_CUSTOMER_PRODUCT_ID				MI_FANS_CTL_PID
#define MI_MESH_SUB_ADR			MI_MESH_FANS_SUB_ADR
	#else
#define TL_CUSTOMER_PRODUCT_ID				MI_YEELIGHT_CT_LIGHT_PID
#define MI_MESH_SUB_ADR			MI_MESH_PLUG_SUB_ADR
	#endif
		
#else
#define TL_CUSTOMER_PRODUCT_ID              889//develop board 
#endif

#define PRODUCT_ID              TL_CUSTOMER_PRODUCT_ID
//#define MODEL_NAME              "xiaomi.dev.ble"
#define MODEL_NAME              "lemesh.light.test04"

#if (TL_CUSTOMER_PRODUCT_ID == MI_LEISHI_TEST_PID)
#define LS_TEST_ENABLE 			1
#else
#define LS_TEST_ENABLE 			0
#endif
#define MAX_ATT_MTU		132
//extern unsigned int ota_program_offset;
//extern unsigned int  ota_firmware_size_k;
//#define DFU_NVM_START          (ota_program_offset)
//#define DFU_NVM_SIZE           (ota_firmware_size_k<<10)
#define STORE_DFU_IDX_EVERY_N_FRAG  4


