/********************************************************************************************************
 * @file     user_config.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

#pragma once

#if ANDROID_APP_ENABLE || IOS_APP_ENABLE 
#pragma pack(1)
#endif
#include "proj/mcu/config.h"

#if (__PROJECT_8266_MASTER_KMA_DONGLE__ || __PROJECT_8267_MASTER_KMA_DONGLE__)
	#include "../8267_master_kma_dongle/app_config.h"
#elif (__PROJECT_MESH__ || WIN32)	// include MESH / lib_sig_mesh.
	#if((MCU_CORE_TYPE == MCU_CORE_8269) || WIN32)
	#include "app/app_config_8269.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8258)
	#include "app/app_config_8258.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "app/app_config_8278.h"
	#endif
#elif (__PROJECT_MESH_LPN__)		// include MESH_LPN / lib_sig_mesh_LPN.
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	#include "../mesh_lpn/app_config_8269.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8258)
	#include "../mesh_lpn/app_config_8258.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "../mesh_lpn/app_config_8278.h"
	#endif
#elif (__PROJECT_MESH_SWITCH__)
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	#include "../mesh_switch/app_config_8269.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8258)
	#include "../mesh_switch/app_config_8258.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "../mesh_switch/app_config_8278.h"
	#endif
#elif (__PROJECT_MESH_PRO__ || __PROJECT_MESH_GW_NODE__)
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	#include "../mesh_provision/app_config_8269.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8258)
	#if(__PROJECT_MESH_GW_NODE_HK__)
	#include "../mesh_gw_node_homekit/app_config_8258.h"
	#else
	#include "../mesh_provision/app_config_8258.h"
	#endif
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "../mesh_provision/app_config_8278.h"
	#endif
#elif (__PROJECT_SPIRIT_LPN__)
	#if(MCU_CORE_TYPE == MCU_CORE_8258)
	#include "../spirit_lpn/app_config_8258.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "../spirit_lpn/app_config_8278.h"
	#endif
#elif (__PROJECT_BOOTLOADER__)
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	#include "../boot_loader/app_config_8269.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8258)
	#include "../boot_loader/app_config_8258.h"
	#elif(MCU_CORE_TYPE == MCU_CORE_8278)
	#include "../boot_loader/app_config_8278.h"
	#endif
#else
	#include "mesh/default_config.h"
#endif

