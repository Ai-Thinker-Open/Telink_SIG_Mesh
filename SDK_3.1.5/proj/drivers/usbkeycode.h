/********************************************************************************************************
 * @file     usbkeycode.h 
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

#define VK_NONE        		0x00

// function key bitmap
#define VK_MSK_CTRL			0x01
#define VK_MSK_SHIFT		0x02
#define VK_MSK_ALT			0x04
#define VK_MSK_WIN			0x08

#define VK_MSK_LCTRL		0x01
#define VK_MSK_LSHIFT		0x02
#define VK_MSK_LALT			0x04
#define VK_MSK_LWIN			0x08

#define VK_MSK_RCTRL		0x10
#define VK_MSK_RSHIFT		0x20
#define VK_MSK_RALT			0x40
#define VK_MSK_RWIN			0x80

// ordinary keys
#define VK_A				0x04
#define VK_B				0x05
#define VK_C				0x06
#define VK_D				0x07
#define VK_E				0x08
#define VK_F				0x09
#define VK_G				0x0a
#define VK_H				0x0b
#define VK_I				0x0c
#define VK_J				0x0d
#define VK_K				0x0e
#define VK_L				0x0f
#define VK_M				0x10
#define VK_N				0x11
#define VK_O				0x12
#define VK_P				0x13
#define VK_Q				0x14
#define VK_R				0x15
#define VK_S				0x16
#define VK_T				0x17
#define VK_U				0x18
#define VK_V				0x19
#define VK_W				0x1a
#define VK_X				0x1b
#define VK_Y				0x1c
#define VK_Z				0x1d
#define VK_1				0x1e
#define VK_2				0x1f
#define VK_3				0x20
#define VK_4				0x21
#define VK_5				0x22
#define VK_6				0x23
#define VK_7				0x24
#define VK_8				0x25
#define VK_9				0x26
#define VK_0				0x27
#define VK_ENTER			0x28
#define VK_ESC				0x29
#define VK_BACKSPACE		0x2a
#define VK_TAB				0x2b
#define VK_SPACE			0x2c
#define VK_MINUS			0x2d
#define VK_EQUAL			0x2e
#define VK_LBRACE			0x2f
#define VK_RBRACE			0x30
#define VK_BACKSLASH		0x31
#define VK_NUMBER			0x32
#define VK_SEMICOLON		0x33
#define VK_QUOTE			0x34
#define VK_TILDE			0x35
#define VK_COMMA			0x36
#define VK_PERIOD			0x37
#define VK_SLASH			0x38
#define VK_CAPITAL			0x39
#define VK_F1				0x3a
#define VK_F2				0x3b
#define VK_F3				0x3c
#define VK_F4				0x3d
#define VK_F5				0x3e
#define VK_F6				0x3f
#define VK_F7				0x40
#define VK_F8				0x41
#define VK_F9				0x42
#define VK_F10				0x43
#define VK_F11				0x44
#define VK_F12				0x45
#define VK_PRINTSCREEN		0x46
#define VK_SCR_LOCK			0x47
#define VK_PAUSE			0x48
#define VK_INSERT			0x49
#define VK_HOME				0x4a
#define VK_PAGE_UP			0x4b
#define VK_DELETE			0x4c
#define VK_END				0x4d
#define VK_PAGE_DOWN		0x4e
#define VK_RIGHT			0x4f
#define VK_LEFT				0x50
#define VK_DOWN				0x51
#define VK_UP				0x52
#define VK_NUM_LOCK			0x53
#define VKPAD_SLASH			0x54
#define VKPAD_ASTERIX		0x55
#define VKPAD_MINUS			0x56
#define VKPAD_PLUS			0x57
#define VKPAD_ENTER			0x58
#define VKPAD_1				0x59
#define VKPAD_2				0x5a
#define VKPAD_3				0x5b
#define VKPAD_4				0x5c
#define VKPAD_5				0x5d
#define VKPAD_6				0x5e
#define VKPAD_7				0x5f
#define VKPAD_8				0x60
#define VKPAD_9				0x61
#define VKPAD_0				0x62
#define VKPAD_PERIOD		0x63
#define VK_K45				0x64
#define VK_APP				0x65
// below KEY is for ELAN's application matrix
#define VK_C9R1				0xf0	//C9R1 00
#define VK_C9R6				0xf1	//C9R6 000
#define VK_RMB				0xf2	//C7R3
#define VK_EURO				0xf3	//C0R2
#define VK_MMODE			0xf4	//C9R4

#define VK_K107				0x85	//ok
#define VK_K56				0x87	//ok
#define VK_ROMA				0x88	//ok
#define	VK_K14				0x89	//ok
#define VK_CHG				0x8a	//ok
#define VK_NCHG				0x8b	//ok
#define VK_KCR				0x90	//ok,K151
#define VK_KCL				0x91	//ok,K150

//  NOT standard,  use these reserved code to distinguish ctrol keys
#ifndef CTRL_SHIFT_E0E7
#define CTRL_SHIFT_E0E7		1
#endif

#if		CTRL_SHIFT_E0E7
#define	VK_CTRL				0xe0
#define	VK_SHIFT			0xe1
#define	VK_ALT				0xe2
#define VK_WIN				0xe3
#define	VK_RCTRL			0xe4
#define	VK_RSHIFT			0xe5
#define	VK_RALT				0xe6
#define VK_RWIN				0xe7
#else
#define	VK_CTRL				0x90
#define	VK_SHIFT			0x91
#define	VK_ALT				0x92
#define VK_WIN				0x93
#define	VK_RCTRL			0x94
#define	VK_RSHIFT			0x95
#define	VK_RALT				0x96
#define VK_RWIN				0x97
#endif

enum{
    VK_EXT_START = 			0xa0,

    VK_SYS_START = 			VK_EXT_START,	//0xa0
    VK_SLEEP = 				VK_SYS_START,	//0xa0,	sleep
    VK_POWER,								//0xa1,	power
    VK_WAKEUP,								//0xa2, wake-up
//    VK_MCE_STR,							//0xa3
//    VK_MY_MUSIC,							//0xa4
    VK_SYS_END,								//0xa3
	VK_SYS_CNT =			(VK_SYS_END - VK_SYS_START),//0xa3-0xa0=0x03

    VK_MEDIA_START = 		VK_SYS_END,		//0xa3
    VK_W_SRCH = 			VK_MEDIA_START,	//0xa3
    VK_WEB,									//0xa4
    VK_W_BACK,
    VK_W_FORWRD,
    VK_W_STOP,
    VK_W_REFRESH,
    VK_W_FAV,								//0xa9
    VK_MEDIA,
    VK_MAIL,
    VK_CAL,
    VK_MY_COMP,
    VK_NEXT_TRK,
    VK_PREV_TRK,
    VK_STOP,       	//b0
    VK_PLAY_PAUSE,
    VK_W_MUTE,
    VK_VOL_UP,
    VK_VOL_DN,


    VK_MEDIA_END,
    VK_EXT_END = 			VK_MEDIA_END,
	VK_MEDIA_CNT = 			(VK_MEDIA_END - VK_MEDIA_START),//0xb5-0xa3=0x12

	VK_ZOOM_IN = 			(VK_MEDIA_END + 1),//0xb6
    VK_ZOOM_OUT ,							//0xb7

    //special key,do it later
    VK_CH_UP = 0xf0,
    VK_CH_DN = 0xf1,
    VK_FAST_FORWARD = 0xf2,
    VK_FAST_BACKWARD = 0xf3,
    VK_W_SHOPPING = 0xf4,
    VK_W_APP_STORE = 0xf5,
    VK_MY_FAVORIT = 0xf6,
    VK_MENU = 0xf7,
    VK_EXIT = 0xf8,
    VK_CONFIRM = 0xf9,
    VK_RETURN = 0xfa,
    VK_VOICE_SEARCH = 0xfb,
    VK_PROGRAM = 0xfc,
    VK_LOW_BATT = 0xfd,
    VK_TV_PLUS = 0xfe,
    VK_TV_MINUS = 0xff,
    VK_IN_OUTPUT = 0xef,
    VK_TV_POWER = 0xee,
    VK_STB_POWER = 0xed,


};
#define	VK_FN				0xff

#define	VK_EXT_LEN           2
typedef struct{
	unsigned char  val[VK_EXT_LEN];
}vk_ext_t;

// mulit-byte keycode for media keys, cannot used directly in c code..for reference
#define VK_POWER_V			0x01
#define VK_SLEEP_V			0x02
#define VK_WAKEUP_V			0x04

#define VK_W_SRCH_V			{0x21,0x02}
#define VK_WEB_V			{0x23,0x02}
#define VK_W_BACK_V			{0x24,0x02}
#define VK_W_FORWRD_V		{0x25,0x02}
#define VK_W_STOP_V			{0x26,0x02}
#define VK_W_REFRESH_V		{0x27,0x02}
// favorite
#define VK_W_FAV_V			{0x2a,0x02}
#define VK_MEDIA_V			{0x83,0x01}
#define VK_MAIL_V			{0x8a,0x01}
// calculator
#define VK_CAL_V			{0x92,0x01}
#define VK_MY_COMP_V		{0x94,0x01}
// next track -- 01(mosue-ep/USB_EDP_MOUSE) 05(len) 03(kb-report-id/USB_HID_KB_MEDIA)
// b5(val) 00 00 00
#define VK_NEXT_TRK_V		{0xb5,0x00}
#define VK_PREV_TRK_V		{0xb6,0x00}
#define VK_STOP_V			{0xb7,0x00}
#define VK_PLAY_PAUSE_V		{0xcd,0x00}
#define VK_W_MUTE_V			{0xe2,0x00}
#define VK_VOL_UP_V			{0xe9,0x00}
#define VK_VOL_DN_V			{0xea,0x00}

