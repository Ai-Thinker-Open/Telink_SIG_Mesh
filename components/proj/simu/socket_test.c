/********************************************************************************************************
 * @file     socket_test.c 
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

#include "socket_test.h"

#ifdef WIN32

#if 0

#include "../os/ev.h"

SOCKET server_socket;
SOCKET client_socket;
SOCKADDR_IN socketadd_local;
SOCKADDR_IN socketadd_remote;

s8 send_ip[32];
u32 send_port;
u32 rece_port;

extern u32 g_testcount;
extern void drvr_write_rx_buf(u8 *buf, u8 len);
extern void trace_out(u8 * out, u32 len);
#if 1
#define MAX_FILE_SIZE 256
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'
static int load_ini_file(const char *file, char *buf,int *file_size)
{
    FILE *in = NULL;
    int i=0;
    *file_size =0;

    assert(file !=NULL);
    assert(buf !=NULL);

    in = fopen(file,"r");
    if( NULL == in) {
        return 0;
    }

    buf[i]=fgetc(in);
    
    //load initialization file
    while( buf[i]!= (char)EOF) {
        i++;
        assert( i < MAX_FILE_SIZE); //file too big
        buf[i]=fgetc(in);
    }
    
    buf[i]='\0';
    *file_size = i;

    fclose(in);
    return 1;
}

static int isnewline(char c)
{
    return ('\n' == c ||  '\r' == c )? 1 : 0;
}

static int isend(char c)
{
    return '\0'==c? 1 : 0;
}

static int isleftbarce(char c)
{
    return LEFT_BRACE == c? 1 : 0;
}

static int isrightbrace(char c )
{
    return RIGHT_BRACE == c? 1 : 0;
}

static int parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
                      int *key_s,int *key_e, int *value_s, int *value_e)
{
    const char *p = buf;
    int i=0;

    assert(buf!=NULL);
    assert(section != NULL && strlen(section));
    assert(key != NULL && strlen(key));

    *sec_e = *sec_s = *key_e = *key_s = *value_s = *value_e = -1;

    while( !isend(p[i]) ) {
        //find the section
        if( ( 0==i ||  isnewline(p[i-1]) ) && isleftbarce(p[i]) )
        {
            int section_start=i+1;

            //find the ']'
            do {
                i++;
            } while( !isrightbrace(p[i]) && !isend(p[i]));

            if( 0 == strncmp(p+section_start,section, i-section_start)) {
                int newline_start=0;

                i++;

                //Skip over space char after ']'
                while(isspace(p[i])) {
                    i++;
                }

                //find the section
                *sec_s = section_start;
                *sec_e = i;

                while( ! (isnewline(p[i-1]) && isleftbarce(p[i])) 
                && !isend(p[i]) ) {
                    int j=0;
                    //get a new line
                    newline_start = i;

                    while( !isnewline(p[i]) &&  !isend(p[i]) ) {
                        i++;
                    }
                    
                    //now i  is equal to end of the line
                    j = newline_start;

                    if(';' != p[j]) //skip over comment
                    {
                        while(j < i && p[j]!='=') {
                            j++;
                            if('=' == p[j]) {
                                if(strncmp(key,p+newline_start,j-newline_start)==0)
                                {
                                    //find the key ok
                                    *key_s = newline_start;
                                    *key_e = j-1;

                                    *value_s = j+1;
                                    *value_e = i;

                                    return 1;
                                }
                            }
                        }
                    }

                    i++;
                }
            }
        }
        else
        {
            i++;
        }
    }
    return 0;
}

int read_profile_string( const char *section, const char *key,char *value, 
         int size, const char *default_value, const char *file)
{
    char buf[MAX_FILE_SIZE]={0};
    int file_size;
    int sec_s,sec_e,key_s,key_e, value_s, value_e;

    //check parameters
    assert(section != NULL && strlen(section));
    assert(key != NULL && strlen(key));
    assert(value != NULL);
    assert(size > 0);
    assert(file !=NULL &&strlen(key));

    if(!load_ini_file(file,buf,&file_size))
    {
        if(default_value!=NULL)
        {
            strncpy(value,default_value, size);
        }
        return 0;
    }

    if(!parse_file(section,key,buf,&sec_s,&sec_e,&key_s,&key_e,&value_s,&value_e))
    {
        if(default_value!=NULL)
        {
            strncpy(value,default_value, size);
        }
        return 0; //not find the key
    }
    else
    {
        int cpcount = value_e -value_s;

        if( size-1 < cpcount)
        {
            cpcount =  size-1;
        }
    
        memset(value, 0, size);
        memcpy(value,buf+value_s, cpcount );
        value[cpcount] = '\0';

        return 1;
    }
}

int read_profile_int( const char *section, const char *key,int default_value, 
                const char *file)
{
    char value[32] = {0};
    if(!read_profile_string(section,key,value, sizeof(value),NULL,file))
    {
        return default_value;
    }
    else
    {
        return atoi(value);
    }
}
#endif

#if (TL_RF4CE && RF4CE_COORD)
/*server: send to 7002*/
void server_socket_send(u8 *buf, u8 len){
    socketadd_remote.sin_family = AF_INET;
    socketadd_remote.sin_port = htons(send_port);
    socketadd_remote.sin_addr.S_un.S_addr = inet_addr((char*)(&send_ip[0]));
    printf("\ng_testcount=%d send to client ip:%s to client port:%d",g_testcount++,send_ip,send_port);
    trace_out(buf,len);
    sendto(server_socket,(char*)buf,len,0,(SOCKADDR*)&socketadd_remote,sizeof(SOCKADDR));
}

/*server:receive from 7001*/
void server_socket_receive(){
    char recvBuf[RECEIVE_MAX_LENGTH];
    int ret;
    int len = sizeof(SOCKADDR);
    
    memset(&socketadd_remote,0x00,len);
    memset(recvBuf,0x00,sizeof(recvBuf));
    ret = recvfrom(server_socket,recvBuf,sizeof(recvBuf),0,(SOCKADDR*)&socketadd_remote,&len);
    if (ret > 0){
        printf("\ng_testcount=%d receive from client ip:%s from client port:%d",g_testcount++,inet_ntoa(socketadd_remote.sin_addr), ntohs(socketadd_remote.sin_port));
        trace_out((u8*)recvBuf,ret);
        drvr_write_rx_buf((u8*)recvBuf,ret);
    }
}
#else
/*client: send to 7001*/
void client_socket_send(u8 *buf, u8 len){
    socketadd_remote.sin_family = AF_INET;
    socketadd_remote.sin_port = htons(send_port);
    socketadd_remote.sin_addr.S_un.S_addr = inet_addr(send_ip);
    printf("\ng_testcount=%d send to server ip:%s to server port:%d",g_testcount++,send_ip,send_port);
    trace_out(buf,len);
    sendto(client_socket,(char*)buf,len,0,(SOCKADDR*)&socketadd_remote,sizeof(SOCKADDR));
}

/*client: receive from 7002*/
void client_socket_receive(){
    char recvBuf[RECEIVE_MAX_LENGTH];
    int ret;
    int len = sizeof(SOCKADDR);
    
    memset(&socketadd_remote,0x00,len);
    memset(recvBuf,0x00,sizeof(recvBuf));
    ret = recvfrom(client_socket,recvBuf,sizeof(recvBuf),0,(SOCKADDR*)&socketadd_remote,&len);
    if (ret > 0){
        printf("\ng_testcount=%d receive from server ip:%s from server port:%d",g_testcount++,inet_ntoa(socketadd_remote.sin_addr), ntohs(socketadd_remote.sin_port));
        trace_out((u8*)recvBuf,ret);
        drvr_write_rx_buf((u8*)recvBuf,ret);
    }
}
#endif

void socket_init(){
     WSADATA ws;
     int ret;
     int len = sizeof(SOCKADDR);
     ret = WSAStartup(MAKEWORD(2,2),&ws);
     if (0 != ret){
        return;
     }
    
#if (TL_RF4CE && RF4CE_COORD)
    /*ini file*/
    memset(send_ip,0x00,sizeof(send_ip));
    read_profile_string("simu","send_ip",(char*)&send_ip[0],sizeof(send_ip),"127.0.0.1","simu_s.ini");
    printf("\n%s %d: send_ip=%s.",__FUNCTION__,__LINE__,send_ip);
    send_port = read_profile_int("simu","send_port",7002,"simu_s.ini");
    printf("\n%s %d: send_port=%d.",__FUNCTION__,__LINE__,send_port);
    rece_port = read_profile_int("simu","rece_port",7001,"simu_s.ini");
    printf("\n%s %d: rece_port=%d.",__FUNCTION__,__LINE__,rece_port);
    
    server_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (INVALID_SOCKET == server_socket){
        WSACleanup();
        return;
    }

    socketadd_local.sin_family = AF_INET;
    socketadd_local.sin_port = htons(rece_port);
    socketadd_local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    ret = bind(server_socket,(SOCKADDR*)&socketadd_local,len);
    if (SOCKET_ERROR == ret){
        WSACleanup();
        return;
    }
    int mode = 1;
    ioctlsocket(server_socket,FIONBIO,(u_long FAR*)&mode);
    
#if(FREAKZ_ENABLE)    
    ev_on_poll(EV_POLL_SIM_RECV,server_socket_receive);
#endif
#else
    /*ini file*/
    memset(send_ip,0x00,sizeof(send_ip));
    read_profile_string("simu","send_ip",send_ip,sizeof(send_ip),"127.0.0.1","simu_c.ini");
    printf("\n%s %d: send_ip=%s.",__FUNCTION__,__LINE__,send_ip);
    send_port = read_profile_int("simu","send_port",7001,"simu_c.ini");
    printf("\n%s %d: send_port=%d.",__FUNCTION__,__LINE__,send_port);
    rece_port = read_profile_int("simu","rece_port",7002,"simu_c.ini");
    printf("\n%s %d: rece_port=%d.",__FUNCTION__,__LINE__,rece_port);
    
    client_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (INVALID_SOCKET == client_socket){
        WSACleanup();
        return;
    }

    socketadd_local.sin_family = AF_INET;
    socketadd_local.sin_port = htons(rece_port);
    socketadd_local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    ret = bind(client_socket,(SOCKADDR*)&socketadd_local,len);
    if (SOCKET_ERROR == ret){
        return;
    } 
    int mode = 1;
    ioctlsocket(client_socket,FIONBIO,(u32 FAR*)&mode);
    
    ev_on_poll(EV_POLL_SIM_RECV,client_socket_receive);
#endif
}

#endif  /* 0 */

#endif

