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
_attribute_data_retention_	unsigned int rnd_m_w = 0;
_attribute_data_retention_	unsigned int rnd_m_z = 0;

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


