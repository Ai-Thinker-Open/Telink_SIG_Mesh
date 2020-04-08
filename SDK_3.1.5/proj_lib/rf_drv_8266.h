/********************************************************************************************************
 * @file     rf_drv_8266.h 
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
#ifndef _RF_DRV_H_
#define _RF_DRV_H_

////////////External Crystal Type///////////////////
enum{
	XTAL_12M_RF_1m_MODE   = BIT(0),
	XTAL_12M_RF_2m_MODE   = BIT(1),
	XTAL_12M_RF_250k_MODE = BIT(2),
	XTAL_16M_RF_1m_MODE   = BIT(3),
	XTAL_16M_RF_2m_MODE   = BIT(4),
	XTAL_16M_RF_250k_MODE = BIT(5),

	XTAL_12M	= XTAL_12M_RF_1m_MODE,
	XTAL_16M    = XTAL_16M_RF_1m_MODE,
};

#define IS_XTAL_12M(mode)  		( (mode) & 0x07)
#define IS_XTAL_16M(mode)  		( (mode) & 0x38)
#define IS_RF_1M_MODE(mode)		( (mode) & 0x09)
#define IS_RF_2M_MODE(mode)		( (mode) & 0x12)
#define IS_RF_250K_MODE(mode)	( (mode) & 0x24)

#ifndef	CRYSTAL_TYPE
#define CRYSTAL_TYPE		XTAL_12M
#endif



#define RF_CHN_AUTO_CAP 	0xff00
#define RF_CHN_TABLE 		0x8000
#define RF_SET_TX_MANAUL	0x4000

#define FRE_OFFSET 	0
#define FRE_STEP 	5
#define MAX_RF_CHANNEL  16

#define RF_CHANNEL_MAX			16
#define RF_CHANNEL_MASK			(RF_CHANNEL_MAX - 1)

extern const unsigned char	rf_chn[RF_CHANNEL_MAX];


enum {
	RF_POWER_8dBm	= 0,
	RF_POWER_4dBm	= 1,
	RF_POWER_0dBm	= 2,
	RF_POWER_m4dBm	= 3,
	RF_POWER_m10dBm	= 4,
	RF_POWER_m14dBm	= 5,
	RF_POWER_m20dBm	= 6,
	RF_POWER_m24dBm	= 8,
	RF_POWER_m28dBm	= 9,
	RF_POWER_m30dBm	= 10,
	RF_POWER_m37dBm	= 11,
	RF_POWER_OFF	= 16,
};


extern u8 		sar_adc_pwdn_en;
extern int		xtalType_rfMode;


#define POWER_DOWN_64MHZ_CLK			analog_write(0x82, IS_XTAL_12M(xtalType_rfMode) ? 0x00 : 0x14)
#define POWER_ON_64MHZ_CLK				analog_write(0x82, IS_XTAL_12M(xtalType_rfMode) ? 0x20 : 0x34)


static inline void adc_clk_powerdown(void)
{
	sar_adc_pwdn_en = 1;
}
static inline void adc_clk_poweron(void)
{
	sar_adc_pwdn_en = 0;
}

#define PHY_POWER_DOWN                  analog_write(0x06, sar_adc_pwdn_en ? 0xff : 0xfe)
#define PHY_POWER_UP					analog_write(0x06, sar_adc_pwdn_en ? 0x01 : 0x00)

void SetRxMode (signed char chn, unsigned short set);
void SetTxMode (signed char chn, unsigned short set);
void TxPkt (void* addr);

void rf_set_ble_channel (signed char chn);
void rf_start_stx2rx  (void* addr, u32 tick);
void rf_start_btx (void* addr, u32 tick);


static inline void rf_set_tx_pipe_long_packet (u8 pipe)
{
	write_reg8 (0x800f15, 0x70 | pipe);
}

static inline void rf_set_tx_pipe (u8 pipe)
{
	write_reg8 (0x800f15, 0xf0 | pipe);
}

static inline void rf_set_ble_crc (u8 *p)
{
	write_reg32 (0x80044c, p[0] | (p[1]<<8) | (p[2]<<16));
}

static inline void rf_set_ble_crc_value (u32 crc)
{
	write_reg32 (0x80044c, crc);
}

static inline void rf_set_ble_crc_adv ()
{
	write_reg32 (0x80044c, 0x555555);
}

static inline void rf_set_ble_access_code (u8 *p)
{
	write_reg32 (0x800408, p[3] | (p[2]<<8) | (p[1]<<16) | (p[0]<<24));
}


static inline void rf_set_ble_access_code_value (u32 ac)
{
	write_reg32 (0x800408, ac);
}

static inline void rf_set_ble_access_code_adv ()
{
#if (TEST_SPECAIL_ADV_ACCESS_CODE)
	write_reg32 (0x800408, 0x12345678);
#else
	write_reg32 (0x800408, 0xd6be898e);
#endif
}

static inline void rf_set_access_code0 (u32 code)
{
	write_reg32 (0x800408, (read_reg32(0x800408) & 0xff) | (code & 0xffffff00));
	write_reg8  (0x80040c, code);
}

static inline u32 rf_get_access_code0 (void)
{
	return read_reg8 (0x80040c) | (read_reg32(0x800408) & 0xffffff00);
}

static inline void rf_set_access_code1 (u32 code)
{
	write_reg32 (0x800410, (read_reg32(0x800410) & 0xff) | (code & 0xffffff00));
	write_reg8  (0x800414, code);
}

static inline u32 rf_get_access_code1 (void)
{
	return read_reg8 (0x800414) | (read_reg32(0x800410) & 0xffffff00);
}

static inline u32 light_proc_conflict_ac_32(u32 ac)
{
	u32 r = ac;
	u8 zero_cnt = 0;
	u8 one_cnt = 0;
	foreach(i, 16){
		if((u8)(r & 0x3) == 0x01){
		    ++one_cnt;
		}else if((u8)(r & 0x3) == 0x02){
		    ++zero_cnt;
		}
		r = r >> 2;
	}
    r = ac;
	if(one_cnt > 5 || zero_cnt > 5){
        u8 comp_val = (one_cnt > 5)?0x01:0x02;
	    u8 convert_val = (one_cnt > 5)?0x02:0x01;
        u8 off_set = 0;
        u32 mask_val[4] = {0x3FFFFFFF, 0xCFFFFFFF, 0xF3FFFFFF, 0xFCFFFFFF};
        foreach(i, 4){
            off_set = 32 - (i+1)*2;    
            if(((r>>off_set) & 0x03) == comp_val){
                r = (convert_val<<off_set)|(r&mask_val[i]);
            }
        }
	}

	return r;
}

static inline u16 light_proc_conflict_ac_16(u16 ac)
{
	u16 r = ac;
	u8 zero_cnt = 0;
	u8 one_cnt = 0;
	foreach(i, 16){
		if(ac & BIT(i)){
		    ++one_cnt;
		}else{
		    ++zero_cnt;
		}
	}
	if(one_cnt < 3 || zero_cnt < 3){
	    r ^= 0xFF00;
	}
	return r;
}

static inline u32 rf_access_code_16to32 (u16 code)
{
    u16 conver_code = light_proc_conflict_ac_16(code);
	u32 r = 0;
	for (int i=0; i<16; i++) {
		r = r << 2;
		r |= conver_code & BIT(i) ? 1 : 2;
	}
	return r;
}

static inline void rf_stop_trx (void)
{
	write_reg8  (0x800f00, 0x80);			// stop
}

static inline void rf_reset_sn (void)
{
	write_reg8  (0x800f01, 0x3f);
	write_reg8  (0x800f01, 0x00);
}

extern u8 emi_var[];
void 	rf_set_power_level_index (int level);

static inline void emi_init(  u8 tx_power_emi ){
    emi_var[0] = analog_read(0xa5);
    emi_var[1] = read_reg8(0x8004e8);
    //emi_var[2] = read_reg8(0x800524);
    //emi_var3 = read_reg8(0x800402);
    emi_var[4] = read_reg8(0x80050f);
    emi_var[5] = read_reg8(0x80050e);
    emi_var[6]  = read_reg8(0x800400);

    //8366 must enable, 8266 not verify
	//rf_power_enable (1);
	
    rf_set_power_level_index (tx_power_emi);
}

static inline void emi_carrier_init( void ){
    write_reg8 (0x800f02, 0x45);  // reset tx/rx enable reset
}

static inline void emi_cd_init( u32 cd_fifo){
	//reset zb & dma
	write_reg16(0x800060, 0x0480);
	write_reg16(0x800060, 0x0000);

	//TX mode
	//write_reg8 (0x800400,0x0b);//0b for 2Mbps, 03 for Zigbee, 0f for New2M and BLE Nrd 1Mbps
	//write_reg8 (0x800408,0x00);//0 for random , 1 for 0xf0f0, 2 fro 0x5555

	//txsetting
	//write_reg8(0x800402, 0x21);	//preamble length=1

	//txdma( &fifo_emi.start, FIFO_DEPTH );
    write_reg8(0x80050c,  cd_fifo & 0xff );
	write_reg8(0x80050d, (cd_fifo >> 8) & 0xff );
	write_reg8(0x80050e, (*(unsigned int*)cd_fifo -1) >> 4 );   //reg_50e = TX_buffer_size /16
	write_reg8(0x80050f, *(unsigned int*)cd_fifo -1 );

	//txpktsend
	write_reg8(0x800524, 0x08);
}

//as zhongqi's suggestion
static inline void emi_carrier_generate( void ){
    //write_reg8 (0x800400, 0x6f);//[6:5] 11: send 1, 10: send 0
    //tx_cyc1 manual
    analog_write (0xa5, 0x44);
    write_reg8 (0x8004e8, 0x04);
}

static inline void emi_carrier_recovery( void ){
    analog_write (0xa5, emi_var[0]);
    write_reg8 (0x8004e8, emi_var[1]);
}

static inline void emi_cd_recovery( void ){
    write_reg8(0x800524, 0);
    //write_reg8(0x800402, emi_var3);

    write_reg8(0x80050f, emi_var[4]);
    write_reg8(0x80050e, emi_var[5]);
    write_reg8(0x800400, emi_var[6]);
}


static inline void rf_start_brx  (void* addr, u32 tick)
{
//	write_reg32 (0x800f04, 0x56);						// tx_wait = 0; tx_settle = 86 us
	write_reg32 (0x800f28, 0x0fffffff);					// first timeout
	write_reg32(0x800f18, tick);						// Setting schedule trigger time
    write_reg8(0x800f16, read_reg8(0x800f16) | 0x04);	// Enable cmd_schedule mode
	write_reg8 (0x800f00, 0x82);						// ble rx
	write_reg16 (0x80050c, (u16)((u32)addr));
}


void 	rf_drv_init (int xtal_type);
void	rf_power_down ();
void	rf_set_channel (signed char chn, unsigned short set);
void 	rf_set_tx_rx_off (void);
void	rf_set_rxmode (void);
void	rf_set_txmode (void);
void	rf_send_packet (void* addr, u16 rx_waittime, u8 retry);
void 	rf_multi_receiving_init (u8 channel_mask);
void	rf_multi_receiving_start  (signed char chn, unsigned short set);
void	rf_set_ack_packet  (void* addr);
void 	rf_receiving_pipe_enble(u8 channel_mask);

void 	rf_update_tp_value (u8 tp0, u8 tp1);


#ifdef		RF_MODE_250K
#define		RF_FAST_MODE_2M		0
#define		RF_FAST_MODE_1M		0
#endif

#ifndef		RF_FAST_MODE_1M
#define		RF_FAST_MODE_1M		1
#endif

#ifndef		RF_FAST_MODE_2M
#define		RF_FAST_MODE_2M		(!RF_FAST_MODE_1M)
#endif

#ifndef		RF_LONG_PACKET_EN
#define		RF_LONG_PACKET_EN		0
#endif

#if			RF_FAST_MODE_2M
	#if			RF_LONG_PACKET_EN
		#define		RF_PACKET_LENGTH_OK(p)		(p[0] == p[12]+13)
		#define		RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x40)
	#else
		#define		RF_PACKET_LENGTH_OK(p)		(p[0] == (p[12]&0x3f)+15)
		#define		RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x40)
	#endif
#elif		RF_FAST_MODE_1M
#define		RF_PACKET_LENGTH_OK(p)		(p[0] == (p[13]&0x3f)+17)
#define		RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x40)
#else
#define		RF_PACKET_LENGTH_OK(p)		(p[0] == p[12]+13)
#define		RF_PACKET_CRC_OK(p)			((p[p[0]+3] & 0x51) == 0x10)
#endif

#if (RF_FAST_MODE_2M)
	#define			RF_FAST_MODE			1
	#define			RF_TRX_MODE				0x80
	#define			RF_TRX_OFF				0x44		//f02
#elif (RF_FAST_MODE_1M)
	#define			RF_FAST_MODE			1
	#define			RF_TRX_MODE				0x80
	#define			RF_TRX_OFF				0x45		//f02
#else
	#define			RF_FAST_MODE			0
	#define			RF_TRX_MODE				0xe0
	#define			RF_TRX_OFF				0x45		//f02
#endif

#define             RF_TRX_OFF_MANUAL       0x55        //f02

#define				STOP_RF_STATE_MACHINE	( REG_ADDR8(0xf00) = 0x80 )

static inline void rf_ble_tx_on ()
{
	write_reg8  (0x800f02, RF_TRX_OFF | BIT(4));	// TX enable
	write_reg32 (0x800f04, 0x38);
}

static inline void rf_ble_tx_done ()
{
	write_reg8  (0x800f02, RF_TRX_OFF);	// TX enable
	write_reg32 (0x800f04, 0x50);
}

static inline void rf_ble_trx_off ()
{
	/////////////////// turn on LDO and baseband PLL ////////////////
	//analog_write (0x06, 0xfe);
	write_reg8 (0x800f16, 0x29);
	write_reg8 (0x800428, RF_TRX_MODE);	// rx disable
	write_reg8 (0x800f02, RF_TRX_OFF);	// reset tx/rx state machine
}

static inline void emi_cd_prepare( void ){
    write_reg8 (0x800f02, RF_TRX_OFF);  //must trx disable before cd switch
}

static inline void rf_ble_stx  (void* addr, u32 tick)
{
	//write_reg32 (0x800f04, 0);						// tx wail & settle time: 0
	write_reg32(0x800f18, tick);						// Setting schedule trigger time
    write_reg8(0x800f16, read_reg8(0x800f16) | 0x04);	// Enable cmd_schedule mode
	write_reg8 (0x800f00, 0x87);						// single TX2RX
	write_reg16 (0x80050c, (u16)((u32)addr));
}

static inline void rf_set_gain_manualMax (void)
{
	write_reg8 (0x800433, 0x00);
	write_reg8 (0x800434, 0x01);
	write_reg8 (0x80043a, 0x77);
	write_reg8 (0x80043e, 0xc9);
	write_reg8 (0x8004cd, 0x06);
}

static inline void rf_set_AGC (void)
{
	write_reg8 (0x800433, 0x01);
	write_reg8 (0x800434, 0x21);
	write_reg8 (0x80043a, 0x22);
	write_reg8 (0x80043e, 0x20);
	write_reg8 (0x8004cd, 0x04);
}

static inline void rf_set_12M_Crystal_1m_mode(void)
{
	write_reg8 (0x8004eb, 0xe0);
	analog_write(0x99, 0xb1);
	analog_write(0x82, 0x20);
	analog_write(0x9e, 0x56);
}


static inline void rf_set_12M_Crystal_2m_mode(void)
{
	write_reg8 (0x8004eb, 0xe0);
	analog_write(0x99, 0xb1);
	analog_write(0x82, 0x20);
	analog_write(0x9e, 0xad);
}

static inline void rf_set_16M_Crystal_1m_mode(void)
{
	write_reg8 (0x8004eb, 0x60);
	analog_write(0x99, 0x31);
	analog_write(0x82, 0x34);
	analog_write(0x9e, 0x41);
}


static inline void rf_set_16M_Crystal_2m_mode(void)
{
	write_reg8 (0x8004eb, 0x60);
	analog_write(0x99, 0x31);
	analog_write(0x82, 0x34);
	analog_write(0x9e, 0x82);
}

#endif
