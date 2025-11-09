/*
 * Protothreads-based simple task scheduler. 
 */


#include <stdint.h>
#include "rtpt.h"

#if RTPT_TARGET == RTPT_TARGET_PTHREAD
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


// prescaler = 1:1, tick = RTPT_TICK_MS ms
#define RTPT_PIC_TICK (uint16_t)((RTPT_CONFIG_PIC_TMR_FREQ / 4.0) / 1000.0 * RTPT_TICK_MS)
#define RTPT_PIC_TMR_INIT_VAL (uint16_t)(0xFFFF - RTPT_PIC_TICK)
#define RTPT_PIC_TMR_INIT(high, low) do { low = RTPT_PIC_TMR_INIT_VAL & 0xFF; high = RTPT_PIC_TMR_INIT_VAL >> 8; } while (0)


static RTPT_Task RTPT_tasks[RTPT_MAX_TASKS];
static int8_t RTPT_tasks_count;
volatile uint16_t RTPT_tick;  // replaced in ISR


int_fast8_t RTPT_Init() {
    RTPT_tasks_count = 0;
    RTPT_tick = 0;
    return 1;
}

RTPT_TaskID RTPT_CreateTask(void *ctx, uint16_t (*task_func)(void*)) {
    if (RTPT_tasks_count >= RTPT_MAX_TASKS) return -1;

    RTPT_Task *new_task = &RTPT_tasks[RTPT_tasks_count];
    new_task->task_func = task_func;
    new_task->ctx = ctx;
    new_task->next_tick = 0;
    new_task->state = RTPT_STATE_SKIPPED;
    LC_INIT(((RTPT_TaskContext*)ctx)->lc);
    return (RTPT_TaskID)RTPT_tasks_count++;
}

#ifndef RTPT_INCLUDE_COUNT
static
#endif
int_fast8_t RTPT_CountActiveTasks() {
    #if RTPT_CONFIG_CONTINUOUS == 1
        return 1;
    #else
        int_fast8_t count = 0;
        for (int_fast8_t i = 0; i < RTPT_tasks_count; ++i) {
            count += (RTPT_tasks[i].state & RTPT_STATE_STOPPED) == 0;
        }
        return count;
    #endif
}

#ifdef RTPT_INCLUDE_SUSPEND
int_fast8_t RTPT_SuspendTask(RTPT_TaskID task_id) {
    if (task_id < 0 || task_id >= RTPT_tasks_count) return 0;

    RTPT_Task *task = &RTPT_tasks[task_id];
    if (task->state & RTPT_STATE_EXITED) return 0;
    if (task->state & RTPT_STATE_SUSPENDED) return 0;

    task->state |= RTPT_STATE_SUSPENDED;
    return 1;
}
int RTPT_ResumeTask(RTPT_TaskID task_id) {
    if (task_id < 0 || task_id >= RTPT_tasks_count) return 0;

    RTPT_Task *task = &RTPT_tasks[task_id];
    if (task->state & RTPT_STATE_EXITED) return 0;
    if (~task->state & RTPT_STATE_SUSPENDED) return 0;

    task->state ^= RTPT_STATE_SUSPENDED;
    task->state |= RTPT_STATE_SKIPPED;
    return 1;
}
#endif

#ifdef RTPT_INCLUDE_KILL
int_fast8_t RTPT_KillTask(RTPT_TaskID task_id) {
    if (task_id < 0 || task_id >= RTPT_tasks_count) return 0;

    RTPT_Task *task = &RTPT_tasks[task_id];
    if (task->state & RTPT_STATE_EXITED) return 0;

    task->state |= RTPT_STATE_EXITED;
    return 1;
}
void RTPT_KillTasks() {
    for (int i = 0; i < RTPT_tasks_count; ++i) {
        RTPT_tasks[i].state |= RTPT_STATE_EXITED;
    }
}
#endif

static RTPT_TaskID RTPT_FindNextTask(uint16_t tick) {
    for (int_fast8_t i = 0; i < RTPT_tasks_count; ++i) {
        RTPT_Task *task = &RTPT_tasks[i];
        if (task->state & RTPT_STATE_STOPPED) continue;
        if (task->state & RTPT_STATE_SKIPPED) {
            task->state ^= RTPT_STATE_SKIPPED;
            return i;
        }
        if (task->next_tick == tick) return i;
    }
    return -1;
}
static void RTPT_MarkSkippedTasks(uint16_t start_tick, uint16_t end_tick) {
    if (start_tick <= end_tick) {
        for (int_fast8_t i = 0; i < RTPT_tasks_count; ++i) {
            RTPT_Task *task = &RTPT_tasks[i];
            if ((~task->state & RTPT_STATE_STOPPED)
                    && task->next_tick >= start_tick
                    && task->next_tick < end_tick
            ) {
                task->state |= RTPT_STATE_SKIPPED;
            }
        }
    } else {
        for (int_fast8_t i = 0; i < RTPT_tasks_count; ++i) {
            RTPT_Task *task = &RTPT_tasks[i];
            if (~task->state & RTPT_STATE_STOPPED
                    && (task->next_tick >= start_tick
                        || task->next_tick < end_tick)
            ) {
                task->state |= RTPT_STATE_SKIPPED;
            }
        }
    }
}


// nanosleep const
#if RTPT_TARGET == RTPT_TARGET_POSIX || RTPT_TARGET == RTPT_TARGET_PTHREAD
    static const struct timespec nanosleep_tick = {
        .tv_sec = 0,
        .tv_nsec = RTPT_TICK_MS * 1000000L  // ms
    };
#endif

// async tick counter
#if RTPT_TARGET == RTPT_TARGET_PTHREAD
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
    #if RTPT_CONFIG_PIC_TMR == 10 ||  RTPT_CONFIG_PIC_TMR == 11 || RTPT_CONFIG_PIC_INT == 0
        static uint8_t timer_high;  // higher 8-bit for 8-bit timers
    #endif
    #if RTPT_CONFIG_PIC_INT == 1
        static void __interrupt() RTPT_isr() {
            #if RTPT_CONFIG_PIC_TMR == 0
                if (TMR0IF) {
                    RTPT_PIC_TMR_INIT(TMR0H, TMR0L);
                    TMR0IF = 0;
                    ++RTPT_tick;
                }
            #elif RTPT_CONFIG_PIC_TMR == 10
                if (TMR0IF) {
                    if (++timer_high == 0) {
                        RTPT_PIC_TMR_INIT(timer_high, TMR0);
                        ++RTPT_tick;
                    }
                    TMR0IF = 0;
                }
            #elif RTPT_CONFIG_PIC_TMR == 1
                if (TMR1IF) {
                    RTPT_PIC_TMR_INIT(TMR1H, TMR1L);
                    TMR1IF = 0;
                    ++RTPT_tick;
                }
            #elif RTPT_CONFIG_PIC_TMR == 3
                if (TMR3IF) {
                    RTPT_PIC_TMR_INIT(TMR3H, TMR3L);
                    TMR3IF = 0;
                    ++RTPT_tick;
                }
            #else
                #error "RTPT_CONFIG_PIC_XC8_TMR"
            #endif
        }
    #endif
#endif

void RTPT_StartTasks() {
    RTPT_tick = 0;

    // setup async tick counter
    #if RTPT_TARGET == RTPT_TARGET_PTHREAD
        if (pthread_create(&RTPT_isr_thread, NULL, RTPT_isr, NULL) != 0) {
            return;
        }
        signal(SIGTERM, on_exit_handler);
    #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8 && RTPT_CONFIG_PIC_INT == 1
        #if RTPT_CONFIG_PIC_TMR == 0
            #ifdef T08BIT_bit
                T08BIT = 0;  // 16-bit timer0
            #endif
            T0CS = 0;  // clock source
            #ifdef PSA_bit
                PSA = 1;  // prescaler 0:on/1:off
            #endif
            #ifdef T0PS_bit
                T0PS = 0;  // 1:2
            #endif
            #ifdef TMR0ON_bit
                TMR0ON = 0;
            #endif
            RTPT_PIC_TMR_INIT(TMR0H, TMR0L);
            TMR0IE = 1;  // overflow interrupt
            #ifdef TMR0ON_bit
                TMR0ON = 1;
            #endif
        #elif RTPT_CONFIG_PIC_TMR == 10
            #ifdef T08BIT_bit
                T08BIT = 1;  // 8-bit timer0
            #endif
            T0CS = 0;  // clock source
            #ifdef PSA_bit
                PSA = 1;  // prescaler 0:on/1:off
            #endif
            #ifdef T0PS_bit
                T0PS = 0;  // 1:2
            #endif
            #ifdef TMR0ON_bit
                TMR0ON = 0;
            #endif
            RTPT_PIC_TMR_INIT(timer_high, TMR0);
            TMR0IE = 1;  // overflow interrupt
            #ifdef TMR0ON_bit
                TMR0ON = 1;
            #endif
        #elif RTPT_CONFIG_PIC_TMR == 1
            #ifdef RD16_bit
                T1CONbits.RD16 = 1;  // 16-bit rw
            #endif
            #ifdef T1CKPS_bit
                T1CKPS = 0;  // 1:1
            #endif
            T1OSCEN = 1;
            RTPT_PIC_TMR_INIT(TMR1H, TMR1L);
            TMR1IE = 1;  // overflow interrupt
            TMR1ON = 1;
        #elif RTPT_CONFIG_PIC_TMR == 3
            T3CONBits.RD16 = 1;  // 16-bit rw
            T3CKPS = 0;  // 1:1
            T3CS = 0;
            RTPT_PIC_TMR_INIT(TMR3H, TMR3L);
            TMR3IE = 1;  // overflow interrupt
            TMR3ON = 1;
        #endif
    #endif

    while (RTPT_CountActiveTasks()) {
        uint16_t tick0 = RTPT_tick;
        RTPT_TaskID task_id = RTPT_FindNextTask(tick0);
        if (task_id == -1) {
            // sync tick counter
            #if RTPT_TARGET == RTPT_TARGET_POSIX
                nanosleep(&nanosleep_tick, NULL);
                ++RTPT_tick;
            #elif RTPT_TARGET == RTPT_TARGET_WIN32
                Sleep(RTPT_TICK_MS);
                ++RTPT_tick;
            #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8 && RTPT_CONFIG_PIC_INT == 0
                if (!TMR0) {
                    if (++timer_high == 0) {
                        RTPT_PIC_TMR_INIT(timer_high, TMR0);
                        ++RTPT_tick;
                    }
                }
            #endif

            continue;
        }

        RTPT_Task *task = &RTPT_tasks[task_id];
        uint16_t interval = task->task_func(task->ctx);
        if (interval == 0) task->state |= RTPT_STATE_EXITED;

        // sync tick counter
        #if RTPT_TARGET == RTPT_TARGET_POSIX
            nanosleep(&nanosleep_tick, NULL);
            ++RTPT_tick;
        #elif RTPT_TARGET == RTPT_TARGET_WIN32
            Sleep(RTPT_TICK_MS);
            ++RTPT_tick;
        #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8 && RTPT_CONFIG_PIC_INT == 0
            if (!TMR0) {
                if (++timer_high == 0) {
                    RTPT_PIC_TMR_INIT(timer_high, TMR0);
                    ++RTPT_tick;
                }
            }
        #endif

        uint16_t tick1 = RTPT_tick;
        RTPT_MarkSkippedTasks(tick0, tick1);
        task->next_tick = tick1 + interval;
        task->state &= ~RTPT_STATE_SKIPPED;
    }

    // cleanup async tick counter
    #if RTPT_TARGET == RTPT_TARGET_PTHREAD
        pthread_cancel(RTPT_isr_thread);
    #elif RTPT_TARGET == RTPT_TARGET_PIC_XC8 && RTPT_CONFIG_PIC_INT == 1
        #if RTPT_CONFIG_PIC_TMR == 0 || RTPT_CONFIG_PIC_TMR == 10
            TMR0IE = 0;
            #ifdef TMR0ON_bit
                TMR0ON = 0;
            #endif
        #elif RTPT_CONFIG_PIC_TMR == 1
            TMR1IE = 0;
            TMR1ON = 0;
        #elif RTPT_CONFIG_PIC_TMR == 3
            TMR3IE = 0;
            TMR3ON = 0;
        #endif
    #endif
}

