/********************************************************************************************************
 * @file     usb_simu.c 
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
#include "../drivers/usbhw.h"
#include "../drivers/usb.h"
#include "../app/usbmouse.h"
#include "../app/usbkb.h"

#if(RF_PROTOCOL == RF_PROTO_PROPRIETARY)
#endif

#if WIN32
#include <stdio.h>
#endif

static u8 pkt_get_device_desc_req[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0x80,0x06,0x00,0x01,0x00,0x00,0x40,0x00        //
                        };

static u8 pkt_get_device_desc_req2[] = {0,0,0,0                // len (2 bytes), endpoint (2 bytes)
                        ,0x80,0x06,0x00,0x01,0x00,0x00,0x12,0x00        //
                        };
static u8 pkt_get_conf_desc_req[] = {0,0,0,0                // len (2 bytes), endpoint (2 bytes)
                        ,0x80,0x06,0x00,0x02,0x00,0x00,0x00,0x02        //
                        };
static u8 pkt_get_set_conf_req[] = {0,0,0,0                // len (2 bytes), endpoint (2 bytes)
                        ,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00        //
                        };
static u8 pkt_get_string_req[] = {0,0,0,0                // len (2 bytes), endpoint (2 bytes)
                        ,0x80,0x06,0x03,0x03,0x09,0x04,0xff,0x00        //
                        };
static u8 pkt_get_vol_cur[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0xa1,0x81,0x00,0x01,0x00,0x02,0x01,0x00        //
                        };
static u8 pkt_set_sampling[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0x22,0x01,0x00,0x01,0x87,0x00,0x03,0x00,0x39,0x71        //
                        };

static u8 pkt_get_sampling[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0xA2,0x81,0x00,0x01,0x87,0x00,0x03,0x00        //
                        };

static u8 pkt_get_cur2[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0xa1,0x81,0x00,0x01,0x01,0x05,0x01,0x00        //
                        };
static u8 pkt_set_intf[] = {0,0,0,0                            // len (2 bytes), endpoint (2 bytes)
                        ,0x01,0x0B,0x01,0x00,0x02,0x00,0x00,0x00        //
                        };
static int pkt_buffer_count = 0;
static u8 *pkt_send_buffer[200];

u8 usb_sim_ctrl_ep_buffer[128];
u8 usb_sim_ctrl_ep_ptr;
u8 usb_sim_eps_buffer[8][128];
u8 usb_sim_eps_ptr[8];

//  add test packets into buffer
#define USB_SIMU_PKT_INIT(pkt)        usb_simu_pkt_init(&pkt[0], ARRAY_SIZE(pkt))
static void usb_simu_pkt_init(u8 *pkt, u32 len){
    *((u16*)pkt) = (u16)(len - 4);                                // len, endpoint
    pkt_send_buffer[pkt_buffer_count++] = pkt;
}
// init packets with len, mac, ip etc. for convinience
static void usb_simu_pkts_init(void){
//    USB_SIMU_PKT_INIT(pkt_get_device_desc_req);
//    USB_SIMU_PKT_INIT(pkt_get_device_desc_req2);
//    USB_SIMU_PKT_INIT(pkt_get_conf_desc_req);
//    USB_SIMU_PKT_INIT(pkt_get_set_conf_req);
//    USB_SIMU_PKT_INIT(pkt_get_string_req);
//    USB_SIMU_PKT_INIT(pkt_get_vol_cur);
//    USB_SIMU_PKT_INIT(pkt_set_sampling);
//    USB_SIMU_PKT_INIT(pkt_get_sampling);
//    USB_SIMU_PKT_INIT(pkt_get_cur2);
    USB_SIMU_PKT_INIT(pkt_set_intf);

}

// emulate receiving packets
extern u8* usb_write_ptr;
static int usb_simu_pkt_send_timer(void *data){
    static int i = 0;
    if(i < pkt_buffer_count){
        u8 *pp = pkt_send_buffer[i];
        u16 len = (*(u16*)pp);
        if(0 == pp[2]){                // endpoint number
            memcpy4((int*)&usb_sim_ctrl_ep_buffer[0], (int*)(pp + 4), ROUND_TO_POW2(len, 4));
            usb_sim_ctrl_ep_ptr = 0;
            reg_ctrl_ep_irq_sta = FLD_CTRL_EP_IRQ_SETUP;
        }else if(8 == pp[2]){
            memcpy4((int*)&usb_sim_eps_buffer[0], (int*)(pp + 4), ROUND_TO_POW2(len, 4));
            usb_sim_eps_ptr[0] = 0;
        }else{
            memcpy4((int*)&usb_sim_eps_buffer[pp[2]], (int*)(pp + 4), ROUND_TO_POW2(len, 4));
            usb_sim_eps_ptr[pp[2]] = 0;
        }
        ++i;
#if (__DEBUG_PRINT__)
        u8 *p = pp + 4;
        printf("\r\nusb_sim:r: ");
        foreach(j, len){
            printf("%02x,",p[j]);
        }
        printf("\r\n");
#endif
        return 0;
    }
    return -1;
}

static int usb_simu_mouse_report(void *data){
    static int ind = 0;
    static u8 p[] = {0,0,0,0};    //  X, Y, wheel, btn
    p[0] = 1;
    if(ind < 20){
        //p[0] = 10;
        p[1] = 1;
        //p[2] = 0;
        //p[3] = 0;
    }else if(ind < 40){
        //p[0] = 10;
        p[1] = -1;
        //p[2] = -1;
        //p[3] = 0;
    }else if(ind < 60){
        //p[0] = -10;
        p[1] = 1;
        //p[2] = 0;
        //p[3] = 0;
    }else{
        //p[0] = 10;
        p[1] = -1;
    }
    //printf("mouse report: %d,%d,%d,%d\r\n",p[0], p[1], p[2], p[3]);
    usbmouse_hid_report(USB_HID_MOUSE, p, 4);
    // redo a round
    if(ind++ > 80){
        ind = 0;
    }
    return 0;
}

#include "../drivers/usbkeycode.h"
//extern void usbkb_hid_report(kb_data_t *data);
static int usb_simu_kb_report(void *data){
    kb_data_t kb_data = {1, 0, {4}};
//    usbkb_hid_report(&kb_data);
//     usbkb_report_media_key(VK_VOL_UP);
    return 0;
}

#if(RF_PROTOCOL == RF_PROTO_PROPRIETARY)
extern void usbmouse_add_frame (rf_packet_mouse_t *packet_mouse);
void usb_simu_mouse_frame(void){
    rf_packet_mouse_t mouse_data;
    memset(&mouse_data,0,sizeof(mouse_data));
#if 0
    mouse_data.odi = 0;
    mouse_data.cnt = 1;
    mouse_data.data[0].btn = 1;
    mouse_data.data[0].x = 2;
    mouse_data.data[0].y = 3;
    mouse_data.data[0].w = 4;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 0;
    mouse_data.cnt = 2;
    mouse_data.data[1].btn = 5;
    mouse_data.data[1].x = 6;
    mouse_data.data[1].y = 7;
    mouse_data.data[1].w = 8;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 0;
    mouse_data.cnt = 3;
    mouse_data.data[2].btn = 11;
    mouse_data.data[2].x = 12;
    mouse_data.data[2].y = 13;
    mouse_data.data[2].w = 14;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 0;
    mouse_data.cnt = 0;
    mouse_data.data[3].btn = 15;
    mouse_data.data[3].x = 16;
    mouse_data.data[3].y = 17;
    mouse_data.data[3].w = 18;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 1;
    mouse_data.ods = 1;
    mouse_data.data[0].btn = 21;
    mouse_data.data[0].x = 22;
    mouse_data.data[0].y = 23;
    mouse_data.data[0].w = 24;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 2;
    mouse_data.ods = 2;
    mouse_data.data[1].btn = 31;
    mouse_data.data[1].x = 32;
    mouse_data.data[1].y = 33;
    mouse_data.data[1].w = 34;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 3;
    mouse_data.ods = 2;
    mouse_data.data[3].btn = 41;
    mouse_data.data[3].x = 42;
    mouse_data.data[3].y = 43;
    mouse_data.data[3].w = 44;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 4;
    mouse_data.ods = 3;
    mouse_data.data[3].btn = 41;
    mouse_data.data[3].x = 42;
    mouse_data.data[3].y = 43;
    mouse_data.data[3].w = 44;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 255;
    mouse_data.ods = 3;
    mouse_data.data[2].btn = 51;
    mouse_data.data[2].x = 52;
    mouse_data.data[2].y = 53;
    mouse_data.data[2].w = 54;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 0;
    mouse_data.ods = 0;
    mouse_data.data[0].btn = 61;
    mouse_data.data[0].x = 62;
    mouse_data.data[0].y = 63;
    mouse_data.data[0].w = 64;
    mouse_data.data[1].btn = 71;
    mouse_data.data[1].x = 72;
    mouse_data.data[1].y = 73;
    mouse_data.data[1].w = 74;
    mouse_data.data[2].btn = 81;
    mouse_data.data[2].x = 82;
    mouse_data.data[2].y = 83;
    mouse_data.data[2].w = 84;
    mouse_data.data[3].btn = 91;
    mouse_data.data[3].x = 92;
    mouse_data.data[3].y = 93;
    mouse_data.data[3].w = 94;
    usbmouse_add_frame(&mouse_data);

    mouse_data.odi = 1;
    mouse_data.ods = 1;
    usbmouse_add_frame(&mouse_data);
    mouse_data.odi = 2;
    mouse_data.ods = 2;
    usbmouse_add_frame(&mouse_data);
    mouse_data.odi = 3;
    mouse_data.ods = 3;
    usbmouse_add_frame(&mouse_data);
    mouse_data.odi = 4;
    mouse_data.ods = 0;
    usbmouse_add_frame(&mouse_data);
    mouse_data.odi = 5;
    mouse_data.ods = 1;
    usbmouse_add_frame(&mouse_data);
    mouse_data.odi = 5;
    mouse_data.ods = 2;
    usbmouse_add_frame(&mouse_data);
#endif
}
#endif

void usb_simu_init(void){
    usb_simu_pkts_init();

#if(RF_PROTOCOL == RF_PROTO_PROPRIETARY)
    //usb_simu_mouse_frame();
#endif
#ifndef WIN32
    static ev_time_event_t usb_timer = {usb_simu_pkt_send_timer};
    ev_on_timer(&usb_timer, 1000*1000);
#endif
//    static ev_time_event_t usb_rpt = {usb_simu_mouse_report};
//    ev_on_timer(&usb_rpt, 1000*8);

//    static ev_time_event_t kb_rpt = {usb_simu_kb_report};
//    ev_on_timer(&kb_rpt, 1500*1000);
}


