/*
 * homekit_def.h
 *
 *  Created on: 2018-12-27
 *      Author: Administrator
 */

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
