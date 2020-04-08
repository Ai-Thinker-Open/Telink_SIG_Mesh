/********************************************************************************************************
 * @file     factory_reset.c 
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

#include "proj/tl_common.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj_lib/ble/ll//ll.h"
#include "app_beacon.h"

//FLASH_ADDRESS_EXTERN;

//////////////////Factory Reset///////////////////////////////////////////////////////////////////////
void show_ota_result(int result);
void show_factory_reset();
int factory_reset();
void set_firmware_type_init();
int mesh_reset_network(u8 provision_enable);

extern u8 manual_factory_reset;

#define FACTORY_RESET_LOG_EN        0

#if !WIN32
static int adr_reset_cnt_idx = 0;

#if 0   // org mode
static int reset_cnt = 0;

#define SERIALS_CNT                     (5)   // must less than 7

const u8 factory_reset_serials[SERIALS_CNT * 2] = { 0, 3,    // [0]:must 0
                                                	0, 3,    // [2]:must 0
                                                	0, 3,    // [4]:must 0
                                                	3, 30,
                                                	3, 30};

#define RESET_CNT_RECOUNT_FLAG          0
#define RESET_FLAG                      0x80

void	reset_cnt_clean ()
{
	if (adr_reset_cnt_idx < 3840)
	{
		return;
	}
	flash_erase_sector (FLASH_ADR_RESET_CNT);
	adr_reset_cnt_idx = 0;
}

void write_reset_cnt (u8 cnt)
{
	flash_write_page (FLASH_ADR_RESET_CNT + adr_reset_cnt_idx, 1, (u8 *)(&cnt));
}

void clear_reset_cnt ()
{
    write_reset_cnt(RESET_CNT_RECOUNT_FLAG);
}

int reset_cnt_get_idx ()		//return 0 if unconfigured
{
	u8 *pf = (u8 *)FLASH_ADR_RESET_CNT;
	for (adr_reset_cnt_idx=0; adr_reset_cnt_idx<4096; adr_reset_cnt_idx++)
	{
	    u8 restcnt_bit = pf[adr_reset_cnt_idx];
		if (restcnt_bit != RESET_CNT_RECOUNT_FLAG)	//end
		{
        	if(((u8)(~(BIT(0)|BIT(1)|BIT(2)|BIT(3))) == restcnt_bit)  // the fourth not valid
        	 ||((u8)(~(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5))) == restcnt_bit)){  // the fifth not valid
                clear_reset_cnt();
            }else{
			    break;
			}
		}
	}

    reset_cnt_clean();
    
	return 1;
}

u8 get_reset_cnt_bit ()
{
	if (adr_reset_cnt_idx < 0)
	{
	    reset_cnt_clean();
		return 0;
	}
	
	u8 reset_cnt;
	flash_read_page(FLASH_ADR_RESET_CNT + adr_reset_cnt_idx, 1, &reset_cnt);
	return reset_cnt;
}

void increase_reset_cnt ()
{
	u8 restcnt_bit = get_reset_cnt_bit();
	foreach(i,8){
        if(restcnt_bit & BIT(i)){
            if(i < 3){
                reset_cnt = i;
            }else if(i < 5){
                reset_cnt = 3;
            }else if(i < 7){
                reset_cnt = 4;
            }
            
            restcnt_bit &= ~(BIT(i));
            write_reset_cnt(restcnt_bit);
            break;
        }
	}
}

int factory_reset_handle ()
{
    reset_cnt_get_idx();   
    u8 restcnt_bit; 
    restcnt_bit = get_reset_cnt_bit();
	if(restcnt_bit == RESET_FLAG){
        irq_disable();
        factory_reset();
        show_ota_result(OTA_SUCCESS);
	    start_reboot();
	}else{
        increase_reset_cnt();
	}
	return 0;
}

int factory_reset_cnt_check ()
{
    static u8 clear_st = 3;
    static u32 reset_check_time;

	if(0 == clear_st) return 0;

	if(3 == clear_st){
        clear_st--;
        reset_check_time = factory_reset_serials[reset_cnt*2];
    }
    
	if((2 == clear_st) && clock_time_exceed(0, reset_check_time*1000*1000)){
	    clear_st--;
	    reset_check_time = factory_reset_serials[reset_cnt*2 + 1];
	    if(3 == reset_cnt || 4 == reset_cnt){
            increase_reset_cnt();
        }
	}
    
	if((1 == clear_st) && clock_time_exceed(0, reset_check_time*1000*1000)){
	    clear_st = 0;
        clear_reset_cnt();
	}
	
	return 0;
}

#else

/****************************************
new factory reset:
user can change any one of factory_reset_serials, and also can change SERIALS_CNT
*****************************************/

const u8 factory_reset_serials[] = { 0, 3,
                                     0, 3,
                                     0, 3,
                                     0, 3,
                                     0, 3,};

#define RESET_CNT_INVALID               0
#define RESET_TRIGGER_VAL               (sizeof((factory_reset_serials)))

STATIC_ASSERT(sizeof(factory_reset_serials) % 2 == 0);
STATIC_ASSERT(sizeof(factory_reset_serials) < 100);

void	reset_cnt_clean ()
{
	if (adr_reset_cnt_idx < 3840)
	{
		return;
	}
	flash_erase_sector (FLASH_ADR_RESET_CNT);
	adr_reset_cnt_idx = 0;
}

void write_reset_cnt (u8 cnt) // reset cnt value from 1 to 254, 0 is invalid cnt
{
    reset_cnt_clean ();
	flash_write_page (FLASH_ADR_RESET_CNT + adr_reset_cnt_idx, 1, (u8 *)(&cnt));
}

void clear_reset_cnt ()
{
    write_reset_cnt(RESET_CNT_INVALID);
}

void reset_cnt_get_idx ()		//return 0 if unconfigured
{
	u8 *pf = (u8 *)FLASH_ADR_RESET_CNT;
	for (adr_reset_cnt_idx=0; adr_reset_cnt_idx<4096; adr_reset_cnt_idx++)
	{
	    u8 restcnt = pf[adr_reset_cnt_idx];
		if (restcnt != RESET_CNT_INVALID)	//end
		{
		    if(0xFF == restcnt){
		        // do nothing
		    }else if((restcnt > RESET_TRIGGER_VAL) || (restcnt & BIT(0))){   // invalid state
                clear_reset_cnt();
            }
			break;
		}
	}
}

u8 get_reset_cnt () // reset cnt value from 1 to 254, 0 is invalid cnt
{
	u8 reset_cnt;
	flash_read_page(FLASH_ADR_RESET_CNT + adr_reset_cnt_idx, 1, &reset_cnt);
	return reset_cnt;
}

void increase_reset_cnt ()
{
	u8 reset_cnt = get_reset_cnt();
	if(0xFF == reset_cnt){
	    reset_cnt = 0;
	}else if(reset_cnt){
	    clear_reset_cnt();      // clear current BYTE and then use next BYTE
        adr_reset_cnt_idx++;
	}
	
	reset_cnt++;
	write_reset_cnt(reset_cnt);
	#if FACTORY_RESET_LOG_EN
    LOG_USER_MSG_INFO(0,0,"cnt %d\r\n",reset_cnt);
    #endif
}

int factory_reset_handle ()
{
    reset_cnt_get_idx();   
	if(get_reset_cnt() == RESET_TRIGGER_VAL){
	    #if MANUAL_FACTORY_RESET_TX_STATUS_EN
	    manual_factory_reset = 1;
        #else
        irq_disable();
        factory_reset();
            #if DUAL_MODE_WITH_TLK_MESH_EN
        UI_resotre_TLK_4K_with_check();
            #endif
            #if FACTORY_RESET_LOG_EN
        LOG_USER_MSG_INFO(0,0,"factory reset success\r\n",0);
            #endif
        show_factory_reset();
	    //start_reboot();
	    #endif
	}else{
        increase_reset_cnt();
	}
	return 0;
}

#define VALID_POWER_ON_TIME_US 	(50*1000)
int factory_reset_cnt_check ()
{
    static u8 clear_st = 4;
    static u32 reset_check_time;

	if(0 == clear_st) return 0;
	if(4 == clear_st && clock_time_exceed(0, VALID_POWER_ON_TIME_US)){
		clear_st--;
		factory_reset_handle();
	}
	if(3 == clear_st){
        clear_st--;
        reset_check_time = factory_reset_serials[get_reset_cnt() - 1];
    }
    
	if((2 == clear_st) && clock_time_exceed(0, reset_check_time*1000*1000)){
	    clear_st--;
        increase_reset_cnt();
	    reset_check_time = factory_reset_serials[get_reset_cnt() - 1];
	}
    
	if((1 == clear_st) && clock_time_exceed(0, reset_check_time*1000*1000)){
	    clear_st = 0;
        clear_reset_cnt();
        #if FACTORY_RESET_LOG_EN
        LOG_USER_MSG_INFO(0,0,"cnt clear\r\n",0);
        #endif
	}
	
	return 0;
}

#endif
#endif

#if FLASH_1M_ENABLE
int factory_reset() // 1M flash
{
	u8 r = irq_disable ();
	for(int i = 0; i < (FLASH_ADR_AREA_1_END - FLASH_ADR_AREA_1_START) / 4096; ++i){
	    u32 adr = FLASH_ADR_AREA_1_START + i*0x1000;
	    if(adr != FLASH_ADR_RESET_CNT){
		    flash_erase_sector(adr);
		}
	}

	if((FLASH_ADR_MESH_TYPE_FLAG < FLASH_ADR_AREA_1_START) || (FLASH_ADR_MESH_TYPE_FLAG >= FLASH_ADR_AREA_1_END)){
        flash_erase_sector(FLASH_ADR_MESH_TYPE_FLAG);
    }
	// no area2

	#if HOMEKIT_EN
        #if 1
	extern flash_adr_layout_def flash_adr_layout;
 	flash_erase_sector((u32)flash_adr_layout.flash_adr_hash_id);

    flash_erase_sector((u32)flash_adr_layout.flash_adr_device_id);
    flash_erase_sector((u32)flash_adr_layout.flash_adr_srp_key);
    flash_erase_sector((u32)flash_adr_layout.flash_adr_id_info);
    flash_erase_sector((u32)flash_adr_layout.flash_adr_id_info + 0x1000);
    flash_erase_sector((u32)flash_adr_layout.flash_adr_global_state);
    flash_erase_sector((u32)flash_adr_layout.flash_adr_char_value);
//R10 clear broadcast event param.
    extern void blt_clean_broadcast_param(void);
	blt_clean_broadcast_param();
        #else
	for(int i = 0; i < (FLASH_ADR_HOMEKIT_AREA_END - FLASH_ADR_HOMEKIT_AREA_START) / 4096; ++i){
	    flash_erase_sector(FLASH_ADR_HOMEKIT_AREA_START + i*0x1000);
	}
        #endif
    #elif (MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
    for(int i = 0; i < (FLASH_ADR_USER_MESH_END - FLASH_ADR_USER_MESH_START) / 4096; ++i){
        flash_erase_sector(FLASH_ADR_USER_MESH_START + i*0x1000);
    }
	#endif
	
    flash_erase_sector(FLASH_ADR_RESET_CNT);    // at last should be better, when power off during factory reset erase.
    irq_restore(r);
	return 0;
}
#else
int factory_reset(){
	u8 r = irq_disable ();
	for(int i = 0; i < (FLASH_ADR_AREA_1_END - FLASH_ADR_AREA_1_START) / 4096; ++i){
	    u32 adr = FLASH_ADR_AREA_1_START + i*0x1000;
	    if(adr != FLASH_ADR_RESET_CNT){
		    flash_erase_sector(adr);
		}
	}
	
	for(int i = 0; i < (FLASH_ADR_AREA_2_END - FLASH_ADR_AREA_2_START) / 4096; ++i){
	    u32 adr = FLASH_ADR_AREA_2_START + i*0x1000;
	    if(adr != FLASH_ADR_RESET_CNT){
		    flash_erase_sector(adr);
		}
	}
	
	for(int i = 1; i < (FLASH_ADR_PAR_USER_MAX - (CFG_SECTOR_ADR_CALIBRATION_CODE)) / 4096; ++i){
		#if XIAOMI_MODULE_ENABLE
		if(i < (((FLASH_ADR_PAR_USER_MAX - (CFG_SECTOR_ADR_CALIBRATION_CODE)) / 4096) - 1)){ // the last sector is FLASH_ADR_MI_AUTH
            u32 adr = (CFG_SECTOR_ADR_CALIBRATION_CODE + i*0x1000);
		    #if DUAL_VENDOR_EN
		    if(adr != FLASH_ADR_THREE_PARA_ADR)
		    #endif
		    {
			    flash_erase_sector(adr);
			}
		}
		#else
		//flash_erase_sector(CFG_SECTOR_ADR_CALIBRATION_CODE + i*0x1000);
		#endif
	}

	#if DUAL_MODE_ADAPT_EN
	if(DUAL_MODE_NOT_SUPPORT != dual_mode_state){   // dual_mode_state have been init before
	    set_firmware_type_init();
	}
	#endif

    flash_erase_sector(FLASH_ADR_RESET_CNT);    // at last should be better, when power off during factory reset erase.

    irq_restore(r);
	return 0;
}
#endif

void kick_out(){
	#if !WIN32
	// add terminate cmd 
	if(bls_ll_isConnectState()){
		bls_ll_terminateConnection (0x13);
	}
	#endif
	sleep_us(500000);   // wait tx buffer send completed.
    factory_reset();
    #if DUAL_MODE_WITH_TLK_MESH_EN
    UI_resotre_TLK_4K_with_check();
    #endif
#if 0
	mesh_reset_network(1);
	show_factory_reset();
#else
    #if !WIN32
    show_factory_reset(); // reboot when showed
    #endif
#endif
}

