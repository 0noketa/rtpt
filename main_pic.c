/*
 * Protothreads-based simple task scheduler. 
 */


#include <xc.h>
#include "config.h"
#include "rtpt.h"

#if defined(_18F4520)
    #define LED0(x) PORTDbits.RD0 = (x)
    #define LED1(x) PORTDbits.RD1 = (x)
    #define LED0_DIR(x) TRISDbits.TRISD0 = (x)
    #define LED1_DIR(x) TRISDbits.TRISD1 = (x)
#elif defined(_18F24J50)
    #define LED0(x) PORTAbits.RA2 = (x)
    #define LED1(x) PORTAbits.RA3 = (x)
    #define LED0_DIR(x) TRISAbits.TRISA2 = (x)
    #define LED1_DIR(x) TRISAbits.TRISA3 = (x)
#elif defined(_12F675)
    #define LED0(x) GPIObits.GPIO0 = (x)
    #define LED1(x) GPIObits.GPIO1 = (x)
    #define LED0_DIR(x) do { WPU0 = 0; TRISIObits.TRISIO0 = (x); } while(0)
    #define LED1_DIR(x) do { WPU1 = 0; TRISIObits.TRISIO1 = (x); } while(0)
#endif


typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task0Context;

RTPT_BEGIN_TASK(task0, Task0Context, ctx)
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
RTPT_END_TASK(ctx)

typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task1Context;

uint16_t task1(void *param) {
    Task1Context *ctx = param;
    RTPT_BEGIN(ctx);

    while (1) {
        LED1(0);
        RTPT_DELAY(ctx, 1000);
        LED1(1);
        RTPT_DELAY(ctx, 1000);
    }
    RTPT_END(ctx);
}

void init(void) {
    #ifdef _OSCCON_IRCF_LENGTH
        OSCCONbits.IRCF = HZ_TO_IRCF(_XTAL_FREQ);
    #endif
    GIE = 1;
    PEIE = 1;
    LED0_DIR(0);
    LED1_DIR(0);
}

void main(void) {
    init();
    Task0Context task0_ctx;
    Task1Context task1_ctx;
    RTPT_Init();
    RTPT_CreateTask(&task0_ctx, task0);
    RTPT_CreateTask(&task1_ctx, task1);
    RTPT_StartTasks();
}
