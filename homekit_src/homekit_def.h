/********************************************************************************************************
 * @file     homekit_def.h 
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

#ifndef HOMEKIT_DEF_H_
#define HOMEKIT_DEF_H_


typedef struct {
    unsigned short property;
    unsigned short reserved;
    const unsigned char * format;
    unsigned char desc_length;
    unsigned char range_length;
    unsigned char step_length;
    unsigned char reserved2;
    const unsigned char * desc;
    const unsigned char * range;
    const unsigned char * step;
    unsigned char* broadcast_interval_flag;
} hap_characteristic_desc_t;

typedef struct {
    unsigned int instanceID;
    unsigned short service_property;
    unsigned short linked_services_length;
    const unsigned short ** linked_services;
//-    const unsigned char * valid_values_descriptor;
//-    const unsigned char * valid_values_range_descriptor;
} hap_service_desc_t;

#endif /* HOMEKIT_DEF_H_ */
