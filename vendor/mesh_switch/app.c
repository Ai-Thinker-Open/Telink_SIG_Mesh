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
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj/drivers/adc.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ble_smp.h"
#include "proj_lib/mesh_crypto/mesh_crypto.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"

#include "proj_lib/sig_mesh/app_mesh.h"
#include "../common/app_provison.h"
#include "../common/app_beacon.h"
#include "../common/app_proxy.h"
#include "../common/app_health.h"
#include "proj/drivers/keyboard.h"
#include "app.h"
#include "stack/ble/gap/gap.h"
#include "vendor/common/blt_soft_timer.h"
#include "proj/drivers/rf_pa.h"

#if (HCI_ACCESS==HCI_USE_UART)
#include "proj/drivers/uart.h"
#endif

#if AIS_ENABLE
#define BLT_RX_BUF_NUM	16
#else
#define BLT_RX_BUF_NUM	8
#endif
MYFIFO_INIT(blt_rxfifo, 64, BLT_RX_BUF_NUM);
MYFIFO_INIT(blt_txfifo, 40, 16);

//u8		peer_type;
//u8		peer_mac[12];

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

	//------------ ADV packet --------------------------------------------
		if (subcode == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
		{
			event_adv_report_t *pa = (event_adv_report_t *)p;
			if(LL_TYPE_ADV_NONCONN_IND != (pa->event_type & 0x0F)){
				return 0;
			}
			
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
	static u32 tick_scan, scan_io_interval_us = 4000;
	if (!clock_time_exceed (tick_scan, scan_io_interval_us))
	{
		return;
	}
	tick_scan = clock_time();

	mesh_proc_keyboard();
}

rc_para_mag rc_mag;
#if MULTI_ADDR_FOR_SWITCH_EN
u8 multi_addr_key_flag = 0;
#endif
static int	rc_repeat_key = 0;
static int	rc_key_pressed;
static int	rc_long_pressed;
static u32  mesh_active_time;
static u8   key_released =1;
#define		ACTIVE_INTERVAL				32000
#define		ADV_TIMEOUT					(30*1000) //30s
void mesh_proc_keyboard ()
{
	static u32		tick_key_pressed, tick_key_repeat;
	static u8		kb_last[2];
	extern kb_data_t	kb_event;
	kb_event.keycode[0] = 0;
	kb_event.keycode[1] = 0;
	int det_key = kb_scan_key (0, 1);
	///////////////////////////////////////////////////////////////////////////////////////
	//			key change:pressed or released
	///////////////////////////////////////////////////////////////////////////////////////
	if (det_key) 	{
		/////////////////////////// key pressed  /////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////
		if (kb_event.keycode[0]) {
			if(key_released && rc_mag.adv_timeout_def_ms == ADV_TIMEOUT){
				rc_mag.adv_timeout_def_ms = 0;// press any key can exit adv mode.
			}
		    mesh_active_time = clock_time();
            if ((kb_event.keycode[0] == RC_KEY_A_ON && kb_event.keycode[1] == RC_KEY_1_OFF) ||
                (kb_event.keycode[1] == RC_KEY_A_ON && kb_event.keycode[0] == RC_KEY_1_OFF))
		    {
		    	// trigger the adv mode ,enterring configuration mode
		    	
			}else if ((kb_event.keycode[0] == RC_KEY_A_ON && kb_event.keycode[1] == RC_KEY_4_OFF) ||
                (kb_event.keycode[1] == RC_KEY_A_ON && kb_event.keycode[0] == RC_KEY_4_OFF))
		    {
                //irq_disable();
		    }else{
		    	int report_flag = 1;
		        u8 cmd[11] = {0};
    			if (kb_event.keycode[0] == RC_KEY_1_ON){
    			    cmd[0] = LGT_CMD_LIGHT_ON;
    			}else if (kb_event.keycode[0] == RC_KEY_2_ON){
    			    cmd[0] = LGT_CMD_LIGHT_ON;
    			}else if (kb_event.keycode[0] == RC_KEY_3_ON){
    			    cmd[0] = LGT_CMD_LIGHT_ON;
    			}else if (kb_event.keycode[0] == RC_KEY_4_ON){
    			    cmd[0] = LGT_CMD_LIGHT_ON;
    			}else if (kb_event.keycode[0] == RC_KEY_A_ON){
#if MULTI_ADDR_FOR_SWITCH_EN
					multi_addr_key_flag = 1;
					foreach(i, 3){
						access_cmd_onoff(0x0202, 0, G_ON, CMD_NO_ACK, 0);
						blt_send_adv2scan_mode(1);
					}
					foreach(i, 3){
						access_cmd_onoff(0x0404, 0, G_ON, CMD_NO_ACK, 0);
						blt_send_adv2scan_mode(1);
					}
#else
					// send all on cmd 
					access_cmd_onoff(0xffff, 0, G_ON, CMD_NO_ACK, 0);
#endif
					// when in the adv_send_mode ,press the key ,it will exit the mode
					if(rc_mag.adv_send_enable && key_released){
						rc_mag.adv_send_enable=0;	
					}
					report_flag = 0;
    			}else if (kb_event.keycode[0] == RC_KEY_1_OFF){
    			    cmd[0] = LGT_CMD_LIGHT_OFF;
    			}else if (kb_event.keycode[0] == RC_KEY_2_OFF){
    			    cmd[0] = LGT_CMD_LIGHT_OFF;
    			}else if (kb_event.keycode[0] == RC_KEY_3_OFF){
    			    cmd[0] = LGT_CMD_LIGHT_OFF;
    			}else if (kb_event.keycode[0] == RC_KEY_4_OFF){
    			    cmd[0] = LGT_CMD_LIGHT_OFF;
    			}else if (kb_event.keycode[0] == RC_KEY_A_OFF){
#if MULTI_ADDR_FOR_SWITCH_EN
					multi_addr_key_flag = 1;
					foreach(i, 3){
						access_cmd_onoff(0x0202, 0, G_OFF, CMD_NO_ACK, 0);
						blt_send_adv2scan_mode(1);
					}
					foreach(i, 3){
						access_cmd_onoff(0x0404, 0, G_OFF, CMD_NO_ACK, 0);
						blt_send_adv2scan_mode(1);
					}
#else
    				// send all off cmd
					access_cmd_onoff(0xffff, 0, G_OFF, CMD_NO_ACK, 0);
#endif
					if(rc_mag.adv_send_enable && key_released){
						rc_mag.adv_send_enable =0;
					}
					report_flag = 0;
    			}else if (kb_event.keycode[0] == RC_KEY_UP){
    			    cmd[0] = LGT_CMD_LUM_UP;
    			}else if (kb_event.keycode[0] == RC_KEY_DN){
    			    cmd[0] = LGT_CMD_LUM_DOWN;
    			}else if (kb_event.keycode[0] == RC_KEY_L){
    			    cmd[0] = LGT_CMD_LEFT_KEY;
    			}else if (kb_event.keycode[0] == RC_KEY_R){
    			    cmd[0] = LGT_CMD_RIGHT_KEY;
    			}else{
    				// invalid key
    				memset4(&kb_event, 0, sizeof(kb_event));
					key_released = 0;
    				return;
    			}

    			if(report_flag){
    				vd_cmd_key_report(ADR_ALL_NODES, kb_event.keycode[0]);
    			}
                rf_link_light_event_callback(LGT_CMD_SWITCH_CMD);
			}
			key_released = 0;
		}
		///////////////////////////   key released  ///////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////
		else {
			rc_repeat_key = 0;
			key_released = 1;
		}

		tick_key_repeat = tick_key_pressed = clock_time ();
		kb_last[0] = kb_event.keycode[0];
		kb_last[1] = kb_event.keycode[1];

		//reg_usb_ep8_dat = kb_last[0];			//debug purpose, output to usb UART
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	//				no key change event
	//////////////////////////////////////////////////////////////////////////////////////////
	else if (kb_last[0])
	{
		//	long pressed
		mesh_active_time = clock_time();
		if (clock_time_exceed(tick_key_pressed, 2000000))		// long pressed
		{
			 if ((kb_last[0] == RC_KEY_A_ON && kb_last[1] == RC_KEY_1_OFF) ||
                 (kb_last[1] == RC_KEY_A_ON && kb_last[0] == RC_KEY_1_OFF))
			 {
			 		if(!rc_mag.adv_send_enable){
						rf_link_light_event_callback(LGT_CMD_SWITCH_PROVISION);
						rc_mag.adv_send_enable =1;
						rc_mag.adv_send_tick = clock_time();
						rc_mag.adv_timeout_def_ms = ADV_TIMEOUT;//30s
			 		}
			 }
			 if ((kb_last[0] == RC_KEY_A_ON && kb_last[1] == RC_KEY_4_OFF) ||
                 (kb_last[1] == RC_KEY_A_ON && kb_last[0] == RC_KEY_4_OFF))
			 {
				 rf_link_light_event_callback(LGT_CMD_SWITCH_PROVISION);
				 factory_reset();
				 start_reboot();
			 }
		}
		if (clock_time_exceed (tick_key_pressed, 500000))		//repeat key mode
		{
			if (kb_last[0] == VK_UP)
			{
				rc_repeat_key = LGT_CMD_LUM_UP;
			}
			else if (kb_last[0] == VK_DOWN)
			{
				rc_repeat_key = LGT_CMD_LUM_DOWN;
			}
			else if (kb_last[0] == VK_LEFT)
			{
				rc_repeat_key = LGT_CMD_LEFT_KEY;
			}
			else if (kb_last[0] == VK_RIGHT)
			{
				rc_repeat_key = LGT_CMD_RIGHT_KEY;
			}
		}

		if (!rc_long_pressed && clock_time_exceed(tick_key_pressed, 700000))
		{
			rc_long_pressed = 1;
			if (kb_last[0] == VK_1)		//group 1
			{
				
			}
		}

	}else{
		key_released = 1;
	}

	if (rc_repeat_key && clock_time_exceed (tick_key_repeat, ACTIVE_INTERVAL * TRANSMIT_CNT))	// should be longer than interval of command
	{
		tick_key_repeat = clock_time ();
		//tx_command_repeat ();
	}
	rc_key_pressed = kb_last[0];
	rc_mag.kb_pressed = !key_released;
	/////////////////////////////////////////////////////////////////////////////////
}

void set_rc_flag_when_send_pkt(int is_sending)
{
	if(!is_sending){
		rc_mag.rc_deep_flag =1;
	}

	rc_mag.rc_sending_flag = is_sending;
}

void proc_rc_ui_suspend()
{
	if(blt_state== BLS_LINK_STATE_CONN||rc_mag.kb_pressed){
		return ;
	}
	
	// test the rc_tick part 
	if(clock_time_exceed(rc_mag.rc_start_tick,2000*1000)&&!rc_mag.rc_sending_flag){
		rc_mag.rc_deep_flag =1;
	}

	#if 1
	if(	 (rc_mag.adv_send_enable&&clock_time_exceed(rc_mag.adv_send_tick,rc_mag.adv_timeout_def_ms*1000))||
		#if MULTI_ADDR_FOR_SWITCH_EN
		(multi_addr_key_flag == 1 && rc_mag.adv_timeout_def_ms != ADV_TIMEOUT) || 
		#endif
		 (!rc_mag.adv_send_enable&&rc_mag.rc_deep_flag)	){
		#if MULTI_ADDR_FOR_SWITCH_EN
		multi_addr_key_flag = 0;
		#endif
        int sleep_mode = DEEPSLEEP_MODE;
        #if (PM_DEEPSLEEP_RETENTION_ENABLE)
        sleep_mode = DEEPSLEEP_MODE_RET_SRAM_LOW32K;
        global_reset_new_key_wakeup();
        #endif
        
		usb_dp_pullup_en (0);
		cpu_sleep_wakeup(sleep_mode, PM_WAKEUP_PAD, 0);
	}
	#endif
}

void mesh_switch_init()
{
	rc_mag.rc_start_tick = clock_time();
	mesh_tid.tx[0] = analog_read(REGA_TID);
    ////////// set up wakeup source: driver pin of keyboard  //////////
    u32 pin[] = KB_DRIVE_PINS;
    //#if (0 == DEEP_SLEEP_EN)   //suspend
    for (int i=0; i<sizeof (pin)/sizeof(u32); i++)
    {
        //cpu_set_gpio_wakeup (pin[i], 0, 1);
        gpio_set_wakeup (pin[i], 1, 1);         // level : 1 (high); 0 (low)
    }
    gpio_core_wakeup_enable_all (1);
    //#else   //deep sleep
    for (int i=0; i<sizeof (pin)/sizeof(u32); i++)
    {
        cpu_set_gpio_wakeup (pin[i], 1, 1);     // level : 1 (high); 0 (low)
    }
    //#endif
	// gpio led init part 
	gpio_setup_up_down_resistor(GPIO_LED,PM_PIN_PULLDOWN_100K);
	gpio_set_func(GPIO_LED,AS_GPIO);
	gpio_set_output_en(GPIO_LED,1);
	gpio_write(GPIO_LED,1);
	rf_link_light_event_callback(LGT_CMD_SWITCH_POWERON);
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
void main_loop ()
{
	static u32 tick_loop;

	tick_loop ++;
#if (BLT_SOFTWARE_TIMER_ENABLE)
	blt_soft_timer_process(MAINLOOP_ENTRY);
#endif
	#if DUAL_MODE_ADAPT_EN
	if(RF_MODE_BLE != dual_mode_proc()){    // should be before is mesh latency window()
        proc_ui();
        proc_led();
        factory_reset_cnt_check();
		return ;
	}
	#endif
	
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop ();


	////////////////////////////////////// UI entry /////////////////////////////////
	//  add spp UI task:
#if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(1);
#endif
	proc_ui();
	proc_led();
	
	mesh_loop_process();
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

	proc_rc_ui_suspend();
}

void user_init()
{
    #if (BATT_CHECK_ENABLE)
    app_battery_power_check_and_sleep_handle(0); //battery check must do before OTA relative operation
    #endif
    switch_project_flag = 1;
	mesh_global_var_init();
	set_blc_hci_flag_fun(0);// disable the hci part of for the lib .
	proc_telink_mesh_to_sig_mesh();		// must at first

	#if (DUAL_MODE_ADAPT_EN)
	dual_mode_en_init();    // must before factory_reset_handle, because "dual_mode_state" is used in it.
	#endif
	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value and tp value

	usb_id_init();
	usb_log_init ();
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
#if BLE_REMOTE_PM_ENABLE
	blc_ll_initPowerManagement_module();        //pm module:      	 optional
#endif

	//l2cap initialization
	//blc_l2cap_register_handler (blc_l2cap_packet_receive);
	blc_l2cap_register_handler (app_l2cap_packet_receive);
	
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

	rf_set_power_level_index (my_rf_power_index);
	bls_pm_setSuspendMask (SUSPEND_DISABLE);//(SUSPEND_ADV | SUSPEND_CONN)
    blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_ADVERTISING_REPORT|HCI_LE_EVT_MASK_CONNECTION_COMPLETE);

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
	blc_hci_registerControllerEventHandler(app_event_handler);		//register event callback
	//bls_hci_mod_setEventMask_cmd(0xffff);			//enable all 15 events,event list see ble_ll.h
	bls_set_advertise_prepare (app_advertise_prepare_handler);
	//bls_set_update_chn_cb(chn_conn_update_dispatch);
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);

	app_enable_scan_all_device ();

	// mesh_mode and layer init
	mesh_init_all();

	// OTA init
	#if (DUAL_MODE_ADAPT_EN && (0 == FW_START_BY_BOOTLOADER_EN) || DUAL_MODE_WITH_TLK_MESH_EN)
	if(DUAL_MODE_NOT_SUPPORT == dual_mode_state)
	#endif
	{bls_ota_clearNewFwDataArea();	 //must
	}
	mesh_switch_init();
	//blc_ll_initScanning_module(tbl_mac);
	#if((MCU_CORE_TYPE == MCU_CORE_8258) || (MCU_CORE_TYPE == MCU_CORE_8278))
	blc_gap_peripheral_init();    //gap initialization
	#endif
	
	mesh_scan_rsp_init();
	my_att_init (provision_mag.gatt_mode);
	blc_att_setServerDataPendingTime_upon_ClientCmd(10);
	extern u32 system_time_tick;
	system_time_tick = clock_time();
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
	rf_set_power_level_index (my_rf_power_index);

	blc_ll_recoverDeepRetention();

	DBG_CHN0_HIGH;    //debug
#if (HCI_ACCESS == HCI_USE_UART)	//uart
	uart_drv_init();
#endif
#if ADC_ENABLE
	adc_drv_init();
#endif

    mesh_switch_init();

    // should enable IRQ here, because it may use irq here, for example BLE connect.
    irq_enable();
    
}

void global_reset_new_key_wakeup()
{
    rc_key_pressed = 0;
    rc_long_pressed = rc_repeat_key = 0;
    memset(&kb_event, 0, sizeof(kb_event));
    global_var_no_ret_init_kb();
}

#endif

