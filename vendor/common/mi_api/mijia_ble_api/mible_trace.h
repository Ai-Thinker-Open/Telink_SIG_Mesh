#ifndef __MIBLE_TRACE_H__
#define __MIBLE_TRACE_H__

/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex-M3 above with a DWT unit */
#ifdef DWT
#define DWT_CYCCNTENA_BIT       (1UL<<0)
#define TRCENA_BIT              (1UL<<24)

/*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register */

#define InitCycleCounter() \
  CoreDebug->DEMCR |= TRCENA_BIT

#define ResetCycleCounter() \
  DWT->CYCCNT = 0

#define EnableCycleCounter() \
  DWT->CTRL |= DWT_CYCCNTENA_BIT

#define DisableCycleCounter() \
  DWT->CTRL &= ~DWT_CYCCNTENA_BIT

#define GetCycleCounter() \
        (DWT->CYCCNT)
#else
#define InitCycleCounter()
#define ResetCycleCounter()
#define EnableCycleCounter()
#define DisableCycleCounter()
#define GetCycleCounter()
#endif /* DWT END*/

#if TIME_PROFILE
#define TIMING_INIT()                                                           \
    InitCycleCounter()

#define TIMING_BEGIN()                                                          \
    ResetCycleCounter()

#define TIMING_END(name)                                                        \
    MI_LOG_DEBUG("%s consume time: %u us\n", (char*)name, GetCycleCounter()/(SystemCoreClock/1000000))
#else
#define TIMING_INIT()
#define TIMING_BEGIN()
#define TIMING_END(name)
#endif

static uint32_t ref_tick;
static uint32_t cpu_clk = 38400000;

static __INLINE void init_time_profile(uint8_t cpu_running_hz)
{
    if (cpu_running_hz > 0 )
        cpu_clk = cpu_running_hz;

    InitCycleCounter();
    ResetCycleCounter();
    EnableCycleCounter();
}

static __INLINE void set_time_ref()
{
    //ref_tick = GetCycleCounter();
}

static __INLINE uint32_t us_from_time_ref()
{
    return (GetCycleCounter() - ref_tick) * 10 / 384;
}

#endif /* __MIBLE_TRACE_H__ */
