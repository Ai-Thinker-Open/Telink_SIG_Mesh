/********************************************************************************************************
 * @file     ed25519-donna.h
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
#pragma once

#include "ed25519-donna-portable.h"

#define ED25519_32BIT
#include "curve25519-donna-32bit.h"

#include "curve25519-donna-helpers.h"

/* separate uint128 check for 64 bit sse2 */
#include "modm-donna-32bit.h"

typedef unsigned char hash_512bits[64];

/*
	Timing safe memory compare
 */
static int
ed25519_verify(const unsigned char *x, const unsigned char *y, size_t len) {
    size_t differentbits = 0;
    while (len--)
        differentbits |= (*x++ ^ *y++);
    return (int) (1 & ((differentbits - 1) >> 8));
}


/*
 * Arithmetic on the twisted Edwards curve -x^2 + y^2 = 1 + dx^2y^2
 * with d = -(121665/121666) = 37095705934669439343138083508754565189542113879843219016388785533085940283555
 * Base point: (15112221349535400772501151409588531511454012693041857206046113283949847762202,46316835694926478169428394003475163141307993866256225615783033603165251855960);
 */

typedef struct ge25519_t {
    bignum25519 x, y, z, t;
} ge25519;

typedef struct ge25519_p1p1_t {
    bignum25519 x, y, z, t;
} ge25519_p1p1;

typedef struct ge25519_niels_t {
    bignum25519 ysubx, xaddy, t2d;
} ge25519_niels;

typedef struct ge25519_pniels_t {
    bignum25519 ysubx, xaddy, z, t2d;
} ge25519_pniels;

#include "ed25519-donna-basepoint-table.h"

#include "ed25519-donna-32bit-tables.h"

#include "ed25519-donna-impl-base.h"

 