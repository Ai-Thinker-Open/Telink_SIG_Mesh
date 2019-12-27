/********************************************************************************************************
 * @file     att_handle_index.h 
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
#ifndef _ATT_HANDLE_INDEX_
#define _ATT_HANDLE_INDEX_

////////////////////////////////////////////////////////////////////
// corrseponding to my_Attributes[]
#if VIN_TEST_MODE 
#define MAX_ATT_FUNC				74
#else
#define	MAX_ATT_FUNC                  (65+7+1)  //
#endif 

#define HID_HANDLE_CONSUME_CONTROL_IN (7+7)     //14
#define HID_HANDLE_KEYBOARD_REPORT    (18+7)    //25

#define AUDIO_HANDLE_MIC              (36+7)    //43 mic_data

//Battery
#define HANDLE_BATTERY_VALUE          (33+7)    //0x28, 1bytes

//Immediate Alert
#define HANDLE_ALERT_LEVEL            (46+7)    //0x35

//Device Information Service, STR for ASCII char
#define HANDLE_SYS_ID                 (49+7)    //0x38, 1byte
#define HANDLE_MODEL_NO_STR           (51+7)    //0x3a
#define HANDLE_SERIAL_NO_STR          (53+7)    //0x3c
#define HANDLE_FW_VER_STR             (55+7)    //0x3e
#define HANDLE_HW_VER_STR             (57+7)    //0x40
#define HANDLE_SW_VER_STR             (59+7)    //0x42
#define HANDLE_MANU_NAME_STR          (61+7)    //0x44
#define HANDLE_CERT_DATA_STR          (63+7)    //0x46 IEEE
#define HANDLE_PNP_ID                 (65+7)    //0x48


#define CONTROL_START                 0xF0
#define CONTROL_PAIR                  0xF0
#define CONTROL_UNPAIR                0xF1
#define CONTROL_OTA		              0xF8
#define CONTROL_REBOOT		          0xF9

// cmd reg  :0x8000
// len      :0x8002~0x8003
// data     :0x8008~0x800f


#endif
