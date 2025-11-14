/*
 * Protothreads-based simple task scheduler. 
 */
#ifndef _RTPT_CONFIG_H_
#define _RTPT_CONFIG_H_


#include <stdint.h>

#define RTPT_TARGET_POSIX 0
#define RTPT_TARGET_PTHREAD 1
#define RTPT_TARGET_WIN32 2
#define RTPT_TARGET_PIC_XC8 3


#define RTPT_MAX_TASKS 2
#define RTPT_TICK_MS 4
#define RTPT_TARGET RTPT_TARGET_PIC_XC8
// #define RTPT_TARGET RTPT_TARGET_PTHREAD

// 0: context variable == argument. task functions can be used for multiple tasks. 1: context variable == global variable
#define RTPT_CONFIG_STATIC 1

// 0: stop task scheduler when all task are stopped, 1: task sceduler will not stop
#define RTPT_CONFIG_CONTINUOUS 0

// 0: Timer0(16-bit), 1: Timer1(16-bit), 3: Timer3(16-bit), 10: Timer0(8-bit)
#define RTPT_CONFIG_PIC_TMR 0
// 0: use busy-waiting + Timer0(8-bit) and ignore RTPT_CONFIG_PIC_TMR, 1: use interrupts
#define RTPT_CONFIG_PIC_INT 0

// #define RTPT_INCLUDE_COUNT
// #define RTPT_INCLUDE_SUSPEND
// #define RTPT_INCLUDE_KILL


// fixed INTOSC
#ifndef RTPT_CONFIG_PIC_TMR_FREQ
    #if defined(_18F4520)
        #define RTPT_CONFIG_PIC_TMR_FREQ 8000000
    #elif defined(_18F24J50)
        #define RTPT_CONFIG_PIC_TMR_FREQ 8000000
    #elif defined(_16F505)
        #define RTPT_CONFIG_PIC_TMR_FREQ 4000000
    #elif defined(_12F675)
        #define RTPT_CONFIG_PIC_TMR_FREQ 4000000
    #endif
#endif

#if RTPT_MAX_TASKS >= 127
    #undef RTPT_MAX_TASKS
    #define RTPT_MAX_TASKS 127
#endif


#endif  // _RTPT_CONFIG_H_
