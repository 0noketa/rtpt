/*
 * Protothreads-based simple task scheduler. 
 */


 #include <stdint.h>
#include <lc.h>
#define RTPT_MAX_TASKS 4

#define RTPT_BEGIN(ctx) LC_RESUME((ctx)->rtpt_ctx.lc)
#ifdef __LC_ADDRLABELS_H__
#define RTPT_DELAY(ctx, interval) (ctx)->rtpt_ctx.lc = &&LC_CONCAT(LC_LABEL,__LINE__); return interval; LC_SET((ctx)->rtpt_ctx.lc)
#define RTPT_HALT(ctx) (ctx)->rtpt_ctx.lc = &&RTPT_EXIT_LABEL; return -1
#define RTPT_END(ctx) RTPT_EXIT_LABEL:; LC_END((ctx)->rtpt_ctx.lc); return -1
typedef struct {
    void *lc;
} RTPT_TaskContext;
#else
#define RTPT_DELAY(ctx, interval) (ctx)->rtpt_ctx.lc = __LINE__; return interval; LC_SET((ctx)->rtpt_ctx.lc)
#define RTPT_HALT(ctx) (ctx)->rtpt_ctx.lc = INT16_MAX; return -1
#define RTPT_END(ctx) case INT16_MAX:; LC_END((ctx)->rtpt_ctx.lc); return -1
typedef struct {
    int16_t lc;
} RTPT_TaskContext;
#endif

#define RTPT_BEGIN_TASK(name, type, ctx_name) int name(void *RTPT_param) { type *ctx_name = RTPT_param; RTPT_BEGIN(ctx_name)
#define RTPT_END_TASK(ctx) RTPT_END(ctx); }


typedef struct {
    int (*task_func)(void*);
    void *ctx;
    uint16_t next_tick;
    uint8_t skipped;
} RTPT_Task;

RTPT_Task RTPT_tasks[RTPT_MAX_TASKS];
int RTPT_tasks_count;
volatile uint16_t RTPT_tick;  // replaced in ISR


int RTPT_CreateTask(void *ctx, int *(task_func)(void*)) {
    if (RTPT_tasks_count >= RTPT_MAX_TASKS) return 0;

    RTPT_tasks[RTPT_tasks_count].task_func = task_func;
    RTPT_tasks[RTPT_tasks_count].ctx = ctx;
    RTPT_tasks[RTPT_tasks_count].next_tick = RTPT_tick;
    RTPT_tasks[RTPT_tasks_count].skipped = 0;
    ++RTPT_tasks_count;
    return 1;
}
int RTPT_FindTask(uint16_t tick) {
    for (int i = 0; i < RTPT_tasks_count; ++i) {
        if (RTPT_tasks[i].skipped) return i;
        if (RTPT_tasks[i].next_tick == tick) return i;
    }
    return -1;
}
void RTPT_MarkSkippedTasks(uint16_t start_tick, uint16_t end_tick) {
    if (start_tick <= end_tick) {
        for (int i = 0; i < RTPT_tasks_count; ++i) {
            if (RTPT_tasks[i].next_tick >= start_tick
                    && RTPT_tasks[i].next_tick < end_tick
            ) {
                RTPT_tasks[i].skipped = 1;
            }
        }
    } else {
        for (int i = 0; i < RTPT_tasks_count; ++i) {
            if (RTPT_tasks[i].next_tick >= start_tick
                    || RTPT_tasks[i].next_tick < end_tick
            ) {
                RTPT_tasks[i].skipped = 1;
            }
        }
    }
}
void RTPT_StartTasks() {
    RTPT_tick = 0;
    while (1) {
        uint16_t tick0 = RTPT_tick;
        int task_id = RTPT_FindTask();
        if (task_id == -1) break;
        int interval = RTPT_tasks[task_id].task_func(RTPT_tasks[task_id].ctx);
        uint16_t tick1 = RTPT_tick;
        RTPT_tasks[task_id].skipped = 0;
        RTPT_tasks[task_id].next_tick = tick1 + interval;
        RTPT_MarkSkippedTasks(tick0, tick1);
    }
}




typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task0Context;
RTPT_BEGIN_TASK(task0, Task0Context, ctx)
    for (ctx->i = 0; ctx->i < 10; ++ctx->i) {
        RTPT_DELAY(ctx, 1000);
    }
RTPT_END_TASK(ctx)

typedef struct {
    RTPT_TaskContext rtpt_ctx;
    int i;
} Task1Context;
int task1(void *param) {
    Task1Context *ctx = param;
    RTPT_BEGIN(ctx);

    for (ctx->i = 0; ctx->i < 10; ++ctx->i) {
        RTPT_DELAY(ctx, 1000);
    }

    RTPT_END(ctx);
}

void main(void) {
    Task0Context task0_ctx;
    Task1Context task1_ctx;
    RTPT_CreateTask(&task0_ctx, task0);
    RTPT_CreateTask(&task1_ctx, task1);
    RTPT_StartTasks();
    while (1);
}
