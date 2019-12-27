/********************************************************************************************************
 * @file     socket_api.c 
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
#include "../common/types.h"
#include "../common/assert.h"
#include "socket_api.h"

#ifdef WIN32

#include "../os/ev.h"
#include<stdio.h>
#include<winsock2.h>
#include <windows.h>
#include <time.h>



static SOCKET server_socket_serial;
static SOCKET server_socket_rf;
static SOCKET server_socket_wireshark;
static SOCKET client_socket_serial;
static SOCKET client_socket_rf;
static SOCKET client_socket_wireshark;
static SOCKADDR_IN socketadd_local_serial;
static SOCKADDR_IN socketadd_local_rf;
static SOCKADDR_IN socketadd_local_wireshark;
static SOCKADDR_IN socketadd_remote_serial;
static SOCKADDR_IN socketadd_remote_rf;
static SOCKADDR_IN socketadd_remote_wireshark;

static s8 send_ip[32];
static s8 send_ip_wireshark[32] = {"192.168.56.1\0"};
static u32 send_port_serial;
static u32 send_port_rf;
static u32 rece_port_serial;
static u32 rece_port_rf;

extern u32 g_testcount;
extern void drvr_write_rx_buf(u8 *buf, u8 len);
extern void trace_out(u8 * out, u32 len);


handle_recv_data handle_received_serial_data;
handle_recv_data handle_received_rf_data;

#if (TL_RF4CE && RF4CE_COORD)
#define SEND_SERIAL_PORT           7001
#define SEND_RF_PORT               8001
#define RECV_SERIAL_PORT           7002
#define RECV_RF_PORT               8002
#else
#define SEND_SERIAL_PORT           7003
#define SEND_RF_PORT               8003
#define RECV_SERIAL_PORT           7004
#define RECV_RF_PORT               8004
#endif



#define MAX_DST_SOCK_NUM    4
#define MAX_ASCII_LINE_SZ   256



#define SIMU_PORT_OFFSET_ON_CH_15   0
#define SIMU_PORT_OFFSET_ON_CH_20   1
#define SIMU_PORT_OFFSET_ON_CH_25   2


typedef struct {
	SOCKET sock;
	u8 ip[32];
	u32 port;
} sock_t;



typedef struct {
    u32    channel;                       /* channel 15: port+0, 20:port+1, 25:port+2 */

    sock_t srcSock;                       /* rf source socket, UDP */

    u8     dstSockNum;                    /* rf dst socket number, which means how many other node in the network */
    sock_t dstSock[MAX_DST_SOCK_NUM];     /* rf dst socket ip address and port information */
    sock_t wiresharkSock;                 /* Wireshark socket, which is used to capture sniffer log */

    sock_t thSock;                        /* Test harness socket, TCP server */
    sock_t thRemoteSock;
}simu_ctrl_t;


simu_ctrl_t simu_vars;
simu_ctrl_t *simu_v = &simu_vars;




void copyIpStr(char* dst, char* src)
{
    while( (*src != '\r') && (*src != '\n') && (*src != ';' )) {
        *dst ++ = *src++;
    }
}



int simu_readIpPortCfg(FILE* fp, u8* maxSockNum, sock_t* pSock)
{
    char     lineStr[MAX_ASCII_LINE_SZ + 1];
    char*    strSearch;
    u32 len = 15;
    u8 sockIdx = 0;

    while(!feof(fp)) {
        fgets(lineStr, MAX_ASCII_LINE_SZ, fp);
        if (*lineStr==';' || *lineStr=='#' || *lineStr==' ' ) {
            /* comment lines */
            continue;
        }

        else if (NULL != strstr(lineStr, "num") &&
            NULL != (strSearch = strstr(lineStr, "=")) ) {
            *maxSockNum = atoi(strSearch+1);
        }

        else if (NULL != strstr(lineStr, "ip") &&
            NULL != (strSearch = strstr(lineStr, "=")) ) {
            copyIpStr((char*)(pSock->ip), strSearch+2);
        }

        else if (NULL != strstr(lineStr, "port") &&
            NULL != (strSearch = strstr(lineStr, "=")) ) {
            pSock->port = atoi(strSearch+1);
            if (*maxSockNum == ++sockIdx) {
                return 0;
            } else {
                pSock++;
            }
        }
    }

    return 0;
}


int simu_readCfgFile(const char* filename)
{
    FILE* fp;
    char  lineStr[MAX_ASCII_LINE_SZ + 1];
    u8 num = 1;
    sock_t *pSock;
    u8 *pMaxSockNum;
    u8 fReadDetail = 0;


    /* open the configuration file. */
    if (NULL == filename) {
        return -1;
    }

    if ( NULL == (fp=fopen(filename, "r")) ) {
        printf("the file %s opened failed!\n", filename);
        return -1;
    }

    while(!feof(fp)) {
        fgets(lineStr, MAX_ASCII_LINE_SZ, fp);

        if(*lineStr==';' || *lineStr=='#' || *lineStr==' ' || *lineStr =='\n') {
            /* comment lines */
            continue;
        }
        else if(NULL != strstr(lineStr, "[src]")) {
            pSock = &simu_v->srcSock;
            pMaxSockNum = &num;
            fReadDetail = 1;
        }
        else if(NULL != strstr(lineStr, "[dst]")) {
            pSock = &simu_v->dstSock[0];
            pMaxSockNum = &simu_v->dstSockNum;
            fReadDetail = 1;
        }
        else if(NULL != strstr(lineStr, "[wireshark]")) {
            pSock = &simu_v->wiresharkSock;
            pMaxSockNum = &num;
            fReadDetail = 1;
        }
        else if(NULL != strstr(lineStr, "[TcpServer]")) {
            pSock = &simu_v->thSock;
            pMaxSockNum = &num;
            fReadDetail = 1;
        }
        if (fReadDetail) {
            simu_readIpPortCfg(fp, pMaxSockNum, pSock);
            fReadDetail = 0;
        }
        
    }
}


void sock_init(sock_t *pSock, u8 type)
{
    SOCKADDR_IN sockaddr;
    int ret;
    int len = sizeof(SOCKADDR);

    /* Create corresponding socket */
    if (SIMU_SOCK_UDP == type) {
        pSock->sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if (INVALID_SOCKET == pSock->sock){
            WSACleanup();
            return;
        }
    } else {
        pSock->sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (INVALID_SOCKET == pSock->sock){
            WSACleanup();
            return;
        }
    }


    /* Bind to address */
    sockaddr.sin_family = AF_INET;
    if (SIMU_SOCK_UDP == type) {
        sockaddr.sin_port = htons(pSock->port + simu_v->channel);
    } else {
        sockaddr.sin_port = htons(pSock->port);
    }
    
    sockaddr.sin_addr.S_un.S_addr = inet_addr((char*)pSock->ip);

    ret = bind(pSock->sock,(SOCKADDR*)&sockaddr, len);
    if (SOCKET_ERROR == ret){
        WSACleanup();
        return;
    }

    int mode = 1;

    /* Set socket option of Non-block */
    int iResult = ioctlsocket(pSock->sock,FIONBIO,(u_long FAR*)&mode);
    if (iResult != NO_ERROR) {
        printf("ioctlsocket failed with error: %ld\n", iResult);
    }

    if (SIMU_SOCK_TCP == type) {
        if(-1 == listen(pSock->sock, 255)) {
            printf("sock_init: socket listen failed.\n");
            return;
        }

        struct linger m_sLinger;
        struct sockaddr_in fromAddr;
        int fromLen = sizeof(struct sockaddr_in);

        /* Wait until Test Client connected */
        while (!simu_v->thRemoteSock.sock) {
            if (-1 == (simu_v->thRemoteSock.sock = accept(pSock->sock, (struct sockaddr*)(&fromAddr), &fromLen))) {
                //printf("net_recv: accept error! errno = %d\n", errno);
                simu_v->thRemoteSock.sock = 0;
            }
            Sleep(1);
        }
        
        m_sLinger.l_onoff = 1;
        m_sLinger.l_linger = 0;
        setsockopt(simu_v->thRemoteSock.sock, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger,sizeof(m_sLinger));

        
    }    
}


void win32_param_init(void)
{
    WSADATA ws;
    int ret;
    int len = sizeof(SOCKADDR);
    ret = WSAStartup(MAKEWORD(2,2),&ws);
    
    if (0 != ret){
        return;
    }

    simu_readCfgFile("simuCfg.ini");

    srand((int)time(0));
}

void socket_param_init(u8 type)
{
    switch(type){
    case SIMU_SOCKET:
        /*initialize socket for simulator */
        sock_init(&simu_v->srcSock, SIMU_SOCK_UDP);
        break;
    case TEST_SOCKET:
        /*initialize socket for host for test */
        sock_init(&simu_v->thSock, SIMU_SOCK_TCP);
        break;

    case WIRESHARK_SOCKET:
        /*initialize socket wireshark */
        sock_init(&simu_v->wiresharkSock, SIMU_SOCK_UDP);
        break;

    default:
    
        break;
    }

}



int simu_recvFromRF(u8 *recvBuf, u8 bufLen)
{
    int ret;
    int len = sizeof(SOCKADDR);

    memset(&socketadd_remote_rf,0x00,len);
    ret = recvfrom(simu_v->srcSock.sock,(char*)recvBuf,bufLen,0,(SOCKADDR*)&socketadd_remote_rf,&len);
    //Sleep(1);
    return ret;
}


int simu_recvFromTH(u8 *recvBuf, u8 bufLen)
{
    int ret;
    int len = sizeof(SOCKADDR);

    ret = recv(simu_v->thRemoteSock.sock,(char*)recvBuf,bufLen,0);
    //Sleep(1);
    /* Tcp Disconnect */
    if (0 == ret) {
        /* Close socket and restart the connection */
        closesocket(simu_v->thRemoteSock.sock);
        closesocket(simu_v->thSock.sock);
        simu_v->thSock.sock = 0;
        simu_v->thRemoteSock.sock = 0;
        sock_init(&simu_v->thSock, SIMU_SOCK_TCP);
    }

    return ret;
}

int simu_sendToTH(u8* buf, u8 len)
{
    int ret;    

    if (-1 == (ret = send(simu_v->thRemoteSock.sock, (char*)buf, len, 0))) {
        printf("\nnet_send: send error! errno = %d", errno);
        return ret;
    }

    return ret;
}

int simu_sendToRF(u8 *buf, u8 len)
{
    //printf("\ng_testcount=%d send to client ip:%s to client port:%d",g_testcount++,send_ip,send_port);
    u8 i = 0;
    int ret;
    for(i = 0; i < simu_v->dstSockNum; i++) {
        socketadd_remote_rf.sin_family = AF_INET;
        socketadd_remote_rf.sin_port = htons(simu_v->dstSock[i].port + simu_v->channel);
        socketadd_remote_rf.sin_addr.S_un.S_addr = inet_addr((char*)(simu_v->dstSock[0].ip));

        ret = sendto(simu_v->srcSock.sock,(char*)buf,len,0,(SOCKADDR*)&socketadd_remote_rf,sizeof(SOCKADDR));
        if (ret == -1) {
            break;
        }
    }

    return ret;
}


int simu_sendToWireshark(u8 *buf, u8 len)
{
    socketadd_remote_wireshark.sin_family = AF_INET;
    socketadd_remote_wireshark.sin_port = htons(simu_v->wiresharkSock.port);
    socketadd_remote_wireshark.sin_addr.S_un.S_addr = inet_addr((char*)simu_v->wiresharkSock.ip);
    //printf("\ng_testcount=%d send to client ip:%s to client port:%d",g_testcount++,send_ip,send_port);
    //trace_out(buf,len);
    return sendto(simu_v->srcSock.sock,(char*)buf,len,0,(SOCKADDR*)&socketadd_remote_wireshark,sizeof(SOCKADDR));
}


void simu_setChannel(u8 ch) 
{
    if (15 == ch) {
        simu_v->channel = SIMU_PORT_OFFSET_ON_CH_15;
    } else if (20 == ch) {
        simu_v->channel = SIMU_PORT_OFFSET_ON_CH_20;
    } else if (25 == ch) {
        simu_v->channel = SIMU_PORT_OFFSET_ON_CH_25;
    }
    
    printf("\r\n------------------Current Ch %d --------\r\n", ch);
    closesocket(simu_v->srcSock.sock);
    sock_init(&simu_v->srcSock, SIMU_SOCK_UDP);
}


void init_random(void)
{
	srand((int)time(0));
}

u16 my_random()
{
    return rand();
}



void vs_sockEnvironmentInit(void)
{
	WSADATA ws;
	int ret;
	int len = sizeof(SOCKADDR);
	ret = WSAStartup(MAKEWORD(2,2),&ws);
	if (0 != ret){
		return;
	}
}



sock_t appSock;
void appSock_init(const char *ip, u32 port, u8 type)
{
	memcpy(appSock.ip, ip, 32);
	appSock.port = port;
	sock_init(&appSock, SIMU_SOCK_UDP);
}



int appSock_recv(u8 *recvBuf, u8 bufLen)
{
	int ret;
	int len = sizeof(SOCKADDR);

	memset(&socketadd_remote_rf,0x00,len);
	ret = recvfrom(appSock.sock,(char*)recvBuf,bufLen,0,(SOCKADDR*)&socketadd_remote_rf,&len);
	//Sleep(1);
	return ret;
}




#endif  /* WIN32 */

