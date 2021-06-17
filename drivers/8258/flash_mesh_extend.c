/********************************************************************************************************
 * @file	flash_mesh_extend.c
 *
 * @brief	This is the source file for B85
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

#include "proj/tl_common.h"
#include "proj/drivers/spi.h"
#include "flash.h"
#include "proj/drivers/spi_i.h"
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/ble/blt_config.h"

u32 flash_sector_mac_address = CFG_SECTOR_ADR_MAC_CODE;
u32 flash_sector_calibration = CFG_SECTOR_ADR_CALIBRATION_CODE;

#if AUTO_ADAPT_MAC_ADDR_TO_FLASH_TYPE_EN
void blc_readFlashSize_autoConfigCustomFlashSector(void)
{
#if (((MCU_CORE_TYPE == MCU_CORE_8267)||(MCU_CORE_TYPE == MCU_CORE_8269)) \
    || (MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE) || DUAL_MESH_ZB_BL_EN)
    // always use fixed customized address
#else
	u8 *temp_buf;
	unsigned int mid = flash_read_mid();
	temp_buf = (u8 *)&mid;
	u8	flash_cap = temp_buf[2];

    if(CFG_ADR_MAC_512K_FLASH == CFG_SECTOR_ADR_MAC_CODE){
    	if(flash_cap == FLASH_SIZE_1M){
    	    #define MAC_SIZE_CHECK      (6)
    	    u8 mac_null[MAC_SIZE_CHECK] = {0xff,0xff,0xff,0xff,0xff,0xff};
    	    u8 mac_512[MAC_SIZE_CHECK], mac_1M[MAC_SIZE_CHECK];
    	    flash_read_page(CFG_ADR_MAC_512K_FLASH, MAC_SIZE_CHECK, mac_512);
    	    flash_read_page(CFG_ADR_MAC_1M_FLASH, MAC_SIZE_CHECK, mac_1M);
    	    if((0 == memcmp(mac_512,mac_null, MAC_SIZE_CHECK))
    	     &&(0 != memcmp(mac_1M,mac_null, MAC_SIZE_CHECK))){
        		flash_sector_mac_address = CFG_ADR_MAC_1M_FLASH;
        		flash_sector_calibration = CFG_ADR_CALIBRATION_1M_FLASH;
    		}
    	}
	}else if(CFG_ADR_MAC_1M_FLASH == CFG_SECTOR_ADR_MAC_CODE){
	    if(flash_cap != FLASH_SIZE_1M){
            while(1){ // please check your Flash size
                #if(MODULE_WATCHDOG_ENABLE)
                wd_clear();
                #endif
            }
		}
	}

	flash_set_capacity(flash_cap);
#endif
}
#endif



#if(HOMEKIT_EN)

void flash_write_data (unsigned long addr, unsigned long len, unsigned char *buf)
{
	int ns = 256 - (addr & 0xff);
	do {
		int nw = len > ns ? ns : len;
		flash_write_page (addr, nw, buf);
		ns = 256;
		addr += nw;
		buf += nw;
		len -= nw;
	} while (len > 0);
}

void flash_write_val(unsigned long adr, unsigned long flag_invalid, unsigned long dat)
{
	unsigned long p = 0xffffffff;
	int i;
	for (i=0; i<1024; i++)
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);
		if (p == 0xffffffff)
		{
			break;
		}
		else if (p != flag_invalid)
		{
			if (i == 1023)
			{
				flash_erase_sector (adr);
				i = 0;
			}
			else
			{
				flash_write_data (adr + i * 4, 4, (unsigned char *)&flag_invalid);
				i++;
			}
			break;
		}
	}
	flash_write_data (adr + i * 4, 4, (unsigned char *)&dat);
}

unsigned long flash_read_val(unsigned long adr, unsigned long flag_invalid)
{
	unsigned long p = 0xffffffff;
	int i = 0;
	for (i=0; i<1024; i++)
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);
		if (p == 0xffffffff)
		{
			break;
		}
		else if (p != flag_invalid)
		{
			return p;
		}
	}
	if( i>1000 )
		flash_erase_sector (adr);
	return flag_invalid;
}


void flash_write_long_val(unsigned long adr, unsigned long flag_invalid, unsigned char* dat, unsigned char len)
{
	unsigned long p = 0xffffffff;
	int i;
	int data_len = len / 4;
	for (i=0; i<(1024-data_len);)
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);
		if (p == 0xffffffff)
		{
			break;
		}
		else if (p != flag_invalid)
		{
			if (i >= 1024-data_len)
			{
				flash_erase_sector (adr);
				i = 0;
				break;
			}
			else
			{
				flash_write_data (adr + i * 4, 4, (unsigned char *)&flag_invalid);
				i+=data_len;
				break;
			}
		}
		i+=data_len;
	}
	flash_write_data (adr + i * 4, len, dat);

}

unsigned long flash_read_long_val(unsigned long adr, unsigned long flag_invalid, unsigned char* dat, unsigned char len)
{
	unsigned long p = 0xffffffff;

	int data_len = (len + 3) / 4;		//+3 to make len to be true len.
	int i = 0;
	for ( i=0; i<(1024-data_len); )
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);

		if( p == 0xffffffff )
		{
			break;
		}
		else if (p != flag_invalid)
		{
			flash_read_page(adr + i * 4, len, dat);
			return p;
		}
		i+=data_len;
	}
	if( i >= 1000 )
		flash_erase_sector (adr);
	return flag_invalid;
}

unsigned long flash_subregion_write_val (unsigned long adr, unsigned long flag_invalid, unsigned long dat, unsigned long num)
{
	unsigned long p = 0xffffffff;
	int i;
	for (i=0; i<num; i++)
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);
		if (p == 0xffffffff)
		{
			break;
		}
		else if (p != flag_invalid)
		{
			if (i >= (num - 1))
			{
				return flag_invalid;
			}
			else
			{
				flash_write_data (adr + i * 4, 4, (unsigned char *)&flag_invalid);
				i++;
			}
			break;
		}
	}
	flash_write_data (adr + i * 4, 4, (unsigned char *)&dat);

	return 1;
}

unsigned long flash_subregion_read_val (unsigned long adr, unsigned long flag_invalid, unsigned long num)
{
	unsigned long p = 0xffffffff;
	int i = 0;
	for (i=0; i<num; i++)
	{
		flash_read_page(adr + i *4, 4, (unsigned char *)&p);
		if (p == 0xffffffff)
		{
			break;
		}
		else if (p != flag_invalid)
		{
			return p;
		}
	}

	//if((i>1000) || (i>num))
	//	flash_erase_sector (adr);

	return flag_invalid;
}
#endif


