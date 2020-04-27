#ifndef _REMOTE_PROV_H
#define _REMOTE_PROV_H

#include "proj/tl_common.h"
#include "app/app.h"
// #include "mesh_lpn/app.h"
// #include "mesh_provision/app.h"
// #include "mesh_switch/app.h"
#include "mesh_lpn.h"
#include "mesh_fn.h"
#include "time_model.h"
#include "scheduler.h"
#include "mesh_property.h"

#define REMOTE_PROV_SCAN_ITEM_CNT   4

enum{
    STS_PR_SCAN_IDLE = 0,
    STS_PR_SCAN_MULTI_NODES = 1,
    STS_PR_SCAN_SINGLE_NODE = 2,
    STS_PR_SCAN_RFU =3
};

enum{
    STS_PR_LINK_IDLE =0,
    STS_PR_LINK_OPEN =1,
    STS_PR_LINK_ACTIVE =2,
    STS_PR_OUTBOUND_TRANS =3,
    STS_PR_LINK_CLOSE =4,
    STS_PR_PROHIBIT =5
};

typedef struct{
    u8 maxScannedItems;
    u8 activeScan;
}remote_prov_scan_cap_sts;

typedef struct{
    u8 scannedItemsLimit;
    u8 timeout;
    u8 uuid[16];
}remote_prov_scan_start;

typedef struct{
    u8 status;
    u8 prScanningState;
    u8 scannedItemsLimit;
    u8 timeout;
}remote_prov_scan_sts;

typedef struct{
    s8 rssi;
    u8 uuid[16];
    u16 oob;
}remote_prov_scan_report;

typedef struct{
    u16 unicast;
    remote_prov_scan_report scan_report;
}remote_prov_scan_report_win32;

typedef struct{
    u8 ADTypeFilterCount;
    u8 ADTypeFilter[8];
    u8 uuid[16];
    u8 timeout;
}remote_prov_extend_scan_start;

typedef struct{
    u8 status;
    u8 uuid[16];
    u16 OOBinformation;
    u8 AdvStructures[1];
}remote_prov_extend_scan_report;

typedef struct{
    u8 uuid[16];
}remote_prov_link_open;


enum{
    STS_PR_LINK_CLS_SUC =0,
    STS_PR_LINK_CLS_PROHI = 1,
    STS_PR_LINK_CLS_FAIL =2,
    STS_PR_LINK_CLS_RFU =3
};

typedef struct{
    u8 reason;
}remote_prov_link_close;

typedef struct{
    u8 status;
    u8 RPstate;
}remote_prov_link_status;

typedef struct{
    u8 status;
    u8 RPstatus;
    u8 reasons;
}remote_prov_link_report;

typedef struct{
    u8 OutboundPDUNumber;
    u8 ProvisioningPDU[0x48];
}remote_prov_pdu_send;

typedef struct{
    u8 OutboundPDUNumber;
}remote_prov_pdu_outbound_report;

typedef struct{
    u16 opcode;
    u8 InboundPDUNumber;
    u8 ProvisioningPDU[0x48];
}remote_prov_pdu_report;

#define REMOTE_PROV_SCAN_CAPA_GET   0x4F80
#define REMOTE_PROV_SCAN_CAPA_STS   0x5080
#define REMOTE_PROV_SCAN_GET        0x5180
#define REMOTE_PROV_SCAN_START      0x5280
#define REMOTE_PROV_SCAN_STOP       0x5380
#define REMOTE_PROV_SCAN_STS        0x5480
#define REMOTE_PROV_SCAN_REPORT     0x5580
#define REMOTE_PROV_EXTEND_SCAN_START   0x5680
#define REMOTE_PROV_EXTEND_SCAN_REPORT  0x5780
#define REMOTE_PROV_LINK_GET        0x5880
#define REMOTE_PROV_LINK_OPEN       0x5980
#define REMOTE_PROV_LINK_CLOSE      0x5A80
#define REMOTE_PROV_LINK_STS        0x5B80
#define REMOTE_PROV_LINK_REPORT     0x5C80
#define REMOTE_PROV_PDU_SEND        0x5D80
#define REMOTE_PROV_PDU_OUTBOUND_REPORT     0x5E80
#define REMOTE_PROV_PDU_REPORT      0x5F80

enum{
    REMOTE_PROV_STS_SUC = 0,
    REMOTE_PROV_SCAN_NOT_START = 1,
    REMOTE_PROV_INVALID_STS = 2,
    REMOTE_PROV_LIMITED_RES = 3,
    REMOTE_PROV_LINK_CANNOT_OPEN = 4,
    REMOTE_PROV_LINK_OPEN_FAIL = 5,
    REMOTE_PROV_LINK_CLOSE_BY_DEVICE = 6,
    REMOTE_PROV_LINK_CLOSE_BY_SERVER = 7,
    REMOTE_PROV_LINK_CLOSE_BY_CLIENT = 8,
    REMOTE_PROV_LINK_CANNOT_RCV = 9,
    REMOTE_PROV_LINK_CANNOT_SEND = 0x0A,
    REMOTE_PROV_LINK_CANNOT_DELIVER_PDU_REP = 0x0B,
    REMOTE_PROV_LINK_CANNOT_DELIVER_OUTBOUND_REP = 0x0C,
    REMOTE_PROV_STS_RFU = 0x0D
};
typedef struct{
	model_common_t com;
	u8 rfu;
}model_rp_server_common_t;

typedef struct{
	model_common_t com;
	u8 rfu;
}model_rp_client_common_t;

typedef struct{
    #if MD_SERVER_EN
    model_rp_server_common_t srv[1];
    #endif
    #if MD_CLIENT_EN
    model_rp_client_common_t client[1];
    #endif
}model_remote_prov_t;
extern model_remote_prov_t model_remote_prov;


#define MAX_SCAN_ITEMS_UUID_CNT 4
#define ACTIVE_SCAN_ENABLE  0
typedef struct{
    u8 status ;
    u8 PRScanningState;
    u8 scannedItemsLimit;
    u8 timeout;
    u8 scan_start_uuid[16];
    u32 tick;
    u16 src;
}remote_prov_scan_sts_str;

typedef struct{
    u8 valid;
    u8 rssi;
    u8 uuid[16];
    u8 oob[2];
}rp_scan_report_str;

typedef struct{
    u8 status;
    u8 uuid[16];
    u8 oob_info[2];
    u8 adv_str[13];
}rp_extend_scan_report_str;
#define MAX_ADTYPE_FILTER_CNT   8
typedef struct{
    u8 ADTypeFilterCount;
    u8 ADTypeFilter[MAX_ADTYPE_FILTER_CNT];
    u8 uuid[16];
    u8 timeout;
    u32 tick;
    rp_extend_scan_report_str report;
}remote_prov_extend_scan_str;

typedef struct{
    u8 status;
    u8 RPState;
    u8 reason;
}remote_prov_link_sts_str;

#define REMOTE_PROV_SERVER_CMD_FLAG         0x80
#define REMOTE_PROV_SERVER_OUTBOUND_FLAG    0x40

#define REMOTE_PROV_SERVER_RETRY_INTER  3000*1000
typedef struct{
    u8 retry_flag;
    u32 tick;
    pro_PB_ADV adv;    
}remote_prov_retry_str;

typedef struct{
    u8 outbound;
    u8 inbound;
    u16 src;
    u8 sts;
    remote_prov_retry_str re_send;
}remote_proc_pdu_sts_str;


typedef struct{
    //capa sts part 
    u8 active_scan;
    //scan sts part 
    u8 rp_scan_en;
    remote_prov_scan_sts_str rp_scan_sts;
    //report sts part 
    rp_scan_report_str rp_rep[MAX_SCAN_ITEMS_UUID_CNT];
    //extend scan sts part 
    remote_prov_extend_scan_str rp_extend;
    // link sts part 
    remote_prov_link_sts_str rp_link;
    // rp link adr 
    u16 link_adr;
    // remote prov sts 
    remote_proc_pdu_sts_str rp_pdu;
}rp_mag_str;
// remote provision server sts 
typedef enum{
    RP_SER_IDLE =0,
    RP_SER_LINK_OPEN_SEND,
    RP_SER_LINK_OPEN_ACK,
    RP_SER_INVITE_SEND,
    RP_SER_INVITE_ACK,
    PR_SER_CAPA_RCV,
    RP_SER_START_SEND,
    RP_SER_START_ACK,
    RP_SER_PUBKEY_SEND,
    PR_SER_PUBKEY_ACK,
    PR_SER_PUBKEY_RSP,
    PR_SER_COMFIRM_SEND,
    PR_SER_COMFIRM_SEND_ACK,
    PR_SER_COMFIRM_RSP,
    PR_SER_RANDOM_SEND,
    PR_SER_RANDOM_SEND_ACK,
    PR_SER_RANDOM_RSP,
    PR_SER_DATA_SEND,
    PR_SER_DATA_SEND_ACK,
    PR_SER_COMPLETE_RSP,
    PR_SER_COMPLETE_SUC,
}REMOTE_PROV_SERVER_ENUM;



void mesh_rp_para_init();
int mesh_cmd_sig_rp_scan_capa(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_capa_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_stop(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_scan_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_extend_scan_start(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_extend_scan_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_link_get(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_link_open(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_link_close(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_link_sts(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_link_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_pdu_send(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_pdu_outbound_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int mesh_cmd_sig_rp_pdu_report(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par);
int remote_prov_report_cb(u8 rssi,u8 *p_uuid,u8 * p_oob);
int remote_prov_report_raw_pkt_cb(u8 *p_beacon);
void mesh_cmd_sig_rp_loop_proc();
u8 mesh_pr_sts_work_or_not();
int mesh_cmd_sig_send_rp_pdu_send(u8 *par,int par_len);
void mesh_rp_client_para_reset();
void mesh_prov_pdu_send_retry_clear();





// remote prov client part 
typedef struct{
    //capa sts part 
    u8 outbound;
    u8 rp_flag;
    u16 node_adr;
    u8 prov_sts;
    u8 retry_flag;
    u32 tick;
}rp_mag_cli_str;

typedef enum{
    RP_PROV_IDLE_STS =0,
    RP_PROV_INVITE_CMD  = 1,
    RP_PROV_INVITE_CMD_ACK ,
    RP_PROV_CAPA_RSP    ,
    RP_PROV_START_CMD   ,
    RP_PROV_START_CMD_ACK,
    RP_PROV_PUBKEY_CMD,
    RP_PROV_PUBKEY_CMD_ACK,
    RP_PROV_PUBKEY_RSP,
    RP_PROV_COMFIRM_CMD,
    RP_PROV_COMFIRM_CMD_ACK,
    RP_PROV_COMFIRM_RSP,
    RP_PROV_RANDOM_CMD,
    RP_PROV_RANDOM_CMD_ACK,
    RP_PROV_RANDOM_RSP,
    RP_PROV_DATA_CMD,
    RP_PROV_DATA_CMD_ACK,
    RP_PROV_COMPLETE_RSP,
}RP_PROV_CMD_ENUM;

#define REMOTE_PROV_PDU_CLI_INTER  6000*1000

int mesh_cmd_sig_rp_cli_send_capa(u16 node_adr);
int mesh_cmd_sig_rp_cli_send_scan_get(u16 node_adr);
int mesh_cmd_sig_rp_cli_send_scan_start(u16 node_adr,u8 scan_limit,u8 timeout,u8 *p_uuid);
int mesh_cmd_sig_rp_cli_send_scan_stop(u16 node_adr);
int mesh_cmd_sig_rp_cli_send_extend_scan_start(u16 node_adr,u8 adcnt,
                          u8 *p_adfilter,u8 *p_uuid,u8 timeout);
int mesh_cmd_sig_cp_cli_send_invite(u8 *p,u16 len);
int mesh_cmd_sig_rp_cli_send_pdu(u8 *p_pdu,u16 len);
void mesh_rp_proc_en(u8 en);
void mesh_rp_proc_set_node_adr(u16 unicast);
int mesh_cmd_sig_rp_cli_send_link_open(u16 node_adr,u8 *p_uuid);
int mesh_cmd_sig_rp_cli_send_link_close(u16 node_adr,u8 reason);
u8 get_mesh_rp_proc_en();
u8 mesh_rsp_opcode_is_rp(u16 opcode);
int mesh_rp_client_rx_cb(u8 * p_data,u16 unicast);
int mesh_remote_prov_st_publish(u8 idx);
void mesh_rp_client_set_prov_sts(u8 sts);
void mesh_rp_server_set_sts(u8 sts);
void mesh_prov_server_send_cmd(u8 *par,u8 len);
void mesh_prov_server_rcv_cmd(pro_PB_ADV *p_adv);
int mesh_cmd_sig_rp_pdu_outbound_send();
void remote_prov_scan_report_cb(u8 *par,u8 len);
extern model_remote_prov_t model_remote_prov;
extern u32 mesh_md_rp_addr ;
void mesh_rp_start_settings(u16 adr,u8 *p_uuid);
void remote_prov_capa_sts_cb(u8 max_item,u8 active_scan);
void mesh_rp_pdu_retry_clear();
void mesh_rp_server_pdu_reset();
void mesh_rp_server_prov_end_cb();


#endif

