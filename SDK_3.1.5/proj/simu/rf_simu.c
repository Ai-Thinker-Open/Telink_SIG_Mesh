/********************************************************************************************************
 * @file     rf_simu.c 
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
#include "proj_lib/rf_drv.h"
#include "../drivers/rfhw_i.h"
#include "../drivers/rf.h"
#include "../telink_rf/frame.h"

#ifdef WIN32
#include <stdio.h>
#endif

#define RF_PKT_BUFF_CNT        200

_attribute_aligned_(4) static u8 pkt_beacon_req[] = {0,0,0,0,0,0,0,0                // rf phy header
                        ,0,0,0,0,0,RF_PROTO_BYTE,0,0,0,0,0,0,0,0,0,0                // dma_len, and rf_len, proto, vid, gid, did, rsv[4] === 16 bytes
                        ,FRAME_TYPE_ACK,20,0xff,0xff,0xff,0xff,0xff,0xff        // dmac
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e        // smac
                        ,0x8,0x06                            // ethernet type
                        ,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e
                        ,0,0,0,0,0,0,0,0,0,0
                        ,0xc0,0xa8,0x01,0x03
                        ,0x00,0x00,0x00,0x00                // crc == 4 bytes
                        };

_attribute_aligned_(4) static u8 pkt_speaker_req[] = {0,0,0,0,0,0,0,0    // rf phy header
                        ,0,0,0,0,0,RF_PROTO_BYTE,0,0,0,0,0,0,0,0,0,0                // dma_len, and rf_len, proto, vid, gid, did, rsv[4] === 16 bytes
                        ,FRAME_TYPE_AUDIO_PCM,35,5,1,0xff,0xff        // dmac
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e        // smac
                        ,0x8,0x06                            // ethernet type
                        ,0x00,0x01,0x08,0x00,0x06,0x04,0x00,0x01
                        ,0xf0,0xde,0xf1,0x46,0xc0,0x1e
                        ,0,0,0,0,0,0,0,0,0,0,0,0
                        ,0xc0,0xa8,0x01,0x03
                        ,0x00,0x00,0x00,0x00                // crc == 4 bytes
                        };

static u8 *pkt_send_buffer[RF_PKT_BUFF_CNT];
static int pkt_wr = 0;
static int pkt_rd = 0;

//  add test packets into buffer 
#define RF_SIMU_PKT_INIT(pkt)        rf_simu_pkt_init(&pkt[0], ARRAY_SIZE(pkt))
static void rf_simu_pkt_init(u8 *pkt, u32 len){

    // set the packet verification charaters
    u8 * p = pkt;
    p[0] = (u8)len - 2;
    p[4] = 60;        //  == (60 - 110) == -50dbm
    p[p[0]+3] = 0x40;
    p += 8;
    *((u32*)p) = (len - 16);
    p += 4;
    *p = ((u8)len - 16 + 1);

    p += 13;    // len of first element
    *p = (u8)(len - 30);
    assert(len / 4 * 4 == len);    // 保证每个 TLV 都是从 32 bit 开始
    // end: set the packet verification charaters

    int next = pkt_wr + 1;
    if(next >= RF_PKT_BUFF_CNT) next = 0;
    if(next != pkt_rd){
        pkt_send_buffer[pkt_wr] = pkt;
        pkt_wr = next;
    }
}
// init packets with len, mac, ip etc. for convinience
static void rf_simu_pkts_init(void){
//    RF_SIMU_PKT_INIT(pkt_beacon_req);
    RF_SIMU_PKT_INIT(pkt_speaker_req);
}
static int rf_simu_add_recv_pkts(void *data){
    return 0;
}

// emulate receiving packets
extern u8* rf_write_ptr;
void rf_simu_pkt_recv(void *data){
    u8 *pp = (u8*)data;
    // simulation !!,  first copy the packet to the packet buffer, then invoke a irq
    u8 * rx_buf = rf_write_ptr;
    memcpy4((int*)rx_buf, (int*)pp, ROUND_TO_POW2((*(u32*)pp)+4, 4));
    rf_rx_irq_handler();
#if (__DEBUG_PRINT__)
    u8 *p = pp + 12;
    printf("rf_sim:r: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\r\n"
        ,p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],p[16],p[17],p[18],p[19],p[20]);
#endif        
}
static int rf_simu_pkt_recv_timer(void *data){
    if(pkt_rd != pkt_wr){
        u8 *pp = pkt_send_buffer[pkt_rd];
        rf_simu_pkt_recv(pp);
        ++pkt_rd;
        if(pkt_rd >= RF_PKT_BUFF_CNT){
            pkt_rd = 0;
        }
    }
    return 0;
}

static int rf_simu_pkts_hw_send(void *data){
    SetTxMode (rf_channel, RF_CHN_TABLE);
    ++pkt_beacon_req[17];
    rf_add_tx_pkt(((u8*)(&pkt_beacon_req)) + 8);
    return 0;
}

void rf_simu_init(void){
    rf_simu_pkts_init();
    static ev_time_event_t rf_timer = {rf_simu_pkt_recv_timer};
    ev_on_timer(&rf_timer, 100*1000);
//    static ev_time_event_t rf_beacon_timer = {rf_simu_add_recv_pkts};
//    ev_on_timer(&rf_beacon_timer, 80*1000);
//    static ev_time_event_t rf_hw_send = {rf_simu_pkts_hw_send};
//    ev_on_timer(&rf_hw_send, 10*1000);

}


