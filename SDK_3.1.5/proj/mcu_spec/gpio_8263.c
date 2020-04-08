/********************************************************************************************************
 * @file     gpio_8263.c 
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


#include "../config/user_config.h"
#include "../mcu/config.h"


#if(__TL_LIB_8263__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8263))

#include "../common/types.h"
#include "../common/compatibility.h"
#include "../common/bit.h"
#include "../common/tutility.h"
#include "../common/static_assert.h"
#include "../mcu/compiler.h"
#include "../mcu/register.h"
#include "../mcu/anareg.h"
#include "../mcu/analog.h"

#include "../mcu/gpio.h"



u8 pullupdn_ana_mask_table[GPIO_PULLUPDN_COUNT*2] =
{
	0x28, 0x02, //GP0   ana_28[1]
	0x28, 0x04, //GP1   ana_28[2]
	0x28, 0x08, //GP2   ana_28[3]
	0x28, 0x10, //GP3   ana_28[4]
	0x28, 0x20, //GP4   ana_28[5]
	0x28, 0x40, //GP5   ana_28[6]
	0x28, 0x80, //GP6   ana_28[7]

	0x29, 0x01, //GP7   ana_29[0]
	0x29, 0x02, //GP8   ana_29[1]
	0x29, 0x04, //GP9   ana_29[2]
	0x29, 0x08, //GP10  ana_29[3]
	0x29, 0x10, //GP11  ana_29[4]
	0x29, 0x20, //GP12  ana_29[5]
	0x29, 0x40, //GP13  ana_29[6]
	0x29, 0x80, //GP14  ana_29[7]

	0x2a, 0x01, //GP15  ana_2a[0]
	0x2a, 0x02, //GP16  ana_2a[1]

	0x08, 0x03, //GP17  ana_08[1:0]
	0x08, 0x0c, //GP18  ana_08[3:2]
	0x08, 0x30, //GP19  ana_08[5:4]
	0x08, 0xc0, //GP20  ana_08[7:6]

	0x09, 0x03, //GP21  ana_09[1:0]
	0x09, 0x0c, //GP22  ana_09[3:2]
	0x09, 0x30, //GP23  ana_09[5:4]
	0x09, 0xc0, //GP24  ana_09[7:6]

	0x00, 0x00, //GP25  no pullup

	0x2a, 0x0c, //GP26  ana_2a[3:2]
	0x2a, 0x30, //GP27  ana_2a[5:4]

	0x00, 0x00, //GP28  no pullupdn
	0x00, 0x00, //GP29  no pullupdn
	0x00, 0x00, //GP30  no pullupdn

	0x2a, 0xc0, //GP31  ana_2a[7:6]

	0x28, 0x01, //GP32  ana_28[0]
};


//132 + 66 = 196
void gpio_setup_up_down_resistor(u32 gpio, u32 up_down)
{
	u8 i;
	u8 pin_num,reg,shift,mask;
	u8 r_val;
	u8 bit = gpio&0xff;
	if(!bit){
		return;
	}

	if( up_down == PM_PIN_UP_DOWN_FLOAT ){
		r_val = 0;
	}
	else if(up_down == PM_PIN_PULLUP_1M){
		r_val = PM_PIN_PULLUP_1M;
	}
	else if(up_down == PM_PIN_PULLUP_10K){
		r_val = PM_PIN_PULLUP_10K;
	}
	else{
		r_val = PM_PIN_PULLDOWN_100K;
	}


	for(i=0;i<7;i++){
		if(bit&(1<<i)){
			pin_num = i;
			break;
		}
	}

	pin_num += ((gpio>>8)<<3 );

	reg =  pullupdn_ana_mask_table[pin_num<<1];
	if(!reg){
		return;
	}

	mask = pullupdn_ana_mask_table[(pin_num<<1)+1];

	for(i=0;i<7;i++){
		if(mask&(1<<i)){
			shift = i;
			break;
		}
	}

	analog_write(reg, ( analog_read(reg) & (~mask) ) | (r_val<<shift) );
}

#endif

