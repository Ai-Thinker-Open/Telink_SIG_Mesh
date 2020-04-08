/********************************************************************************************************
 * @file     interface.h 
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
// gerneral defination 
#include "../../../reference/tl_bulk/lib_file/Gatt_provision.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "app_mesh.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/pm.h"
#include "mesh/app_proxy.h"
#include "mesh/app_health.h"


/************************************output function *************************************/

/************************** get_lib_version **************************
function :get the version of the sig-mesh lib version part  
para:   none
ret: the version value 
******************************************************************/	
unsigned int get_lib_version();

/************************** test_pragma_pack **************************
function :get the pack size of the lib to make sure the pack is right  
para:   none
ret: the value is 7
******************************************************************/	
unsigned char test_pragma_pack();

/************************** master_clock_init **************************
function :init the master clock part ,and get the clock of the master ,as a integer of u32 
para:   none
******************************************************************/	
void  master_clock_init();

/************************** mesh_init_all **************************
function :init sig-mesh stack para init. 
para:   none
******************************************************************/	
void mesh_init_all();

/************************** Thread_main_process **************************
function :the loop process for sigmesh stack part ,should run by per seconds for a loop ,we suggest 2s 
para:   none
******************************************************************/	
void Thread_main_process();

/************************** provision_dispatch_direct **************************
function :use the proxy protocol to pack the notify data into one packet .
para:  
        p : the input notify data of the buffer ,to dispatch the data part 
        len : the input notify data  of the proxy data.
        proxy_buf: the output buffer of the proxy data.
        p_proxy_len:the output buffer length of the proxy data part 
return 
        MSG_COMPOSITE_WAIT:  the proxy buffer is not complete ,need to wait for the next pdu
        MSG_NETWORK_PDU: the proxy data type is network packet 
        MSG_MESH_BEACON: the proxy data type is a mesh beacon packet 
        MSG_PROXY_CONFIG: the proxy data type is a proxy config packet 
        MSG_PROVISION_PDU: the proxy data type is a provision packet
******************************************************************/	

u8 provision_dispatch_direct(u8 *p,u8 len,u8* proxy_buf,u16* p_proxy_len);

/************************** gatt_rcv_pro_pkt_dispatch **************************
function :gatt provision function ,input the gatt buffer into this function ,it can proc the gatt-provision bearer part 
para:  
        p : the input buffer of the provision data
        len : the input buffer length of the provision data part 
return 
    
******************************************************************/	
void gatt_rcv_pro_pkt_dispatch(u8 *p ,u8 len );

/************************** push_notify_into_fifo **********************
function :push the notify data into fifo part  
para:  
        p : the input buffer of the provision data
        len : the input buffer length of the provision data part 
return 
******************************************************************/	
void push_notify_into_fifo(u8 *p ,u32 len );

/************************** flash_erase_512K **********************
function :erase all the config file part in the app part 
para:  
       
return 
******************************************************************/	
void flash_erase_512K();
extern mesh_cfg_cmd_sub_set_par_t mesh_cfg_cmd_sub_set_par;

/************************** mesh_construct_adv_bear_with_nw *********
function :push the network layer data into the bear data part 
para:  
        bear: the bearer part data 
        nw: the network data pointer
        len_nw: the length of the network data layer part 
       
return : the length of the bearer layer data length
******************************************************************/	
int mesh_construct_adv_bear_with_nw(u8 *bear, u8 *nw, u8 len_nw);

/************************** write_no_rsps_pkts *********
function :write the data into the handle .
para:  
        p: the pointer of the data will put into the send buffer part  
        len: the len of the pointer length part 
        handle: the handle which the ble master will use(in android and ios will find by uuid part )
        msg_type:   
                        MSG_NETWORK_PDU: the proxy data type is network packet 
                        MSG_MESH_BEACON: the proxy data type is a mesh beacon packet 
                        MSG_PROXY_CONFIG: the proxy data type is a proxy config packet 
                        MSG_PROVISION_PDU: the proxy data type is a provision packet 
return : the length of the bearer layer data length
******************************************************************/	
void write_no_rsps_pkts(u8 *p,u16 len,u16 handle,u8 msg_type);

/************************** set_app_key_pro_data *********
function :set the provision data .
para:  
        p_dat: the pointer of the provision data .
        len : the length of the provision data .
return : the length of the bearer layer data length.
******************************************************************/	
u8 set_app_key_pro_data(u8 *p_dat,u8 len);

/************************** start_provision_invite *********
function :start the provision procedure .
para:  
return : none
******************************************************************/	
void start_provision_invite();


/************************** app_event_handler_adv *********
function :proc the  mesh packet 
para:  

            p_payload: the payload of the mesh packet 
            src_type: ADV_FROM_GATT the packet from the gatt-provision bearer 
            need_proxy_and_trans_par_val: the ble master ,this value should be set to 0
return :        
           -1: err
           0: OK
******************************************************************/
int app_event_handler_adv(u8 *p_payload, int src_type, u8 need_proxy_and_trans_par_val);

/************************** clock_time_exceed *********
function :the clock time proc function part 
para:  
          ref: the ref tick in the clock time part 
          span_us: the inter tick of the clock time part 
return :        
          1: means the now tick pass the ref by span_us
          0: means the now tick not pass the ref by span_us
******************************************************************/

unsigned int  clock_time_exceed(unsigned int ref, unsigned int span_us);

/************************** clock_time *********
function :get the tick of the u32 tick by the sys tick part 
para:  
       
return :        
          the tick now 
******************************************************************/

unsigned int  clock_time();



/************************** mesh_provision_par_set_dir **************************
function : set the provisioner's internal node provision data  
para:
	typedef struct{
		u8  net_work_key[16];
		u16  key_index;
		u8  flags;
		u8  iv_index[4];
		u16  unicast_address;
	}provison_net_info_str;
ret: 0  means OK 
	-1 or other value means err
****************************************************************************/	
int mesh_provision_par_set_dir(u8 *prov_par);

/************************** check_pkt_is_unprovision_beacon **************************
function : check the pkt is unprovision beacon or not    
para:
	dat: the pointer of the rcv pkt part 
ret: 0  means OK 
	-1 or other value means err
****************************************************************************/
int check_pkt_is_unprovision_beacon(u8 *dat);

/************************** cfg_cmd_reset_node **************************
function : send the cmd to reset the node part    
para:
	node_adr: the node adr of the mesh node 
ret: 0      means OK 
	-1    other value means err
****************************************************************************/

int cfg_cmd_reset_node(u16 node_adr);

/************************** mesh_proxy_set_filter_init **************************
function :gatt provisioner set the filter list to white list ,and add the slef adr to the white list table    
para:
	self_adr: the unicast adr of the gatt provisioner.
ret: 0      means OK 
	-1    other value means err
****************************************************************************/
int mesh_proxy_set_filter_init(u16 self_adr);

/************************** mesh_kc_cfgcl_mode_para_set **************************
function :gatt provisioner set the appkey part ,and add the netkey idx to a unicast adr    
para:
	apk_idx: the appkey idx part 
	p_appkey: the appkey value (16 bytes )
	unicast : the unicast adr of the node 
	nk_idx: the netkey idx 
	fast_bind: 
	        1: the node should bind it automatically 
	        0: the node should proc normal mode 
ret: 0      means OK 
	-1    other value means err
****************************************************************************/
void mesh_kc_cfgcl_mode_para_set(u16 apk_idx,u8 *p_appkey,u16 unicast,u16 nk_idx,u8 fast_bind);

/************************** master_terminate_ble_callback **************************
function :the callback interface when master terminate the ble connection part   
para:
ret: none
****************************************************************************/
void master_terminate_ble_callback();

/************************** set_gatt_pro_cloud_en **************************
function :set the cloud caculate mode part    
para:
ret: none
****************************************************************************/
void set_gatt_pro_cloud_en(u8 en);

/************************** set_random_enable **************************
function :enable the provision key enable part   
para:
        en :1 enable ,0 disable 
ret: none
****************************************************************************/
void set_random_enable(u8 en);

/************************** gatt_get_node_ele_cnt **************************
function :get the element cnt and data.  
para:
        unicast : the unicast adr of the node
        p_ele: the element pointer of the ele part
ret: the ele cnt of the node 
****************************************************************************/
int gatt_get_node_ele_cnt(u16 unicast ,u8 *p_ele);


/*******************json_get_node_cps_info****************************
function :get the composition data of the node ,and it can call when appkey bind endcallback 
para:
        unicast_adr: the input para ,the node's unicast adr part 
        p_node: the pointer of the composition data of the node part 
return :
        the node idx part of the vc node part 
**************************************************************/
u8 json_get_node_cps_info(u16 unicast_adr,VC_node_info_t *p_node);


/*******************lum2level****************************
function :transfer the lum to level 
para:
       lum: the lightness percent
return :
       the level of the value
**************************************************************/

extern s16 lum2level(u8 lum);


/*******************level2lum****************************
function :transfer the level to lum
para:
       level: the light level
return :
       the lum of the light 
**************************************************************/

extern u8 level2lum(s16 level);

/*******************lum2_lightness****************************
function :transfer the lum to lightness
para:
       lum: the lum 
return :
       lightness 
**************************************************************/

extern u16 lum2_lightness(u8 lum);


/*******************lightness2_lum****************************
function :transfer the lightness to lum 
para:
       lightness: 
return :
       lum 
**************************************************************/

extern u8 lightness2_lum(u16 lightness);

/*******************temp100_to_temp****************************
function :transfer temp100 to the ct temp
para:
       temp100: 
return :
       ct temp 
**************************************************************/
extern u16 temp100_to_temp(u8 temp100);

/*******************temp_to_temp100****************************
function :transfer ct  temp to the temp100
para:
       ct temp: 
return :
       temp100 
**************************************************************/
extern u8 temp_to_temp100(u16 temp);

void mesh_rp_start_settings(u16 adr,u8 *p_uuid);





/*************************input function *************************
**************************************************************/

/******************* mesh_tx_reliable_stop_report ****************************
function :printf the reliable stop cmd part 
para:
        op: opcode part 
        rsp_max: the rsp the cmd need 
        rsp_cnt: the cnt of the rsp actually 
return :
        none
**************************************************************/
void mesh_tx_reliable_stop_report(u16 op, u16 adr_dst,u32 rsp_max, u32 rsp_cnt);

/******************* SendPkt ****************************
function :master send the ble packet part 
para:
        handle: the write handle ,android and ios will use the uuid to get the handle part 
        p: the pointer of the send packet part 
        len : the length of the send packet 
return :
        none
**************************************************************/
extern unsigned char SendPkt(unsigned short handle,unsigned char *p ,unsigned char  len);

/******************* OnAppendLog_vs ****************************
function :app data callback part .
para:
        p: the pointer of the rcv data part 
        len : the length of the rcv data packet 
return :
        none
**************************************************************/
extern int OnAppendLog_vs(unsigned char *p, int len);

/******************* clock_time_vc_hw ****************************
function :realize the clock time u32 by the upper machine 
para:
return :
        none
**************************************************************/
extern unsigned int  clock_time_vc_hw();

/******************* flash_write_page ****************************
function :write the cfg file by the way of using adr 
para:
        addr: the addr of the cfg file part 
        len : the length of the write file 
        buf: the pointer of the buf
return :
        none
**************************************************************/

extern void flash_write_page(u32 addr, u32 len, const u8 *buf);

/******************* flash_read_page ****************************
function :read the cfg file by the way of using adr 
para:
        addr: the addr of the cfg file part 
        len : the length of the write file 
        buf: the pointer of the buf
return :
        none
**************************************************************/
extern void flash_read_page(u32 addr, u32 len, u8 *buf);

/******************* RefreshStatusNotifyByHw ****************************
function :update the status part 
para:
        p: the pointer of the refresh status 
        len: the length of the mesh status 
return :
        none
**************************************************************/
extern void RefreshStatusNotifyByHw(unsigned char *p, int len);

/******************* LogMsgModuleDlg_and_buf ****************************
function :the interface about the log msg printf part  
para:
        pbuf: the printf data part 
        len: the length of print data 
        log_str: the string about the log
        format: the prinf special char part ,such as %d
        list: the parameter list part 
return :
        none
**************************************************************/
extern int LogMsgModuleDlg_and_buf(u8 *pbuf,int len,char *log_str,char *format, va_list list);

/******************* App_key_bind_end_callback ****************************
function :the end callback about the key bind proc 
para:
       event:
            MESH_APP_KEY_BIND_EVENT_SUC: keybind suc 
            MESH_APP_KEY_BIND_EVENT_TIMEOUT: keybind timeout
return :
        not define 
**************************************************************/

extern int App_key_bind_end_callback(u8 event);

/******************* provision_end_callback ****************************
function :the end callback about the provision proc 
para:
       reason:
           PROV_NORMAL_RET =0,
	    PROV_TIMEOUT_RET =1,
	    PROV_COMFIRM_ERR =2,
	    PROV_FAIL_CMD_ERR =3,
return :
        not define 
**************************************************************/
extern void provision_end_callback(u8 reason);

/******************* mesh_proxy_master_terminate_cmd ****************************
function :to reliaze the ble master can terminate the ble connection  
para:
      
return :
        not define 
**************************************************************/
extern void mesh_proxy_master_terminate_cmd();
/******************* mesh_proxy_master_terminate_cmd ****************************
function :to set the pid and the mac part 
para:
      
return :
        not define 
**************************************************************/
extern int mesh_set_prov_cloud_para(u8 *p_pid,u8 *p_mac);
/**************************mesh_sec_prov_cloud_comfirm***************************
function : use the para of the provision comfirm  key and the provision random to caculate the  comfirm value
para:
	p_comfirm: the result of the comfirm value (16 bytes) (out)
	p_comfirm_key: the pointer of the comfirm key (16 bytes)(in)
	pro_random: the pointer of the random value (16 bytes)(in)
ret: 1  means OK 
	not use the return 
notice: it should wait the result of the comfirm value before the function return ,or the flow will be error
	
******************************************************************************/
extern int mesh_sec_prov_cloud_comfirm(u8* p_comfirm,u8 *p_comfirm_key,u8 *pro_random);
/**************************mesh_cloud_dev_comfirm_check***************************
function : use the para of the comfirm key and the dev random to calculate the device random ,and compare with the random
		value receive from the device .
para:
	p_comfirm_key: the pointer of the comfirm key (16 bytes)(in)
	p_dev_random: the device random send by the device(16 bytes)(in)
	pro_random: the pointer of the random value (16 bytes)(in)
ret: 1  means OK 
	0  will terminate the gatt connection 
notice: it should wait the result of the comfirm value before the function return ,or the flow will be error
	
******************************************************************************/
extern int mesh_cloud_dev_comfirm_check(u8 *p_comfirm_key,u8* p_dev_random,
								u8*p_dev_comfirm);
/**************************gatt_provision_net_info_callback***************************
function : when the provision enter the provision data send ,it will cause this callback ,will used to set 
		the provision data para ,and we will use the gatt_provision_net_info_callback to set .
para:
ret: void
notice: it should wait the result of the comfirm value before the function return ,or the flow will be error
	
******************************************************************************/
extern void gatt_provision_net_info_callback();
/**************************set_gatt_provision_net_info***************************
function : set the provision parameters ,and use in the gatt_provision_net_info_callback
para:
	p_netkey: the pointer of the netkey (16 bytes )(in)
	p_key_index: the pointer of the key index (2 bytes)(in)
	p_flag: the pointer of the flags about the netkey (1 bytes)(in)
	p_ivi: the pointer of the ivi index (4 bytes)(in)
	p_unicast: the pointer of the unicast address(2 bytes)(in)
ret: void
notice: it should wait the result of the comfirm value before the function return ,or the flow will be error
******************************************************************************/
extern void set_gatt_provision_net_info(u8 p_netkey[16],u16*p_key_index,u8*p_flag,u8 p_ivi[4],u16*p_unicast);

/**************************model_need_key_bind_whitelist***************************
function : set the model filter part ,and return the list part 
para:
key_bind_list_buf: the list of the models filter
p_list_cnt: the cnt of the models need to bind 
max_cnt: the max cnt need to bind 
ret:not concern
notice: should add 1 byte header for divided message, use the hanlde according to the msg_type for write transation
******************************************************************************/
extern u8 model_need_key_bind_whitelist(u16 *key_bind_list_buf,u8 *p_list_cnt,u8 max_cnt);

/**************************gatt_write_transaction_callback***************************
function : divide message according to the MTU size, Messages sent using the Proxy protocol
proxy_PDU:
-----------------------
|field  |	size(bits)|
-----------------------
|SAR	|	2		  |
-----------------------
|type	|	6	      |
-----------------------
|data	| variable	  |
-----------------------
para:
p:pointer to message
len: message length
msg_type:message type
ret:not concern
notice: should add 1 byte header for divided message, use the hanlde according to the msg_type for write transation
******************************************************************************/
extern int gatt_write_transaction_callback(u8 *p,u16 len,u8 msg_type);
/**************************mesh_par_retrieve_store_win32***************************
function : retrieve and store the information about the flash information part  
para:
in_out: the pointer the store and retrieve 
p_adr: the adr to operate  
adr_base: the base adr to operate 
size:the size of the pointer to operate
flag: the flag to indicate to retrieve or store
        #define MESH_PARA_RETRIEVE_VAL          1
        #define MESH_PARA_STORE_VAL             0
ret: 0 suc  other err
******************************************************************************/

extern int mesh_par_retrieve_store_win32(u8 *in_out, u32 *p_adr, u32 adr_base, u32 size,u8 flag);

// need to reliaze 
u8 win32_proved_state();


void mesh_heartbeat_cb_data(u16 src, u16 dst,u8 *p_hb);


void remote_prov_capa_sts_cb(u8 max_item,u8 active_scan);
void remote_prov_scan_report_cb(u8 *par,u8 len);
u16 get_win32_prov_unicast_adr();
int set_win32_prov_unicast_adr(u16 adr);
u8 get_win32_ele_cnt(u8 pid);



