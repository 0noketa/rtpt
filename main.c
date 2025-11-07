/*
 * Protothreads-based simple task scheduler. 
 */


#include <stdio.h>
#include "rtpt_config.h"
#include "rtpt.h"


typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task0Context;

RTPT_BEGIN_TASK(task0, Task0Context, ctx)
    for (ctx->i = 0; ctx->i < 10; ++ctx->i) {
        putchar('A');
        fflush(stdout);
        RTPT_DELAY(ctx, 1000);
    }
RTPT_END_TASK(ctx)

typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task1Context;

uint16_t task1(void *param) {
    Task1Context *ctx = param;
    RTPT_BEGIN(ctx);

    for (ctx->i = 0; ctx->i < 10; ++ctx->i) {
        putchar('B');
        fflush(stdout);
        RTPT_DELAY(ctx, 500);
    }

    RTPT_END(ctx);
}

void main(void) {
    Task0Context task0_ctx;
    Task1Context task1_ctx;
    RTPT_Init();
    RTPT_CreateTask(&task0_ctx, task0);
    RTPT_CreateTask(&task1_ctx, task1);
    RTPT_StartTasks();

    putchar('\n');
    printf("0.i: %d, 0.lc: %d\n", task0_ctx.i, (int)task0_ctx.rtpt_ctx.lc);
    printf("1.i: %d, 1.lc: %d\n", task1_ctx.i, (int)task1_ctx.rtpt_ctx.lc);
    puts("done");
}
