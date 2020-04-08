/********************************************************************************************************
 * @file     mouse_simu.c 
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
#include "../os/ev.h"
#include "../drivers/mouse.h"


//STATIC_ASSERT(IMPLIES(__MOUSE_SIMU__, MODULE_MOUSE_ENABLE));

s8 mouse_simu_x;
s8 mouse_simu_y;
s32 mouse_simu_wheel_value;
u8 mouse_simu_btn;

// from cos 1 to cos 180
const int cos_table[] =
{
    10000, 9998,9993,9986,9975,9961,9945,9925,9902,9876,9848,
    9816,9781,9743,9702,9659,9612,9563,9510,9455,9396,
    9335,9271,9205,9135,9063,8987,8910,8829,8746,8660,
    8571,8480,8386,8290,8191,8090,7986,7880,7771,7660,
    7547,7431,7313,7193,7071,6946,6819,6691,6560,6427,
    6293,6156,6018,5877,5735,5591,5446,5299,5150,5000,
    4848,4694,4539,4383,4226,4067,3907,3746,3583,3420,
    3255,3090,2923,2756,2588,2419,2249,2079,1908,1736,
    1564,1391,1218,1045,871,697,523,348,174,0,    
    -174,-348,-523,-697,-871,-1045,-1218,-1391,-1564,-1736,
    -1908,-2079,-2249,-2419,-2588,-2756,-2923,-3090,-3255,-3420,
    -3583,-3746,-3907,-4067,-4226,-4383,-4539,-4694,-4848,-4999,
    -5150,-5299,-5446,-5591,-5735,-5877,-6018,-6156,-6293,-6427,
    -6560,-6691,-6819,-6946,-7071,-7193,-7313,-7431,-7547,-7660,
    -7771,-7880,-7986,-8090,-8191,-8290,-8386,-8480,-8571,-8660,
    -8746,-8829,-8910,-8987,-9063,-9135,-9205,-9271,-9335,-9396,
    -9455,-9510,-9563,-9612,-9659,-9702,-9743,-9781,-9816,-9848,
    -9876,-9902,-9925,-9945,-9961,-9975,-9986,-9993,-9998
};
 
// from sin 1 to sin 180
const int sin_table[] =
{
    0,174,348,523,697,871,1045,1218,1391,1564,1736,
    1908,2079,2249,2419,2588,2756,2923,3090,3255,3420,
    3583,3746,3907,4067,4226,4383,4539,4694,4848,5000,
    5150,5299,5446,5591,5735,5877,6018,6156,6293,6427,
    6560,6691,6819,6946,7071,7193,7313,7431,7547,7660,
    7771,7880,7986,8090,8191,8290,8386,8480,8571,8660,
    8746,8829,8910,8987,9063,9135,9205,9271,9335,9396,
    9455,9510,9563,9612,9659,9702,9743,9781,9816,9848,
    9876,9902,9925,9945,9961,9975,9986,9993,9998,10000,
    9998,9993,9986,9975,9961,9945,9925,9902,9876,9848,
    9816,9781,9743,9702,9659,9612,9563,9510,9455,9396,
    9335,9271,9205,9135,9063,8987,8910,8829,8746,8660,
    8571,8480,8386,8290,8191,8090,7986,7880,7771,7660,
    7547,7431,7313,7193,7071,6946,6819,6691,6560,6427,
    6293,6156,6018,5877,5735,5591,5446,5299,5150,5000,
    4848,4694,4539,4383,4226,4067,3907,3746,3583,3420,
    3255,3090,2923,2756,2588,2419,2249,2079,1908,1736,
    1564,1391,1218,1045,871,697,523,348,174
};

STATIC_ASSERT(ARRAY_SIZE(cos_table) == 180);
STATIC_ASSERT(ARRAY_SIZE(sin_table) == 180);

#define MOUSE_SIMU_MAX_RADIUS    300

int mouse_simu_x0 = 0, mouse_simu_y0 = 0;
int acc_x = 0, acc_y = 0;
void mouse_simu_circle(void){
    static int i = 0;
    static int r = 0;

    if(i >= 360){
        i = 0;
    }
    if(0 == i){
        if(0 == r){
            r = 1;
        }else if (r > 10){
            r +=20;
        }else if(r < MOUSE_SIMU_MAX_RADIUS){
            r = r << 1;
        }
    }
    if( r > MOUSE_SIMU_MAX_RADIUS){
        mouse_simu_btn = 0;
        mouse_simu_x = mouse_simu_y = 0;
        return;
    }
    if(0 == i){
        mouse_simu_x0 = r;
        mouse_simu_y0 = 0;
        acc_x = acc_y = 0;
    }
    mouse_simu_btn = MOUSE_LEFT_BTN_DOWN_MASK;
    if(i < 180){
        mouse_simu_x = (s8)(r*cos_table[i]/10000 - mouse_simu_x0 - acc_x);
        mouse_simu_y = (s8)(r*sin_table[i]/10000 - mouse_simu_y0 - acc_y);
    }else{
        mouse_simu_x = (s8)(-r*cos_table[i - 180]/10000 - mouse_simu_x0 - acc_x);
        mouse_simu_y = (s8)(-r*sin_table[i - 180]/10000 - mouse_simu_y0 - acc_y);
    }
    acc_x += mouse_simu_x;
    acc_y += mouse_simu_y;
    ++i;
    
}

int mouse_simu_dev_timer(void *data){
    static s32 count = 0;
#if 0
    static u32 wait_start = 0;
    static u32 time_exceed = 6*1000*1000;
    static u8 last_is_left = 0;
    mouse_simu_btn = 0;
    if(!wait_start){
        wait_start = clock_time();
    }
    if(clock_time_exceed(wait_start, time_exceed)){
        wait_start = clock_time();
        if(last_is_left){
            last_is_left = 0;
            mouse_simu_btn = MOUSE_RIGHT_BTN_DOWN_MASK;
        }else{
            last_is_left = 1;
            mouse_simu_btn = MOUSE_LEFT_BTN_DOWN_MASK;
        }
    }
#endif
#if 0 //  wheel simu
    static s32 mouse_simu_wheel_value_idx = 1;
    mouse_simu_wheel_value = mouse_simu_wheel_value_idx;
    //mouse_simu_wheel_value = 0;
    if (count%10 == 0){
        sleep_us(1000*2000);
        if (mouse_simu_wheel_value_idx == 1){
            mouse_simu_wheel_value_idx = -1;
        }else{
            mouse_simu_wheel_value_idx = 1;
        }
    }
#endif
#if 0 //  mouse simu -- include mouse_simu_init()
    if (count%20 == 0){
        if (mouse_simu_y == 1)
            mouse_simu_y = -1;
        else
            mouse_simu_y = 1;
    }
    if (count%600 == 0){
        if (mouse_simu_x == 1)
            mouse_simu_x = -1;
        else
            mouse_simu_x = 1;
    }
#endif

#if 1
    mouse_simu_circle();
#endif

#if 0        //  模拟传感器产生一个方波
    static int cnt = 0;
    if(0 == cnt){
        mouse_simu_circle();
    }else{
        mouse_simu_x = mouse_simu_y = 0;
    }
    BOUND_INC(cnt, 2);
#endif
    ++count;
    return 0;
}

void mouse_simu_init(void){
    static ev_time_event_t mouse_simu_timer = {mouse_simu_dev_timer};
    ev_on_timer(&mouse_simu_timer, 8000);

}

