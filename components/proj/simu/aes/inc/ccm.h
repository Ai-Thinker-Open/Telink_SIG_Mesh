/********************************************************************************************************
 * @file     ccm.h 
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
/*
 ---------------------------------------------------------------------------
Relevant documents are:

    NIST Special Publication 800-38C: DRAFT Recommendation for Block Cipher
    Modes of Operation: The CCM Mode For AUthentication and Confidentiality.
    September 2003.

    IEEE Std 802.11i/D5.0, August 2003.   Draft Amendment to standard for
    Telecommunications and Information Exchange Between Systems - LAN/MAN
    Specific Requirements - Part 11: Wireless Medium Access Control (MAC)
    and physical layer (PHY) specifications:  Medium Access Control (MAC)
    Security Enhancements

 The length of the mesaage data must be less than 2^32 bytes unless the
 define LONG_MESSAGES is set.  NOTE that this implementation is not fully
 compliant with the CCM specification because, if an authentication error
 is detected when the last block is processed, blocks processed earlier will
 already have been returned to the caller. This violates the specification
 but is costly to avoid for large messages that cannot be memory resident as
 a single block. In this case the message would have to be processed twice
 so that the final authentication value can be checked before the output is
 provided on a second pass.
*/

#ifndef _CCM_H
#define _CCM_H

#if 1
#  define LONG_MESSAGES
#endif

/*  This define sets the memory alignment that will be used for fast move
    and xor operations on buffers when the alignment matches this value.
*/
#if !defined( UNIT_BITS )
#  if 1
#    define UNIT_BITS 64
#  elif 0
#    define UNIT_BITS 32
#  else
#    define UNIT_BITS  8
#  endif
#endif

#if ( UNIT_BITS == 64 || defined( LONG_MESSAGES ) ) && !defined( NEED_UINT_64T )
#  define NEED_UINT_64T
#endif

#include "aes.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/*  After encryption or decryption operations the return value of
    'compute tag' will be one of the values RETURN_GOOD, RETURN_WARN
    or RETURN_ERROR, the latter indicating an error. A return value
    RETURN_GOOD indicates that both encryption and authentication
    have taken place and resulted in the returned tag value. If
    the returned value is RETURN_WARN, the tag value is the result
    of authentication alone without encryption (CCM) or decryption
    (GCM and EAX).
*/
#ifndef RETURN_GOOD
# define RETURN_WARN      1
# define RETURN_GOOD      0
# define RETURN_ERROR    -1
#endif

#define CCM_BAD_KEY               -2
#define CCM_BAD_AUTH_FIELD_LENGTH -3
#define CCM_BAD_NONCE_LENGTH      -4
#define CCM_BAD_AUTH_CALL         -5
#define CCM_AUTH_LENGTH_ERROR     -6
#define CCM_MSG_LENGTH_ERROR      -7

//typedef int ret_type;
UNIT_TYPEDEF(ccm_unit_t, UNIT_BITS);
BUFR_TYPEDEF(ccm_buf_t, UNIT_BITS, AES_BLOCK_SIZE);

#define CCM_BLOCK_SIZE  AES_BLOCK_SIZE

/* The CCM context  */

/*
#if defined( LONG_MESSAGES )
  typedef uint_64t length_t;
#else
  typedef u32 length_t;
#endif
*/

typedef struct
{   ccm_buf_t    ctr_val;                        /* counter block                */
    ccm_buf_t    enc_ctr;                        /* encrypted counter block      */
    ccm_buf_t    cbc_buf;                        /* running CBC value            */
    aes_encrypt_ctx aes[1];                     /* AES context                  */
    length_t    hdr_len;                        /* the associated data length   */
    length_t    msg_len;                        /* message data length          */
    length_t    hdr_lim;                        /* message auth length (bytes)  */
    length_t    cnt;                            /* position counter             */
    length_t    txt_acnt;                       /* position counter             */
    u32    iv_len;                         /* the nonce length             */
    u32    tag_len;                        /* authentication field length  */
    u32    state;                          /* algorithm state/error value  */
} ccm_ctx;

/* The following calls handle mode initialisation, keying and completion        */

u32 ccm_init_and_key(                      /* initialise mode and set key  */
            const unsigned char key[],          /* the key value                */
            unsigned long key_len,              /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_end(                               /* clean up and end operation   */
            ccm_ctx ctx[1]);                    /* the mode context             */

/* The following calls handle complete messages in memory in a single operation */

u32 ccm_encrypt_message(                   /* encrypt an entire message    */
            const unsigned char iv[],           /* the initialisation vector    */
            unsigned long iv_len,               /* and its length in bytes      */
            const unsigned char hdr[],          /* the header buffer            */
            length_t hdr_len,                   /* and its length in bytes      */
            unsigned char msg[],                /* the message buffer           */
            length_t msg_len,                   /* and its length in bytes      */
            unsigned char tag[],                /* the buffer for the tag       */
            unsigned long tag_len,              /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

                                    /* RETURN_GOOD is returned if the input tag */
                                    /* matches that for the decrypted message   */
u32 ccm_decrypt_message(                   /* decrypt an entire message    */
            const unsigned char iv[],           /* the initialisation vector    */
            unsigned long iv_len,               /* and its length in bytes      */
            const unsigned char hdr[],          /* the header buffer            */
            length_t hdr_len,                   /* and its length in bytes      */
            unsigned char msg[],                /* the message buffer           */
            length_t msg_len,                   /* and its length in bytes      */
            const unsigned char tag[],          /* the buffer for the tag       */
            unsigned long tag_len,              /* and its length in bytes      */
            unsigned char ctag[],          /* the buffer for the tag       */
            ccm_ctx ctx[1]);                    /* the mode context             */

/* The following calls handle messages in a sequence of operations followed by  */
/* tag computation after the sequence has been completed. In these calls the    */
/* user is responsible for verfiying the computed tag on decryption             */

u32 ccm_init_message(                      /* initialise a new message     */
            const unsigned char iv[],           /* the initialisation vector    */
            unsigned long iv_len,               /* the nonce length             */
            length_t hdr_len,                   /* the associated data length   */
            length_t msg_len,                   /* message data length          */
            unsigned long tag_len,              /* authentication field length  */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_auth_header(                       /* authenticate message header  */
            const unsigned char hdr[],          /* the header buffer            */
            length_t hdr_len,              /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_encrypt(                           /* encrypt & authenticate data  */
            unsigned char data[],               /* the data buffer              */
            unsigned long data_len,             /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_decrypt(                           /* authenticate & decrypt data  */
            unsigned char data[],               /* the data buffer              */
            unsigned long data_len,             /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_compute_tag(                       /* compute authentication tag   */
            unsigned char tag[],                /* the buffer for the tag       */
            unsigned long tag_len,              /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

/*  The use of the following calls should be avoided if possible because their
    use requires a very good understanding of the way this encryption mode
    works and the way in which this code implements it in order to use them
    correctly.

    The ccm_auth_data routine is used to authenticate the data to be encrypted
    or to authenticate the result of decryption.  In message encryption
    ccm_auth_data must be called before any encryption operations are performed
    on the data using ccm_crypt_data. In message decryption data must be
    decrypted before it is authenticatied so the call order is reversed.

    If these calls are used it is up to the user to ensure that these routines
    are called in the correct order and that the correct data is passed to them.

    When ccm_compute_tag is called it is assumed that an error in use has
    occurred if both encryption (or decryption) and authentication have taken
    place but the total lengths of the message data respectively authenticated
    and encrypted are not the same. If authentication has taken place but there
    has been no corresponding encryption or decryption operations (none at all)
    only a warning is issued since this might (just) possibly be intentional.
*/

u32 ccm_auth_data(                         /* authenticate plaintext data  */
            const unsigned char data[],         /* the data buffer              */
            unsigned long data_len,             /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

u32 ccm_crypt_data(                        /* encrypt or decrypt data      */
            unsigned char data[],               /* the data buffer              */
            unsigned long data_len,             /* and its length in bytes      */
            ccm_ctx ctx[1]);                    /* the mode context             */

#if defined(__cplusplus)
}
#endif

#endif
