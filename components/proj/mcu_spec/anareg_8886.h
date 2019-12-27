/********************************************************************************************************
 * @file     anareg_8886.h 
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

#pragma once
#include "../common/types.h"
#include "../common/bit.h"

//// analog registers respectively ///////////////
#define rega_xtal_ctrl			0x01
enum {
	FLDA_XTAL_CS =				BIT_RNG(0,4),
	FLDA_LDO_TRIM_3V = 			BIT_RNG(5,6),
	FLDA_XTAL_COM_REF = 		BIT(7),
};

enum{
	ANALOG_LDO_VOL_1P6 = 0, 	// 1.6v
	ANALOG_LDO_VOL_1P8 = 2,		// 1.8v
};

#define rega_vol_ldo_ctrl		0x02
enum {
	FLDA_VOL_LDO_RESV = 		BIT_RNG(0,2),
	FLDA_VOL_LDO_OUTPUT = 		BIT_RNG(3,4),
	FLDA_VOL_BAND_GAP = 		BIT_RNG(5,7),
};

#define rega_pwdn_setting1		0x05
enum {
	FLDA_32K_RC_PWDN = 			BIT(0),
	FLDA_32K_XTAL_PWDN =		BIT(1),
	FLDA_32M_RC_PWDN = 			BIT(2),
	FLDA_32M_XTAL_PWDN = 		BIT(3),
	FLDA_LDO_PWDN = 			BIT(4),
	FLDA_BGIREF_3V_PWDN = 		BIT(5),
	FLDA_COMP_PWDN = 			BIT(6),
	FLDA_TEMPSEN_PWDN = 		BIT(7),
};

#define rega_pwdn_ldo			0x06
enum {
	FLDA_RX_LO_LDO_PWDN = 		BIT(0),
	FLDA_BG_I_EXT_PWDN =		BIT(1),
	FLDA_RX_ANA_LDO_PWDN = 		BIT(2),
	FLDA_RX_RF_LDO_PWDN = 		BIT(3),
	FLDA_PLL_BG_PWDN = 			BIT(4),
	FLDA_PLL_DS_LDO_PWDN = 		BIT(5),
	FLDA_PLL_LDO_PWDN = 		BIT(6),
	FLDA_PLL_LDO2_PWDN = 		BIT(7),
};

#define rega_ldo_setting1		0x07
enum {
	FLDA_RX_ANA_LDO_O_CTRL = 	BIT_RNG(0,1),
	FLDA_RX_RF_LDO_O_CTRL = 	BIT_RNG(2,3),
	FLDA_PLL_LDO_B2 = 			BIT_RNG(4,5),
	FLDA_PLL_LDO_DS = 			BIT_RNG(6,7),
};

#define rega_ldo_setting2		0x0c
enum {
	FLDA_RX_LOLDO_OUTCTRL = 	BIT_RNG(0,1),
	FLDA_LDO_ANA_LDOTRIM = 		BIT(2),
	FLDA_XTAL_LDO_TRIM = 		BIT(3),
	FLDA_PULLDN_LOLDO =			BIT(4),
	FLDA_PULLDN_DIGLDO = 		BIT(5),	// digit  LDO
	FLDA_LDO_SETTING2_RSV =		BIT_RNG(6,7),
};

#define rega_32k_tm_thre0		0x10
#define rega_32k_tm_thre1		0x11
#define rega_32k_tm_thre2		0x12
#define rega_32k_tm_thre3		0x13
enum {
	FLDA_32K_THR3_H = 			BIT_RNG(0,1),		// conflict with rega_32k_tm_val3 in document !!!
	FLDA_32K_CONTINUE_MODE =	BIT(2),				// 0: one shot,  1: continuing
	FLDA_32K_TIMER_EN =			BIT(3),
	FLDA_DELAY_AFTER_WAKEUP =	BIT_RNG(4,7),
};
#define rega_delay_after_wakeup	0x14

#define rega_wakeup_pin_l		0x15
#define rega_wakeup_pin_h		0x16
#define rega_pm_ctrl			0x17
enum {
	FLDA_PM_WAKEUP_FROM_DIG = 	BIT(0),				// voltage
	FLDA_PM_WAKEUP_32K_SEL = 	BIT(1),				// conflict with the document !!!
	FLDA_PM_RESV0 = 			BIT_RNG(2,3),
	FLDA_PM_WAKEUP_POL = 		BIT(4),
	FLDA_PM_RESV1 =				BIT(5),
	FLDA_PM_PWDN_LDO = 			BIT(6),
	FLDA_PM_PWDN_EN = 			BIT(7),
};

#define rega_pwdn_setting2		0x18
enum {
	FLDA_PWDN_OSC_32K_EN = 		BIT(0),
#if(MCU_CORE_TYPE == MCU_CORE_5320)
	FLDA_PWDN_PAD_32K_EN =		BIT(1),
#else
	FLDA_RESET_32K_TIMER =		BIT(1),
#endif
	FLDA_PWDN_TX = 				BIT(2),
	FLDA_PWDN_RX = 				BIT(3),
	FLDA_PWDN_SAR_ADC = 		BIT(4),
	FLDA_PWDN_LOW_LEAK_LDO = 	BIT(5),
	FLDA_PWDN_REG456_EN = 		BIT(6),
	FLDA_PWDN_ISO_EN = 			BIT(7),
};

#define rega_32k_tm_val0		0x20
#define rega_32k_tm_val1		0x21
#define rega_32k_tm_val2		0x22
#define rega_32k_tm_val3		0x23
enum{
	FLDA_32K_CNT_CONTINUOUS = BIT(3),
	FLDA_32K_RST_TIMER = BIT(4),
	FLDA_32K_TRIGGER_TIMER = BIT(7),
};
//#define raga_gpio_wkup_pol 		0x24
enum{
	FLDA_WKUP_POL_PAD0 = BIT(0),
	FLDA_WKUP_POL_PAD1 = BIT(1),
	FLDA_WKUP_POL_PAD2 = BIT(2),
	FLDA_WKUP_POL_PAD3 = BIT(3),
	FLDA_WKUP_POL_PAD4 = BIT(4),
	FLDA_WKUP_POL_PAD5 = BIT(5),
};
//#define rega_wakeup_en_val0  0x27
//#define rega_wakeup_en_val1  0x28
//#define rega_wakeup_en_val2  0x29

// from 0x2c, 0x2d, 0x2e, 0x2f, 0x30, fro customer use
//#define rega_pkt_seq_no			0x2d
//#define rega_deepsleep_rf_chn	0x2e
//#define rega_deepsleep_flag		0x2f

#define rega_xtal_csel			0x81
enum {
	FLDA_XTAL_FREQ_OFF = 		BIT_RNG(0, 4),
	FLDA_XTAL_PWDN = 			BIT(5),
	FLDA_XTAL_VOLT = 			BIT_RNG(6,7),
};

#define rega_bp_pll				0x84
enum {
	FLDA_BP_PLL_CP_PWDN = 		BIT(0),
	FLDA_BP_PLL_3ST_VCO_PWDN = 	BIT(1),
	FLDA_BP_PLL_4ST_VCO_PWDN = 	BIT(2),
	FLDA_BP_PLL_VCO_SEL = 		BIT(3),
	FLDA_BP_PLL_R_DIV = 		BIT_RNG(6,7),
};

#define rega_aud_ctrl			0x86
enum {
	FLDA_AUD_PWDN_LEFT = 		BIT(0),
	FLDA_AUD_PWDN_RIGHT = 		BIT(1),
	FLDA_AUD_MUTE_RIGHT = 		BIT(2),
	FLDA_AUD_MUTE_LEFT = 		BIT(3),
	FLDA_AUD_PRE_GAIN_RIGHT = 	BIT_RNG(4,5),
	FLDA_AUD_PRE_GAIN_LEFT = 	BIT_RNG(6,7),
};

#define rega_aud_ctrl2			0x87
enum {
	FLDA_AUD_PST_GAIN_RIGHT = 	BIT_RNG(0,3),
	FLDA_AUD_PST_GAIN_LEFT = 	BIT_RNG(4,7),
};

#define rega_dcdc_ctrl			0x88
enum {
	FLDA_DCDC_RESV = 			BIT(0),
	FLDA_RNS_PWDN = 			BIT(1),
	FLDA_DCDC_RESETB = 			BIT(2),
	FLDA_DCDC_6P75_PWDN = 		BIT(3),
	FLDA_DCDC_IN_2P8_PS = 		BIT(4),
	FLDA_DCDC_OUT_2P8_SEL = 	BIT(5),
	FLDA_DCDC_OUT_6P6_SEL = 	BIT(6),
	FLDA_DCDC_2P8_PWDN = 		BIT(7),
};

#define rega_dig_ldo_sel_atb	0x89
enum {
	FLDA_LDO_SEL_ATB = 			BIT_RNG(0,2),
	FLDA_PWDN_CLK_BB = 			BIT(3),
	FLDA_PWDN_CLK_RF = 			BIT(4),
	FLDA_PWDN_BUF_RCCAL = 		BIT(5),
	FLDA_PWDN_BUF_DIG = 		BIT(6),
};
#define rega_adc_setting1		0x8a
#define rega_adc_setting2		0x8b
enum{
	FLDA_RX_ADC_COMPLEX_EN = 	BIT(0),
	FLDA_RX_ADC_CTUNE = 		BIT_RNG(1,4),
	FLDA_RX_ADC_DLY_CTRL =		BIT_RNG(5,6),
	FLDA_RX_ADC_PWDN = 			BIT(7),
};

#define rega_rx_lna				0x91
enum{
	FLDA_RX_LNA_OUT_TUNE = 		BIT_RNG(0,2),
	FLDA_RX_LNA_BIAS =			BIT_RNG(3,5),
	FLDA_RX_LNA_ON = 			BIT(6),
	FLDA_PA_TX_EXT_C1_EN = 		BIT(7),
};

#define rega_rx_pga				0x94
enum{
	FLDA_RX_PGA_IQ_SWAP_EN = 	BIT(0),
	FLDA_RX_PGA_BW_TUNE = 		BIT(1),
	FLDA_RX_PGA_CAP_CTRL = 		BIT_RNG(2,5),
	FLDA_RX_PGA_COMP_EN = 		BIT(6),
	FLDA_RX_PGA_PWDN = 			BIT(7),
};
#define rega_rx_test			0x99
enum{
	FLDA_RF_PEAK_DET_DLY1_EN = 	BIT(0),
	FLDA_RF_PEAK_DET_DLY2_EN =	BIT(1),
	FLDA_RF_PEAK_DET_DLY3_EN = 	BIT(2),
	FLDA_TEST_PGA_OUT_EN = 		BIT(3),
	FLDA_TEST_PGA_IN_EN = 		BIT(4),
	FLDA_TEST_IQ_MIXER_EN = 	BIT(5),
	FLDA_RF_PEAK_DET1_EN = 		BIT(6),
	FLDA_RF_PEAK_DET2_EN = 		BIT(7),
};

#define rega_tx_pa_ctrl			0x9e
enum{
	FLDA_PA_TX_BIAS = 			BIT_RNG(0,4),
	FLDA_PA_TX_UNIT = 			BIT_RNG(3,5),
};
#define rega_tx_pa_ctrl2		0x9f

#define rega_pfd				0xa5
enum{
	FLDA_PFD_MUX_L = 			BIT_RNG(0,2),
	FLDA_PFD_NP_INTG = 			BIT(3),
	FLDA_INTG_PFD_S0 = 			BIT(4),
	FLDA_INTG_PFD_S1 = 			BIT(5),
	FLDA_REFBUF_PWDN = 			BIT(6),
	FLDA_REG_DC_MOD0 = 			BIT(7),
};

#define rega_intg_n0			0xa0
enum{
	FLDA_DPLL_EN = 				BIT(0),
	FLDA_IDIV2_UP_RX = 			BIT(1),
	FLDA_IDIV2_UP_TX =			BIT(2),
	FLDA_RX_INTG_N0 = 			BIT_RNG(3,7),
};

#define rega_intg_n1			0xa1
enum{
	FLDA_RX_INTG_N1 = 			BIT_RNG(0,4),
	FLDA_RX_IB1 = 				BIT_RNG(5,6),
	FLDA_LPF_PWDN = 			BIT(7),
};

#define rega_intg_rdiv			0xa2
enum{
	FLDA_INTG_RDIV = 			BIT_RNG(0,5),
	FLDA_MASH2_EN = 			BIT(6),
	FLDA_NDIV_EN = 				BIT(7),
};

#define rega_reg_dc_mod1		0xa6
#define rega_reg_dc_mod2		0xa7
#define rega_vco_cal_comp		0xa8
enum{
	FLDA_RF_TEST_PWDN = 		BIT(0),
	FLDA_RX_IB = 				BIT_RNG(1,2),
	FLDA_VCO_CAL_COMP = 		BIT_RNG(3,7),
};

#define rega_vco_setting		0xa9
enum{
	FLDA_TX_IB = 				BIT_RNG(0,1),
	FLDA_VCO_RXI = 				BIT_RNG(2,4),
	FLDA_VCO_TXI = 				BIT_RNG(5,7),
};

#define rega_intg_rxcap			0xaa
enum{
	FLDA_PWDN_INTG_PLL = 		BIT(0),
	FLDA_INTG_RXCAP = 			BIT_RNG(1,7),
};

#define rega_intg_txcap			0xab
#define rega_pll_dpll			0xac
#define rega_pll_dpll_setting	0xad
enum{
	FLDA_PLL_DPLL = 			BIT_RNG(0,1),
	FLDA_PLL_DPLL_ADJ = 		BIT_RNG(2,4),
	FLDA_PLL_DPLL_REF = 		BIT_RNG(5,6),
	FLDA_TXCALDIV2BUF_PWDN = 	BIT(7),

};

#define rega_pll_dpll_wait		0xae
enum{
	FLDA_PLL_DPLL_WAIT = 		BIT_RNG(0,5),
	FLDA_RX_MAN_EN =			BIT(6),
	FLDA_TX_MAN_EN =			BIT(7),
};

