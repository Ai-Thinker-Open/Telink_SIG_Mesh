/*
 * smp_sm.h
 *
 *  Created on: 2019-6-17
 *      Author: Administrator
 */

#ifndef SMP_MS_H_
#define SMP_MS_H_

#include "stack/ble/smp/smp.h"
#include "stack/ble/smp/smp_const.h"
#include "stack/ble/smp/smp_storage.h"
#include "stack/ble/smp/smp_peripheral.h"
#include "stack/ble/smp/smp_central.h"


extern u8 restart_encryption;
extern u8 blt_smp_empty_pkt_entry;
extern smp_trigger_t blm_smpTrig;
extern smp_trigger_cb_t		func_smp_trigger;
extern blt_event_callback_t		blt_p_event_callback ;

extern bool smp_erase_flash_sector(u32 addr);


typedef struct{
	u8 trigger_mask;
	u8 smp_begin_flg;
	u8 manual_smp_start;
	u8 rsvd;
}smpm_trigger_t;

/*
 * smp parameter about peer device.
 * */
typedef struct{

	u8		peer_key_size;   // bond and key_size
	u8		peer_id_address_type;  //peer identity address information in key distribution, used to identify
	u8		peer_id_address[6];

	u8  	peer_rand[16];  //peer device RANDOM
	u8		peer_confirm[16];

	u8  	peer_ltk[16];   //peer device LTK
	u8		peer_csrk[16];
	u8		peer_irk[16];

	u16     rsvd0;
	u16 	peer_ediv;      //peer device EDIV
	u8		peer_random[8]; //peer device RANDOM

	u8      rsvd1;
	u8 		peer_addr_type;  //address used in link layer connection
	u8		peer_conn_addr[6];
}smp_ms_param_peer_t;

/*
 * smp parameter about own device.
 * */
typedef struct{

	u8 						own_rand[16];
	u8 						own_confirm[16];

	smp_authReq_t			auth_req;
	u8						own_addr_type;  //current connection peer own type
	u8						own_conn_addr[6];

	u8						paring_tk[16];   // in security connection to keep own random
	u8						own_ltk[16];    //used for generate ediv and random
	u8						own_csrk[16];
	u8						own_irk[16];

	smp_paring_req_rsp_t  	paring_req;
	u8						rsvd0;

	smp_paring_req_rsp_t  	paring_rsp;
	u8						rsvd1;

}smp_ms_param_own_t;

typedef struct {
	u32 smp_timeout_start_tick;//SMP 开启时间标记

	u16 smpDistirbuteKeyOrder; //密钥分配启动标志
	smp_keyDistribution_t smp_DistributeKeyInit; //master distribute key transmit
	smp_keyDistribution_t smp_DistributeKeyResp; //slave distribute key transmit

	smp_ms_param_peer_t smp_param_peer;
	smp_ms_param_own_t  smp_param_own;

	smpm_trigger_t 		blm_smpTrig; //used by master-role
	secReq_ctl_t   		SecReq_ctrl; //used by slave-role
	smp_mng_t 			smpMng;
}smp_ms_param_t;

extern smp_ms_param_t blms_smpMng[];

u8 blms_isSmpBusy(void);
int blms_smp_isParingBusy(u16 connHandle );
void blms_smp_setParingBusy(u16 connHandle, u8 busy);
u8 blms_isMultiRoleIdle(void);
u8  blms_ll_pushEncPkt (u16 connHandle, u8 type);
int blms_ltk_request(u16 connHandle, u8* random, u16 ediv);
void blms_ll_security_proc(u16 connHandle);
int blms_smp_encryption_done(u16 connHandle);
u8 *blms_smp_pushPkt (u16 connHandle, int type);
void blms_smp_saveKey (u16 connHandle);
u8 * blms_smp_sendInfo (u16 connHandle);
void blms_smp_procParingEnd(u16 connHandle, u8 err_reason);
u8 * l2cap_ms_smp_handler(u16 connHandle, u8 * p);
void blms_smp_certTimeoutLoopEvt (void);
void blms_smp_param_init(void);




#endif /* SMP_SM_H_ */
