/********************************************************************************************************
 * @file     uart_simu.c 
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
#include "uart_simu.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef __CYGWIN
#include <termios.h>
#include <errno.h>
#else
#include <windows.h>
#endif



/**************************** Private type Definitions ************************/

#ifdef __CYGWIN
static int speed_arr[]={ B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300};
static int name_arr[] ={ 115200, 38400, 19200, 9600, 4800, 2400, 1200, 300};
#endif


/**************************** Private Function Prototypes *********************/

u32       static halUartSetBaud(SERIAL_ID fd, u32 speed);
u32       static halUartSetParity(SERIAL_ID fd, parity_t parity);
u32       static halUartSetStopbits(SERIAL_ID fd, u8 stopBits);
u32       static halUartSetBasic(SERIAL_ID fd);
SERIAL_ID static halUartOpen(u8 port);

/**************************** Public Functions Definitions ********************/

SERIAL_ID halUartInit(u8 port, u32 baudRate, parity_t parity, u8 stopBits, u8 mode)
{
    SERIAL_ID fd;
    u32 rst;

    fd = halUartOpen(port);
    if (fd == ERR_SERIAL_ID) {
        return fd;
    }

    rst = halUartSetBasic(fd);
    if (rst != ERR_NONE) {
        halUartClose(fd);
        return ERR_SERIAL_ID;
    }

    rst = halUartSetBaud(fd, baudRate);
    if (rst != ERR_NONE) {
        halUartClose(fd);
        return ERR_SERIAL_ID;
    }

    rst = halUartSetParity(fd, parity);
    if (rst != ERR_NONE) {
        halUartClose(fd);
        return ERR_SERIAL_ID;
    }

    rst = halUartSetStopbits(fd, stopBits);
    if (rst != ERR_NONE) {
        halUartClose(fd);
        return ERR_SERIAL_ID;
    }

    return fd;
}


/******************************************************************************/


u32 halUartWrite(SERIAL_ID fd, u8* data, u32 length)
{
    u8 offset;
#ifdef __CYGWIN

    int sendLen = write(fd, data, length);
    if(sendLen < 0) {
        return ERR_SERIAL_SEND_FRAME;
    }

#else

    int errornum;
    int nWrite=0;
	int sendLen = WriteFile(fd , data , length, (DWORD *)&nWrite ,NULL );
	if(!sendLen) {
        errornum = GetLastError();
		printf("send error 1 = %d\n",errornum);
        return ERR_SERIAL_SEND_FRAME;
    }


#endif

#if 0
    /* for Debug */
    for (offset = 0; offset < length; offset++) {
       printf("%02x ", data[offset]);
    }
    printf("\n");
#endif

    return ERR_NONE;
}

/******************************************************************************/

u32 halUartRead(SERIAL_ID fd, u8* data, u32 length)
{

    int thisRead;
    int offset = 0;

#ifdef __CYGWIN
    do {
        thisRead = read(fd, &data[offset], length);
        if (thisRead < 0) {
            if (errno == EINTR) {
                break;
            }
            else {
                perror("error!");
            }
            return ERR_SERIAL_RECV_FRAME;
        }
        offset += thisRead;
    } while(offset < length);

#else

    int nRead = 0;
    do {
        thisRead = ReadFile(fd , &data[offset] , length ,(DWORD *)&nRead , NULL);
        if (thisRead == 0) {
            printf("recv error\n");
            return ERR_SERIAL_RECV_FRAME;
        }
        offset += nRead;
    } while(offset < length);

#endif

#if 0
    /* for Debug */
    for (offset = 0; offset < length; offset++) {
       printf("%02x ", data[offset]);
    }
    printf("\n");
#endif

    return ERR_NONE;
}


u32 halUartRead_nonBlock_byte(SERIAL_ID fd, u8* data)
{
    int thisRead;
    int offset = 0;

#ifdef __CYGWIN
    thisRead = read(fd, data, 1);
    if (thisRead < 0) {
        if (errno == EINTR) {
            perror("error!");
        }
        else {
            perror("error!");
        }
        return ERR_SERIAL_RECV_FRAME;
    }


#else

    int nRead = 0;

    thisRead = ReadFile(fd , data , 1 ,(DWORD *)&nRead , NULL);
    if (thisRead == 0) {
        printf("recv error\n");
        return ERR_SERIAL_RECV_FRAME;
    }


#endif

    return ERR_NONE;
}

/******************************************************************************/

void halUartClose(SERIAL_ID fd)
{
#ifdef __CYGWIN
    close(fd);
#else
    CloseHandle(fd);
#endif
}


/**************************** Private Functions Definitions ********************/


u32 static halUartSetBaud(SERIAL_ID fd, u32 speed)
{
#ifdef __CYGWIN

    struct termios options;
    u8 i;

    tcgetattr(fd, &options);
    for(i=0; i<sizeof(speed_arr)/sizeof(int); i++)	{
		if(speed == name_arr[i]) {
		    /* set io speed */
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);

			/*set working mode*/
			tcsetattr(fd, TCSANOW, &options);
			break;
		}
		tcflush(fd, TCIOFLUSH);
	}

	return ERR_NONE;
#else
    DCB dcb;
    int status;
    int errornum;

    status = GetCommState(fd, &dcb);
    if (!status) {
        errornum = GetLastError();
		printf("get comm state error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    dcb.BaudRate = speed;

    status = SetCommState(fd, &dcb);
	if (!status) {
        errornum = GetLastError();
		printf("set DCB error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    return ERR_NONE;

#endif
}

/******************************************************************************/

u32 static halUartSetParity(SERIAL_ID fd, parity_t parity)
{
#ifdef __CYGWIN

    struct termios options;
    u8 i;

    tcgetattr(fd, &options);

    switch (parity) {
        case PARITY_UART_NONE:
            options.c_cflag &= ~PARENB ;
            break;
        case PARITY_UART_ODD:
            options.c_cflag |= PARENB;
		    options.c_cflag &= PARODD;
            break;
        case PARITY_UART_EVEN:
            options.c_cflag |= PARENB;
		    options.c_cflag |= PARODD;
            break;
        break;
    }

    tcsetattr(fd, TCSANOW, &options);
    tcflush(fd, TCIOFLUSH);

	return ERR_NONE;
#else
    DCB dcb;
    int status;
    int errornum;

    status = GetCommState(fd, &dcb);
    if (!status) {
        errornum = GetLastError();
		printf("get comm state error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    switch (parity) {
        case PARITY_UART_NONE: dcb.Parity = NOPARITY;   break;
        case PARITY_UART_ODD:  dcb.Parity = ODDPARITY;  break;
        case PARITY_UART_EVEN: dcb.Parity = EVENPARITY; break;
        break;
    }

    status = SetCommState(fd, &dcb);
	if (!status) {
        errornum = GetLastError();
		printf("set DCB error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    return ERR_NONE;

#endif
}

/******************************************************************************/

u32 static halUartSetStopbits(SERIAL_ID fd, u8 stopBits)
{
#ifdef __CYGWIN

    struct termios options;

    tcgetattr(fd, &options);

    if(stopBits == 2) {
        options.c_cflag |= CSTOPB;
    } else if (stopBits == 1) {
        options.c_cflag &= ~CSTOPB;
    }

    tcsetattr(fd, TCSANOW, &options);
    tcflush(fd, TCIOFLUSH);

	return ERR_NONE;
#else
    DCB dcb;
    int status;
    int errornum;

    status = GetCommState(fd, &dcb);
    if (!status) {
        errornum = GetLastError();
		printf("get comm state error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    if(stopBits == 2) {
        dcb.StopBits = TWOSTOPBITS;
    } else if (stopBits == 1) {
        dcb.StopBits = ONESTOPBIT;
    }


    status = SetCommState(fd, &dcb);
	if (!status) {
        errornum = GetLastError();
		printf("set DCB error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    return ERR_NONE;

#endif
}

/******************************************************************************/

u32 static halUartSetBasic(SERIAL_ID fd)
{
#ifdef __CYGWIN
    struct termios options;
    /* get the current options  */
    tcgetattr(fd, &options);

    /* set raw input, 1 second timeout   */
    options.c_cflag |= (CLOCAL | CREAD );
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;

    options.c_iflag &= ~INPCK;	/* Enable parity checking */
    options.c_iflag |= IGNPAR;

    //options.c_cflag &= ~PARENB;	/* Clear parity enable */
    //options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;

    options.c_oflag &= ~(IXON | IXOFF | IXANY);	/* no flow control */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;	/* No output processing */
    options.c_iflag &= ~INLCR;	/* Don't convert linefeeds */
    options.c_iflag &= ~ICRNL;	/* Don't convert linefeeds */

     /* Miscellaneous stuff    */
    options.c_cflag |= (CLOCAL | CREAD);	/* Enable receiver, set local */

    options.c_cflag |= (IXON | IXOFF);	/* Software flow control */
    options.c_lflag = 0;	/* no local flags */
    options.c_cflag |= HUPCL;	/* Drop DTR on close */

    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // add

    /*  Clear the line */
    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &options) != 0) {
	    perror("setting serial tc");
    }
    tcflush(fd, TCIFLUSH);

    return ERR_NONE;
#else
    int status;
    DCB dcb;
    COMMTIMEOUTS timeout;
    int errornum;

    status = SetupComm(fd, 512, 512);
    if (!status) {
        errornum = GetLastError();
		printf("set buff error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    status = PurgeComm(fd, PURGE_TXCLEAR|PURGE_RXCLEAR);
    if (!status) {
        errornum = GetLastError();
		printf("purge error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    status = GetCommState(fd, &dcb);
    if (!status) {
        errornum = GetLastError();
		printf("get comm state error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    dcb.ByteSize = 8;
    //dcb.Parity = NOPARITY;
    dcb.fParity  = FALSE;
    dcb.fBinary           = TRUE;
    dcb.fOutxCtsFlow      = FALSE;
    dcb.fOutxDsrFlow      = FALSE;
    dcb.fDtrControl       = DTR_CONTROL_ENABLE;
    dcb.fRtsControl       = RTS_CONTROL_ENABLE;
    dcb.fDsrSensitivity   = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fOutX             = FALSE;
    dcb.fInX              = FALSE;
    dcb.fErrorChar        = FALSE;
    dcb.fNull             = FALSE;
    dcb.fAbortOnError     = FALSE;

    /*
    sprintf(compara,"com%d:%d,n,%d,%d", port, baudRate, 8, stopBits);
    if(!BuildCommDCB(compara, &dcb)){
		printf("Build Comm DCB error\n");
		return INVALID_HANDLE_VALUE;
	};
     */

    status = SetCommState(fd, &dcb);
	if (!status) {
        errornum = GetLastError();
		printf("set DCB error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    status = GetCommTimeouts(fd, &timeout);
    if (!status) {
        errornum = GetLastError();
		printf("get timer out error 1 = %d\n",errornum);
        return ERR_SERIAL_CONFIG_FAIL;
    }

    timeout.ReadIntervalTimeout= MAXDWORD;//MAXDWORD;
    timeout.ReadTotalTimeoutMultiplier=0;
    timeout.ReadTotalTimeoutConstant= 0;
	timeout.WriteTotalTimeoutMultiplier=0;//50;
	timeout.WriteTotalTimeoutConstant=0;//500;


	status = SetCommTimeouts(fd, &timeout);
    if (!status) {
        return ERR_SERIAL_CONFIG_FAIL;
    }

    return ERR_NONE;


#endif
}

/******************************************************************************/

SERIAL_ID halUartOpen(u8 port)
{
    char dev[50] = {0};
    SERIAL_ID fd;
#ifdef __CYGWIN

    /* open port */
    sprintf(dev, "/dev/ttyS%d", port-1);
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("cannot open \n");
    }
    return fd;

#else
    int status;
    int errornum;

    sprintf(dev,"\\\\.\\com%d", port);
    fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0 , NULL);
    if (fd == INVALID_HANDLE_VALUE) {
        errornum = GetLastError();
		printf("create error = %d\n",errornum);
        return INVALID_HANDLE_VALUE;
    }

    return fd;
#endif
}




