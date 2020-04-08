/********************************************************************************************************
 * @file     rf_drv.h 
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
#ifndef LINUX
#include "proj/mcu/config.h"
#if(__TL_LIB_8266__ || MCU_CORE_TYPE == MCU_CORE_8266)
#include "rf_drv_8266.h"
#elif(__TL_LIB_8267__ || MCU_CORE_TYPE == MCU_CORE_8267 || \
	  __TL_LIB_8261__ || MCU_CORE_TYPE == MCU_CORE_8261 || \
	  __TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
#include "rf_drv_8267.h"
#elif(__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258 )
#include "drivers/8258/rf_drv.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278 )
#include "drivers/8278/rf_drv.h"
#elif(__TL_LIB_8263__ || MCU_CORE_TYPE == MCU_CORE_8263)
#include "rf_drv_8263.h"
#endif
#endif
