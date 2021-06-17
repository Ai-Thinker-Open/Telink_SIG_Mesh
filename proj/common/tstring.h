/********************************************************************************************************
 * @file     tstring.h 
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

#pragma once
#ifndef WIN32 
void *  memmove(void * dest, const void * src, unsigned int n);
void *  memset(void * d, int c, unsigned int  n);
void *  memcpy(void * des_ptr, const void * src_ptr, unsigned int);

// do not return void*,  otherwise, we must use a variable to store the dest porinter, that is not performance 

void *	memchr(const void *_s, int _c, unsigned int _n);
int		memcmp(const void *_s1, const void *_s2, unsigned int _n);

char *	strcat(char *_s1, const char *_s2);
char *	strchr(const char *_s, int _c);
int		strcmp(const char *_s1, const char *_s2);
int		strcoll(const char *_s1, const char *_s2);
char *	strcpy(char *_s1, const char *_s2);
unsigned int	strcspn(const char *_s1, const char *_s2);
char *	strerror(int _errcode);
unsigned int	strlen(const char *_s);
char *	strncat(char *_s1, const char *_s2, unsigned int _n);
int		strncmp(const char *_s1, const char *_s2, unsigned int _n);
char *	strncpy(char *_s1, const char *_s2, unsigned int _n);
char *	strpbrk(const char *_s1, const char *_s2);
char *	strrchr(const char *_s, int _c);
unsigned int	strspn(const char *_s1, const char *_s2);
char *	strstr(const char *s1, const char *s2);
char *	strtok(char *_s1, const char *_s2);
unsigned int	strxfrm(char *_s1, const char *_s2, unsigned int _n);
char * 	strchr (register const char *s, int c);
void * 	memchr (register const void * src_void, int c, unsigned int length);
#endif 

void   	memcpy4(void * dest, const void * src, unsigned int);
int is_buf_zero(void *data, unsigned int len);
int is_buf_all_one(void *data, unsigned int len);
int ismemzero4(void *data, unsigned int len);
int ismemf4(void *data, unsigned int len);
void * memset4(void * dest, int val, unsigned int len);
void zeromem4(void *data, unsigned int len);


