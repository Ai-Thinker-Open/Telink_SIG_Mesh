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

#define AT_VERSION "0.2"

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

u16 HEX2BIN(u8 * p_hexstr, u8 * p_binstr)
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

u16 HEX2U16(u8 * p_hexstr) //将16进制字符串转换成U16类型的整数
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

//AT+SEND2ALI=8421,12345678
static unsigned char atCmd_Send2Ali(char *pbuf,  int mode, int lenth)
{
	char * tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }

	tmp[0] = 0; tmp++;

	u16 op = HEX2U16((u8*)pbuf);  //获取Op Code
	u32 len = lenth -(tmp - pbuf);

	for(int i = 0; i < len; i ++) //将16进制字符串转换成二进制数组
	{
		if(((tmp[i] >= '0') && (tmp[i] <= '9')) || ((tmp[i] >= 'A') && (tmp[i] <= 'F')))
		{
			if((tmp[i] >= '0') && (tmp[i] <= '9'))
			{
				tmp[i] -= '0';
			}
			else
			{
				tmp[i] -= 'A';
				tmp[i] += 0x0A;
			}

			if(i%2)
			{
				tmp[i/2] = (tmp[i-1] << 4) | tmp[i];
			}
		}
		else
		{
			return 2;
		}
	}

	mesh_tx_cmd_rsp(op, (u8 *)tmp, len/2, ele_adr_primary, 0xffff, 0, 0);
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

	u16 data_len = STR2U16((u8*)pbuf); //获取数据长度

	bls_att_pushNotifyData(USER_DEFINE_ATT_HANDLE, (u8*)tmp, data_len); //release
	return 0;
}

_at_command_t gAtCmdTb_writeRead[] =
{ 
	{ "SEND2ALI", 	atCmd_Send2Ali,		"Send data to Tmall \r\n"},	
	{ "SEND2APP", 	atCmd_Send2App,	"Send data to phone\r\n"},	
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
