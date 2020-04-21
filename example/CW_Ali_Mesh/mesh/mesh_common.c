/********************************************************************************************************
 * @file     mesh_common.c 
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
#if !WIN32
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"
#include "myprintf.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "app_provison.h"
#include "lighting_model.h"
#include "mesh/sensors_model.h"
#include "mesh/remote_prov.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
#include "lighting_model_LC.h"
#include "mesh_ota.h"
#include "mesh_common.h"
#include "mesh_config.h"
    
#if(__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258))
#include "stack/ble/ble.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "stack/ble_8278/ble.h"
#endif
#if WIN32 
#include "../../../reference/tl_bulk/lib_file/host_fifo.h"
#endif
#if MI_API_ENABLE 
#include "mesh/mi_api/telink_sdk_mible_api.h"
#endif 
#if FAST_PROVISION_ENABLE
#include "mesh/fast_provision_model.h"
#endif

#if (HCI_ACCESS==HCI_USE_UART)
#include "proj/drivers/uart.h"
#endif

#if HOMEKIT_EN
#include "../mesh_gw_node_homekit/hk_CategoryDef.h"
#include "../../homekit_src/homekit_inc.h"
#include "homekit_src/hk_adv_packet.h"
#include "../mesh_gw_node_homekit/app.h"
#endif

#ifndef WIN32
#if PM_DEEPSLEEP_RETENTION_ENABLE
asm(".equ __PM_DEEPSLEEP_RETENTION_ENABLE,    1");
#else
asm(".equ __PM_DEEPSLEEP_RETENTION_ENABLE,    0");
#endif
asm(".global     __PM_DEEPSLEEP_RETENTION_ENABLE");

#if FLASH_1M_ENABLE && (0 == FW_START_BY_BOOTLOADER_EN)
asm(".equ __FLASH_512K_ENABLE,    0");
#else
asm(".equ __FLASH_512K_ENABLE,    1");
#endif
asm(".global     __FLASH_512K_ENABLE");

#if __PROJECT_BOOTLOADER__
asm(".equ __FW_OFFSET,      0"); // must be 0
#elif (FW_START_BY_BOOTLOADER_EN)
    #if (DUAL_MODE_FW_ADDR_SIGMESH == 0x80000)
asm(".equ __FW_OFFSET,      0x80000");  // must be equal to DUAL_MODE_FW_ADDR_SIGMESH
    #endif
#else
asm(".equ __FW_OFFSET,      0");
#endif
asm(".global     __FW_OFFSET");

#if (FW_START_BY_BOOTLOADER_EN)
asm(".equ __FW_START_BY_BOOTLOADER_EN,  1");
#else
asm(".equ __FW_START_BY_BOOTLOADER_EN,  0");
#endif
asm(".global     __FW_START_BY_BOOTLOADER_EN");

#if __PROJECT_BOOTLOADER__
asm(".equ __BOOT_LOADER_EN,         1");
#else
asm(".equ __BOOT_LOADER_EN,         0");
#endif
asm(".global     __BOOT_LOADER_EN");

#if (FW_RAMCODE_SIZE_MAX == 0x4000)
asm(".equ __FW_RAMCODE_SIZE_MAX,    0x4000");   // must be equal to FW_RAMCODE_SIZE_MAX
#endif
asm(".global     __FW_RAMCODE_SIZE_MAX");

#if (MCU_RUN_SRAM_WITH_CACHE_EN || MCU_RUN_SRAM_EN)
asm(".equ __MCU_RUN_SRAM_EN,         1");
#else
asm(".equ __MCU_RUN_SRAM_EN,         0");
#endif
asm(".global     __MCU_RUN_SRAM_EN");

#endif


STATIC_ASSERT(TRANSMIT_CNT_DEF < 8);
STATIC_ASSERT(TRANSMIT_CNT_DEF_RELAY < 8);
STATIC_ASSERT(sizeof(mesh_scan_rsp_t) <= 31);
#if (CHIP_TYPE == CHIP_TYPE_8269)
STATIC_ASSERT((0 == FLASH_1M_ENABLE) && (0 == PINGPONG_OTA_DISABLE));
#endif
#if (PINGPONG_OTA_DISABLE)
STATIC_ASSERT(1 == FLASH_1M_ENABLE);
#else
STATIC_ASSERT(FLASH_ADR_AREA_FIRMWARE_END <= 0x3F000);
#endif
#ifdef FLASH_ADR_AREA_FIRMWARE_END
STATIC_ASSERT(FLASH_ADR_AREA_FIRMWARE_END%0x1000 == 0);
#endif

#if((HCI_ACCESS==HCI_USE_UART)&&(MCU_CORE_TYPE == MCU_CORE_8269))
#if (UART_GPIO_SEL == UART_GPIO_8267_PC2_PC3)
#define PWM_UART_IO_CONFLICT_PC2    ((PWM_R == GPIO_PC2)||(PWM_G == GPIO_PC2)||(PWM_B == GPIO_PC2)||(PWM_W == GPIO_PC2))
#define PWM_UART_IO_CONFLICT_PC3    ((PWM_R == GPIO_PC3)||(PWM_G == GPIO_PC3)||(PWM_B == GPIO_PC3)||(PWM_W == GPIO_PC3))
//STATIC_ASSERT((!PWM_UART_IO_CONFLICT_PC2) && (!PWM_UART_IO_CONFLICT_PC3));
//TODO("PWM IO was conflicted with UART IO. Please redefine PWM IO !!!");
// ASSERT
typedef char assert_PWM_IO_is_conflicted_with_UART_IO_Please_redefine_PWM_IO[(!!(!(PWM_UART_IO_CONFLICT_PC2 || PWM_UART_IO_CONFLICT_PC3)))*2-1];
#endif
#endif

#if TESTCASE_FLAG_ENABLE
#define HCI_TX_FIFO_SIZE	(132)
#define HCI_TX_FIFO_NUM		(8)
#else
#if (IS_VC_PROJECT)
#define HCI_TX_FIFO_SIZE	(256)
#define HCI_TX_FIFO_NUM		(128)
#else
#if (FEATURE_PROXY_EN)
	#if (HCI_ACCESS == HCI_USE_USB)
#define HCI_TX_FIFO_SIZE	(REPORT_ADV_BUF_SIZE)
	#else
	    #if HCI_LOG_FW_EN
#define HCI_TX_FIFO_SIZE	(80)
        #else
#define HCI_TX_FIFO_SIZE	(64)
        #endif
	#endif
	#if __PROJECT_SPIRIT_LPN__
#define HCI_TX_FIFO_NUM		(4)
	#else
#define HCI_TX_FIFO_NUM		(16)
	#endif
#else
#define HCI_TX_FIFO_SIZE	(128)
#define HCI_TX_FIFO_NUM		(8)
#endif
#endif
#endif

#define HCI_TX_FIFO_SIZE_USABLE     (HCI_TX_FIFO_SIZE - 2)  // 2: sizeof(fifo.len)

#if(HCI_ACCESS != HCI_USE_NONE)
MYFIFO_INIT(hci_tx_fifo, HCI_TX_FIFO_SIZE, HCI_TX_FIFO_NUM); // include adv pkt and other event


#if (IS_VC_PROJECT)
MYFIFO_INIT(hci_rx_fifo, 512, 4);   // max play load 382
#else
#define UART_DATA_SIZE              (72)    // increase or decrease 16bytes for each step.
#define HCI_RX_FIFO_SIZE            (UART_DATA_SIZE + 4 + 4)    // 4: sizeof DMA len;  4: margin reserve
STATIC_ASSERT(HCI_RX_FIFO_SIZE % 16 == 0);

MYFIFO_INIT(hci_rx_fifo, HCI_RX_FIFO_SIZE, 4);
#endif
#endif

u8 g_reliable_retry_cnt_def = RELIABLE_RETRY_CNT_DEF;
u8 pair_login_ok = 0;

u16 g_vendor_id = VENDOR_ID;
#if WIN32
u32 g_vendor_md_light_vc_s = VENDOR_MD_LIGHT_S;
u32 g_vendor_md_light_vc_c = VENDOR_MD_LIGHT_C;
    #if MD_VENDOR_2ND_EN
u32 g_vendor_md_light_vc_s2 = VENDOR_MD_LIGHT_S2;
    #endif
#endif
u8  mesh_user_define_mode = MESH_USER_DEFINE_MODE;  // for libary use
u16	current_connHandle = BLE_INVALID_CONNECTION_HANDLE;	 //	handle of  connection

u8 mesh_rsp_random_delay_step = 0;// 10ms unit 
u32 mesh_rsp_random_delay_tick =0;
u8 mesh_need_random_delay = 0;

#define MAX_BEACON_SEND_INTERVAL 	2000*1000
beacon_send_str beacon_send;

void beacon_str_init()
{
	beacon_send.conn_beacon_flag = 1;
	beacon_send.start_time_s = system_time_s;
	beacon_send.tick = clock_time();
	beacon_send.en = 1;
	beacon_send.inter = MAX_BEACON_SEND_INTERVAL;
	return ;
}


void mesh_mi_cfg_segmust_set(material_tx_cmd_t *p)
{
#if MI_API_ENABLE
	#if MI_SWITCH_LPN_EN
	p->seg_must = 0;// in the mi_mode ,when the lpn mode enable ,the segment should change to 0
	#else
	p->seg_must = 1;// in the mi_mode should set the segment to 1
	#endif
#endif
}

u8 get_flash_data_is_valid(u8 *p_data,u16 len)
{
	for(int i=0;i<len;i++){
		if(p_data[i] != 0xff){
			return 1;
		}
	}
	return 0;
}

#if !WIN32
u8 check_ecdh_crc(mesh_ecdh_key_str *p_key)
{
	u32 crc = crc16(p_key->dsk,sizeof(p_key->dsk)+sizeof(p_key->dpk));
	if(crc == p_key->crc){
		return 1;
	}else{
		return 0;
	}
}

void erase_ecdh_sector_restore_info(u32 adr,u8 *p_data,u16 len)
{
	u32 sec_adr = (adr>>12);
	sec_adr = sec_adr << 12;// get the base sector adr
	u8 temp_buf[SECTOR_PAR_SIZE_MAX];
 	u16 offset = (adr - sec_adr);
	flash_read_page(sec_adr,SECTOR_PAR_SIZE_MAX,temp_buf);
	if(p_data == NULL || len == 0 || len>0x100){
		memset(temp_buf+offset,0xff,0x100);
	}else{
		memcpy(temp_buf+offset,p_data,len);
	}
	flash_erase_sector(sec_adr);
	flash_write_page(sec_adr,SECTOR_PAR_SIZE_MAX,temp_buf);
}

u8 get_ecdh_key_sts(mesh_ecdh_key_str *p_key)
{
	u8 dsk_valid = get_flash_data_is_valid(p_key->dsk,sizeof(p_key->dsk));
	u8 dpk_valid = get_flash_data_is_valid(p_key->dpk,sizeof(p_key->dpk));
	if(dsk_valid && dpk_valid ){
		return ECDH_KEY_STS_TWO_VALID;
	}else if(!dsk_valid && !dpk_valid){
		return ECDH_KEY_STS_NONE_VALID;
	}else{
		return ECDH_KEY_STS_ONE_VALID;
	}
}

u8 cal_dsk_dpk_is_valid_or_not(mesh_ecdh_key_str *p_key)
{
	
	u8 dev_dsk[32]={ 0x52,0x9a,0xa0,0x67,0x0d,0x72,0xcd,0x64, 0x97,0x50,0x2e,0xd4,0x73,0x50,0x2b,0x03,
					 0x7e,0x88,0x03,0xb5,0xc6,0x08,0x29,0xa5, 0xa3,0xca,0xa2,0x19,0x50,0x55,0x30,0xba};
	u8 dev_dpk[64]={ 0xf4,0x65,0xe4,0x3f,0xf2,0x3d,0x3f,0x1b, 0x9d,0xc7,0xdf,0xc0,0x4d,0xa8,0x75,0x81,
					 0x84,0xdb,0xc9,0x66,0x20,0x47,0x96,0xec, 0xcf,0x0d,0x6c,0xf5,0xe1,0x65,0x00,0xcc,
					 0x02,0x01,0xd0,0x48,0xbc,0xbb,0xd8,0x99, 0xee,0xef,0xc4,0x24,0x16,0x4e,0x33,0xc2,
					 0x01,0xc2,0xb0,0x10,0xca,0x6b,0x4d,0x43, 0xa8,0xa1,0x55,0xca,0xd8,0xec,0xb2,0x79};
	u8 k0[32],k1[32];
	tn_p256_dhkey (k0, dev_dsk, p_key->dpk, p_key->dpk+0x20);
	tn_p256_dhkey (k1, p_key->dsk, dev_dpk, dev_dpk+0x20);
	if(!memcmp(k0,k1,sizeof(k0))){
		return 1;
	}else{
		return 0;
	}
}

void get_private_key(u8 *p_private_key)
{
	mesh_ecdh_key_str key_str;
	flash_read_page(FLASH_ADR_EDCH_PARA,sizeof(key_str),(u8*)&key_str);
	memcpy(p_private_key, key_str.dsk, 32);
}

void get_public_key(u8 *p_public_key)
{
	mesh_ecdh_key_str key_str;
	flash_read_page(FLASH_ADR_EDCH_PARA,sizeof(key_str),(u8*)&key_str);
	memcpy(p_public_key, key_str.dpk, 64);
}

void cal_private_and_public_key()
{
	// first need to change the format to key(32+64),flag,crc(use crc16) 
	// get all the para data first 
	mesh_ecdh_key_str key_str;
	flash_read_page(FLASH_ADR_EDCH_PARA,sizeof(key_str),(u8*)&key_str);
	if(key_str.valid == ECDH_KEY_VALID_FLAG){
		if(check_ecdh_crc(&key_str)){// crc is valid
			return ;
		}else{
			erase_ecdh_sector_restore_info(FLASH_ADR_EDCH_PARA,0,0);
			start_reboot();
		}
	}else if(key_str.valid == 0xffffffff){
		u8 key_sts  = get_ecdh_key_sts(&key_str);//certify
		if(key_sts == ECDH_KEY_STS_NONE_VALID){
			// create the key flag crc and valid 
			u8 irq_res = irq_disable();
			tn_p256_keypair_mac(key_str.dsk,key_str.dpk,key_str.dpk+32,tbl_mac,6);// create the key part 
			irq_restore(irq_res); 
			key_str.valid = ECDH_KEY_VALID_FLAG;
			key_str.crc = crc16(key_str.dsk,sizeof(key_str.dsk)+sizeof(key_str.dpk));
			flash_write_page(FLASH_ADR_EDCH_PARA,sizeof(key_str),(u8*)&key_str);

		}else if (key_sts == ECDH_KEY_STS_TWO_VALID){ //cetify
			// need to dispatch the key is valid or not ,use init the dsk and dpk to calculate is valid or not 
			if(cal_dsk_dpk_is_valid_or_not(&key_str)){// key is valid
				key_str.valid = ECDH_KEY_VALID_FLAG;
				key_str.crc = crc16(key_str.dsk,sizeof(key_str.dsk)+sizeof(key_str.dpk));
				flash_write_page(FLASH_ADR_EDCH_PARA+96,8,(u8*)&(key_str.valid));
			}else{
				erase_ecdh_sector_restore_info(FLASH_ADR_EDCH_PARA,0,0);
				start_reboot();
			}
		}else{// there is only one key in  the valid ,need to erase and reboot 
			erase_ecdh_sector_restore_info(FLASH_ADR_EDCH_PARA,0,0);
			start_reboot();
		}
	}else{ // the valid flag is unvalid
		erase_ecdh_sector_restore_info(FLASH_ADR_EDCH_PARA,0,0);
		start_reboot();
	}
	
}
#endif 

int mesh_get_proxy_hci_type()
{
	return PROXY_HCI_SEL;
}

void mesh_secure_beacon_loop_proc()
{
	static u32 beacon_100ms_cnt =0;
	if((++beacon_100ms_cnt > SEC_NW_BC_INV_DEF_100MS) && is_provision_success()){
   		beacon_100ms_cnt = 0;
   		mesh_tx_sec_nw_beacon_all_net(0);	
	}
}

void mesh_tid_save(int ele_idx)
{
#if __PROJECT_MESH_SWITCH__
	analog_write(REGA_TID, mesh_tid.tx[ele_idx]);
#endif
}

#if !WIN32
u8 adv_filter = 1;
u8 adv_mesh_en_flag = 0;
u8 mesh_kr_filter_flag =0;
u8 mesh_provisioner_buf_enable =0;
void enable_mesh_adv_filter()
{
	adv_mesh_en_flag = 1;
}

void disable_mesh_adv_filter()
{
	adv_mesh_en_flag = 0;
}

void enable_mesh_kr_cfg_filter()
{
	mesh_kr_filter_flag = 1;
}

void disable_mesh_kr_cfg_filter()
{
	mesh_kr_filter_flag = 0;
}

void  enable_mesh_provision_buf()
{
	mesh_provisioner_buf_enable =1;
}
void  disable_mesh_provision_buf()
{
	mesh_provisioner_buf_enable =0;
}

extern attribute_t* gAttributes;
u8 get_att_permission(u8 handle)
{
#if HOMEKIT_EN
	return gAttributes[handle].perm;
#else
	return *gAttributes[handle].p_perm;
#endif
}

#if HOMEKIT_EN
_attribute_ram_code_ void adv_homekit_filter(u8 *raw_pkt)
{
	extern u8 blt_pair_ok;
	extern u8 proc_homekit_pair;
	if((!blt_pair_ok) && (ble_state == BLE_STATE_BRX_S)){// homekit pair proc in irq
		rf_packet_att_write_t *p = (rf_packet_att_write_t*)raw_pkt;
		if(p->rf_len){
			u8 type = p->type&3;
			if(type && (type != LLID_CONTROL)){// l2cap pkt
				if(p->chanId == L2CAP_CID_ATTR_PROTOCOL){
					if( !(p->opcode & 0x01) || p->opcode == ATT_OP_EXCHANGE_MTU_RSP)		//att server handler
					{
						if((ATT_OP_WRITE_CMD == p->opcode) || ( ATT_OP_WRITE_REQ == p->opcode)	|| ( ATT_OP_EXECUTE_WRITE_REQ == p->opcode) \
						 || ( ATT_OP_READ_REQ == p->opcode) || (ATT_OP_READ_BLOB_REQ == p->opcode)){
							if(is_homekit_pair_handle(p->handle)){
								proc_homekit_pair = 1;
								static u32 A_homekit_pair=0;A_homekit_pair++;
							}
						}
						else{

						}
					}
				}
			}
		}
	}
}
#endif

#define USER_ADV_FILTER_EN		0
#if (USER_ADV_FILTER_EN)
_attribute_ram_code_ u8 user_adv_filter_proc(rf_packet_adv_t * p_adv)
{
	#if 0 // demo
	PB_GATT_ADV_DAT *p_pb_adv = (PB_GATT_ADV_DAT *)p_adv->data;
	u8 temp_uuid[2]=SIG_MESH_PROVISION_SERVICE;
	if(!memcmp(temp_uuid, p_pb_adv->uuid_pb_uuid, sizeof(temp_uuid))){
		static u32 A_pb_adv_cnt = 0;
		A_pb_adv_cnt++;
		return 1;
	}
	#endif
	return 0;
}
#endif

_attribute_ram_code_ u8 adv_filter_proc(u8 *raw_pkt ,u8 blt_sts)
{
#define BLE_RCV_FIFO_MAX_CNT 	6
	u8 next_buffer =1;
	#if ((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	rf_packet_adv_t * pAdv = (rf_packet_adv_t *) (raw_pkt + 0);
	#elif (MCU_CORE_TYPE == MCU_CORE_8269)
	rf_packet_adv_t * pAdv = (rf_packet_adv_t *) (raw_pkt + 8);
	#endif
	u8 adv_type = pAdv->header.type;
	if(!adv_filter){
		return next_buffer;
	}

	u8 fifo_free_cnt = blt_rxfifo.num - ((u8)(blt_rxfifo.wptr - blt_rxfifo.rptr)&(blt_rxfifo.num-1));
	if(blt_sts == BLS_LINK_STATE_CONN){
		if(ble_state == BLE_STATE_BRX_E){
			if(fifo_free_cnt < BLE_RCV_FIFO_MAX_CNT+2){
				return 0;
			}
			else if(adv_type != LL_TYPE_ADV_NONCONN_IND){
				next_buffer = 0;
				#if (USER_ADV_FILTER_EN)
				next_buffer = user_adv_filter_proc(pAdv);
				#endif
			}
		}
		#if(HOMEKIT_EN)
		else {
			adv_homekit_filter(raw_pkt);
		}
		#endif
	}else{
		if(adv_type != LL_TYPE_ADV_NONCONN_IND){
			next_buffer = 0;
			#if (USER_ADV_FILTER_EN)
			next_buffer = user_adv_filter_proc(pAdv);
			#endif
		}else{
			// add the filter part last 
			if(adv_mesh_en_flag){
				if( pAdv->data[1] != MESH_ADV_TYPE_PRO || (fifo_free_cnt < 4)){	
					// not need to reserve fifo for the ble part 
					return 0;
				}
			#if __PROJECT_MESH_PRO__
			}else if(mesh_kr_filter_flag){
				// keybind filter flag ,to improve the envirnment of the gateway part
				if( pAdv->data[1] != MESH_ADV_TYPE_MESSAGE || (fifo_free_cnt < 4)){
					return 0;
				}
			}else if (mesh_provisioner_buf_enable){
				if(fifo_free_cnt < 2){
					// special dispatch for the provision only 
					return 0;
				}
			#endif
			}else if (fifo_free_cnt < 4){
					// can not make the fifo overflow 
				return 0;
			}else{}
		}
	}
	return next_buffer;
}
#endif
void reliable_retry_cnt_def_set(u8 retry_cnt)
{
    g_reliable_retry_cnt_def = retry_cnt > RELIABLE_RETRY_CNT_MAX ? RELIABLE_RETRY_CNT_MAX : retry_cnt;
}

#if (!WIN32 && MD_BIND_WHITE_LIST_EN)
const u16 master_filter_list[]={
	SIG_MD_G_ONOFF_S,SIG_MD_G_LEVEL_S,SIG_MD_G_DEF_TRANSIT_TIME_S,SIG_MD_LIGHTNESS_S,
	SIG_MD_LIGHTNESS_SETUP_S,SIG_MD_LIGHT_CTL_S,SIG_MD_LIGHT_CTL_SETUP_S,SIG_MD_LIGHT_CTL_TEMP_S,
	SIG_MD_LIGHT_LC_S,SIG_MD_LIGHT_LC_SETUP_S,
    SIG_MD_LIGHT_HSL_S,SIG_MD_LIGHT_HSL_SETUP_S,SIG_MD_LIGHT_HSL_HUE_S,SIG_MD_LIGHT_HSL_SAT_S,
	SIG_MD_FW_UPDATE_S,SIG_MD_FW_UPDATE_C,SIG_MD_FW_DISTRIBUT_S,SIG_MD_FW_DISTRIBUT_C,SIG_MD_OBJ_TRANSFER_S,SIG_MD_OBJ_TRANSFER_C,
};
u8 model_need_key_bind_whitelist(u16 *key_bind_list_buf,u8 *p_list_cnt,u8 max_cnt)
{
	if(ARRAY_SIZE(master_filter_list) >= max_cnt){
		return 0;
	}
	memcpy((u8 *)key_bind_list_buf,(u8 *)master_filter_list,sizeof(master_filter_list));
	*p_list_cnt= ARRAY_SIZE(master_filter_list);
	return 1;
}
#endif

// SUBSCRIPTION SHARE
#if SUBSCRIPTION_SHARE_EN
	#if MI_API_ENABLE
typedef struct{
	u32 mode_id;
	u32 ele_idx;
}sub_share_mode_t;

#if (MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_CT_LIGHT)	
const sub_share_mode_t share_mode[5] = {
		{SIG_MD_G_ONOFF_S,0},{SIG_MD_LIGHTNESS_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},
		{MIOT_VENDOR_MD_SER,0},{SIG_MD_LIGHT_CTL_TEMP_S,1}
};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_LAMP)
const sub_share_mode_t share_mode[4] = {
		{SIG_MD_G_ONOFF_S,0},{SIG_MD_LIGHTNESS_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},
		{MIOT_VENDOR_MD_SER,0}
};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_ONE_ONOFF)
const sub_share_mode_t share_mode[3] = {
		{SIG_MD_G_ONOFF_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},{MIOT_VENDOR_MD_SER,0}
};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_TWO_ONOFF)
const sub_share_mode_t share_mode[4] = {
		{SIG_MD_G_ONOFF_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},{MIOT_VENDOR_MD_SER,0},
		{SIG_MD_G_ONOFF_S,1}	
};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_THREE_ONOFF)
const sub_share_mode_t share_mode[5] = {
		{SIG_MD_G_ONOFF_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},{MIOT_VENDOR_MD_SER,0},
		{SIG_MD_G_ONOFF_S,1},{SIG_MD_G_ONOFF_S,2}	
};
#elif(MI_PRODUCT_TYPE == MI_PRODUCT_TYPE_PLUG)
const sub_share_mode_t share_mode[3] = {
		{SIG_MD_G_ONOFF_S,0},{MIOT_SEPC_VENDOR_MODEL_SER,0},{MIOT_VENDOR_MD_SER,0}
};
#endif



u8 mi_is_sig_model(u32 model_id)
{
	if((model_id & 0xffff) == g_vendor_id){
		return 0;
	}else{
		return 1;
	}
}

void mi_share_mode_sub_proc(u8 ele_idx,u16 sub_adr_primary,u16 op,u8*uuid)
{
	foreach_arr(i,share_mode){
		const sub_share_mode_t *p_share = &(share_mode[i]);
		mesh_sub_search_and_set(op, ele_adr_primary+p_share->ele_idx, sub_adr_primary+p_share->ele_idx, 
								uuid, p_share->mode_id, mi_is_sig_model(p_share->mode_id));
	}
	return ;
}

u16 mi_share_model_sub(u16 op,u16 ele_adr,u16 sub_adr,u8 *uuid,u32 model_id)
{
	u8 ele_idx = ele_adr - ele_adr_primary;
	u16 sub_adr_primary = sub_adr - ele_idx;
	mi_share_mode_sub_proc(ele_idx,sub_adr_primary,op,uuid);
	return sub_adr_primary;
}
#endif

#if(AIS_ENABLE)
const u16 sub_share_model_sig[] = {
    SIG_MD_G_ONOFF_S, SIG_MD_G_LEVEL_S, SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_SETUP_S,
    SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_SETUP_S, SIG_MD_LIGHT_CTL_TEMP_S,
    SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_SETUP_S, SIG_MD_LIGHT_HSL_HUE_S, SIG_MD_LIGHT_HSL_SAT_S,
    SIG_MD_SCENE_S,
};

const u32 sub_share_model_vendor[] = {
    VENDOR_MD_LIGHT_S,
#if MD_VENDOR_2ND_EN
    VENDOR_MD_LIGHT_S2,
#endif
};

void share_model_sub(u16 op, u16 sub_adr, u8 *uuid)
{
	foreach(light_idx,LIGHT_CNT){
		u16 ele_adr = ele_adr_primary + (light_idx * ELE_CNT_EVERY_LIGHT);
			
		foreach_arr(i,sub_share_model_sig){
			mesh_sub_search_and_set(op, ele_adr, sub_adr, uuid, sub_share_model_sig[i], 1);
		}
			
		foreach_arr(i,sub_share_model_vendor){
			mesh_sub_search_and_set(op, ele_adr, sub_adr, uuid, sub_share_model_vendor[i], 0);
		}
			
  #if (ELE_CNT_EVERY_LIGHT >= 2)
		foreach(i,ELE_CNT_EVERY_LIGHT - 1){
			mesh_sub_search_and_set(op, ele_adr+i, sub_adr, uuid, SIG_MD_G_LEVEL_S, 1);
		}
   	#if (LIGHT_TYPE_CT_EN)
		mesh_sub_search_and_set(op, ele_adr+1, sub_adr, uuid, SIG_MD_LIGHT_CTL_TEMP_S, 1);
    #endif
    #if (LIGHT_TYPE_HSL_EN)
       	#if (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL)
		u32 offset = 1;
        #elif (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
		u32 offset = 2; // temp model first
        #endif
		mesh_sub_search_and_set(op, ele_adr+offset, sub_adr, uuid, SIG_MD_LIGHT_HSL_HUE_S, 1);
		mesh_sub_search_and_set(op, ele_adr+offset+1, sub_adr, uuid, SIG_MD_LIGHT_HSL_SAT_S, 1);
    #endif
  #endif
	}
}

#endif


#endif

u16 sub_adr_onoff =0;
#if MI_API_ENABLE
void mi_share_model_add_sub_adr(u8*uuid)
{
	foreach_arr(i,share_mode){
		const sub_share_mode_t *p_share = &(share_mode[i]);
		mesh_sub_search_and_set(CFG_MODEL_SUB_ADD, ele_adr_primary+p_share->ele_idx, MI_MESH_SUB_ADR, 
								uuid, p_share->mode_id, mi_is_sig_model(p_share->mode_id));
	}
	return ;
}

u8 mi_share_model_op_is_valid(u16 op)
{
	#if MI_SWITCH_LPN_EN
	if(((CFG_MODEL_SUB_OVER_WRITE == op)||
		(CFG_MODEL_SUB_VIRTUAL_ADR_OVER_WRITE == op))||
    	(CFG_MODEL_SUB_DEL_ALL == op) ||
    	(CFG_MODEL_SUB_DEL == op) ){
		return 1;
	}
	#else
	if(((CFG_MODEL_SUB_OVER_WRITE == op)||
		(CFG_MODEL_SUB_VIRTUAL_ADR_OVER_WRITE == op))||
    	(CFG_MODEL_SUB_DEL_ALL == op)){
		return 1;
	}
	#endif
	else{
		return 0;
	}
}
#endif
void share_model_sub_by_rx_cmd(u16 op, u16 ele_adr, u16 sub_adr, u16 dst_adr,u8 *uuid, u32 model_id, int sig_model)
{
#if SUBSCRIPTION_SHARE_EN
	#if (VENDOR_MD_MI_EN)
	// use model_id to get the idx and the model type 
	#if DUAL_VENDOR_EN
	if(DUAL_VENDOR_ST_MI == provision_mag.dual_vendor_st)
	#endif
	{
    	#if MI_SWITCH_LPN_EN
    	if( op == CFG_MODEL_SUB_OVER_WRITE || 
    		op == CFG_MODEL_SUB_DEL)// special proc for the overwrite and rsp the vendor part 
    	#else
    	if(op == CFG_MODEL_SUB_OVER_WRITE)// special proc for the overwrite and rsp the vendor part 
    	#endif
    	{
    		sub_adr_onoff = mi_share_model_sub(op, ele_adr,sub_adr, uuid,model_id);
    	}	
    	if(mi_share_model_op_is_valid(op)){
    		mi_share_model_add_sub_adr(uuid);
    	}
	}
    #endif

	#if (VENDOR_MD_NORMAL_EN)
    #if DUAL_VENDOR_EN
	if(DUAL_VENDOR_ST_ALI == provision_mag.dual_vendor_st)
    #endif
    {
    	if(ele_adr == ele_adr_primary){
            if(sig_model){
                if(model_id == SIG_MD_G_ONOFF_S){
                    share_model_sub(op, sub_adr, uuid);
                }
    		}
        }
    }
	#endif
       
#endif
}

typedef struct{
	u8 id;
	u16 group_id;
}mi_vendor_sts_str;

int mesh_cmd_sig_cfg_model_sub_cb(u8 st,mesh_cfg_model_sub_set_t * p_sub_set,int sig_model,u16 adr_src)
{
#if MI_API_ENABLE
	if(sub_adr_onoff){
		// need to rsp the status 
		mi_vendor_sts_str mi_sts;
		mi_sts.id =MI_SIMPLE_ROW_RSP;
		mi_sts.group_id = sub_adr_onoff;
		int i=3;
		while(i--){
			mesh_tx_cmd2normal_primary(VD_MI_NODE_GW, (u8 *)&mi_sts, sizeof(mi_sts),adr_src,0);
		}
		sub_adr_onoff =0;
		#if MI_SWITCH_LPN_EN // need to add random delay part 
		mesh_rsp_random_delay_step = 120 + (rand() %10);    // random delay between 1200~1300ms
		mesh_rsp_random_delay_tick = clock_time();
		mesh_need_random_delay = 1;
		#endif
		return 1;
	}else{
		return mesh_rsp_sub_status(st, p_sub_set, sig_model, adr_src);
	}
#else
	return mesh_rsp_sub_status(st, p_sub_set, sig_model, adr_src);
#endif
}

void mesh_node_prov_event_callback(u8 evt_code)
{
#if WIN32
#else
    if( evt_code == EVENT_MESH_NODE_RC_LINK_START ||
        evt_code == EVENT_MESH_PRO_RC_LINK_START ){
        if(blt_state == BLS_LINK_STATE_CONN){
            blc_ll_setScanEnable (0, 0);
			blc_att_setServerDataPendingTime_upon_ClientCmd(1);
			set_prov_timeout_tick(clock_time()|1); 
        }else{
             // disable the mesh provision filter part 
		    enable_mesh_adv_filter();
		    set_prov_timeout_tick(clock_time()|1); 
        }
		#if SPIRIT_PRIVATE_LPN_EN
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
		#endif
    }else{
		blc_ll_setScanEnable (BLS_FLAG_SCAN_ENABLE | BLS_FLAG_ADV_IN_SLAVE_MODE, 0);
		blc_att_setServerDataPendingTime_upon_ClientCmd(10);
		set_prov_timeout_tick(0);
		disable_mesh_adv_filter();
		mesh_provision_para_reset();
		if( evt_code == EVENT_MESH_NODE_RC_LINK_TIMEOUT ||
	        evt_code == EVENT_MESH_NODE_RC_CONFIRM_FAILED||
	        evt_code == EVENT_MESH_NODE_DISCONNECT||
	        evt_code == EVENT_MESH_NODE_RC_LINK_SUC||
	        evt_code == EVENT_MESH_NODE_CONNECT){
#if (AIS_ENABLE)
			if(evt_code == EVENT_MESH_NODE_RC_LINK_SUC){
				mesh_scan_rsp_init(); // set prov_flag				
			}
#endif
	    }
    }

	#if (AIS_ENABLE)
	beacon_send.start_time_s = system_time_s;
	#endif
#endif 
}


#if !WIN32
//----------------------- OTA --------------------------------------------
u8 	ui_ota_is_working = 0;
u8 ota_reboot_type = 0;
u32 ota_reboot_later_tick = 0;

void entry_ota_mode(void)
{
	ui_ota_is_working = 1;
	blc_ll_setScanEnable (0, 0);
	bls_ota_setTimeout(OTA_CMD_INTER_TIMEOUT_S * 1000000); //set OTA timeout
	#if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
	dual_mode_disable();
	// bls_ota_clearNewFwDataArea(); // may disconnect
	#endif
}

u8 ota_condition_enable()
{
	return pair_login_ok; // 1;
}

void set_firmware_type(u32 sdk_type)
{
    u32 mesh_type = 0;
    flash_read_page(FLASH_ADR_MESH_TYPE_FLAG, sizeof(mesh_type), (u8 *)&mesh_type);
	if(mesh_type != 0xffffffff){
		flash_erase_sector(FLASH_ADR_MESH_TYPE_FLAG);
	}
	mesh_type = sdk_type;
	flash_write_page (FLASH_ADR_MESH_TYPE_FLAG, 4, (u8 *)&mesh_type);
}

void set_firmware_type_SIG_mesh()
{
    set_firmware_type(TYPE_SIG_MESH);
}

#if FW_START_BY_BOOTLOADER_EN
void set_firmware_type_zb_with_factory_reset()
{
    set_firmware_type(TYPE_DUAL_MODE_ZIGBEE_RESET);
}
#endif

#if DUAL_MODE_WITH_TLK_MESH_EN
void set_firmware_type_recover()
{
    set_firmware_type(TYPE_DUAL_MODE_RECOVER);
}
#endif

void set_firmware_type_init()
{
    flash_erase_sector(FLASH_ADR_MESH_TYPE_FLAG);
}

u8 proc_telink_mesh_to_sig_mesh(void)
{
    #if (DUAL_VENDOR_EN)
    return 1;
    #endif
    
	u32 mesh_type = *(u32 *) FLASH_ADR_MESH_TYPE_FLAG;

	#if DUAL_MODE_ADAPT_EN
    LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"sdk type 0x%x:0x%x", FLASH_ADR_MESH_TYPE_FLAG, mesh_type);
	if(TYPE_DUAL_MODE_STANDBY == mesh_type){
		return 0;
	}if(0xffffffff == mesh_type){
        set_firmware_type(TYPE_DUAL_MODE_STANDBY);
        LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"sdk type: Factory status", 0);
		return 0;
	}else if(TYPE_DUAL_MODE_RECOVER == mesh_type){
        factory_reset();
        set_firmware_type(TYPE_DUAL_MODE_STANDBY);
        LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"sdk type: Recover from Zigbee", 0);
        return 0;
	}
	#endif

	if(TYPE_SIG_MESH == mesh_type){
		return 1;
	}
	
	u8 ret = 0;
	#if (0 == FLASH_1M_ENABLE)
	u32 sig_cust_cap_flag = 0;
	// 0x77004 is fixed address of telink mesh, not depend on flash size.
	flash_read_page(0x77004, 4, (u8 *)&sig_cust_cap_flag);	// reserve 3 byte for CUST_CAP_INFO
    #endif
    
	if((TYPE_TLK_BLE_SDK == mesh_type) || (TYPE_TLK_ZIGBEE == mesh_type)){
		ret = 1;
	}else if((TYPE_TLK_MESH == mesh_type)
	#if (0 == FLASH_1M_ENABLE)
	||(-1 != sig_cust_cap_flag)
	#endif
	){
	    #if (0 == FLASH_1M_ENABLE)
		flash_erase_sector(flash_sector_calibration);

		u8 flash_data = 0;
		flash_read_page(flash_sector_mac_address + 0x10, 1, &flash_data);
		flash_write_page(flash_sector_calibration + CALIB_OFFSET_CAP_INFO, 1, &flash_data);

        #if ((MCU_CORE_TYPE == MCU_CORE_8267)||(MCU_CORE_TYPE == MCU_CORE_8269))
		flash_read_page(flash_sector_mac_address + 0x11, 1, &flash_data);
		flash_write_page(flash_sector_calibration + CALIB_OFFSET_TP_INFO, 1, &flash_data);

		flash_read_page(flash_sector_mac_address + 0x12, 1, &flash_data);
		flash_write_page(flash_sector_calibration + CALIB_OFFSET_TP_INFO + 1, 1, &flash_data);
		// no RC32K_CAP_INFO
		#endif
		#endif
		
		ret = 1;
	}
	if(ret == 1){
		factory_reset();
	}

	#if DUAL_MODE_WITH_TLK_MESH_EN
	if(DUAL_MODE_SUPPORT_ENABLE == dual_mode_state){
	    set_firmware_type_recover();    // keep recover mode before provision ok
	}else
	#endif
	{
	    set_firmware_type_SIG_mesh();
	}
	
	return ret;
}

void mesh_ota_reboot_set(u8 type)
{
    ota_reboot_later_tick = clock_time() | 1;
    ota_reboot_type = type;
}

void mesh_ota_reboot_check_refresh()
{
    if(ota_reboot_later_tick){
        ota_reboot_later_tick = clock_time() | 1;
    }
}

void mesh_ota_reboot_proc()
{
    if(ota_reboot_later_tick && clock_time_exceed(ota_reboot_later_tick, 1500*1000)){
        irq_disable();
        if(OTA_SUCCESS_DEBUG == ota_reboot_type){
            while(1){show_ota_result(OTA_SUCCESS);}
        }else{
            if(OTA_SUCCESS == ota_reboot_type){
                ota_set_flag();
            }
            show_ota_result(ota_reboot_type);
            start_reboot();
        }
    }
}

//----------------------- ble connect callback --------------------------------------------
void mesh_ble_connect_cb(u8 e, u8 *p, int n)
{
	#if MESH_RX_TEST
	bls_l2cap_requestConnParamUpdate (8, 8, 0, 200);
	#else
	bls_l2cap_requestConnParamUpdate (16, 32, 0, 200);
	#endif
	
	#if ONLINE_STATUS_EN
    mesh_node_report_init ();
	#endif
	mesh_node_prov_event_callback(EVENT_MESH_NODE_CONNECT);

#if AIS_ENABLE
	ais_gatt_auth_init();
#endif	
#if HOMEKIT_EN
	task_connect(e, p, n);
#endif
}

void mesh_ble_disconnect_cb()
{
	app_adr = 0;
	pair_login_ok = 0;
    mesh_node_prov_event_callback(EVENT_MESH_NODE_DISCONNECT);
	#if MD_REMOTE_PROV
	mesh_rp_para_init();// avoid unexpected disconnect 
	#endif
	#if ONLINE_STATUS_EN
    mesh_report_status_enable(0);
	#endif
#if HOMEKIT_EN
	proc_homekit_pair = 0;
	ble_remote_terminate(0, 0, 0);
#endif
}

#if DEBUG_VC_FUNCTION
u8 send_vc_fifo(u8 cmd,u8 *pfifo,u8 cmd_len)
{
	if(cmd_len>0x50){
		return 0;
	}
	u8 head[2] = {0};
	head[0] = cmd;
	head[1] = cmd_len;
	my_fifo_push_hci_tx_fifo(pfifo, cmd_len, head, 2);
	return 1;
}
#endif

//u8		dev_mac[12] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff,  0xc4,0xe1,0xe2,0x63, 0xe4,0xc7};
//s8		dev_rssi_th = 0x7f;
void app_enable_scan_all_device ()
{
	//memset (dev_mac, 0, 12);
	//dev_rssi_th = 0x80;
	blc_ll_setScanEnable (BLS_FLAG_SCAN_ENABLE | BLS_FLAG_ADV_IN_SLAVE_MODE, 0);
}

int	app_device_mac_match (u8 *mac, u8 *mask)
{
	u8 m[6];
	for (int i=0; i<6; i++)
	{
		m[i] = mac[i] & mask[i];
	}
	return memcmp (m, mask + 6, 6) == 0;
}

// ------------set adv buffer--------
void set_random_adv_delay(int en)
{
	// random from 0--10ms
	u8 cnt = 0;
	if(en){
		cnt = (rand() & 0x0F) + 1;	// unit : 625us; CMD_RAND_DELAY_MAX_MS
	}
	bls_set_adv_delay(cnt);
}

void set_random_adv_delay_normal_adv(int en)
{
	// random from 0--10ms
	u8 cnt = 0;
	if(en){
		cnt = (rand() & 0x7F) + 1;	// unit : 625us; CMD_RAND_DELAY_MAX_MS
	}
	bls_set_adv_delay(cnt);
}

#if(__TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278)) 
void bls_ota_set_fwSize_and_fwBootAddr(int firmware_size_k, int boot_addr)
{
	ota_firmware_size_k = firmware_size_k;
	ota_program_bootAddr = boot_addr;
}
#endif

#if 0
void set_ota_firmwaresize(int adr)  // if needed, must be called before cpu_wakeup_init()
{
	int firmware_size_k =0;
	firmware_size_k = (adr>>12)<<2;
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	bls_ota_setFirmwareSizeAndOffset(firmware_size_k,0x40000);
	#elif((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	    #if(PINGPONG_OTA_DISABLE)
    bls_ota_set_fwSize_and_fwBootAddr(firmware_size_k,adr);
	    #else
	bls_ota_set_fwSize_and_fwBootAddr(firmware_size_k,0x40000);
	    #endif
	#endif
}
#endif

int mesh_adv_tx_test()
{
	static u8 send_test_cnt;
 	if(send_test_cnt){
		send_test_cnt--;
		unprov_beacon_send(MESH_UNPROVISION_BEACON_WITH_URI,0);
		/*
		set_pb_gatt_adv(p->data,5);
		p->header.type = LL_TYPE_ADV_IND;
		memcpy(p->advA,tbl_mac,6);
		memcpy(p->data, p->data, 29);
		p->rf_len = 6 + 29;
		p->dma_len = p->rf_len + 2;
		return 1 ;
		*/
	}
	
	return 0;
}

u8 gatt_adv_send_flag = 1;

int gatt_adv_prepare_handler(rf_packet_adv_t * p, int rand_flag)
{
    if(relay_adv_prepare_handler(p)){
        return 1;
    }

    int ret = 0;
#if __PROJECT_MESH_SWITCH__
    if(!rc_mag.adv_send_enable){
        return 0;
    }
#endif
    
    if(is_provision_working() || (!gatt_adv_send_flag)){
        return 0;
    }
    
    // dispatch gatt part 
#if   !__PROJECT_MESH_PRO__
    if((blt_state!=BLS_LINK_STATE_CONN)
    #if FEATURE_LOWPOWER_EN
    && ((!is_lpn_support_and_en))
    #endif
    ){
        static u32 gatt_adv_tick = 0;
        static u32 gatt_adv_inv_us = 0;// send adv for the first time
        static u32 gatt_adv_cnt = 0;
        if(clock_time_exceed(gatt_adv_tick, gatt_adv_inv_us)||MI_SWITCH_LPN_EN || SPIRIT_PRIVATE_LPN_EN){
			#if ((!MI_SWITCH_LPN_EN) && (!SPIRIT_PRIVATE_LPN_EN))
            if(gatt_adv_inv_us){
                if(rand_flag){
                    set_random_adv_delay(1);    // random 10~20ms
                }
                gatt_adv_inv_us = 0;
                return 0;   // TX in next loop
            }
			#endif
            gatt_adv_tick = clock_time();
			gatt_adv_cnt++;
			u32 adv_inter =0;
			#if MI_API_ENABLE
			if(is_provision_success()){
				adv_inter = ADV_INTERVAL_MS_PROVED;
			}else{
				adv_inter = ADV_INTERVAL_MS;
			}
			#else
				adv_inter = ADV_INTERVAL_MS;
			#endif
			
			#if MI_SWITCH_LPN_EN
			gatt_adv_inv_us = adv_inter; // use const 20ms mi_beacon interval 
			#else
            gatt_adv_inv_us = (adv_inter - 10 - 5 + ((rand() % 3)*10)) * 1000; // random (0~20 + 0~10)ms // -10: next loop delay, -5: margin for clock_time_exceed.
            #endif
            
            #if (MI_API_ENABLE && !AIS_ENABLE)
            set_adv_mi_prehandler(p);
            ret = 1;
            #else
            if(0){  // just for compile
            }
				#if HOMEKIT_EN
			else if(gatt_adv_cnt%ADV_SWITCH_MESH_TIMES){

				///////////////////// modify adv packet//////////////////////////////////////
				blt_set_normal_adv_pkt(HK_CATEGORY, DEVICE_SHORT_NAME);
				u16 hk_category = HK_CATEGORY;
				blt_adv_update_pkt(ADV_PKT_CATEGORY_FIELD, (const u8 *)&hk_category);
				extern u8 hk_setupHash[4];
				blt_adv_update_pkt(ADV_PKT_SETUP_HASH, hk_setupHash);
			
				task_adv_pre(p);
				ret=1;
			}
				#endif
				#if (DUAL_VENDOR_EN)
			else if((DUAL_VENDOR_ST_MI == provision_mag.dual_vendor_st)
			    || ((DUAL_VENDOR_ST_STANDBY == provision_mag.dual_vendor_st) && (gatt_adv_cnt%4))){ // 3/4 adv for mi
				set_adv_mi_prehandler(p);
            	ret = 1;
			}
				#endif
                #if PROVISION_GATT_ENABLE
            else if(provision_mag.gatt_mode == GATT_PROVISION_MODE){
                set_adv_provision(p);
                ret=1;
                // dispatch proxy part adv 
            }
                #endif 
                #if FEATURE_PROXY_EN
            else if (provision_mag.gatt_mode == GATT_PROXY_MODE){
                set_adv_proxy(p);
                ret=1;
            }
                #endif 
            else{
            }
            #endif
        }
    }
#endif 

    return ret;
}

int app_advertise_prepare_handler (rf_packet_adv_t * p)
{
    int ret = 0;			// default not send ADV
    static u8 mesh_tx_cmd_busy_cnt;
	static u32 adv_sn = 0;
	adv_sn++;

	#if (MESH_MONITOR_EN)
	if(monitor_mode_en){
		return 0;
	}
	#endif
	set_random_adv_delay(0);
	bls_set_adv_retry_cnt(0);
	
    #if FEATURE_LOWPOWER_EN && FEATURE_PROXY_EN
    if(send_gatt_adv_now_flag){
        if(provision_mag.gatt_mode == GATT_PROXY_MODE){
            set_adv_proxy(p);
            return 1;
        }else{
            return 0;
        }
    }
    #endif 
    
	#if 0
	if(mesh_adv_tx_test()){
	    return 1;
	}
	#endif
	
    my_fifo_t *p_fifo = 0;
    u8 *p_buf = 0;
	
    #if FEATURE_FRIEND_EN
    p_fifo = &mesh_adv_fifo_fn2lpn;
    p_buf = my_fifo_get(p_fifo);
    if(p_buf){
        mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)p_buf;
        mesh_transmit_t *p_trans_par = (mesh_transmit_t *)&p_bear->trans_par_val;
        ret = mesh_adv_cmd_set((u8 *)p, (u8 *)p_bear);
        bls_set_adv_retry_cnt(p_trans_par->count);
        my_fifo_pop(p_fifo);
        return ret;
    }
    #endif
    
    p_fifo = &mesh_adv_cmd_fifo;
    p_buf = 0;
    if(0 == mesh_tx_cmd_busy_cnt){
        p_buf = my_fifo_get(p_fifo);
        if(!p_buf
		#if	SPIRIT_PRIVATE_LPN_EN
		&& !mesh_sleep_time.soft_timer_send_flag
		#endif
		){
            if(gatt_adv_prepare_handler(p, 1)){
                return 1;
            }
        }
    }else{
        mesh_tx_cmd_busy_cnt--;
        return gatt_adv_prepare_handler(p, 0);
    }

    if(p_buf){  // network layer + beacon
        mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)p_buf;
        mesh_transmit_t *p_trans_par = (mesh_transmit_t *)&p_bear->trans_par_val;

		if(p_bear->type & RSP_DELAY_FLAG){
			if(mesh_rsp_random_delay_step && !clock_time_exceed(mesh_rsp_random_delay_tick,mesh_rsp_random_delay_step*10000)){
				return 0;
			}else{
				mesh_rsp_random_delay_step =0;
			}
			p_bear->type &= ~RSP_DELAY_FLAG;
		}
        set_random_adv_delay(1);
		
		#if(MULTI_ADDR_FOR_SWITCH_EN)
		extern u8 multi_addr_key_flag;
		#endif
        int adv_retry_flag = (is_lpn_support_and_en)
                    #if(MULTI_ADDR_FOR_SWITCH_EN)
                    		|| (multi_addr_key_flag)
                    #endif
                            ;
                            
        if(!adv_retry_flag){
			#if (!MESH_RX_TEST)   // because is 10ms interval for TEST mode: mesh_conn_param_update_req()
        	if(BLS_LINK_STATE_CONN == blt_state ||MI_SWITCH_LPN_EN || SPIRIT_PRIVATE_LPN_EN){
        	    mesh_tx_cmd_busy_cnt = 0;
            }else
            #endif
            {
            	mesh_tx_cmd_busy_cnt = p_trans_par->invl_steps;
            }
        }

        ret = mesh_adv_cmd_set((u8 *)p, (u8 *)p_bear);

        #if __PROJECT_MESH_SWITCH__
		set_rc_flag_when_send_pkt(p_trans_par->count);
		#if(MULTI_ADDR_FOR_SWITCH_EN)
		if(multi_addr_key_flag){
			set_rc_flag_when_send_pkt(0);
		}
		#endif
		#endif
		                            
        if(adv_retry_flag){
            bls_set_adv_retry_cnt(p_trans_par->count);
            p_trans_par->count = 0;
            mesh_tx_cmd_busy_cnt = 0;   // no need
        }
        
        if(0 == p_trans_par->count){
            my_fifo_pop(p_fifo);
            if(p_fifo == &mesh_adv_cmd_fifo){   // only cmd fifo check segment,
                #if DEBUG_MESH_DONGLE_IN_VC_EN
                debug_mesh_report_one_pkt_completed();
                #else
                irq_ev_one_pkt_completed = 1;	// don't do much function in irq, because irq stack.
                #endif
            }
            
    		if(is_lpn_support_and_en){
            	mesh_lpn_sleep_set_ready();
                lpn_debug_alter_debug_pin(0);
            }
        }else{
			p_trans_par->count--;
        }
    }
	return ret;		//ready to send ADV packet
}

int app_l2cap_packet_receive (u16 handle, u8 * raw_pkt)
{
	#if((HCI_ACCESS != HCI_USE_NONE) && (! DEBUG_MESH_DONGLE_IN_VC_EN && !TESTCASE_FLAG_ENABLE))
	u8 *p = raw_pkt + 12;
	u8 conn = 0x40;
	blc_hci_send_data (conn | HCI_FLAG_ACL_BT_STD, p, p[1]);	//dump packet to txFIFO
	#endif

	blc_l2cap_packet_receive (handle, raw_pkt);

	return 0;
}

int chn_conn_update_dispatch(u8 *p)
{
#if 0
	static u8 A_mesh_dbg_idle[40];
	static u8 A_mesh_dgb_cnt=0;
	memcpy(A_mesh_dbg_idle,p,sizeof(A_mesh_dbg_idle));
	A_mesh_dgb_cnt++;
#endif
	return 1;
}

void sim_tx_cmd_node2node()
{
#if 1
	static u32 sim_tx_last_time = 0;
	if(clock_time_exceed(sim_tx_last_time, 3000*1000)){
		#if 0
		static u8 sim_key_code;
        if(0 == vd_cmd_key_report(ADR_ALL_NODES, sim_key_code)){
        	sim_tx_last_time = clock_time();
        	sim_key_code++;
        }
		#else
		static u8 sim_onoff;
        if(0 == access_cmd_onoff(0xffff, 1, sim_onoff, CMD_NO_ACK, 0)){
        	sim_tx_last_time = clock_time();
        	sim_onoff = !sim_onoff;
        }
        #endif
	}
#endif
}

void usb_id_init()
{
#if(MCU_CORE_TYPE == MCU_CORE_8269)
	REG_ADDR8(0x74) = 0x53;
#elif((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	REG_ADDR8(0x74) = 0x62;
#endif
	#if(MCU_CORE_TYPE == MCU_CORE_8269)
	#if MESH_MONITOR_EN
    REG_ADDR16(0x7e) = 0x08a1;			//mesh ID   0x08a1 means 0x08a0
	#else
		#if GATEWAY_ENABLE
		REG_ADDR16(0x7e) = 0x08d5;			//mesh ID   0x08d5 means 0x08d4
		#else
    	REG_ADDR16(0x7e) = 0x08d3;			//mesh ID   0x08d3 means 0x08d2
		#endif
    #endif
	#else
	#if MESH_MONITOR_EN
    REG_ADDR16(0x7e) = 0x08a0;			//mesh ID   0x08a1 means 0x08a0
	#else
		#if GATEWAY_ENABLE
		REG_ADDR16(0x7e) = 0x08d4;			//mesh ID   0x08d5 means 0x08d4
		#else
    	REG_ADDR16(0x7e) = 0x08d2;			//mesh ID   0x08d3 means 0x08d2
		#endif
	#endif
	#endif
	REG_ADDR8(0x74) = 0x00;
}

void ble_mac_init()
{
    if(user_mac_proc()){
        return ;
    }

	if(flash_sector_mac_address == 0){
		return;
	}

	u8 mac_read[8];
	flash_read_page(flash_sector_mac_address, 8, mac_read);

	u8 ff_six_byte[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if ( memcmp(mac_read, ff_six_byte, 6) ) {
		memcpy(tbl_mac, mac_read, 6);  //copy public address from flash
	}
	else{  //no public address on flash
		#if MI_API_ENABLE
        extern void set_mi_mac_address(unsigned char *p_mac);
        set_mi_mac_address(tbl_mac);
        #else
        u8 value_rand[5];
        generateRandomNum(5, value_rand);
		tbl_mac[0] = value_rand[0];
		tbl_mac[1] = value_rand[1];
		tbl_mac[2] = value_rand[2];

		#if(MCU_CORE_TYPE == MCU_CORE_8258)
			tbl_mac[3] = 0x38;             //company id: 0xA4C138
			tbl_mac[4] = 0xC1;
			tbl_mac[5] = 0xA4;
		#elif(MCU_CORE_TYPE == MCU_CORE_8278)
			tbl_mac[3] = 0xD1;             //company id: 0xC119D1
			tbl_mac[4] = 0x19;
			tbl_mac[5] = 0xC4;
		#endif
		#endif

		flash_write_page (flash_sector_mac_address, 6, tbl_mac);
	}
}

void mesh_scan_rsp_init()
{
	mesh_scan_rsp_t tbl_scanRsp={0};
	tbl_scanRsp.adr_primary = ele_adr_primary;
	memcpy(tbl_scanRsp.mac_adr, tbl_mac, sizeof(tbl_scanRsp.mac_adr));
	foreach(i,sizeof(tbl_scanRsp.rsv_user)){
		tbl_scanRsp.rsv_user[i] = 1 + i;
	}
	
	tbl_scanRsp.type = GAP_ADTYPE_MANUFACTURER_SPECIFIC;	// manufacture data
	tbl_scanRsp.len = sizeof(mesh_scan_rsp_t) - 1;
	u8 rsp_len = tbl_scanRsp.len+1;
	#if(AIS_ENABLE)
	rsp_len = ais_pri_data_set(&tbl_scanRsp.len);	
	#endif
	bls_ll_setScanRspData((u8 *)&tbl_scanRsp, rsp_len);
}
#else
void mesh_ota_reboot_set(u8 type){}
void mesh_ota_reboot_check_refresh(){}
#endif

u8 random_enable =1;
void set_random_enable(u8 en)
{
	random_enable = en;
	return;
}

#if MD_SERVER_EN
void publish_when_powerup()
{
#if SEND_STATUS_WHEN_POWER_ON
    user_power_on_proc();
#endif

    st_pub_list_t pub_list = {{0}};
    foreach_arr(i,pub_list.st){
        pub_list.st[i] = ST_G_LEVEL_SET_PUB_NOW;
    }
    mesh_publish_all_manual(&pub_list, SIG_MD_LIGHTNESS_S, 1);
	#if MI_API_ENABLE
	mesh_publish_all_manual(&pub_list, SIG_MD_G_ONOFF_S, 1);
	mesh_publish_all_manual(&pub_list, SIG_MD_LIGHT_CTL_TEMP_S, 1);
	#endif
}
#endif

void mesh_vd_init()
{
#if MD_SERVER_EN
	light_res_sw_load();
	light_pwm_init();
	#if !WIN32
	publish_powerup_random_ms = rand() % 1500;  // 0--1500ms
	#endif
	#if STEP_PUB_MODE_EN
	mi_pub_para_init();
	#endif
	STATIC_ASSERT(MESH_POWERUP_BASE_TIME >=200);
	publish_powerup_random_ms += MESH_POWERUP_BASE_TIME; // 200ms: base time.
#endif
#if FAST_PROVISION_ENABLE
	mesh_fast_prov_val_init();
#endif
}

#if WIN32
void APP_reset_vendor_id(u16 vd_id)
{
    g_vendor_id = vd_id;
    g_vendor_md_light_vc_s = (g_vendor_md_light_vc_s & 0xFFFF0000) | vd_id;
	#if MD_VENDOR_2ND_EN
    g_vendor_md_light_vc_s2 = (g_vendor_md_light_vc_s2 & 0xFFFF0000) | vd_id;
	#endif
    g_vendor_md_light_vc_c = (g_vendor_md_light_vc_c & 0xFFFF0000) | vd_id;
    APP_set_vd_id_mesh_save_map(vd_id);
    APP_set_vd_id_mesh_cmd_vd_func(vd_id);
}
#endif

#if (DUAL_VENDOR_EN)
void vendor_id_check_and_update() //confirm cps and vendor model
{
    if(DUAL_VENDOR_ST_ALI == provision_mag.dual_vendor_st){
        traversal_cps_reset_vendor_id(SHA256_BLE_MESH_PID);
        vendor_md_cb_pub_st_set2ali();
    }
}
#endif

/*
mesh_global_var_init(): run in mesh_init_all() and before read parameters in flash.
                                  it's used to set default vaule for compilation.
*/
void mesh_global_var_init()
{
    get_fw_id();    // must first
#if !WIN32
	blc_readFlashSize_autoConfigCustomFlashSector();
#endif
    mesh_factory_test_mode_en(FACTORY_TEST_MODE_ENABLE);
    user_mesh_cps_init();
    u8 dev_key_def[16] = DEVKEY_DEF;
    memcpy(mesh_key.dev_key, dev_key_def, sizeof(mesh_key.dev_key));
#if (FEATURE_LOWPOWER_EN)
	model_sig_cfg_s.sec_nw_beacon = NW_BEACON_NOT_BROADCASTING;
#else
	model_sig_cfg_s.sec_nw_beacon = SEC_NW_BC_BROADCAST_DEFAULT; // (NODE_CAN_SEND_ADV_FLAG) ? SEC_NW_BC_BROADCAST_DEFAULT : NW_BEACON_NOT_BROADCASTING;
#endif
	model_sig_cfg_s.ttl_def = TTL_DEFAULT;
	
#if DEBUG_PROXY_FRIEND_SHIP
	#if WIN32
	model_sig_cfg_s.frid = FEATURE_FRIEND_EN ? FRIEND_SUPPORT_ENABLE : FRIEND_NOT_SUPPORT;
	#else
	model_sig_cfg_s.frid = FRIEND_NOT_SUPPORT;
	#endif
	#if(SPIRIT_PRIVATE_LPN_EN)
	model_sig_cfg_s.frid = FRIEND_SUPPORT_DISABLE;
	#endif
#else
	#if (NODE_CAN_SEND_ADV_FLAG)
	model_sig_cfg_s.frid = FEATURE_FRIEND_EN ? FRIEND_SUPPORT_ENABLE : FRIEND_NOT_SUPPORT;
	#else
	model_sig_cfg_s.frid = FEATURE_FRIEND_EN ? FRIEND_SUPPORT_DISABLE : FRIEND_NOT_SUPPORT;
	#endif
#endif

	model_sig_cfg_s.gatt_proxy = FEATURE_PROXY_EN ? GATT_PROXY_SUPPORT_ENABLE : GATT_PROXY_NOT_SUPPORT;
	model_sig_cfg_s.node_identity_def = NODE_IDENTITY_SUBNET_SUPPORT_DISABLE;
	model_sig_cfg_s.nw_transmit.count = TRANSMIT_CNT_DEF;
	model_sig_cfg_s.nw_transmit.invl_steps = TRANSMIT_INVL_STEPS_DEF;
	#if 0   // TEST_CASE_NODE_CFG_CFGR_BV01_EN in pts7_3_1.exe
	model_sig_cfg_s.relay = RELAY_NOT_SUPPORT;
	#else
	model_sig_cfg_s.relay_retransmit.count = TRANSMIT_CNT_DEF_RELAY;
	model_sig_cfg_s.relay_retransmit.invl_steps = TRANSMIT_INVL_STEPS_DEF_RELAY;
	model_sig_cfg_s.relay = FEATURE_RELAY_EN ? RELAY_SUPPORT_ENABLE : RELAY_NOT_SUPPORT;
	#endif

#if DUAL_VENDOR_EN
	provision_mag.dual_vendor_st = DUAL_VENDOR_ST_STANDBY;
#endif
	mesh_global_var_init_fn_buf();
#if MD_SERVER_EN
	mesh_global_var_init_light_sw();
    #if (MD_SENSOR_EN)
	mesh_global_var_init_sensor_descrip();
    #endif
    #if (MD_PROPERTY_EN)
    mesh_property_global_init();
    #endif
    #if (MD_LIGHT_CONTROL_EN)
    light_LC_global_init();
    #endif
#endif
}
/****************************************************
struct of the unprovisioned beacon 
typedef struct {
	mesh_beacon_header header;
	u8 beacon_type;
	u8 device_uuid[16];
	u8 oob_info[2];
	u8 uri_hash[4];
}beacon_data_pk;
*******************************************************/
void set_unprov_beacon_para(u8 *p_uuid ,u8 *p_info)
{
	if(p_uuid!=0){
		memcpy(prov_para.device_uuid,p_uuid,16);
	}
	if(p_info !=0){
		memcpy(prov_para.oob_info,p_info,2);
	}

	#if 0   // please modify URI_DATA directly.
	if(uri_para !=0){
		memcpy(uri_dat,uri_para,uri_len);
	}
	#endif
}
/**************************************************
typedef struct {
	u8 flag_len;
	u8 flag_type;
	u8 flag_content;
	u8 uuid_len;
	u8 uuid_type;
	u8 uuid_pb_uuid[2];
	u8 service_len;
	u8 service_type;
	u8 service_uuid[2];
	u8 service_data[16];
	u8 oob_info[2];
}PB_GATT_ADV_DAT;
**************************************************/
void set_provision_adv_data(u8 *p_uuid,u8 *oob_info)
{
	if(p_uuid != 0){
		memcpy(prov_para.device_uuid,p_uuid,16);
	}
	if(oob_info!=0){
		memcpy(prov_para.oob_info,oob_info,2);
	}
	return ;
}
/************************************************
typedef struct {
	u8 flag_len;
	u8 flag_type;
	u8 flag_data;
	u8 uuid_len;
	u8 uuid_type;
	u8 uuid_data[2];
	u8 serv_len;
	u8 serv_type;
	u8 serv_uuid[2];
	u8 identify_type;
	u8 hash[8];
	u8 random[8];
}proxy_adv_node_identity;
*************************************************/
void set_proxy_adv_data(u8 *p_hash,u8 *p_random)
{
	if(p_hash != 0){
		memcpy(prov_para.hash,p_hash,8);
	}
	if(p_random !=0){
		memcpy(prov_para.random,p_random,8);
	}
	return ;
}

/* 
	tx cmd interface.
	don't modify this interfaces, library use internal.
	use nk_array_idx instead of nk_idx, because no need to calculate nk index for response.
*/
void set_material_tx_cmd(material_tx_cmd_t *p_mat, u16 op, u8 *par, u32 par_len,
							u16 adr_src, u16 adr_dst, u8 retry_cnt, int rsp_max, u8 *uuid, 
							u8 nk_array_idx, u8 ak_array_idx, model_common_t *pub_md)
{
	memset(p_mat, 0, sizeof(material_tx_cmd_t));
	p_mat->op = op;
	p_mat->p_ac = par;
	p_mat->len_ac = par_len;
	p_mat->adr_src = adr_src;
	p_mat->adr_dst = adr_dst;
	p_mat->retry_cnt = retry_cnt;
	p_mat->rsp_max = rsp_max;
	
	if((0 == nk_array_idx)&&(0 == ak_array_idx)){	// use the first valid key
		nk_array_idx = get_nk_arr_idx_first_valid();
		ak_array_idx = get_ak_arr_idx_first_valid(nk_array_idx);
	}
	p_mat->nk_array_idx = nk_array_idx;
	p_mat->ak_array_idx = ak_array_idx;
	p_mat->pub_md = pub_md;
	
	if(uuid){
		p_mat->uuid = uuid;
		if(is_virtual_adr(adr_dst)){
			// no need to recalculate
		}else{
			p_mat->adr_dst = cala_vir_adr_by_uuid(uuid);
		}
	}
}

int mesh_tx_cmd(material_tx_cmd_t *p)
{
	if(mesh_adr_check(p->adr_src, p->adr_dst)){
	    LOG_MSG_ERR(TL_LOG_MESH,0, 0 ,"src or dst is invalid",0);
		return -1;
	}

	#if RELIABLE_CMD_EN
	int reliable = is_reliable_cmd(p->op, p->op_rsp);
	
    if(reliable){	
		if(is_unicast_adr(p->adr_dst)){
			p->rsp_max = 1;
		}
		
        return mesh_tx_cmd_reliable(p);
    }else
    #endif
    {
        //if(mesh_tx_reliable.busy){
            //mesh_tx_reliable_finish();     // can send reliable and unreliable command at the same time.
        //}
        return mesh_tx_cmd_unreliable(p);
    }
}

int mesh_tx_cmd2normal(u16 op, u8 *par, u32 par_len, u16 adr_src, u16 adr_dst, int rsp_max)
{
	material_tx_cmd_t mat;
    u8 nk_array_idx = get_nk_arr_idx_first_valid();
    u8 ak_array_idx = get_ak_arr_idx_first_valid(nk_array_idx);
	set_material_tx_cmd(&mat, op, par, par_len, adr_src, adr_dst, g_reliable_retry_cnt_def, rsp_max, 0, nk_array_idx, ak_array_idx, 0);
	return mesh_tx_cmd(&mat);
}

int mesh_tx_cmd2normal_primary(u16 op, u8 *par, u32 par_len, u16 adr_dst, int rsp_max)
{
	return mesh_tx_cmd2normal(op, par, par_len, ele_adr_primary, adr_dst, rsp_max);
}

int mesh_tx_cmd2uuid(u16 op, u8 *par, u32 par_len, u16 adr_src, u16 adr_dst, int rsp_max, u8 *uuid)
{
	material_tx_cmd_t mat;
    u8 nk_array_idx = get_nk_arr_idx_first_valid();
    u8 ak_array_idx = get_ak_arr_idx_first_valid(nk_array_idx);
	set_material_tx_cmd(&mat, op, par, par_len, adr_src, adr_dst, g_reliable_retry_cnt_def, rsp_max, uuid, nk_array_idx, ak_array_idx, 0);
	return mesh_tx_cmd(&mat);
}

int SendOpParaDebug(u16 adr_dst, u8 rsp_max, u16 op, u8 *par, int len)
{
	#if WIN32
	return SendOpParaDebug_VC(adr_dst, rsp_max, op, par, len);
	#else
	LOG_MSG_LIB(TL_LOG_NODE_SDK,par, len,"send access cmd %04x code\r\n",op);
	return mesh_tx_cmd2normal_primary(op, par, len, adr_dst, rsp_max);
	#endif
}

int SendOpParaDebug_vendor(u16 adr_dst, u8 rsp_max, u16 op, u8 *par, int len, u8 rsp_op, u8 tid)
{
#if WIN32
	u8 par_tmp[MESH_CMD_ACCESS_LEN_MAX+2];
	par_tmp[0] = (u8)g_vendor_id;
	par_tmp[1] = g_vendor_id>>8;
	par_tmp[2] = rsp_op;
	par_tmp[3] = tid;
	memcpy(par_tmp+4, par, len);
	return SendOpParaDebug_VC(adr_dst, rsp_max, op, par_tmp, len+4);
#else
	LOG_MSG_LIB(TL_LOG_NODE_SDK,par, len,"send access cmd %04x code\r\n",op);
	return mesh_tx_cmd2normal_primary(op, par, len, adr_dst, rsp_max);
#endif
}

/********
    user can use mesh_tx_cmd() or mesh_tx_cmd2self_primary() to send command to self.
*/
int mesh_tx_cmd2self_primary(u8 *ac, int len_ac)    // ac: op code + par
{
    mesh_cmd_nw_t nw = {0};
    nw.src = ele_adr_primary;
    nw.dst = ele_adr_primary;
    return mesh_rc_data_layer_access(ac, len_ac, &nw);
}

int is_need_response_to_self(u16 adr_dst, u16 op)
{
#if (!MD_CLIENT_EN)
    if(is_own_ele(adr_dst)){
        if(0){   // is resp_op
            return 0;
        }
    }
#endif

    return 1;
}
//-------------------------------- end

void mesh_rsp_delay_set(u32 delay_step, u8 is_seg_ack)
{	
#if!WIN32
#if SPIRIT_PRIVATE_LPN_EN
	if(lpn_provision_ok){ 
		delay_step = MESH_RSP_BASE_DELAY_STEP + (rand() %10);
	}
#endif
	if(delay_step == 0 || (blt_state == BLS_LINK_STATE_CONN)){
		return;
	}
	mesh_need_random_delay = 1;
	mesh_rsp_random_delay_step = delay_step;
	mesh_rsp_random_delay_tick = clock_time();
#endif
}
/**
* when receive a message, this function would be called if op supported and address match.
* @params: pointer to access layer which exclude op code.
* @p_res->cb: callback function define in mesh_cmd_sig_func[] or mesh_cmd_vd_func[]
*/

/** @addtogroup Mesh_Common
  * @{
  */
  
/** @defgroup Mesh_Common
  * @brief Mesh Common Code.
  * @{
  */

/**
 * @brief  when received a message, this function would be called 
 *   if opcode supported and address matched.
 * @param  params: Pointer to message data (excluding Opcode).
 * @param  par_len: The length of the message data.
 * @param  cb_par: Some information about function callbacks.
 * @retval Whether the message was processed
 *   (0 Message processed or -1 Message not processed)
 */
int mesh_rc_data_layer_access_cb(u8 *params, int par_len, mesh_cb_fun_par_t *cb_par)
{
    LOG_MSG_LIB(TL_LOG_NODE_SDK,params, par_len,"rcv access layer,retransaction:%d,src:0x%x op:0x%04x, par:",cb_par->retransaction,cb_par->adr_src, cb_par->op);
    mesh_op_resource_t *p_res = (mesh_op_resource_t *)cb_par->res;
    if(!is_cfg_model(p_res->id, p_res->sig)){ // user should not handle config model op code
        #if (VENDOR_MD_NORMAL_EN)
            #if (VENDOR_OP_MODE_SEL == VENDOR_OP_MODE_DEFAULT)
        if(IS_VENDOR_OP(cb_par->op)){
            if((cb_par->op >= VD_OP_RESERVE_FOR_TELINK_START) && (cb_par->op <= VD_OP_RESERVE_FOR_TELINK_END)){
                return -1;
            }
        }
            #endif
        #endif
        
        #if (!WIN32 && !FEATURE_LOWPOWER_EN)
        if(0 == mesh_rsp_random_delay_step){
            if((blt_state == BLS_LINK_STATE_ADV) && (cb_par->op_rsp != STATUS_NONE)){
				u32 random_delay_step = 0;
				if(is_group_adr(cb_par->adr_dst)){
					#if MI_API_ENABLE
						#if MI_SWITCH_LPN_EN
					random_delay_step = 120 + (rand() %80);    // random delay between 1200~2000ms
						#else
					random_delay_step = 80 + (rand() %120);    // random delay between 800~2000ms
						#endif
					#else
					random_delay_step = MESH_RSP_BASE_DELAY_STEP + (rand() & 0x1f);    // unit : ADV_INTERVAL_MIN(10ms)
					#endif
				}else if (is_unicast_adr(cb_par->adr_dst)){
					#if MI_SWITCH_LPN_EN
					random_delay_step = 120 + (rand() %10);    // random delay between 1200~1300ms
					#endif
				}
				mesh_rsp_delay_set(random_delay_step, 0);
            }
        }
        #endif
        
        // TODO
    }

    int err = 0;
    /*! p_res->cb: callback function define in mesh_cmd_sig_func[] 
     or mesh_cmd_vd_func[] */
    if(p_res->cb){ // have been check in library, check again.
        p_res->cb(params, par_len, cb_par);   // use mesh_need_random_delay in this function in library.
    }
    mesh_need_random_delay = 0; // must be clear here 
    return err;
}

// mesh rsp handle cb 
int mesh_rsp_handle_cb(mesh_rc_rsp_t *p_rsp)
{
	#if GATEWAY_ENABLE
	u16 op_rsp = rf_link_get_op_by_ac(p_rsp->data);
	if(op_rsp != COMPOSITION_DATA_STATUS){
        gateway_model_cmd_rsp((u8 *)&(p_rsp->src),p_rsp->len);
	}
		#if MD_MESH_OTA_EN // VC_DISTRIBUTOR_UPDATE_CLIENT_EN
	u16 op = rf_link_get_op_by_ac(p_rsp->data);
	u32 size_op = SIZE_OF_OP(op);
    //u8 *par = p_rsp->data + size_op;
    //u16 par_len = GET_PAR_LEN_FROM_RSP(p_rsp->len, size_op);
	if(mesh_ota_master_rx(p_rsp, op, size_op)){
	    return 1;
	}
		#endif
	#endif 
	return 1;
}

// -------------------------------  uart module---------
int my_fifo_push_hci_tx_fifo (u8 *p, u16 n, u8 *head, u8 head_len)
{
#if (HCI_ACCESS != HCI_USE_NONE)
	int ret = my_fifo_push(&hci_tx_fifo, p, n, head, head_len);
	if (-1 == ret){
		LOG_MSG_INFO(TL_LOG_MESH,0, 0,"my_fifo_push_hci_tx_fifo:debug printf tx FIFO overflow %d",ret);
	}
	return ret;
#else
	return -1;
#endif
}

int hci_send_data_user (u8 *para, int n)
{
    if(n > (HCI_TX_FIFO_SIZE - 2 - 1)){ // 2: size of length,  1: size of type
        return -1;
    }
    
	u8 head[1] = {HCI_RSP_USER};
	return my_fifo_push_hci_tx_fifo(para, n, head, 1);
}

#if GATEWAY_ENABLE
void mesh_tx_reliable_stop_report(u16 op,u16 dst, u32 rsp_max, u32 rsp_cnt)
{
	if(op == NODE_RESET){
		del_vc_node_info_by_unicast(dst);  
	}
}

#endif

//------------ command from upper  -------------------------------------------------
#if (!IS_VC_PROJECT)
int app_hci_cmd_from_usb (void)	// be called in blt_sdk_main_loop()
{
	int ret = 0;
	static u8 buff[72];
	extern int usb_bulk_out_get_data (u8 *p);
	int n = usb_bulk_out_get_data (buff);
	if (n){
		ret = app_hci_cmd_from_usb_handle(buff, n);
	}
	return 0;
}
#endif

int app_hci_cmd_from_usb_handle (u8 *buff, int n) // for both usb and uart
{
	if(n <= 2){		// error len
		return 0;
	}

	static u32 app_uuu;
    app_uuu++;
#if TESTCASE_FLAG_ENABLE    
    mesh_rc_cmd_tscript(buff, n);//2bytes:0xa0 0xff 
#endif
    u16 type = buff[0] + (buff[1] << 8);
    u8 *hci_data = buff + HCI_CMD_LEN;
    u16 hci_data_len = n - HCI_CMD_LEN;
	
	if (HCI_CMD_BULK_CMD2MODEL == type)
	{
        mesh_bulk_cmd2model(hci_data, hci_data_len);
	}
	else if (HCI_CMD_BULK_CMD2DEBUG == type)
	{
        mesh_bulk_cmd((mesh_bulk_cmd_par_t *)hci_data, hci_data_len);
	}
	#if (!IS_VC_PROJECT)
	#if (PROXY_HCI_SEL == PROXY_HCI_USB)
	else if(HCI_CMD_ADV_PKT == type){
		mesh_nw_pdu_from_gatt_handle(hci_data);
	}
	#endif
	#if DEBUG_MESH_DONGLE_IN_VC_EN
	else if(HCI_CMD_ADV_DEBUG_MESH_DONGLE2BEAR == type){
		mesh_tx_cmd_add_packet(hci_data);
	}
	else if(HCI_CMD_ADV_DEBUG_MESH_DONGLE2GATT == type){
		if(mesh_is_proxy_ready()){	// is_proxy_support_and_en
			mesh_proxy_adv2gatt(hci_data, MESH_ADV_TYPE_MESSAGE);
		}
	}
	else if(HCI_CMD_ADV_DEBUG_MESH_LED == type){
		mesh_light_control_t *p = (mesh_light_control_t *)(hci_data);
		pwm_set_lum(p->id, p->val, p->pol);
	}
	#endif
	#endif
	else if (HCI_CMD_BULK_SET_PAR == type)
	{
		// set parameters of VC node
    	mesh_bulk_set_par(hci_data, hci_data_len);
	}
	else if (HCI_CMD_BULK_SET_PAR2USB == type)
	{
		#if IS_VC_PROJECT
		fifo_push_vc_cmd2dongle_usb(buff, n);
		#else
    	mesh_bulk_set_par2usb(hci_data, hci_data_len);
    	#endif
	}
	else if (HCI_CMD_PROVISION == type)
	{
		#if FEATURE_PROV_EN
		mesh_provision_cmd_process();   
	   	#endif
	}
	#if 0
	else if (buff[0] == 0xe0 && buff[1] == 0xff)			//set mac address filter
	{
		#if 0
		extern s8 dev_rssi_th;
		extern u8 dev_mac[];
		dev_rssi_th = buff[4];
		memcpy (dev_mac, buff + 5, 12);
		#endif
	}
	#endif
	else if (HCI_CMD_USER_DEFINE == type)	//user defined command
	{
		app_led_en (buff[4], buff[5]);
	}else if (HCI_CMD_GATEWAY_CMD == type){
		//gateway cmd
		#if IS_VC_PROJECT
		fifo_push_vc_cmd2dongle_usb(buff, n);
		#else
		#if GATEWAY_ENABLE
		// if the cmd is the node reset cmd ,need to proc the vc node info part
		mesh_bulk_cmd((mesh_bulk_cmd_par_t *)(hci_data), hci_data_len);
		#endif
		#endif
	}else if (HCI_CMD_GATEWAY_CTL == type){
		#if IS_VC_PROJECT
		fifo_push_vc_cmd2dongle_usb(buff, n);
		#else
		#if GATEWAY_ENABLE
    	gateway_cmd_from_host_ctl(hci_data, hci_data_len);
    	#endif
		#endif 
	}else if (HCI_CMD_GATEWAY_OTA == type){
		#if GATEWAY_ENABLE
		gateway_cmd_from_host_ota(hci_data, hci_data_len);
		#endif
	}else if (HCI_CMD_MESH_OTA == type){
		#if GATEWAY_ENABLE
		gateway_cmd_from_host_mesh_ota(hci_data, hci_data_len);
		#endif
	}else{
		#if (__PROJECT_8267_MASTER_KMA_DONGLE__)
		blc_hci_handler (buff, n);
		#endif
	}
	return 0;
}


#if (HCI_ACCESS==HCI_USE_UART)
#define UART_HW_HEAD_LEN    (4 + 2) //4:uart dma_len,  2: uart margin

#if  (!(ANDROID_APP_ENABLE || IOS_APP_ENABLE))
_attribute_aligned_(4)	
#endif
#if(HCI_LOG_FW_EN)
u8 uart_hw_tx_buf[160 + UART_HW_HEAD_LEN]; // not for user
#else
u8 uart_hw_tx_buf[HCI_TX_FIFO_SIZE_USABLE + UART_HW_HEAD_LEN]; // not for user;  2: sizeof(fifo.len)
#endif
const u8 UART_TX_LEN_MAX = (sizeof(uart_hw_tx_buf) - UART_HW_HEAD_LEN);

void uart_drv_init()
{
#if(MCU_CORE_TYPE == MCU_CORE_8258 || (MCU_CORE_TYPE == MCU_CORE_8278))
//note: dma addr must be set first before any other uart initialization! (confirmed by sihui)
	u8 *uart_rx_addr = hci_rx_fifo.p + (hci_rx_fifo.wptr & (hci_rx_fifo.num-1)) * hci_rx_fifo.size;
	uart_recbuff_init( uart_rx_addr, hci_rx_fifo.size, uart_hw_tx_buf);
	uart_gpio_set(UART_TX_PIN, UART_RX_PIN);

	uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

	//baud rate: 115200
	#if (CLOCK_SYS_CLOCK_HZ == 16000000)
		uart_init(9, 13, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
		uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
		uart_init(8, 30, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
		uart_init(15, 25, PARITY_NONE, STOP_BIT_ONE);
	#endif

	uart_dma_enable(1, 1); 	//uart data in hardware buffer moved by dma, so we need enable them first

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);   	//uart Rx/Tx dma irq enable
#else

	//todo:uart init here
	uart_io_init(UART_GPIO_SEL);
#if(CLOCK_SYS_CLOCK_HZ == 32000000)
	CLK32M_UART115200;
#elif(CLOCK_SYS_CLOCK_HZ == 16000000)
	CLK16M_UART115200;
#endif
	uart_BuffInit(hci_rx_fifo_b, hci_rx_fifo.size, uart_hw_tx_buf);
#endif
}

int blc_rx_from_uart (void)
{
	uart_ErrorCLR();
	
	uart_data_t* p = (uart_data_t *)my_fifo_get(&hci_rx_fifo);
	if(p){
		u32 rx_len = p->len & 0xff; //usually <= 255 so 1 byte should be sufficient
		if (rx_len)
		{
			#if 0 // Serial loop test
			#if 1
			if(0 == my_fifo_push_hci_tx_fifo(p->data, rx_len, 0, 0));
			#else
			if(uart_Send(p->data, rx_len))
			#endif
			{
				my_fifo_pop(&hci_rx_fifo);
			}
			#else
			app_hci_cmd_from_usb_handle(p->data, rx_len);
			my_fifo_pop(&hci_rx_fifo);
			#endif
		}
	}
	return 0;
}

int blc_hci_tx_to_uart ()
{
	my_fifo_buf_t *p = (my_fifo_buf_t *)my_fifo_get (&hci_tx_fifo);
	#if 1
	if (p && !uart_tx_is_busy ())
	{
		#if (1 == HCI_LOG_FW_EN)
	    if(p->data[0] == HCI_LOG){// printf part ,not send the hci log part 
			u8 uart_tx_buf[UART_TX_LEN_MAX + 8];
			u8 head_len = p->data[1];
			if(head_len > UART_TX_LEN_MAX){
			    return -1;
			}
			memcpy(uart_tx_buf, p->data,head_len);
			
			u8 data_len = p->len - head_len;
			u32 len_usable = UART_TX_LEN_MAX - (head_len - 2);
			u8 dump_len = printf_Bin2Text((char *)(uart_tx_buf+head_len), len_usable, (char *)(p->data+head_len), data_len);
			if (uart_Send(uart_tx_buf+2, (head_len-2)+dump_len))
			{
				my_fifo_pop (&hci_tx_fifo);
			}
		}else
		#endif
		{
			#if (GATEWAY_ENABLE&&(HCI_ACCESS == HCI_USE_UART ))
			if (uart_Send((u8 *)p, p->len+2))
			{
				my_fifo_pop (&hci_tx_fifo);
			}
			#else
			if (uart_Send(p->data, p->len))
			{
				my_fifo_pop (&hci_tx_fifo);
			}
			#endif
			
		}
	}
	#else	// only report user data
	if(p){
		u8 hci_type = p->data[0];
		if((hci_type >= HCI_RSP_USER_START) && (hci_type <= HCI_RSP_USER_END)){
			if (!uart_tx_is_busy ()){
				if (uart_Send(p->data+1, p->len-1)){
					my_fifo_pop (&hci_tx_fifo);
				}
			}
		}else{
			my_fifo_pop (&hci_tx_fifo);	// not report
		}
	}
	#endif
	
	return 0;
}
#endif



STATIC_ASSERT(sizeof(simple_flow_dev_uuid_t) <= 16);   // because sizeof dev uuid is 16
void set_dev_uuid_for_simple_flow( u8 *p_uuid)
{
    simple_flow_dev_uuid_t *p_dev_uuid = (simple_flow_dev_uuid_t *)p_uuid;
    memcpy(&p_dev_uuid->cps_head, &gp_page0->head, sizeof(p_dev_uuid->cps_head));
    memcpy(p_dev_uuid->mac, tbl_mac, sizeof(p_dev_uuid->mac));
    // set uuid
}
u8 prov_uuid_fastbind_mode(u8 *p_uuid)
{
    simple_flow_dev_uuid_t *p_dev_uuid = (simple_flow_dev_uuid_t *)p_uuid;
    if(p_dev_uuid->cps_head.cid == 0x0211){
        return 1;
    }else{
        return 0;
    }
}


void wd_clear_lib()
{
	#if (MODULE_WATCHDOG_ENABLE&&!WIN32)
	wd_clear();
	#endif
}

//-------------------------------- uart end
// add the test interface for the part of the proxy client test part 
// MESH/CL/PROXY/BV-01C receive complete msg to lower tester

// MESH/CL/PROXY/BV-02C receive segmented msg to lower tester 
// wait for the part of the terminate cmd when the proxy msg is not complete
// MESH/CL/PROXY/BV-03C send complete msg to lower tester

int mesh_send_cl_proxy_bv03(u16 node_adr)
{
	u8 key_add_tmp=0;
	return SendOpParaDebug(node_adr, 0, APPKEY_ADD, (u8 *)&key_add_tmp, 0);
}

//  MESH/CL/PROXY/BV-04C send segment msg to lower tester
int mesh_send_cl_proxy_bv04(u16 node_adr)
{
	u8 app_key_idx[3]={0x00,0x01,0xff};
	u8 app_key[16];
	memset(app_key,2,sizeof(app_key));
	return cfg_cmd_sig_model_app_set(node_adr,app_key_idx,app_key);
}

// MESH/CL/PROXY/BV-05C send filter proxy cfg msg 
int mesh_send_cl_proxy_bv05(u16 node_adr)
{
	return mesh_proxy_set_filter_type(FILTER_BLACK_LIST);
}

// MESH/CL/PROXY/BV-06C add address to filter config msg 
int mesh_send_cl_proxy_bv06(u16 node_adr)
{
	return mesh_proxy_filter_add_adr(0x0011);
}

// MESH/CL/PROXY/BV-07C remove address to filter config msg 
int mesh_send_cl_proxy_bv07(u16 node_adr)
{
	return mesh_proxy_filter_remove_adr(0x0011);
}

// MESH/CL/PROXY/BV-08C receive the secure beacon when connected 
// MESH/CL/PROXY/BV-09-C [Send Secure Network Beacon When IV Index Updated]

// MESH/CL/PROX/BI-01-C [Ignore Invalid Message Type] ,ignore the invalid msg 

/*************** log *************/
const char  TL_LOG_STRING[TL_LOG_LEVEL_MAX][MAX_LEVEL_STRING_CNT] = {
    {"[USER]:"},
    {"[LIB]:"},    // library and important log
    {"[ERR]:"},
    {"[WARN]:"},
    {"[INFO]:"},
    {"[DEBUG]:"},
};

const char tl_log_module_mesh[TL_LOG_MAX][MAX_MODULE_STRING_CNT] ={
	"(mesh)","(provision)","(lowpower)","(friend)",
	"(proxy)","(GattProv)","(log_win32)","(GATEWAY)",
	"(KEYBIND)","(sdk)","(Basic)","(RemotePro)","(cmd_rsp)",
	"(common)","(cmd_name)","(sdk_nw_ut)","(iv_update)","(gw_vc_log)","(USER)"
};

STATIC_ASSERT(TL_LOG_LEVEL_MAX < 8); // because only 3bit, please refer to LOG_GET_LEVEL_MODULE
STATIC_ASSERT(TL_LOG_MAX < 32); // because 32bit, and LOG_GET_LEVEL_MODULE

int tl_log_msg_valid(char *pstr,u32 len_max,u32 module)
{
	int ret =1;
	if (module >= TL_LOG_MAX){
		ret =0;
	}else if (!(BIT(module)& TL_LOG_SEL_VAL)){
		ret =0;
	}
	if(ret){
		#if WIN32
			#if VC_APP_ENABLE
			strcat_s(pstr,len_max,tl_log_module_mesh[module]);
			#else 
			strcat(pstr,tl_log_module_mesh[module]);
			#endif 
		#else
		    u32 str_len = strlen(pstr);
		    str_len = min(str_len, MAX_LEVEL_STRING_CNT);
		    if((str_len + MAX_MODULE_STRING_CNT) <= len_max ){
			    memcpy(pstr+str_len,tl_log_module_mesh[module],MAX_MODULE_STRING_CNT);
			}
		#endif
	}
	return ret;
}

void tl_log_msg(u32 level_module,u8 *pbuf,int len,char  *format,...)
{
#if (WIN32 || HCI_LOG_FW_EN)
	char tl_log_str[MAX_STRCAT_BUF] = {0};
	u32 module = LOG_GET_MODULE(level_module);
	u32 log_level = LOG_GET_LEVEL(level_module);
	
	// if((0 == log_level) || (log_level > TL_LOG_LEVEL_MAX)){
	//     return ;
	// }else{
    //     memcpy(tl_log_str,TL_LOG_STRING[log_level - 1],MAX_LEVEL_STRING_CNT);
	// }
	
	// if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str), module)){
	//     if(log_level != TL_LOG_LEVEL_ERROR){
	// 	    return ;
	// 	}
	// }
	
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
#endif
}

#if 0
void tl_log_msg_err(u16 module,u8 *pbuf,int len,char  *format,...)
{
#if (WIN32 || HCI_LOG_FW_EN)
	char tl_log_str[MAX_STRCAT_BUF] = TL_LOG_ERROR_STRING;
	if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str), module)){
		//return ;
	}
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
#endif
}

void tl_log_msg_warn(u16 module,u8 *pbuf,int len,char  *format,...)
{
#if (WIN32 || HCI_LOG_FW_EN)
	char tl_log_str[MAX_STRCAT_BUF] = TL_LOG_WARNING_STRING;
	if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str),module)){
		return ;
	}
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
#endif
}

void tl_log_msg_info(u16 module,u8 *pbuf,int len,char  *format,...)
{
#if (WIN32 || HCI_LOG_FW_EN)
	char tl_log_str[MAX_STRCAT_BUF] = TL_LOG_INFO_STRING;
	if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str),module)){
		return ;
	}
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
#endif
}

void user_log_info(u8 *pbuf,int len,char  *format,...)
{
    char tl_log_str[MAX_STRCAT_BUF] = TL_LOG_INFO_STRING;
	if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str),TL_LOG_USER)){
		return ;
	}
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
}

void tl_log_msg_dbg(u16 module,u8 *pbuf,int len,char  *format,...)
{
#if (WIN32 || HCI_LOG_FW_EN)
	char tl_log_str[MAX_STRCAT_BUF] = TL_LOG_DEBUG_STRING;
	if(!tl_log_msg_valid(tl_log_str,sizeof(tl_log_str),module)){
		return ;
	}
	va_list list;
	va_start( list, format );
	LogMsgModuleDlg_and_buf(pbuf,len,tl_log_str,format,list);	
#endif
}
#endif

#if !WIN32
#if HCI_LOG_FW_EN
_attribute_no_retention_bss_ char log_dst[256];// make sure enough RAM
int LogMsgModdule_uart_mode(u8 *pbuf,int len,char *log_str,char *format, va_list list)
{
    #if (GATEWAY_ENABLE)
	return 1;
	#endif
	#if (HCI_ACCESS == HCI_USE_UART)    
	char *p_buf;
	char **pp_buf;
	p_buf = log_dst+2;
	pp_buf = &(p_buf);

	memset(log_dst, 0, sizeof(log_dst));
	u32 head_len = print(pp_buf,log_str, 0)+print(pp_buf,format,list);   // log_dst[] is enough ram.
	head_len += 2;  // sizeof(log_dst[0]) + sizeof(log_dst[1]) 
	if(head_len > sizeof(log_dst)){
	    while(1){   // assert, RAM conflict, 
	        show_ota_result(OTA_DATA_CRC_ERR);
	    }
	}
	
	if(head_len > HCI_TX_FIFO_SIZE_USABLE){
		return 0;
	}

	log_dst[0] = HCI_LOG; // type
	log_dst[1] = head_len;
	u8 data_len_max = HCI_TX_FIFO_SIZE_USABLE - head_len;
	if(len > data_len_max){
		len = data_len_max;
	}
	memcpy(log_dst+head_len, pbuf, len);
	my_fifo_push_hci_tx_fifo((u8 *)log_dst, head_len+len, 0, 0);
	if(is_lpn_support_and_en){
		blc_hci_tx_to_uart ();
			#if PTS_TEST_EN
		while(uart_tx_is_busy ());
			#endif
	}
	#endif
    return 1;
}

int LogMsgModule_io_simu(u8 *pbuf,int len,char *log_str,char *format, va_list list)
{
	char *p_buf;
	char **pp_buf;
	p_buf = log_dst;
	pp_buf = &(p_buf);
	u32 head_len = print(pp_buf,log_str, 0)+print(pp_buf,format,list);   // log_dst[] is enough ram.
	if((head_len + get_len_Bin2Text(len))> sizeof(log_dst)){
        return 0;
	}
	u8 dump_len = printf_Bin2Text((char *)(log_dst+head_len), sizeof(log_dst) - head_len, (char *)(pbuf), len);
	uart_simu_send_bytes((u8 *)log_dst, head_len+dump_len);
	return 1;
}
#endif


int LogMsgModuleDlg_and_buf(u8 *pbuf,int len,char *log_str,char *format, va_list list)
{
    #if (0 == HCI_LOG_FW_EN)
    return 1;
    #else
        #if 1
    return LogMsgModule_io_simu(pbuf,len,log_str,format,list);
        #else
    return LogMsgModdule_uart_mode(pbuf,len,log_str,format,list);
        #endif
    #endif
}
#endif	


/**
  * @}
  */

/**
  * @}
  */


