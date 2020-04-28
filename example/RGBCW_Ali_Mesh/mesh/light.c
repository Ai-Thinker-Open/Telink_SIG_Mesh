/********************************************************************************************************
 * @file     light.c 
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
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "mesh/lighting_model.h"
#include "mesh/lighting_model_HSL.h"
#include "mesh/lighting_model_xyL.h"
#include "mesh/lighting_model_LC.h"
#include "mesh/generic_model.h"
#include "mesh/scene.h"
#include "light.h"
#if HOMEKIT_EN
#include "mesh/led_cfg.h"
#endif
#if WIN32
#include <stdlib.h>
#else
#include "proj/mcu/watchdog_i.h"
#include "proj_lib/pm.h"
#endif


/** @addtogroup Mesh_Common
  * @{
  */
  
/** @defgroup Light
  * @brief Light Code.
  * @{
  */


typedef struct{
    u32 gpio;
    u8 id;
    u8 invert;
    u8 func;
    u8 rsv[1];
}light_res_hw_t;

#if ((LIGHT_TYPE_SEL == LIGHT_TYPE_CT) || (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL))
u8 ct_flag = 1; // default CT
#else   // (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL)
u8 ct_flag = 0; // always HSL
#endif

#if (IS_VC_PROJECT)
#define RES_HW_PWM_R    {0, 2, 0, 0}
#define RES_HW_PWM_G    {0, 1, 0, 0}
#define RES_HW_PWM_B    {0, 3, 0, 0}
#define RES_HW_PWM_W    {0, 4, 0, 0}
#else
#define RES_HW_PWM_R    {PWM_R, PWMID_R, PWM_INV_R, PWM_FUNC_R}
#define RES_HW_PWM_G    {PWM_G, PWMID_G, PWM_INV_G, PWM_FUNC_G}
#define RES_HW_PWM_B    {PWM_B, PWMID_B, PWM_INV_B, PWM_FUNC_B}
#define RES_HW_PWM_C    {PWM_C, PWMID_C, PWM_INV_C, PWM_FUNC_C}
#define RES_HW_PWM_W    {PWM_W, PWMID_W, PWM_INV_W, PWM_FUNC_W}
#endif

#if (LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)
const light_res_hw_t light_res_hw[LIGHT_CNT][5] = {
	/*[0] = */{RES_HW_PWM_R, RES_HW_PWM_G, RES_HW_PWM_B, RES_HW_PWM_C, RES_HW_PWM_W}, // vc can't use "[0]="
};
#elif (LIGHT_TYPE_SEL == LIGHT_TYPE_HSL || LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
const light_res_hw_t light_res_hw[LIGHT_CNT][3] = {
	/*[0] = */{RES_HW_PWM_R, RES_HW_PWM_G, RES_HW_PWM_B},
};
#elif (LIGHT_TYPE_SEL == LIGHT_TYPE_CT)
	#if XIAOMI_MODULE_ENABLE
const light_res_hw_t light_res_hw[LIGHT_CNT][3] = {
	/*[0] = */{RES_HW_PWM_R, RES_HW_PWM_G, RES_HW_PWM_W},
};
	#else
const light_res_hw_t light_res_hw[LIGHT_CNT][2] = {
	/*[0] = */{RES_HW_PWM_R, RES_HW_PWM_G},
};
	#endif
#else
const light_res_hw_t light_res_hw[LIGHT_CNT][1] = {
	[0] = {RES_HW_PWM_R},
    #if (LIGHT_CNT >= 2)
    [1] = {RES_HW_PWM_G},
    #endif
    #if (LIGHT_CNT >= 3)
    [2] = {RES_HW_PWM_B},
    #endif
    #if (LIGHT_CNT >= 4)
    [3] = {RES_HW_PWM_W},
    #endif
};
#endif

const u32 GPIO_LED_INDEX = (GPIO_LED == PWM_R) ? 0 : ((GPIO_LED == PWM_G) ? 1 : ((GPIO_LED == PWM_B) ? 2 : ((GPIO_LED == PWM_W) ? 3 : 0)));

#define LIGHT_ADJUST_INTERVAL       (20)   // unit :ms;     min:20ms; max 100ms

light_res_sw_save_t light_res_sw_save[LIGHT_CNT] = {{{{0}}}};
light_res_sw_trans_t light_res_sw[LIGHT_CNT] ;

#define P_SW_LEVEL_SAVE(idx, type)	(&light_res_sw_save[idx].level[type])
#define P_ST_TRANS(idx, type)		(&light_res_sw[idx].trans[type])

u8 light_pub_trans_step = ST_PUB_TRANS_IDLE;    // 0
u8 *light_pub_model_priority = 0;


STATIC_ASSERT(PWM_MAX_TICK < 0x10000);  // 16 bit for PWM register. if use smaller frequence, please set reg_pwm_clk.
STATIC_ASSERT(LIGHTNESS_DEFAULT != 0);	// if want to set 0, please set ONOFF_DEFAULT to 0,
STATIC_ASSERT(LIGHTNESS_MIN != 0);
//STATIC_ASSERT(sizeof(light_res_sw_t) % 4 == 0); // for align
STATIC_ASSERT(LIGHT_ADJUST_INTERVAL <= 100);

// 0-100%  (pwm's value index: this is pwm compare value, and the pwm cycle is 255*256)
const u16 rgb_lumen_map[101] = {
  0,2*256+128,2*256+192,3*256,3*256+128,4*256,4*256+128,5*256,5*256+128,6*256,  7*256,
      8*256,  9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256, 16*256, 17*256,
     18*256, 19*256, 21*256, 23*256, 25*256, 27*256, 29*256, 31*256, 33*256, 35*256,
     37*256, 39*256, 41*256, 43*256, 45*256, 47*256, 49*256, 51*256, 53*256, 55*256,
     57*256, 59*256, 61*256, 63*256, 65*256, 67*256, 69*256, 71*256, 73*256, 75*256,
     78*256, 81*256, 84*256, 87*256, 90*256, 93*256, 96*256, 99*256,102*256,105*256, 
    108*256,111*256,114*256,117*256,120*256,123*256,126*256,129*256,132*256,135*256,
    138*256,141*256,144*256,147*256,150*256,154*256,158*256,162*256,166*256,170*256,
    174*256,178*256,182*256,186*256,190*256,194*256,198*256,202*256,206*256,210*256,
    214*256,218*256,222*256,226*256,230*256,235*256,240*256,245*256,250*256,255*256,// must less or equal than (255*256)
};

#if (LIGHT_TYPE_HSL_EN)
typedef struct{
    float h;
    float s;
    float l;
}HSL_set;

typedef struct{
    u8 r;
    u8 g;
    u8 b;
}RGB_set;

float Hue_2_RGB(float v1,float v2,float vH){
    if(vH < 0){
        vH+=1;
    }
    if(vH > 1){
        vH-=1;
    }
    if((6.0*vH)<1){
        return (v1+(v2-v1)*6.0f*vH);
    }
    if((2.0*vH)<1){
        return (v2);
    }
    if((3.0*vH)<2){
        return (v1+(v2-v1)*((2.0f/3.0f)-vH)*6.0f);
    } 
    return(v1);
}

void HslToRgb(HSL_set hsl, RGB_set *rgb)
{
	float m1,m2;
	if(hsl.s==0){	
	    rgb->r=rgb->g=rgb->b=(u8)(hsl.l*255);
	}
	else
	{
	    if(hsl.l<0.5){
		    m2=hsl.l*(1+hsl.s);
	    }
	    else{
		    m2=hsl.l+hsl.s-hsl.l*hsl.s;   	
	    }
	    m1=2*hsl.l-m2;
	    rgb->r=(u8)(255*Hue_2_RGB(m1,m2,hsl.h+(1.0f/3.0f)));
	    rgb->g=(u8)(255*Hue_2_RGB(m1,m2,hsl.h));
	    rgb->b=(u8)(255*Hue_2_RGB(m1,m2,hsl.h-(1.0f/3.0f)));
	}	
}
#endif 

u8 mode_sw_data[4] ={ 0 }; //切换冷暖与颜色时的中间数据

void set_ct_mode(u8 mode)
{
	st_transition_t *p_trans = P_ST_TRANS(0, ST_TRANS_LIGHTNESS);

    if(ct_flag != mode){

		LOG_MSG_INFO(TL_LOG_MESH,0,0,"switch mode %d .....................", ct_flag);

		if(ct_flag == 1) //当前为 CT Mode
		{
			u8 lum_100 = level2lum(p_trans->present); //获取亮度
			u8 ct_100 = 0;

			if(ct_flag && (lum_100 != 0))
			{
				u16 temp = light_ctl_temp_prensent_get(0);
				ct_100 = temp_to_temp100_hw(temp);
			}
			mode_sw_data[0] = 1;
			mode_sw_data[1] = (100-ct_100)*lum_100/100;
			mode_sw_data[2] = ct_100*lum_100/100;
		}
		else
		{
			st_transition_t *p_hue = P_ST_TRANS(0, ST_TRANS_HSL_HUE);
			st_transition_t *p_sat = P_ST_TRANS(0, ST_TRANS_HSL_SAT);
			HSL_set HSL;
			RGB_set RGB;
			HSL.h = ((float)(s16_to_u16(p_hue->present))/65535.0f);
			HSL.s = ((float)(s16_to_u16(p_sat->present))/65535.0f);
			HSL.l = ((float)(s16_to_u16(p_trans->present))/65535.0f);
			HslToRgb(HSL,&RGB);
			mode_sw_data[0] = 2;
			mode_sw_data[1] = RGB.r*100/255;
			mode_sw_data[2] = RGB.g*100/255;
			mode_sw_data[3] = RGB.b*100/255;
		}

		p_trans ->present = -32768;

	    ct_flag = mode;
	    mesh_misc_store();
	}
}

#if MD_SERVER_EN

/**
 * @brief  Save Generic OnOff States for next power-on.
 * @param  idx: Element index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @param  onoff: OnOff States.
 *     @arg G_OFF: Off state.
 *     @arg G_ON: On state.
 * @retval None
 */
void set_on_power_up_onoff(int idx, int st_trans_type, u8 onoff)
{
	sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
	p_save->onoff = onoff;
	light_par_save(0);
}

void set_on_power_up_last(sw_level_save_t *p_save, s16 last)
{
	if(LEVEL_OFF == last){
		p_save->onoff = 0;  // active for all level. include CT, Hue, Sat...
	}else{
		p_save->onoff = 1;
		p_save->last = last;
	}
}

s16 get_on_power_up_last(sw_level_save_t *p_save)
{
	return (p_save->onoff ? p_save->last : LEVEL_OFF);
}

void mesh_global_var_init_light_sw()
{
	foreach_arr(i,light_res_sw){
		foreach_arr(k,light_res_sw[i].trans){
			sw_level_save_t *p_save = &light_res_sw_save[i].level[k];
			st_transition_t *p_trans = &light_res_sw[i].trans[k];
			s16 val_init = 0;
			if(ST_TRANS_LIGHTNESS == k){
				val_init = u16_to_s16(LIGHTNESS_DEFAULT);
				p_save->min = u16_to_s16(LIGHTNESS_MIN);
				p_save->max = u16_to_s16(LIGHTNESS_MAX);
			    p_save->def = u16_to_s16(0);	// use last, spec page172 requested.
			#if (LIGHT_TYPE_CT_EN)
			}else if(ST_TRANS_CTL_TEMP == k){
				p_save->min = u16_to_s16(CTL_TEMP_MIN);   // save as s16 temp value, not related to range
				p_save->max = u16_to_s16(CTL_TEMP_MAX);   // save as s16 temp value, not related to range
				val_init = get_level_from_ctl_temp(CTL_TEMP_DEFAULT, CTL_TEMP_MIN, CTL_TEMP_MAX);
			    p_save->def = u16_to_s16(CTL_TEMP_DEFAULT);
			}else if(ST_TRANS_CTL_D_UV == k){
				val_init = CTL_D_UV_DEFAULT;
				p_save->min = CTL_D_UV_MIN;
				p_save->max = CTL_D_UV_MAX;
			    p_save->def = val_init;
			#endif
			#if (LIGHT_TYPE_HSL_EN)
			}else if(ST_TRANS_HSL_HUE == k){
				val_init = u16_to_s16(HSL_HUE_DEFAULT);
				p_save->min = u16_to_s16(HSL_HUE_MIN);
				p_save->max = u16_to_s16(HSL_HUE_MAX);
			    p_save->def = val_init;
			}else if(ST_TRANS_HSL_SAT == k){
				val_init = u16_to_s16(HSL_SAT_DEFAULT);
				p_save->min = u16_to_s16(HSL_SAT_MIN);
				p_save->max = u16_to_s16(HSL_SAT_MAX);
			    p_save->def = val_init;
			#endif
			#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
			}else if(ST_TRANS_XYL_X == k){
				val_init = u16_to_s16(XYL_X_DEFAULT);
				p_save->min = u16_to_s16(XYL_X_MIN);
				p_save->max = u16_to_s16(XYL_X_MAX);
			    p_save->def = val_init;
			}else if(ST_TRANS_XYL_Y == k){
				val_init = u16_to_s16(XYL_Y_DEFAULT);
				p_save->min = u16_to_s16(XYL_Y_MIN);
				p_save->max = u16_to_s16(XYL_Y_MAX);
			    p_save->def = val_init;
			#endif
			}
			
            p_save->last = val_init;    // must init "last", even though it's -32768.
			p_save->onoff = (ST_TRANS_LIGHTNESS == k) ? ONOFF_DEFAULT : 1;
            
			p_trans->present = p_trans->target = get_on_power_up_last(p_save);
		}

        #if FEATURE_LOWPOWER_EN
        ONPOWER_UP_VAL(i) = ONPOWER_UP_OFF;
        #else
		ONPOWER_UP_VAL(i) = ONPOWER_UP_DEFAULT; // ONPOWER_UP_STORE; // 
		#endif
		g_def_trans_time_val(i) = PTS_TEST_EN ? 0 : TRANSITION_TIME_DEFAULT_VAL;
	}
}

void light_res_sw_load()
{
	foreach_arr(i,light_res_sw){
		foreach_arr(k,light_res_sw[i].trans){
			sw_level_save_t *p_save = &light_res_sw_save[i].level[k];
			st_transition_t *p_trans = &light_res_sw[i].trans[k];
			s16 level_poweron = 0;
			#if(WIN32)
			if(ONPOWER_UP_STORE == ONPOWER_UP_VAL(i)){
			#else
			if((ONPOWER_UP_STORE == ONPOWER_UP_VAL(i)) || ((ST_TRANS_LIGHTNESS == k) && (analog_read(DEEP_ANA_REG0)&BIT(OTA_REBOOT_FLAG)))){
			#endif
				level_poweron = get_on_power_up_last(p_save);
			}
			else if((ONPOWER_UP_OFF == ONPOWER_UP_VAL(i))&&(ST_TRANS_LIGHTNESS == k)){
			    // ONPOWER_UP_OFF only for lightness, others is same to default. please refer to LCTL/BV-06-C 
				level_poweron = LEVEL_OFF;
				set_on_power_up_last(p_save, level_poweron);
			}
			else{	// (ONPOWER_UP_DEFAULT == ONPOWER_UP_VAL(i))
                s16 def = light_g_level_def_get(i, k);
                if(ST_TRANS_LIGHTNESS == k){
                    if(LEVEL_OFF == def){
                        def = p_save->last;
                    }
                }
                #if (LIGHT_TYPE_CT_EN)
                else if(ST_TRANS_CTL_TEMP == k){
                    // "def" here is CT temp in s16, not level.
                    def = get_level_from_ctl_temp_light_idx(s16_to_u16(def), i);
                }
                #endif

                #if (0 == PTS_TEST_EN)  // PTS MMDL/SR/LCTL/BV-06 will failed because spec page 174, 6.1.3.2
			    if (ONPOWER_UP_DEFAULT == ONPOWER_UP_VAL(i) && (ST_TRANS_LIGHTNESS != k)){
			        level_poweron = get_on_power_up_last(p_save);
			    }else
			    #endif
			    {
                    level_poweron = def;
                    set_on_power_up_last(p_save, level_poweron);
                }
			} 
			
			p_trans->present = p_trans->target = level_poweron;
		}
	}
}

void light_transition_onoff_manual(u8 onoff, u8 transit_t, u8 light_idx)
{
    mesh_cmd_g_onoff_set_t cmd_onoff = {0, 0, 0, 0};
    cmd_onoff.onoff = !!onoff;
    cmd_onoff.transit_t = transit_t;
    st_pub_list_t pub_list = {{0}};
    g_onoff_set(&cmd_onoff, sizeof(cmd_onoff),1,light_idx,0, &pub_list);
}

u8 edch_is_exist()
{
	u32 *p_edch = (u32 *) FLASH_ADR_EDCH_PARA;
	if(*p_edch == 0xffffffff){
		return 0;
	}	
	return 1;
}

void light_pwm_init()
{
#if ((!IS_VC_PROJECT)&&(!__PROJECT_SPIRIT_LPN__))
    #if (MCU_CORE_TYPE == MCU_CORE_8267 || MCU_CORE_TYPE == MCU_CORE_8269)
    REG_ADDR8(0x5b1) = 0x0;     // set default  function Mux to PWM
    REG_ADDR8(0x5b4) |= 0x3;    // set default  function Mux to PWM for PE0/PE1
    #endif

    #if PWM_CLK_DIV_LIGHT
    reg_pwm_clk = PWM_CLK_DIV_LIGHT;    // default value of reg_pwm_clk is 0.
    #endif
    
    foreach(i, LIGHT_CNT){
    	foreach_arr(k,light_res_hw[0]){
			u16 level_def = 0;	// PWM_MAX_TICK;	 //
			const light_res_hw_t *p_hw = &light_res_hw[i][k];
	        pwm_set(p_hw->id, PWM_MAX_TICK, p_hw->invert ? (PWM_MAX_TICK - level_def) : level_def);
	        // light_dim_refresh(i);
	        pwm_start(p_hw->id);
	        #if((MCU_CORE_TYPE==MCU_CORE_8258) || (MCU_CORE_TYPE==MCU_CORE_8278))
	        gpio_set_func(p_hw->gpio, p_hw->func);
	        #else
	        gpio_set_func(p_hw->gpio, AS_PWM);
	        #endif
        }
        
        int onoff_present = light_g_onoff_present_get(i);
        #if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
        if(DUAL_MODE_SUPPORT_ENABLE == dual_mode_state){
            light_transition_onoff_manual(G_ON, 0, i);      // no transition for dual mode,because led flash: LGT_CMD_DUAL_MODE_MESH.
        }else
        #endif
        {
            light_transition_onoff_manual(G_OFF, 0, i);
            if(onoff_present){
				#if !MI_SWITCH_LPN_EN
				light_transition_onoff_manual(G_ON, (analog_read(DEEP_ANA_REG0)&BIT(OTA_REBOOT_FLAG))?0:edch_is_exist()?g_def_trans_time_val(i):0, i);
				#endif
			}
        }
    }
#endif
}
#else
void light_pwm_init(){}
#endif

static u32 tick_light_save;
void light_par_save(int quick)
{
	tick_light_save = (quick ? (clock_time() - BIT(31)) : clock_time()) | 1;
}

void light_par_save_proc()
{
	// save proc
	if(tick_light_save && clock_time_exceed(tick_light_save, 3*1000*1000)){
		tick_light_save = 0;
		if(!is_actived_factory_test_mode()){
		    mesh_common_store(FLASH_ADR_SW_LEVEL);
		}
	}
}

#if (MD_SCENE_EN)
void scene_status_change_check_all()
{
    #if MD_SERVER_EN
	foreach_arr(i,light_res_sw){
		foreach_arr(trans_type,light_res_sw[i].trans){
			st_transition_t *p_trans = P_ST_TRANS(i, trans_type);
        	scene_status_change_check(i, p_trans->present, trans_type);
	    }
	}
    #endif
}
#endif

void pwm_set_lum (int id, u16 y, int pol)
{
#if (IS_VC_PROJECT)
	#if DEBUG_MESH_DONGLE_IN_VC_EN
	extern int DebugMeshLed(int id, u16 y, int pol);
	DebugMeshLed(id, y, pol);
	#endif
#else
    u32 level = ((u32)y * PWM_MAX_TICK) / (255*256);

	pwm_set_cmp (id, pol ? PWM_MAX_TICK - level : level);
#endif
}

u32 get_pwm_cmp(u8 val, u8 lum){
    if(lum >= ARRAY_SIZE(rgb_lumen_map) - 1){
        lum = ARRAY_SIZE(rgb_lumen_map) - 1;
    }
    u32 val_lumen_map = rgb_lumen_map[lum];
    
    return ((u32)val * val_lumen_map) / 255;
}


/**
 * @brief  Control lights on hardware.
 * @param  idx: 1st index of light_res_hw array, also means index 
 *   of LIGHT_CNT.
 * @param  idx2: The 2nd index of light_res_hw array.
 * @param  val: Value.
 * @retval None
 */
void light_dim_set_hw(int idx, int idx2, u16 val)
{
    if((idx < ARRAY_SIZE(light_res_hw)) && (idx2 < ARRAY_SIZE(light_res_hw[0]))){
		const light_res_hw_t *p_hw = &light_res_hw[idx][idx2];
		#if FEATURE_LOWPOWER_EN
		led_onoff_gpio(p_hw->gpio, 0 != val);
		#else
        pwm_set_lum(p_hw->id, val, p_hw->invert);
        #endif
    }
}

#if MD_SERVER_EN
void light_res_sw_g_level_last_set(int idx, int st_trans_type)
{
	//set_level_current_type(idx, st_trans_type);
	sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
	st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
	set_on_power_up_last(p_save, p_trans->target);
	light_par_save(0);
}
#endif

/**
 * @brief  Set Generic Level parameters(Global variables) for light.
 * @param  idx: Light index.
 * @param  level: General Level value.
 * @param  init_time_flag: Reset transition parameter flag.
 *     @arg 0: Don't reset transition parameter.
 *     @arg 1: Reset transition parameter.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval None
 */
void light_res_sw_g_level_set(int idx, s16 level, int init_time_flag, int st_trans_type)
{
	set_level_current_type(idx, st_trans_type);
	st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
	p_trans->present = level;
	if(init_time_flag){
		p_trans->target = level;
		p_trans->remain_t_ms = 0;
		p_trans->delay_ms = 0;
	}
}

void light_res_sw_g_level_target_set(int idx, s16 level, int st_trans_type)	// only for move set command
{
	//set_level_current_type(idx, st_trans_type);
	st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
	p_trans->target = level;
	p_trans->remain_t_ms = 0;
	p_trans->delay_ms = 0;
}

// --------------------------------
/**
 * @brief  Refresh the light status through the global variables 
 *   that have been set.
 * @param  idx: Light index.
 * @retval None
 */
void light_dim_refresh(int idx) // idx: index of LIGHT_CNT.
{
	st_transition_t *p_trans = P_ST_TRANS(idx, ST_TRANS_LIGHTNESS);
	// u16 lightness = get_lightness_from_level(p_trans->present);
#if (LIGHT_TYPE_SEL != LIGHT_TYPE_HSL)
	u8 lum_100 = level2lum(p_trans->present);
#endif
    //LOG_MSG_INFO(DEBUG_SHOW_VC_SELF_EN ? TL_LOG_COMMON : TL_LOG_MESH,0,0,"present_lum %d", lum_100);
	CB_NL_PAR_NUM_3(p_nl_level_state_changed,idx * ELE_CNT_EVERY_LIGHT + ST_TRANS_LIGHTNESS, p_trans->present, p_trans->target);
#if (FEATURE_LOWPOWER_EN)
    foreach_arr(i,light_res_hw[LIGHT_CNT]){
        const light_res_hw_t *p_hw = &light_res_hw[idx][i];
        led_onoff_gpio(p_hw->gpio, 0 != lum_100);
    }
	return ;
#else
    #if(!(LIGHT_TYPE_CT_EN || LIGHT_TYPE_HSL_EN))
    light_dim_set_hw(idx, 0, get_pwm_cmp(0xff, lum_100));
    #else
	    #if (LIGHT_TYPE_CT_EN)
    u8 ct_100 = 0;
    if(ct_flag && (lum_100 != 0)){
	    u16 temp = light_ctl_temp_prensent_get(idx);
	    ct_100 = temp_to_temp100_hw(temp);
		static u8 debug_ct_100; debug_ct_100 = ct_100;
    }
            #if (XIAOMI_MODULE_ENABLE&&!AIS_ENABLE)
    //calc the temp100 transfer for mi 
    u16 mi_temp = light_ctl_temp_prensent_get(idx);
    u8 mi_ct = 0;
    if(mi_temp<2700){
        mi_ct =0;
    }else if (mi_temp > 6500){
        mi_ct = 100;
    }else {
        mi_ct = ((mi_temp-2700)*100)/(6500-2700);
    }
    light_dim_set_hw(idx, 0, 0xffff);// turn on the filter part 
    light_dim_set_hw(idx, 1, get_pwm_cmp(0xff, mi_ct*lum_100/100));
    light_dim_set_hw(idx, 2, get_pwm_cmp(0xff, (100-mi_ct)*lum_100/100));
            #else
    if(ct_flag){
        light_dim_set_hw(idx, 4, get_pwm_cmp(0xff,(100-ct_100)*lum_100/100));
        light_dim_set_hw(idx, 3, get_pwm_cmp(0xff, ct_100*lum_100/100)); 

		if(mode_sw_data[0] == 2) //渐变关闭RGB灯
		{ 
			if(mode_sw_data[1] > 1) mode_sw_data[1] -= 2;
			if(mode_sw_data[2] > 1) mode_sw_data[2] -= 2;
			if(mode_sw_data[3] > 1) mode_sw_data[3] -= 2;

			if((mode_sw_data[1] < 2) && (mode_sw_data[2] < 2) && (mode_sw_data[3] < 2)) 
			{
				mode_sw_data[0] = 0;
				mode_sw_data[1] = 0;
				mode_sw_data[2] = 0;
				mode_sw_data[3] = 0;
			}

			light_dim_set_hw(idx, 0, get_pwm_cmp(0xff, mode_sw_data[1]));
			light_dim_set_hw(idx, 1, get_pwm_cmp(0xff, mode_sw_data[2])); 
			light_dim_set_hw(idx, 2, get_pwm_cmp(0xff, mode_sw_data[3]));
		}
    }
            #endif   
        #endif
        
        #if (LIGHT_TYPE_HSL_EN)
    if(!ct_flag){
        //u8 r = irq_disable();
        //u32 tick_1 = clock_time();
        st_transition_t *p_hue = P_ST_TRANS(idx, ST_TRANS_HSL_HUE);
        st_transition_t *p_sat = P_ST_TRANS(idx, ST_TRANS_HSL_SAT);
        HSL_set HSL;
        RGB_set RGB;
        HSL.h = ((float)(s16_to_u16(p_hue->present))/65535.0f);
        HSL.s = ((float)(s16_to_u16(p_sat->present))/65535.0f);
        HSL.l = ((float)(s16_to_u16(p_trans->present))/65535.0f);
        HslToRgb(HSL,&RGB);
        light_dim_set_hw(idx, 0, get_pwm_cmp(0xff, RGB.r*100/255));
        light_dim_set_hw(idx, 1, get_pwm_cmp(0xff, RGB.g*100/255));
        light_dim_set_hw(idx, 2, get_pwm_cmp(0xff, RGB.b*100/255));
        //static u32 A_1[100];
        // static u32 A_1_cnt;
        //if(A_1_cnt < 100){
        //A_1[A_1_cnt++] = (clock_time()-tick_1)/32;
        //}
        //irq_restore(r);

		if(mode_sw_data[0] == 1) //渐变关闭冷暖灯
		{
			if(mode_sw_data[1] > 1) mode_sw_data[1] -= 2;
			if(mode_sw_data[2] > 1) mode_sw_data[2] -= 2;

			if((mode_sw_data[1] < 2) && (mode_sw_data[2] < 2)) 
			{
				mode_sw_data[0] = 0;
				mode_sw_data[1] = 0;
				mode_sw_data[2] = 0;
			}

			light_dim_set_hw(idx, 4, get_pwm_cmp(0xff, mode_sw_data[1]));
			light_dim_set_hw(idx, 3, get_pwm_cmp(0xff, mode_sw_data[2])); 
		}
    }
	    #endif
    #endif
#endif
}

void get_light_pub_list(int st_trans_type, s16 present_level, s16 target_level, int pub_trans_flag, st_pub_list_t *pub_list)
{
    if(pub_trans_flag){
        pub_list->st[st_trans_type] = ST_G_LEVEL_SET_PUB_TRANS;
    }else{
        pub_list->st[st_trans_type] = (target_level != present_level) ? ST_G_LEVEL_SET_PUB_NOW : ST_G_LEVEL_SET_PUB_NONE;
    }
    if(ST_TRANS_LIGHTNESS == st_trans_type){
        if(pub_list->st[ST_TRANS_LIGHTNESS] && ((LEVEL_OFF == present_level) || (LEVEL_OFF == target_level))){
            pub_list->st[ST_TRANS_PUB_ONOFF] = pub_list->st[ST_TRANS_LIGHTNESS];
        }
    }
}

/**
 * @brief  Set Generic Level for light.
 * @param  idx: Light index.
 * @param  level: General Level value.
 * @param  init_time_flag: Reset transition parameter flag.
 *     @arg 0: Don't reset transition parameter.
 *     @arg 1: Reset transition parameter.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @param  pub_list: list of publication.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_set(int idx, s16 level, int init_time_flag, int st_trans_type, st_pub_list_t *pub_list)
{
	st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
    //if(level != p_trans->present){
        if(pub_list){
            get_light_pub_list(st_trans_type, p_trans->present, level, 0, pub_list);
        }
    	light_res_sw_g_level_set(idx, level, init_time_flag, st_trans_type);
        #if 0  // if support generic level server model
        u16 power = light_power_actual_get(idx);
        light_power_actual_set_par(idx, power, init_time_flag);
        #endif
        
        light_dim_refresh(idx);
    //}
    return 0;
}

//------------------ model call back func
//------get function
u8 light_remain_time_get(st_transition_t *p_trans)
{
	u32 remain_ms = p_trans->remain_t_ms;
	u32 delay_ms = p_trans->delay_ms;

	u8 remain_t = get_transition_step_res(remain_ms/100);
	if(0 == remain_t){
		remain_t = get_transition_step_res((delay_ms+99)/100);
	}
	return remain_t;
}

int light_g_level_get(u8 *rsp, int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		set_level_current_type(idx, st_trans_type);
		mesh_cmd_g_level_st_t *p_st = (mesh_cmd_g_level_st_t *)rsp;
		st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
		p_st->present_level = p_trans->present;
		p_st->target_level = p_trans->target;
		p_st->remain_t = light_remain_time_get(p_trans);
		return 0;
	}
	return -1;
}

/**
 * @brief  Get Light CTL Default Value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Light CTL Default Value.
 */
s16 light_g_level_def_get(int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
		return p_save->def;
	}
	return 0;
}

/**
 * @brief  Get Light CTL Default Value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Light CTL Default Value.
 */
u16 light_g_level_def_get_u16(int idx, int st_trans_type)
{
	return s16_to_u16(light_g_level_def_get(idx, st_trans_type));
}

/**
 * @brief  Set Light CTL Default Value.
 * @param  val: Light CTL Default Value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_def_set(s16 val, int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		//set_level_current_type(idx, st_trans_type);
		sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
		p_save->def = val;
		light_par_save(1);
		return 0;
	}
	return -1;
}

/**
 * @brief  Set Light CTL Default Value.
 * @param  val: Light CTL Default Value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_def_set_u16(u16 val, int idx, int st_trans_type)
{
	return light_g_level_def_set(u16_to_s16(val), idx, st_trans_type);
}

/**
 * @brief  Get level range value.
 * @param  p_range: Point to Light Range value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_range_get(light_range_s16_t *p_range, int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		//set_level_current_type(idx, st_trans_type);
		sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
		p_range->min = p_save->min;
		p_range->max = p_save->max;
		return 0;
	}else{
		memset(p_range, 0, sizeof(light_range_s16_t));
		return -1;
	}
}

/**
 * @brief  Get Light CTL Temperature Range value.
 * @param  p_range: Point to Light CTL Temperature Range value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_range_get_u16(light_range_u16_t *p_range, int idx, int st_trans_type)
{
	light_range_s16_t range_s16 = {0};
	int err = light_g_level_range_get(&range_s16, idx, st_trans_type);
	p_range->min = get_lightness_from_level(range_s16.min);
	p_range->max = get_lightness_from_level(range_s16.max);
	return err;
}


/**
 * @brief  Set Light (Lightness/CTL Temperature/HSL/xyL) Range value.
 * @param  min: Range Min.
 * @param  max: Range Max.
 * @param  idx: Element index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_range_set(u16 min, u16 max, int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		//set_level_current_type(idx, st_trans_type);
		sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
		p_save->min = get_level_from_lightness(min);
		p_save->max = get_level_from_lightness(max);
		light_par_save(1);
		return 0;
	}
	return -1;
}

/**
 * @brief  Get light target level value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval light target level value.
 */
s16 light_g_level_target_get(int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		set_level_current_type(idx, st_trans_type);
		st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
		return p_trans->target;
	}
	return 0;
}

/**
 * @brief  Get light present level value.
 * @param  idx: Element(Light) index.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @retval light present level value.
 */
s16 light_g_level_present_get(int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		set_level_current_type(idx, st_trans_type);
		st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
		return p_trans->present;
	}
	return 0;
}

u16 light_g_level_present_get_u16(int idx, int st_trans_type)
{
	return s16_to_u16(light_g_level_present_get(idx, st_trans_type));
}

u8 light_g_onoff_present_get(int idx)
{
    return (light_g_level_present_get(idx, ST_TRANS_LIGHTNESS) != LEVEL_OFF);
}

s16 light_g_level_last_get(int idx, int st_trans_type)
{
	if(idx < LIGHT_CNT){
		//set_level_current_type(idx, st_trans_type);
		sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
		return p_save->last;
	}
	return 0;
}

s16 get_light_g_level_by_onoff(int idx, int on, int st_trans_type, int force_last)
{
	if(on){
	    s16 last = light_g_level_last_get(idx, st_trans_type);
		if(ST_TRANS_LIGHTNESS == st_trans_type){
            s16 def = light_g_level_def_get(idx, st_trans_type);
		    return ((force_last || (LEVEL_OFF == def)) ? last : def); // refer to Lightness LLN-BV11-C
		}else{
			return last;
		}
	}else{
		return LEVEL_OFF;
	}
}

#if (LIGHT_TYPE_CT_EN)
u16 light_ctl_temp_get_ll(int light_idx, int target_flag)
{
    u16 temp = 0;
	if(light_idx < LIGHT_CNT){
		st_transition_t *p_trans = P_ST_TRANS(light_idx, ST_TRANS_CTL_TEMP);
        light_range_u16_t range_u16 = {0};
        light_g_level_range_get_u16(&range_u16, light_idx, ST_TRANS_CTL_TEMP);
		temp = get_ctl_temp_from_level(target_flag ? p_trans->target : p_trans->present, range_u16.min, range_u16.max);
	}
	return temp;
}

u16 light_ctl_temp_prensent_get(int light_idx)
{
	return light_ctl_temp_get_ll(light_idx, 0);
}

u16 light_ctl_temp_target_get(int light_idx)
{
	return light_ctl_temp_get_ll(light_idx, 1);
}
#endif

s16 lum2level(u8 lum)
{
	if(lum > 100){
		lum  = 100;
	}
	return (-32768 + division_round(65535*lum,100));
}

u8 level2lum(s16 level)
{
	u16 lightness = level + 32768;
	u32 fix_1p2 = 0;
	if(lightness){	// fix decimals
		#define LEVEL_UNIT_1P2	(65535/100/2)
		if(lightness < LEVEL_UNIT_1P2 + 2){     // +2 to fix accuracy missing
			lightness = LEVEL_UNIT_1P2 * 2;		// make sure lum is not zero when light on.
		}
		fix_1p2 = LEVEL_UNIT_1P2;
	}
	return (((lightness + fix_1p2)*100)/65535);
}

u16 lum2_lightness(u8 lum)
{
	return (get_lightness_from_level(lum2level(lum)));
}

u8 lightness2_lum(u16 lightness)
{
	return (level2lum(get_level_from_lightness(lightness)));
}

u16 temp100_to_temp(u8 temp100)
{
	if(temp100 > 100){
		temp100  = 100;
	}
	return (CTL_TEMP_MIN + ((CTL_TEMP_MAX - CTL_TEMP_MIN)*temp100)/100);
}

u8 temp_to_temp100_hw(u16 temp) // use for driver pwm, 0--100 is absolute value, not related to temp range
{
	if(temp < CTL_TEMP_MIN){
		temp = CTL_TEMP_MIN;
	}
	
	if(temp > CTL_TEMP_MAX){
		temp = CTL_TEMP_MAX;
	}
	u32 fix_1p2 = (CTL_TEMP_MAX - CTL_TEMP_MIN)/100/2;	// fix decimals
	return (((temp - CTL_TEMP_MIN + fix_1p2)*100)/(CTL_TEMP_MAX - CTL_TEMP_MIN));   // temp100 can be zero.
}

u8 temp_to_temp100(u16 temp)
{
	return temp_to_temp100_hw(temp);   // comfirm later, related with temp range
}

u8 light_lum_get(int idx, int target_flag)
{
	st_transition_t *p_trans = P_ST_TRANS(idx, ST_TRANS_LIGHTNESS);
    return level2lum(target_flag? p_trans->target : p_trans->present);
}

#if (LIGHT_TYPE_CT_EN)
u8 light_ct_lum_get(int idx, int target_flag)
{
    return temp_to_temp100(light_ctl_temp_get_ll(idx, target_flag));
}
#endif

//------set function
#if 1
int light_onoff_idx(int idx, int on, int init_time_flag){
    if(idx < LIGHT_CNT){
    	int st_trans_type = ST_TRANS_LIGHTNESS;
    	st_pub_list_t pub_list = {{0}};
    	light_g_level_set(idx, get_light_g_level_by_onoff(idx, on, st_trans_type, 1), init_time_flag, st_trans_type, &pub_list);
    }
    return 0;
}
#endif

/**
 * @brief  Set Generic Level for light by index.
 * @param  idx: Light index.
 * @param  level: General Level value.
 * @param  init_time_flag: Reset transition parameter flag.
 *     @arg 0: Don't reset transition parameter.
 *     @arg 1: Reset transition parameter.
 * @param  st_trans_type: A value in the enumeration type ST_TRANS_TYPE.
 * @param  pub_list: list of publication.
 * @retval Whether the function executed successfully
 *   (0: success; others: error)
 */
int light_g_level_set_idx(int idx, s16 level, int init_time_flag, int st_trans_type, st_pub_list_t *pub_list)
{
    if(idx < LIGHT_CNT){
        light_g_level_set(idx, level, init_time_flag, st_trans_type, pub_list);
    }
    return 0;
}

#if MD_SERVER_EN
void light_g_level_set_idx_with_trans(u8 *set_trans, int idx, int st_trans_type)
{
    if(idx < LIGHT_CNT){
		set_level_current_type(idx, st_trans_type);
		st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
		mesh_set_trans_t *p_set = (mesh_set_trans_t *)set_trans;
		p_trans->target = p_set->target_val;
		if(0x3F == (p_set->transit_t & 0x3F)){
			p_trans->remain_t_ms = -1;
		}else{
			p_trans->remain_t_ms = 100 * get_transition_100ms((trans_time_t *)&p_set->transit_t);
			if(p_trans->remain_t_ms){
                if(is_level_move_set_op(p_set->op)){
                    // have been make sure (target_val != present_val) and (level_move != 0) before.
				    p_trans->step_1p32768 = ((p_set->level_move * 32768) /(s32)(p_trans->remain_t_ms)) * LIGHT_ADJUST_INTERVAL;
                    u32 abs_step = abs(p_set->level_move);
                    u32 abs_delta = (p_set->target_val - p_set->present_val);
                    u32 mod = abs_delta % abs_step;
                    u32 remain_t_ms_org = p_trans->remain_t_ms;
                    u32 val;
                    if(remain_t_ms_org >= 65536){
                        // remain_t_ms_org is less than 37800000, so (remain_t_ms_org * 100) is less than 0xffffffff
                        val = (((remain_t_ms_org * 100)/abs_step)*(mod))/100;
                    }else{
                        // make sure not overflow
                        val = (remain_t_ms_org * mod) / abs_step;
                    }
                    p_trans->remain_t_ms = remain_t_ms_org * (abs_delta / abs_step) + val;
                }else{
				    p_trans->step_1p32768 = (((p_trans->target - p_trans->present) * 32768) /(s32)(p_trans->remain_t_ms)) * LIGHT_ADJUST_INTERVAL;
				}
			}else{
				p_trans->step_1p32768 = 0;
			}
			p_trans->present_1p32768 = 0; // init
		}
		p_trans->delay_ms = p_set->delay * 5;
    }
}
#endif

void light_onoff_all(u8 on){
    foreach(i, LIGHT_CNT){
        light_onoff_idx(i, on, 1);
    }
}

#if CMD_LINEAR_EN
int set_light_linear_flag(int idx,u16 linear)
{
	if(idx < LIGHT_CNT){
	    light_res_sw[idx].linear_set_flag = 1;
	    light_res_sw[idx].linear = linear;
	}
	return 0;
}

int clear_light_linear_flag(int idx)
{
	if(idx < LIGHT_CNT){
	    light_res_sw[idx].linear_set_flag = 0;
	    light_res_sw[idx].linear = 0;
	}
	return 0;
}

u16 get_light_linear_val(int idx)
{
    return light_res_sw[idx].linear;
}

int is_linear_flag(int idx)
{
	if(idx < LIGHT_CNT){
	    return light_res_sw[idx].linear_set_flag;
	}
	return 0;
}
#endif

void app_led_en (int id, int en)
{
    if(id < LIGHT_CNT){
        light_onoff_idx(id, en, 1);
    }
}

#if MD_SERVER_EN
// transition and delay proc
s16 light_get_next_level(int idx, int st_trans_type)
{
    st_transition_t *p_trans = P_ST_TRANS(idx, st_trans_type);
    sw_level_save_t *p_save = P_SW_LEVEL_SAVE(idx, st_trans_type);
	s32 adjust_1p32768 = p_trans->step_1p32768+ p_trans->present_1p32768;
	s32 result = p_trans->present + (adjust_1p32768 / 32768);
	p_trans->present_1p32768 = adjust_1p32768 % 32768;
	
    result = get_val_with_check_range(result, p_save->min, p_save->max, st_trans_type);
	
	return (s16)result;
}

void light_transition_log(int st_trans_type, s16 present_level)
{
	if(ST_TRANS_LIGHTNESS == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_level %d", present_level);
	#if (LIGHT_TYPE_CT_EN)
	}else if(ST_TRANS_CTL_TEMP == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_ctl_temp 0x%04x", s16_to_u16(present_level));
	}else if(ST_TRANS_CTL_D_UV == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_ctl_D_UV %d", present_level);
	#endif
	#if (LIGHT_TYPE_HSL_EN)
	}else if(ST_TRANS_HSL_HUE == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_hsl_hue 0x%04x", s16_to_u16(present_level));
	}else if(ST_TRANS_HSL_SAT == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_hsl_sat 0x%04x", s16_to_u16(present_level));
	#endif
	#if (LIGHT_TYPE_SEL == LIGHT_TYPE_XYL)
	}else if(ST_TRANS_XYL_X == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_xyl_x 0x%04x", s16_to_u16(present_level));
	}else if(ST_TRANS_XYL_Y == st_trans_type){
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"present_xyl_y 0x%04x", s16_to_u16(present_level));
	#endif
	}else{
		LOG_MSG_INFO(TL_LOG_MESH,0,0,"xxxx 0x%04x", s16_to_u16(present_level));
	}
}

void light_transition_proc()
{
    int all_trans_ok = 1;   // include no transition
	foreach_arr(i,light_res_sw){
		foreach_arr(trans_type,light_res_sw[i].trans){
			st_transition_t *p_trans = P_ST_TRANS(i, trans_type);
			int complete_level = 0;
			if(p_trans->delay_ms){
				p_trans->delay_ms--;
				if((0 == p_trans->delay_ms) && (0 == p_trans->remain_t_ms)){
					complete_level = 1;
				}else{
				    all_trans_ok = 0;
				}
			}else{
				if(p_trans->remain_t_ms){	
					if(p_trans->present != p_trans->target){
						if(0 == (p_trans->remain_t_ms % LIGHT_ADJUST_INTERVAL)){
                            s16 next_val = light_get_next_level(i, trans_type);
                            st_pub_list_t pub_list = {{0}};
							light_g_level_set_idx(i, next_val, 0, trans_type, &pub_list);
							light_transition_log(trans_type, p_trans->present);
						}
					}
					p_trans->remain_t_ms--;
					if(0 == p_trans->remain_t_ms){
						complete_level = 1;	// make sure the last status is same with target
					}else{
				        all_trans_ok = 0;
				    }
				}
			}

			if(complete_level){
                st_pub_list_t pub_list = {{0}};
				light_g_level_set_idx(i, p_trans->target, 0, trans_type, &pub_list);
				light_transition_log(trans_type, p_trans->present);

                #if MD_SCENE_EN
				scene_target_complete_check(i);
				#endif
			}
		}
	}

	if(all_trans_ok){
	    if(light_pub_trans_step){
	        light_pub_trans_step = ST_PUB_TRANS_ALL_OK;
	    }
	}
}
#endif


// LED function
void light_dim_refresh_all()
{
    foreach(i, LIGHT_CNT){
        light_dim_refresh(i);
    }
}


typedef void (*fp_proc_led)(void);
fp_proc_led 				p_vendor_proc_led 				= 0;


static u32 led_event_pending;
static int led_count = 0;

void cfg_led_event (u32 e)
{
	led_event_pending = e;
}

int is_led_busy()
{
    return (!(!led_count && !led_event_pending));
}

void led_onoff_gpio(u32 gpio, u8 on){
#if FEATURE_LOWPOWER_EN
    gpio_set_func (gpio, AS_GPIO);
    gpio_set_output_en (gpio, 0);
    gpio_write(gpio, 0);
    gpio_setup_up_down_resistor(gpio, on ? PM_PIN_PULLUP_10K : PM_PIN_PULLDOWN_100K);
#else
    gpio_set_func (gpio, AS_GPIO);
    gpio_set_output_en (gpio, 1);
    gpio_write(gpio, on);
#endif
}

#if __PROJECT_MESH_SWITCH__
void proc_led()
{
	if(p_vendor_proc_led){
		p_vendor_proc_led();
		return;
	}
	static	u32 led_ton;
	static	u32 led_toff;
	static	int led_sel;						//
	static	u32 led_tick;
	static	int led_no;
	static	int led_is_on;

	if(!led_count && !led_event_pending) {
		return;  //led flash finished
	}

	if (led_event_pending)
	{
		// new event
		led_ton = (led_event_pending & 0xff) * 64000 * CLOCK_SYS_CLOCK_1US;
		led_toff = ((led_event_pending>>8) & 0xff) * 64000 * CLOCK_SYS_CLOCK_1US;
		led_count = (led_event_pending>>16) & 0xff;
		led_sel = led_event_pending>>24;

		led_event_pending = 0;
		led_tick = clock_time () + 30000000 * CLOCK_SYS_CLOCK_1US;
		led_no = 0;
		led_is_on = 0;
	}

	if( 1 ){
		if( (u32)(clock_time() - led_tick) >= (led_is_on ? led_ton : led_toff) ){
			led_tick = clock_time ();
			
			led_is_on = !led_is_on;
			if (led_is_on)
			{
				led_no++;
				if (led_no - 1 == led_count)
				{
					led_count = led_no = 0;
					return ;
				}
			}
			
			int led_off = (!led_is_on || !led_ton) && led_toff;
			int led_on = led_is_on && led_ton;
			
			if( led_off || led_on  ){
				if (led_sel & BIT(0))
				{
					led_onoff_gpio(GPIO_LED, led_on);
				}
            }
        }
	}

}
#else

void proc_led(void)
{
	if(p_vendor_proc_led){
		p_vendor_proc_led();
		return;
	}
	static	u32 led_ton;
	static	u32 led_toff;
	static	int led_action;						//LED事件结束后动作
	static	u32 led_tick;
	static	int led_no;
	static	int led_is_on;

	if(!led_count && !led_event_pending) {
		return;  //led flash finished
	}

	if (led_event_pending)
	{
		// new event
		led_ton = (led_event_pending & 0xff) * 64000 * CLOCK_SYS_CLOCK_1US;
		led_toff = ((led_event_pending>>8) & 0xff) * 64000 * CLOCK_SYS_CLOCK_1US;
		led_count = (led_event_pending>>16) & 0xff;
		led_action = led_event_pending>>24;

		
		led_tick = clock_time () + 30000000 * CLOCK_SYS_CLOCK_1US;
		led_no = 0;
		led_is_on = 0;
	}

	if( 1 ){
		if( (u32)(clock_time() - led_tick) >= (led_is_on ? led_ton : led_toff) ){
			led_tick = clock_time ();
			int led_off = (led_is_on || !led_ton) && led_toff;
			int led_on = !led_is_on && led_ton;

			led_is_on = !led_is_on;
			if (led_is_on)
			{
				led_no++;
				//led_dbg++;
				if (led_no - 1 == led_count)
				{
					if(led_action == 0xff) //重启事件
					{
						sleep_ms(500);
						start_reboot();
					}

					led_count = led_no = 0;
					//light_dim_refresh_all(); // should not report online status again
					st_transition_t *p_trans = P_ST_TRANS(0, 0);
					p_trans->present = -32768;
					p_trans->target = 32767 * 0.8;
					p_trans->remain_t_ms = 1000;
					p_trans->delay_ms = 500;
					p_trans->step_1p32768 = (((p_trans->target - p_trans->present) * 32768) /(s32)(p_trans->remain_t_ms)) * LIGHT_ADJUST_INTERVAL;

					return ;
				}
			}
			
			if( led_off || led_on  ){
                // light_dim_set_hw(GPIO_LED_INDEX, 0, LED_INDICATE_VAL * led_on);
				st_transition_t *p_trans = P_ST_TRANS(0, 0);

				p_trans->present = led_on?-32768:15000;
				p_trans->target = led_on?15000:-32768;
				p_trans->remain_t_ms = 1000;

				if(led_event_pending != 0)
				{
					led_event_pending = 0;
					p_trans->delay_ms = 500;
				}
				p_trans->step_1p32768 = (((p_trans->target - p_trans->present) * 32768) /(s32)(p_trans->remain_t_ms)) * LIGHT_ADJUST_INTERVAL;
            }
        }
	}

}
#endif 
void rf_link_light_event_callback (u8 status)
{
#ifdef WIN32
	return ;
#endif

#if FEATURE_LOWPOWER_EN
    if(lpn_provision_ok){
        #if 1 // for test
        if(LGT_CMD_FRIEND_SHIP_OK == status){
            cfg_led_event(LED_EVENT_FLASH_4HZ_3T);
        }else{
            cfg_led_event(0); // LED_EVENT_FLASH_4HZ_2T
        }
		
        while(is_led_busy()){
            proc_led();
            wd_clear();
        }
        #else
        cfg_led_event(0); // don't show LED
        #endif
        
        return ;
    }
#endif

	if(status == LGT_CMD_SET_MESH_INFO){
        cfg_led_event(LED_EVENT_PROVISION_SUCCESS);
    }else if(status == LGT_CMD_SET_SUBSCRIPTION){
        //cfg_led_event(LED_EVENT_FLASH_1HZ_3S);
#if DEBUG_BLE_EVENT_ENABLE
    }else if(status == LGT_CMD_BLE_ADV){
		cfg_led_event(LED_EVENT_FLASH_1HZ_2S);
	}else if(status == LGT_CMD_BLE_CONN){
		cfg_led_event(LED_EVENT_FLASH_4HZ_3T);
#endif
#if __PROJECT_MESH_SWITCH__
	}else if(status == LGT_CMD_SWITCH_POWERON){
		cfg_led_event(LED_EVENT_FLASH_1HZ_1S);
	}else if(status == LGT_CMD_SWITCH_PROVISION){
		cfg_led_event(LED_EVENT_FLASH_4HZ_3T);
	}else if(status == LGT_CMD_SWITCH_CMD){
		cfg_led_event(LED_EVENT_FLASH_4HZ_1T);
#endif
#if DONGLE_PROVISION_EN
	}else if(status == PROV_START_LED_CMD){
		cfg_led_event(LED_EVENT_FLASH_4HZ_3T);
	}else if(status == PROV_END_LED_CMD){
		cfg_led_event(LED_EVENT_FLASH_STOP);
#endif
#if (DUAL_MODE_ADAPT_EN || DUAL_MODE_WITH_TLK_MESH_EN)
	}else if(status == LGT_CMD_DUAL_MODE_MESH){
		cfg_led_event(DUAL_MODE_WITH_TLK_MESH_EN ? LED_EVENT_FLASH_2HZ_2T : LED_EVENT_FLASH_2HZ_1T);
#endif
	}
}

#ifndef WIN32
void light_ev_with_sleep(u32 count, u32 half_cycle_us)
{
	gpio_set_func(GPIO_LED, AS_GPIO);
	gpio_set_output_en(GPIO_LED, 1);
	gpio_set_input_en(GPIO_LED, 0);
	
	for(u32 i=0; i< count;i++){
#if(MODULE_WATCHDOG_ENABLE)
        wd_clear();
#endif
		gpio_write(GPIO_LED, 0);
		sleep_us(half_cycle_us);
#if(MODULE_WATCHDOG_ENABLE)
        wd_clear();
#endif
		gpio_write(GPIO_LED, 1);
		sleep_us(half_cycle_us);
	}

	gpio_write(GPIO_LED, 0);
}

void show_ota_result(int result)
{
	if(result == OTA_SUCCESS){
		light_ev_with_sleep(3, 1000*1000);	//0.5Hz shine for  6 second
	}
	else{
		light_ev_with_sleep(30, 100*1000);	//5Hz shine for  6 second
		//write_reg8(0x8000,result); ;while(1);  //debug which err lead to OTA fail
	}
}

void show_factory_reset()
{
	//light_ev_with_sleep(8, 500*1000);	//1Hz shine for  8 second
	cfg_led_event(LED_EVENT_FACTORY_SUCCESS);
}
#endif

/**
  * @}
  */
    
/**
  * @}
  */


