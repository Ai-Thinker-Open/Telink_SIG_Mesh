/********************************************************************************************************
 * @file     app.c 
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
#include "proj_lib/rf_drv.h"
#include "proj_lib/pm.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ll/ll_whitelist.h"
#include "proj_lib/ble/trace.h"
#include "proj/mcu/pwm.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj/drivers/adc.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ble_smp.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "../common/app_provison.h"
#include "../common//app_beacon.h"
#include "../common//app_proxy.h"
#include "../common//app_health.h"
#include "../common//vendor_model.h"
#include "proj/drivers/keyboard.h"
#include "app.h"
#include "stack/ble/gap/gap.h"
#include "vendor/common/blt_soft_timer.h"
#include "proj/drivers/rf_pa.h"

#if MI_API_ENABLE
#include "vendor/common/mi_api/telink_sdk_mible_api.h"
#include "vendor/common/mi_api/libs/mesh_auth/mi_service_server.h"
#endif 
#if (HCI_ACCESS==HCI_USE_UART)
#include "proj/drivers/uart.h"
#endif

MYFIFO_INIT(blt_rxfifo, 64, 16);
MYFIFO_INIT(blt_txfifo, 40, 32);



//u8		peer_type;
//u8		peer_mac[12];

u8 mode_config = 0;
extern u8 gatt_adv_send_flag;
//////////////////////////////////////////////////////////////////////////////
//	Initialization: MAC address, Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////

//----------------------- UI ---------------------------------------------
#if (BLT_SOFTWARE_TIMER_ENABLE)
/**
 * @brief   This function is soft timer callback function.
 * @return  <0:delete the timer; ==0:timer use the same interval as prior; >0:timer use the return value as new interval. 
 */
int soft_timer_test0(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;
	static u32 soft_timer_cnt0 = 0;
	soft_timer_cnt0++;
	return 0;
}
#endif


//----------------------- handle BLE event ---------------------------------------------
int app_event_handler (u32 h, u8 *p, int n)
{
	static u32 event_cb_num;
	event_cb_num++;
	int send_to_hci = 1;

	if (h == (HCI_FLAG_EVENT_BT_STD | HCI_EVT_LE_META))		//LE event
	{
		u8 subcode = p[0];
		#if MI_API_ENABLE
		telink_ble_mi_app_event(subcode,p,n);
		#endif 
	//------------ ADV packet --------------------------------------------
		if (subcode == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
		{
			event_adv_report_t *pa = (event_adv_report_t *)p;
			if(LL_TYPE_ADV_NONCONN_IND != (pa->event_type & 0x0F)){
				return 0;
			}

			#if 0 // TESTCASE_FLAG_ENABLE
			u8 mac_pts[] = {0xDA,0xE2,0x08,0xDC,0x1B,0x00};	// 0x001BDC08E2DA
			u8 mac_pts2[] = {0xDB,0xE2,0x08,0xDC,0x1B,0x00};	// 0x001BDC08E2DA
			if(memcmp(pa->mac, mac_pts,6) && memcmp(pa->mac, mac_pts2,6)){
				return 0;
			}
			#endif
			
			#if DEBUG_MESH_DONGLE_IN_VC_EN
			send_to_hci = mesh_dongle_adv_report2vc(pa->data, MESH_ADV_PAYLOAD);
			#else
			send_to_hci = app_event_handler_adv(pa->data, MESH_BEAR_ADV, 1);
			#endif
		}

	//------------ connection complete -------------------------------------
		else if (subcode == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
		{
			event_connection_complete_t *pc = (event_connection_complete_t *)p;
			if (!pc->status)							// status OK
			{
				//app_led_en (pc->handle, 1);

				//peer_type = pc->peer_adr_type;
				//memcpy (peer_mac, pc->mac, 6);
			}
			#if DEBUG_BLE_EVENT_ENABLE
			rf_link_light_event_callback(LGT_CMD_BLE_CONN);
			#endif

			#if DEBUG_MESH_DONGLE_IN_VC_EN
			debug_mesh_report_BLE_st2usb(1);
			#endif
			proxy_cfg_list_init_upon_connection();
			mesh_service_change_report();
		}

	//------------ connection update complete -------------------------------
		else if (subcode == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
		{
		}
	}

	//------------ disconnect -------------------------------------
	else if (h == (HCI_FLAG_EVENT_BT_STD | HCI_EVT_DISCONNECTION_COMPLETE))		//disconnect
	{

		event_disconnection_t	*pd = (event_disconnection_t *)p;
		//app_led_en (pd->handle, 0);
		#if MI_API_ENABLE
		telink_ble_mi_app_event(HCI_EVT_DISCONNECTION_COMPLETE,p,n);
		#endif 
		//terminate reason
		if(pd->reason == HCI_ERR_CONN_TIMEOUT){

		}
		else if(pd->reason == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

		}
		else if(pd->reason == SLAVE_TERMINATE_CONN_ACKED || pd->reason == SLAVE_TERMINATE_CONN_TIMEOUT){

		}
		#if DEBUG_BLE_EVENT_ENABLE
		rf_link_light_event_callback(LGT_CMD_BLE_ADV);
		#endif 

		#if DEBUG_MESH_DONGLE_IN_VC_EN
		debug_mesh_report_BLE_st2usb(0);
		#endif

		mesh_ble_disconnect_cb();
	}

	if (send_to_hci)
	{
		//blc_hci_send_data (h, p, n);
	}

	return 0;
}

void proc_ui()
{
	static u32 tick, scan_io_interval_us = 40000;
	if (!clock_time_exceed (tick, scan_io_interval_us))
	{
		return;
	}
	tick = clock_time();

	#if 0
	static u8 st_sw1_last,st_sw2_last;	
	u8 st_sw1 = !gpio_read(SW1_GPIO);
	u8 st_sw2 = !gpio_read(SW2_GPIO);
	
	if(!(st_sw1_last)&&st_sw1){
	    scan_io_interval_us = 100*1000; // fix dithering
	    access_cmd_onoff(0xffff, 0, G_ON, CMD_NO_ACK, 0);
		foreach(i,NET_KEY_MAX){
					mesh_key.net_key[i][0].node_identity =1;
		}
	}
	st_sw1_last = st_sw1;
	
	if(!(st_sw2_last)&&st_sw2){
	    scan_io_interval_us = 100*1000; // fix dithering
	    access_cmd_onoff(0xffff, 0, G_OFF, CMD_NO_ACK, 0);
	}
	st_sw2_last = st_sw2;

	
	#endif

	#if 0
	static u8 st_sw2_last;	
	u8 st_sw2 = !gpio_read(SW2_GPIO);
	
	if(!(st_sw2_last)&&st_sw2){ // dispatch just when you press the button 
		//trigger the unprivison data packet 
		static u8 beacon_data_num;
		beacon_data_num =1;
		mesh_provision_para_reset();
		while(beacon_data_num--){
			unprov_beacon_send(MESH_UNPROVISION_BEACON_WITH_URI,0);
		}
		provision_mag.initial_pro_roles = MESH_INI_ROLE_NODE;
	    scan_io_interval_us = 100*1000; // fix dithering
	}
	st_sw2_last = st_sw2;
	#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
#if 0
u8 notify_test_flag =0;
u8 notify_test_buf[19];
void test_sig_mesh_cmd_fun()
{
	if(notify_test_flag){
		static u32 tick_notify_test_tick =0;
		static u16 A_debug_sts_level =0;
		int ret_tmp =-1;
		if(!clock_time_exceed(tick_notify_test_tick,10*1000)){
			return;
		}	
		tick_notify_test_tick = clock_time();
		ret_tmp = mesh_tx_cmd_rsp(G_LEVEL_STATUS, (u8 *)&A_debug_sts_level, sizeof(A_debug_sts_level), ele_adr_primary, 
						ele_adr_primary, 0, 0);
		if(A_debug_notify_pkt_sts == BLE_SUCCESS && ret_tmp == 0){
			A_debug_notify_pkt_sts = HCI_ERR_MAC_CONN_FAILED;
			A_debug_sts_level++;
		}
	}
}
#endif

int get_mesh_adv_interval()
{
	u8 *p_buf = my_fifo_get(&mesh_adv_cmd_fifo);
	mesh_cmd_bear_unseg_t *p_bear = (mesh_cmd_bear_unseg_t *)p_buf;
    mesh_transmit_t *p_trans_par = (mesh_transmit_t *)&p_bear->trans_par_val;
	u32 interval_step = 0;
	if(p_bear->type & RSP_DELAY_FLAG){
		extern u8 mesh_rsp_random_delay_step;
		interval_step = mesh_rsp_random_delay_step;
	}
	else{
		interval_step = p_trans_par->invl_steps+1;
	}
	return interval_step*10000;
}

extern void blt_adv_expect_time_refresh(u8 en);
int soft_timer_send_mesh_adv()
{	
	
	blt_adv_expect_time_refresh(0);
	mesh_sleep_time.soft_timer_send_flag = 1;
	blt_send_adv2scan_mode(1);
	mesh_sleep_time.soft_timer_send_flag = 0;
	blt_adv_expect_time_refresh(1);

	if(my_fifo_data_cnt_get(&mesh_adv_cmd_fifo)){
		return get_mesh_adv_interval();
	}
	mesh_sleep_time.soft_timer_pending = 0;
	return -1;
}

void soft_timer_mesh_adv_proc()
{
	if(mesh_sleep_time.soft_timer_pending){
		return;
	}
	
	if(my_fifo_data_cnt_get(&mesh_adv_cmd_fifo)){	
		mesh_sleep_time.soft_timer_pending = 1;
		blt_soft_timer_add(&soft_timer_send_mesh_adv, get_mesh_adv_interval());
	}
	return;
}

void proc_suspend_low_power()
{
	if(!lpn_provision_ok){
	    if(is_provision_success() && node_binding_tick 
	    && clock_time_exceed(node_binding_tick, 3*1000*1000)){
	        lpn_provision_ok = 1;
	        node_binding_tick = 0;
	    }

		if(is_provision_success()){
			return; // don't enter suspend until appkey bind ok.
		}
	}
	
	if(blt_state == BLS_LINK_STATE_CONN){ 
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
		blt_soft_timer_delete(&soft_timer_send_mesh_adv);
	}else if (blt_state == BLS_LINK_STATE_ADV){
		if((!mesh_sleep_time.appWakeup_flg) && clock_time_exceed(mesh_sleep_time.last_tick, mesh_sleep_time.run_time_us)){
			mesh_sleep_time.appWakeup_flg = 0;
			#if BLE_REMOTE_PM_ENABLE
			if(!is_provision_working() ){
				bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
			}
			#endif			
		}
	}
}

void main_loop ()
{
	static u32 tick_loop;

	tick_loop ++;
#if (BLT_SOFTWARE_TIMER_ENABLE)
	soft_timer_mesh_adv_proc();
	blt_soft_timer_process(MAINLOOP_ENTRY);
#endif	
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop ();
	if(mesh_sleep_time.appWakeup_flg && !mesh_tx_seg_par.busy){
		return;//save running time in early wakeup.
	}
	else{
		if(blt_state == BLS_LINK_STATE_ADV){
			extern void bls_phy_scan_mode (int set_chn);
			bls_phy_scan_mode (0); // switch scan channel
		}
	}
	mesh_tx_indication_proc();	
	#if 0 // for indication test
	static u8 A_send_indication=0;
	if(A_send_indication){
		A_send_indication = 0;
		u16 val= 0x3344;
		access_cmd_attr_indication(VD_MSG_ATTR_INDICA,0xffff,ATTR_CURRENT_TEMP, (u8 *)&val, sizeof(val));
	}
	#endif	
	////////////////////////////////////// UI entry /////////////////////////////////
	//  add spp UI task:
#if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(1);
#endif
	proc_ui();
	proc_led();
	factory_reset_cnt_check();
	
	mesh_loop_process();
	#if MI_API_ENABLE
	ev_main();
	#if XIAOMI_MODULE_ENABLE
	mi_api_loop_run();
	#endif
	mi_schd_process();
	#endif 
	
#if ADC_ENABLE
	static u32 adc_check_time;
	if(clock_time_exceed(adc_check_time, 1000*1000)){
		adc_check_time = clock_time();
		static u16 T_adc_val;
	#if(MCU_CORE_TYPE == MCU_CORE_8269)     
		T_adc_val = adc_BatteryValueGet();
	#else
		T_adc_val = adc_sample_and_get_result();
	#endif
	}  
#endif

	//sim_tx_cmd_node2node();

	#if DEBUG_IV_UPDATE_TEST_EN
	iv_index_test_button_firmware();
	#endif
	proc_suspend_low_power();
}

void spirit_lpn_wakeup_init(u8 e, u8 *p, int n)
{
	mesh_sleep_time.appWakeup_flg = bltPm.appWakeup_flg;
	if(!mesh_sleep_time.appWakeup_flg){
		mesh_sleep_time.last_tick = clock_time()|1;
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
	}
}


void spirit_lpn_suspend_enter(u8 e, u8 *p, int n){
	bls_pm_setWakeupSource(PM_WAKEUP_PAD);
}

void spirit_lpn_ui_init(){
	cpu_set_gpio_wakeup(SW1_GPIO, 0, 1);// SW1 switch gatt_mode
	cpu_set_gpio_wakeup(SW2_GPIO, 0, 1);

	bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
}

void user_init()
{
    #if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(0); //battery check must do before OTA relative operation
    #endif
	lpn_provision_ok = is_net_key_save();
	#if DEBUG_EVB_EN
	set_sha256_init_para_mode(1);	// must 1
	#else
		if((MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)||
			(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)||
			(MESH_USER_DEFINE_MODE == MESH_TAIBAI_ENABLE)){
            set_sha256_init_para_mode(1);
		}else if (MESH_USER_DEFINE_MODE == MESH_AES_ENABLE){

		}else{
            set_sha256_init_para_mode(0);// normal mode settings part 
		}
	#endif
	mesh_global_var_init();
	proc_telink_mesh_to_sig_mesh();		// must at first
	set_blc_hci_flag_fun(0);// disable the hci part of for the lib .

	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value and tp value

	usb_id_init();
	usb_log_init();
	usb_dp_pullup_en (1);  //open USB enum

	////////////////// BLE stack initialization ////////////////////////////////////
	ble_mac_init();    

	//link layer initialization
	//bls_ll_init (tbl_mac);
#if(MCU_CORE_TYPE == MCU_CORE_8269)
	blc_ll_initBasicMCU(tbl_mac);   //mandatory
#elif((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(tbl_mac);				//mandatory
#endif
	blc_ll_initAdvertising_module(tbl_mac); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,

#if(BLE_REMOTE_PM_ENABLE)
	blc_ll_initPowerManagement_module();        //pm module:      	 optional
	#if SPIRIT_PRIVATE_LPN_EN
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
	#else
	bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
	#endif
	blc_pm_setDeepsleepRetentionThreshold(50, 30);
	blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);//(SUSPEND_ADV | SUSPEND_CONN)
#endif

	//l2cap initialization
	//blc_l2cap_register_handler (blc_l2cap_packet_receive);
	blc_l2cap_register_handler (app_l2cap_packet_receive); // define the l2cap part 
	///////////////////// USER application initialization ///////////////////

	u8 status = bls_ll_setAdvParam( ADV_INTERVAL_MIN, ADV_INTERVAL_MAX, \
			 	 	 	 	 	     ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, \
			 	 	 	 	 	     0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);

	if(status != BLE_SUCCESS){  //adv setting err
		write_reg8(0x8000, 0x11);  //debug
		while(1);
	}
	
	// normally use this settings 
	blc_ll_setAdvCustomedChannel (37, 38, 39);
	bls_ll_setAdvEnable(1);  //adv enable

	rf_set_power_level_index (MY_RF_POWER_INDEX);
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_ADVERTISING_REPORT|
								HCI_LE_EVT_MASK_CONNECTION_COMPLETE|
								HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE);

	////////////////// SPP initialization ///////////////////////////////////
#if (HCI_ACCESS != HCI_USE_NONE)
	#if (HCI_ACCESS==HCI_USE_USB)
	//blt_set_bluetooth_version (BLUETOOTH_VER_4_2);
	//bls_ll_setAdvChannelMap (BLT_ENABLE_ADV_ALL);
	usb_bulk_drv_init (0);
	blc_register_hci_handler (app_hci_cmd_from_usb, blc_hci_tx_to_usb);
	#elif (HCI_ACCESS == HCI_USE_UART)	//uart
	uart_drv_init();
	blc_register_hci_handler (blc_rx_from_uart, blc_hci_tx_to_uart);		//default handler
	//blc_register_hci_handler(rx_from_uart_cb,tx_to_uart_cb);				//customized uart handler
	#endif
#endif
	#if ADC_ENABLE
	adc_drv_init();
	#endif
	rf_pa_init();
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, (blt_event_callback_t)&mesh_ble_connect_cb);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &spirit_lpn_wakeup_init);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &spirit_lpn_suspend_enter);
	blc_hci_registerControllerEventHandler(app_event_handler);		//register event callback
	//bls_hci_mod_setEventMask_cmd(0xffff);			//enable all 15 events,event list see ble_ll.h
	bls_set_advertise_prepare (app_advertise_prepare_handler);
	//bls_set_update_chn_cb(chn_conn_update_dispatch);
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);

	app_enable_scan_all_device ();	// enable scan adv packet 

	// mesh_mode and layer init
	mesh_init_all();
	// OTA init
	bls_ota_clearNewFwDataArea();	 //must
	//blc_ll_initScanning_module(tbl_mac);
	#if((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	blc_gap_peripheral_init();    //gap initialization
	#endif
	
	mesh_scan_rsp_init();
	my_att_init (provision_mag.gatt_mode);
	blc_att_setServerDataPendingTime_upon_ClientCmd(10);
#if MI_API_ENABLE
	//// used for the telink callback part 
	//extern int mi_mesh_otp_program_simulation();
	//mi_mesh_otp_program_simulation();
	blc_att_setServerDataPendingTime_upon_ClientCmd(1);
	telink_record_part_init();
	#if 0 // XIAOMI_MODULE_ENABLE
	test_mi_api_part(); // just for test
	#endif
	blc_l2cap_register_pre_handler(telink_ble_mi_event_cb_att);// for telink event callback
	advertise_init();
	mi_sevice_init();
	mi_scheduler_init(20, mi_schd_event_handler, NULL, NULL, NULL);
#endif 
	extern u32 system_time_tick;
	system_time_tick = clock_time();
#if TESTCASE_FLAG_ENABLE
	memset(&model_sig_cfg_s.hb_sub, 0x00, sizeof(mesh_heartbeat_sub_str)); // init para for test
#endif

	spirit_lpn_ui_init();
#if (BLT_SOFTWARE_TIMER_ENABLE)
	blt_soft_timer_init();
	//blt_soft_timer_add(&soft_timer_test0, 1*1000*1000);
#endif
}

#if (PM_DEEPSLEEP_RETENTION_ENABLE)
_attribute_ram_code_ void user_init_deepRetn(void)
{
    blc_app_loadCustomizedParameters();
	blc_ll_initBasicMCU();   //mandatory
	spirit_lpn_wakeup_init(0, 0, 0); //bltPm.appWakeup_flg is clear in blc_ll_recoverDeepRetention
	rf_set_power_level_index (MY_RF_POWER_INDEX);
	
	blc_ll_recoverDeepRetention();
#if (HCI_ACCESS == HCI_USE_UART)	//uart
	uart_drv_init();
#endif
#if ADC_ENABLE
	adc_drv_init();
#endif

	spirit_lpn_ui_init();
}
#endif
