/********************************************************************************************************
 * @file     mesh_common.c 
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

#include "proj/tl_common.h"
#include "proj_lib/ble/ll/ll.h"

#if(BEACON_ENABLE)
#if 0
//apple ibeacon
typedef struct{
    u8 flag_len; //constant, 0x02
    u8 flag_type;//constant, 0x01, flags type
    u8 flag; //constant, 0x06, flags indicating LE General discoverable and not supporting BR/EDR
    u8 ibeacon_len;//constant, 0x1A
    u8 manu_spec; //constant, 0xFF
    u16 company_ID;//constant,0X004C, Apple Inc
    u16 beacon_type; //constant,0X1502
    u8 proximity_UUID[16];//Defined by customer
    u16 major;//Defined by Customer
    u16 minor;//Defined by Customer
    s8 measured_power;//Defined by Customer, TX Power-41
}ibeacon_adv_t;
#endif

//apple ibeacon
u8 ibeacon[30] = {
	0x02, 0x01, 0x06,		// not connect
	0x1A, // ibeacon_len
	0xFF, // manu_spec
	0x4C, 0x00, // company_ID 0x004C=apple
	0x02, 0x15, // beacon_type + remaining bytes
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // proximity_UUID
	0x27, 0x33, // major
	0xBA, 0x1F, // minor
	(7 - 57),			// measured_power: just demo power value, Measured power one meter distance,
};

// google eddystone
u8 eddystone_uid[31] = {
                    0x02, 0x01, 0x06,       // not connect
                    0x03, 0x03, 0xAA, 0xFE, // uuid
                    0x17, 0x16, 0xAA, 0xFE, // UID type's len is 0x17
                        0x00,               // UID type
                        0x08,               // tx power
                        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, // NID
                        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,                         // BID
                        0x00, 0x00          // RFU
                    };
u8 eddystone_url[31] = {
                    0x02, 0x01, 0x06,       // not connect
                    0x03, 0x03, 0xAA, 0xFE, // uuid
                    0x12, 0x16, 0xAA, 0xFE, // URL type's len is variable
                        0x10,               // URL type
                        0x08,               // tx power
                        0x00,               // URL Scheme 0x00-http://www.  0x01-https://www.  0x02-http://  0x03-https://
                        0x74, 0x65, 0x6c, 0x69, 0x6e, 0x6b, 0x2d, 0x73, 0x65, 0x6d, 0x69,// telink-semi
                        0x07,               // 0x07-.com  0x08-.org 0x09-.edu  0x0a-.net............
                    };

u8 eddystone_tlm[31] = {
                    0x02, 0x01, 0x06,       // not connect
                    0x03, 0x03, 0xAA, 0xFE, // uuid
                    0x11, 0x16, 0xAA, 0xFE, // TLM type's len is 0x11
                        0x20,               // TLM type
                        0x00,               // TLM version
                        0x00, 0x00,         // Battery voltage 1mV/bit
                        0x00, 0x80,         // Temperature
                        0x00, 0x00, 0x00, 0x00, // ADV_CNT
                        0x00, 0x00, 0x00, 0x00, // SEC_CNT unit:0.1s
                    };

u8 beacon_len = 0;
extern u8 tbl_mac[];
STATIC_ASSERT(BEACON_INTERVAL >= 10 && BEACON_INTERVAL%10 == 0);

/**
 * @brief   This function prepare the beacon data into TX buffer.
 * @param   p - pointer of TX buffer.
 * @return  1:need to send a beacon  0:idle.
 */
int pre_set_beacon_to_adv(rf_packet_adv_t *p){
	static u32 last_time = 0;
	static u8 beacon_idx = 0;
	// send one beacon packet every BEACON_INTERVAL+random(10-20) ms 
	if(clock_time_exceed(last_time, BEACON_INTERVAL*1000)){
		last_time = clock_time()|1;
		if(beacon_idx == 0){// apple ibeacon
			beacon_idx = 1;
			beacon_len = 30;
			memcpy(p->data, ibeacon, beacon_len);
		}else if(beacon_idx == 1){// eddystone UID
			beacon_idx = 2;
			beacon_len = 31;
			memcpy(eddystone_uid + 23, tbl_mac, 6);
			memcpy(p->data, eddystone_uid, beacon_len);
		}else if(beacon_idx == 2){// eddystone URL
			beacon_idx = 3;
			beacon_len = eddystone_url[0]+eddystone_url[3]+eddystone_url[7]+3;
			memcpy(p->data, eddystone_url, beacon_len);
		}else if(beacon_idx == 3){// eddystone TLM
			beacon_idx = 0;
			beacon_len = 25;
			memcpy(p->data, eddystone_tlm, beacon_len);
		}
		
		p->dma_len = beacon_len + 8;
		p->header.type = LL_TYPE_ADV_NONCONN_IND;//Set ADV type to non-connectable
		p->rf_len = beacon_len + 6;
		memcpy(p->advA, tbl_mac, 6);
		
		return 1;
 	}
 	
 	return 0;
}


#endif

 

