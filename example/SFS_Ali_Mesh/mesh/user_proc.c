#include "user_proc.h"
#include "app_health.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh/lighting_model.h"
#include "vendor_model.h"
#include "fast_provision_model.h"
#include "proj_lib/mesh_crypto/aes_att.h"
#include "proj_lib/mesh_crypto/mesh_md5.h"


#if(AIS_ENABLE)
#define AIS_DEVICE_NAME	"Mesh Ali"
u8 ais_pri_data_set(u8 *p)
{
	u8 device_name[]={AIS_DEVICE_NAME};
	u8 name_len = (sizeof(device_name) - 1)>9?9:sizeof(device_name) - 1;//max 31 -( 4 bytes uuid + 16 byte vendor data) - 2 bytes head
	//service uuid
	p[0] = 3;
	p[1] = 2;//imcomplete  service uuid
	p[2] = 0xb3;
	p[3] = 0xfe;
	//name
	p[4] = name_len + 1;
	p[5] = 0x09;
	memcpy(p+6, device_name, name_len);
	//ais data
	ais_pri_t *pri_data = (ais_pri_t *)(p+name_len+4+2);
	pri_data->length = sizeof(ais_pri_t) - 1;
	pri_data->type = 0xff;
	pri_data->cid = g_vendor_id;
	pri_data->vid = 0x85;
	pri_data->ble_version = 0x01;
	pri_data->ota_support = 1;
	pri_data->authen_en = 1;
	pri_data->secret_type = 1;
	pri_data->prov_flag = is_provision_success();
	pri_data->rfu = 0;
	extern u32 con_product_id;
	pri_data->pid = con_product_id;
	memcpy(pri_data->mac, tbl_mac, 6);

	u8 ais_rsp_len = 4+name_len+2+sizeof(ais_pri_t);
	return ais_rsp_len;
}
#endif

#if (MESH_USER_DEFINE_MODE == MESH_AES_ENABLE)
u8 magic_aes_nums[16]={	0x11,0xff,0x22,0xee,0x33,0xdd,0x44,0xcc,
						0x55,0xbb,0x66,0xaa,0x77,0x99,0x88,0x88};

void caculate_aes_to_create_static_oob()
{
	#if !WIN32 
	u8 aes_in[16];
	u8 aes_out[16];
	memset(aes_in,0,sizeof(aes_in));
	memcpy(aes_in,tbl_mac,6);
	tn_aes_128(magic_aes_nums,aes_in,aes_out);
	mesh_set_dev_auth(aes_out, 16);
	#endif
}
#endif
void user_sha256_data_proc()
{
    #if (AIS_ENABLE)
    set_sha256_init_para_mode(1);
    #endif
}


void user_node_oob_set()
{
    if(AIS_ENABLE || (mesh_user_define_mode == MESH_AES_ENABLE)){
        set_node_prov_para_no_pubkey_static_oob();
    }else {
        #if 0 // output oob test
        //set_node_prov_para_no_pubkey_output_oob();//debug for the bluez 
        #else
        if(0){ // oob test // read from flash later
            set_node_prov_para_no_pubkey_static_oob();  // set static oob type
            u8 oob[16] = {0xff};
			mesh_set_dev_auth(oob,sizeof(oob));
        }else{
			u8 oob[16] = {0};
			if(memcmp(dev_auth, oob, 16)){
				set_node_prov_para_no_pubkey_static_oob();// static oob
			}
			else{
            	set_node_prov_capa_oob_init(); // no oob
			}
        }
        #endif
    }
}

u8 user_mac_proc()
{
	if(AIS_ENABLE && !MI_API_ENABLE){// not need to set the tbl mac address
		return 1;
	}else{
        return 0;
	}
}

#if MD_SERVER_EN
void user_power_on_proc()
{
#if ((MESH_USER_DEFINE_MODE != MESH_SPIRIT_ENABLE)&&!MI_API_ENABLE)
       mesh_tx_cmd_lightness_st(0, ele_adr_primary, 0xffff, LIGHTNESS_STATUS, 0, 0);
#endif
}
#endif

void user_mesh_cps_init()
{
	if(AIS_ENABLE){
		//gp_page0->head.cid = g_vendor_id;     // have been set default value
		gp_page0->head.pid = 0;
		gp_page0->head.vid = 0x0001;
		gp_page0->head.crpl = 100;
	}else{
	    // use pre-define value
	}
}

void user_set_def_sub_adr()
{
    #if (AIS_ENABLE)
    const u16 group_def_set[] = {0xc000, 0xcfff};
    foreach_arr(i,group_def_set){
        share_model_sub(CFG_MODEL_SUB_ADD, group_def_set[i], 0);
    }
    #endif
}

void user_system_time_proc()
{
#if(AIS_ENABLE)
	sha256_dev_uuid_str *p_uuid = (sha256_dev_uuid_str *)prov_para.device_uuid;
	if((p_uuid->adv_flag == 0)&&(clock_time_exceed_s(beacon_send.start_time_s, 10*60))){
		beacon_send.inter = 60*1000*1000;
		p_uuid->adv_flag = 1;
	}
#endif
}

//if return 0, will not send transation ack for link open cmd
int user_node_rc_link_open_callback()
{
	#if (MESH_USER_DEFINE_MODE == MESH_SPIRIT_ENABLE)
	sha256_dev_uuid_str *p_uuid = (sha256_dev_uuid_str *)prov_para.device_uuid;
	if(p_uuid->adv_flag){
		return 0;
	}	
	#endif
	return 1;
}


void prov_random_proc()
{
    #if !WIN32
    u8 *p_random = prov_para.random;
    for(int i=0;i<8;i++){
            p_random[i]= rand()&0xff;
    }
    #endif
    return ;
}

void mesh_provision_para_init()
{
	const u8 hash[8]={0x00,0x86,0x17,0x65,0xae,0xfc,0xc5,0x7b};
	mesh_provision_para_reset();
	prov_para.oob_info[0]=0x00;
	prov_para.oob_info[1]=0x00;
	provision_mag.pro_stop_flag = 1;// make the provisioner to initial state to stop
	memcpy(prov_para.hash,hash,sizeof(hash));// hash can caculate automatically 
	//provision_mag.unicast_adr_last =1;
	prov_para.ele_cnt =1;
	prov_random_proc();
	#if !WIN32
	user_prov_multi_device_uuid();// use the mac address part to create the device uuid part
	#if (!AIS_ENABLE)
	u8 oob_data[16];//get_flash_data_is_valid
	flash_read_page(FLASH_ADR_STATIC_OOB,16,oob_data);
	if(get_flash_data_is_valid(oob_data,sizeof(oob_data))){//oob was burned in flash
		mesh_set_dev_auth(oob_data,sizeof(oob_data));
	}else
	#endif
	{
		user_prov_multi_oob();	
	}
	#endif
	user_node_oob_set();
}

void user_prov_multi_oob()
{
#if !WIN32
    #if (AIS_ENABLE)
        caculate_sha256_to_create_static_oob();
    #elif (MESH_USER_DEFINE_MODE == MESH_AES_ENABLE)
        caculate_aes_to_create_static_oob();
    #else 
        // use static oob mode 
    #endif
#endif
}

void uuid_create_by_mac(u8 *mac,u8 *uuid)
{
// test md5 function part 
	/*
	static char name_string[]="www.widgets.com";
	uuid_create_md5_from_name((uuid_mesh_t *)uuid, NameSpace_DNS, name_string, 15);
	*/
    uuid_mesh_t NameSpace_DNS = { /* 6ba7b810-9dad-11d1-80b4-00c04fd430c8 */
            0x6ba7b810,
            0x9dad,
            0x11d1,
            0x80, 0xb4, 
            {0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8}
    };
        
	char name_string[16] = {0};
	memcpy(name_string,mac,6);
	uuid_create_md5_from_name((uuid_mesh_t *)uuid, NameSpace_DNS, name_string, 15);

    //special proc to set the mac address into the uuid part 
    #if MD_REMOTE_PROV
	uuid_mesh_t * p_uuid = (uuid_mesh_t * )uuid;
    memcpy(p_uuid->node,mac,6);
    #endif
}

void user_prov_multi_device_uuid()
{
#if !WIN32
    #if (AIS_ENABLE)
        set_dev_uuid_for_sha256();
    #elif (MESH_USER_DEFINE_MODE == MESH_AES_ENABLE)
        uuid_create_by_mac(tbl_mac,prov_para.device_uuid);
    #elif (MESH_USER_DEFINE_MODE == MESH_GN_ENABLE)
        set_dev_uuid_for_simple_flow(prov_para.device_uuid);
    #elif (MESH_USER_DEFINE_MODE == MESH_NORMAL_MODE || MESH_USER_DEFINE_MODE == MESH_IRONMAN_MENLO_ENABLE)
        if(PROVISION_FLOW_SIMPLE_EN){
		    set_dev_uuid_for_simple_flow(prov_para.device_uuid);
	    }else{
			uuid_create_by_mac(tbl_mac,prov_para.device_uuid);
		}
    #endif
#endif
}

