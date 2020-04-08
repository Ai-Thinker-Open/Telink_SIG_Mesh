/********************************************************************************************************
 * @file     ip_adapt_i.h 
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

//  IP protocol adaption for different ip stack,  uip-stack only for now

#pragma once
#include "../tl_common.h"

#if(MODULE_ETH_ENABLE)

#include "ethernet.h"
#include "ip_adapt.h"
#include "eth_hw_i.h"
#include "../uip/core/net/uip.h"
#include "../uip/core/net/uip_arp.h"
#include "../uip/apps/dhcp.h"

static inline u8* ipa_get_my_mac(void){
	return &uip_ethaddr.addr[0];
}

static inline u8* ipa_get_my_ip(void){
	return (u8*)(&uip_hostaddr);
}

static inline u8* ipa_get_gateway(void){
	return (u8*)(&uip_draddr);
}

static inline u8* ipa_get_netmask(void){
	return (u8*)(&uip_netmask);
}

static inline int ipa_is_my_ip(u8 * ip){
	u8* my_ip = ipa_get_my_ip();
	assert((((int)ip) >> 2) << 2 == ((int)ip));			// address must alighn to 4
	assert((((int)my_ip) >> 2) << 2 == ((int)my_ip));	// address must alighn to 4
	return *((u32*)ip) == *((u32*)my_ip);
}

static inline int ipa_is_my_mac(u8 * mac){
	u8 *my_mac = ipa_get_my_mac();
	assert((((int)mac) >> 2) << 2 == ((int)mac));			// address must alighn to 4
	assert((((int)my_mac) >> 2) << 2 == ((int)my_mac));	// address must alighn to 4
	return ((*(u32*)mac == *(u32*)my_mac) && (*(u16*)(mac + 4) == *(u16*)(my_mac + 4)));
}

static inline int ipa_is_broadcast_mac(u8 * mac){
	assert((((int)mac) >> 2) << 2 == ((int)mac));			// address must alighn to 4
	return (*((u32*)mac) == U32_MAX && (*(u16*)(mac + 4)) == U16_MAX);	//  same as 0xffffffff,  but faster
}

static inline u8 ipa_send_ip_pkt(void){
	uip_arp_out();
	if(uip_len > 0){
		eth_send_pkt((u8*)uip_aligned_buf, uip_len);
	}
	return 0;
}

static inline void ipa_handle_arp_pkt(u8 *pkt, u32 len){
	uip_aligned_buf = (uip_buf_t*)pkt;
	uip_len = (u16)len;

	uip_arp_arpin(); 
	if(uip_len > 0){ 
		eth_send_pkt(pkt, uip_len);
	}
}

static inline void ipa_handle_ip_pkt(u8 *pkt, u32 len){
	uip_aligned_buf = (uip_buf_t*)pkt;
	uip_len = (u16)len;

	tcpip_input(); 
}

// 用于主动发送数据包的情况，不能跟硬件读写 buffer 冲突
// 用于一开始可能要发送的数据包, arp request, dhcp, etc. 必须尽早初始化
static inline void ipa_set_uip_buf_to_default_tx(void){
	uip_aligned_buf = (uip_buf_t *)(eth_hw_get_tx_buff() + 4);
}

#define UIP_DHCP_ENABLE		0

#if (UIP_DHCP_ENABLE)
PROCESS_NAME(dhcp_process);
#endif
static inline void ipa_init(void){
	ipa_set_uip_buf_to_default_tx();
	
	//MAC/IP for test only, the actual MAC/IP might be read from flash or somewhere else
	u8 my_test_mac[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
	ipa_set_my_mac(&my_test_mac[0]);
#if (!UIP_DHCP_ENABLE)
	u8 my_test_ip[] = {192, 168, 1, 3};
	ipa_set_my_ip(&my_test_ip[0]);
	u8 my_test_netmask[] = {255, 255, 255, 0};
	ipa_set_netmask(&my_test_netmask[0]);
	u8 my_test_gateway[] = {192, 168, 1, 8};
	ipa_set_gateway(&my_test_gateway[0]);
#endif
	process_init();
	process_start(&tcpip_process, NULL);
#if (UIP_DHCP_ENABLE)
	process_start(&dhcp_process, NULL);
	process_post(&dhcp_process, PROCESS_EVENT_DHCP_REQ, NULL);
#endif
	tcpip_set_outputfunc(ipa_send_ip_pkt);				// set the send out packet driver
	uip_arp_init();
}
#endif
