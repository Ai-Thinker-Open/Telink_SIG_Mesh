/********************************************************************************************************
 * @file     hk_ble_ll_ota.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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

#ifndef HKBLE_LL_OTA_H_
#define HKBLE_LL_OTA_H_

#ifndef	HKBLE_OTA_ENABLE
#define HKBLE_OTA_ENABLE      0
#endif

#ifndef HKBLE_OTA_FW_CHECK_EN

#define HKBLE_OTA_FW_CHECK_EN					1

#if HKBLE_OTA_FW_CHECK_EN
#define HKBLE_OTA_DATA_VERIFY_AFTER_SAVE_EN	1
#endif

#endif

static const unsigned long crc32_half_tbl[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};



#define HKBLE_CMD_OTA_FW_VERSION              0xFF00
#define HKBLE_CMD_OTA_START                   0xFF01
#define HKBLE_CMD_OTA_END                     0xFF02

#define HKBLE_FLAG_FW_CHECK						0x5D
#define HKBLE_FW_CHECK_AGTHM2					0x02


typedef struct{
	u8  ota_start_flag;
#if (HKBLE_OTA_FW_CHECK_EN)
	u8 	fw_check_en;
	u16 fw_crc_last_index;
	u32 fw_crc_init;
#endif
}hkble_ota_service_t;

extern hkble_ota_service_t hkble_blcOta;

extern int hkble_ota_adr_index;
extern u32 hkble_ota_data_timeout;
extern flash_adr_layout_def flash_adr_layout;

typedef void (*hkble_ota_startCb_t)(void);
typedef void (*hkble_ota_vendorCb_t)(void *p);

extern hkble_ota_startCb_t    hkble_otaStartCb;


enum{
	HKBLE_OTA_SUCCESS = 0,     //success
	HKBLE_OTA_PACKET_LOSS,     //lost one or more OTA PDU
	HKBLE_OTA_DATA_CRC_ERR,     //data CRC err
	HKBLE_OTA_WRITE_FLASH_ERR,  //write OTA data to flash ERR
 	HKBLE_OTA_DATA_UNCOMPLETE,  //lost last one or more OTA PDU
 	HKBLE_OTA_TIMEOUT, 		  //
 	HKBLE_OTA_FW_CHECK_ERR,
 	HKBLE_OTA_FW_SIZE_ERR,
 	HKBLE_OTA_DATA_INDEX_ERR,
};

enum{
	HKBLE_OTA_DATA_NONE = 0,
	HKBLE_OTA_DATA_START = (1 << 0),
	HKBLE_OTA_DATA_END = (1 << 1),
};


extern void hkble_setOtaStartCb(hkble_ota_startCb_t cb);
extern void hkble_setCustVendorCmdCb(hkble_ota_vendorCb_t cb);
extern int hkble_otaWrite(void * p);
extern int hkble_otaRead(void * p);
extern void dev_mac_addr_update(void);
extern void hkble_ota_set_flag(void);

#endif /* HKBLE_LL_OTA_H_ */
