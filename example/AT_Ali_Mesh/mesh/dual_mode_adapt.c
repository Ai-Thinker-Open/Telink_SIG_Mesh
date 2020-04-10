/********************************************************************************************************
 * @file     dual_mode_adapt.c 
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
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh_common.h"
#if (__TL_LIB_8258__ || MCU_CORE_TYPE == MCU_CORE_8258)
#include "drivers/8258/rf_drv.h"
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
#include "../../drivers/8278/rf_drv.h"
#endif

void rf_setTxModeNew(void);
int is_zigbee_found();

#if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
u8 dual_mode_state = (FW_START_BY_BOOTLOADER_EN) ? DUAL_MODE_SUPPORT_DISABLE : DUAL_MODE_NOT_SUPPORT;
u8 rf_mode = RF_MODE_BLE;

#if (DUAL_MODE_WITH_TLK_MESH_EN)
#define CALI_CHUNK_SIZE     (64)

u32 backup_TLK_4K_fw()
{
    u8 data_read[CALI_CHUNK_SIZE];
    u8 data_write[CALI_CHUNK_SIZE];
    u32 crc_total = 0;
    u32 addr = 0;
    while(addr < (4096)){
        flash_read_page(addr, CALI_CHUNK_SIZE, data_read);
        u32 adr_write = FLASH_ADR_DUAL_MODE_4K + addr;
        flash_write_page(adr_write, CALI_CHUNK_SIZE,data_read);
        flash_read_page(adr_write, CALI_CHUNK_SIZE, data_write);
        if(0 != memcmp(data_read, data_write, CALI_CHUNK_SIZE)){
            start_reboot();
        }
    
        crc_total += crc16(data_read, CALI_CHUNK_SIZE);
        addr += CALI_CHUNK_SIZE;
    }

    return crc_total;
}

void dual_mode_backup_TLK_4K()
{
    // avoid power off during restore flash, so it should be better save in 4K in flash but not RAM.
    int backup_ok = 0;
    u32 cali_flag = 0;
    flash_read_page(CFG_ADR_DUAL_CALI_VAL_FLAG, 4, (u8 *)&cali_flag);
    if(cali_flag){
        if(0xffffffff == cali_flag){
            u32 crc_total = backup_TLK_4K_fw();
            cali_flag = DUAL_MODE_CALI_VAL_FLAG;
            flash_write_page(CFG_ADR_DUAL_CALI_VAL_FLAG, 4, (u8 *)&cali_flag);
            flash_write_page(CFG_ADR_DUAL_CALI_VAL, 4, (u8 *)&crc_total);
            backup_ok = 1;
        }else if(DUAL_MODE_CALI_VAL_FLAG == cali_flag){
            u32 fw_4k = 0;
            flash_read_page(FLASH_ADR_DUAL_MODE_4K, 4, (u8 *)&fw_4k);
            if(0xffffffff == fw_4k){
                u32 crc_total = backup_TLK_4K_fw();
                u32 crc_MIC = 0;
                flash_read_page(CFG_ADR_DUAL_CALI_VAL, 4, (u8 *)&crc_MIC);
                if(crc_total == crc_MIC){
                    backup_ok = 1;
                }
            }else{
                backup_ok = 1;
                // already backup
            }
        }else{
            u32 zero = 0;
            flash_write_page(CFG_ADR_DUAL_CALI_VAL_FLAG, 4, (u8 *)&zero);
        }
    }

    if(0 == backup_ok){
        dual_mode_state = DUAL_MODE_NOT_SUPPORT;    // should not happen
    }
}

void dual_mode_restore_TLK_4K()
{
    // restore 4K, and calibration, if calibrate failed, try again.
    u32 cali_flag = 0;
    flash_read_page(CFG_ADR_DUAL_CALI_VAL_FLAG, 4, (u8 *)&cali_flag);
    if((DUAL_MODE_CALI_VAL_FLAG == cali_flag) && (0 == ota_program_offset)){
        flash_erase_sector(0);
        
        u8 data_read[CALI_CHUNK_SIZE];
        u8 data_write[CALI_CHUNK_SIZE];
        u32 crc_total = 0;
        u32 addr = 0;
        while(addr < (4096)){
            flash_read_page(FLASH_ADR_DUAL_MODE_4K + addr, CALI_CHUNK_SIZE, data_read);
            crc_total += crc16(data_read, CALI_CHUNK_SIZE);
            if(0 == addr){
                data_read[8] = 0xff;    // org is 0
            }
            flash_write_page(addr, CALI_CHUNK_SIZE,data_read);
            flash_read_page(addr, CALI_CHUNK_SIZE, data_write);
            if(0 != memcmp(data_read, data_write, CALI_CHUNK_SIZE)){
                start_reboot();
            }
        
            addr += CALI_CHUNK_SIZE;
        }
        
        u32 crc_MIC = 0;
        flash_read_page(CFG_ADR_DUAL_CALI_VAL, 4, (u8 *)&crc_MIC);
        if(crc_MIC == crc_total){
            u8 start_flag = 0x4b;
            flash_write_page(8, 1, (u8 *)&start_flag);      //Set FW flag
        }
    }
}

int UI_resotre_TLK_4K_with_check()
{
    if(DUAL_MODE_NOT_SUPPORT != dual_mode_state){
        dual_mode_restore_TLK_4K();
        return 1;
    }
    return 0;
}
#endif

#define START_UP_FLAG		(0x544c4e4b)

void dual_mode_en_init()		// call in mesh_init_all();
{
#if (0 == FW_START_BY_BOOTLOADER_EN)
	u8 en = 0;
	flash_read_page(CFG_ADR_DUAL_MODE_EN, 1, (u8 *)&en);
    LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"dual mode enable flag 0x76080:0x%x",en);
	if(0xff == en){
	    en = DUAL_MODE_SAVE_ENABLE;
	    flash_write_page(CFG_ADR_DUAL_MODE_EN, 1, (u8 *)&en);
	}

	if(DUAL_MODE_SAVE_ENABLE == en)
#endif
	{
		u32 startup_flag1 = 0;
		u32 startup_flag2 = 0;
		flash_read_page(DUAL_MODE_FW_ADDR_SIGMESH + 8, 4, (u8 *)&startup_flag1);
		startup_flag1 |= 0x4b;  // recover.
		flash_read_page(DUAL_MODE_FW_ADDR_ZIGBEE + 8, 4, (u8 *)&startup_flag2);
		if((START_UP_FLAG == startup_flag1) && (START_UP_FLAG == startup_flag2)){
            u32 mesh_type = 0;
            flash_read_page(FLASH_ADR_MESH_TYPE_FLAG, sizeof(mesh_type), (u8 *)&mesh_type);
            #if DUAL_MODE_WITH_TLK_MESH_EN
            if(TYPE_DUAL_MODE_STANDBY == mesh_type){
                dual_mode_backup_TLK_4K();
            }
            
            if(TYPE_DUAL_MODE_RECOVER == mesh_type){
                dual_mode_state = DUAL_MODE_SUPPORT_ENABLE;
                dual_mode_restore_TLK_4K();
                start_reboot();
            }else if(TYPE_DUAL_MODE_STANDBY == mesh_type){
			    dual_mode_state = DUAL_MODE_SUPPORT_ENABLE;
                LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support enable",0);
			}else{  // only TYPE_SIG_MESH
			    dual_mode_state = DUAL_MODE_SUPPORT_DISABLE;
                LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support disable",0);
			}
            #else
		    if(TYPE_DUAL_MODE_STANDBY == mesh_type){
			    dual_mode_state = DUAL_MODE_SUPPORT_ENABLE;
                LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support enable",0);
		    }else{
			    dual_mode_state = DUAL_MODE_SUPPORT_DISABLE;
                LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support disable",0);
			}
			#endif
		}else{
		    #if (0 == FW_START_BY_BOOTLOADER_EN)
            en = 0;
            #endif
		}
	}

#if (0 == FW_START_BY_BOOTLOADER_EN)
	if(en && (DUAL_MODE_SAVE_ENABLE != en)){
	    en = 0;
	    flash_write_page(CFG_ADR_DUAL_MODE_EN, 1, (u8 *)&en);
	}
#endif

	if(DUAL_MODE_SUPPORT_ENABLE == dual_mode_state){
		rf_link_light_event_callback(LGT_CMD_DUAL_MODE_MESH);
	}
}

void dual_mode_disable()
{
#if (FW_START_BY_BOOTLOADER_EN)
    // enable forever
#else
	if(DUAL_MODE_NOT_SUPPORT != dual_mode_state){
		dual_mode_state = DUAL_MODE_NOT_SUPPORT;
		u8 zero = 0;
		flash_write_page(CFG_ADR_DUAL_MODE_EN, 1, (u8 *)&zero);
        LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode not support",0);
	}
#endif
}

void dual_mode_select()    // 
{
	if(DUAL_MODE_SUPPORT_ENABLE == dual_mode_state){
		dual_mode_state = DUAL_MODE_SUPPORT_DISABLE;
		#if DUAL_MODE_WITH_TLK_MESH_EN
        set_firmware_type_SIG_mesh();
        LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support disable: select SIG MESH",0);
		#else
		if(rf_mode == RF_MODE_BLE){
            set_firmware_type_SIG_mesh();
            LOG_MSG_LIB(TL_LOG_NODE_SDK,0, 0,"Dual mode support disable: select BLE",0);
		}else{
		    #if (FW_START_BY_BOOTLOADER_EN)
            set_firmware_type_zb_with_factory_reset();
		    #else
            u8 zero = 0;
			u32 adr_boot_disable = ota_program_offset ? 0 : 0x40000;
            flash_write_page(adr_boot_disable + 8, 1, (u8 *)&zero);
            #endif
            start_reboot();
		}
		#endif
	}
}
#else
void dual_mode_en_init(){}
void dual_mode_disable(){};
#endif

#if DUAL_MODE_ADAPT_EN
#define DUAL_MODE_SWITCH_INV_US		(3000*1000)

// ---------------------ZigBee driver

#define			RF_MANUAL_AGC_MAX_GAIN	1

#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )

#define TX_GAIN     		0x93
//#define RF_PKT_BUFF_LEN		128	// should be same with BLE: 256
#define TP_2M_G0_DFT	 	0x45
#define TP_2M_G1_DFT		0x39

#define 	ZB_RF_ACTUAL_PAYLOAD_POST		13
#define		ZB_RF_PACKET_LENGTH_OK(p)		(p[0] == p[12]+13)
#define		ZB_RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x10)

const TBLCMDSET  setting_rf_250k[] = {
	{0x9e, 0xad,  TCMD_UNDER_BOTH | TCMD_WAREG}, 		//reg_dc_mod (500K); ble: 250k
	{0xa3, 0x10,  TCMD_UNDER_BOTH | TCMD_WAREG}, 		//pa_ramp_en = 1, pa ramp table max
	{0xaa, 0x2a,  TCMD_UNDER_BOTH | TCMD_WAREG},		//filter iq_swap, 2M bandwidth

	{0x0400, 0x03,	TCMD_UNDER_BOTH | TCMD_WRITE},	// 250K mode
	{0x0401, 0x40,	TCMD_UNDER_BOTH | TCMD_WRITE},	// pn enable
	{0x0402, 0x26,	TCMD_UNDER_BOTH | TCMD_WRITE},	// 8-byte pre-amble
	{0x0404, 0xc0,	TCMD_UNDER_BOTH | TCMD_WRITE},	// head_mode/crc_mode: normal c0
	{0x0405, 0x04,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code length 4
	{0x0408, 0xc9,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code byte3
	{0x0409, 0x8a,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code byte2
	{0x040a, 0x11,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code byte1
	{0x040b, 0xf8,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code byte0
	{0x04eb, 0xa0,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code byte0

	{0x0420, 0x90,	TCMD_UNDER_BOTH | TCMD_WRITE},	// sync threshold: 1e (4); 26 (5)
	{0x0421, 0x00,	TCMD_UNDER_BOTH | TCMD_WRITE},	// no avg
	{0x0422, 0x1a,	TCMD_UNDER_BOTH | TCMD_WRITE},	// threshold
	{0x0424, 0x52,	TCMD_UNDER_BOTH | TCMD_WRITE},	// number for sync: bit[6:4]
	{0x042b, 0xf5,	TCMD_UNDER_BOTH | TCMD_WRITE},	// access code: 1
	{0x042c, 0x88,	TCMD_UNDER_BOTH | TCMD_WRITE},	// maxiumum length 48-byte

	{0x0f03, 0x1e,	TCMD_UNDER_BOTH | TCMD_WRITE},	// bit3: crc2_en; normal 1e
	{0x0060, 0x80,	TCMD_UNDER_BOTH | TCMD_WRITE},	// reset baseband
	{0x0060, 0x00,	TCMD_UNDER_BOTH | TCMD_WRITE},	// reset baseband
};

/* set Rx mode, maxium receiver buffer size, enable Rx/Tx interrupt */
#define ZB_RADIO_TRX_CFG(len)	do{				\
									reg_dma2_ctrl = FLD_DMA_WR_MEM | ((len)>>4);   \
									reg_dma_chn_irq_msk  &= ~(FLD_DMA_CHN_RF_RX|FLD_DMA_CHN_RF_TX); \
									reg_irq_mask |= FLD_IRQ_ZB_RT_EN;   \
									reg_rf_irq_mask = FLD_RF_IRQ_RX | FLD_RF_IRQ_TX; \
								}while(0)

#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))

#define TP_2M_G0_DFT	 	0x40
#define TP_2M_G1_DFT		0x39

#define 	ZB_RF_ACTUAL_PAYLOAD_POST		5
#define		ZB_RF_PACKET_LENGTH_OK(p)		(p[0]  == p[4]+9)
#define		ZB_RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x10)

const TBLCMDSET setting_rf_250k_init[] = {
    {{0x12d2}, {0x9b},  {TCMD_UNDER_BOTH | TCMD_WRITE}}, //DCOC_DBG0
    {{0x12d3}, {0x19},  {TCMD_UNDER_BOTH | TCMD_WRITE}}, //DCOC_DBG1
    {{0x127b}, {0x0e},  {TCMD_UNDER_BOTH | TCMD_WRITE}}, //BYPASS_FILT_1
    {{0x1276}, {0x50},  {TCMD_UNDER_BOTH | TCMD_WRITE}}, //FREQ_CORR_CFG2_0
    {{0x1277}, {0x73},  {TCMD_UNDER_BOTH | TCMD_WRITE}}, //FREQ_CORR_CFG2_1
};

const TBLCMDSET  setting_rf_250k[] =
{
    {{0x1220}, {0x04}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x1221}, {0x2b}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x1222}, {0x43}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x1223}, {0x86}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x122a}, {0x90}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x1254}, {0x0e}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD1_2M_0
    {{0x1255}, {0x09}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD1_2M_1
    {{0x1256}, {0x0c}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD2_2M_0
    {{0x1257}, {0x08}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD2_2M_1
    {{0x1258}, {0x09}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD3_2M_0
    {{0x1259}, {0x0f}, {TCMD_UNDER_BOTH | TCMD_WRITE}}, //AGC_THRSHLD3_2M_1

    {{0x400}, {0x13}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//{{0x400}, {0x0a},	{TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x401}, {0x00}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//zigBee must set
    {{0x420}, {0x18}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x404}, {0xc0}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x405}, {0x04}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x421}, {0x23}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x422}, {0x04}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x408}, {0xa7}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x409}, {0x00}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x40a}, {0x00}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    {{0x40b}, {0x00}, {TCMD_UNDER_BOTH | TCMD_WRITE}},
    //AGC table 2M
    {{0x460}, {0x36}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_0
    {{0x461}, {0x46}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_1
    {{0x462}, {0x51}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_2
    {{0x463}, {0x61}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_3
    {{0x464}, {0x6d}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_4
    {{0x465}, {0x78}, {TCMD_UNDER_BOTH | TCMD_WRITE}},//grx_5
};

/* set Rx mode, maxium receiver buffer size, enable Rx/Tx interrupt */
#define ZB_RADIO_TRX_CFG(len)				do{\
												reg_dma_rf_rx_size = FLD_DMA_WR_MEM | (len >> 4);\
												dma_irq_disable(FLD_DMA_CHN_RF_RX|FLD_DMA_CHN_RF_TX); \
												reg_rf_irq_mask |= FLD_RF_IRQ_RX | FLD_RF_IRQ_TX; \
												irq_set_mask(FLD_IRQ_ZB_RT_EN); \
											}while(0)


void rf_ble_1m_param_recovery_from_zb(void)
{
    // exclude parameters have been recover in rf_drv_init_()
    write_reg8(0x1220, 0x16);
    write_reg8(0x1221, 0x0a);
    write_reg8(0x1222, 0x20);
    write_reg8(0x1223, 0x23);
    write_reg8(0x1228, 0x13);
    write_reg8(0x1229, 0x00);
    write_reg8(0x122a, 0x90);

    write_reg8(0x1254, 0x89);
    write_reg8(0x1255, 0x06);
    write_reg8(0x1256, 0x8c);
    write_reg8(0x1257, 0x07);
    write_reg8(0x1258, 0x09);
    write_reg8(0x1259, 0x0f);

    /*recover parameter in rf_set_channel()*/
    //write_reg8(0x1244, 0x20);
    CLR_FLD(REG_ADDR8(0x1244), BIT(0)); // have been set in rf_set_channel()
    //write_reg8(0x1245, 0x26);
    //write_reg8(0x1229, 0x00);
    /*recover end*/

    write_reg8(0x400, 0x1f);  //poweron_dft: 0x1f, no need set to save timing
    write_reg8(0x405, 0x04);  //poweron_dft: 0x04, no need set to save timing
    write_reg8(0x420, 0x1e);  //poweron_dft: 0x1e, no need set to save timing
    write_reg8(0x421, 0xa1);  //poweron_dft: 0xa1, no need set to save timing
    write_reg8(0x422, 0x00);
    write_reg8(0x408, 0xc9);
    write_reg8(0x409, 0x8a);
    write_reg8(0x40a, 0x11);
    write_reg8(0x40b, 0xf8);
}
#endif


static u8 TP_2M_G0 = TP_2M_G0_DFT;		//69
static u8 TP_2M_G1 = TP_2M_G1_DFT;		//57

#define		TP_GET_GAIN(g0, g1)		((g0 - g1)*256/80)

static int			rf_tp_baseNew = TP_2M_G0_DFT;
static int			rf_tp_gainNew = TP_GET_GAIN(TP_2M_G0_DFT, TP_2M_G1_DFT);

u32		rf_ac;
static  u32 rf_buff_250k;

static u8 rf_curCh;
static u8 rf_txTpGain;
//static u8 rf_rxTpGain;
static u8 bufIdx = 0;
static u8 curChannel = 11;
static u8 zigbeeNetworkFound = 0;

static u8 _attribute_aligned_(4) rf_tx_buf[64];

typedef struct{
	u8 protocolId;
	u16	stackProfile:4;
	u16	nwkProtocolVer:4;
	u16	resv:2;
	u16 routerCap:1;
	u16 deviceDepth:4;
	u16 edCap:1;
}zb_beacon_pld_t;

typedef struct{
	u16 frmCtrl;
	u8  seqNo;
	u16 srcPanID;
	u16 srcAddr;
}zb_mac_hdr_t;

typedef struct{
	u16 sfSpecification;
	u8  gts;
	u8 pendAddr;
	zb_beacon_pld_t beaconInfo;
}zb_mac_pld_t;

u8 zbBeaconReq[] = {0x03, 0x08, /* frame control*/
					0x0, /* sequence number*/
					0xff, 0xff, /* destination PANID*/
					0xff, 0xff, /* destination address*/
					0x07		/* commandId: beacon request */
					};

static const unsigned char rf_channelNew[MAX_RF_CHANNEL] = {
	FRE_OFFSET+ 5, FRE_OFFSET+10, FRE_OFFSET+15, FRE_OFFSET+20,
	FRE_OFFSET+25, FRE_OFFSET+30, FRE_OFFSET+35, FRE_OFFSET+40,
	FRE_OFFSET+45, FRE_OFFSET+50, FRE_OFFSET+55, FRE_OFFSET+60,
	FRE_OFFSET+65, FRE_OFFSET+70, FRE_OFFSET+75, FRE_OFFSET+80,
};

void rf_drv_250k_new (void)
{
#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
	write_reg32(0x800508,rf_buff_250k);
#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	write_reg32(0x800c08,rf_buff_250k);
#endif
	rf_tp_baseNew = TP_2M_G0;
	rf_tp_gainNew = TP_GET_GAIN(TP_2M_G0, TP_2M_G1);
#if (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	LoadTblCmdSet(setting_rf_250k_init, sizeof(setting_rf_250k_init)/sizeof(TBLCMDSET));
#endif
	LoadTblCmdSet(setting_rf_250k, sizeof(setting_rf_250k)/sizeof(TBLCMDSET));
}

void rf_recv_mode_250k_new(u16 mode, u16 size, u16 addr){
#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
	rf_buff_250k = (mode << 16) | (size << 16) | addr;
#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	rf_buff_250k = (mode << 24) | (size << 16) | addr;
#endif
}

/*********************************************************************
 * @fn      rf_tx
 *
 * @brief   Transmit the data out.
 *
 * @param   buf - The buffer to be transmitted
 * @param   len - The length of byte to be transmitted
 *
 * @return  none
 */
void rf_txNew(u8* buf, u8 len)
{
	if(len > (64 - 5)){
		return;
	}
    /* Fill the telink RF header */
    // Dma length
	rf_tx_buf[0] = len+1;
	rf_tx_buf[1] = 0;
	rf_tx_buf[2] = 0;
	rf_tx_buf[3] = 0;

	// RF length
	rf_tx_buf[4] = len+2;

    // Payload
	memcpy(rf_tx_buf+5, buf, len);
	reg_dma_rf_tx_addr = (u16)(u32)(rf_tx_buf);

    // Enter tx mode
    rf_setTxModeNew();

	//trig
#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
    write_reg8(0x800f00, 0x85);
#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
    write_reg8(0x800c43, 0x04);
    reg_dma_tx_rdy0 |= FLD_DMA_CHN_RF_TX;
    //while((read_reg8(0x800f20) & 0x02) != 0x02);
    //write_reg8(0x800f20, read_reg8(0x800f20) | 0x02);
#endif

	// Manual Mode
    //write_reg8(0x800f02, 0x45 | BIT(4));  // Manual TX enable
	//write_reg8(0x800524, 0x08);
}


_attribute_ram_code_ void rf_setTxModeNew(void)
{
	u8 chn = rf_channelNew[rf_curCh];
#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
	//analog_write (0x06, 0xfe);
    write_reg8 (0x800428, 0xe0);  //RF_TRX_MODE);	// rx disable
    write_reg8 (0x800f02, 0x45);  //RF_TRX_OFF);	    // reset tx/rx state machine
    write_reg16 (0x8004d6, 2400 + chn);	// {intg_N}
    analog_write (TX_GAIN, rf_txTpGain);
	//analog_write (0x06, 0x00);
	//write_reg8(0x800f00, 0x80);
#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
    //rf_trx_state_set(RF_MODE_TX, rf_curCh);
    rf_set_channel(chn, 0);

    WRITE_REG8(0xf02, 0x45);  //reset tx_en in manual mode
    WRITE_REG8(0xf02, 0x55);  //tx_en
    WRITE_REG8(0x428, READ_REG8(0x428) & 0xfe);  //rx disable

//    write_reg8(0x800428, 0xe0);
//    write_reg8(0x800f02, 0x45);
//    write_reg8(0x800f02, 0x55);

    WaitUs(150);
#endif
}

_attribute_ram_code_ void rf_setRxModeNew(void)
{
	u8 chn = rf_channelNew[rf_curCh];
#if (__TL_LIB_8269__ || MCU_CORE_TYPE == MCU_CORE_8269 )
	//analog_write (0x06, 0xfe);
    write_reg8 (0x800428, 0xe0);  //RF_TRX_MODE);	// rx disable
    write_reg8 (0x800f02, 0x45);  //RF_TRX_OFF);	    // reset tx/rx state machine
    write_reg16 (0x8004d6, 2400 + chn);	// {intg_N}
    analog_write (TX_GAIN, 0x0);//rf_rxTpGain);

    write_reg8 (0x800428, 0xe0 | BIT(0));	// rx enable
    write_reg8 (0x800f02, 0x45 | BIT(5));	// RX enable
    //analog_write (0x06, 0x00);
	//write_reg8(0x800f00, 0x80);
#elif (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
    //rf_trx_state_set(RF_MODE_RX, rf_curCh);

    rf_set_channel(chn, 0);

	WRITE_REG8(0xf02, 0x45);  //reset tx_en in manual mode
    WRITE_REG8(0xf02, 0x65);
    WRITE_REG8(0x428, READ_REG8(0x428) | 0x01);  //rx disable

//    write_reg8(0x800428, 0xe1);
//    write_reg8(0x800f02, 0x45);
//    write_reg8(0x800f02, 0x65);
#endif
}

void rf_setChannelNew(u8 ch){
    rf_curCh = ch - 11;
	u8 chn = rf_channelNew[rf_curCh];
    //rf_rxTpGain = gain_tp[(((chn+4)*6)>>5)&15]; //set RX gain
    rf_txTpGain = rf_tp_baseNew - ((chn * rf_tp_gainNew + 128) >> 8); //set TX gain

	//write_reg16 (0x8004d6, 2400 + chn);	// {intg_N}
    //analog_write (TX_GAIN, rf_txTpGain);
}

volatile u8 T_zbRfTxRxCnt[4] = {0};
_attribute_ram_code_ /*__attribute__((optimize("-Os")))*/ void zb_rf_tx_irq_handler(void){
	/* Set to clear the interrupt flag */
	reg_rf_irq_status = FLD_RF_IRQ_TX;

	/* Must change to RX mode first, otherwise the next ACK may miss */

	rf_setRxModeNew();
	T_zbRfTxRxCnt[0]++;
}


_attribute_ram_code_ /*__attribute__((optimize("-Os")))*/ void zb_rf_rx_irq_handler(void){

	u8 * raw_pkt = (u8 *) (blt_rxfifo_b + (blt_rxfifo.wptr & (blt_rxfifo.num-1)) * blt_rxfifo.size);

	T_zbRfTxRxCnt[1]++;
	if ( (!ZB_RF_PACKET_CRC_OK(raw_pkt)) || (!ZB_RF_PACKET_LENGTH_OK(raw_pkt)) ) {
		//raw_pkt[12] = 0;
		//*((u32*)raw_pkt) = 0;
		reg_rf_irq_status = FLD_RF_IRQ_RX;
		return;
	}

	blt_rxfifo.wptr++;
	u8 * new_pkt = (blt_rxfifo_b + (blt_rxfifo.wptr & (blt_rxfifo.num-1)) * blt_rxfifo.size);

	bufIdx = bufIdx ? 0 : 1;
	reg_dma_rf_rx_addr = (u16)(u32)new_pkt;//(T_zb_rxRxBuf[bufIdx]);  //new_pkt;

	/* Set to clear the interrupt flag */
	reg_rf_irq_status = FLD_RF_IRQ_RX;

	/* Must change to RX mode first, otherwise the next ACK may miss */

	//rf_setRxMode();
	T_zbRfTxRxCnt[2]++;
	//dma_channel_disable_all();
	//while(1);
}

_attribute_ram_code_ void irq_zigbee_sdk_handler(void){
	u16  src_rf = reg_rf_irq_status;
	if(src_rf & FLD_RF_IRQ_RX){
		zb_rf_rx_irq_handler();
	}

	if(src_rf & FLD_RF_IRQ_TX){
		zb_rf_tx_irq_handler();
	}
}

#define ZB_ACTIVE_SCAN_DURATION			 (DUAL_MODE_SWITCH_INV_US/27) // - 200*1000) // 10 * 1000 * 1000
volatile u8 T_zigbee_network_scan_cnt[2] = {0};
static u32 scanDuration = 0;
void zigbee_network_scan(void){
	//static u8 once = 1;
	if(!is_zigbee_found() && clock_time_exceed(scanDuration, ZB_ACTIVE_SCAN_DURATION)){
		scanDuration = clock_time();
		T_zigbee_network_scan_cnt[1]++;
		rf_setChannelNew(curChannel);
		rf_txNew(zbBeaconReq, 8);
		curChannel++;
		//once = 0;
	}
}

volatile u8 T_zbRfRxCnt[4] = {0};

void zigbee_recv_data_poll(void){
	while (blt_rxfifo.rptr != blt_rxfifo.wptr){
		u8 *raw_pkt = (u8 *) (blt_rxfifo_b + blt_rxfifo.size * (blt_rxfifo.rptr++ & (blt_rxfifo.num-1)));
		T_zbRfRxCnt[0]++;

		zb_mac_hdr_t hdr;
		memcpy(&hdr, &raw_pkt[ZB_RF_ACTUAL_PAYLOAD_POST], sizeof(zb_mac_hdr_t));
		if(hdr.frmCtrl == 0x8000){
			T_zbRfRxCnt[1]++;
			zb_mac_pld_t macPld;
			memcpy(&macPld, &raw_pkt[ZB_RF_ACTUAL_PAYLOAD_POST+sizeof(zb_mac_hdr_t)], sizeof(zb_mac_pld_t));
			if(macPld.gts == 0 && ((macPld.sfSpecification & 0xbfff) == 0x8fff) && macPld.pendAddr == 0 &&
				macPld.beaconInfo.protocolId == 0 &&
				macPld.beaconInfo.stackProfile == 0x02 &&
				macPld.beaconInfo.nwkProtocolVer == 0x02 &&
				macPld.beaconInfo.routerCap == 0x01 &&
				macPld.beaconInfo.edCap == 0x01){
				T_zbRfRxCnt[2]++;
				zigbeeNetworkFound = 1;
			}
		}
	}
}


void dual_mode_zigbee_init(void){
	dma_channel_disable_all();

	u8 *raw_pkt = (u8 *)(blt_rxfifo_b + (blt_rxfifo.wptr & (blt_rxfifo.num-1)) * blt_rxfifo.size);
		//u8 * new_pkt = (blt_rxfifo_b + (blt_rxfifo.wptr & (blt_rxfifo.num-1)) * blt_rxfifo.size);

	ZB_RADIO_TRX_CFG(RF_PKT_BUFF_LEN);

	/* Set TX Buffer */
	reg_dma_rf_tx_addr = (u16)(u32)(rf_tx_buf);

	//u8 * raw_pkt = (u8 *) (blt_rxfifo_b + (blt_rxfifo.wptr & (blt_rxfifo.num-1)) * blt_rxfifo.size);
	rf_recv_mode_250k_new(FLD_DMA_WR_MEM, (128>>4), (u16)((u32)raw_pkt));
	bufIdx = 0;
	rf_drv_250k_new();

	dma_channel_enable_all();

	rf_setChannelNew(11);

	//rf_setRxModeNew();
	rf_setTxModeNew();
}


// ---------------------dual mode switch check
int is_ble_found()
{
	return ((BLS_LINK_STATE_ADV != blt_state) || (get_provision_state() != STATE_DEV_UNPROV));
}

int is_zigbee_found()
{
	return zigbeeNetworkFound;
}

void zigbee_found_clear()
{
	zigbeeNetworkFound = 0;
}

volatile u8 T_zigbeeSdkRun;
volatile u8 T_DBG_zigbeeTest[2] = {0};
u8 dual_mode_proc()
{
	if(DUAL_MODE_SUPPORT_ENABLE != dual_mode_state){
		return RF_MODE_BLE;
	}
	
	static u32 dual_mode_tick;
	if(is_ble_found()){
		dual_mode_tick = clock_time();	// switch mode pause
	}else if(is_zigbee_found()){
		dual_mode_tick = clock_time();	// switch mode pause

		if(rf_mode == RF_MODE_ZIGBEE){
			T_zigbeeSdkRun = 1;
			dual_mode_select(); // just select, disable by Zigbee SDK when OTA start
			//have been reboot in dual mode slecte()  from zigbee sdk
			start_reboot();
		}else{
		    zigbee_found_clear();
		}
	}else{
		if(clock_time_exceed(dual_mode_tick, DUAL_MODE_SWITCH_INV_US)){
			dual_mode_tick = clock_time();
			u8 r = irq_disable();
			static u8 val_settle;
			if(rf_mode == RF_MODE_BLE){
				rf_mode = RF_MODE_ZIGBEE;

				T_DBG_zigbeeTest[0]++;

				curChannel = 11;
                val_settle = REG_ADDR8(0xf04);
				dual_mode_zigbee_init();
			}else{
				#if 0	// comfirm later
				start_reboot();
				#else
				rf_mode = RF_MODE_BLE;
				if(!val_settle){
                    val_settle = REG_ADDR8(0xf04);  // init
				}
				
				    #if (__TL_LIB_8258__ || (MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
				rf_ble_1m_param_recovery_from_zb();
				rf_drv_init(RF_MODE_BLE_1M);    // it would init settle time and RF offset
	            blc_ll_initBasicMCU();
                blc_ll_initStandby_module(tbl_mac);             //mandatory
	                #else
				rf_drv_init(CRYSTAL_TYPE);      // it would init settle time and RF offset
				blc_ll_initBasicMCU(tbl_mac);   // init DMA register
				    #endif
				// restore
				REG_ADDR8(0xf04) = val_settle;
				blc_app_loadCustomizedParameters();
				static u8 dual_mode_A_1;dual_mode_A_1++;
				#endif
			}
			irq_restore(r);
		}
	}
	
	if(rf_mode == RF_MODE_ZIGBEE){
		static u32 dual_mode_Zigbee_loop;dual_mode_Zigbee_loop++;
		zigbee_network_scan();
		zigbee_recv_data_poll();
		
		T_DBG_zigbeeTest[1]++;

		return RF_MODE_ZIGBEE;
	}

	return RF_MODE_BLE;
}
#endif


