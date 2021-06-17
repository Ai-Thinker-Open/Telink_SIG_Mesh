/********************************************************************************************************
 * @file	flash_mid1360eb.h
 *
 * @brief	This is the header file for b85m
 *
 * @author	Driver Group
 * @date	2020
 *
 * @par     Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *
 *              1. Redistributions of source code must retain the above copyright
 *              notice, this list of conditions and the following disclaimer.
 *
 *              2. Unless for usage inside a TELINK integrated circuit, redistributions
 *              in binary form must reproduce the above copyright notice, this list of
 *              conditions and the following disclaimer in the documentation and/or other
 *              materials provided with the distribution.
 *
 *              3. Neither the name of TELINK, nor the names of its contributors may be
 *              used to endorse or promote products derived from this software without
 *              specific prior written permission.
 *
 *              4. This software, with or without modification, must only be used with a
 *              TELINK integrated circuit. All other usages are subject to written permission
 *              from TELINK and different commercial license may apply.
 *
 *              5. Licensee shall be solely responsible for any claim to the extent arising out of or
 *              relating to such deletion(s), modification(s) or alteration(s).
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 *          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************************************/
#ifndef __MID1360EB_H__
#define __MID1360EB_H__

/*
 * @brief     MID = 0x1360eb Flash include TH25D40UA / TH25D40LA.
 */


/**
 * @brief     define the section of the protected memory area which is read-only and unalterable.
 */
typedef enum{
	FLASH_LOCK_NONE_MID1360EB		=	0x0000,	//000000h-000000h	//0x0020 0x407c 0x4030...
	FLASH_LOCK_UP_64K_MID1360EB		=	0x0004,	//070000h-07FFFFh
	FLASH_LOCK_UP_128K_MID1360EB	=	0x0008,	//060000h-07FFFFh
	FLASH_LOCK_UP_256K_MID1360EB	=	0x000c,	//040000h-07FFFFh	//0x402c
	FLASH_LOCK_LOW_64K_MID1360EB	=	0x0024,	//000000h-00FFFFh
	FLASH_LOCK_LOW_128K_MID1360EB	=	0x0028,	//000000h-01FFFFh
	FLASH_LOCK_LOW_256K_MID1360EB	=	0x002c,	//000000h-03FFFFh	//0x400c
	FLASH_LOCK_UP_4K_MID1360EB		=	0x0044,	//07F000h-07FFFFh
	FLASH_LOCK_UP_8K_MID1360EB		=	0x0048,	//07E000h-07FFFFh
	FLASH_LOCK_UP_16K_MID1360EB		=	0x004c,	//07C000h-07FFFFh
	FLASH_LOCK_UP_32K_MID1360EB		=	0x0050,	//078000h-07FFFFh	//0x0054 0x0058
	FLASH_LOCK_LOW_4K_MID1360EB		=	0x0064,	//000000h-000FFFh
	FLASH_LOCK_LOW_8K_MID1360EB		=	0x0068,	//000000h-001FFFh
	FLASH_LOCK_LOW_16K_MID1360EB	=	0x006c,	//000000h-003FFFh
	FLASH_LOCK_LOW_32K_MID1360EB	=	0x0070,	//000000h-007FFFh	//0x0074 0x0078
	FLASH_LOCK_LOW_448K_MID1360EB	=	0x4004,	//000000h-06FFFFh
	FLASH_LOCK_LOW_384K_MID1360EB	=	0x4008,	//000000h-05FFFFh
	FLASH_LOCK_UP_448K_MID1360EB	=	0x4024,	//010000h-07FFFFh
	FLASH_LOCK_UP_384K_MID1360EB	=	0x4028,	//020000h-07FFFFh
	FLASH_LOCK_LOW_508K_MID1360EB	=	0x4044,	//000000h-07EFFFh
	FLASH_LOCK_LOW_504K_MID1360EB	=	0x4048,	//000000h-07DFFFh
	FLASH_LOCK_LOW_496K_MID1360EB	=	0x404c,	//000000h-07BFFFh
	FLASH_LOCK_LOW_480K_MID1360EB	=	0x4050,	//000000h-077FFFh	//0x4054 0x4058
	FLASH_LOCK_UP_508K_MID1360EB	=	0x4064,	//001000h-07FFFFh
	FLASH_LOCK_UP_504K_MID1360EB	=	0x4068,	//002000h-07FFFFh
	FLASH_LOCK_UP_496K_MID1360EB	=	0x406c,	//004000h-07FFFFh
	FLASH_LOCK_UP_480K_MID1360EB	=	0x4070,	//008000h-07FFFFh	//0x4074 0x4078
	FLASH_LOCK_ALL_512K_MID1360EB	=	0x007c,	//000000h-07FFFFh	//0x4000 0x4040 0x4020 0x4060...
}mid1360eb_lock_block_e;

/**
 * @brief     The starting address of the Security Registers.
 */
typedef enum{
	FLASH_OTP_0x001000_512K_MID1360EB	=	0x001000,	//001000h-0011FFh
	FLASH_OTP_0x002000_512K_MID1360EB	=	0x002000,	//002000h-0021FFh
	FLASH_OTP_0x003000_512K_MID1360EB	=	0x003000,	//003000h-0031FFh
}mid1360eb_otp_block_e;

/**
 * @brief     the lock area of the Security Registers.
 */
typedef enum{
	FLASH_LOCK_OTP_0x001000_512K_MID1360EB	=	0x0800,	//001000h-0011FFh
	FLASH_LOCK_OTP_0x002000_512K_MID1360EB	=	0x1000,	//002000h-0021FFh
	FLASH_LOCK_OTP_0x003000_512K_MID1360EB	=	0x2000,	//003000h-0031FFh
}mid1360eb_lock_otp_e;

/**
 * @brief     the range of bits to be modified when writing status.
 */
typedef enum{
	FLASH_WRITE_STATUS_BP_MID1360EB		=	0x407c,
	FLASH_WRITE_STATUS_OTP_MID1360EB	=	0x3800,
}mid1360eb_write_status_bit_e;


/**
 * @brief 		This function reads the status of flash.
 * @return 		the value of status.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
unsigned short flash_read_status_mid1360eb(void);

/**
 * @brief 		This function write the status of flash.
 * @param[in]  	data	- the value of status.
 * @param[in]  	bit		- the range of bits to be modified when writing status.
 * @return 		none.
 * @note		Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_write_status_mid1360eb(unsigned short data, mid1360eb_write_status_bit_e bit);

/**
 * @brief 		This function serves to set the protection area of the flash.
 * @param[in]   data	- refer to the protection area definition in the .h file.
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_lock_mid1360eb(mid1360eb_lock_block_e data);

/**
 * @brief 		This function serves to flash release protection.
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_unlock_mid1360eb(void);

/**
 * @brief 		This function serves to read data from the Security Registers of the flash.
 * @param[in]   addr	- the start address of the Security Registers.
 *						the address of the  Security Registers #1 0x001000-0x0011ff
 *						the address of the  Security Registers #2 0x002000-0x0021ff
 *						the address of the  Security Registers #3 0x003000-0x0031ff
 * @param[in]   len		- the length of the content to be read.
 * @param[out]  buf		- the starting address of the content to be read.
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_read_otp_mid1360eb(unsigned long addr, unsigned long len, unsigned char* buf);

/**
 * @brief 		This function serves to write data to the Security Registers of the flash you choose.
 * @param[in]   addr	- the start address of the Security Registers.
 *						the address of the  Security Registers #1 0x001000-0x0011ff
 *						the address of the  Security Registers #2 0x002000-0x0021ff
 *						the address of the  Security Registers #3 0x003000-0x0031ff
 * @param[in]   len		- the length of content to be written.
 * @param[in]   buf		- the starting address of the content to be written.
 * @return 		none.
 * @note        Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_write_otp_mid1360eb(unsigned long addr, unsigned long len, unsigned char *buf);

/**
 * @brief 		This function serves to erase the data of the Security Registers that you choose.
 * 				You can erase 512-byte one time.
 * @param[in]   addr	- the address that you want to erase.
 *						the address of the  Security Registers #1 0x001000-0x0011ff
 *						the address of the  Security Registers #2 0x002000-0x0021ff
 *						the address of the  Security Registers #3 0x003000-0x0031ff
 * @return 		none.
 * @note		Even you choose the middle area of the Security Registers,it will erase the whole area.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_erase_otp_mid1360eb(mid1360eb_otp_block_e addr);

/**
 * @brief 		This function serves to provide the write protect control to the Security Registers.
 * @param[in]   data	- the lock area of the Security Registers.
 * @return 		none.
 * @note		once they are set to 1, the Security Registers will become read-only permanently,
 * 				you can't write or erase the area anymore.
 *
 *              Attention: Before calling the FLASH function, please check the power supply voltage of the chip.
 *              Only if the detected voltage is greater than the safe voltage value, the FLASH function can be called.
 *              Taking into account the factors such as power supply fluctuations, the safe voltage value needs to be greater
 *              than the minimum chip operating voltage. For the specific value, please make a reasonable setting according
 *              to the specific application and hardware circuit.
 *
 *              Risk description: When the chip power supply voltage is relatively low, due to the unstable power supply,
 *              there may be a risk of error in the operation of the flash (especially for the write and erase operations.
 *              If an abnormality occurs, the firmware and user data may be rewritten, resulting in the final Product failure)
 */
void flash_lock_otp_mid1360eb(mid1360eb_lock_otp_e data);


#endif

