/********************************************************************************************************
 * @file     ble_smp.h 
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

#ifndef BLE_SMP_H_
#define BLE_SMP_H_

#include "ble_common.h"
#if 1
#include "stack/ble/smp/smp_const.h"
#else


#define 		BOND_DEVICE_WHITELIST_MANAGEMANT_ENABLE		1

#define 		SMP_BONDING_DEVICE_MAX_NUM					4



#if (LL_MASTER_MULTI_CONNECTION || LL_MASTER_SINGLE_CONNECTION)
	#define 		SMP_SLAVE_SAVE_PERR_LTK_ENABLE				1
#else
	#define 		SMP_SLAVE_SAVE_PERR_LTK_ENABLE				0
#endif


#define 		SMP_SAVE_PEER_CSRK_ENABLE					0

#if (SMP_SLAVE_SAVE_PERR_LTK_ENABLE)
	#define 		SMP_PARAM_NV_UNIT						96

	#define 		SMP_PARAM_INIT_CLEAR_MAGIN_ADDR 		3072
	#define 		SMP_PARAM_LOOP_CLEAR_MAGIN_ADDR 		3584

#else
	#define 		SMP_PARAM_NV_UNIT						64

	#define 		SMP_PARAM_INIT_CLEAR_MAGIN_ADDR 		3072  //64 * 48
	#define 		SMP_PARAM_LOOP_CLEAR_MAGIN_ADDR 		3520  //64 * 55 (56 device most)
#endif


#define			SMP_PARAM_NV_MAX_LEN						4096


extern 			int				SMP_PARAM_NV_ADDR_START;

#define			SMP_PARAM_NV_SEC_ADDR_START				(SMP_PARAM_NV_ADDR_START + SMP_PARAM_NV_MAX_LEN)
#define			SMP_PARAM_NV_SEC_ADDR_END				(SMP_PARAM_NV_SEC_ADDR_START + SMP_PARAM_NV_MAX_LEN - 1)





#define		FLAG_SMP_PARAM_SAVE_PENDING				0x7B  // 0111 1011
#define		FLAG_SMP_PARAM_SAVE_OK					0x5A  // 0101 1010
#define		FLAG_SMP_PARAM_SAVE_ERASE				0x00  //


#define		FLAG_SMP_SECTOR_USE						0x3C
#define		FLAG_SMP_SECTOR_CLEAR					0x00

#define     FLASH_SECTOR_OFFSET						4080

#define		TYPE_WHITELIST							BIT(0)
#define		TYPE_RESOLVINGLIST						BIT(1)





#define SMP_STANDARD_PAIR   	0
#define SMP_FAST_CONNECT   		1




typedef union {
	struct{
		u8 bondingFlag : 2;
		u8 MITM : 1;
		u8 SC	: 1;
		u8 keyPress: 1;
		u8 rsvd: 3;
	};
	u8 authType;
}smp_authReq_t;

typedef union{
	struct {
		u32 encKey : 1;
		u32 idKey : 1;
		u32 sign  : 1;
		u32 linkKey : 4;
	};
	u8 keyIni;
}smp_keyDistribution_t;

typedef struct{
	u8 code;  //req = 0x01; rsp = 0x02;
	u8 ioCapablity;
	u8 oobDataFlag;
	smp_authReq_t authReq;
	u8 maxEncrySize;

	smp_keyDistribution_t initKeyDistribution;
	smp_keyDistribution_t rspKeyDistribution;
}smp_paring_req_rsp_t;



typedef struct{
	u8  code; //0x04
	u16 randomValue[16];
}smp_paring_random_t;

typedef struct{
	u8  code; //0x03
	u16 confirmValue[16];
}smp_paring_confirm_t;

typedef struct{
	u8 code; // 0x05
	u8 reason;
}smp_paring_failed_t;

typedef struct{
	u8 code;//0x0b

	smp_authReq_t authReq;
}smp_secure_req_t;

typedef struct{
	u8  code;//0xa
	u8 	signalKey[16];
}smp_signal_info_t;

typedef struct{
	u8 code;//0x9
	u8 addrType;
	u8 bdAddr[6];
}smp_id_addr_info_t;

typedef struct{
	u8 code;//0x8
	u8 idResolveKey[16];
}smp_id_info_t;

typedef struct{
	u8  code;//0x7
	u16 edivPtr[2];
	u8 	masterRand[8];
}smp_master_id_t;

typedef struct{
	u8  code;//0x6
	u8 	LTK[16];
}smp_enc_info_t;

// -------add core 4.2 ------
typedef struct{
	u8  code;//0xc
	u8  publicKeyX[32];
	u8  publicKeyY[32];
}smp_paring_public_key_t;

typedef struct{
	u8  code;//0xd
	u8  DHKeyCheck[16];
}smp_DHkey_check_t;

typedef struct{
	u8  code;//0xe
	u8  notifyType;
}smp_key_notify_t;

typedef struct{
	u8 sc_sk_dhk_own[32];  // keep sk before receive Ea. and keep dhkey after that.
	u8 sc_pk_own[64];
	u8 sc_pk_peer[64];
}smp_sc_key_t;



/*
 * smp parameter about peer device.
 * */
typedef struct{
	u8		paring_enable;
	u8 		peer_addr_type;  //address used in link layer connection
	u8		peer_addr[6];

	u8		peer_key_size;   // bond and key_size
	u8		peer_id_address_type;  //peer identity address information in key distribution, used to identify
	u8		peer_id_address[6];

	u8		peer_csrk[16];
	u8		peer_irk[16];
	u8		paring_peer_rand[16];

#if (SMP_SLAVE_SAVE_PERR_LTK_ENABLE)
	u16 	peer_ediv;
	u8		peer_random[8];
	u8		peer_ltk[16];
#endif

}smp_param_peer_t;


/*
 * smp parameter need save to flash.
 * */





typedef struct {  //82
	u8		flag;
	u8		peer_addr_type;  //address used in link layer connection
	u8		peer_addr[6];

	u8 		peer_key_size;
	u8		peer_id_adrType; //peer identity address information in key distribution, used to identify
	u8		peer_id_addr[6];

	u8 		own_ltk[16];      //own_ltk[16]
	u8		peer_irk[16];
	u8		peer_csrk[16];

#if (SMP_SLAVE_SAVE_PERR_LTK_ENABLE)
	u8		peer_ltk[16];
	u8		peer_random[8];
	u16		peer_ediv;
#endif



}smp_param_save_t;

/*
 * smp parameter about own device.
 * */
typedef struct{
	smp_paring_req_rsp_t  	paring_req;
	smp_paring_req_rsp_t  	paring_rsp;
	u8						own_conn_type;  //current connection peer own type
	u8						own_conn_addr[6];
	smp_authReq_t			auth_req;
	u8						paring_tk[16];   // in security connection to keep own random
	u8						paring_confirm[16];  // in security connection oob mode to keep peer random
	u8						own_ltk[16];   //used for generate ediv and random

	u8						save_key_flag;
}smp_param_own_t;

u8 cur_enc_keysize;

typedef struct {
	/* data */
	u8 csrk[16];
	u32 signCounter;
} smp_secSigInfo_t;



#define  ADDR_NOT_BONDED	0xFF
#define  ADDR_NEW_BONDED	0xFE
#define  ADDR_DELETE_BOND	0xFD

#define  KEY_FLAG_IDLE		0xFF
#define  KEY_FLAG_NEW		0xFE
#define  KEY_FLAG_FAIL		0xFD


typedef struct {
	u8 maxNum;
	u8 curNum;
	u8 addrIndex;
	u8 keyIndex;
	//u8 dev_wl_en;
	//u8 dev_wl_maxNum;  //device in whilteList max number

	u32 bond_flash_idx[SMP_BONDING_DEVICE_MAX_NUM];  //mark paired slave mac address in flash
} bond_device_t;





typedef void (*smp_check_handler_t)(u32);
typedef void (*smp_init_handler_t)(u8 *p);
typedef u8 * (*smp_info_handler_t)(void);
typedef void (*smp_bond_clean_handler_t)(void);
typedef int (*smp_enc_done_cb_t)(void);


extern smp_check_handler_t		func_smp_check; //HID on android 7.0
extern smp_init_handler_t		func_smp_init;
extern smp_info_handler_t		func_smp_info;
extern smp_bond_clean_handler_t  func_bond_check_clean;
extern smp_enc_done_cb_t		func_smp_enc_done_cb;



typedef enum {
	JUST_WORKS,
	PK_RESP_INPUT,  // Initiator displays PK, initiator inputs PK
	PK_INIT_INPUT,  // Responder displays PK, responder inputs PK
	OK_BOTH_INPUT,  // Only input on both, both input PK
	OOB             // OOB available on both sides
} stk_generationMethod_t;

// IO Capability Values
typedef enum {
	IO_CAPABILITY_DISPLAY_ONLY = 0,
	IO_CAPABILITY_DISPLAY_YES_NO,
	IO_CAPABILITY_KEYBOARD_ONLY,
	IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
	IO_CAPABILITY_KEYBOARD_DISPLAY, // not used by secure simple pairing
	IO_CAPABILITY_UNKNOWN = 0xff
} io_capability_t;

// horizontal: initiator capabilities
// vertial:    responder capabilities
static const stk_generationMethod_t stk_generation_method[5][5] = {
	{ JUST_WORKS,      JUST_WORKS,       PK_INIT_INPUT,   JUST_WORKS,    PK_INIT_INPUT },
	{ JUST_WORKS,      JUST_WORKS,       PK_INIT_INPUT,   JUST_WORKS,    PK_INIT_INPUT },
	{ PK_RESP_INPUT,   PK_RESP_INPUT,    OK_BOTH_INPUT,   JUST_WORKS,    PK_RESP_INPUT },
	{ JUST_WORKS,      JUST_WORKS,       JUST_WORKS,      JUST_WORKS,    JUST_WORKS    },
	{ PK_RESP_INPUT,   PK_RESP_INPUT,    PK_INIT_INPUT,   JUST_WORKS,    PK_RESP_INPUT },
};

#define IO_CAPABLITY_DISPLAY_ONLY		0x00
#define IO_CAPABLITY_DISPLAY_YESNO		0x01
#define IO_CAPABLITY_KEYBOARD_ONLY		0x02
#define IO_CAPABLITY_NO_IN_NO_OUT		0x03
#define IO_CAPABLITY_KEYBOARD_DISPLAY	0x04

#define PASSKEY_TYPE_ENTRY_STARTED		0x00
#define PASSKEY_TYPE_DIGIT_ENTERED		0x01
#define PASSKEY_TYPE_DIGIT_ERASED		0x02
#define PASSKEY_TYPE_CLEARED			0x03
#define PASSKEY_TYPE_ENTRY_COMPLETED	0x04

#define PARING_FAIL_REASON_PASSKEY_ENTRY			0x01
#define PARING_FAIL_REASON_OOB_NOT_AVAILABLE		0x02
#define PARING_FAIL_REASON_AUTH_REQUIRE				0x03
#define PARING_FAIL_REASON_CONFIRM_FAILED			0x04
#define PARING_FAIL_REASON_PARING_NOT_SUPPORTED		0x05
#define PARING_FAIL_REASON_ENCRYPT_KEY_SIZE			0x06
//-- core 4.2
#define PARING_FAIL_REASON_CMD_NOT_SUPPORT			0x07
#define PARING_FAIL_REASON_UNSPECIFIED_REASON		0x08
#define PARING_FAIL_REASON_REPEATED_ATTEMPT			0x09
#define PARING_FAIL_REASON_INVAILD_PARAMETER		0x0a
#define PARING_FAIL_REASON_DHKEY_CHECK_FAIL			0x0b
#define PARING_FAIL_REASON_NUMUERIC_FAILED			0x0c
#define PARING_FAIL_REASON_BREDR_PARING				0x0d
#define PARING_FAIL_REASON_CROSS_TRANSKEY_NOT_ALLOW		0x0e

#define	ENCRYPRION_KEY_SIZE_MAXINUM				16
#define	ENCRYPRION_KEY_SIZE_MINIMUN				7

typedef enum{
	SMP_PARING_DISABLE_TRRIGER = 0,
	SMP_PARING_CONN_TRRIGER ,
	SMP_PARING_PEER_TRRIGER,
}smp_paringTrriger_t;














/////////////////////////////////////////////////////////////////////////////////////////////////
//								SLAVE
/////////////////////////////////////////////////////////////////////////////////////////////////


/******************************* User Interface  ************************************/


/**************************************************
 * API used for slave enable the device paring.
 * encrypt_en	SMP_PARING_DISABLE_TRRIGER   -  not allow encryption
 * 				SMP_PARING_CONN_TRRIGER      -  paring process start once connect.
 * 				SMP_PARING_PEER_TRRIGER      -  paring process start once peer device start.
 */
int 		bls_smp_enableParing (smp_paringTrriger_t encrypt_en);




void 		bls_smp_configParingSecurityInfoStorageAddr (int addr);

ble_sts_t  	blc_smp_param_setBondingDeviceMaxNumber ( int device_num);

u8			blc_smp_param_getCurrentBondingDeviceNumber(void);


u32 		blc_smp_param_loadByIndex(u8 index, smp_param_save_t* smp_param_load);

u32			blc_smp_param_loadByAddr(u8 addr_type, u8* addr, smp_param_save_t* smp_param_load);




/*************************************************
 * 	used for enable oob flag
 */
void blc_smp_enableOobFlag (int en, u8 *oobData);

/*************************************************
 * 	used for set MAX key size
 * */
void blc_smp_setMaxKeySize (u8 maxKeySize);

/*************************************************
 * 	@brief 		used for enable authentication MITM
 * 	@return  	0 - setting success
 * 				others - pin code not in ranged.(0 ~ 999,999)
 */
int blc_smp_enableAuthMITM (int en, u32 pinCodeInput);

/*************************************************
 * 	@brief 		used for enable authentication bonding flag.
 */
int blc_smp_enableBonding (int en);

/*************************************************
 * 	used for set IO capability
 * */
void blc_smp_setIoCapability (u8 ioCapablility);

/*
 * API used for set distribute key enable.
 * */
smp_keyDistribution_t blc_smp_setDistributeKey (u8 LTK_distributeEn, u8 IRK_distributeEn, u8 CSRK_DistributeEn);

/*
 * API used for set distribute key enable.
 * */
smp_keyDistribution_t blc_smp_expectDistributeKey (u8 LTK_distributeEn, u8 IRK_distributeEn, u8 CSRK_DistributeEn);







/************************* Stack Interface, user can not use!!! ***************************/

/*
 * Return STK generate method.
 * */
int blc_smp_getGenMethod ();

/**************************************************
 * 	used for handle link layer callback (ltk event callback), packet LL_ENC_request .
 */
int bls_smp_getLtkReq (u16 connHandle, u8 * random, u16 ediv);

/*
 * Used for set smp parameter to default.
 * */
void blc_smp_paramInitDefault ( );



/*************************************************
 * 	@brief 		used for reset smp param to default value.
 */
int blc_smp_paramInit ();



/**************************************************
 *  API used for slave start encryption.
 */
int bls_smp_startEncryption ();










typedef struct {
	u8	secReq_pending;
	u8	secReq_laterSend;
	u16  rsvd;
} smp_ctrl_t;

extern smp_ctrl_t 	blc_smp_ctrl;

void blc_smp_checkSecurityReqeustSending(u32 connStart_tick);
void HID_service_on_android7p0_init(void);

















/////////////////////////////////////////////////////////////////////////////////////////////////
//	MATER

/////////////////////////////////////////////////////////////////////////////////////////////////


typedef int (*smp_finish_callback_t)(void);
void blm_smp_registerSmpFinishCb (smp_finish_callback_t cb);




//  6 byte slave_MAC   8 byte rand  2 byte ediv
// 16 byte ltk
#define PAIR_INFO_SECTOR_SIZE	 				64

#define PAIR_OFFSET_SLAVE_MAC	 				2

#define PAIR_OFFSET_RAND		 				8
#define PAIR_OFFSET_EDIV		 				16
#define PAIR_OFFSET_ATT			 				18   //ATT handle
#define PAIR_OFFSET_LTK			 				32
#define PAIR_OFFSET_IRK			 				48

#if (LL_MASTER_MULTI_CONNECTION)
	#define	PAIR_SLAVE_MAX_NUM            			8
#else
	#define	PAIR_SLAVE_MAX_NUM            			1
#endif

typedef struct {
	u8 bond_mark;
	u8 adr_type;
	u8 address[6];
} mac_adr_t;


#define FlAG_BOND				BIT(0)
#define FLAG_FASTSMP			BIT(4)

typedef struct {
	u8 curNum;
	u8 curIndex;
	u8 isBond_fastSmp;
	u8 rsvd;  //auto smp, no need SEC_REQ
	u32 bond_flash_idx[PAIR_SLAVE_MAX_NUM];  //mark paired slave mac address in flash
	mac_adr_t bond_device[PAIR_SLAVE_MAX_NUM];
} bond_slave_t;




#define SLAVE_TRIGGER_SMP_FIRST_PAIRING				0   	//first pair, slave send security_request to trigger master's pairing&encryption
#define MASTER_TRIGGER_SMP_FIRST_PAIRING			BIT(0)

#define SLAVE_TRIGGER_SMP_AUTO_CONNECT				0   	//auto connect, slave send security_request to trigger master's encryption
#define MASTER_TRIGGER_SMP_AUTO_CONNECT				BIT(1)


void	blm_host_smp_init (u32 adr);

void    blm_host_smp_setSecurityTrigger(u8 trigger);
u8		blm_host_smp_getSecurityTrigger(void);
void 	blm_host_smp_procSecurityTrigger(u16 connHandle);

void 	blm_host_smp_handler(u16 conn_handle, u8 *p);
int 	tbl_bond_slave_search(u8 adr_type, u8 * addr);
int 	tbl_bond_slave_delete_by_adr(u8 adr_type, u8 *addr);
void 	tbl_bond_slave_unpair_proc(u8 adr_type, u8 *addr);

void	blm_smp_encChangeEvt(u8 status, u16 connhandle, u8 enc_enable);
#endif




#endif /* BLE_SMP_H_ */
