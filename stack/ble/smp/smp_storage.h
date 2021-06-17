/********************************************************************************************************
 * @file     smp_storage.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     Sep. 18, 2015
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
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
/*
 * smp_storage.h
 *
 *  Created on: 2018-12-4
 *      Author: Administrator
 */

#ifndef SMP_STORAGE_H_
#define SMP_STORAGE_H_



#if (SIMPLE_MULTI_MAC_EN)
	#include "smp.h"
	extern u8 device_mac_index;
#endif


#define 	SMP_PARAM_NV_UNIT						64

#define 	SMP_PARAM_INIT_CLEAR_MAGIN_ADDR 		3072  //64 * 48
#define 	SMP_PARAM_LOOP_CLEAR_MAGIN_ADDR 		3520  //64 * 55 (56 device most)



#define		SMP_PARAM_NV_MAX_LEN					4096


extern 		int	 SMP_PARAM_NV_ADDR_START;

#define		SMP_PARAM_NV_SEC_ADDR_START				(SMP_PARAM_NV_ADDR_START + SMP_PARAM_NV_MAX_LEN)
#define		SMP_PARAM_NV_SEC_ADDR_END				(SMP_PARAM_NV_SEC_ADDR_START + SMP_PARAM_NV_MAX_LEN - 1)




#define		NEW_FLASH_STORAGE_KEY_ENABLE			0   //be compatible with old flash storage methods


#define		FLAG_SMP_PARAM_SAVE_OLD					0x5A  // 0101 1010  old storage

#if (SIMPLE_MULTI_MAC_EN)
														  // 10xx 1YYY  new storage,  xx: see "paring_sts_t" definition YYY:device_mac_index default
#define		FLAG_SMP_PARAM_SAVE_BASE				(0x88+(device_mac_index&0x07))//0x8A  // 1000 1010
#else
														  // 10xx 101y  new storage,  xx: see "paring_sts_t" definition
#define		FLAG_SMP_PARAM_SAVE_BASE				0x8B  // 1000 1011
#endif
#define		FLAG_SMP_PARAM_SAVE_UNANTHEN			0x9B  // 1001 1011  new storage Unauthenticated_LTK
#define		FLAG_SMP_PARAM_SAVE_AUTHEN				0xAB  // 1010 1011  new storage Authenticated_LTK_Legacy_Paring
#define		FLAG_SMP_PARAM_SAVE_AUTHEN_SC			0xBB  // 1011 1011  new storage Authenticated_LTK_Secure_Connection

#define		FLAG_SMP_PARAM_SAVE_PENDING				0xBF  // 1011 1111
#define		FLAG_SMP_PARAM_SAVE_ERASE				0x00  //

#if (SIMPLE_MULTI_MAC_EN)
#define 	FLAG_SMP_PARAM_MASK						0xC8
#define     FLAG_SMP_PARAM_VALID					0x88
#else
#define 	FLAG_SMP_PARAM_MASK						0x0E  // 0000 1110
#define     FLAG_SMP_PARAM_VALID					0x0A  // 0000 1010
#endif
#define 	FLAG_SMP_PARING_STATUS_MASK				0x30  // 0011 0000


#define		FLAG_SMP_SECTOR_USE						0x3C
#define		FLAG_SMP_SECTOR_CLEAR					0x00

#define     FLASH_SECTOR_OFFSET						4080  //0xFF0

#if (LL_FEATURE_ENABLE_LL_PRIVACY)
/*
 *  Address resolution is not supported by default. After pairing and binding, we need to obtain the central Address Resolution
 *  feature value of the opposite end to determine whether the opposite end supports the address resolution function, and write
 *  the result to smp_bonding_flg. Currently, we leave it to the user to obtain this feature.
 */
#define 	IS_PEER_ADDR_RES_SUPPORT(peerAddrResSuppFlg)	(!(peerAddrResSuppFlg &1))

#endif



extern int		bond_device_flash_cfg_idx;



typedef enum {
	Index_Update_by_Pairing_Order = 0,     //default value
	Index_Update_by_Connect_Order = 1,
} index_updateMethod_t;



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
	u8		local_irk[16];

}smp_param_save_t;





/******************************* User Interface  *****************************************/


u8			blc_smp_param_getCurrentBondingDeviceNumber(void);



void 		bls_smp_configParingSecurityInfoStorageAddr (int addr);


u32 		bls_smp_param_loadByIndex(u8 index, smp_param_save_t* smp_param_load);

u32			bls_smp_param_loadByAddr(u8 addr_type, u8* addr, smp_param_save_t* smp_param_load);


void		bls_smp_setIndexUpdateMethod(index_updateMethod_t method);


void		bls_smp_eraseAllParingInformation(void);

#if (LL_FEATURE_ENABLE_LL_PRIVACY)
void		blc_smp_setPeerAddrResSupportFlg(u32 flash_addr, u8 support);
#endif

/************************* Stack Interface, user can not use!!! ***************************/
int     bls_smp_param_saveBondingInfo (smp_param_save_t*);
void 	bls_smp_param_initFromFlash(void);
void	bls_smp_param_Cleanflash (void);
u32 	bls_smp_loadParamVsRand (u16 ediv, u8* random);
int     bls_smp_loadParamVsAddr (u16 addr_type, u8* addr);
u8		bls_smp_param_getIndexByFLashAddr(u32 flash_addr);


int		bls_smp_param_deleteByIndex(u8 index);
u8		bls_smp_param_getBondFlag_by_flashAddress(u32 flash_addr);

u32		bls_smp_searchBondingDevice_in_Flash_by_Address(u8 addr_type, u8* addr );
u32 	blc_smp_param_updateToNearestByIndex(u8 index);



#endif /* SMP_STORAGE_H_ */
