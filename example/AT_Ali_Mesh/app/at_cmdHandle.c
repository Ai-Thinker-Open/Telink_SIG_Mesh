#include "tl_common.h"
#include "at_cmd.h"
#include "drivers.h"

#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh/remote_prov.h"
#include "mesh/fast_provision_model.h"
#include "mesh/app_heartbeat.h"
#include "mesh/app_health.h"

#include "stack/ble/ble.h"

#define AT_VERSION "0.1"

#define STORAGE_NAME 1
#define STORAGE_BAUD 2
#define STORAGE_ATE  3
#define STORAGE_MODE 4

extern u8 baud_buf[];
extern  const u8 tbl_scanRsp[];
extern u8 my_scanRsp[32];
extern u8 ATE;
extern u8  mac_public[6];

u8 HEX2BYTE(u8 hex_ch)
{
	if (hex_ch >= '0' && hex_ch <= '9')
	{
		return hex_ch - '0';
	}

	if (hex_ch >= 'a' && hex_ch <= 'f')
	{
		return hex_ch - 'a' + 10;
	}

	if (hex_ch >= 'A' && hex_ch <= 'F')
	{
		return hex_ch - 'A' + 10;
	}

	return 0x00;
}

u8 HEX2BIN(u8 * p_hexstr, u8 * p_binstr)
{
	u8 bin_len = 0;
	u8 hex_len = strlen((char *)p_hexstr);
	u8 index = 0;

	if (hex_len % 2 == 1)
	{
		hex_len -= 1;
	}

	bin_len = hex_len / 2;
	for(index = 0; index < hex_len; index+=2)
	{
		p_binstr[index/2] = ((HEX2BYTE(p_hexstr[index]) << 4) & 0xF0) + HEX2BYTE(p_hexstr[index + 1]); 
	}

	return bin_len;
}

u8 HEX2U16(u8 * p_hexstr) //将16进制字符串转换成U16类型的整数
{
	u8 hexStr_len = strlen((char *)p_hexstr);
	u16 numBer = 0;
	u8 index = 0;

	for(index = 0; index < hexStr_len; index++)
	{
		numBer <<=4;
		numBer += HEX2BYTE(p_hexstr[index]);
	}

	return numBer;
}

u8 STR2U16(u8 * p_hexstr) //将10进制字符串转换成U16类型的整数
{
	u8 hexStr_len = strlen((char *)p_hexstr);
	u16 numBer = 0;
	u8 index = 0;

	for(index = 0; index < hexStr_len; index++)
	{
		numBer *= 10;
		numBer += HEX2BYTE(p_hexstr[index]);
	}

	return numBer;
}

static unsigned char atCmd_ATE0(char *pbuf,  int mode, int lenth)
{
	ATE = 0;
	//tinyFlash_Write(STORAGE_ATE, &ATE, 1);
	return 0;
}

static unsigned char atCmd_ATE1(char *pbuf,  int mode, int lenth)
{
	ATE = 1;
	//tinyFlash_Write(STORAGE_ATE, &ATE, 1);
	return 0;
}

static unsigned char atCmd_GMR(char *pbuf,  int mode, int lenth)
{
	at_print("\r\n+VER:"AT_VERSION);
	return 0;
}

static unsigned char atCmd_Reset(char *pbuf,  int mode, int lenth)
{
	at_print("\r\nOK\r\n");
	start_reboot();
	return 0;
}

static unsigned char atCmd_Restore(char *pbuf,  int mode, int lenth)
{
	irq_disable();
	factory_reset();
	at_print("\r\nOK\r\n");
	start_reboot();
	return 0;
}

static unsigned char atCmd_State(char *pbuf,  int mode, int lenth)
{
	if(is_provision_success())
	{
		at_print("\r\nSTATE:1\r\n");
	}
	else
	{
		at_print("\r\nSTATE:0\r\n");
	}
	
	return 0;
}

#include "mesh/mesh_node.h"
extern mesh_key_t mesh_key;  // not init here to decrease firmware size.
//mesh_key_t mesh_key;
static unsigned char atCmd_Netname(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_SET) //设置netKey
    {
        uint8_t key[16] = { 0 };

        if(lenth >15)
        {
            return 1;
        }

        memcpy(key,pbuf,lenth);

		memcpy(mesh_key.net_key[0][0].key,key,16);

		mesh_key.net_key[0][0].index = 0;
		mesh_key.net_key[0][0].valid = 1;

		mesh_key_save();
        return 0;
    }

	char buf[64] = {0};
	u_sprintf(buf,"+NETNAME:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
	mesh_key.net_key[0][0].key[0],
	mesh_key.net_key[0][0].key[1],
	mesh_key.net_key[0][0].key[2],
	mesh_key.net_key[0][0].key[3],
	mesh_key.net_key[0][0].key[4],
	mesh_key.net_key[0][0].key[5],
	mesh_key.net_key[0][0].key[6],
	mesh_key.net_key[0][0].key[7],
	mesh_key.net_key[0][0].key[8],
	mesh_key.net_key[0][0].key[9],
	mesh_key.net_key[0][0].key[10],
	mesh_key.net_key[0][0].key[11],
	mesh_key.net_key[0][0].key[12],
	mesh_key.net_key[0][0].key[13],
	mesh_key.net_key[0][0].key[14],
	mesh_key.net_key[0][0].key[15]
	);
	
	at_print(buf);
	return 0;
}

static unsigned char atCmd_Pasword(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_SET) //设置netKey
    {
        uint8_t key[16] = { 0 };

        if(lenth >15)
        {
            return 1;
        }

        memcpy(key,pbuf,lenth);

		memcpy(mesh_key.net_key[0][0].app_key[0].key,key,16);
		mesh_key.net_key[0][0].app_key[0].index = 0;
		mesh_key.net_key[0][0].app_key[0].valid = 1;

		mesh_key_save();
        return 0;
    }

	char buf[64] = {0};
	u_sprintf(buf,"+PASWORD:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
	mesh_key.net_key[0][0].app_key[0].key[0],
	mesh_key.net_key[0][0].app_key[0].key[1],
	mesh_key.net_key[0][0].app_key[0].key[2],
	mesh_key.net_key[0][0].app_key[0].key[3],
	mesh_key.net_key[0][0].app_key[0].key[4],
	mesh_key.net_key[0][0].app_key[0].key[5],
	mesh_key.net_key[0][0].app_key[0].key[6],
	mesh_key.net_key[0][0].app_key[0].key[7],
	mesh_key.net_key[0][0].app_key[0].key[8],
	mesh_key.net_key[0][0].app_key[0].key[9],
	mesh_key.net_key[0][0].app_key[0].key[10],
	mesh_key.net_key[0][0].app_key[0].key[11],
	mesh_key.net_key[0][0].app_key[0].key[12],
	mesh_key.net_key[0][0].app_key[0].key[13],
	mesh_key.net_key[0][0].app_key[0].key[14],
	mesh_key.net_key[0][0].app_key[0].key[15]
	);
	at_print(buf);
	return 0;
}

static unsigned char atCmd_Send(char *pbuf,  int mode, int lenth)
{
	char * tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }

	tmp[0] = 0; tmp++;

	u16 addr_dst = HEX2U16(pbuf);  //获取目的地址

	pbuf = tmp;

	tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }
	tmp[0] = 0; tmp++;

	u16 data_len = STR2U16(pbuf); //获取数据长度

	SendOpParaDebug(addr_dst,0,0x0182, tmp,data_len);
	return 0;
}

extern const u8 USER_DEFINE_ATT_HANDLE ;
extern u32 device_in_connection_state;//连接状态
//AT+SEND2APP=5,12345
static unsigned char atCmd_Send2App(char *pbuf,  int mode, int lenth)
{
	if(device_in_connection_state == 0) return 2;

	char * tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }

	tmp[0] = 0; tmp++;

	u16 data_len = STR2U16(pbuf); //获取数据长度

	bls_att_pushNotifyData(USER_DEFINE_ATT_HANDLE, tmp, data_len); //release
	return 0;
}
static unsigned char atCmd_Addr(char *pbuf,  int mode, int lenth)
{
	char buf[64] = {0};

	if(mode == AT_CMD_MODE_SET) //设置netKey
    {
		mesh_set_ele_adr(HEX2U16(pbuf));
        return 0;
    }

	if(mode == AT_CMD_MODE_READ)
	{
		char buf[64] = {0};
		u_sprintf(buf,"+ADDR:%X", ele_adr_primary);
		at_print(buf);
		return 0;
	}
}
_at_command_t gAtCmdTb_writeRead[] =
{ 
	{ "NETNAME", 	atCmd_Netname,	"Set/Read NetName\r\n"},
	{ "PASWORD", 	atCmd_Pasword,	"Set/Read Password\r\n"},
	{ "MESHSEND", 	atCmd_Send,	"Send data to other module\r\n"},	
	{ "SEND2APP", 	atCmd_Send2App,	"Send data to other module\r\n"},	
	{ "ADDR", 		atCmd_Addr,	"Set/Read module address\r\n"},	
	{0, 	0,	0}
};

_at_command_t gAtCmdTb_exe[] =
{
	{ "1", 		atCmd_ATE1, "ATE1\r\n"},  //ATE1
	{ "0", 		atCmd_ATE0, "ATE0\r\n"},  //ATE0
	{ "GMR", 	atCmd_GMR,  "Get Version\r\n"}, 
	{ "RST", 	atCmd_Reset, "RESET\r\n"}, 
	{ "RESTORE",atCmd_Restore, "Restore Factory\r\n"}, 
	{ "STATE", 	atCmd_State, "Prove State\r\n"}, 
	{0, 	0,	0}
};
