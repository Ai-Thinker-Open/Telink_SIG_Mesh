#include "remote_prov.h"
#include "proj/tl_common.h"
#ifndef WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "user_config.h"
#include "lighting_model.h"
#include "sensors_model.h"
#include "lighting_model_LC.h"
#include "mesh_ota.h"
#include "mesh_common.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "proj_lib/mesh_crypto/le_crypto.h"
#if WIN32 
#include "../../../reference/tl_bulk/lib_file/host_fifo.h"
#include "../../../reference/tl_bulk/lib_file/gatt_provision.h"

#endif
#if MI_API_ENABLE 
#include "mesh/mi_api/telink_sdk_mible_api.h"
#endif 

#if MD_REMOTE_PROV
model_remote_prov_t model_remote_prov;
u32 mesh_md_rp_addr = FLASH_ADR_MD_REMOTE_PROV;
rp_mag_str rp_mag;

void mesh_rp_para_init()
{
    memset(&rp_mag,0,sizeof(rp_mag));
}

int mesh_rp_cmd_st_rsp(u16 op_code,u8 *p_rsp,u8 len ,mesh_cb_fun_par_t *cb_par)
{
    model_rp_server_common_t *p_model = (model_rp_server_common_t *)cb_par->model;
    return mesh_tx_cmd_rsp(op_code,p_rsp,len,p_model->com.ele_adr,cb_par->adr_src,0,0);
}

int mesh_cmd_sig_rp_scan_capa(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	remote_prov_scan_cap_sts cap_sts;
	cap_sts.activeScan = ACTIVE_SCAN_ENABLE;
	cap_sts.maxScannedItems = MAX_SCAN_ITEMS_UUID_CNT;
	return mesh_rp_cmd_st_rsp(REMOTE_PROV_SCAN_CAPA_STS,(u8 *)(&cap_sts),sizeof(cap_sts),cb_par);
}

int mesh_cmd_sig_rp_scan_capa_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
	return err;
}

int mesh_cmd_sig_rp_scan_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mesh_cmd_sig_rp_scan_sts(par,par_len,cb_par);
}

int mesh_cmd_sig_rp_scan_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err =-1;
    remote_prov_scan_start *p_scan_start = (remote_prov_scan_start *)par;
    remote_prov_scan_sts_str *p_scan_sts = &(rp_mag.rp_scan_sts);
    if(p_scan_start->scannedItemsLimit >= MAX_SCAN_ITEMS_UUID_CNT){
        p_scan_sts->status = REMOTE_PROV_LIMITED_RES;
        p_scan_sts->PRScanningState = STS_PR_SCAN_IDLE;
        p_scan_sts->scannedItemsLimit = MAX_SCAN_ITEMS_UUID_CNT;
        p_scan_sts->timeout =0;
    }
    if(par_len == 2){// on uuid mode 
        p_scan_sts->PRScanningState = STS_PR_SCAN_MULTI_NODES;
    }else if (par_len == 18){// with uuid mode 
        p_scan_sts->PRScanningState = STS_PR_SCAN_SINGLE_NODE;
        // need to store the uuid part 
        memcpy(p_scan_sts->scan_start_uuid,p_scan_start->uuid,16);
    }else{
        return err;
    }
    p_scan_sts->status = REMOTE_PROV_STS_SUC;
    p_scan_sts->scannedItemsLimit = p_scan_start->scannedItemsLimit;
    p_scan_sts->timeout = p_scan_start->timeout;
    p_scan_sts->tick = clock_time();
    p_scan_sts->src = cb_par->adr_src;
    // need to clean the str of the uuid part .
    rp_scan_report_str *p_rp_report =(rp_mag.rp_rep);
    memset(p_rp_report,0,sizeof(rp_scan_report_str)*MAX_SCAN_ITEMS_UUID_CNT);
    // rsp the scan sts part 
    return mesh_cmd_sig_rp_scan_sts(par,par_len,cb_par);
}

void mesh_cmd_sig_rsp_scan_init()
{
    remote_prov_scan_sts_str *p_scan_sts = &(rp_mag.rp_scan_sts);
    rp_scan_report_str *p_rep = (rp_mag.rp_rep);
    memset((u8*)p_scan_sts,0,sizeof(remote_prov_scan_sts_str));
    memset((u8*)p_rep,0,sizeof(rp_scan_report_str)*MAX_SCAN_ITEMS_UUID_CNT);
    return ;
}

int mesh_cmd_sig_rp_scan_stop(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	int err =-1;
    err = mesh_cmd_sig_rp_scan_sts(par,par_len,cb_par);
	mesh_cmd_sig_rp_scan_report(0,0,0);
	return err;
}

int mesh_cmd_sig_rp_scan_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    remote_prov_scan_sts_str *p_scan = &(rp_mag.rp_scan_sts);
    return mesh_rp_cmd_st_rsp(REMOTE_PROV_SCAN_STS,(u8 *)(p_scan),4,cb_par);
}
u8 remote_prov_get_scan_report_cnt()
{
    u8 cnt=0;
     for(int i=0;i<MAX_SCAN_ITEMS_UUID_CNT;i++){
        rp_scan_report_str *p_rp_report =&(rp_mag.rp_rep[i]);
        if(p_rp_report->valid){
            cnt++;
        }
    }
    return cnt;
}

int remote_prov_find_report_idx(u8 *p_uuid)
{
    int i;
    for(i=0;i<MAX_SCAN_ITEMS_UUID_CNT;i++){
        rp_scan_report_str *p_rp_report =&(rp_mag.rp_rep[i]);
        if(p_rp_report->valid){
            if(!memcmp(p_rp_report->uuid,p_uuid,16)){
                break;
            }
        }
    }
    if(i==4){
        // find the first empty str 
        for(int j=0;j<MAX_SCAN_ITEMS_UUID_CNT;j++){
            rp_scan_report_str *p_rp_report =&(rp_mag.rp_rep[j]);
            if(!p_rp_report->valid){
                return j;
            }
        }
    }
    return -1;
}
void remote_prov_add_report_info(u8 idx,u8 rssi,u8 *p_uuid,u8 * p_oob)
{
    rp_scan_report_str *p_rp_report =&(rp_mag.rp_rep[idx]);
    p_rp_report->valid = 1;
    p_rp_report->rssi = rssi;
    memcpy(p_rp_report->uuid,p_uuid,16);
    memcpy(p_rp_report->oob,p_oob,2);
}

int remote_prov_report_cb(u8 rssi,u8 *p_uuid,u8 * p_oob)
{
    remote_prov_scan_sts_str *p_scan = &(rp_mag.rp_scan_sts);
    if( p_scan->PRScanningState == STS_PR_SCAN_MULTI_NODES){
        // add the report information into the info part 
        int idx =0;
        idx = remote_prov_find_report_idx(p_uuid);
        if(idx>=0){
            remote_prov_add_report_info(idx,rssi,p_uuid,p_oob);
            // get the remote prov report cnt

            u8 rep_cnt = remote_prov_get_scan_report_cnt();
            if(rep_cnt == p_scan->scannedItemsLimit){
                // need to stop the scan part and report 
                // send the scan report part 
                mesh_cmd_sig_rp_scan_report(0,0,0);
            }
        }else{
        }
    }else if (p_scan->PRScanningState == STS_PR_SCAN_SINGLE_NODE){
        // only add one report information into the str part 
        if(!memcmp(p_scan->scan_start_uuid,p_uuid,16)){
            // find the dst uuid beacon ,need to terminate the scan part 
            remote_prov_add_report_info(0,rssi,p_uuid,p_oob);
            // terminate the scan part 
            // send the scan report part 
            mesh_cmd_sig_rp_scan_report(0,0,0);
        }
    }
    return 1;
}

int remote_prov_report_raw_pkt_cb(u8 *p_beacon)
{
    beacon_data_pk *p_adv = (beacon_data_pk *)p_beacon;
    u8 len  = p_adv->header.len;
    u8 rssi = p_beacon[len+1];
    remote_prov_report_cb(rssi,p_adv->device_uuid,p_adv->oob_info);
    return 1;
}


int mesh_cmd_sig_rp_scan_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // just need to enable to send the scan report data
    rp_mag.rp_scan_en = 1;
    return 0;
}

int mesh_cmd_sig_rp_scan_proc()
{
    remote_prov_scan_sts_str *p_scan_sts = &(rp_mag.rp_scan_sts);
    // disptch the timeout para
    if((clock_time_exceed(p_scan_sts->tick,p_scan_sts->timeout*1000*1000)&&
         p_scan_sts->timeout)){
        // the scan produce end
        if( p_scan_sts->PRScanningState == STS_PR_SCAN_MULTI_NODES || 
            p_scan_sts->PRScanningState == STS_PR_SCAN_SINGLE_NODE  ){
            mesh_cmd_sig_rp_scan_report(0,0,0);
        }
        p_scan_sts->timeout =0;
        p_scan_sts->tick = clock_time();
    }
    static u32 idx=0;
    int err =0;
    if(!rp_mag.rp_scan_en){
        return 0;
    }
    if(idx < MAX_SCAN_ITEMS_UUID_CNT){
        if(!is_busy_segment_flow()){// only in this condition it can send cmd 
            rp_scan_report_str *p_rp_report =&(rp_mag.rp_rep[idx]);
            if(p_rp_report->valid){
                err = mesh_tx_cmd_rsp(REMOTE_PROV_SCAN_REPORT,&(p_rp_report->rssi),sizeof(rp_scan_report_str)-1,
                      ele_adr_primary,p_scan_sts->src,0,0);                    
            }
            idx++;
        }
    }else{
        idx = 0;
        rp_mag.rp_scan_en = 0;// disable sending part 
        mesh_cmd_sig_rsp_scan_init();
    }
    return 1;
}

int mesh_cmd_sig_rp_scan_extend_proc()
{
    remote_prov_extend_scan_str *p_ex_scan = &(rp_mag.rp_extend);
    if(clock_time_exceed_s(p_ex_scan->tick,p_ex_scan->timeout)&&p_ex_scan->timeout){
        // timeout 
        memset(p_ex_scan,0,sizeof(remote_prov_extend_scan_str));
    }
    return 1;
}

// extend cmd will reserve to dispatch ??
int mesh_cmd_sig_rp_extend_scan_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    remote_prov_extend_scan_start *p_extend_scan = (remote_prov_extend_scan_start *)par;
    remote_prov_extend_scan_str *p_scan = &(rp_mag.rp_extend);
    p_scan->ADTypeFilterCount = p_extend_scan->ADTypeFilterCount;
    if(p_scan->ADTypeFilterCount<=MAX_ADTYPE_FILTER_CNT){
        memcpy(p_scan->ADTypeFilter,p_extend_scan->ADTypeFilter,p_scan->ADTypeFilterCount);
    }else{
        return -1;
    }
    memcpy(p_scan->uuid,par+1+p_extend_scan->ADTypeFilterCount,16);
    p_scan->timeout = par[17+p_extend_scan->ADTypeFilterCount];
    return 0;
}

int mesh_cmd_sig_rp_extend_scan_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err =-1;
    return err;
}
int mesh_cmd_sig_rp_send_link_sts(mesh_cb_fun_par_t *cb_par)
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    return mesh_rp_cmd_st_rsp(REMOTE_PROV_LINK_STS,(u8 *)(p_link_sts),2,cb_par);

}

int mesh_cmd_sig_rp_send_link_open_sts()
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    return mesh_tx_cmd_rsp(REMOTE_PROV_LINK_STS,(u8 *)(p_link_sts),2,ele_adr_primary,rp_mag.link_adr,0,0);
}


int mesh_cmd_sig_rp_link_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    return mesh_cmd_sig_rp_send_link_sts(cb_par);
}
void mesh_rp_server_set_sts(u8 sts)
{
    remote_proc_pdu_sts_str *p_rp_pdu = &(rp_mag.rp_pdu);
    p_rp_pdu->sts = sts;
}
void mesh_rp_server_pdu_reset()
{
    remote_proc_pdu_sts_str *p_rp_pdu = &(rp_mag.rp_pdu);
    memset(p_rp_pdu,0,sizeof(remote_proc_pdu_sts_str));
}

void mesh_rp_server_set_link_sts(u8 sts)
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    p_link_sts->RPState = sts;    
}

void mesh_rp_server_prov_end_cb()
{
    mesh_rp_server_set_link_sts(STS_PR_LINK_IDLE);
	mesh_rp_server_pdu_reset();
}

int mesh_cmd_sig_rp_link_open(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // trigger to send the link open cmd
    remote_prov_link_open *p_link_open = (remote_prov_link_open *)par;
    if(par_len!=sizeof(remote_prov_link_open)){
        return -1;
    }
    // link sts proc 
    mesh_rp_server_set_link_sts(STS_PR_LINK_OPEN);
    // not use the provision state ,just use the bearer part 
    mesh_adv_prov_link_open(p_link_open->uuid);
    rp_mag.link_adr = cb_par->adr_src;
    mesh_rp_server_set_sts(RP_SER_LINK_OPEN_ACK);
    return 1;
}

int mesh_cmd_sig_rp_link_close(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    // trigger to send the link close cmd 
    remote_prov_link_close *p_link_cls = (remote_prov_link_close *)par;
    if(par_len != sizeof(remote_prov_link_close)){
        return -1;
    }
    mesh_rp_server_set_link_sts(STS_PR_LINK_CLOSE);
    send_rcv_retry_clr();// clear the retry part 
    prov_set_link_close_code(p_link_cls->reason);
    mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
    mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
    mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
    mesh_send_provison_data(BEARS_CTL,LINK_CLOSE,0,0);
    mesh_rp_server_pdu_reset();
    return mesh_cmd_sig_rp_send_link_sts(cb_par);
}

int mesh_cmd_sig_rp_link_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err =-1;
    return err;
}

int mesh_cmd_sig_rp_link_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    return mesh_rp_cmd_st_rsp(REMOTE_PROV_LINK_REPORT,(u8 *)(p_link_sts),sizeof(remote_prov_link_sts_str),cb_par);
}



void mesh_adv_prov_link_open(u8 *p_uuid)
{
    // not use the provision state ,just use the bearer part 
    memcpy(prov_link_uuid,p_uuid,16);
    send_rcv_retry_set(PRO_BEARS_CTL,LINK_OPEN,0); //test case use start_tick in mesh_send_provison_data
    prov_para.link_id = clock_time();
	mesh_send_provison_data(BEARS_CTL,LINK_OPEN,p_uuid,16);
    return ;
}
#define MAX_REMOTE_PROV_TIME    25*1000*1000
u32 mesh_rp_ser_tick =0;
void mesh_rp_ser_tick_set()
{
    mesh_rp_ser_tick = clock_time()|1;
}
void mesh_rp_ser_tick_reset()
{
    mesh_rp_ser_tick = 0;
}
void mesh_rp_ser_tick_loop()
{
#if !WIN32
    if(mesh_rp_ser_tick && clock_time_exceed(mesh_rp_ser_tick,MAX_REMOTE_PROV_TIME)){
		prov_para.link_id_filter = 0;
        mesh_rp_ser_tick_reset();
        mesh_rp_server_pdu_reset();
        mesh_rp_server_prov_end_cb();
	    send_rcv_retry_clr();
		disable_mesh_adv_filter();
    }
#endif
}
void mesh_prov_server_send_cmd(u8 *par,u8 len)
{
#if !WIN32
    remote_proc_pdu_sts_str *p_pdu_sts = &(rp_mag.rp_pdu);
    mesh_pro_data_structer *p_client_rcv = (mesh_pro_data_structer *)(par);
    mesh_pro_data_structer *p_send_str = (mesh_pro_data_structer *)(para_pro);
    u8 prov_cmd = p_client_rcv->invite.header.type;
    switch(p_pdu_sts->sts){
        case RP_SER_INVITE_SEND:
            if(prov_cmd == PRO_INVITE){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_INVITE);
                mesh_rp_ser_tick_set();// timeout start tick 
                mesh_adv_prov_send_invite(p_send_str);
                mesh_rp_server_set_sts(RP_SER_INVITE_ACK);
				// only enable the rcv of the provision data part 
				enable_mesh_adv_filter();
				prov_para.link_id_filter =1;
            }
            break;
        case RP_SER_START_SEND:
            if(prov_cmd == PRO_START){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_START);
				mesh_prov_pdu_send_retry_clear();
                mesh_adv_prov_send_start_cmd(p_send_str,&(p_client_rcv->start));
                mesh_rp_server_set_sts(RP_SER_START_ACK);
				enable_mesh_adv_filter();
            }
            break;
        case RP_SER_PUBKEY_SEND:
            if(prov_cmd == PRO_PUB_KEY){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_PUB_KEY);
				mesh_prov_pdu_send_retry_clear();
                mesh_adv_prov_pubkey_send(p_send_str,(p_client_rcv->pubkey.pubKeyX));
                mesh_rp_server_set_sts(PR_SER_PUBKEY_ACK);
				enable_mesh_adv_filter();
            }
            break;
        case PR_SER_COMFIRM_SEND:
            if(prov_cmd == PRO_CONFIRM){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_CONFIRM);
				mesh_prov_pdu_send_retry_clear();
                mesh_adv_prov_comfirm_cmd(p_send_str,(p_client_rcv->comfirm.comfirm));
                mesh_rp_server_set_sts(PR_SER_COMFIRM_SEND_ACK);
				enable_mesh_adv_filter();
            }
            break;
        case PR_SER_RANDOM_SEND:
            if(prov_cmd == PRO_RANDOM){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_RANDOM);
				mesh_prov_pdu_send_retry_clear();
                mesh_adv_prov_random_cmd(p_send_str,(p_client_rcv->random.random));
                mesh_rp_server_set_sts(PR_SER_RANDOM_SEND_ACK);
				enable_mesh_adv_filter();
            }
            break;
        case PR_SER_DATA_SEND:
            if(prov_cmd == PRO_DATA){
                LAYER_PARA_DEBUG(A_DEBUG_PRO_DATA);
				mesh_prov_pdu_send_retry_clear();
                mesh_adv_prov_data_send(p_send_str,(p_client_rcv->data.encProData));
                mesh_rp_server_set_sts(PR_SER_DATA_SEND_ACK);   
				enable_mesh_adv_filter();
            }
            break;
        default:
            break;
    }
#endif
    return ;
}

int mesh_prov_server_to_client_cmd(pro_PB_ADV *p_adv)
{
    u8 pro_cmd = p_adv->transStart.data[0];
	u8 len = get_mesh_pro_str_len(pro_cmd);
    return mesh_cmd_sig_send_rp_pdu_send(p_adv->transStart.data,len);
}

void mesh_prov_pdu_send_retry_proc()
{
    remote_prov_retry_str *p_retry = &(rp_mag.rp_pdu.re_send);
    if( p_retry->retry_flag && 
        (p_retry->tick!=0) && 
        clock_time_exceed(p_retry->tick ,REMOTE_PROV_SERVER_RETRY_INTER)&&
        !is_busy_segment_flow()){
        if(p_retry->retry_flag == REMOTE_PROV_SERVER_CMD_FLAG){
            p_retry->tick = clock_time()|1;     
            mesh_prov_server_to_client_cmd(&(p_retry->adv));
        }else if (p_retry->retry_flag == REMOTE_PROV_SERVER_OUTBOUND_FLAG){
            // send only once 
            mesh_cmd_sig_rp_pdu_outbound_send();
            memset(p_retry,0,sizeof(remote_prov_retry_str));
            
        }
    }
}

void mesh_prov_pdu_send_retry_set(pro_PB_ADV *p_adv,u8 flag)
{
    remote_prov_retry_str *p_retry = &(rp_mag.rp_pdu.re_send);
    p_retry->retry_flag =flag;
    p_retry->tick =(clock_time()-BIT(30))|1;
    memcpy(&(p_retry->adv),p_adv,sizeof(pro_PB_ADV));
    // send first cmd should flow the segment not busy 
}

void mesh_prov_pdu_send_retry_clear()
{
	remote_prov_retry_str *p_retry = &(rp_mag.rp_pdu.re_send);
	memset(p_retry,0,sizeof(remote_prov_retry_str));
	return ;
}

void mesh_prov_server_rcv_cmd(pro_PB_ADV *p_adv)
{
#if !WIN32
    remote_proc_pdu_sts_str *p_pdu_sts = &(rp_mag.rp_pdu);
    switch(p_pdu_sts->sts){
        case RP_SER_LINK_OPEN_ACK:
            if( p_adv->transBear.bearAck.header.GPCF == BEARS_CTL &&
				p_adv->transBear.bearAck.header.BearCtl == LINK_ACK){
				LAYER_PARA_DEBUG(A_DEBUG_LINK_OPEN_ACK);
				mesh_cmd_sig_rp_send_link_open_sts();
                mesh_adv_prov_link_open_ack(p_adv);
				disable_mesh_adv_filter();
                mesh_rp_server_set_sts(RP_SER_INVITE_SEND); 
			}
			break;
	    case RP_SER_INVITE_ACK:
	        if( p_adv->transAck.GPCF == TRANS_ACK && 
	            p_adv->trans_num >= prov_para.trans_num_last){
	            LAYER_PARA_DEBUG(A_DEBUG_INVITE_ACK);
                mesh_adv_prov_invite_ack(p_adv);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
                mesh_rp_server_set_sts(PR_SER_CAPA_RCV); 
				
	        }
	        break;
	    case PR_SER_CAPA_RCV:
	        if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_CAPABLI){
				LAYER_PARA_DEBUG(A_DEBUG_PRO_CAPABLI);
                mesh_adv_prov_capa_rcv(p_adv);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_CMD_FLAG);
				disable_mesh_adv_filter();
                mesh_rp_server_set_sts(RP_SER_START_SEND); 
				
			}
			break;
	    case RP_SER_START_ACK:
	        if( p_adv->transAck.GPCF == TRANS_ACK && 
	            p_adv->trans_num >= prov_para.trans_num_last){
	            LAYER_PARA_DEBUG(A_DEBUG_START_ACK);
                mesh_adv_prov_start_ack(p_adv);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
				disable_mesh_adv_filter();
                mesh_rp_server_set_sts(RP_SER_PUBKEY_SEND);
                
	        }
	        break;
	    case PR_SER_PUBKEY_ACK:
	        if( p_adv->transAck.GPCF == TRANS_ACK && 
	            p_adv->trans_num >= prov_para.trans_num_last){
	            LAYER_PARA_DEBUG(A_DEBUG_PUBKEY_ACK);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
                mesh_rp_server_set_sts(PR_SER_PUBKEY_RSP);
                
	        }
	        break;
	    case PR_SER_PUBKEY_RSP:
	        if( p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_PUB_KEY){
				LAYER_PARA_DEBUG(A_DEBUG_PUBKEY_RSP);
				send_rcv_retry_clr();// when we need to send cmd ,we should stop provision cmd send first
				mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_CMD_FLAG);
                mesh_adv_prov_pubkey_rsp(p_adv);
				disable_mesh_adv_filter();
                mesh_rp_server_set_sts(PR_SER_COMFIRM_SEND);
                
		    }
		    break;
		case PR_SER_COMFIRM_SEND_ACK:
		    if( p_adv->transAck.GPCF == TRANS_ACK && 
		        p_adv->trans_num >= prov_para.trans_num_last){
		        LAYER_PARA_DEBUG(A_DEBUG_COMFIRM_SEND_ACK);
				mesh_adv_prov_comfirm_ack(p_adv);
				mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
                mesh_rp_server_set_sts(PR_SER_COMFIRM_RSP);
				
		    }
		    break;
		case PR_SER_COMFIRM_RSP:
		    if( p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_CONFIRM){
				LAYER_PARA_DEBUG(A_DEBUG_COMFIRM_RSP);
				mesh_adv_prov_comfirm_rsp(p_adv);
				send_rcv_retry_clr();// when we need to send cmd ,we should stop provision cmd send first
				mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_CMD_FLAG);
				disable_mesh_adv_filter();
				mesh_rp_server_set_sts(PR_SER_RANDOM_SEND);
				
		    }
		    break;
		case PR_SER_RANDOM_SEND_ACK:
		    if(p_adv->transAck.GPCF == TRANS_ACK&&p_adv->trans_num >= prov_para.trans_num_last){
                LAYER_PARA_DEBUG(A_DEBUG_RANDOM_SEND_ACK);
				mesh_adv_prov_random_ack_cmd(p_adv);
				mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
				mesh_rp_server_set_sts(PR_SER_RANDOM_RSP);	
			}
            break;
        case PR_SER_RANDOM_RSP:
            if( p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_RANDOM){
				LAYER_PARA_DEBUG(A_DEBUG_RANDOM_RSP);
                mesh_adv_prov_random_rsp(p_adv);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_CMD_FLAG);
				disable_mesh_adv_filter();
                mesh_rp_server_set_sts(PR_SER_DATA_SEND);
		    }
		    break;
        case PR_SER_DATA_SEND_ACK:
            if( p_adv->transAck.GPCF == TRANS_ACK && 
                p_adv->trans_num >= prov_para.trans_num_last){
                LAYER_PARA_DEBUG(A_DEBUG_DATA_SEND_ACK);
                mesh_adv_prov_data_ack(p_adv);
                mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_OUTBOUND_FLAG);
                mesh_rp_server_set_sts(PR_SER_COMPLETE_RSP);	
            }
            break;
        case PR_SER_COMPLETE_RSP:
            if(p_adv->transBear.bearAck.header.GPCF == TRANS_START &&
				p_adv->transStart.data[0]== PRO_COMPLETE){
				// return the test case result 
				    LAYER_PARA_DEBUG(A_DEBUG_PRO_COMPLETE);
					p_pdu_sts->sts = PR_SER_COMPLETE_SUC;
					prov_para.link_id_filter = 0;
				    mesh_rp_ser_tick_reset();// timeout start tick 
				    mesh_rp_adv_prov_complete_rsp(p_adv);
				    mesh_prov_pdu_send_retry_set(p_adv,REMOTE_PROV_SERVER_CMD_FLAG);
					mesh_prov_end_set_tick();// trigger event callback part 
					disable_mesh_adv_filter();
				}
			break;	
	    default:
	        break;
    }
#endif
    return ;
}


int mesh_cmd_sig_rp_prov_pdu_send(u8 *par,int par_len)
{
    if(par_len > sizeof(pro_trans_pubkey)){
        return 0;
    }
    mesh_prov_server_send_cmd(par,par_len);
    return 1;
}

int mesh_cmd_sig_rp_pdu_outbound_send()
{
    int err =-1;
    remote_proc_pdu_sts_str *p_pdu_sts = &(rp_mag.rp_pdu);
    err = mesh_tx_cmd_rsp(REMOTE_PROV_PDU_OUTBOUND_REPORT,(u8 *)&(p_pdu_sts->outbound),
                                                1,ele_adr_primary,p_pdu_sts->src,0,0);
    return err;
}

int mesh_cmd_sig_rp_pdu_send(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    remote_prov_pdu_send *p_pdu = (remote_prov_pdu_send *)par;
    remote_proc_pdu_sts_str *p_pdu_sts = &(rp_mag.rp_pdu);
    p_pdu_sts->outbound = p_pdu->OutboundPDUNumber;
    p_pdu_sts->src = cb_par->adr_src;
    // send provision data part 
    mesh_cmd_sig_rp_prov_pdu_send(p_pdu->ProvisioningPDU,par_len-1);
    p_link_sts->RPState = STS_PR_OUTBOUND_TRANS;
    // need to dispatch by the node provisioner self 
    p_link_sts->RPState = STS_PR_LINK_OPEN;
    return 0;
}

int mesh_cmd_sig_rp_pdu_outbound_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err =-1;
    return err;
}

u8 mesh_pr_sts_work_or_not()
{
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    if( p_link_sts->RPState == STS_PR_LINK_OPEN||
        p_link_sts->RPState ==STS_PR_OUTBOUND_TRANS  ){
        return 1;
    }else{
        return 0;
    }
}

int mesh_cmd_sig_send_rp_pdu_send(u8 *par,int par_len)
{
    int err =-1;
    remote_prov_link_sts_str *p_link_sts = &(rp_mag.rp_link);
    if( p_link_sts->RPState!=STS_PR_LINK_OPEN ||
        p_link_sts->RPState!=STS_PR_OUTBOUND_TRANS   ){
        // link layer not suc 
       // return err;
    }
    remote_proc_pdu_sts_str *p_pdu_sts =&(rp_mag.rp_pdu);
    if(par_len > sizeof(pro_trans_pubkey)){
      //  return -1;
    }
    u8 prov_pdu[sizeof(pro_trans_pubkey)];
    prov_pdu[0]= p_pdu_sts->inbound;
    memcpy(prov_pdu+1,par,par_len);
    err = mesh_tx_cmd_rsp(REMOTE_PROV_PDU_REPORT,prov_pdu,par_len+1,ele_adr_primary,p_pdu_sts->src,0,0);
    if(err != -1){
        // send suc 
        p_pdu_sts->inbound++;
    }
    return err;
}

int mesh_cmd_sig_rp_pdu_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    int err =-1;
    return err;
}

int mesh_remote_prov_st_publish(u8 idx)
{
	return 1;
}


void mesh_cmd_sig_rp_server_loop_proc()
{
    mesh_prov_pdu_send_retry_proc();
    mesh_cmd_sig_rp_scan_proc();
    mesh_cmd_sig_rp_scan_extend_proc();
}


u8 mesh_rsp_opcode_is_rp(u16 opcode)
{
    if(opcode >= REMOTE_PROV_SCAN_CAPA_GET && opcode<=REMOTE_PROV_PDU_REPORT){
        return 1;
    }else{
        return 0;
    }
}
#endif

// remote prov client proc part ,only concern about the gatt provision part 

#if WIN32
rp_mag_cli_str rp_client;

int mesh_cmd_sig_rp_cli_send_capa(u16 node_adr)
{
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_SCAN_CAPA_GET, 0, 0);
}

int mesh_cmd_sig_rp_cli_send_scan_get(u16 node_adr)
{
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_SCAN_GET, 0, 0);
}

int mesh_cmd_sig_rp_cli_send_scan_start(u16 node_adr,u8 scan_limit,u8 timeout,u8 *p_uuid)
{
    remote_prov_scan_start scan_start;
    scan_start.scannedItemsLimit = scan_limit;
    scan_start.timeout = timeout;
    memcpy(scan_start.uuid,p_uuid,16);
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_SCAN_GET, (u8 *)(&scan_start), sizeof(scan_start));
}

int mesh_cmd_sig_rp_cli_send_scan_stop(u16 node_adr)
{
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_SCAN_STOP, 0, 0);
}

int mesh_cmd_sig_rp_cli_send_extend_scan_start(u16 node_adr,u8 adcnt,
                                                             u8 *p_adfilter,u8 *p_uuid,u8 timeout)
{
    remote_prov_extend_scan_start ex_scan_start;
    ex_scan_start.ADTypeFilterCount = adcnt;
    memcpy(ex_scan_start.ADTypeFilter,p_adfilter,adcnt);
    // get the uuid pointer part
    u8 *p_start = (u8 *)(&ex_scan_start)+1+adcnt;
    memcpy(p_start,p_uuid,16);
    p_start[17] = timeout;
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_EXTEND_SCAN_START, 
                                (u8 *)&ex_scan_start, 18+adcnt);

}

int mesh_cmd_sig_rp_cli_send_link_get(u16 node_adr)
{
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_LINK_GET, 0, 0);
}
void mesh_rp_proc_en(u8 en)
{
    rp_client.rp_flag = en;
}

u8 get_mesh_rp_proc_en()
{
    return rp_client.rp_flag;
}

void mesh_rp_proc_set_node_adr(u16 unicast)
{
    rp_client.node_adr = unicast;
}

int mesh_cmd_sig_rp_cli_send_link_open(u16 node_adr,u8 *p_uuid)
{
    remote_prov_link_open link_open;
    memcpy(link_open.uuid,p_uuid,16);
    rp_client.rp_flag =1;
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_LINK_OPEN, (u8 *)&link_open,sizeof(link_open));
}

int mesh_cmd_sig_rp_cli_send_link_close(u16 node_adr,u8 reason)
{
    remote_prov_link_close link_cls;
    link_cls.reason = reason;
    rp_client.rp_flag =0;
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_LINK_CLOSE, (u8 *)&link_cls, sizeof(link_cls));
}

int mesh_cmd_sig_rp_cli_send_prov_pdu(u16 node_adr,u8 *p_pdu,u16 len)
{
    remote_prov_pdu_send prov_pdu;
    prov_pdu.OutboundPDUNumber =rp_client.outbound;
    rp_client.outbound++;
    memcpy(prov_pdu.ProvisioningPDU,p_pdu,len);
    return SendOpParaDebug(node_adr, 1, REMOTE_PROV_PDU_SEND, (u8 *)&prov_pdu, len+1);
}

void mesh_rp_client_set_retry_send()
{
    rp_mag_cli_str *p_rp = &rp_client;
    p_rp->retry_flag = 1;
    p_rp->tick = (clock_time()-BIT(30))|1;
}

int mesh_cmd_sig_cp_cli_send_invite(u8 *p,u16 len)
{
    rp_client.prov_sts = RP_PROV_CAPA_RSP;
    LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:SEND_RP_PROV_INVITE_CMD",0);
    return mesh_cmd_sig_rp_cli_send_pdu(p,len);
    
}

int mesh_cmd_sig_rp_cli_send_pdu(u8 *p_pdu,u16 len)
{
    mesh_rp_client_set_retry_send();
    return 1;
}

int mesh_cmd_sig_rp_cli_send_pdu_direct(u8 *p_pdu,u16 len)
{
    return mesh_cmd_sig_rp_cli_send_prov_pdu(rp_client.node_adr,p_pdu,len);
}
void mesh_rp_client_para_reset()
{
    memset(&rp_client,0,sizeof(rp_client));
}

void mesh_rp_client_set_prov_sts(u8 sts)
{
    rp_mag_cli_str *p_rp = &rp_client;
    p_rp->prov_sts = sts;
}


int mesh_rp_client_rx_cb(u8 * p_data,u16 unicast)
{
    rp_mag_cli_str *p_rp = &rp_client;
    remote_prov_pdu_report *p_event = (remote_prov_pdu_report *)p_data;
    u8 *p_rp_data = (p_data+2);
    if(p_event->opcode == REMOTE_PROV_SCAN_CAPA_STS){
        remote_prov_scan_cap_sts *p_capa = (remote_prov_scan_cap_sts *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:REMOTE_PROV_SCAN_CAPA_STS,\
                maxScannedItems is 0x%x,activeScan is 0x%x",p_capa->maxScannedItems,p_capa->activeScan);
        remote_prov_capa_sts_cb(p_capa->maxScannedItems,p_capa->activeScan);
    }else if (p_event->opcode == REMOTE_PROV_SCAN_STS){
        remote_prov_scan_sts *p_scan = (remote_prov_scan_sts *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,(u8 *)p_scan,sizeof(remote_prov_scan_sts),"CLIENT:REMOTE_PROV_SCAN_STS",0);
    }else if (p_event->opcode == REMOTE_PROV_SCAN_REPORT){
        remote_prov_scan_report *p_report = (remote_prov_scan_report *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,(u8 *)p_report,sizeof(remote_prov_scan_report),"CLIENT:REMOTE_PROV_SCAN_REPORT",0);
        remote_prov_scan_report_win32 rep_win32;
        rep_win32.unicast = unicast;
        memcpy((u8 *)&(rep_win32.scan_report),p_report,sizeof(remote_prov_scan_report));
        remote_prov_scan_report_cb((u8 *)&rep_win32,sizeof(remote_prov_scan_report_win32));        
    }else if (p_event->opcode == REMOTE_PROV_EXTEND_SCAN_REPORT){
        remote_prov_extend_scan_report *p_report = (remote_prov_extend_scan_report *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,(u8 *)p_report,sizeof(remote_prov_extend_scan_report),"CLIENT:REMOTE_PROV_EXTEND_SCAN_REPORT",0);
    }else if (p_event->opcode == REMOTE_PROV_LINK_STS){
        remote_prov_link_status *p_link_sts = (remote_prov_link_status *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,(u8 *)p_link_sts,sizeof(remote_prov_link_status),"CLIENT:REMOTE_PROV_LINK_STS",0);
    }else if (p_event->opcode == REMOTE_PROV_LINK_REPORT){ 
        remote_prov_link_report *p_link_rep = (remote_prov_link_report *)p_rp_data;
        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,(u8 *)p_link_rep,sizeof(remote_prov_link_report),"CLIENT:REMOTE_PROV_LINK_REPORT",0);
    }else if(p_event->opcode == REMOTE_PROV_PDU_OUTBOUND_REPORT || REMOTE_PROV_PDU_REPORT){
        mesh_pro_data_structer *p_send = (mesh_pro_data_structer *)gatt_para_pro;
        mesh_pro_data_structer *p_rcv = (mesh_pro_data_structer *)p_event->ProvisioningPDU;
        u8 prov_code = p_event->ProvisioningPDU[0];
        if(prov_code == PRO_FAIL){
            // terminate,need to send link close cmd 
            // reset all the status part 
            LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:PRO_FAIL",0);
            mesh_rp_client_para_reset();
            mesh_cmd_sig_rp_cli_send_link_close(rp_client.node_adr,UNEXPECT_ERROR);        
        }
        //prov the rp sts ,and the rp opcode part 
        switch(p_rp->prov_sts){
            // rcv capa and send start cmd 
            case RP_PROV_CAPA_RSP:
                if(p_event->opcode == REMOTE_PROV_PDU_REPORT){
                    // receive the capa cmd 
                    if(prov_code == PRO_CAPABLI){
                        gatt_prov_rcv_capa(p_rcv,0);
                        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_CAPA_RSP",0);
                        mesh_rp_client_set_prov_sts(RP_PROV_START_CMD);
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            case RP_PROV_START_CMD:
                // need to send the rp prov start cmd 
                gatt_prov_send_start(p_rcv,p_send);
                mesh_cmd_sig_rp_cli_send_pdu(gatt_para_pro,gatt_para_len);
                LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_START_CMD",0);
                mesh_rp_client_set_prov_sts(RP_PROV_PUBKEY_CMD);
                break;
            case RP_PROV_PUBKEY_CMD:
                if(p_event->opcode == REMOTE_PROV_PDU_OUTBOUND_REPORT){
                    gatt_prov_send_pubkey(p_rcv,p_send);
                    mesh_cmd_sig_rp_cli_send_pdu(gatt_para_pro,gatt_para_len);
                    LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_PUBKEY_CMD",0);
                    mesh_rp_client_set_prov_sts(RP_PROV_PUBKEY_RSP);
                }
                break;
            case RP_PROV_PUBKEY_RSP:
                if(p_event->opcode == REMOTE_PROV_PDU_REPORT){
                    // receive the pubkey from the node 
                    if(prov_code == PRO_PUB_KEY){
                        gatt_prov_rcv_pubkey(p_rcv,p_send);
                        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_PUBKEY_RSP",0);
                        mesh_rp_client_set_prov_sts(RP_PROV_COMFIRM_CMD);
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            case RP_PROV_COMFIRM_CMD:
                gatt_prov_send_comfirm(p_rcv,p_send);
                mesh_cmd_sig_rp_cli_send_pdu(gatt_para_pro,gatt_para_len);
                LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_COMFIRM_CMD",0);
                mesh_rp_client_set_prov_sts(RP_PROV_COMFIRM_RSP);
                break;
            case RP_PROV_COMFIRM_RSP:
                if(p_event->opcode == REMOTE_PROV_PDU_REPORT){
                    // receive the pubkey from the node 
                    if(prov_code == PRO_CONFIRM){
                        gatt_prov_rcv_comfirm(p_rcv,p_send);
                        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_COMFIRM_RSP",0);
                        mesh_rp_client_set_prov_sts(RP_PROV_RANDOM_CMD);
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            case RP_PROV_RANDOM_CMD:
                gatt_prov_send_random(p_rcv,p_send);
                mesh_cmd_sig_rp_cli_send_pdu(gatt_para_pro,gatt_para_len);
                LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_RANDOM_CMD",0);
                mesh_rp_client_set_prov_sts(RP_PROV_RANDOM_RSP);
                break;
            case RP_PROV_RANDOM_RSP:
                if(p_event->opcode == REMOTE_PROV_PDU_REPORT){
                    // receive the pubkey from the node 
                    if(prov_code == PRO_RANDOM){
                        gatt_prov_rcv_random(p_rcv,p_send);
                        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_RANDOM_RSP",0);
                        mesh_rp_client_set_prov_sts(RP_PROV_DATA_CMD);
                    }else{
                        break;
                    }
                }else{
                    break;
                }
            case RP_PROV_DATA_CMD:
                gatt_prov_send_pro_data(0,p_send);
                mesh_cmd_sig_rp_cli_send_pdu(gatt_para_pro,gatt_para_len);
                LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_DATA_CMD",0);
                mesh_rp_client_set_prov_sts(RP_PROV_COMPLETE_RSP);
                break;
            case RP_PROV_COMPLETE_RSP:
                if(p_event->opcode == REMOTE_PROV_PDU_REPORT){
                    // receive the pubkey from the node 
                    if(prov_code == PRO_COMPLETE){
                        prov_timer_clr();
                        gatt_prov_rcv_pro_complete();	
                        provision_end_callback(PROV_NORMAL_RET);
                        gatt_provision_mag.provison_send_state = STATE_PRO_SUC;
                        mesh_rp_client_para_reset();
                        LOG_MSG_INFO(TL_LOG_REMOTE_PROV,0,0,"CLIENT:RP_PROV_COMPLETE_RSP",0);
                        cache_init(0xffff);
                    }
                }
                break;
            default: break;
               
        } 
    }
	return 1;
}

void mesh_rp_start_settings(u16 adr,u8 *p_uuid)
{
    prov_timer_start();
    mesh_rp_proc_en(1);
	mesh_rp_proc_set_node_adr(adr);
	mesh_cmd_sig_rp_cli_send_link_open(adr,p_uuid);
	mesh_rp_client_set_prov_sts(RP_PROV_IDLE_STS);
}

void mesh_rp_pdu_retry_send()
{
    rp_mag_cli_str *p_rp = &rp_client;
    if(p_rp->retry_flag){
        if( p_rp->tick!=0 && 
            clock_time_exceed(p_rp->tick,REMOTE_PROV_PDU_CLI_INTER)&&
            !is_busy_segment_flow()&&
            !mesh_tx_reliable.busy){
            p_rp->tick = clock_time()|1;
            mesh_cmd_sig_rp_cli_send_pdu_direct(gatt_para_pro,gatt_para_len);
        }
    }
}
void mesh_rp_pdu_retry_clear()
{
    rp_mag_cli_str *p_rp = &rp_client;
    p_rp->retry_flag =0;
    p_rp->tick =0;
}
#endif

#if MD_REMOTE_PROV
void mesh_cmd_sig_rp_loop_proc()
{
    #if WIN32
    mesh_rp_pdu_retry_send();
    #else
    mesh_rp_ser_tick_loop();
    mesh_cmd_sig_rp_server_loop_proc();
    #endif
}
#endif






