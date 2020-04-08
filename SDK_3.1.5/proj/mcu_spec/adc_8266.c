/********************************************************************************************************
 * @file     adc_8266.c 
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

#if(__TL_LIB_8266__ || (MCU_CORE_TYPE == MCU_CORE_8266))

#if ( MODULE_ADC_ENABLE)
#include "proj_lib/rf_drv.h"
static inline void adc_set_clk_freq(u8 mhz){
	reg_adc_step_l = mhz*4;
	reg_adc_mod = MASK_VAL(FLD_ADC_MOD, 192*4, FLD_ADC_CLK_EN, 1);
}


static inline void adc_set_l_chn(u8 chn, u8 mic_en){
	reg_adc_pga_sel_l = chn >> 8;	//C0, C1, C2 & C3
	reg_adc_chn_l_sel = chn | FLD_ADC_DATA_SIGNED;
	if(mic_en){
		analog_write (raga_pga_gain0, 0x50);
		analog_write (raga_pga_gain1, 0x22);

		reg_clk_en2 |= FLD_CLK2_DIFIO_EN;
		reg_dfifo_ana_in = FLD_DFIFO_AUD_INPUT_MONO | FLD_DFIFO_MIC_ADC_IN;
		reg_dfifo_scale = 5;				// down scale by 6,  96K / 6 == 16K
		reg_aud_hpf_alc = 11;				// volume setting
		reg_aud_alc_vol = 2;

	}else{
		analog_write (raga_pga_gain0, 0x53);
		analog_write (raga_pga_gain1, 0x00);
	}

}

static inline void adc_set_period(void){
	reg_adc_period_chn0 = 77;
	reg_adc_period_chn12 = 8;
}

void adc_clk_en(int en){
	if (en) {
		// Eanble the clock
		reg_adc_clk_en |= BIT(7);

		// Enable ADC LDO
		analog_write(0x06,(analog_read(0x06)&0xfe));
	} else {
	    // Disable ADC clock
	    reg_adc_clk_en &= ~ BIT(7);

	    // Disable ADC LDO
		analog_write(0x06,(analog_read(0x06)&0xfe));
	}

	adc_clk_poweron ();
}
u8 adc_clk_step_l = 0;
u8	adc_chn_m_input = 0;

void adc_init()
{
	// pin selection
	reg_adc_chn_m_sel = ADC_CHNM_ANA_INPUT;
	adc_chn_m_input = ADC_CHNM_ANA_INPUT;

    // Set sample rate and cycle, default to 14bit
	reg_adc_samp_res = MASK_VAL(FLD_ADC_CHNM_SAMP_CYCLE, ADC_SAMPLING_CYCLE_6
		, FLD_ADC_CHNM_SAMP_RESOL, ADC_SAMPLING_RES_14BIT
		, FLD_ADC_CHNLR_SAMP_CYCLE, ADC_SAMPLING_CYCLE_9);

    // Set Reference voltage
  	reg_adc_ref = ADC_CHNM_REF_SRC;		// ref: 0 1.3v  01 AVDD

	// Set ALL manual mode
	reg_adc_ctrl = 0x20;

	// Setting clk
	adc_set_clk_freq(4);
	adc_clk_step_l = 4;

	// Enable ADC
	adc_clk_en(1);
}


u16 adc_get(void)
{
	// Set a run signal
	reg_adc_chn1_outp = FLD_ADC_CHN_MANU_START;

	// wait for data
	sleep_us(5);

	// read data
	return (reg_adc_dat_byp_outp & 0x3FFF);
}

/*between start and end must > 200us
 * ½öÊÊÓÃÓÚ8266 A2Ð¾Æ¬
 * power down step£º
 * 	1, change input to GND
 * 	2, lower adc clk
 * 	3, start adc
 * 	4, wait > 200us(we can do something else)
 * 	5, start adc again
 * 	6, wait adc latch state,the interval calculation base on system clk and adc clk
 * 	7, power down adc in analog register
 * 	9, recover adc setting
 * */
void adc_power_down(void){
	u8 adc_m_input = reg_adc_chn_m_sel;
	u8 adc_step_low = reg_adc_step_l;
	/*step 0*/
	write_reg8(0x2c,0x12);  //chnm: GND
	write_reg8(0x69,0x01);  //set adc clk = 192M/(192*4) = 0.25K
	write_reg8(0x35,0x80);  //start

	sleep_us(200);
	/*step 1*/
	write_reg8(0x35,0x80);  //start again
	sleep_us(26);             //14 us
	analog_write(0x06,analog_read(0x06) | 0x01);
	/*step 2*/
	reg_adc_chn_m_sel = adc_m_input;
	reg_adc_step_l = adc_step_low;//set adc clk= 192M * 16/(192 *4) = 4M
}

void adc_power_down_start(void){

	write_reg8(0x2c,0x12);  //chn: GND
	write_reg8(0x69,0x01);  //set adc clk = 192M/(192*4) = 0.25K
	write_reg8(0x35,0x80);  //start
}
void adc_power_down_end(void){
	write_reg8(0x35,0x80);  //start again
	sleep_us(26);             //14 us
	analog_write(0x06,analog_read(0x06) | 0x01);
}
void adc_setting_recover(void){
#if 1
	reg_adc_step_l = adc_clk_step_l;
	reg_adc_chn_m_sel = adc_chn_m_input;
#else
	reg_adc_chn_m_sel = FLD_ADC_CHN_C7;
	reg_adc_step_l = 16;//set adc clk= 192M * 16/(192 *4) = 4M

#endif
}

/*
void adc_example(void)
{
    adc_init(GPIO_PC2, ADC_REF_1_3V);
    ......
    u16 result = adc_get();
    printf("voltage = %dmv\r\n", ((result*1300)>>14)); // Here >>14 means ( result * 1300 / 0x3fff)
}
*/

#endif
#endif

