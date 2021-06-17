/********************************************************************************************************
 * @file	flash.h
 *
 * @brief	This is the header file for B85
 *
 * @author	Driver Group
 * @date	May 8,2018
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
#pragma once

#include "proj/tl_common.h"
#include "compiler.h"


#define PAGE_SIZE			   256
#define PAGE_SIZE_OTP		   256
#define FLASH_LOCK_EN          0


/**
 * @brief     flash command definition
 */
enum{
	//common cmd
	FLASH_WRITE_CMD						=	0x02,
	FLASH_READ_CMD						=	0x03,
	FLASH_WRITE_SECURITY_REGISTERS_CMD	=	0x42,
	FLASH_READ_SECURITY_REGISTERS_CMD	=	0x48,

	FLASH_SECT_ERASE_CMD				=	0x20,
	FLASH_ERASE_SECURITY_REGISTERS_CMD	=	0x44,

	FLASH_READ_UID_CMD_GD_PUYA_ZB_UT	=	0x4B,	//Flash Type = GD/PUYA/ZB/UT
	FLASH_READ_UID_CMD_XTX				=	0x5A,	//Flash Type = XTX

	FLASH_GET_JEDEC_ID					=	0x9F,

	//special cmd
	FLASH_WRITE_STATUS_CMD_LOWBYTE		=	0x01,
	FLASH_WRITE_STATUS_CMD_HIGHBYTE		=	0x31,

	FLASH_READ_STATUS_CMD_LOWBYTE		=	0x05,
	FLASH_READ_STATUS_CMD_HIGHBYTE		=	0x35,

	FLASH_WRITE_DISABLE_CMD 			= 	0x04,
	FLASH_WRITE_ENABLE_CMD 				= 	0x06,

};

/**
 * @brief     flash status type definition
 */
typedef enum{
	FLASH_TYPE_8BIT_STATUS   			= 0,
	FLASH_TYPE_16BIT_STATUS_ONE_CMD  	= 1,
	FLASH_TYPE_16BIT_STATUS_TWO_CMD  	= 2,
}flash_status_typedef_e;

/**
 * @brief     flash uid cmd definition
 */
typedef enum{
	FLASH_UID_CMD_GD_PUYA   = 0x4b,
	FLASH_XTX_READ_UID_CMD	= 0x5A,
}flash_uid_cmddef_e;

/**
 * @brief	flash capacity definition
 *			Call flash_read_mid function to get the size of flash capacity.
 *			Example is as follows:
 *			unsigned int mid = flash_read_mid();
 *			The value of (mid&0x00ff0000)>>16 reflects flash capacity.
 */
typedef enum {
	FLASH_SIZE_64K 	= 0x10,
	FLASH_SIZE_128K = 0x11,
	FLASH_SIZE_256K = 0x12,
	FLASH_SIZE_512K = 0x13,
	FLASH_SIZE_1M 	= 0x14,
	FLASH_SIZE_2M 	= 0x15,
	FLASH_SIZE_4M 	= 0x16,
	FLASH_SIZE_8M 	= 0x17,
} Flash_CapacityDef;

/**
 * @brief	flash voltage definition
 */
typedef enum {
    FLASH_VOLTAGE_1V95     = 0x07,
    FLASH_VOLTAGE_1V9      = 0x06,
    FLASH_VOLTAGE_1V85     = 0x05,
    FLASH_VOLTAGE_1V8      = 0x04,
    FLASH_VOLTAGE_1V75     = 0x03,
    FLASH_VOLTAGE_1V7      = 0x02,
    FLASH_VOLTAGE_1V65     = 0x01,
    FLASH_VOLTAGE_1V6      = 0x00,
} Flash_VoltageDef;

/*******************************************************************************************************************
 *												Primary interface
 ******************************************************************************************************************/

extern  unsigned char zbit_flash_flag;

/**
 * @brief 		This function serves to erase a sector.
 * @param[in]   addr	- the start address of the sector needs to erase.
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
void flash_erase_sector(unsigned long addr);

/**
 * @brief 		This function reads the content from a page to the buf.
 * @param[in]   addr	- the start address of the page.
 * @param[in]   len		- the length(in byte) of content needs to read out from the page.
 * @param[out]  buf		- the start address of the buffer.
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
void flash_read_page(unsigned long addr, unsigned long len, unsigned char *buf);

/**
 * @brief 		This function writes the buffer's content to the flash.
 * @param[in]   addr	- the start address of the area.
 * @param[in]   len		- the length(in byte) of content needs to write into the flash.
 * @param[in]   buf		- the start address of the content needs to write into.
 * @return 		none.
 * @note        the funciton support cross-page writing,which means the len of buf can bigger than 256.
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
void flash_write_page(unsigned long addr, unsigned long len, unsigned char *buf);


#if(HOMEKIT_EN)
void 			flash_write_val(unsigned long adr, unsigned long flag_invalid, unsigned long dat);
unsigned long 	flash_read_val(unsigned long adr, unsigned long flag_invalid);
void 			flash_write_long_val(unsigned long adr, unsigned long flag_invalid, unsigned char* dat, unsigned char len);
unsigned long 	flash_read_long_val(unsigned long adr, unsigned long flag_invalid, unsigned char* dat, unsigned char len);
unsigned long flash_subregion_write_val (unsigned long adr, unsigned long flag_invalid, unsigned long dat, unsigned long num);
unsigned long flash_subregion_read_val (unsigned long adr, unsigned long flag_invalid, unsigned long num);
#endif

/**
 * @brief	  	This function serves to read MID of flash(MAC id).
 * @return    	MID of the flash(3 bytes).
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
unsigned int flash_read_raw_mid(void);

/**
 * @brief	  	This function serves to read MID of flash(MAC id). Before reading UID of flash,
 * 				you must read MID of flash. and then you can look up the related table to select
 * 				the idcmd and read UID of flash
 * @return    	MID of the flash.
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
unsigned int flash_read_mid(void);
/**
 * @brief	  	This function serves to read UID of flash.Before reading UID of flash, you must read MID of flash.
 * 				and then you can look up the related table to select the idcmd and read UID of flash.
 * @param[in] 	idcmd	- different flash vendor have different read-uid command. E.g: GD/PUYA:0x4B; XTX: 0x5A
 * @param[in] 	buf		- store UID of flash.
 * @return    	none.
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
void flash_read_uid(unsigned char idcmd, unsigned char *buf);

/*******************************************************************************************************************
 *												Primary interface
 ******************************************************************************************************************/

/**
 * @brief		This function serves to read flash mid and uid,and check the correctness of mid and uid.
 * @param[out]	flash_mid	- Flash Manufacturer ID.
 * @param[out]	flash_uid	- Flash Unique ID.
 * @return		0: flash no uid or not a known flash model 	 1:the flash model is known and the uid is read.
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
int flash_read_mid_uid_with_check( unsigned int *flash_mid, unsigned char *flash_uid);

/**
 * @brief		This function serves to find whether it is zb flash.
 * @param[in]	none.
 * @return		1 - is zb flash;   0 - is not zb flash.
 */
unsigned char flash_is_zb(void);

/**
 * @brief		This function serves to calibration the flash voltage(VDD_F),if the flash has the calib_value,we will use it,either will
 * 				trim vdd_f to 1.95V(2b'111 the max) if the flash is zb.
 * @param[in]	vol - the voltage which you want to set.
 * @return		none.
 */
void flash_vdd_f_calib(void);


#if (MCU_CORE_TYPE == MCU_CORE_8258)
/**
 * @brief		This function serves to get the vdd_f calibration value.
 * @param[in]	none.
 * @return		none.
 */
static inline unsigned char flash_get_vdd_f_calib_value(void)
{
	unsigned int mid = flash_read_mid();
	unsigned char dcdc_flash_volatage = 0;
	switch((mid & 0xff0000) >> 16)
	{
	case(FLASH_SIZE_64K):
		flash_read_page(0xe1c0, 1, &dcdc_flash_volatage);
		break;
	case(FLASH_SIZE_128K):
		flash_read_page(0x1e1c0, 1, &dcdc_flash_volatage);
		break;
	case(FLASH_SIZE_512K):
		flash_read_page(0x771c0, 1, &dcdc_flash_volatage);
		break;
	case(FLASH_SIZE_1M):
		flash_read_page(0xfe1c0, 1, &dcdc_flash_volatage);
		break;
	case(FLASH_SIZE_2M):
		flash_read_page(0x1fe1c0, 1, &dcdc_flash_volatage);
		break;
	default:
		dcdc_flash_volatage = 0xff;
		break;
	}
	return dcdc_flash_volatage;
}
#elif (MCU_CORE_TYPE == MCU_CORE_8278)
/**
 * @brief		This function serves to get the vdd_f calibration value.
 * @param[in]	none.
 * @return		none.
 */
static inline unsigned short flash_get_vdd_f_calib_value(void)
{
	unsigned int mid = flash_read_mid();
	unsigned short flash_volatage = 0;
	switch((mid & 0xff0000) >> 16)
	{
	case(FLASH_SIZE_64K):
		flash_read_page(0xe1c0, 2, (unsigned char*)&flash_volatage);
		break;
	case(FLASH_SIZE_128K):
		flash_read_page(0x1e1c0, 2, (unsigned char*)&flash_volatage);
		break;
	case(FLASH_SIZE_512K):
		flash_read_page(0x771c0, 2, (unsigned char*)&flash_volatage);
		break;
	case(FLASH_SIZE_1M):
		flash_read_page(0xfe1c0, 2, (unsigned char*)&flash_volatage);
		break;
	case(FLASH_SIZE_2M):
		flash_read_page(0x1fe1c0, 2, (unsigned char*)&flash_volatage);
		break;
	default:
		flash_volatage = 0xff;
		break;
	}
	return flash_volatage;
}
#endif
void flash_set_capacity(Flash_CapacityDef flash_cap);

Flash_CapacityDef flash_get_capacity(void);
