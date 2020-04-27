/*
 * ble_ll_ota.c
 *
 *  Created on: 2015-7-20
 *      Author: Administrator
 */
#include "proj/tl_common.h"
#include "proj_lib/ble/ble_common.h"
#include "proj_lib/ble/trace.h"
#include "proj_lib/pm.h"
#include "drivers/8258/flash.h"
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh_ota.h"
#include "version.h"
#if AIS_ENABLE
#include "proj_lib/mesh_crypto/aes_cbc.h"
#include "user_ali.h"
#endif

_attribute_data_retention_ int ota_adr_index = -1;
_attribute_data_retention_ u32 blt_ota_start_tick;
_attribute_data_retention_ u32 blt_ota_timeout_us = 30000000;  //default 30 second
u32 blt_ota_finished_time = 0;
u8  blt_ota_finished_flag = 0;
u8  blt_ota_terminate_flag = 0;

_attribute_data_retention_ ota_service_t blcOta;


#if(__TL_LIB_8267__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8269))
	u32		ota_program_offset = 0x40000;
	u32 	ota_firmware_size_k = FW_SIZE_MAX_K;
#elif(__TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
    #if (FLASH_1M_ENABLE && PINGPONG_OTA_DISABLE)
	_attribute_data_retention_	int		ota_program_bootAddr = FLASH_ADR_UPDATE_NEW_FW; // it will be used in cpu_wakeup init, and set value for ota_program_offset_
    #else
	_attribute_data_retention_	int		ota_program_bootAddr = 0x40000;
    #endif
    
	_attribute_data_retention_	u32		ota_program_offset = 0;
	_attribute_data_retention_	u32 	ota_firmware_size_k = FW_SIZE_MAX_K;
#else  //8266
	u32		ota_program_offset = 0x40000;
	u32 	ota_firmware_size_k = FW_SIZE_MAX_K;
	u32		bls_ota_bootFlagAddr = 	0x40000;
#endif



_attribute_data_retention_	ota_startCb_t		otaStartCb = NULL;
_attribute_data_retention_	ota_versionCb_t 	otaVersionCb = NULL;
_attribute_data_retention_	ota_resIndicateCb_t otaResIndicateCb = NULL;



unsigned short crc16 (unsigned char *pD, int len)
{

    static unsigned short poly[2]={0, 0xa001};              //0x8005 <==> 0xa001
    unsigned short crc = 0xffff;
    //unsigned char ds;
    int i,j;

    for(j=len; j>0; j--)
    {
        unsigned char ds = *pD++;
        for(i=0; i<8; i++)
        {
            crc = (crc >> 1) ^ poly[(crc ^ ds ) & 1];
            ds = ds >> 1;
        }
    }

     return crc;
}


#if(MCU_CORE_TYPE == MCU_CORE_8269)
void bls_ota_setFirmwareSizeAndOffset(int firmware_size_k, u32 ota_offset)
{
	ota_firmware_size_k = firmware_size_k;

#if(__TL_LIB_8267__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8269))

#else
	ota_program_offset = ota_offset;
#endif
}
#endif

void bls_ota_setBootFlagAddr(u32 bootFlag_addr)
{
	bls_ota_bootFlagAddr = bootFlag_addr;
}


void bls_ota_registerStartCmdCb(ota_startCb_t cb)
{
	otaStartCb = cb;
}

void bls_ota_registerVersionReqCb(ota_versionCb_t cb)
{
	otaVersionCb = cb;
}

void bls_ota_registerResultIndicateCb(ota_resIndicateCb_t cb)
{
	otaResIndicateCb = cb;
}

void bls_ota_setTimeout(u32 timeout_us)
{
	blt_ota_timeout_us = timeout_us;
}

_attribute_ram_code_ void start_reboot(void)
{
#if(__TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	if(blt_miscParam.pad32k_en){
		analog_write(SYS_DEEP_ANA_REG, analog_read(SYS_DEEP_ANA_REG) & (~SYS_NEED_REINIT_EXT32K)); //clear
	}
#endif
	irq_disable ();
#if SLEEP_FUNCTION_DISABLE
	REG_ADDR8(0x6f) = 0x20;  //reboot
#else
	cpu_sleep_wakeup(1, PM_WAKEUP_TIMER, clock_time() + 1*1000*sys_tick_per_us);//reboot
#endif
	while (1);
}


void ota_set_flag()
{
#if(__TL_LIB_8267__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8267) || \
	__TL_LIB_8269__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8269))
	u32 flag = 0x4b;
	flash_write_page(ota_program_offset + 8, 1, (u8 *)&flag);		//Set FW flag
	flag = 0;
	flash_write_page((ota_program_offset ? 0 : 0x40000) + 8, 4, (u8 *)&flag);	//Invalid flag
#elif(__TL_LIB_8258__ || (MCU_CORE_TYPE && MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278)) //8258
	u32 flag = 0x4b;
	flash_write_page(ota_program_offset + 8, 1, (u8 *)&flag);		//Set FW flag
	#if (!PINGPONG_OTA_DISABLE)
	flag = 0;
	flash_write_page((ota_program_offset ? 0 : ota_program_bootAddr) + 8, 4, (u8 *)&flag);	//Invalid flag
	#endif
#endif
}
u8 fw_ota_value =0;
u8 get_fw_ota_value()
{
	return fw_ota_value;
}
void ota_save_data(u32 adr, u8 * data, u16 len){
#if 1 // (! PINGPONG_OTA_DISABLE)
	if (adr == 0)
	{
		fw_ota_value = data[8];
		data[8] = 0xff;					//FW flag invalid
	}
#endif

	flash_write_page(ota_program_offset + adr, len, data);
}
u8 ota_reboot_flag = 1;// initial it will reboot 
void set_ota_reboot_flag(u8 flag)
{
	ota_reboot_flag = flag;
	return ;
}

int otaWrite(void * p)
{
#if 0 // DUAL_VENDOR_EN	// comfirm later
    return 0;
#endif

	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	static u32 fw_check_val = 0;
	static u8 need_check_type = 0;//=1:crc val sum
	static u16 ota_pkt_total = 0;

	u8 err_flg = OTA_SUCCESS;

	if(!ota_condition_enable()){
		return 0;
	}
	
	u16 ota_adr =  req->dat[0] | (req->dat[1]<<8);
	u16 crc;
	if(ota_adr == CMD_OTA_FW_VERSION){
		//to be add
		//log_event(TR_T_ota_version);
		blt_ota_start_tick = clock_time()|1;  //mark time
		if(otaVersionCb){
			otaVersionCb();
		}
	}
	else if(ota_adr == CMD_OTA_START){
		ota_adr_index = -1;
		if(ota_reboot_flag){
			blcOta.ota_start_flag = 1;   //set flag
			blt_ota_start_tick = clock_time()|1;  //mark time
			if(otaStartCb){
				otaStartCb();
			}
		}
		
		//log_event(TR_T_ota_start);
	}
	else if(ota_adr == CMD_OTA_END){
		//log_event(TR_T_ota_end);

		u16 adrIndex_max	   = req->dat[2] | (req->dat[3]<<8);
		u16 adrIndex_max_check = req->dat[4] | (req->dat[5]<<8);


		//if no index_max check, set ota success directly, otherwise we check if any index_max match
		if( req->l2cap == 9 && (adrIndex_max ^ adrIndex_max_check) == 0xffff){  //index_max valid, we can check
			if(adrIndex_max != ota_adr_index){  //last one or more packets missed
				err_flg = OTA_DATA_UNCOMPLETE;
			}
		}
		if(ota_pkt_total != ota_adr_index + 1){
			err_flg = OTA_DATA_UNCOMPLETE;
		}

		blt_ota_finished_flag_set(err_flg);
	}
	else{
		//log_task_begin(TR_T_ota_data);
		if(ota_adr_index + 1 == ota_adr){   //correct OTA data index
			blt_ota_start_tick = clock_time()|1;  //mark time
			crc = (req->dat[19]<<8) | req->dat[18];
			if(crc == crc16(req->dat, 18)){
				if(ota_adr == 0){
				    if(req->dat[8] == 0x5D){
				    	need_check_type = req->dat[9] ;
				    }
				    if(need_check_type == 1){
				    	fw_check_val = (req->dat[18] | req->dat[19]<<8);
				    }
				}else if(ota_adr == 1){
					ota_pkt_total = (((req->dat[10]) |( (req->dat[11] << 8) & 0xFF00) | ((req->dat[12] << 16) & 0xFF0000) | ((req->dat[13] << 24) & 0xFF000000)) + 15)/16;
					if(ota_pkt_total < 3){
						// invalid fw
						err_flg = OTA_ERR_STS;
					}else if(need_check_type == 1){
						fw_check_val += (req->dat[18] | req->dat[19]<<8);
					}
				}else if(ota_adr < ota_pkt_total - 1 && need_check_type == 1){
					fw_check_val += (req->dat[18] | req->dat[19]<<8);
				}else if(ota_adr == ota_pkt_total - 1 && need_check_type == 1){
					if(fw_check_val != ((req->dat[2]) |( (req->dat[3] << 8) & 0xFF00) | ((req->dat[4] << 16) & 0xFF0000) | ((req->dat[5] << 24) & 0xFF000000)) ){
						err_flg = OTA_ERR_STS;
					}
				}
				//log_data(TR_24_ota_adr,ota_adr);
				if((ota_adr<<4)>=(ota_firmware_size_k<<10)){
					err_flg = OTA_OVERFLOW;
				}else{
					ota_save_data (ota_adr<<4, req->dat + 2, 16);

					u8 flash_check[16];

					flash_read_page(ota_program_offset + (ota_adr<<4),16,flash_check);

					if(!memcmp(flash_check,req->dat + 2,16)){  //OK
						ota_adr_index = ota_adr;
					}
					else{ //flash write err
						err_flg = OTA_WRITE_FLASH_ERR;
					}
				}
				
			}
			else{  //crc err
				err_flg = OTA_DATA_CRC_ERR;
			}
		}
		else if(ota_adr_index >= ota_adr){  //maybe repeated OTA data, we neglect it, do not consider it ERR

		}
		else{  //adr index err, missing at least one OTA data
			err_flg = OTA_PACKET_LOSS;
		}
		//log_task_end(TR_T_ota_data);

	}

	if(err_flg){
		//log_event(TR_T_ota_err);
		blt_ota_finished_flag_set(err_flg);
	}

	return 0;
}

int otaRead(void * p)
{
	return 0;
}


void bls_ota_clearNewFwDataArea(void)
{
#if 1
		u32 tmp1 = 0;
		u32 tmp2 = 0;
		int cur_flash_setor;
		for(u32 i = 0; i < (ota_firmware_size_k>>2); ++i)
		{
			cur_flash_setor = ota_program_offset + i*0x1000;
			flash_read_page(cur_flash_setor, 		4, (u8 *)&tmp1);
			flash_read_page(cur_flash_setor + 2048, 4, (u8 *)&tmp2);

			if(tmp1 != ONES_32 || tmp2 != ONES_32)
			{
				flash_erase_sector(cur_flash_setor);
			}
		}

#else
		u32 tmp1 = 0;
		u32 tmp2 = 0;
		u32 tmp3 = 0;
		flash_read_page(ota_program_offset, 4, (u8 *)&tmp1);
		flash_read_page(ota_program_offset + 4092, 4, (u8 *)&tmp2);
		if(tmp1 != ONES_32 || tmp2 != ONES_32)
		{
			for(u32 i = (ota_firmware_size_k - 1)>>2; i>=0; --i)  //erase from end to head
			{
				flash_read_page(ota_program_offset + i*0x1000, 4, (u8 *)&tmp1);
				if(tmp1 == ONES_32){
					flash_read_page(ota_program_offset + i*0x1000 + 16, 4, (u8 *)&tmp2);
					if(tmp2 == ONES_32){
						flash_read_page(ota_program_offset + i*0x1000 + 4092, 4, (u8 *)&tmp3);
					}
				}

				if(tmp1 != ONES_32 || tmp2 != ONES_32 || tmp3 != ONES_32)
				{
					flash_erase_sector(ota_program_offset+i*0x1000);
				}
			}
		}
#endif
}


void bls_ota_procTimeout(void)
{
	blt_slave_ota_finish_handle();

	if(clock_time_exceed(blt_ota_start_tick , blt_ota_timeout_us)){  //OTA timeout
		rf_link_slave_ota_finish_led_and_reboot(OTA_TIMEOUT);
	}
}

void blt_ota_finished_flag_set(u8 reset_flag)
{
	if(blt_ota_finished_time == 0){
		blt_ota_finished_flag = reset_flag;
		blt_ota_finished_time = clock_time()|1;
	}
}

void rf_link_slave_ota_finish_led_and_reboot(u8 st)
{
	if(OTA_SUCCESS == st){
        ota_set_flag ();
    }
    else{
       if(ota_adr_index>=0){
			irq_disable();

			//for(int i=0;i<=ota_adr_index;i+=256)
			for(int i=(ota_adr_index&0x3ff00); i>=0; i-=256) //erase from end to head
			{  //4K/16 = 256
				flash_erase_sector(ota_program_offset + (i<<4));
			}
		}
    }
	
#if KEEP_ONOFF_STATE_AFTER_OTA 
	analog_write(DEEP_ANA_REG0, analog_read(DEEP_ANA_REG0) | BIT(OTA_REBOOT_FLAG));
#endif
	if(otaResIndicateCb){
		otaResIndicateCb(st);   // should be better at last.
	}
    irq_disable ();
    start_reboot();
}

void blt_slave_ota_finish_handle()		
{	
    if(blt_ota_finished_time){
        static u8 terminate_cnt;
        u8 reboot_flag = 0;
        if((0 == terminate_cnt) && (blt_ota_terminate_flag)){
               terminate_cnt = 6;
               bls_ll_terminateConnection(0x13);
        }
        
        if(terminate_cnt){
            terminate_cnt--;
            if(!terminate_cnt){
                reboot_flag = 1;
            }
        }
        
        if((!blt_ota_terminate_flag)
         &&((u32)(clock_time() - blt_ota_finished_time) > 2000*1000 * sys_tick_per_us)){
            blt_ota_terminate_flag = 1;    // for ios: no last read command
        }
        
        if(((u32)(clock_time() - blt_ota_finished_time) > 4000*1000 * sys_tick_per_us)){
            reboot_flag = 1;
        }
        
        if(reboot_flag){
            rf_link_slave_ota_finish_led_and_reboot(blt_ota_finished_flag);
            // have been reboot
        }
    }
}

#if(AIS_ENABLE)
#define MAIN_VERSION		0
#define SUB_VERSION			0
#define MODIFY_VERSION		0

const ais_fw_info_t  ais_fw_info = { 
	.device_type = (u8)MESH_PID_SEL, // device type
    .fw_version = (MAIN_VERSION<<16) | (SUB_VERSION<<8) | MODIFY_VERSION, 
};

int ais_ota_version_get()
{
	ais_msg_t ais_version;
	memset(&ais_version, 0x00, sizeof(ais_version));
	ais_version.msg_type = AIS_FW_VERSION_RSP;
	ais_version.length = ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_fw_info);
	memcpy(ais_version.data, &ais_fw_info, sizeof(ais_fw_info));
	if(ais_gatt_auth.auth_ok){
		ais_version.enc_flag = 1;
		AES128_pkcs7_padding(ais_version.data, sizeof(ais_fw_info), ais_version.data);
		aes_cbc_encrypt(ais_version.data, sizeof(ais_fw_info), &ctx, ais_gatt_auth.ble_key, iv);
	}
	return bls_att_pushNotifyData(AIS_NOTIFY_HANDLE, (u8 *)&ais_version, OFFSETOF(ais_msg_t,data)+(ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_fw_info)));
}

int ais_ota_req(u8 *p)
{
	ais_msg_t ais_msg_rsp;
	//ais_ota_req_t *ota_req_p = (ais_ota_req_t *)p;
	memset(&ais_msg_rsp, 0x00, sizeof(ais_msg_rsp));		

	ais_msg_rsp.msg_type = AIS_OTA_RSP;
	ais_msg_rsp.length = ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_ota_rsp_t);
	ais_msg_rsp.ais_ota_rsp.one_round_pkts = 0;//must set to 0 now.
#if 0 // set 0 always allow ota.
	if((ota_req_p->device_type == ais_fw_info.device_type) && (ota_req_p->fw_version > ais_fw_info.fw_version)&&(ota_req_p->ota_flag == 0))
#endif
	{
		ais_msg_rsp.ais_ota_rsp.allow_ota = ais_gatt_auth.auth_ok;

		ota_adr_index = -1;
		blcOta.ota_start_flag = ais_gatt_auth.auth_ok;   //set flag
		blt_ota_start_tick = clock_time()|1;  //mark time
		if(otaStartCb && ais_gatt_auth.auth_ok){
			otaStartCb();
		}
	}
	
	if(ais_gatt_auth.auth_ok){	
		ais_msg_rsp.enc_flag = 1;
		AES128_pkcs7_padding(ais_msg_rsp.data, sizeof(ais_ota_rsp_t), ais_msg_rsp.data);
		aes_cbc_encrypt(ais_msg_rsp.data, sizeof(ais_ota_rsp_t), &ctx, ais_gatt_auth.ble_key, iv);
	}
	
	return bls_att_pushNotifyData(AIS_NOTIFY_HANDLE, (u8 *)&ais_msg_rsp, OFFSETOF(ais_msg_t,data)+(ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_ota_rsp_t)));
}

int ais_ota_result(u8 result)
{
	ais_msg_t ais_msg_result;
	memset(&ais_msg_result, 0x00, sizeof(ais_msg_result));		

	ais_msg_result.msg_type = AIS_OTA_RESULT;
	ais_msg_result.length = ais_gatt_auth.auth_ok?AES_BLOCKLEN:1;
	ais_msg_result.ota_result = (OTA_SUCCESS==result) ? 1:0;

	if(ais_gatt_auth.auth_ok){
		ais_msg_result.enc_flag = 1;
		AES128_pkcs7_padding(ais_msg_result.data, 1, ais_msg_result.data);
		aes_cbc_encrypt(ais_msg_result.data, 1, &ctx, ais_gatt_auth.ble_key, iv);
	}
	return bls_att_pushNotifyData(AIS_NOTIFY_HANDLE, (u8 *)&ais_msg_result, OFFSETOF(ais_msg_t,data)+(ais_gatt_auth.auth_ok?AES_BLOCKLEN:1));
}

int ais_ota_rc_report(u8 frame_desc, u32 trans_size)
{
	ais_msg_t ais_msg_result;
	memset(&ais_msg_result, 0x00, sizeof(ais_msg_result));		

	ais_msg_result.msg_type = AIS_OTA_RECEVIED;
	ais_msg_result.length = ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_ota_receive_t);
	ais_msg_result.ais_ota_rcv.seg_index = frame_desc;
	ais_msg_result.ais_ota_rcv.trans_size_last = trans_size;
	if(ais_gatt_auth.auth_ok){
		ais_msg_result.enc_flag = 1;
		AES128_pkcs7_padding(ais_msg_result.data, sizeof(ais_ota_receive_t), ais_msg_result.data);
		aes_cbc_encrypt(ais_msg_result.data, sizeof(ais_ota_receive_t), &ctx, ais_gatt_auth.ble_key, iv);
	}
	return bls_att_pushNotifyData(AIS_NOTIFY_HANDLE, (u8 *)&ais_msg_result, OFFSETOF(ais_msg_t,data)+(ais_gatt_auth.auth_ok?AES_BLOCKLEN:sizeof(ais_ota_receive_t)));
}


extern u8 mesh_cmd_ut_rx_seg[];
const u8 company[4] = {'K', 'N', 'L', 'T'};
int ais_otaWrite(void * p)
{
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	static u8 err_flg = OTA_SUCCESS;
	static u32 fw_rcv_total_size = 0;
	ais_msg_t *msg_p = (ais_msg_t *)req->dat;
	if(ais_gatt_auth.auth_ok && (msg_p->msg_type != AIS_OTA_DATA)){
		u16 len = ((req->l2cap-3)+AES_BLOCKLEN-1)/AES_BLOCKLEN*AES_BLOCKLEN;
		aes_cbc_decrypt(msg_p->data, (len>AIS_MAX_DATA_SIZE)?0:len, &ctx, ais_gatt_auth.ble_key, iv);
	}
	
	if(msg_p->msg_type == AIS_FW_VERSION_GET){
		ais_ota_version_get();
	}
	else if(msg_p->msg_type == AIS_OTA_REQ){
		ais_ota_req(msg_p->data);
	}

	if(!blcOta.ota_start_flag){
		return 0;
	}
	
	if(msg_p->msg_type == AIS_OTA_END){
		if(OTA_CHECK_TYPE_TELINK_MESH == get_ota_check_type()){
			if(is_valid_ota_check_type1()){
				err_flg = OTA_SUCCESS;
			}
			else{
				err_flg = OTA_DATA_CRC_ERR;
			}
		}
		 	
		blt_ota_finished_flag_set(err_flg);

		ais_ota_result(err_flg);
	}
	else if(msg_p->msg_type == AIS_OTA_DATA){
		u16 cur_index =  ota_adr_index+1;
		if((msg_p->frame_seq == (cur_index%(msg_p->frame_total+1)))){
			blt_ota_start_tick = clock_time()|1;  //mark time			
			u16 data_len = msg_p->length;
			
			if(cur_index == 0){
				if(memcmp(req->dat+12, company, sizeof(company))){
					err_flg = OTA_ERR_STS;
				}
			}
			//log_data(TR_24_ota_adr,ota_adr);
			if((cur_index*data_len)>=(ota_firmware_size_k<<10)){
				err_flg = OTA_OVERFLOW;
			}else{
				ota_save_data (fw_rcv_total_size, req->dat + 4, data_len);

				flash_read_page(ota_program_offset + fw_rcv_total_size, data_len, mesh_cmd_ut_rx_seg);

				if(!memcmp(mesh_cmd_ut_rx_seg,req->dat + 4, data_len)){  //OK
					ota_adr_index++;
					fw_rcv_total_size += data_len;				
					if((!ais_gatt_auth.auth_ok) || (msg_p->frame_total == msg_p->frame_seq)){
						ais_ota_rc_report(msg_p->frame_desc, fw_rcv_total_size);
					}
				}
				else{ //flash write err
					err_flg = OTA_WRITE_FLASH_ERR;
				}
			}
				
		}
		else if(msg_p->frame_seq == (cur_index%(msg_p->frame_total+1))){  //maybe repeated OTA data, we neglect it, do not consider it ERR
			ais_ota_rc_report((msg_p->frame_desc & 0xf0)|(ota_adr_index % (msg_p->frame_total+1)), fw_rcv_total_size);
		}
		else{  //adr index err, missing at least one OTA data
			ais_ota_rc_report((msg_p->frame_desc & 0xf0)|(ota_adr_index % (msg_p->frame_total+1)), fw_rcv_total_size);
		}
	}

	if(err_flg){
		blt_ota_finished_flag_set(err_flg);
	}

	return 0;
}
#endif
