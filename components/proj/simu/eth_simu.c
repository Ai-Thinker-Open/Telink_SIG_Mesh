/********************************************************************************************************
 * @file     eth_simu.c 
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

#include "../tl_common.h"

#if(MODULE_ETH_ENABLE)
#include "../drivers/eth_hw.h"
#include "../drivers/ethernet.h"

#ifdef WIN32
#include <stdio.h>
#endif

static u8 pkt_arp_req[] = {0,0,0,0                            // len
                        ,0xff,0xff,0xff,0xff,0xff,0xff        // dmac
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e        // smac
                        ,0x8,0x06                            // ethernet type
                        ,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e
                        ,0,0,0,0,0,0,0,0,0,0
                        ,0xc0,0xa8,0x01,0x03
                        };

static u8 pkt_ping_req[] = {0,0,0,0                            // len
                        ,0x11,0x11,0x11,0x11,0x11,0x11        // dmac
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e        // smac
                        ,0x08,0x00,0x45,0x00,0x00,0x3c        // ethernet type, ip version, head, length, total len
                        ,0x01,0x9a,0x00,0x00,0x40,0x01,0xf5,0xcb
                        ,0xc0,0xa8,0x01,0x08,0xc0,0xa8,0x01,0x03    // dip, sip
                        ,0x08,0x00,0xfa,0x20,0x00,0x02,0x53,0x39
                        ,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68
                        ,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70
                        ,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62
                        ,0x63,0x64,0x65,0x66,0x67,0x68,0x69
                        };

static int pkt_buffer_count = 0;
static u8 *pkt_send_buffer[200];

//  add test packets into buffer 
#define ETH_SIMU_PKT_INIT(pkt)        eth_simu_pkt_init(&pkt[0], ARRAY_SIZE(pkt))
static void eth_simu_pkt_init(u8 *pkt, u32 len){
    *((u32*)pkt) = (len - 4);
    pkt_send_buffer[pkt_buffer_count++] = pkt;
}
// init packets with len, mac, ip etc. for convinience
static void eth_simu_pkts_init(void){
    ETH_SIMU_PKT_INIT(pkt_arp_req);
    ETH_SIMU_PKT_INIT(pkt_ping_req);
}
// emulate receiving packets
extern u8* eth_write_ptr;
static void eth_simu_pkt_recv(void){
    static int i = 0;
    if(i < pkt_buffer_count){
        u8 *pp = pkt_send_buffer[i];
        // simulation !!,  first copy the packet to the packet buffer, then invoke a irq
        memcpy4((int*)eth_write_ptr, (int*)pp, ROUND_TO_POW2((*(u32*)pp)+4, 4));
        eth_rx_irq_handler();
        ++i;
#if (__DEBUG_PRINT__)
        u32 len = *(u32*)pp;
        u8 *p = pp + 4;
        printf("eth_sim:r:len=%d, %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n"
            ,len, p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],p[16],p[17],p[18],p[19],p[20]);
#endif        
    }
}

static void eth_simu_pkt_hw_send(void){
    static u8 eth_pkt[] = {0x58,2,0,0,0xff,0xff,0xff,0xff,0xff,0xff,0x11,0x11,0x11,0x11,0x11,0x11,0x08,0x00};
    eth_hw_send_pkt((u8*)eth_pkt);
#if (__DEBUG_PRINT__)
    u32 len = *(u32*)(&eth_pkt[0]);
    u8 *p = (&eth_pkt[0]) + 4;
    printf("eth_sim:s:len=%d, %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n"
        ,len, p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],p[16],p[17],p[18],p[19],p[20]);
#endif        
}

static int eth_simu_timer(void *data){
    eth_simu_pkt_recv();
    eth_simu_pkt_hw_send();
    return 0;
}

void eth_simu_init(void){
    eth_simu_pkts_init();

    static ev_time_event_t eth_timer = {eth_simu_timer};
    ev_on_timer(&eth_timer, 1000*1000);    //  1 sec
}

#endif

