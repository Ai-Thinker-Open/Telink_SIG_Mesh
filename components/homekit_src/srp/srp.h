/********************************************************************************************************
 * @file     srp.h
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

#ifndef _SRP_H_
#define _SRP_H_

#include "cstr.h"
#include "srp_aux.h"
#include "t_sha.h"
#include "../gmp/mini-gmp/mini-gmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SRP library version identification */
#define SRP_VERSION_MAJOR 2
#define SRP_VERSION_MINOR 0
#define SRP_VERSION_PATCHLEVEL 1

typedef int SRP_RESULT;
/* Returned codes for SRP API functions */
#define SRP_OK(v) ((v) == SRP_SUCCESS)
#define SRP_SUCCESS 0
#define SRP_ERROR -1

/* Set the minimum number of bits acceptable in an SRP modulus */
#define SRP_DEFAULT_MIN_BITS 512

/* Magic numbers for the SRP context header */
#define SRP_MAGIC_CLIENT 12
#define SRP_MAGIC_SERVER 28

/* Flag bits for SRP struct */
#define SRP_FLAG_MOD_ACCEL 0x1	/* accelerate modexp operations */
#define SRP_FLAG_LEFT_PAD 0x2	/* left-pad to length-of-N inside hashes */

#define RFC2945_KEY_LEN 	SHA_DIGESTSIZE	/* length of session key (bytes) */
#define RFC2945_RESP_LEN 	SHA_DIGESTSIZE	/* length of proof hashes (bytes) */

typedef struct
{
  const char *	username;
  unsigned char salt[16];

  __mpz_struct modulus;
  __mpz_struct generator;
  

  __mpz_struct verifier;
  __mpz_struct password;

  __mpz_struct pubkey;
  __mpz_struct secret;
  __mpz_struct u;

  __mpz_struct key;

  SHACTX hash;						//not used
  SHACTX ckhash;
  SHACTX oldhash;
  SHACTX oldckhash;
  unsigned char k[64];
  unsigned char r[64];
  
  int			num;			//mysrp_buff_offset;
  mp_limb_t		buff[256];		//mysrp_buffer;
} mysrp_st;

#define			SRP_INFO_FLAG				0x9192
#define			SRP_ADR_PASSWORD			4
#define			SRP_ADR_SALT				16
#define			SRP_ADR_PASS				32
#define			SRP_ADR_VERIFIER			96
#define			SRP_ADR_PUBKEY				(SRP_ADR_VERIFIER + 384)

void mysrp_init (mysrp_st * srp, u32 flash_adr_srp);

void mysrp_set_username (mysrp_st * srp, const char *p);
SRP_RESULT mysrp_set_params (mysrp_st * srp, unsigned char * salt);
SRP_RESULT mysrp_set_auth_password (mysrp_st * srp, char * pass);
SRP_RESULT mysrp_gen_pub (mysrp_st * srp, unsigned char * result);
SRP_RESULT mysrp_compute_key(mysrp_st * srp, unsigned char * result, const unsigned char * pubkey, int pubkeylen);
SRP_RESULT mysrp_verify(mysrp_st * srp, const unsigned char * proof, int prooflen);
SRP_RESULT mysrp_respond(mysrp_st * srp, unsigned char * proof);

#ifdef __cplusplus
}
#endif

#endif /* _SRP_H_ */
