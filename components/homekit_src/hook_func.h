/********************************************************************************************************
 * @file     hook_func.h
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
#ifndef _HOOK_FUNC_H_
#define _HOOK_FUNC_H_


/* ------------------------------------
    Header Files
   ------------------------------------ */


/* ------------------------------------
    Macro Definitions
   ------------------------------------ */
#undef EXTERN

#ifdef _HOOK_FUNC_C_
    #define EXTERN
#else
    #define EXTERN  extern
#endif


/* ------------------------------------
    Type Definitions
   ------------------------------------ */
typedef void (*hook_callback_func_t) (unsigned HookPointNum, unsigned char * parameter);
enum {
    FUNC_NULL = 0,
    Begin1stBootInitKeyPhase = 1,   // Begin1stBootInitKeyPhase
    Finish1stBootInitKeyPhase = 2,  // Finish1stBootInitKeyPhase
    ReceivePairRequestPhase = 3,    // ReceivePairRequestPhase
    WaitSetupCodeInPhase = 4,       // WaitSetupCodeInPhase
    FinishSetupCodeInOrTripleTryPhase = 5,  // FinishSetupCodeInOrTripleTryPhase
    Register1stAdminKeyPhase = 6,   // Register1stAdminKeyPhase
    Phase7 = 7,                     // PHASE7
    Phase8 = 8,                     // PHASE8
    DoRemovePair1Phase = 9,         // DoRemovePair1Phase
    DoRemoveAllPairPhase = 10,      // DoRemoveAllPairPhase
    CUST_HOOK_FUNC1 = 11,
    CUST_HOOK_FUNC2,
    CUST_HOOK_FUNC3,
    CUST_HOOK_FUNC4,
    CUST_HOOK_FUNC5,
    FUNC_MAX,
};

/* ------------------------------------
    Variables Definitions/Declarations
   ------------------------------------ */


/* ------------------------------------
    Function Prototypes
   ------------------------------------ */
EXTERN void hook_callback_func_init (unsigned char HookPointNum, unsigned char * parameter);
EXTERN void hook_callback_func_execute (unsigned char HookPointNum, unsigned char * parameter);
EXTERN void hook_callback_func_register (unsigned char HookPointNum, hook_callback_func_t pFunc);


#endif /* _HOOK_FUNC_H_ */

/* %% End Of File %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
