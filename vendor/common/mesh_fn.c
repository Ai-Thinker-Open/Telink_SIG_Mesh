/********************************************************************************************************
 * @file     mesh_fn.c 
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
#include "vendor/common/user_config.h"
#include "proj_lib/pm.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "lighting_model.h"
#include "vendor/common/sensors_model.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "mesh_common.h"
#include "vendor/common/app_health.h"
#include "vendor/common/app_heartbeat.h"

// ------- friend node && LPN common
u8 mesh_subsc_adr_cnt_get (mesh_cmd_bear_unseg_t *p_br)
{
    u32 len_lt = mesh_lt_len_get_by_bear(p_br);
    return (len_lt - 1)/2;
}

int fn_cache_get_extend_adv_short_unseg(u32 ctl, u8 len_ut_max_single_seg_rx)
{
    #if (MESH_DLE_MODE == MESH_DLE_MODE_EXTEND_BEAR)
    if(len_ut_max_single_seg_rx == (mesh_max_payload_get(ctl, 1))){
        return 1; // keep the same max bear length
    }
    #endif
    
    return 0;
}

void friend_cmd_send_fn(u8 lpn_idx, u8 op)  // always need.
{
#if (FRI_SAMPLE_EN)
	friend_cmd_send_sample_message(op);
#else
    if(CMD_CTL_HEARTBEAT == op){
		u16 feature =0;
		memcpy((u8 *)(&feature),(u8 *)&(gp_page0->head.feature),2);
		heartbeat_cmd_send_conf(model_sig_cfg_s.hb_pub.ttl,
			feature,model_sig_cfg_s.hb_pub.dst_adr);
	}else if(CMD_CTL_CLEAR == op){
	#if (FEATURE_FRIEND_EN || FEATURE_LOWPOWER_EN)
		mesh_ctl_fri_clear_t fri_clear;
		u16 adr_dst = 0;
		#if FEATURE_LOWPOWER_EN
		fri_clear.LPNAdr = mesh_lpn_par.LPNAdr;
		fri_clear.LPNCounter = 2;       // comfirm later, should use parameters in last request command.
		adr_dst = mesh_lpn_par.FriAdr;  // comfirm later, should use parameters in last request command.
		#elif FEATURE_FRIEND_EN
		fri_clear.LPNAdr = fn_other_par[lpn_idx].LPNAdr;
		fri_clear.LPNCounter = fn_req[lpn_idx].LPNCounter;
		adr_dst = fn_req[lpn_idx].PreAdr;
		#endif
        friend_cmd_send_clear(adr_dst, (u8 *)&fri_clear, sizeof(mesh_ctl_fri_clear_t));
    #endif
    }
	#if FEATURE_FRIEND_EN
	else if(CMD_CTL_OFFER == op){
        #if FRIEND_MSG_TEST_EN
        if(mesh_adv_tx_cmd_sno < 0x014820){
            mesh_adv_tx_cmd_sno = 0x014820; // for test
        }
        #endif
        friend_cmd_send_offer(lpn_idx);
    }else if(CMD_CTL_UPDATE == op){
        #if FRIEND_MSG_TEST_EN
        if(mesh_adv_tx_cmd_sno < 0x014834){
            mesh_adv_tx_cmd_sno = 0x014834; // for test
        }
        #endif
        friend_cmd_send_update(lpn_idx, 0);
    }
    #endif
#endif
}


#if (FEATURE_FRIEND_EN || FEATURE_LOWPOWER_EN)
// cache data should be not encryption. p_bear should have been big endianness
void mesh_friend_logmsg(mesh_cmd_bear_unseg_t *p_bear_big, u8 len)
{
	#if (DEBUG_PROXY_FRIEND_SHIP && WIN32)
	extern u8 log_en_lpn1,log_en_lpn2;
	u16 adr_dst = p_bear_big->nw.dst;
	endianness_swap_u16((u8 *)&adr_dst);
	if((log_en_lpn1 && (adr_dst == (PROXY_FRIEND_SHIP_MAC_LPN1 & 0xff)))
	 || (log_en_lpn2 && (adr_dst == (PROXY_FRIEND_SHIP_MAC_LPN2 & 0xff)))){
		LOG_MSG_WARN(TL_LOG_FRIEND,(u8 *)p_bear_big, len,"mesh_friend_logmsg:send message to LPN_win32: \r\n",0);
	}
	#else
	LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_bear_big, len,"mesh_friend_logmsg:send message to LPN: \r\n",0);
	#endif
}

void friend_subsc_list_add_adr(lpn_adr_list_t *adr_list_src, lpn_adr_list_t *adr_list_add, u32 cnt)
{
    // should not use u16* as parameter,because it is not sure 2bytes aligned
    foreach(i,cnt){
        int exit_flag = 0;
        foreach(j,SUB_LIST_MAX_LPN){
            if(adr_list_add->adr[i] == adr_list_src->adr[j]){
                exit_flag = 1;
                break;
            }
        }
        
        if(!exit_flag){
            foreach(k,SUB_LIST_MAX_LPN){
                if(0 == adr_list_src->adr[k]){
                    adr_list_src->adr[k] = adr_list_add->adr[i];
                    break;
                }
            }
        }
    }
}

// use 'inline' should be better. 
void friend_subsc_list_rmv_adr(lpn_adr_list_t *adr_list_src, lpn_adr_list_t *adr_list_rmv, u32 cnt)
{
    // should not use u16* as parameter,because it is not sure 2bytes aligned
    foreach(i,cnt){
        foreach(j,SUB_LIST_MAX_LPN){
            if(adr_list_rmv->adr[i] == adr_list_src->adr[j]){
                adr_list_src->adr[j] = 0;
                //break;
            }
        }
    }
}

void friend_cmd_send_clear(u16 adr_dst, u8 *par, u32 len)
{	
	LOG_MSG_LIB(TL_LOG_FRIEND,par, len,"send friend clear:",0);
    mesh_tx_cmd_layer_upper_ctl(CMD_CTL_CLEAR, par, len, ele_adr_primary, adr_dst,0);
}


#if TEST_CASE_HBS_BV_05_EN
void friend_add_special_grp_addr()
{
	u16 sub_adr = 0xc100;
	friend_subsc_list_add_adr(fn_other_par[0].SubsList, &sub_adr, 1);
	return ;
}
#endif

void lpn_quick_send_adv()
{
#ifndef WIN32
	u8 r = irq_disable();
    if(blt_state == BLS_LINK_STATE_ADV || ((blt_state == BLS_LINK_STATE_CONN)&&(ble_state == BLE_STATE_BRX_E))){
        blt_send_adv2scan_mode(1);
    }
    irq_restore(r);
#endif
}
#endif

// ------- friend node
#if (FEATURE_FRIEND_EN)
u8 get_tx_nk_arr_idx_friend(u16 adr, u16 op)
{
	u8 nk_arr_idx = 0;
    if(is_fn_support_and_en && adr && is_unicast_adr(adr)){
        foreach(i, g_max_lpn_num){
            if(is_ele_in_node(adr, fn_other_par[i].LPNAdr, fn_req[i].NumEle)){
                if((CMD_CTL_OFFER == op) || is_in_mesh_friend_st_fn(i)){
                    if(fn_other_par[i].nk_sel_dec_fn < NET_KEY_MAX){
                        nk_arr_idx = fn_other_par[i].nk_sel_dec_fn;
                    }
                }
                break;
            }
        }
    }
    return nk_arr_idx;
}

u16 mesh_mac_match_friend(u16 adr)
{
	u16 match_F2L = 0;
    if(is_fn_support_and_en && is_unicast_adr(adr)){
        foreach(i, g_max_lpn_num){
            if(is_in_mesh_friend_st_fn(i) && is_ele_in_node(adr, fn_other_par[i].LPNAdr, fn_req[i].NumEle)){
                match_F2L |= BIT(i);
                break;
            }
        }
    }
    return match_F2L;
}

u16 mesh_group_match_friend(u16 adr)
{
	u16 match_F2L = 0;
    if(is_fn_support_and_en){
        foreach(i, g_max_lpn_num){
            if(is_in_mesh_friend_st_fn(i)){
                foreach_arr(m,fn_other_par[i].SubsList){
                    if(fn_other_par[i].SubsList[m] == adr){
                        match_F2L |= BIT(i);
                        break;
                    }
                }
            }
        }
    }
    return match_F2L;
}

int is_unicast_friend_msg_from_lpn(mesh_cmd_nw_t *p_nw)
{
	foreach(i,g_max_lpn_num){
	    if(is_in_mesh_friend_st_fn(i)
	        && (fn_other_par[i].LPNAdr == p_nw->src)
	        && (fn_other_par[i].FriAdr == p_nw->dst)){    // must because of group address
	        return 1;
	    }
	}
	return 0;
}

int is_unicast_friend_msg_to_lpn(mesh_cmd_nw_t *p_nw)
{
	if(is_unicast_adr(p_nw->dst)){
		foreach(i,g_max_lpn_num){
		    if(is_in_mesh_friend_st_fn(i)
		        && (fn_other_par[i].LPNAdr == p_nw->dst)
		        && (fn_other_par[i].FriAdr == p_nw->src)){    // must because of relay message
		        return 1;
		    }
		}
	}
	return 0;
}

int is_cmd2lpn(u16 adr_dst)
{
	foreach(i,g_max_lpn_num){
		if(fn_other_par[i].LPNAdr == adr_dst){
			return 1;
		}
	}
	return 0;
}

int friend_cache_check_replace(u8 lpn_idx, mesh_cmd_bear_unseg_t *bear_big)
{
    int replace = 0;
    if(bear_big->nw.ctl){
        u8 op = bear_big->lt_ctl_unseg.opcode;
        if((CMD_CTL_UPDATE==op)||(CMD_CTL_ACK==op)){
            u8 r = irq_disable();
            mesh_cmd_bear_unseg_t *p_buf_bear;
            u8 cnt = my_fifo_data_cnt_get(fn_other_par[lpn_idx].p_cache);
            foreach(i,cnt){
                p_buf_bear = (mesh_cmd_bear_unseg_t *)my_fifo_get_offset(fn_other_par[lpn_idx].p_cache, i);
                // is big endianness in cache buff
                u8 op_buf = p_buf_bear->lt_ctl_unseg.opcode;
                if(CMD_CTL_UPDATE==op_buf){
                    if(i){  // the last one have been sent, just wait for check need retry or not.
                        replace = 1;    // always 1
                        memcpy(p_buf_bear->nw.sno, bear_big->nw.sno, 3);
                        if(memcmp(p_buf_bear->lt_ctl_unseg.data, bear_big->lt_ctl_unseg.data,sizeof(mesh_ctl_fri_update_t))){
                            memcpy(&p_buf_bear->lt_ctl_unseg.data, bear_big->lt_ctl_unseg.data, sizeof(mesh_ctl_fri_update_t));
                        }
                        break;
                    }
                }else if(CMD_CTL_ACK==op_buf){
                    if(p_buf_bear->lt_ctl_ack.seqzero == bear_big->lt_ctl_ack.seqzero){
                        replace = 1;
                        memcpy(p_buf_bear->nw.sno, bear_big->nw.sno, 3);
                        memcpy(&p_buf_bear->lt_ctl_ack.seg_map, &bear_big->lt_ctl_ack.seg_map, 4);
                        break;
                    }
                }
            }
            irq_restore(r);
        }
    }

    return replace;
}

mesh_fri_ship_other_t * mesh_fri_cmd2cache(u8 *p_bear_big, u8 len_nw, u8 adv_type, u8 trans_par_val, u16 F2L_bit)
{
    int err = 0;
    mesh_cmd_bear_unseg_t *p_br_big = (mesh_cmd_bear_unseg_t *)p_bear_big;
    mesh_fri_ship_other_t *p_other = 0;
    
    // fifo packet
    p_br_big->trans_par_val = trans_par_val;
    p_br_big->len = len_nw + 1;
    p_br_big->type = adv_type;

    foreach(i,g_max_lpn_num){
    	if(F2L_bit & BIT(i)){
    	    p_other = &fn_other_par[i];
		    if(!p_other->p_cache){
		        while_1_test;
		    }
		    
		    // big endianness and not encryption in FIFO
		    int replace = friend_cache_check_replace(i, p_br_big);
		    if(0 == replace){
				int overwrite = (my_fifo_free_cnt_get(p_other->p_cache) == 0);
		        err = my_fifo_push_adv(p_other->p_cache, (u8 *)p_br_big, mesh_bear_len_get(p_br_big), 1);
				if(!err){
					if(overwrite){
						p_other->cache_overwrite = 1;
					}
		        }else{
					p_other = 0;
		        }
		    }
	    }
    }
    return p_other;
}

void mesh_iv_update_start_poll_fn(u8 iv_update_by_sno, u8 beacon_iv_update_pkt_flag)
{
    foreach(i, g_max_lpn_num){
        if(is_in_mesh_friend_st_fn(i)){
            if(iv_update_by_sno || beacon_iv_update_pkt_flag){
                friend_cmd_send_update(i, 1);//friend_cmd_send_fn(CMD_CTL_UPDATE);
            }
        }
    }
}

int is_friend_ship_link_ok_fn(u8 lpn_idx)
{
    return fn_other_par[lpn_idx].link_ok;
}

void friend_cmd_send_offer(u8 lpn_idx)
{
	LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)(fn_offer+lpn_idx), sizeof(mesh_ctl_fri_offer_t),"send friend offer:",0);
    fn_offer[lpn_idx].FriCounter++; // must before
    mesh_tx_cmd_layer_upper_ctl_FN(CMD_CTL_OFFER, (u8 *)(fn_offer+lpn_idx), sizeof(mesh_ctl_fri_offer_t), fn_other_par[lpn_idx].LPNAdr);
    mesh_friend_key_update_all_nk(lpn_idx, fn_other_par[lpn_idx].nk_sel_dec_fn);
}

void friend_cmd_send_update(u8 lpn_idx, u8 md)
{
    mesh_net_key_t *p_netkey = &mesh_key.net_key[fn_other_par[lpn_idx].nk_sel_dec_fn][0];
    get_iv_update_key_refresh_flag(&fn_update[lpn_idx].flag, fn_update[lpn_idx].IVIndex, p_netkey->key_phase);
    fn_update[lpn_idx].md = !!md;
    LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)(fn_update+lpn_idx), sizeof(mesh_ctl_fri_update_t),"send friend update:",0);
    mesh_tx_cmd_layer_upper_ctl_FN(CMD_CTL_UPDATE, (u8 *)(fn_update+lpn_idx), sizeof(mesh_ctl_fri_update_t), fn_other_par[lpn_idx].LPNAdr);
}

void friend_cmd_send_clear_conf(u16 adr_dst, u8 *par, u32 len)
{
	LOG_MSG_LIB(TL_LOG_FRIEND,par, len,"send friend clear confirm:",0);
    mesh_tx_cmd_layer_upper_ctl_FN(CMD_CTL_CLR_CONF, par, len, adr_dst);	
}

void friend_cmd_send_subsc_conf(u16 adr_dst, u8 transNo)
{
	LOG_MSG_LIB(TL_LOG_FRIEND, (u8 *)&transNo, 1,"send friend sub list confirm:",0);
    use_mesh_adv_fifo_fn2lpn = 1;
    mesh_tx_cmd_layer_upper_ctl_FN(CMD_CTL_SUBS_LIST_CONF, (u8 *)&transNo, 1, adr_dst);
    use_mesh_adv_fifo_fn2lpn = 0;
}

u8 fn2lpn_no_retransmit = 0;
int mesh_tx_cmd_add_packet_fn2lpn(u8 *p_bear)
{
    #if WIN32	// WIN32 can't use mesh_adv_fifo_fn2lpn, because not call app_advertise_prepare_handler_
    use_mesh_adv_fifo_fn2lpn = 0;
    return mesh_tx_cmd_add_packet(p_bear);
    #else
    mesh_cmd_bear_unseg_t *p = (mesh_cmd_bear_unseg_t *)p_bear;
    if(pts_test_en || fn2lpn_no_retransmit){
	    p->trans_par_val = 0x10;    // FN-BV16 / BV19 require no more than NW PDU. comfirm later.
	}
	LOG_MSG_INFO(TL_LOG_MESH,0, 0,"mesh_tx_cmd_add_packet_fn2lpn",0);
    int err = my_fifo_push_adv(&mesh_adv_fifo_fn2lpn, p_bear, mesh_bear_len_get(p), 0);
    lpn_quick_send_adv();
    return err;
    #endif
}

void mesh_rc_segment_handle_fn(mesh_match_type_t *p_match_type, mesh_cmd_nw_t *p_nw)
{
    if(mesh_cmd_action_need_friend(p_match_type)){
        #if FN_PRIVATE_SEG_CACHE_EN
        while(is_friend_seg_cache_busy()){  // push all cache of last segment to friend cache.
            friend_seg_cache2friend_cache(p_match_type->F2L);
        }
        
        mesh_rc_segment2fri_segment(p_nw);
        #else
        mesh_rc_segment2friend_cache(p_nw, p_match_type->F2L);
        #endif
        SEG_DEBUG_LED(1);
    }
}

u32 mesh_get_val_with_factor(u32 val, u32 fac)
{
    u32 result;
    if(1 == fac){
        result = (val * 3) / 2;
    }else if(2 == fac){
        result = (val * 2);
    }else if(3 == fac){
        result = (val * 5) / 2;
    }else{
        result = val;
    }
    return result;
}

#define get_min_cache_size_by_log(n)    (1 << n)

int mesh_friend_request_is_valid(mesh_ctl_fri_req_t *p_req)
{
    return (/*(!p_req->Criteria.RFU)&&*/(p_req->Criteria.MinCacheSizeLog)&&(p_req->RecDelay >= 0x0A)
          &&((p_req->PollTimeout >= 0x0A)&&(p_req->PollTimeout <= 0x34BBFF))&&p_req->NumEle);
}

#if 0
u32 get_poll_timeout_fn(u16 lpn_adr)
{
    foreach_arr(i,fn_other_par){
        if(is_friend_ship_link_ok_fn(i) && (lpn_adr == fn_other_par[i].LPNAdr)){
            return fn_req[i].PollTimeout;
        }
    }
    return 0;
}
#endif

u32 get_current_poll_timeout_timer_fn(u16 lpn_adr)
{
    foreach_arr(i,fn_other_par){
        if(is_friend_ship_link_ok_fn(i) && (lpn_adr == fn_other_par[i].LPNAdr)){
            u32 timeout_100ms = fn_req[i].PollTimeout;
            if (timeout_100ms != 0x000000) {
            	u32 span_100ms = (clock_time_100ms()|1) - fri_ship_proc_fn[i].poll_tick;
            	if(timeout_100ms > span_100ms){
            	    timeout_100ms -= span_100ms;
            	}else{
            	    timeout_100ms = 0;
            	}
            }
            return timeout_100ms;
        }
    }
    return 0;
}

static inline int mesh_friend_poll_is_valid(mesh_ctl_fri_poll_t *p_poll)
{
    return (0 == p_poll->RFU);
}

inline void mesh_friend_ship_set_st_fn(u8 lpn_idx, u8 st)
{
    fri_ship_proc_fn[lpn_idx].status = st;
}

void mesh_friend_ship_proc_init_fn(u8 lpn_idx)
{
    memset(&fri_ship_proc_fn[lpn_idx], 0, sizeof(mesh_fri_ship_proc_fn_t));
}

void mesh_friend_ship_clear_FN(u8 lpn_idx)
{
	mesh_friend_ship_proc_init_fn(lpn_idx);
    memset(fn_other_par+lpn_idx, 0, sizeof(fn_other_par[0]));
    memset(fn_req+lpn_idx, 0, sizeof(fn_req[0]));
    memset(fn_poll+lpn_idx, 0, sizeof(fn_poll[0]));
    memset(fn_ctl_rsp_delay+lpn_idx, 0, sizeof(fn_ctl_rsp_delay[0]));
    mesh_fri_cache_fifo[lpn_idx].wptr = mesh_fri_cache_fifo[lpn_idx].rptr = 0;
}

void friend_ship_disconnect_fn(u8 lpn_idx, int type)
{
    LOG_MSG_LIB(TL_LOG_FRIEND,0, 0,"Friend ship disconnect, LPN addr:0x%04x, type: %d ", fn_other_par[lpn_idx].LPNAdr, type);
    friend_ship_disconnect_cb_fn(lpn_idx, type);
    mesh_friend_ship_clear_FN(lpn_idx);
}

void mesh_friend_ship_init_all()
{
    foreach(i,g_max_lpn_num){
        mesh_friend_ship_clear_FN(i);
    }
}

inline u8* mesh_friend_ship_cache_check(my_fifo_t *f)
{
    return my_fifo_get(f);
}

int is_poll_cmd(mesh_cmd_nw_t *p_nw)
{
    mesh_cmd_bear_unseg_t *p_bear = CONTAINER_OF(p_nw,mesh_cmd_bear_unseg_t,nw);
    return (p_bear->nw.ctl && (CMD_CTL_POLL == p_bear->lt_ctl_unseg.opcode));
}

int is_in_mesh_friend_st_fn(u8 lpn_idx)
{
	if(is_fn_support_and_en){
    	return (is_friend_ship_link_ok_fn(lpn_idx) && (fn_other_par[lpn_idx].LPNAdr != 0));
	}else{
		return 0;
	}
}

u32 mesh_friend_local_delay(u8 lpn_idx)		// for FN
{
    int t_delay = mesh_get_val_with_factor(fn_offer[lpn_idx].RecWin, fn_req[lpn_idx].Criteria.RecWinFac)
                - mesh_get_val_with_factor(fn_offer[lpn_idx].RSSI, fn_req[lpn_idx].Criteria.RSSIFac);

    if(t_delay < 100){
        t_delay = 100;    // spec required.
    }

    return t_delay;
}

inline void mesh_stop_clear_cmd(u8 lpn_idx)
{
	#if 1
	fri_ship_proc_fn[lpn_idx].clear_poll = 0;
	#else
	mesh_friend_ship_proc_init_fn(lpn_idx);
	#endif
}

void mesh_reset_poll_timeout_timer(u8 lpn_idx)
{
    fri_ship_proc_fn[lpn_idx].poll_tick = clock_time_100ms();
}

enum{
    DELAY_POLL = 1,
    DELAY_SUBSC_LIST,
    DELAY_CLEAR_CONF,
};

void mesh_friend_set_delay_par_poll(u8 lpn_idx, u8 *rsp, u32 timeStamp)
{
    fn_ctl_rsp_delay_t *p_delay = &fn_ctl_rsp_delay[lpn_idx];
    p_delay->delay_type = DELAY_POLL;
    p_delay->tick = timeStamp;
    p_delay->poll_rsp = rsp;

	mesh_friend_logmsg((mesh_cmd_bear_unseg_t *)rsp, ((mesh_cmd_bear_unseg_t *)rsp)->len + 2);
}

void mesh_friend_set_delay_par(u8 delay_type, u8 lpn_idx, u16 adr_dst, u16 par_val)
{
    fn_ctl_rsp_delay_t *p_delay = &fn_ctl_rsp_delay[lpn_idx];
    p_delay->delay_type = delay_type;
    p_delay->tick = clock_time();
    p_delay->adr_dst = adr_dst;
    p_delay->par_val = par_val;
}

void mesh_friend_response_delay_proc_fn(u8 lpn_idx)
{
    fn_ctl_rsp_delay_t *p_delay = &fn_ctl_rsp_delay[lpn_idx];
    
    if(p_delay->delay_type && clock_time_exceed(p_delay->tick, fn_req[lpn_idx].RecDelay * 1000 - 1800)){    // 1800us: encryption pkt time
        if(DELAY_POLL == p_delay->delay_type){
            if(p_delay->poll_rsp){
				u8 bear_temp_buf[MESH_BEAR_SIZE];
                mesh_cmd_bear_unseg_t *bear_temp = (mesh_cmd_bear_unseg_t *)bear_temp_buf; // TODO DLE
                memcpy(bear_temp, p_delay->poll_rsp, min(MESH_BEAR_SIZE, mesh_bear_len_get((mesh_cmd_bear_unseg_t *)p_delay->poll_rsp)));
				u8 bear_tx_len = mesh_bear_len_get(bear_temp);
				//LOG_MSG_LIB(TL_LOG_FRIEND,0, 0 ,"bear_tx_len=%d\r\n", bear_tx_len);
                if(bear_tx_len <= MESH_BEAR_SIZE){
    	            //LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)&bear_temp.len, bear_temp.len+1,"Data for poll:",0);
                    
                    mesh_sec_msg_enc_nw_rf_buf((u8 *)(&bear_temp->nw), mesh_lt_len_get_by_bear(bear_temp), FRIENDSHIP, lpn_idx,0,fn_other_par[lpn_idx].nk_sel_dec_fn, 0);
                    mesh_tx_cmd_add_packet_fn2lpn((u8 *)bear_temp);
                }else{
                    LOG_MSG_ERR(TL_LOG_MESH,0, 0 ,"fn rsp len err",0);
                    //  TODO: DLE
                }
            }

            mesh_fri_ship_proc_fn_t *proc_fn = &fri_ship_proc_fn[lpn_idx];
            if(proc_fn->clear_delay_cnt){
                proc_fn->clear_delay_cnt--;
                if(0 == proc_fn->clear_delay_cnt){ // make sure establish friend ship success
                    friend_cmd_send_fn(lpn_idx, CMD_CTL_CLEAR); // use normal fifo
                    proc_fn->clear_cmd_tick = proc_fn->clear_start_tick = clock_time_100ms();
                    proc_fn->clear_int_100ms = FRI_FIRST_CLR_INTERVAL_100MS;
                    proc_fn->clear_poll = 1;
                }
            }
        }else if(DELAY_SUBSC_LIST == p_delay->delay_type){
            friend_cmd_send_subsc_conf(p_delay->adr_dst, (u8)p_delay->par_val);
        }else if(DELAY_CLEAR_CONF == p_delay->delay_type){
            mesh_ctl_fri_clear_t clear;
            clear.LPNAdr = fn_other_par[lpn_idx].LPNAdr;
            clear.LPNCounter = p_delay->par_val;
            use_mesh_adv_fifo_fn2lpn = 1;
            friend_cmd_send_clear_conf(clear.LPNAdr, (u8 *)&clear, sizeof(mesh_ctl_fri_clear_t));
            use_mesh_adv_fifo_fn2lpn = 0;
        }
        
        p_delay->delay_type = 0;
    }
}

void mesh_friend_ship_proc_FN(u8 *bear)
{
	foreach(i,g_max_lpn_num){
		mesh_fri_ship_proc_fn_t *proc_fn = &fri_ship_proc_fn[i];
	    if(!bear){
	        if(proc_fn->status){ // (FRI_ST_IDLE != proc_fn->status)
    	        if(FRI_ST_OFFER == proc_fn->status){
    	            if(clock_time_exceed(proc_fn->offer_tick, proc_fn->offer_delay*1000)){
    	                use_mesh_adv_fifo_fn2lpn = 1;
    	                friend_cmd_send_fn(i, CMD_CTL_OFFER);
    	                use_mesh_adv_fifo_fn2lpn = 0;
    	                proc_fn->offer_tick = clock_time()|1;
    	                mesh_friend_ship_set_st_fn(i, FRI_ST_POLL);
    	            }
    	        }else if(FRI_ST_POLL == proc_fn->status){
    	            // add 500ms, because handling response of POLL was delay some ten ms. 
    	            if(clock_time_exceed(proc_fn->offer_tick, (500+FRI_ESTABLISH_OFFER_MS)*1000)){
    	                mesh_friend_ship_proc_init_fn(i);
    	            }
    	        }else if(FRI_ST_TIMEOUT_CHECK == proc_fn->status){
    	            if(clock_time_exceed_100ms(proc_fn->poll_tick, (u32)(fn_req[i].PollTimeout))){
    	                friend_ship_disconnect_fn(i, FS_DISCONNECT_TYPE_POLL_TIMEOUT);
    	            }
    	        }
	        }
	        	        
	        if(proc_fn->clear_poll){    // clear by other FN
	            if(clock_time_exceed_100ms(proc_fn->clear_start_tick, (u32)(fn_req[i].PollTimeout)*2)){
	                mesh_stop_clear_cmd(i);
	            }else{
	                if(clock_time_exceed_100ms(proc_fn->clear_cmd_tick, proc_fn->clear_int_100ms)){
	                    proc_fn->clear_cmd_tick = clock_time_100ms();
	                    proc_fn->clear_int_100ms = proc_fn->clear_int_100ms << 1;
	                    friend_cmd_send_fn(i, CMD_CTL_CLEAR);
	                }
	            }
	        }

	        if(proc_fn->clear_by_lpn_tick && clock_time_exceed(proc_fn->clear_by_lpn_tick, 5*1000*1000)){
	            // because LPN may retry send clear command when not receive clear comfirm.
                friend_ship_disconnect_fn(i, FS_DISCONNECT_TYPE_CLEAR);
	        }
	    }else{
	        mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)bear;
	        mesh_cmd_nw_t *p_nw = &p_bear->nw;
	        mesh_cmd_lt_ctl_unseg_t *p_lt_ctl_unseg = &p_bear->lt_ctl_unseg;
	        u8 op = p_lt_ctl_unseg->opcode;
	        if(!((CMD_CTL_REQUEST == op)||(CMD_CTL_CLEAR == op)||(CMD_CTL_CLR_CONF == op))
			 && !(proc_fn->status && (fn_other_par[i].LPNAdr == p_nw->src))){
	        	continue ;
	        }

	        if(CMD_CTL_REQUEST == op){
	            mesh_ctl_fri_req_t *p_req = (mesh_ctl_fri_req_t *)(p_lt_ctl_unseg->data);
				#if DEBUG_SUSPEND
	            static u8 mesh_lpn_debug_req;mesh_lpn_debug_req++;
	            static mesh_ctl_fri_req_t mesh_lpn_debug_req_buf;
	            memcpy(&mesh_lpn_debug_req_buf, p_req, sizeof(mesh_ctl_fri_req_t));
				#endif
	            if(0 == mesh_friend_request_is_valid(p_req)){
                    LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_req, sizeof(mesh_ctl_fri_req_t),"Error:rcv Invalid friend request:",0);
	                return ;
	            }
				
				#if DEBUG_SUSPEND
	            static u8 mesh_lpn_debug_req2;mesh_lpn_debug_req2++;
				#endif

	            if(0 == proc_fn->status){
                    mesh_friend_ship_clear_FN(i);   // just init parameters
	            }else if(p_nw->src == fn_other_par[i].LPNAdr){  // was friend before
                    friend_ship_disconnect_fn(i, FS_DISCONNECT_TYPE_RX_REQUEST);
	            }else{
					continue;
	            }
				
				#if DEBUG_SUSPEND
	            static u8 mesh_lpn_debug_req3;mesh_lpn_debug_req3++;
				#endif
	            LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_req, sizeof(mesh_ctl_fri_req_t),"rcv friend request:",0);
	            fn_other_par[i].LPNAdr = p_nw->src;
	            fn_other_par[i].FriAdr = ele_adr_primary;
	            fn_other_par[i].p_cache = &mesh_fri_cache_fifo[i];
	            fn_other_par[i].nk_sel_dec_fn = mesh_key.netkey_sel_dec;
	            fn_other_par[i].TransNo = -1; // init to be different with the first TransNo.
	            fn_poll[i].FSN = 1;           // init to be different with the first poll FSN.
	            memcpy(fn_req+i, p_req,sizeof(mesh_ctl_fri_req_t));
	            proc_fn->offer_tick = clock_time()|1;
	            proc_fn->offer_delay = mesh_friend_local_delay(i);
	            mesh_friend_ship_set_st_fn(i, FRI_ST_OFFER);
	            return ;
	        }else if(CMD_CTL_POLL == op){
	            mesh_ctl_fri_poll_t *p_poll = (mesh_ctl_fri_poll_t *)(p_lt_ctl_unseg->data);
	            if(0 == mesh_friend_poll_is_valid(p_poll)){
                    LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_poll, sizeof(mesh_ctl_fri_poll_t),"Error:rcv Invalid friend poll:",0);
	                return ;
	            }
	            
                if(fn_other_par[i].FriAdr != p_nw->dst){
                    continue ;
                }
				
				#if 0
                static u8 AA_discard_cnt = 0;
                if(AA_discard_cnt){
                    AA_discard_cnt --;
                    return ;
                }
				#endif
	            LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_poll, sizeof(mesh_ctl_fri_poll_t),"rcv friend poll:",0);
	            if(proc_fn->offer_tick){
					fn_other_par[i].link_ok = 1;
	                mesh_friend_ship_proc_init_fn(i);    // init parameters after establish friend ship
	                if(fn_req[i].PreAdr && !is_own_ele(fn_req[i].PreAdr)){
	                    proc_fn->clear_delay_cnt = 1;   // 1: means no delay
	                }
	                friend_ship_establish_ok_cb_fn(i);
	            }else if(proc_fn->clear_by_lpn_tick){
                    friend_ship_disconnect_fn(i, FS_DISCONNECT_TYPE_CLEAR);
                    return ;
	            }
	            
	            my_fifo_t *f_cache = fn_other_par[i].p_cache;
	            u8 *p_br_cache = mesh_friend_ship_cache_check(f_cache);
	            
	            if(fn_poll[i].FSN != p_poll->FSN){
	                fn_poll[i].FSN = p_poll->FSN;
	                if(p_br_cache){
                        if(fn_other_par[i].cache_overwrite){
                            fn_other_par[i].cache_overwrite = 0;
                        }else{
                            my_fifo_pop(f_cache);
	                    }
	                }
	                p_br_cache = mesh_friend_ship_cache_check(f_cache);
	                if(p_br_cache){
                        LOG_MSG_LIB(TL_LOG_FRIEND,p_br_cache+1, 16,"FN Cache message of NW(Big endian):",0);
	                }else{
	                	#if FN_PRIVATE_SEG_CACHE_EN
	                    if(is_friend_seg_cache_busy()){
	                        friend_seg_cache2friend_cache(BIT(i));
	                    }else
	                    #endif
	                    {
	                        friend_cmd_send_fn(i, CMD_CTL_UPDATE);    // push to cache
	                    }
	                    p_br_cache = mesh_friend_ship_cache_check(f_cache);
	                }
	            }else{
                    LOG_MSG_LIB(TL_LOG_FRIEND,0, 0,"FN Cache retry",0);
	            }

                adv_report_extend_t *p_extend = get_adv_report_extend(&p_bear->len);
                u32 timeStamp = p_extend->timeStamp;
                #ifndef WIN32
                if((u32)(clock_time() - timeStamp) > 100*1000*sys_tick_per_us){
                    timeStamp = clock_time();
                }
                #endif
                
	            mesh_friend_set_delay_par_poll(i, p_br_cache, timeStamp);

	            // start poll timeout check
	            mesh_friend_ship_set_st_fn(i, FRI_ST_TIMEOUT_CHECK);
	            mesh_reset_poll_timeout_timer(i);
	            return ;
	        }else if(CMD_CTL_CLEAR == op){
	        	mesh_ctl_fri_clear_t *p_fri_clear = (mesh_ctl_fri_clear_t *)p_lt_ctl_unseg->data;
	            if((fn_other_par[i].LPNAdr == p_fri_clear->LPNAdr)
	            && (p_fri_clear->LPNCounter - fn_req[i].LPNCounter <= 255)){
	            	LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_fri_clear, sizeof(mesh_ctl_fri_clear_t),"rcv friend clear:",0);
	            	if(fn_other_par[i].LPNAdr == p_nw->src){
                        mesh_friend_set_delay_par(DELAY_CLEAR_CONF, i, p_nw->src, p_fri_clear->LPNCounter);
                        proc_fn->clear_by_lpn_tick = clock_time()|1;
					}else{
                        friend_cmd_send_clear_conf(p_nw->src, (u8 *)p_fri_clear, sizeof(mesh_ctl_fri_clear_t));
                        friend_ship_disconnect_fn(i, FS_DISCONNECT_TYPE_CLEAR);
	                }
					return ;
	            }
	        }else if(CMD_CTL_CLR_CONF == op){
	        	mesh_ctl_fri_clear_conf_t *p_clear_conf = (mesh_ctl_fri_clear_conf_t *)p_lt_ctl_unseg->data;
	            if(proc_fn->clear_poll && (fn_other_par[i].LPNAdr == p_clear_conf->LPNAdr)){
					LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_clear_conf, sizeof(mesh_ctl_fri_clear_conf_t),"rcv friend clear confirm:",0);
	                mesh_stop_clear_cmd(i);
					return ;
	            }
	        }else if((CMD_CTL_SUBS_LIST_ADD == op) || (CMD_CTL_SUBS_LIST_REMOVE == op)){
				mesh_ctl_fri_subsc_list_t *p_subsc = CONTAINER_OF(p_lt_ctl_unseg->data,mesh_ctl_fri_subsc_list_t,TransNo);
	            mesh_reset_poll_timeout_timer(i);

	            //if(fn_other_par[i].TransNo != p_subsc->TransNo){
	                u32 subsc_cnt = mesh_subsc_adr_cnt_get(p_bear);
	                u16 adr[SUB_LIST_MAX_IN_ONE_MSG];
	                memcpy(adr, p_subsc->adr, sizeof(adr));
	                if(CMD_CTL_SUBS_LIST_ADD == op){
						LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_subsc->adr, subsc_cnt*2,"rcv friend sub list add:",0);
	                    friend_subsc_list_add_adr((lpn_adr_list_t *)(&fn_other_par[i].SubsList), (lpn_adr_list_t *)adr, subsc_cnt);
	                }else{  // (CMD_CTL_SUBS_LIST_REMOVE == op)
	                	LOG_MSG_LIB(TL_LOG_FRIEND,(u8 *)p_subsc->adr, subsc_cnt*2,"rcv friend sub list remove:",0);
	                    friend_subsc_list_rmv_adr((lpn_adr_list_t *)(&fn_other_par[i].SubsList), (lpn_adr_list_t *)adr, subsc_cnt);
	                }
	            //}
	            mesh_friend_set_delay_par(DELAY_SUBSC_LIST, i, p_nw->src, p_subsc->TransNo);
	            fn_other_par[i].TransNo = p_subsc->TransNo;
	        }else{
                LOG_MSG_LIB(TL_LOG_FRIEND,0, 0,"Error:rcv Invalid op code %2x:",op);
	        }
	    }
	}
}







u8 g_max_lpn_num = MAX_LPN_NUM;

#if 1 // only friend node use
STATIC_ASSERT(MAX_LPN_NUM <= 16);	// because F2L is 16bit

// mesh_friend_seg_cache_t friend_seg_cache;
u8 mesh_fri_cache_fifo_b[MAX_LPN_NUM][(sizeof(mesh_cmd_bear_unseg_t)+DELTA_EXTEND_AND_NORMAL_ALIGN4_BUF) * (1 << FN_CACHE_SIZE_LOG)]={{0}};
my_fifo_t mesh_fri_cache_fifo[MAX_LPN_NUM] = {};//{,,0,0, name##_b};

mesh_ctl_fri_poll_t fn_poll[MAX_LPN_NUM] = {};
mesh_ctl_fri_update_t fn_update[MAX_LPN_NUM] = {};
mesh_ctl_fri_req_t fn_req[MAX_LPN_NUM] = {};
mesh_ctl_fri_offer_t fn_offer[MAX_LPN_NUM] = {};
mesh_fri_ship_other_t fn_other_par[MAX_LPN_NUM] = {};
fn_ctl_rsp_delay_t fn_ctl_rsp_delay[MAX_LPN_NUM] = {};

mesh_fri_ship_proc_fn_t fri_ship_proc_fn[MAX_LPN_NUM] = {};

MYFIFO_INIT(mesh_adv_fifo_fn2lpn, (sizeof(mesh_cmd_bear_unseg_t)+DELTA_EXTEND_AND_NORMAL_ALIGN4_BUF), 4);

#endif

void mesh_feature_set_fn(){
    #if FRI_SAMPLE_EN
	friend_ship_sample_message_test();
    #else
	#if (FEATURE_FRIEND_EN)
	foreach(i, MAX_LPN_NUM){
	    fn_other_par[i].FriAdr = ele_adr_primary;

	    mesh_ctl_fri_update_t *p_update = fn_update+i;
	    memcpy(p_update->IVIndex, iv_idx_st.tx, sizeof(p_update->IVIndex));

	    mesh_ctl_fri_offer_t *p_offer = (fn_offer+i);
	    #if 0
	    if(ele_adr_primary == ADR_FND2){
	        p_offer->RecWin = 0xfa;
	        p_offer->CacheSize = 0x02;
	        p_offer->SubsListSize = 0x05;
	        p_offer->RSSI = -90;    // 0xa6
	        p_offer->FriCounter = 0x000a - 1;  // increase counter later
	    }else
	    #endif
	    {
	        p_offer->RecWin = FRI_REC_WIN_MS; // 200;// 
	        p_offer->CacheSize = 1 << FN_CACHE_SIZE_LOG;
	        p_offer->SubsListSize = SUB_LIST_MAX_LPN;
	        p_offer->RSSI = FN_RSSI_INVALID; // -70;    // 0xba
	        p_offer->FriCounter = 0x072f - 1;  // increase counter later
	    }
	}
	#endif
    #endif
}

#if 0
void fn_update_RecWin(u8 RecWin)
{
#if 0   // no stop friend ship should be better.
    if(!is_fn_support_and_en){
        return;
    }
    
	foreach(i, MAX_LPN_NUM){
	    mesh_ctl_fri_offer_t *p_offer = (fn_offer+i);
	    if(p_offer->RecWin != RecWin){
            p_offer->RecWin = RecWin;
            if(fn_other_par[i].link_ok){
                mesh_friend_ship_proc_init_fn(i);    // stop friend ship
            }
        }
    }
#endif
}

u32 get_RecWin_connected()
{
    return ((get_blt_conn_interval_us()+999)/1000 + 10);
}
#endif

u8 mesh_get_fn_cache_size_log_cnt()
{
    return (1<<FN_CACHE_SIZE_LOG);
}

/**
* friend node: when friend ship establish ok, this function would be called.
* @params: lpn_idx: array index of fn_other_par[].
*/
void friend_ship_establish_ok_cb_fn(u8 lpn_idx)
{

}

/**
* friend node: when friend ship disconnect, this function would be called.
* @params: lpn_idx: array index of fn_other_par[].
* @params: type: disconnect type.for example FS_DISCONNECT_TYPE_POLL_TIMEOUT.
*/
void friend_ship_disconnect_cb_fn(u8 lpn_idx, int type)
{

}


void mesh_global_var_init_fn_buf()
{
	foreach(i,MAX_LPN_NUM){
		mesh_fri_cache_fifo[i].size = (sizeof(mesh_fri_cache_fifo_b[0])/(1 << FN_CACHE_SIZE_LOG));
		mesh_fri_cache_fifo[i].num = (1 << FN_CACHE_SIZE_LOG);
		mesh_fri_cache_fifo[i].wptr = 0;
		mesh_fri_cache_fifo[i].rptr = 0;
		mesh_fri_cache_fifo[i].p = mesh_fri_cache_fifo_b[i];
	}
}
#else
    #if (0 == FEATURE_LOWPOWER_EN)
u8 g_max_lpn_num = 0;   // must 0
mesh_fri_ship_other_t fn_other_par[MAX_LPN_NUM];// = {0};
    #endif

void mesh_iv_update_start_poll_fn(u8 iv_update_by_sno, u8 beacon_iv_update_pkt_flag){}
u16 mesh_group_match_friend(u16 adr){return 0;}
int mesh_tx_cmd_add_packet_fn2lpn(u8 *p_bear){return -1;}
void mesh_friend_ship_init_all(){}
u32 get_poll_timeout_fn(u16 lpn_adr){return 0;}
u16 mesh_mac_match_friend(u16 adr){return 0;}
mesh_fri_ship_other_t * mesh_fri_cmd2cache(u8 *p_bear_big, u8 len_nw, u8 adv_type, u8 trans_par_val, u16 F2L_bit){return 0;}
int is_unicast_friend_msg_to_lpn(mesh_cmd_nw_t *p_nw){return 0;}
void mesh_rc_segment_handle_fn(mesh_match_type_t *p_match_type, mesh_cmd_nw_t *p_nw){}
void mesh_friend_ship_proc_FN(u8 *bear){}
int friend_cache_check_replace(u8 lpn_idx, mesh_cmd_bear_unseg_t *bear_big){return 0;}
u8 get_tx_nk_arr_idx_friend(u16 adr, u16 op){return 0;}
    #if (0 == FEATURE_LOWPOWER_EN)
void mesh_friend_logmsg(mesh_cmd_bear_unseg_t *p_bear_big, u8 len){}
    #endif
int is_unicast_friend_msg_from_lpn(mesh_cmd_nw_t *p_nw){return 0;}
u32 get_current_poll_timeout_timer_fn(u16 lpn_adr){return 0;};

#endif

