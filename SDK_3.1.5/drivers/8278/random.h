/********************************************************************************************************
 * @file     random.h 
 *
 * @brief    This is the header file for TLSR8278
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

#pragma once



#define  RANDOM_OLD_MODE          0 //This macro definition is only turned on when using the random number module of the ADC.
#define  RANDOM_NEW_MODE          1 //This is the random number of the TRNG module




/**
 * @brief     This function performs to get one random number.
 *            TRNG Base address is 0x4400,read 4bytes one time to judge whether generator success.
 *            and get 8bytes later to return the random value in order to sure the value is right.
 * @param[in] none.
 * @return    the value of one random number.
 */
void random_generator_init(void);

/**
 * @brief     This function performs to get one random number
 * @param[in] none.
 * @return    the value of one random number.
 */
unsigned int rand(void);



/**
 * @brief This function serves to set adc sampling and get results.
 * @param[in]  none.
 * @return the result of sampling.
 */
unsigned short adc_rng_result(void);


/**
 * @brief This function is used for ADC configuration of ADC supply voltage sampling.
 * @return none
 */
void rng_init(void);



/**
 * @brief      This function performs to get a serial of random number.
 * @param[in]  len- the length of random number
 * @param[in]  data - the first address of buffer store random number in
 * @return     the result of a serial of random number..
 */
void generateRandomNum(int len, unsigned char *data);


