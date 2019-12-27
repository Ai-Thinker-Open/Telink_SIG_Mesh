/********************************************************************************************************
 * @file     dfifo.c 
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
/*
#include "../mcu/register.h"
#include "../common/tutility.h"
#include "../common/assert.h"
#include "../common/static_assert.h"
#include "dfifo.h"
#include "../mcu/clock.h"
//#include "../tl_common.h"
//#include "dfifo.h"*/
#include "../tl_common.h"
#include "dfifo.h"

#if 0
enum{
	DFIFO_DOWN_SAMPLE1 = 0,
	DFIFO_DOWN_SAMPLE2 = 1,
	DFIFO_DOWN_SAMPLE3 = 2,
	DFIFO_DOWN_SAMPLE4 = 3,
	DFIFO_DOWN_SAMPLE5 = 4,
	DFIFO_DOWN_SAMPLE6 = 5,
	DFIFO_DOWN_SAMPLE7 = 6,
	DFIFO_DOWN_SAMPLE8 = 7,
	DFIFO_DOWN_SAMPLE16 = 8,
	DFIFO_DOWN_SAMPLE32 = 9,	//  there is error in old doc 
	DFIFO_DOWN_SAMPLE64 = 10,
	DFIFO_DOWN_SAMPLE128 = 11,
	DFIFO_DOWN_SAMPLE512 = 12,
};
#define DFIFO_MIC_DOWN_SAMPLE	DFIFO_DOWN_SAMPLE6

void dfifo_init(){

#if(MCU_CORE_TYPE == MCU_CORE_5320 && (!MODULE_ETH_ENABLE))

#if (DFIFO0_SIZE)	
	static unsigned long _attribute_aligned_(4) dfifo0_buf[DFIFO0_SIZE>>2];
	reg_dfifo0_buf = MASK_VAL(FLD_DFIFO_BUF_ADDR, (u16)(u32)(dfifo0_buf), FLD_DFIFO_BUF_SIZE, (DFIFO0_SIZE>>4)-1);
#endif

#if (DFIFO1_SIZE)	
	static unsigned long _attribute_aligned_(4) dfifo1_buf[DFIFO1_SIZE>>2];
	reg_dfifo1_buf = MASK_VAL(FLD_DFIFO_BUF_ADDR, (u16)(u32)(dfifo1_buf), FLD_DFIFO_BUF_SIZE, (DFIFO1_SIZE>>4)-1);
#endif

#if (DFIFO2_SIZE)	
	static unsigned long _attribute_aligned_(4) dfifo2_buf[DFIFO2_SIZE>>2];
	reg_dfifo2_buf = MASK_VAL(FLD_DFIFO_BUF_ADDR, (u16)dfifo2_buf, FLD_DFIFO_BUF_SIZE, (DFIFO2_SIZE>>4)-1);
#endif

#if (DFIFO0_SIZE || DFIFO1_SIZE || DFIFO2_SIZE)	
	reg_dfifo_buf_thres = MASK_VAL(FLD_DFIFO0_LO_THRE, DFIFO0_SIZE>>6, FLD_DFIFO0_HI_THRE, (DFIFO0_SIZE>>4)-(DFIFO0_SIZE>>6),
		FLD_DFIFO1_HI_THRE, DFIFO1_SIZE>>5, FLD_DFIFO2_HI_THRE, DFIFO2_SIZE>>5);
#endif

#endif

	// used in initial only,  because this will overwrite 0x800b13
	reg_dfifo_ctrl = MASK_VAL(FLD_DFIFO_AIN_AUD_INP, MIC_DATA_SRC, FLD_DFIFO_AIN_MONO_INP, MIC_IS_MONO
		, FLD_DFIFO1_AUD_IN_EN, MODULE_MIC_ENABLE?1:0
		, FLD_DFIFO0_AUD_OUT_EN, 1
		, FLD_DFIFO_CIC_DN_SAMP, 0xf0 | DFIFO_MIC_DOWN_SAMPLE);	//  !!!  don't know what 0xf0 means

#if(MODULE_DMIC_ENABLE)
	reg_dmic_step = MASK_VAL(FLD_DMIC_STEP,2) | FLD_DMIC_CLK_EN;		// step,  turn on dmic later
	reg_dmic_mod = CLK_FHS_MZ;
#endif

#if(MODULE_MIC_ENABLE || USB_MIC_ENABLE)

	reg_dfifo_source = FLD_DFIFO0_SOURCE_SYNC0 | FLD_DFIFO1_SOURCE_SYNC1 |
						FLD_DFIFO2_SOURCE_DMIC | FLD_DFIFO0_SOURCE_MONO;
#endif

#if (MODULE_MIC_ENABLE || USB_MIC_ENABLE)
	#if(MIC_SAMPLE_RATE <= 16000)		//  2M / 128,   ÔÚ aud_init  ÖÐÎ¢µ÷
		reg_dfifo_scale = MASK_VAL(FLD_DFIFO2_DEC_CIC, DFIFO_DOWN_SAMPLE128, FLD_DFIFO0_DEC_SCALE, 12); // 0xcb = 1100 1011
	#else
		reg_dfifo_scale = MASK_VAL(FLD_DFIFO2_DEC_CIC, DFIFO_DOWN_SAMPLE64, FLD_DFIFO0_DEC_SCALE, 12); // 0xcb = 1100 1011
	#endif

	STATIC_ASSERT(IMPLIES((MIC_DATA_SRC == MIC_SRC_FROM_ADC), MODULE_ADC_ENABLE));
#endif
	STATIC_ASSERT(IMPLIES(MODULE_AUDIO_ENABLE, DFIFO0_SIZE > 0));

}

int dfifo0_low(){
	return (reg_dfifo_sta & FLD_DFIFO0_LO_STA);
}

int dfifo0_high(){
	return reg_dfifo_sta & FLD_DFIFO0_HI_STA;
}

int dfifo1_high(){
	return reg_dfifo_sta & FLD_DFIFO1_HI_STA;
}

int dfifo2_high(){
	return reg_dfifo_sta & FLD_DFIFO2_HI_STA;
}

#endif

