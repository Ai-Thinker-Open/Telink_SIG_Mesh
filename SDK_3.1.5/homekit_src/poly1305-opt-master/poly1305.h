/********************************************************************************************************
 * @file     poly1305.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
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



#ifdef WIN32
#include <stddef.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif
    
    typedef struct poly1305_context {
        size_t aligner;
        unsigned char opaque[136];
    } poly1305_context;
    
    typedef struct poly1305_context poly1305_state;
    
    void poly1305_init(poly1305_context *ctx, const unsigned char key[32]);
    void poly1305_update(poly1305_context *ctx, const unsigned char *in,
                                size_t len);
    void poly1305_finish(poly1305_context *ctx, unsigned char mac[16]);
    
#ifdef  __cplusplus
}
#endif

