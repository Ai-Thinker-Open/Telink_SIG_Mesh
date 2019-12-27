/********************************************************************************************************
 * @file     crc_sw.c 
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

/**************************** Includes ****************************************/

#include "../tl_common.h"
#include "crc_sw.h"

/**************************** Private Macro Definitions ***********************/

/* None */

/**************************** Private type Definitions ************************/

/* None */

/**************************** Private Enum Definitions ************************/

/* None */

/**************************** Private Variable Definitions ********************/

/* None */

/**************************** Public Variable Definitions *********************/



/**************************** Private Constant Definitions ********************/

static u32 crcTable[256];

u16 crc16_mode_poly[3] = {0x1021, 0x8005, 0x8bb7};

/**************************** Public Constant Definitions *********************/

/* None */

/**************************** Private Function Prototypes *********************/

/* None */

/**************************** Public Functions Definitions ********************/

/****************************************************************************************/

void crc32_buildTable(u8 endian)
{
    u32 polynomial = endian ? CRC_MODE3_POLY : 0xedb88320;
    u32 c;
    int i, j;

    for(i = 0; i < 256; i++)
    {
        c = endian ? (i << 24) : i;
        for(j = 8; j; j--)
        {
            if(endian)
                c = (c & 0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
            else
                c = (c & 1) ? ((c >> 1) ^ polynomial) : (c >> 1);
        }

        crcTable[i] = c;
    }
}


/****************************************************************************************/
#if 0
uint32_t crc32_getCrc(const uint8_t *str, uint32_t count, u8 endian)
{
    uint32_t checksum = endian ? 0 : ~0;
    crc32_calcCrc(&checksum, str, count, endian);
    return endian ? checksum : ~checksum;
}

/****************************************************************************************/

void crc32_calcCrc(uint32_t *checksum, const uint8_t *str, uint32_t count, u8 endian)
{
    uint32_t j = 0;
    for( j = 0; j < count; ++j)
    {
        uint32_t tableIndex = 0;
        if(true == endian)
        {
            tableIndex = (((*checksum) >> 24) ^ *str++) & 0xff;
            *checksum = ((*checksum) << 8) ^ crcTable[tableIndex];
        }
        else
        {
            tableIndex = ((*checksum) ^ *str++) & 0xff;
            *checksum = ((*checksum) >> 8) ^ crcTable[tableIndex];
        }
    }
}
#endif
/****************************************************************************************/

//16-bit CRC calculation function
u16 crc16_bitwise(u8 *p_stream, u32 len_m1, u16 crc16_poly)
{
    unsigned int i,j;
    unsigned short crc16_result;

    crc16_result = 0;

    for(i=0; i<=len_m1; i++) {
        //printf("calc %02x\n",*p_stream);
        for(j=0; j<8; j++,*p_stream<<=1) {
            if(crc16_result & 0x8000) {
                crc16_result <<= 1;
                crc16_result |= (*p_stream & 0x80) >> 7;
                crc16_result ^= crc16_poly;
            }else{
                crc16_result <<= 1;
                crc16_result |= (*p_stream & 0x80) >> 7;
            }
        }

        p_stream++;
    }

    for(j=0; j<16; j++) {
        if(crc16_result & 0x8000) {
            crc16_result <<= 1;
            crc16_result ^= crc16_poly;
        }
        else {
            crc16_result <<= 1;
        }
    }
    return crc16_result;
}

/****************************************************************************************/
u32 crc32_bitwise(u8 *p_stream, u32 len_m1, u32 crc32_poly)
{
    u32 i,j;
    u32 crc32_result;

    crc32_result = 0;

    for(i=0; i<=len_m1; i++)
    {
        for(j=0;j<8;j++,*p_stream<<=1)
        {
            if(crc32_result & 0x80000000)
            {
                crc32_result <<= 1;
                crc32_result |= (*p_stream & 0x80) >> 7;
                crc32_result ^= crc32_poly;
            }
            else
            {
                crc32_result <<= 1;
                crc32_result |= (*p_stream & 0x80) >> 7;
            }
        }

        p_stream++;
    }

    for(i=0;i<32;i++)
    {
        if(crc32_result & 0x80000000)
        {
            crc32_result <<= 1;
            crc32_result ^= crc32_poly;
        }
        else
        {
            crc32_result <<= 1;
        }
    }

    return crc32_result;
}

/****************************************************************************************/

u32 crc_reverseBitU32(const u32 src)
{
    u32 temp = src;
    u32 result = 0;
    int  k;
    for(k=0; k<32;k++)
    {
        result <<= 1;
        result += temp & 1;
        temp >>= 1;
    }
    return result;
}

/****************************************************************************************/

u16 crc_reverseBitU16(const u16 src)
{
    u16 temp = src;
    u16 result = 0;
    int  k;
    for(k=0; k<16;k++)
    {
        result <<= 1;
        result += temp & 1;
        temp >>= 1;
    }
    return result;
}


u8 reverseBitU8(const u8 src)
{
    u8 temp = src;
    u8 result = 0;
    int k;

    for(k = 0; k < 8; k++) {
        result <<= 1;
        result += temp & 1;
        temp >>= 1;
    }
    return result;
}




u16 crc16_calc(u8* crcArray, u8 len)
{
    int i;
    u16 crcResult;
    u8 temp[128];

    memcpy(temp, crcArray, len);

    for(i = 0; i < len; i++) {
        temp[i] = reverseBitU8(temp[i]);
    }

    crcResult = crc16_bitwise(temp, len-1, CRC_MODE0_POLY);
    crcResult= crc_reverseBitU16(crcResult);

    return crcResult;
}

