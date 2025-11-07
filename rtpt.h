/*
 * Protothreads-based simple task scheduler. 
 */
#ifndef _RTPT_H_
#define _RTPT_H_


#include <stdint.h>
#include "lc.h"
#include "rtpt_config.h"


#define RTPT_BEGIN(ctx) LC_RESUME((ctx)->rtpt_ctx.lc)
#ifdef __LC_ADDRLABELS_H__
#define RTPT_DELAY(ctx, interval) do { (ctx)->rtpt_ctx.lc = &&LC_CONCAT(LC_LABEL,__LINE__); return !(interval) ? 1 : (interval) / RTPT_TICK_MS; LC_SET((ctx)->rtpt_ctx.lc); } while (0)
#define RTPT_EXIT(ctx) do { (ctx)->rtpt_ctx.lc = &&RTPT_EXIT_LABEL; return 0; } while (0)
#define RTPT_END(ctx) RTPT_EXIT_LABEL:; LC_END((ctx)->rtpt_ctx.lc); return 0
typedef struct {
    void *lc;
} RTPT_TaskContext;
#else
#define RTPT_DELAY(ctx, interval) do { (ctx)->rtpt_ctx.lc = __LINE__; return !(interval) ? 1 : (interval) / RTPT_TICK_MS; LC_SET((ctx)->rtpt_ctx.lc) } while (0)
#define RTPT_EXIT(ctx) do { (ctx)->rtpt_ctx.lc = INT16_MAX; return 0; } while (0)
#define RTPT_END(ctx) case INT16_MAX:; LC_END((ctx)->rtpt_ctx.lc); return 0
typedef struct {
    uint16_t lc;
} RTPT_TaskContext;
#endif

#define RTPT_BEGIN_TASK(name, type, ctx_name) uint16_t name(void *rtpt_param) { type *ctx_name = rtpt_param; RTPT_BEGIN(ctx_name);
#define RTPT_END_TASK(ctx) RTPT_END(ctx); }

enum RTPT_TaskState {
    RTPT_STATE_EXITED = 0b00000001,
    RTPT_STATE_SUSPENDED = 0b00000010,
    RTPT_STATE_STOPPED = 0b00000011,
    RTPT_STATE_SKIPPED = 0b00000100,
};

typedef struct {
    uint16_t (*task_func)(void*);
    void *ctx;
    uint16_t next_tick;
    uint8_t state;
} RTPT_Task;

typedef int8_t RTPT_TaskID;

extern volatile uint16_t RTPT_tick;  // replaced in ISR

int RTPT_Init();
RTPT_TaskID RTPT_CreateTask(void *ctx, uint16_t (*task_func)(void*));
#ifdef RTPT_INCLUDE_COUNT
int RTPT_CountActiveTasks();
#endif
#ifdef RTPT_INCLUDE_SUSPEND
int RTPT_SuspendTask(RTPT_TaskID task_id);
int RTPT_ResumeTask(RTPT_TaskID task_id);
#endif
#ifdef RTPT_INCLUDE_KILL
int RTPT_KillTask(RTPT_TaskID task_id);
void RTPT_KillTasks();
#endif
void RTPT_StartTasks();


#endif  // _RTPT_H_
