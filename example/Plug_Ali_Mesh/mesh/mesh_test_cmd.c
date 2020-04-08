/********************************************************************************************************
 * @file     mesh_test_cmd.c 
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
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/pm.h"
#include "app_proxy.h"
#include "app_health.h"
#include "mesh_test_cmd.h"

#if 0
#if SEC_MES_DEBUG_EN
mesh_cmd_bear_seg_t B_test_cmd = {
    /*.trans_par_val = */0,
    /*.len =       */0,
    /*.type =     */MESH_ADV_TYPE_MESSAGE,
};

void mesh_message6()
{
    mesh_cmd_bear_seg_t *p_bear = (mesh_cmd_bear_seg_t *)&B_test_cmd;
    if(mesh_adv_tx_cmd_sno < 0x3129ab){
        mesh_adv_tx_cmd_sno = 0x3129ab;
    }
    
    // lower layer
    //p_bear->lt.aid = 0;
    p_bear->lt.akf = 0;
    //p_bear->lt.seg = 1;
    //p_bear->lt.segN = 1;
    //p_bear->lt.segO = 0;
    //p_bear->lt.seqzero = sno;// & BIT_MASK_LEN(13);
    p_bear->lt.szmic = 0;
    
    // network layer
    //p_bear->nw.nid = 0x68;
    //p_bear->nw.ivi = 0;       // ivi is least significant bit
    p_bear->nw.ttl = 4;
    p_bear->nw.ctl = 0;
    #if 0
    p_bear->nw.src = 0x0003;
    p_bear->nw.dst = 0x1201;
    #else
    p_bear->nw.src = ele_adr_primary;
    if(ADR_LPN1 == ele_adr_primary){
        p_bear->nw.dst = ADR_FND2;
    }else{
        p_bear->nw.dst = ADR_LPN1;
    }
    #endif
    
    u8 r_apl[] = {0x02,0x23,0x61,0x45,0x63,0x96,0x47,0x71, 0x73,0x4f,0xbd,0x76,0xe3,0xb4,0x05,0x19,
				  0xd1,0xd9,0x4a,0x48,
				  #if 0 // DEBUG_SEG_RX
				  0x00,0x00,0x00,0x00,0x33,0x33,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x44,0x44,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x00,
				  #endif
				  };

    mesh_match_type_t match_type;
    mesh_match_group_mac(&match_type, p_bear->nw.dst, rf_link_get_op_by_ac(r_apl), 1, ele_adr_primary);

    mesh_tx_cmd_layer_acccess(r_apl, sizeof(r_apl), ele_adr_primary, p_bear->nw.dst, 0, &match_type);
}
#endif

#if FRI_SAMPLE_EN
void mesh_message_fri_msg()
{
#if 1
    mesh_message_fri_msg_ctl_seg();
    return ;
#endif

#if SEC_MES_DEBUG_EN
    mesh_cmd_bear_seg_t *p_bear = (mesh_cmd_bear_seg_t *)&B_test_cmd;

    // lower layer
    //p_bear->lt.aid = 0;
    p_bear->lt.akf = 0;
    //p_bear->lt.seg = 1;
    //p_bear->lt.segN = 1;
    //p_bear->lt.segO = 0;
    //p_bear->lt.seqzero = sno;// & BIT_MASK_LEN(13);
    p_bear->lt.szmic = 0;
    
    // network layer
    //p_bear->nw.nid = 0x68;
    //p_bear->nw.ivi = 0;       // ivi is least significant bit
    p_bear->nw.ttl = 4;
    p_bear->nw.ctl = 0;

    p_bear->nw.src = ele_adr_primary;
    p_bear->nw.dst = ADR_LPN1;
    
    u8 r_apl[] = {0x02,0x23,0x61,0x45,0x63,0x96,0x47,0x71, 0x73,0x4f,0xbd,
                  #if 1 // segment
                  0x76,0xe3,0xb4,0x05,0x19,0xd1,0xd9,0x4a,0x48,
                  #endif
				  #if 0 // DEBUG_SEG_RX
				  0x00,0x00,0x00,0x00,0x33,0x33,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x44,0x44,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x00,
				  #endif
				  };

    SEG_DEBUG_LED(0);
    mesh_match_type_t match_type;
    mesh_match_group_mac(&match_type, p_bear->nw.dst, rf_link_get_op_by_ac(r_apl), 1, ele_adr_primary);
    
    mesh_tx_cmd_layer_acccess(r_apl, sizeof(r_apl), ele_adr_primary, p_bear->nw.dst, 0, &match_type);
#endif
}

static const u8 B_test_ctl_par_org[] = {
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
    0x21,0x22,0x23,0x24,//0x25,0x26,0x27,0x28,
    //0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
};

void mesh_message_fri_msg_ctl_seg()
{
    static u8 B_test_ctl_par[sizeof(B_test_ctl_par_org)] = {0};
    
    static u8 msg_idx;
    foreach_arr(i,B_test_ctl_par){
        B_test_ctl_par[i] = B_test_ctl_par_org[i] + msg_idx*0x30;
    }

    SEG_DEBUG_LED(0);
    int err = mesh_tx_cmd_layer_upper_ctl_primary(0x3F, B_test_ctl_par, sizeof(B_test_ctl_par), ADR_FND2);

    if(!err){
        if(msg_idx >= 4){
            msg_idx = 0;
        }else{
            msg_idx++;
        }
    }
}
#endif

#if 0
void keyboard_handle_mesh()
{
    u16 adr_dst = ADR_LPN1;
    adr_dst = adr_dst;
#if 1
    if(is_fn_support_and_en){
        mesh_message_fri_msg();
    }else if(0){
        mesh_message6();
        SEG_DEBUG_LED(0);
    }else if(is_lpn_support_and_en){
        // send Poll command after suspend
    }
#elif 0
    mesh_model_cfg_cmd_test();
#else   // on off
    u8 par[11-3] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    static u8 cnt;
    par[0] = (++cnt) & 1;
    mesh_tx_cmd2normal_primary(VD_LIGHT_ONOFF_SET, par, 1, adr_dst, 1);
#endif
}
#endif


#if FRI_SAMPLE_EN
void friend_ship_sample_message_test()
{
    mesh_lpn_par.LPNAdr= ADR_LPN1;
    mesh_lpn_par.FriAdr = ADR_FND1;

    // sample data        
    mesh_ctl_fri_req_t *p_req = &mesh_lpn_par.req;
    p_req->Criteria.MinCacheSizeLog = 1; // 3;
    p_req->Criteria.RecWinFac = FRI_REC_WIN_FAC;
    p_req->Criteria.RSSIFac = FRI_REC_RSSI_FAC;
    p_req->RecDelay = FRI_REC_DELAY_MS;
    p_req->PollTimeout = 0x057e40;
    p_req->PreAdr = 0;
    p_req->NumEle = 1;
    p_req->LPNCounter = 0; 
    mesh_ctl_fri_update_t *p_update = &fn_update[0];
    memcpy(p_update->IVIndex, iv_idx_st.tx, sizeof(p_update->IVIndex));

    mesh_ctl_fri_offer_t *p_offer = &mesh_lpn_par.offer;
    p_offer->RecWin = FRI_REC_WIN_MS;
    p_offer->CacheSize = 0x03;
    p_offer->SubsListSize = 0x08;
    p_offer->RSSI = -70;
    p_offer->FriCounter = 0x072f;
    mesh_friend_key_update_all_nk(0);
    friend_cmd_send_request();
}

void friend_cmd_send_sample_message(u8 op)
{
    u16 dev_adr_temp = ele_adr_primary;

    if(CMD_CTL_REQUEST == op){
        ele_adr_primary = ADR_LPN1;
        mesh_adv_tx_cmd_sno = 0x000001;
        friend_cmd_send_request();
    }else if(CMD_CTL_OFFER == op){
        ele_adr_primary = ADR_FND1;
        mesh_adv_tx_cmd_sno = 0x014820;
        friend_cmd_send_offer(0);
    }else if(CMD_CTL_POLL == op){
        ele_adr_primary = ADR_LPN1;
        mesh_adv_tx_cmd_sno = 0x000002;
        friend_cmd_send_poll();
    }else if(CMD_CTL_UPDATE == op){
        ele_adr_primary = ADR_FND1;
        mesh_adv_tx_cmd_sno = 0x014834;
        friend_cmd_send_update(0);
    }
    
    ele_adr_primary = dev_adr_temp;  // restore
}
#endif
#endif

void test_cmd_tdebug()
{
	static volatile u8 A_key = 0,A_key_seg = 1,A_key_virtual = 0;
	static volatile u16 A_key_adr = 0x0001;//0xffff;
	if(A_key){
		A_key = 0;
		static u8 test_onoff;
		u32 len = OFFSETOF(mesh_cmd_g_onoff_set_t,transit_t);	// no delay 
		u8 cmd_buf[32] = {0};
		memset(cmd_buf, 0xaa, sizeof(cmd_buf));
		mesh_cmd_g_onoff_set_t *cmd = (mesh_cmd_g_onoff_set_t *)cmd_buf;
		cmd->onoff = (test_onoff++) & 1;
		if(A_key_seg){
			len += 13;	// test segment;
		}

		if(A_key_virtual){
			mesh_tx_cmd2uuid(G_ONOFF_SET_NOACK, (u8 *)cmd, len, ele_adr_primary, 0, 0, prov_para.device_uuid);
		}else{
			mesh_tx_cmd2normal_primary(G_ONOFF_SET_NOACK, (u8 *)cmd, len, A_key_adr, 0);
		}
	}
}

