/********************************************************************************************************
 * @file     clock.c 
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
 * @par      History:
 * 			 1.initial release(DEC. 26 2018)
 *
 * @version  A001
 *
 *******************************************************************************************************/

#include "register.h"
#include "clock.h"
#include "irq.h"
#include "analog.h"
#include "timer.h"
#include "pm.h"

extern _attribute_data_retention_ unsigned char tl_24mrc_cal;

_attribute_data_retention_	unsigned char system_clk_type;

#if 0
/**
 * @brief       This function to set RC for the system clock.
 * @param[in]   SYS_CLK - the clock source of the system clock.
 * @return      none
 */
void clock_rc_set(SYS_CLK_TypeDef SYS_CLK)
{
	unsigned char temp = analog_read(0x04)&0xfc;
	if(SYS_CLK==SYS_CLK_24M_RC)
	{
		analog_write(0x04, temp|0x00);
		rc_24m_cal();
	}
	else if(SYS_CLK==SYS_CLK_32M_RC)
	{
		analog_write(0x04, temp|0x01);
		//rc_32m_cal();
	}
	else if(SYS_CLK==SYS_CLK_48M_RC)
	{
		analog_write(0x04, temp|0x03);
		rc_48m_cal();
	}
}
#endif
/**
 * @brief       This function to select the system clock source.
 * @param[in]   SYS_CLK - the clock source of the system clock.
 * @return      none
 */
#if (BLC_PM_DEEP_RETENTION_MODE_EN)
_attribute_ram_code_
#endif
void clock_init(SYS_CLK_TypeDef SYS_CLK)
{
	reg_clk_sel = (unsigned char)SYS_CLK;
	system_clk_type = (unsigned char)SYS_CLK;

#if (SYSCLK_RC_CLOCK_EN)
	if(SYS_CLK<SYS_CLK_RC_THRES)
	{
		clock_rc_set(SYS_CLK);
	}
#endif

#if (MODULE_WATCHDOG_ENABLE)
	reg_tmr_ctrl = MASK_VAL( // modify by weixiong in mesh: CLOCK_SYS_CLOCK_1MS --> CLOCK_MCU_RUN_CODE_1MS
		FLD_TMR_WD_CAPT, (MODULE_WATCHDOG_ENABLE ? (WATCHDOG_INIT_TIMEOUT * CLOCK_MCU_RUN_CODE_1MS >> WATCHDOG_TIMEOUT_COEFF):0)
		, FLD_TMR_WD_EN, (MODULE_WATCHDOG_ENABLE?1:0));
#endif

	if(!(pm_is_MCU_deepRetentionWakeup())){
		rc_24m_cal();
	}

}

/**
 * @brief   This function serves to set 32k clock source.
 * @param[in]   variable of 32k type.
 * @return  none.
 */
void clock_32k_init (CLK_32K_TypeDef src)
{
	unsigned char sel_32k   = analog_read(0x2d)&0x7f;
	unsigned char power_32k = analog_read(0x05)&0xfc;
	analog_write(0x2d, sel_32k|(src<<7));
	if(src)
	{
		analog_write(0x05, power_32k|0x1);//32k xtal
		//2.set pc3 as pwm output
		unsigned char sys_clk = read_reg8(0x66);
		write_reg8(0x66,0x43);
		unsigned char reg_596 = read_reg8(0x596);
		write_reg8(0x596,reg_596&0xf7);
		unsigned short reg_798 = read_reg16(0x798);
		write_reg16(0x798,0x01);
		unsigned short reg_79a = read_reg16(0x79a);
		write_reg16(0x79a,0x02);
		unsigned char reg_780 = read_reg8(0x780);
		write_reg8(0x780,0x02);
		write_reg8(0x782,0xf3);

		//3.wait for PWM wake up Xtal
		sleep_ms(5);

		//4.Xtal 32k output
		analog_write(0x03,0x4f); //<7:6>current select

		//5.Recover PC3 as Xtal pin
		write_reg8(0x66,sys_clk);
		write_reg8(0x596,reg_596);
		write_reg16(0x798,reg_798);
		write_reg16(0x79a,reg_79a);
		write_reg8(0x780,reg_780);
	}
	else
	{
		analog_write(0x05, power_32k|0x2);//32k rc
	}
}

/**
 * @brief     This function performs to select 48M RC as the system clock source.
 * @param[in] none.
 * @return    none.
 */
void rc_48m_cal (void)
{
	analog_write(0x33, 0x80);
	analog_write(0x30, 0x20);
    analog_write(0xc7, 0x0e);
    sleep_us(1000);
    analog_write(0xc7, 0x0f);
    while((analog_read(0xcf) & 0x80) == 0);

    volatile unsigned int cal_cnt = analog_read(0xcf)&0x07 ;
    cal_cnt = (cal_cnt<<8) + analog_read(0xce);
    unsigned int f = 64;
    unsigned int temp_v = 0;
    unsigned int temp_d = 0;
    unsigned int temp_d2 = 100;
    unsigned char temp_cap = 0;
    unsigned int i=0;
    while(f>=1)
    {
		temp_v = analog_read(0x33);
    	if(cal_cnt>250)
    	{
    		temp_d =  cal_cnt - 250;
    	}
    	else
    	{
    		temp_d =  250 - cal_cnt;
    	}
    	if(cal_cnt>250)
    	{
    		analog_write(0x33, temp_v-f);
    	}
    	else
    	{
    		analog_write(0x33, temp_v+f);
    	}
    	f = f/2;
        analog_write(0xc7, 0x0e);
        analog_write(0xc7, 0x0f);
        while((analog_read(0xcf) & 0x80) == 0);
        cal_cnt = analog_read(0xcf)&0x07 ;
		cal_cnt = (cal_cnt<<8) + analog_read(0xce);
		i++;
    	if(temp_d2>temp_d)
    	{
    		temp_d2 = temp_d;
    		temp_cap = temp_v;
    	}
    }
    analog_write(0x33, temp_cap);
}

/**
 * @brief     This function performs to select 24M as the system clock source.
 * @param[in] none.
 * @return    none.
 */
void rc_24m_cal (void)
{
    analog_write(0xc8, 0x80);

//    sub_wr_ana(0x30, 1, 7, 7);
    analog_write(0x30, analog_read(0x30) | BIT(7) );

    analog_write(0xc7, 0x0e);
    analog_write(0xc7, 0x0f);
    while((analog_read(0xcf) & 0x80) == 0);
    unsigned char cap = analog_read(0xcb);
    analog_write(0x33, cap);		//write 24m cap into manual register

//	sub_wr_ana(0x30, 0, 7, 7);	//manual on
    analog_write(0x30, analog_read(0x30) & (~BIT(7)) );

	analog_write(0xc7, 0x0e);
	tl_24mrc_cal = analog_read(0x33);
}

/**
 * @brief     This function performs to select 32K as the system clock source.
 * @param[in] none.
 * @return    none.
 */
void rc_32k_cal (void)
{
    analog_write(0x30, 0x60);
    analog_write(0xc6, 0xf6);
    analog_write(0xc6, 0xf7);
    while((analog_read(0xcf) & BIT(6)) == 0);
	unsigned char res1 = analog_read(0xc9);	//read 32k res[13:6]
	analog_write(0x32, res1);		//write 32k res[13:6] into manual register
	unsigned char res2 = analog_read(0xca);	//read 32k res[5:0]
	analog_write(0x31, res2);		//write 32k res[5:0] into manual register
	analog_write(0xc6, 0xf6);
	analog_write(0x30, 0x20);//manual on
}

/**
 * @brief     This function performs to select 32K as source of DMIC.
 * @param[in] source clock to provide DMIC.
 * @return    none.
 */
void dmic_prob_32k(unsigned char src)
{
	write_reg8(0x73,read_reg8(0x73)|0x02);			// sel dmic src
	write_reg8(0x73,(read_reg8(0x73)&0xfe)|src);    // sel 32k src
	write_reg8(0x586,read_reg8(0x586)&0xfd);
	write_reg8(0x5a8,read_reg8(0x5a8)&0xf3);
}

/**
 * @brief     This function performs to select 24M/2 RC as source of DMIC.
 * @param[in] source clock to provide DMIC.
 * @return    none.
 */
void dmic_prob_24M_rc()
{
	//probe 24M/2 RC
	//PD[5] select dmic clk
	write_reg8(0x586,0x1f);
	write_reg8(0x794,0x01);
	write_reg8(0x796,0x02);
	write_reg8(0x781,0x01);
	write_reg8(0x59e,0x5b);
}


