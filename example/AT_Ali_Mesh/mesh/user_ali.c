#include "user_ali.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/mesh_crypto/sha256_telink.h"
#include "vendor_model.h"
#include "fast_provision_model.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj_lib/mesh_crypto/aes_cbc.h"

const char num2char[] = "0123456789abcdef";


#if(AIS_ENABLE)
STATIC_ASSERT(sizeof(sha256_dev_uuid_str) <= 16);   // because sizeof dev uuid is 16

// sha256 init for three parameters 
#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)

u32 con_product_id=192941;// little endiness 
#if(MESH_USER_DEFINE_MODE != MESH_MI_SPIRIT_ENABLE)
const
#endif
u8 con_mac_address[6]={0xee,0x11,0x33,0x55,0x77,0x03};//small endiness
//char con_sec_data[]="claoePqYe1avDpmf8Jcm4jF52kVOLS1Q";
char con_sec_data[32];
#define  SIZE_CON_SEC_DATA  32

#elif((MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE) || (MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE))

#define AIS_SAFE_CERTIFY_THREE_PAR_EN	 0// in Ali demo environment,use this three par to test safe certify
#if !AIS_SAFE_CERTIFY_THREE_PAR_EN
u32 con_product_id=0x00000002;// little endiness 
#if(MESH_USER_DEFINE_MODE != MESH_MI_SPIRIT_ENABLE)
const
#endif
u8  con_mac_address[6]={0x9e,0x16,0x11,0x07,0xda,0x78};//small endiness
    #if 0 // need to open it to make the init three para enable 
u8 con_sec_data[16]={ 0x04,0x6e,0x68,0x11,0x27,0xed,0xe6,0x70,
					  0x94,0x44,0x18,0xdd,0xb1,0xb1,0x7b,0xdc};
    #else
u8 con_sec_data[16];
    #endif
#else
u32 con_product_id = 602;
#if(MESH_USER_DEFINE_MODE != MESH_MI_SPIRIT_ENABLE)
const
#endif
u8 con_mac_address[6]= {0xa8, 0x1c, 0xc1, 0x07, 0xda, 0x78};
u8 con_sec_data[16] = {0x15, 0x65, 0xd0, 0x49, 0x74, 0x00, 0xc9, 0x99, 0xe9, 0x84, 0xcf, 0xfa, 0x4d, 0xa4, 0xfd, 0xf9};
#endif
#define SIZE_CON_SEC_DATA   (sizeof(con_sec_data))
#endif

#if(DUAL_VENDOR_EN)
// default use mi_api's parameters, if device provisioned by spirit, reinit the parameters.
void mesh_ais_global_var_set()
{
    vendor_id_check_and_update();

	memcpy(tbl_mac, con_mac_address, 6);
	extern rf_packet_scan_rsp_t	pkt_scan_rsp;
	memcpy(pkt_adv.advA, tbl_mac, BLE_ADDR_LEN);
	memcpy(pkt_scan_rsp.advA, tbl_mac, BLE_ADDR_LEN);

	model_sig_cfg_s.nw_transmit.count = AIS_TRANSMIT_CNT_DEF;
	model_sig_cfg_s.nw_transmit.invl_steps = AIS_TRANSMIT_INVL_STEPS_DEF;
	model_sig_cfg_s.relay_retransmit.count = AIS_TRANSMIT_CNT_DEF_RELAY;
	model_sig_cfg_s.relay_retransmit.invl_steps = AISTRANSMIT_INVL_STEPS_DEF_RELAY;
	
    //provision_mag_cfg_s_store(); // no need, save later in mesh_provision_par_set()
}
#endif

void read_three_para_sha256_from_flash()
{
#if !WIN32
	u8 idx =0;
	u32 product_id_flash;
	flash_read_page(FLASH_ADR_THREE_PARA_ADR,sizeof(product_id_flash),(u8 *)(&product_id_flash));
	if(product_id_flash == 0xffffffff){
		return;
	}
	flash_read_page(FLASH_ADR_THREE_PARA_ADR,sizeof(con_product_id),(u8 *)&con_product_id);
	idx += sizeof(con_product_id);
	flash_read_page(FLASH_ADR_THREE_PARA_ADR+idx,sizeof(tbl_mac),tbl_mac);// read big endian
	endianness_swap_u48(tbl_mac);// change to small endian
	#if(MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
	memcpy(con_mac_address, tbl_mac, 6);
	#endif
	idx += sizeof(tbl_mac);
	flash_read_page(FLASH_ADR_THREE_PARA_ADR+idx,SIZE_CON_SEC_DATA,(u8 *)con_sec_data);
#endif
	return ;
}


void set_sha256_init_para_mode(u8 mode)
{
	if(mode){
		// init the mac address part 
		memcpy(tbl_mac,con_mac_address,sizeof(con_mac_address));// get mac from the ini para 
		read_three_para_sha256_from_flash();
	}
	return ;
}

void set_dev_uuid_for_sha256()
{
	sha256_dev_uuid_str dev_uuid;
	sha256_dev_uuid_str *p_uuid = &dev_uuid;
	memset(p_uuid,0,sizeof(sha256_dev_uuid_str));
	p_uuid->cid = SHA256_BLE_MESH_PID;
	p_uuid->adv_ver = 0x01;
	p_uuid->ser_fun = 1;
	p_uuid->ota_en =1;
	p_uuid->ble_ver = BLE_4_2;
	memcpy(p_uuid->product_id,&con_product_id,sizeof(con_product_id));
	memcpy(p_uuid->mac, tbl_mac, sizeof(tbl_mac));
	#if(MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
	memcpy(p_uuid->mac, con_mac_address, sizeof(tbl_mac));
	#endif
	p_uuid->uuid_ver = 1;
	p_uuid->adv_flag = 0;
	memcpy(prov_para.device_uuid,(u8 *)(p_uuid),sizeof(sha256_dev_uuid_str));
}

#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
void create_sha256_input_string_node(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret)
{
	u8 idx =0;
	u8 con_product_id_rev[4];
	swap32(con_product_id_rev,pid);
	
	#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
	memcpy(p_input + idx, con_sec_data, SIZE_CON_SEC_DATA);
	idx += SIZE_CON_SEC_DATA;
	#else	
	for(int i=0;i<16;i++){// need to change to string .
		p_input[idx++] = num2char [(p_secret[i]>>4) & 15];
		p_input[idx++] = num2char [p_secret[i] & 15];
	}
	#endif
	p_input[idx++]=',';
	for(int i=0;i<6;i++){
		p_input[idx++] = num2char [(p_mac[i]>>4) & 15];
		p_input[idx++] = num2char [p_mac[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<4;i++){
		p_input[idx++] = num2char [(con_product_id_rev[i]>>4) & 15];
		p_input[idx++] = num2char [con_product_id_rev[i] & 15];
	}
}
#endif

void caculate_sha256_node_oob(u8 *p_oob)
{
	char sha256_in[54];
	// caculate the tbl_mac address part ,use big endian
	u8 mac_address_sha256[6];
	#if(MESH_USER_DEFINE_MODE == MESH_MI_SPIRIT_ENABLE)
	swap48(mac_address_sha256,con_mac_address);
	#else
	swap48(mac_address_sha256,tbl_mac);
	#endif
	#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
	create_sha256_input_string_node(sha256_in,(u8 *)&con_product_id,mac_address_sha256,(u8 *)con_sec_data);
	#else
	create_sha256_input_string(sha256_in,(u8 *)&con_product_id,mac_address_sha256,(u8 *)con_sec_data);
	#endif
	mbedtls_sha256((u8 *)sha256_in,sizeof(sha256_in),p_oob,0);
}

void caculate_sha256_node_auth_value(u8 *auth_value)
{
	#ifndef WIN32		// comfirm later
	u8 sha256_out[32];
	caculate_sha256_node_oob(sha256_out);
	memcpy(auth_value, sha256_out, 16);
	#endif
}

void caculate_sha256_to_create_static_oob()
{
	#if !WIN32		// comfirm later
	u8 sha256_out[32];
	caculate_sha256_node_oob(sha256_out);
	mesh_set_dev_auth(sha256_out, 16);
	#endif
}

ais_gatt_auth_t ais_gatt_auth;
void ais_gatt_auth_init()
{
	ais_gatt_auth.auth_ok = 0;
}

int ais_auth_cipher(ais_msg_t *p_ais_msg)
{
	ais_msg_t ais_msg_result;
	memset(&ais_msg_result, 0x00, sizeof(ais_msg_result));		
	ais_msg_result.header = p_ais_msg->header;
	ais_msg_result.msg_type = AIS_AES_CIPHER;
	ais_msg_result.frame_seq = ais_msg_result.frame_total = 0;
	ais_msg_result.length = 0x10;
	memcpy(ais_msg_result.data, p_ais_msg->data, 16);
	return bls_att_pushIndicateData(AIS_INDICATE_HANDLE, (u8 *)&ais_msg_result, OFFSETOF(ais_msg_t, data)+16);
}

AES_ctx ctx;
const u8 iv[] = "123aqwed#*$!(4ju";
int ais_write_pipe(void *p)
{
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	ais_msg_t *ais_p = (ais_msg_t *)req->dat;

	if(ais_gatt_auth.auth_ok){
		u16 len = ((req->l2cap-3)+AES_BLOCKLEN-1)/AES_BLOCKLEN*AES_BLOCKLEN;
		aes_cbc_decrypt(ais_p->data, len>AIS_MAX_DATA_SIZE?0:len, &ctx, ais_gatt_auth.ble_key, iv);
	}
	
	if((!ais_gatt_auth.auth_ok) && (AIS_AUTH_RANDOM == ais_p->msg_type) && (23 == req->l2cap)){
		u8 mac_address_sha256[6];
		char sha256_in[71];
		u8 sha256_out[32];
		
		memcpy(sha256_in, ais_p->data, 16);
		sha256_in[16] = ',';
		swap48(mac_address_sha256,tbl_mac);
		create_sha256_input_string(sha256_in+17,(u8 *)&con_product_id,mac_address_sha256,(u8 *)con_sec_data);
		mbedtls_sha256((u8 *)sha256_in,sizeof(sha256_in),sha256_out,0);
		memcpy(ais_gatt_auth.ble_key, sha256_out, 16);

		aes_cbc_encrypt(ais_p->data, 16, &ctx, ais_gatt_auth.ble_key, iv);
		ais_auth_cipher(ais_p);
	}
	else if(AIS_AUTH_CHECK == ais_p->msg_type){
		ais_p->msg_type = AIS_AUTH_RESULT;
		if(ais_p->data[0]){//fail
			ais_gatt_auth_init();	
			ais_p->data[0] = 0x01; // 1 means fail
		}
		else{
			ais_p->data[0] = 0;	  // 0 means success
			ais_gatt_auth.auth_ok = 1;
		}
		return bls_att_pushIndicateData(AIS_INDICATE_HANDLE, (u8 *)req->dat, OFFSETOF(ais_msg_t, data)+1);
	}
	return 0;
}


#else
void set_sha256_init_para_mode(u8 mode){}
void set_dev_uuid_for_sha256(){}
void caculate_sha256_to_create_static_oob(){}

#endif
void create_sha256_input_string(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret)
{
	u8 idx =0;
	u8 con_product_id_rev[4];
	swap32(con_product_id_rev,pid);
	for(int i=0;i<4;i++){
		p_input[idx++] = num2char [(con_product_id_rev[i]>>4) & 15];
		p_input[idx++] = num2char [con_product_id_rev[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<6;i++){
		p_input[idx++] = num2char [(p_mac[i]>>4) & 15];
		p_input[idx++] = num2char [p_mac[i] & 15];
	}
	p_input[idx++]=',';
#if(MESH_USER_DEFINE_MODE == MESH_CLOUD_ENABLE)
	memcpy(p_input + idx, con_sec_data, SIZE_CON_SEC_DATA);
	idx += SIZE_CON_SEC_DATA;
#else
	for(int i=0;i<16;i++){// need to change to string .
		p_input[idx++] = num2char [(p_secret[i]>>4) & 15];
		p_input[idx++] = num2char [p_secret[i] & 15];
	}
#endif
}

#define TEST_ALI_AUTH_ENABLE 0
#if TEST_ALI_AUTH_ENABLE
u32 ali_pid = 0x293e2;
u8 ali_mac[6]={0xab,0xcd,0xf0,0xf1,0xf2,0xf3};
u8 ali_secret[16]={	0x53,0xda,0xed,0x80,0x5b,0xc5,0x34,0xa4,
					0xa9,0x3c,0x82,0x5e,0xd2,0x0a,0x70,0x63};
u8 ali_random_dev[16]={	0x78,0x89,0xb0,0xaf,0x41,0x7b,0x96,0x7b,
						0xdc,0xd7,0xb8,0x14,0xd2,0xbb,0xff,0xaf};
u8 ali_random_pro[16]={	0xaa,0x48,0x4b,0x09,0x9a,0xe4,0xc7,0x76,
						0x2f,0xcb,0x1b,0x71,0x96,0x8b,0xa7,0xdf};

void ali_new_create_sha256_input_string(char *p_input,u8 *pid,u8 *p_mac,u8 *p_secret,u8 *p_random)
{
	u8 idx =0;
	u8 con_product_id_rev[4];
	swap32(con_product_id_rev,pid);
	for(int i=0;i<4;i++){
		p_input[idx++] = num2char [(con_product_id_rev[i]>>4) & 15];
		p_input[idx++] = num2char [con_product_id_rev[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<6;i++){
		p_input[idx++] = num2char [(p_mac[i]>>4) & 15];
		p_input[idx++] = num2char [p_mac[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<16;i++){// need to change to string .
		p_input[idx++] = num2char [(p_secret[i]>>4) & 15];
		p_input[idx++] = num2char [p_secret[i] & 15];
	}
	p_input[idx++]=',';
	for(int i=0;i<16;i++){// need to change to string .
		p_input[idx++] = num2char [(p_random[i]>>4) & 15];
		p_input[idx++] = num2char [p_random[i] & 15];
	}
}

void calculate_auth_value()
{
	static u8 ali_input_string[87];
	static u8 ali_output_sha[32];
	ali_new_create_sha256_input_string(ali_input_string,(u8*)(&ali_pid),ali_mac,ali_secret,ali_random_pro);
	mbedtls_sha256(ali_input_string,sizeof(ali_input_string),ali_output_sha,0);
}
#endif



