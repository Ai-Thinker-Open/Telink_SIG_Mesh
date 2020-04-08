/********************************************************************************************************
 * @file     tutility.c 
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
#include "tutility.h"



#if WIN32
extern u16 my_random(void);
#endif


/****************************************************************************
 * @fn          addrExtCmp
 *
 * @brief       Compare two extended addresses.
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are equal, FALSE otherwise
 */
u8 addrExtCmp(const u8 * pAddr1, const u8 * pAddr2)
{
  u8 i;

  for (i = 8; i != 0; i--)
  {
    if (*pAddr1++ != *pAddr2++)
    {
      return FALSE;
    }
  }
  return TRUE;
}


#if 0
void freeTimerEvent(void **arg)
{
    if ( *arg != NULL ) {
#if (__DEBUG_BUFM__)
		if ( SUCCESS != ev_buf_free((u8*)*arg) ) {
			while(1);
		}
#else
		ev_buf_free((u8*)*arg);
#endif
        *arg = NULL;
    }
}
#endif

void freeTimerTask(void **arg)
{
    if ( *arg == NULL ) {
        return;
    }
//    EV_SCHEDULE_HIGH_TASK((ev_task_callback_t)freeTimerEvent, (void*)arg);
}


/*********************************************************************
 * @fn          generateRandomNum
 *
 * @brief       generate random number
 *
 * @param       len - len
 *
 * @param       data -  buffer
 *
 * @return      None
 */
 #ifndef WIN32
void generateRandomNum(int len, unsigned char *data)
{
	 int i;
	 unsigned int randNums = 0;
	 /* if len is odd */
	 for (i=0; i<len; i++ ) {
#if(MCU_CORE_TYPE == MCU_CORE_8269)
		if( (i & 1) == 0 ){
#else
		if( (i & 3) == 0 ){
#endif
			 randNums = rand();
		 }

		 data[i] = randNums & 0xff;
		 randNums >>=8;
	 }
}
#endif

// general swap/endianness utils

void swapX(const u8 *src, u8 *dst, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[len - 1 - i] = src[i];
}

void swap24(u8 dst[3], const u8 src[3])
{
    swapX(src, dst, 3);
}

void swap32(u8 dst[4], const u8 src[4])
{
    swapX(src, dst, 4);
}

void swap48(u8 dst[6], const u8 src[6])
{
    swapX(src, dst, 6);
}

void swap56(u8 dst[7], const u8 src[7])
{
    swapX(src, dst, 7);
}

void swap64(u8 dst[8], const u8 src[8])
{
    swapX(src, dst, 8);
}

void swap128(u8 dst[16], const u8 src[16])
{
    swapX(src, dst, 16);
}

void net_store_16(u8 *buffer, u16 pos, u16 value)
{
    buffer[pos++] = (value >> 8) & 0xff;
    buffer[pos++] = value & 0xff;
}


void flip_addr(u8 *dest, u8 *src){
    dest[0] = src[5];
    dest[1] = src[4];
    dest[2] = src[3];
    dest[3] = src[2];
    dest[4] = src[1];
    dest[5] = src[0];
}

void store_16(u8 *buffer, u16 pos, u16 value){
    buffer[pos++] = value & 0xff;
    buffer[pos++] = value >> 8;
}



void my_fifo_init (my_fifo_t *f, u8 s, u8 n, u8 *p)
{
    // no need irq_disable
	f->size = s;
	f->num = n;
	f->wptr = 0;
	f->rptr = 0;
	f->p = p;
}

u8* my_fifo_wptr (my_fifo_t *f)
{
    // no need irq_disable
	if (((f->wptr - f->rptr) & 255) < f->num)
	{
		return f->p + (f->wptr & (f->num-1)) * f->size;
	}
	return 0;
}

void my_fifo_next (my_fifo_t *f)
{
    // no need irq_disable, because app_advertise_prepare_handler() just use rptr.
	f->wptr++;
}

int my_fifo_push (my_fifo_t *f, u8 *p, u16 n, u8 *head, u8 head_len)
{
	if (((f->wptr - f->rptr) & 255) >= f->num)
	{
		return -1;
	}

	if (n + (2+head_len) > f->size)    // sizeof(len) == 2
	{
		return -1;
	}
	u8 r = irq_disable();
	u8 *pd = f->p + (f->wptr & (f->num-1)) * f->size;
	*pd++ = (n+head_len) & 0xff;
	*pd++ = (n+head_len) >> 8;
	foreach(i,head_len){
		*pd++ = *head++;
	}
	memcpy (pd, p, n);
	f->wptr++;			// should be last for VC
	irq_restore(r);
	return 0;
}

void my_fifo_pop (my_fifo_t *f)
{
	//u8 r = irq_disable();
	f->rptr++;
	//irq_restore(r);
}

u8 * my_fifo_get (my_fifo_t *f)
{
    u8 *p = 0;
    
	u8 r = irq_disable();
	if (f->rptr != f->wptr)
	{
		p = f->p + (f->rptr & (f->num-1)) * f->size;
	}
	irq_restore(r);
	
	return p;
}

u8 * my_fifo_get_offset (my_fifo_t *f, u8 offset)
{
    return (f->p + ((f->rptr + offset) & (f->num-1)) * f->size);
}

u8 my_fifo_data_cnt_get (my_fifo_t *f)
{
	return (f->wptr - f->rptr);
}

u8 my_fifo_free_cnt_get(my_fifo_t *f)
{
	return (f->num - my_fifo_data_cnt_get(f));
}

void my_fifo_reset(my_fifo_t *f)
{
	f->rptr = f->wptr;
}
