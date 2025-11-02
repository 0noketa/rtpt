/*
 * Protothreads-based simple task scheduler. 
 */


#include <stdint.h>
#include "rtpt.h"

#if RTPT_TARGET == RTPT_TARGET_PTHREADS
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#elif RTPT_TARGET == RTPT_TARGET_POSIX
#include <unistd.h>
#include <time.h>
#elif RTPT_TARGET == RTPT_TARGET_WIN32
#include <windows.h>
#elif RTPT_TARGET == RTPT_TARGET_PIC_XC8
#include <xc.h>
#endif


static RTPT_Task RTPT_tasks[RTPT_MAX_TASKS];
static int RTPT_tasks_count;
volatile uint16_t RTPT_tick;  // replaced in ISR


int RTPT_Init() {
    RTPT_tasks_count = 0;
    RTPT_tick = 0;
    return 1;
}

int RTPT_CreateTask(void *ctx, int (*task_func)(void*)) {
    if (RTPT_tasks_count >= RTPT_MAX_TASKS) return 0;

    RTPT_Task *new_task = &RTPT_tasks[RTPT_tasks_count++];
    new_task->task_func = task_func;
    new_task->ctx = ctx;
    new_task->next_tick = 0;
    new_task->skipped = 1;
    new_task->stopped = 0;
    LC_INIT(((RTPT_TaskContext*)ctx)->lc);
    return 1;
}
int RTPT_CountActiveTasks() {
    int count = 0;
    for (int i = 0; i < RTPT_tasks_count; ++i) {
        count += !RTPT_tasks[i].stopped;
    }
    return count;
}
int RTPT_FindTask(uint16_t tick) {
    for (int i = 0; i < RTPT_tasks_count; ++i) {
        if (RTPT_tasks[i].stopped) continue;
        if (RTPT_tasks[i].skipped) return i;
        if (RTPT_tasks[i].next_tick == tick) return i;
    }
    return -1;
}
void RTPT_MarkSkippedTasks(uint16_t start_tick, uint16_t end_tick) {
    if (start_tick <= end_tick) {
        for (int i = 0; i < RTPT_tasks_count; ++i) {
            if (!RTPT_tasks[i].stopped
                    && RTPT_tasks[i].next_tick >= start_tick
                    && RTPT_tasks[i].next_tick < end_tick
            ) {
                RTPT_tasks[i].skipped = 1;
            }
        }
    } else {
        for (int i = 0; i < RTPT_tasks_count; ++i) {
            if (!RTPT_tasks[i].stopped
                    && (RTPT_tasks[i].next_tick >= start_tick
                        || RTPT_tasks[i].next_tick < end_tick)
            ) {
                RTPT_tasks[i].skipped = 1;
            }
        }
    }
}


// nanosleep const
#if RTPT_TARGET == RTPT_TARGET_POSIX || RTPT_TARGET == RTPT_TARGET_PTHREADS
    static const struct timespec nanosleep_tick = {
        .tv_sec = 0,
        .tv_nsec = RTPT_TICK_MS * 1000000L  // ms
    };
#endif

// async tick counter
#if RTPT_TARGET == RTPT_TARGET_PTHREADS
static pthread_t RTPT_isr_thread;

static void RTPT_isr_cleanup(void *_) {
}
static void *RTPT_isr(void *_) {
    pthread_cleanup_push(RTPT_isr_cleanup, NULL);

    while (1) {
        nanosleep(&nanosleep_tick, NULL);
        ++RTPT_tick;
    }

    pthread_cleanup_pop(1);
    return NULL;
}

static void on_exit_handler(int signum) {
    pthread_cancel(RTPT_isr_thread);
}
#elif RTPT_TARGET == RTPT_TARGET_PIC_XC8
static void __interrupt() RTPT_isr() {
    #if RTPT_CONFIG_PIC_XC8_TMR == 0
        if (T0IF) {
            TMR = 0;
            ++RTPT_tick;
        }
    #elif RTPT_CONFIG_PIC_XC8_TMR == 1
        if (T1IF) {
            TMR1H = 0;
            TMR1L = 0;
            ++RTPT_tick;
        }
    #endif
}
#endif

void RTPT_StartTasks() {
    RTPT_tick = 0;

    // setup async tick counter
    #if RTPT_TARGET == RTPT_TARGET_PTHREADS
        if (pthread_create(&RTPT_isr_thread, NULL, RTPT_isr, NULL) != 0) {
            return;
        }
        signal(SIGTERM, on_exit_handler);
    #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8
        #if RTPT_CONFIG_PIC_XC8_TMR == 0
            TMR = 0;
            T0IE = 1;
        #elif RTPT_CONFIG_PIC_XC8_TMR == 1
            TMR1H = 0;
            TMR1L = 0;
            T1IE = 1;
        #endif
    #endif

    while (RTPT_CountActiveTasks()) {
        uint16_t tick0 = RTPT_tick;
        int task_id = RTPT_FindTask(tick0);
        if (task_id == -1) {
            // sync tick counter
            #if RTPT_TARGET == RTPT_TARGET_POSIX
                nanosleep(&nanosleep_tick, NULL);
                ++RTPT_tick;
            #elif RTPT_TARGET == RTPT_TARGET_WIN32
                Sleep(0.001);
                ++RTPT_tick;
            #endif

            continue;
        }

        RTPT_Task *task = &RTPT_tasks[task_id];
        int interval = task->task_func(task->ctx);
        task->skipped = 0;
        task->stopped = (interval == 0);

        // sync tick counter
        #if RTPT_TARGET == RTPT_TARGET_POSIX
            nanosleep(&nanosleep_tick, NULL);
            ++RTPT_tick;
        #elif RTPT_TARGET == RTPT_TARGET_WIN32
            Sleep(0.001);
            ++RTPT_tick;
        #endif

        uint16_t tick1 = RTPT_tick;
        task->next_tick = tick1 + (uint16_t)interval;

        RTPT_MarkSkippedTasks(tick0, tick1);
    }

    // cleanup async tick counter
    #if RTPT_TARGET == RTPT_TARGET_PTHREADS
        pthread_cancel(RTPT_isr_thread);
    #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8
        #if RTPT_CONFIG_PIC_XC8_TMR == 0
            T0IE = 0;
        #elif RTPT_CONFIG_PIC_XC8_TMR == 1
            T1IE = 0;
        #endif
    #endif
}

