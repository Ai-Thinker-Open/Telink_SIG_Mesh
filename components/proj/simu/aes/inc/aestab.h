/********************************************************************************************************
 * @file     aestab.h 
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

 This file contains the code for declaring the tables needed to implement
 AES. The file aesopt.h is assumed to be included before this header file.
 If there are no global variables, the definitions here can be used to put
 the AES tables in a structure so that a pointer can then be added to the
 AES context to pass them to the AES routines that need them.   If this
 facility is used, the calling program has to ensure that this pointer is
 managed appropriately.  In particular, the value of the t_dec(in,it) item
 in the table structure must be set to zero in order to ensure that the
 tables are initialised. In practice the three code sequences in aeskey.c
 that control the calls to aes_init() and the aes_init() routine itself will
 have to be changed for a specific implementation. If global variables are
 available it will generally be preferable to use them with the precomputed
 FIXED_TABLES option that uses static global tables.

 The following defines can be used to control the way the tables
 are defined, initialised and used in embedded environments that
 require special features for these purposes

    the 't_dec' construction is used to declare fixed table arrays
    the 't_set' construction is used to set fixed table values
    the 't_use' construction is used to access fixed table values

    256 byte tables:

        t_xxx(s,box)    => forward S box
        t_xxx(i,box)    => inverse S box

    256 32-bit word OR 4 x 256 32-bit word tables:

        t_xxx(f,n)      => forward normal round
        t_xxx(f,l)      => forward last round
        t_xxx(i,n)      => inverse normal round
        t_xxx(i,l)      => inverse last round
        t_xxx(l,s)      => key schedule table
        t_xxx(i,m)      => key schedule table

    Other variables and tables:

        t_xxx(r,c)      => the rcon table
*/

#if !defined( _AESTAB_H )
#define _AESTAB_H

#define t_dec(m,n) t_##m##n
#define t_set(m,n) t_##m##n
#define t_use(m,n) t_##m##n

#if defined(FIXED_TABLES)
#  if !defined( __GNUC__ ) && (defined( __MSDOS__ ) || defined( __WIN16__ ))
/*   make tables far data to avoid using too much DGROUP space (PG) */
#    define CONST const far
#  else
#    define CONST const
#  endif
#else
#  define CONST
#endif

#if defined(__cplusplus)
#  define EXTERN extern "C"
#elif defined(DO_TABLES)
#  define EXTERN
#else
#  define EXTERN extern
#endif

#if defined(_MSC_VER) && defined(TABLE_ALIGN)
#define ALIGN __declspec(align(TABLE_ALIGN))
#else
#define ALIGN
#endif

#if defined( __WATCOMC__ ) && ( __WATCOMC__ >= 1100 )
#  define XP_DIR __cdecl
#else
#  define XP_DIR
#endif

#if defined(DO_TABLES) && defined(FIXED_TABLES)
#define d_1(t,n,b,e)       EXTERN ALIGN CONST XP_DIR t n[256]    =   b(e)
#define d_4(t,n,b,e,f,g,h) EXTERN ALIGN CONST XP_DIR t n[4][256] = { b(e), b(f), b(g), b(h) }
EXTERN ALIGN CONST uint_32t t_dec(r,c)[RC_LENGTH] = rc_data(w0);
#else
#define d_1(t,n,b,e)       EXTERN ALIGN CONST XP_DIR t n[256]
#define d_4(t,n,b,e,f,g,h) EXTERN ALIGN CONST XP_DIR t n[4][256]
EXTERN ALIGN CONST uint_32t t_dec(r,c)[RC_LENGTH];
#endif

#if defined( SBX_SET )
    d_1(uint_8t, t_dec(s,box), sb_data, h0);
#endif
#if defined( ISB_SET )
    d_1(uint_8t, t_dec(i,box), isb_data, h0);
#endif

#if defined( FT1_SET )
    d_1(uint_32t, t_dec(f,n), sb_data, u0);
#endif
#if defined( FT4_SET )
    d_4(uint_32t, t_dec(f,n), sb_data, u0, u1, u2, u3);
#endif

#if defined( FL1_SET )
    d_1(uint_32t, t_dec(f,l), sb_data, w0);
#endif
#if defined( FL4_SET )
    d_4(uint_32t, t_dec(f,l), sb_data, w0, w1, w2, w3);
#endif

#if defined( IT1_SET )
    d_1(uint_32t, t_dec(i,n), isb_data, v0);
#endif
#if defined( IT4_SET )
    d_4(uint_32t, t_dec(i,n), isb_data, v0, v1, v2, v3);
#endif

#if defined( IL1_SET )
    d_1(uint_32t, t_dec(i,l), isb_data, w0);
#endif
#if defined( IL4_SET )
    d_4(uint_32t, t_dec(i,l), isb_data, w0, w1, w2, w3);
#endif

#if defined( LS1_SET )
#if defined( FL1_SET )
#undef  LS1_SET
#else
    d_1(uint_32t, t_dec(l,s), sb_data, w0);
#endif
#endif

#if defined( LS4_SET )
#if defined( FL4_SET )
#undef  LS4_SET
#else
    d_4(uint_32t, t_dec(l,s), sb_data, w0, w1, w2, w3);
#endif
#endif

#if defined( IM1_SET )
    d_1(uint_32t, t_dec(i,m), mm_data, v0);
#endif
#if defined( IM4_SET )
    d_4(uint_32t, t_dec(i,m), mm_data, v0, v1, v2, v3);
#endif

#endif
