/********************************************************************************************************
 * @file     pwm_8266.c 
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

//#include "../mcu/pwm.h"
#include "../tl_common.h"



#if (MCU_CORE_TYPE == MCU_CORE_8266)

void pwm_set_mode(PWM_ID id, u16 mode,int pulse_num){
	if(PWM0 == id || PWM0_INV == id ){
		reg_pwm_mode = MASK_VAL(FLD_PWM0_MODE,mode);
	}else if(PWM1 == id || PWM1_INV == id){
		reg_pwm_mode = MASK_VAL(FLD_PWM1_MODE,mode);
	}
	reg_pwm_pulse_num(id>>1) = pulse_num;

}
void pwm_set_duty(PWM_ID id, u16 max_tick, u16 cmp_tick){
	id >>= 1;
	reg_pwm_cycle(id) = MASK_VAL(FLD_PWM_CMP, cmp_tick, FLD_PWM_MAX, max_tick);
}

void pwm_start(PWM_ID id){
	id >>= 1;
	BM_SET(reg_pwm_enable, BIT(id));
}

void pwm_stop(PWM_ID id){
	id >>= 1;
    BM_CLR(reg_pwm_enable, BIT(id));
}

void pwm_clk(int pwm_clk){
	reg_pwm_clk = (int)CLOCK_SYS_CLOCK_HZ / pwm_clk - 1;
	reg_clk_en1 |= FLD_CLK_PWM_EN;
}
void pwm_set_invert_en(PWM_ID id,int en){
	if(id%2){
		id >>= 1;
		if(en){
			reg_pwm_n_invert |= BIT(id);
		}else{
			reg_pwm_n_invert &= ~BIT(id);
		}
	}else{
		id >>= 1;
		if(en){
			reg_pwm_invert |= BIT(id);
		}else{
			reg_pwm_invert &= ~BIT(id);
		}
	}
}
void pwm_set_int_mask_enable(PWM_ID id,int en,int is_pnum_int_mask){
	if(is_pnum_int_mask){
		reg_pwm_irq_mask = en ? (reg_pwm_irq_mask | BIT(id>>1)): (reg_pwm_irq_mask & (~BIT(id>>1)));
	}else{
		reg_pwm_irq_mask = en ? (reg_pwm_irq_mask | BIT((id + 4)>>1)):(reg_pwm_irq_mask & ~BIT((id + 4)>>1)) ;
	}
}
u8 pwm_int_Status_read(PWM_ID id,int is_pnum_int_mask){
	if(is_pnum_int_mask){
		return reg_pwm_irq_sta & BIT(id>>1) ? TRUE : FALSE ;
	}else{
		return reg_pwm_irq_sta & BIT((id + 4)>>1) ? TRUE : FALSE;
	}
}
void pwm_int_Status_clear(PWM_ID id,int is_pnum_int_mask){
	if(is_pnum_int_mask){
		 reg_pwm_irq_sta |= BIT(id>>1);
	}else{
		 reg_pwm_irq_sta |= BIT((id + 4)>>1);
	}
}
#if 0
/**
 * PWM example
 * @brief 	In the corresponding IO port output waveform corresponding.
 */
u32 pwmGpio[12] = {GPIO_PWM0,GPIO_PWM0_INV,GPIO_PWM1,GPIO_PWM1_INV,GPIO_PWM2,GPIO_PWM2_INV,
		GPIO_PWM3,GPIO_PWM3_INV,GPIO_PWM4,GPIO_PWM4_INV,GPIO_PWM5,GPIO_PWM5_INV,
};
u8 aa_pwm_int_ret = 0;
void pwm_example_normalMode(u8 pwm_id){
	// set pwm gpio as PWM mode
	gpio_set_func(pwmGpio[pwm_id],AS_PWM);
	/*power on pwm module and setting pwm_module_clock = 2Mhz */
	pwm_clk(2000000);
	/*set pwm0 mode to NORMAL MODE*/
	pwm_set_mode(pwm_id,NORMAL_MODE,0);
	/*Set PWM0_output_frequency = 2khz and high_time_duty_ratio = 20%*/
	/*max_tick = pwm_module_clock / PWM0_output_frequency = 2MHz / 2kHz = 1000 */
	/*cmp_tick = max_tick * high_time_duty_ratio = 1000 * 20% = 200 */
	pwm_set_duty(pwm_id,1000,200);
	/*if we invert PWM0 output,and the high_time_duty_ratio will changed to 80%*/
	pwm_set_invert_en(pwm_id,1);
	/*enable PWM0 output*/
	pwm_start(pwm_id);

	sleep_us(10*1000*1000);

	/*diable PWM0 output*/
	pwm_stop(pwm_id);
}
void pwm_example_countMode(u8 pwm_id){
	u16 pluse_num = 100;
	// set pwm gpio as PWM mode
	gpio_set_func(pwmGpio[pwm_id],AS_PWM);
	/*power on pwm module and setting pwm_module_clock = 2Mhz */
	pwm_clk(2000000);
	/*set pwm0 mode to NORMAL MODE*/
	pwm_set_mode(pwm_id,COUNT_MODE,pluse_num);
	/*Set PWM0_output_frequency = 2khz and high_time_duty_ratio = 20%*/
	/*max_tick = pwm_module_clock / PWM0_output_frequency = 2MHz / 2kHz = 1000 */
	/*cmp_tick = max_tick * high_time_duty_ratio = 1000 * 20% = 200 */
	pwm_set_duty(pwm_id,1000,200);
	/*if we invert PWM0 output,and the high_time_duty_ratio will changed to 80%*/
	pwm_set_invert_en(pwm_id,1);
	/*set pwm int mask enable*/
	pwm_set_int_mask_enable(pwm_id,1,1);
	/*enable PWM0 output*/
	pwm_start(pwm_id);
}
void pwm_example_irMode(u8 pwm_id){
	u16 pluse_num = 100;
	// set pwm gpio as PWM mode
	gpio_set_func(pwmGpio[pwm_id],AS_PWM);
	/*power on pwm module and setting pwm_module_clock = 2Mhz */
	pwm_clk(2000000);
	/*set pwm0 mode to NORMAL MODE*/
	pwm_set_mode(pwm_id,IR_MODE,pluse_num);
	/*Set PWM0_output_frequency = 2khz and high_time_duty_ratio = 20%*/
	/*max_tick = pwm_module_clock / PWM0_output_frequency = 2MHz / 2kHz = 1000 */
	/*cmp_tick = max_tick * high_time_duty_ratio = 1000 * 20% = 200 */
	pwm_set_duty(pwm_id,1000,200);
	/*if we invert PWM0 output,and the high_time_duty_ratio will changed to 80%*/
	pwm_set_invert_en(pwm_id,1);
	/*set pwm int mask enable*/
	pwm_set_int_mask_enable(pwm_id,1,1);
	/*enable PWM0 output*/
	pwm_start(pwm_id);
}
#endif

#endif
