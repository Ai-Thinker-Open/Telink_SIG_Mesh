#include "user_ali.h"
#include "user_du.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "vendor_model.h"
#include "fast_provision_model.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj_lib/mesh_crypto/aes_cbc.h"
#include "vendor/common/mi_api/telink_sdk_mible_api.h"
#include "blt_soft_timer.h"
#if DU_ENABLE
#include "vendor_model.h"
#define SHORT_LOACL_NAME  "DUMESH"
const u8 du_local_name[6]=SHORT_LOACL_NAME;
#define DU_MAGIC_CODE 0x44496F54
_attribute_no_retention_data_  u8  du_ota_buf[240*16];

void du_create_input_string(char *p_input,u16 rand,u32 pid,u8 *p_mac,u8 *p_secret)
{
	u8 idx =0;
	u8 p_rand[2];
	u8 p_pid[4];
	u8 mac[6];
	swap16(p_rand,(u8*)(&rand));
	swap32(p_pid,(u8*)&pid);
	swap48(mac,p_mac);
	for(int i=0;i<2;i++){
		p_input[idx++] = num2char [(p_rand[i]>>4) & 15];
		p_input[idx++] = num2char [p_rand[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<4;i++){
		p_input[idx++] = num2char [(p_pid[i]>>4) & 15];
		p_input[idx++] = num2char [p_pid[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<6;i++){
		p_input[idx++] = num2char [(mac[i]>>4) & 15];
		p_input[idx++] = num2char [mac[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<16;i++){// need to change to string .
		p_input[idx++] = num2char [(p_secret[i]>>4) & 15];
		p_input[idx++] = num2char [p_secret[i] & 15];
	}
}
uint32_t du_soft_crc32(uint8_t const * p_data, uint32_t size, uint32_t init_crc)
{
    uint32_t crc;

    crc = (init_crc == 0) ? 0xFFFFFFFF : ~(init_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

#if 0
void test_du_sha256_cal()
{

	u8 du_sha_in[59];
	u8 du_sha_out[32];
	u16 random = 0x8fb5;
	u32 pid = 0x6adb79;
	u8 mac[6]={0xD4,0x60,0x75,0x12,0x79,0x7D};
	u8 sec[16]={0x49,0x22,0xeb,0x7a, 0x0a,0x45,0x81,0x8d, 
				0xa4,0x34,0x7c,0xd4, 0xed,0x1b,0x4c,0xf9};
	du_create_input_string(du_sha_in,random,pid,mac,sec);
	mbedtls_sha256(du_sha_in,sizeof(du_sha_in),du_sha_out,0);
	//crc check proc 
	char crc_in[]="12345";
	u32 A_debug_crc_out=0;
	A_debug_crc_out = du_soft_crc32(crc_in,3,0);
	A_debug_crc_out = du_soft_crc32(crc_in+3,2,A_debug_crc_out);
}
#endif

du_ota_str du_ota;
du_ota_str *p_ota = &du_ota;


void du_sha256_out(char *p_input,u16 rand,u32 pid,u8 *p_mac,u8 *p_secret)
{
	u8 du_sha_in[59];
	u8 du_sha_out[32];
	du_create_input_string((char *)du_sha_in,rand,pid,p_mac,p_secret);
	mbedtls_sha256(du_sha_in,sizeof(du_sha_in),du_sha_out,0);
	memcpy(p_input,du_sha_out,16);
}

void du_adv_conn_init(rf_packet_adv_t * p)
{
	p->header.type = LL_TYPE_ADV_IND;
	memcpy(p->advA,tbl_mac,6);
	p->rf_len = 6 + sizeof(du_ota_adv_str);
	p->dma_len = p->rf_len + 2; 
	du_ota_adv_str *p_ota_adv = (du_ota_adv_str *)(p->data);
	p_ota_adv->len1 = 2;
	p_ota_adv->ad_type1 = 1;
	p_ota_adv->ad_dat1 = 6;
	p_ota_adv->len2 = 3;
	p_ota_adv->ad_type2 = 2;
	p_ota_adv->ad_dat2 = 0xffb0;
	p_ota_adv->len3 = 7;
	p_ota_adv->ad_type3 = 8;
	memcpy(p_ota_adv->ad_dat3,du_local_name,6);
	p_ota_adv->len4 = 6;
	p_ota_adv->ad_type4= 0xff;
	p_ota_adv->cid = VENDOR_ID;
	p_ota_adv->dev_type = 3;// mesh type
	p_ota_adv->ver = DU_FW_VER;
}

void du_adv_ota_end_init(rf_packet_adv_t * p)
{
	p->header.type = LL_TYPE_ADV_NONCONN_IND;
	memcpy(p->advA,tbl_mac,6);
	p->rf_len = 6 + sizeof(du_ota_end_adv_str);
	p->dma_len = p->rf_len + 2; 
	du_ota_end_adv_str *p_ota_end = (du_ota_end_adv_str *)(p->data);
	p_ota_end->len = 6;
	p_ota_end->ad_type = 0xff;
	p_ota_end->cid = VENDOR_ID;
	p_ota_end->dev_type = 3;// mesh type
	p_ota_end->ver = DU_FW_VER;
}

u8 du_adv_proc(rf_packet_adv_t * p)
{
	if(p_ota->ota_suc){
		du_adv_ota_end_init(p);
	}else{
		du_adv_conn_init(p);
	}
	return 1;
}

extern int bls_du_notify_rsp(u8*p_buf,int len);

int bls_du_cmd_rsp(u8 op,u8 *p_buf,int len)
{
#define DU_BUF_MAX_LEN 0X40
	u8 buf[DU_BUF_MAX_LEN];
	if(len>(DU_BUF_MAX_LEN-1)){
		return 0;
	}
	buf[0]=op;
	memcpy(buf+1,p_buf,len);
	return bls_du_notify_rsp(buf,len+1);
}

void cert_rand_rsp_auth(u8 *pbuf)
{
	u16 rand;
	u8 auth[16];
	rand = pbuf[0]|(pbuf[1]<<8);
	du_sha256_out((char *)auth,rand,con_product_id,con_mac_address,con_sec_data);
	bls_du_cmd_rsp(DU_CERT_IOT_DEV_RSP,auth,sizeof(auth));
}

void resume_break_proc(u8 *pbuf)
{
	
	du_break_point_cmd_str *p_cmd = (du_break_point_cmd_str *)pbuf;
	du_break_point_rsp_str rsp;
	if(p_cmd->rand !=0 &&!memcmp(pbuf,(u8*)&(p_ota->rand_code),4)){//is the same 
		rsp.break_point = p_ota->break_point;
		p_ota->rand_code = 0;
	}else{
		rsp.break_point = 0;
	// not want to store the rand info ,and the ota time is short ,we change the rand every time ,not allow retrans 
		rsp.new_rand = rand();
		p_ota->rand_code = rsp.new_rand;
	}
	bls_du_cmd_rsp(DU_TRANS_RESUME_BREAK_RSP,(u8*)&rsp,sizeof(rsp));
}



void exchange_chk_size_proc(u8 *pbuf)
{
	du_exchange_str* p_cmd = (du_exchange_str*)pbuf;
	du_exchange_str rsp;
	rsp.seg_size = p_ota->seg_size = p_cmd->seg_size;
	rsp.buf_size = p_ota->buf_size = p_cmd->buf_size;
	bls_du_cmd_rsp(DU_EXCHANGE_BUF_CHK_SIZE_RSP,(u8*)&rsp,sizeof(rsp));

}

void start_ota_proc(u8*pbuf)
{
	du_start_ota_str *p_start = (du_start_ota_str *)pbuf;	
	du_start_ota_sts_str ota_sts;
	p_ota->image_size = p_start->image_size;
	p_ota->image_offset = p_start->offset;
	ota_sts.sts =1;
	bls_du_cmd_rsp(DU_START_OTA_RSP,(u8*)&ota_sts,sizeof(ota_sts));
}

void buffer_chk_cmd_proc(u8*pbuf)
{
	du_buf_chk_str *p_chk = (du_buf_chk_str *)pbuf;
	du_buf_chk_sts_str chk_sts;
	//when we receive all the segment buf part 
	if(du_soft_crc32(du_ota_buf,p_ota->buf_idx,0) == p_chk->crc32){
		// if the image_offset is 0,we need to change the buf to 0xff,and after finish ,we need to change back to 0x4f
		if(p_ota->image_offset <=8){
			p_ota->crc = du_soft_crc32(du_ota_buf,p_ota->buf_idx,0);
			du_ota_buf[8]= 0xff;
		}else{
			p_ota->crc = du_soft_crc32(du_ota_buf,p_ota->buf_idx,p_ota->crc);
		}
		flash_write_page(ota_program_offset+p_ota->image_offset,p_ota->buf_idx,du_ota_buf);
		p_ota->image_offset+=p_ota->buf_idx;
		p_ota->buf_idx =0 ;// clear all the fw part
		chk_sts.sts = 1;
	}else{
		chk_sts.sts = 0;
	}
	chk_sts.adjust_offset = 0;
	bls_du_cmd_rsp(DU_BUFFER_CHK_RSP,(u8*)&chk_sts,sizeof(chk_sts));
}

int use_flash_to_get_crc32()
{
	// get the first 8 bytes
	u8 *p_buf = du_ota_buf;
	u32 buf_size = sizeof(du_ota_buf);
	u32 img_size = p_ota->image_size;
	u32 idx =0;
	u32 crc=0;
	flash_read_page(ota_program_offset,buf_size,p_buf);
	idx+=buf_size;
	img_size -= buf_size;
	// change the 8th byte to 0x4b
	p_buf[8]=0x4b;
	crc =du_soft_crc32(du_ota_buf,buf_size,0);
	while(img_size){
		if(img_size >= buf_size){
			flash_read_page(ota_program_offset+idx,buf_size,p_buf);
			crc =du_soft_crc32(du_ota_buf,buf_size,crc);
			img_size-=buf_size;
			idx+=buf_size;
		}else{
			flash_read_page(ota_program_offset+idx,img_size,p_buf);
			crc =du_soft_crc32(du_ota_buf,img_size,crc);
			img_size =0;
		}
	}
	return crc;
}

void whole_img_chk_proc(u8*pbuf)
{
	du_whole_img_chk_sts_str chk;
	// check all the firmware by the flash content
	chk.sts =1;
	u32 crc_flash = use_flash_to_get_crc32();
	if(crc_flash != p_ota->crc){
		start_reboot();
	}
	chk.crc = p_ota->crc;
	bls_du_cmd_rsp(DU_WHOLE_IMG_CHK_RSP,(u8*)&chk,sizeof(chk));
}

void du_ota_suc_proc()
{
	if( blt_state != BLS_LINK_STATE_CONN && 
		p_ota->ota_suc && 
		clock_time_exceed(p_ota->ota_suc_tick,4*1000*1000)){
		ota_set_flag();
		start_reboot();
	}
}
#if DU_LPN_EN

typedef struct{
	u8 sw;
	u8 sw_last;
	u8 press_down;
	u8 press_on;
	u32 on_tick;
	u32 down_tick;
}sw_proc_t;
sw_proc_t sw_but;
u32 du_busy_tick =0;
static u32 du_loop_tick =0;
void du_key_board_long_press_detect()
{
	sw_but.sw = !gpio_read(SW1_GPIO);
	if(!(sw_but.sw_last)&&sw_but.sw){// press on 
	   	sw_but.press_on++;
		sw_but.on_tick = clock_time();
		sw_but.down_tick = clock_time();
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"press on ",0);
	}
	if(sw_but.sw_last && !sw_but.sw){// press down
		sw_but.press_down++;
		sw_but.down_tick = clock_time();
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"press down ",0);
	}
	sw_but.sw_last = sw_but.sw;
	if(sw_but.press_on && clock_time_exceed(sw_but.on_tick,10*1000*1000)){
		// long press trigger to enter provision mode 
		sw_but.press_on = 0;
		sw_but.on_tick = clock_time();
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"long_press",0);
		mi_mesh_state_set(1);
		du_busy_tick = clock_time();
		beacon_str_init();
	}
	if(sw_but.press_down && clock_time_exceed(sw_but.press_down,1*1000*1000)){
		
		if(sw_but.press_on == 1){
			//single press
			LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"single_press",0);
		}else if (sw_but.press_on == 2){
			// twice press
			LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"twice_press",0);
		}
		// clear all the sts ,press end 
		memset(&sw_but,0,sizeof(sw_but));
	}
}

u32 du_bind_tick =0;
void du_bind_end_proc(u16 adr)
{
	du_busy_tick = clock_time();//refresh the busy tick 
	du_bind_tick = clock_time()|1;
	du_set_gateway_adr(adr);
}

void du_bind_end_loop()
{
	if(du_bind_tick && clock_time_exceed(du_bind_tick,10*1000*1000)){
		du_bind_tick = 0;
		mi_mesh_state_set(0);
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"finish join into net ,back to deep mode",0);
	}
}

void du_busy_proc()
{

	if(mi_mesh_get_state() && clock_time_exceed(du_busy_tick,60*1000*1000)){
		du_busy_tick = clock_time();
		mi_mesh_state_set(0);
		beacon_str_disable();
		LOG_MSG_INFO(TL_LOG_NODE_SDK,0,0,"back to normal mode",0);
	}
}


void du_key_board_proc()
{
	static u32 du_log_tick =0;
	if(clock_time_exceed(du_log_tick,40*1000))//40ms print every time 
	{
		du_log_tick = clock_time();
		du_key_board_long_press_detect();
	}
}
#endif


void du_loop_proc()
{
	du_ota_suc_proc();
	#if DU_LPN_EN
	du_key_board_proc();
	du_busy_proc();
	du_bind_end_loop();
	// demo send cmd.
	if(is_provision_success()&&
		!mi_mesh_get_state()&&
		clock_time_exceed(du_loop_tick,15*1000*1000)){
		du_loop_tick = clock_time();
		du_vd_send_loop_proc();
	}
	#endif
}

void du_lpn_suspend_enter()
{
	bls_pm_setWakeupSource(PM_WAKEUP_PAD);  // GPIO_WAKEUP_MODULE needs to be wakened
}
void du_ui_proc_init()
{
	du_get_gateway_adr_init();
	gpio_set_wakeup (SW1_GPIO, Level_Low, 1);
	cpu_set_gpio_wakeup (SW1_GPIO, Level_Low,1);  //drive pin pad high wakeup deepsleep
	bls_pm_setWakeupSource(PM_WAKEUP_PAD);  // GPIO_WAKEUP_MODULE needs to be wakened
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, (blt_event_callback_t)&du_lpn_suspend_enter);
}

void du_ui_proc_init_deep()
{
	gpio_set_wakeup (SW1_GPIO, Level_Low, 1);
	cpu_set_gpio_wakeup (SW1_GPIO, Level_Low,1);  //drive pin pad high wakeup deepsleep
	bls_pm_setWakeupSource(PM_WAKEUP_PAD);	// GPIO_WAKEUP_MODULE needs to be wakened
}

void magic_code_chk_proc(u8* pbuf)
{
	du_magic_code_str *p_magic = (du_magic_code_str *)pbuf;
	du_magic_code_rsp_str rsp;
	if(p_magic->magic == DU_MAGIC_CODE){
		rsp.sts =1;
		//we need to write back the flag part
		p_ota->ota_suc =1;
		p_ota->ota_suc_tick = clock_time();
		//later we will need to reboot
	}else{
		// magic number error will reboot
		start_reboot();
		rsp.sts =0;
	}
	bls_du_cmd_rsp(DU_OVERWRITE_IMG_RSP,(u8*)&rsp,sizeof(rsp));
}

int	du_ctl_Write (void *p)
{
	rf_packet_att_write_t *pw = (rf_packet_att_write_t *)(p);
	du_cmd_str *p_cmd = (du_cmd_str *)(&pw->value);
	switch(p_cmd->opcode){
		case DU_CERT_IOT_DEV_CMD:
			cert_rand_rsp_auth(p_cmd->buf);
			break;
		case DU_TRANS_RESUME_BREAK_CMD:
			resume_break_proc(p_cmd->buf);
			break;
		case DU_EXCHANGE_BUF_CHK_SIZE_CMD:
			exchange_chk_size_proc(p_cmd->buf);
			break;
		case DU_START_OTA_CMD:
			start_ota_proc(p_cmd->buf);
			break;
		case DU_BUFFER_CHK_CMD:
			buffer_chk_cmd_proc(p_cmd->buf);
			break;
		case DU_WHOLE_IMG_CHK_CMD:
			whole_img_chk_proc(p_cmd->buf);
			break;
		case DU_OVERWRITE_IMG_CMD:
			magic_code_chk_proc(p_cmd->buf);
			break;
		default:
			break;
	}
	return 1;
}

int du_fw_proc(void *p)
{
	rf_packet_att_data_t *p_w = (rf_packet_att_data_t*)p;
	u32 len = p_w->l2cap-3;
	memcpy(du_ota_buf+p_ota->buf_idx,p_w->dat,len);
	p_ota->buf_idx +=len;

	return 1;
}
#define DU_STORE_ADR	0x7e000
u16 du_gateway_adr =0;
void du_set_gateway_adr(u16 adr)
{
	du_gateway_adr = adr;
	flash_erase_sector(DU_STORE_ADR);
	flash_write_page(DU_STORE_ADR,sizeof(du_gateway_adr),(u8 *)(&du_gateway_adr));
}

void du_get_gateway_adr_init()
{
	flash_read_page(DU_STORE_ADR,sizeof(du_gateway_adr),(u8 *)(&du_gateway_adr));
}

int du_vd_event_send(u8*p_buf,u8 len,u16 dst)
{
	return mesh_tx_cmd2normal(VD_LPN_REPROT, (u8 *)p_buf, len,ele_adr_primary,dst,0);
}

vd_du_event_t vd_du;

int du_vd_temp_event_send(u16 op,u16 val,u16 dst)
{
	static u8 du_tid =0;
	du_tid++;
	vd_du.tid = du_tid;
	vd_du.op = op;
	vd_du.val = val;
	return du_vd_event_send((u8*)&vd_du,sizeof(vd_du),dst);
}

void du_vd_send_loop_proc()
{
	if(blt_state == BLS_LINK_STATE_ADV){
		//(data/ 100 - 273.15),20 degree is 27315+2000
		du_vd_temp_event_send(VD_DU_TEMP_CMD,27315+2000,du_gateway_adr);
		du_vd_temp_event_send(VD_DU_TEMP_CMD,27315+2000,du_gateway_adr);
		du_vd_temp_event_send(VD_DU_TEMP_CMD,27315+2000,VD_DU_GROUP_DST);
	}
}


#if 0 // wait to retry 2times ,to send 24 times 
void mi_ivi_event_loop()
{
	int err =-1;
	static u32 ivi_sts_tick =0;
	if(ivi_sts_cnt &&!is_busy_tx_segment_or_reliable_flow()&&clock_time_exceed(ivi_sts_tick,500*1000)){
		ivi_sts_tick = clock_time();
		err = mesh_tx_cmd2normal_primary(VD_MI_PROPERTY_STS, (u8 *)&property_ivi, sizeof(property_ivi), 0xfeff, 0);
		if(err == 0){
			ivi_sts_cnt--;
		}
	}
}
#endif

#endif

