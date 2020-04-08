#pragma once
#include "../user_config.h"
#include "nl_model_level.h"
#include "nl_model_schedules.h"

#if NL_API_ENABLE

#define CB_NL_PAR_NUM_1(cb, a)	do{if(cb){cb(a);}}while(0)
#define CB_NL_PAR_NUM_2(cb, a, b)	do{if(cb){cb(a, b);}}while(0)
#define CB_NL_PAR_NUM_3(cb, a, b, c)	do{if(cb){cb(a, b, c);}}while(0)

void register_nl_callback();

#else

#define CB_NL_PAR_NUM_1(cb, a)
#define CB_NL_PAR_NUM_2(cb, a, b)	
#define CB_NL_PAR_NUM_3(cb, a, b, c)	

#define register_nl_callback() 
#endif
