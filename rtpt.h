/*
 * Protothreads-based simple task scheduler. 
 */
#ifndef _RTPT_H_
#define _RTPT_H_


#include <stdint.h>
#include "lc-addrlabels.h"

#define RTPT_MAX_TASKS 4
#define RTPT_TICK_MS 100

#define RTPT_TARGET_POSIX 0
#define RTPT_TARGET_PTHREADS 1
#define RTPT_TARGET_WIN32 2
// not implemented
#define RTPT_TARGET_PIC_XC8 3


#define RTPT_TARGET RTPT_TARGET_POSIX
#define RTPT_CONFIG_PIC_XC8_TMR 1


#define RTPT_INIT(ctx) LC_INIT((ctx)->rtpt_ctx.lc)
#define RTPT_BEGIN(ctx) LC_RESUME((ctx)->rtpt_ctx.lc)
#ifdef __LC_ADDRLABELS_H__
#define RTPT_DELAY(ctx, interval) (ctx)->rtpt_ctx.lc = &&LC_CONCAT(LC_LABEL,__LINE__); return !(interval) ? 1 : (interval) / RTPT_TICK_MS; LC_SET((ctx)->rtpt_ctx.lc)
#define RTPT_HALT(ctx) (ctx)->rtpt_ctx.lc = &&RTPT_EXIT_LABEL; return 0
#define RTPT_END(ctx) RTPT_EXIT_LABEL:; LC_END((ctx)->rtpt_ctx.lc); return 0
typedef struct {
    void *lc;
} RTPT_TaskContext;
#else
#define RTPT_DELAY(ctx, interval) (ctx)->rtpt_ctx.lc = __LINE__; return !(interval) ? 1 : (interval) / RTPT_TICK_MS; LC_SET((ctx)->rtpt_ctx.lc)
#define RTPT_HALT(ctx) (ctx)->rtpt_ctx.lc = INT16_MAX; return 0
#define RTPT_END(ctx) case INT16_MAX:; LC_END((ctx)->rtpt_ctx.lc); return 0
typedef struct {
    uint16_t lc;
} RTPT_TaskContext;
#endif

#define RTPT_BEGIN_TASK(name, type, ctx_name) int name(void *rtpt_param) { type *ctx_name = rtpt_param; RTPT_BEGIN(ctx_name);
#define RTPT_END_TASK(ctx) RTPT_END(ctx); }


typedef struct {
    int (*task_func)(void*);
    void *ctx;
    uint16_t next_tick;
    uint8_t skipped;
    uint8_t stopped;
} RTPT_Task;

extern volatile uint16_t RTPT_tick;  // replaced in ISR

int RTPT_Init();
int RTPT_CreateTask(void *ctx, int (*task_func)(void*));
int RTPT_CountActiveTasks();
int RTPT_FindTask(uint16_t tick);
void RTPT_MarkSkippedTasks(uint16_t start_tick, uint16_t end_tick);
void RTPT_StartTasks();


#endif  // _RTPT_H_
