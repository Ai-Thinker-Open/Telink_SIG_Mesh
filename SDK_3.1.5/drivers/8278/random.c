/********************************************************************************************************
 * @file     random.c
 *
 * @brief    This is the source file for TLSR8278
 *
 * @author	 Driver Group
 * @date     May 8, 2018
 *
 * @par      Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *           The information contained herein is confidential property of Telink
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *           Co., Ltd. and the licensee or the terms described here-in. This heading
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "adc.h"
#include "analog.h"
#include "dfifo.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "random.h"
_attribute_data_retention_	unsigned int rnd_m_w = 0;
_attribute_data_retention_	unsigned int rnd_m_z = 0;


#if  RANDOM_NEW_MODE

/**
 * @brief     This function performs to get one random number.If chip in suspend TRNG module should be close.
 *            else its current will be larger.
 * @param[in] none.
 * @return    the value of one random number.
 */
void random_generator_init(void)
{

	unsigned char done=0;
	unsigned char cnt=0;
	unsigned char time_out=0;
	unsigned char con=0;

	//For TRNG 0x62<BIT3->0> enable TRNG module;  0x65<BIT3->1>turn on TRNG clock
	write_reg8(0x62, 0x00);
	write_reg8(0x65, 0xff);

	write_reg8(0x4400,read_reg8(0x4400)&0xfe);//disable
	write_reg32(0x4404,0x0);//TCR MSEL
	write_reg8(0x4400,read_reg8(0x4400) | 0x1f);//enable

	done=(read_reg8(0x4408) & 0x01);
	con = done || time_out;

	while(con==0)
	{
		done=(read_reg8(0x4408) & 0x01);
		cnt+=1;
		if(cnt>100)
		{
			time_out=1;
		}
		con = done || time_out;
	}
	rnd_m_w = read_reg32(0x440c);
	rnd_m_z = read_reg32(0x440c);

	//For TRNG 0x62<BIT3->0> RESET TRNG module;  0x65<BIT3->1>turn off TRNG clock
	write_reg8(0x62, 0x08);
	write_reg8(0x65, 0xf7);

	write_reg8(0x4400,read_reg8(0x4400)&0xe0);//disable

}

/**
 * @brief     This function performs to get one random number
 * @param[in] none.
 * @return    the value of one random number.
 */
_attribute_ram_code_ unsigned int rand(void)  //16M clock, code in flash 23us, code in sram 4us
{

	rnd_m_w = 18000 * (rnd_m_w & 0xffff) + (rnd_m_w >> 16);
	rnd_m_z = 36969 * (rnd_m_z & 0xffff) + (rnd_m_z >> 16);
	unsigned int result = (rnd_m_z << 16) + rnd_m_w;

	return (unsigned int)( result ^ clock_time() );

}

#else


typedef union
{
	unsigned int rng32;

	struct {
		unsigned int bit0:1;
		unsigned int bit1:1;
		unsigned int bit2:1;
		unsigned int bit3:1;
		unsigned int bit4:1;
		unsigned int bit5:1;
		unsigned int bit6:1;
		unsigned int bit7:1;
		unsigned int bit8:1;
		unsigned int bit9:1;
		unsigned int bit10:1;
		unsigned int bit11:1;
		unsigned int bit12:1;
		unsigned int bit13:1;
		unsigned int bit14:1;
		unsigned int bit15:1;
		unsigned int bit16:1;

	}rng_bits;

}ADC_RNG_ValDef;


volatile static ADC_RNG_ValDef rng = {0};


static unsigned short rng_made(void)
{

	rng.rng_bits.bit16 = rng.rng_bits.bit16 ^ rng.rng_bits.bit15 ^ rng.rng_bits.bit13 ^ rng.rng_bits.bit4 ^ rng.rng_bits.bit0;
	if(rng.rng_bits.bit16)
	{
		rng.rng32 = (rng.rng32<<1)+ 1;
	}
	else
	{
		rng.rng32 = (rng.rng32<<1);
	}

	return ((unsigned short)rng.rng32);
}


/**
 * @brief This function serves to set adc sampling and get results.
 * @param[in]  none.
 * @return the result of sampling.
 */
unsigned short adc_rng_result(void)
{
	volatile signed int adc_dat_buf[16];
	volatile unsigned short rng_result;

	unsigned char i;
	//unsigned int j;
	unsigned int t0 = clock_time();
	//dfifo setting will lose in suspend/deep, so we need config it every time
	adc_config_misc_channel_buf((unsigned short *)adc_dat_buf,16);  //size: ADC_SAMPLE_NUM*4
	dfifo_enable_dfifo2();

	while(!clock_time_exceed(t0, 25));  //wait at least 2 sample cycle(f = 96K, T = 10.4us)

	for(i=0;i<16;i++)
	{
		while((!adc_dat_buf[i])&&(!clock_time_exceed(t0,25)));  //wait for new adc sample data,

		t0 = clock_time();

		rng.rng32 &= 0x0000ffff;
		if(adc_dat_buf[i] & BIT(0))
		{
			rng.rng_bits.bit16 = 1;
		}

		rng_result = rng_made();

	}
	dfifo_disable_dfifo2();
	return rng_result;

}

/**
 * @brief This function is used for ADC configuration of ADC supply voltage sampling.
 * @return none
 */
void rng_init(void)
{
	//set R_max_mc,R_max_c,R_max_s
	//set R_max_mc,R_max_c,R_max_s
	adc_set_length_capture_state_for_chn_misc_rns(0xf0);//max_mc=240
	adc_set_length_set_state(0x0a);//	R_max_s=10
	//set total length for sampling state machine and channel
	adc_set_chn_enable(ADC_MISC_CHN);//same//adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);
	adc_set_max_state_cnt(0x02);//same

	//set channel Vref
	adc_set_ref_voltage(ADC_VREF_1P2V);

	//set Vbat divider select,
	adc_set_vref_vbat_divider(ADC_VBAT_DIVIDER_OFF);

	//set channel mode and channel
	adc_set_ain_channel_differential_mode(VBAT, GND);

	//set resolution for RNG
	adc_set_resolution(RES14);

	//Number of ADC clock cycles in sampling phase
	adc_set_tsample_cycle(SAMPLING_CYCLES_6);

	//set Analog input pre-scaling and
	adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);//  ADC_PRESCALER_1F8

}

void random_generator_init(void)
{
	rng.rng32 = 0x0000ffff;
	//ADC modle init
	adc_init();
	rng_init();
	//After setting the ADC parameters, turn on the ADC power supply control bit
	adc_power_on_sar_adc(1);
	rnd_m_w = adc_rng_result()<<16 | adc_rng_result();
	rnd_m_z = adc_rng_result()<<16 | adc_rng_result();
	adc_power_on_sar_adc(0);
}


/**
 * @brief     This function performs to get one random number
 * @param[in] none.
 * @return    the value of one random number.
 */
_attribute_ram_code_ unsigned int rand(void)  //16M clock, code in flash 23us, code in sram 4us
{
	rnd_m_w = 18000 * (rnd_m_w & 0xffff) + (rnd_m_w >> 16);
	rnd_m_z = 36969 * (rnd_m_z & 0xffff) + (rnd_m_z >> 16);
	unsigned int result = (rnd_m_z << 16) + rnd_m_w;

	return (unsigned int)( result ^ clock_time() );
}


#endif

#if 0
/**
 * @brief      This function performs to get a serial of random number.
 * @param[in]  len- the length of random number
 * @param[in]  data - the first address of buffer store random number in
 * @return     the result of a serial of random number..
 */
void generateRandomNum(int len, unsigned char *data)
{
	int i;
	unsigned int randNums = 0;
    /* if len is odd */
	for (i=0; i<len; i++ ) {
		if( (i & 3) == 0 ){
			randNums = rand();
		}

		data[i] = randNums & 0xff;
		randNums >>=8;
	}
}
#endif


