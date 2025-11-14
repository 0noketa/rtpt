/*
 * Protothreads-based simple task scheduler. 
 */


#include <xc.h>
#include "config.h"
#include "rtpt.h"

#if defined(_18F4520)
    #define LED0(x) PORTDbits.RD0 = (x)
    #define LED1(x) PORTDbits.RD1 = (x)
    #define LEDS_INIT() TRISDbits.TRISD0 = 0; TRISDbits.TRISD1 = 0
#elif defined(_18F24J50)
    #define LED0(x) PORTAbits.RA2 = (x)
    #define LED1(x) PORTAbits.RA3 = (x)
    #define LEDS_INIT() TRISAbits.TRISA2 = 0; TRISAbits.TRISA3 = 0
#elif defined(_12F675)
    #define LED0(x) GPIObits.GPIO0 = (x)
    #define LED1(x) GPIObits.GPIO1 = (x)
    #define LEDS_INIT() do { \
        WPU0 = 0; TRISIObits.TRISIO0 = 0; \
        WPU1 = 0; TRISIObits.TRISIO1 = 0; \
    } while(0)
#elif defined(_16F505)
    #define LED0(x) PORTCbits.RC0 = (x)
    #define LED1(x) PORTCbits.RC1 = (x)
    #define LEDS_INIT() TRISC =  ~((1 << _PORTC_RC0_POSITION) | (1 << _PORTC_RC1_POSITION))
#endif


typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task0Context;

RTPT_BEGIN_TASK(task0, Task0Context, ctx, task0_ctx)
{
    while(1) {
        for (ctx->i = 0; ctx->i < 10; ++ctx->i) {
            LED0(1);
            RTPT_DELAY(ctx, 1000);
            LED0(0);
            RTPT_DELAY(ctx, 1000);
        }
        for (ctx->i = 0; ctx->i < 5; ++ctx->i) {
            LED0(1);
            RTPT_DELAY(ctx, 200);
            LED0(0);
            RTPT_DELAY(ctx, 200);
        }
    }
}
RTPT_END_TASK(ctx)


typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task1Context;

RTPT_BEGIN_TASK(task1, Task1Context, ctx, task1_ctx)
{
    while (1) {
        LED1(0);
        RTPT_DELAY(ctx, 1000);
        LED1(1);
        RTPT_DELAY(ctx, 1000);
    }
}
RTPT_END_TASK(ctx)


void init(void) {
    #ifdef _OSCCON_IRCF_LENGTH
        OSCCONbits.IRCF = HZ_TO_IRCF(_XTAL_FREQ);
    #endif
    #ifdef GIE_bit
        GIE = 1;
    #endif
    #ifdef PEIE_bit
        PEIE = 1;
    #endif

    LEDS_INIT();
}

void main(void) {
    init();
    RTPT_Init();
    RTPT_CreateTask(&task0_ctx, task0);
    RTPT_CreateTask(&task1_ctx, task1);
    RTPT_StartTasks();
}
