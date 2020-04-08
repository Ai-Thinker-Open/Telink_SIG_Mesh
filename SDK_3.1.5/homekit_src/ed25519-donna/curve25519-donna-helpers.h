/********************************************************************************************************
 * @file     curve25519-donna-helpers.h
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


/*
 * In:  b =   2^5 - 2^0
 * Out: b = 2^250 - 2^0
 */

#pragma once

static void
curve25519_pow_two5mtwo0_two250mtwo0(bignum25519 b) {
	bignum25519 ALIGN(16) t0,c;

	/* 2^5  - 2^0 */ /* b */
	/* 2^10 - 2^5 */ curve25519_square_times(t0, b, 5);
	/* 2^10 - 2^0 */ curve25519_mul_noinline(b, t0, b);
	/* 2^20 - 2^10 */ curve25519_square_times(t0, b, 10);
	/* 2^20 - 2^0 */ curve25519_mul_noinline(c, t0, b);
	/* 2^40 - 2^20 */ curve25519_square_times(t0, c, 20);
	/* 2^40 - 2^0 */ curve25519_mul_noinline(t0, t0, c);
	/* 2^50 - 2^10 */ curve25519_square_times(t0, t0, 10);
	/* 2^50 - 2^0 */ curve25519_mul_noinline(b, t0, b);
	/* 2^100 - 2^50 */ curve25519_square_times(t0, b, 50);
	/* 2^100 - 2^0 */ curve25519_mul_noinline(c, t0, b);
	/* 2^200 - 2^100 */ curve25519_square_times(t0, c, 100);
	/* 2^200 - 2^0 */ curve25519_mul_noinline(t0, t0, c);
	/* 2^250 - 2^50 */ curve25519_square_times(t0, t0, 50);
	/* 2^250 - 2^0 */ curve25519_mul_noinline(b, t0, b);
}

/*
 * z^(p - 2) = z(2^255 - 21)
 */
static void
curve25519_recip(bignum25519 out, const bignum25519 z) {
	bignum25519 ALIGN(16) a,t0,b;

	/* 2 */ curve25519_square_times(a, z, 1); /* a = 2 */
	/* 8 */ curve25519_square_times(t0, a, 2);
	/* 9 */ curve25519_mul_noinline(b, t0, z); /* b = 9 */
	/* 11 */ curve25519_mul_noinline(a, b, a); /* a = 11 */
	/* 22 */ curve25519_square_times(t0, a, 1);
	/* 2^5 - 2^0 = 31 */ curve25519_mul_noinline(b, t0, b);
	/* 2^250 - 2^0 */ curve25519_pow_two5mtwo0_two250mtwo0(b);
	/* 2^255 - 2^5 */ curve25519_square_times(b, b, 5);
	/* 2^255 - 21 */ curve25519_mul_noinline(out, b, a);
}

/*
 * z^((p-5)/8) = z^(2^252 - 3)
 */
static void
curve25519_pow_two252m3(bignum25519 two252m3, const bignum25519 z) {
	bignum25519 ALIGN(16) b,c,t0;

	/* 2 */ curve25519_square_times(c, z, 1); /* c = 2 */
	/* 8 */ curve25519_square_times(t0, c, 2); /* t0 = 8 */
	/* 9 */ curve25519_mul_noinline(b, t0, z); /* b = 9 */
	/* 11 */ curve25519_mul_noinline(c, b, c); /* c = 11 */
	/* 22 */ curve25519_square_times(t0, c, 1);
	/* 2^5 - 2^0 = 31 */ curve25519_mul_noinline(b, t0, b);
	/* 2^250 - 2^0 */ curve25519_pow_two5mtwo0_two250mtwo0(b);
	/* 2^252 - 2^2 */ curve25519_square_times(b, b, 2);
	/* 2^252 - 3 */ curve25519_mul_noinline(two252m3, b, z);
}
