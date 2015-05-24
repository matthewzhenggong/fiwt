/*
 * File:   system.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 *
 * This file is part of FIWT.
 *
 * FIWT is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 *  @file Time-Triggered Co-operative Task scheduler
 *
 *  @ref http://petevidler.com/2011/03/simple-co-operative-scheduling/
 *  @ref http://www.embedded.com/electronics-news/4434501/Writing-a-simple-cooperative-scheduler
 *  @ref http://github.com/edartuz/c-ptx
 */

#include "config.h"
#include "task.h"
#include "clock.h"
//#include "system.h"
#include "LedExtBoard.h"

#include <string.h>

static struct TaskHandle_t tasks[MAX_NUM_TASKS];
uint16_t TaskFlags;
unsigned TaskNumber = 0u;

TaskHandle_p TaskListHeader = NULL;
TaskHandle_p TaskFreeListHeader;

void TaskInit(void) {
    unsigned i;

    TaskFlags = 0u;
    TaskNumber = 0u;

    TaskListHeader = NULL;
    for (i = 1u, TaskFreeListHeader = tasks; i < MAX_NUM_TASKS; ++i, ++TaskFreeListHeader) {
        TaskFreeListHeader->delay = 0u;
        TaskFreeListHeader->load_max = 0u;
        TaskFreeListHeader->next = TaskFreeListHeader + 1u;
        TaskFreeListHeader->parameters = NULL;
        TaskFreeListHeader->period = 1u;
        TaskFreeListHeader->cur_status = 0;
        TaskFreeListHeader->task_func = NULL;
        TaskFreeListHeader->priority = NULL;
        TaskFreeListHeader->task_num = i;
        TaskFreeListHeader->task_name[0] = '\0';
    }
    TaskFreeListHeader->next = NULL;
    TaskFreeListHeader = tasks;
}

inline void insert2list(TaskHandle_p *task_list, TaskHandle_p t) {
    TaskHandle_p TaskListHeader;
    TaskHandle_p t2;

    TaskListHeader = *task_list;
    if (TaskListHeader) {
        if (t->priority >= TaskListHeader->priority) {
            /* insert as 1st one */
            t->next = TaskListHeader;
            TaskListHeader = t;
        } else {
            t2 = TaskListHeader;
            while (t2->next) {
                if (t->priority >= t2->next->priority) {
                    /* insert in middle pos */
                    t->next = t2->next;
                    t2->next = t;
                    break;
                }
                t2 = t2->next;
            }
            if (!t2->next) {
                /* append in tail */
                t2->next = t;
            }
        }
    } else {
        /* just as the head */
        TaskListHeader = t;
    }
    *task_list = TaskListHeader;
}

TaskHandle_p TaskCreate(TaskFunction_t task_func, const char * name, void * parameters,
        unsigned period, unsigned delay, unsigned priority) {
    TaskHandle_p t;
    if (TaskNumber < MAX_NUM_TASKS) {
        /* fetch one task node from the free-list*/
        t = TaskFreeListHeader;
        TaskFreeListHeader = TaskFreeListHeader->next;
        /* and reinitialize the node */
        t->task_func = task_func;
        strcpy(t->task_name, name);
        t->parameters = parameters;
        if (period < 1u) {
            period = 1u;
        }
        t->period = period;
        // Avoid underflow in the dispatcher.
        t->delay = delay;
        t->priority = priority;
        t->load_max = 0u;
        t->cur_status = 0;
        t->next = NULL;
        t->runtime_microseconds = 0u;
        t->runtime_cnt = 0u;

        /* insert the node to working-list */
        /* lengthen the working-list*/
        ++TaskNumber;
        insert2list(&TaskListHeader, t);

        return t;
    }
    return NULL;
}

void TaskStartScheduler(void) {
    TaskHandle_p task;
    unsigned int tick_in;
    uint32_t lswd;
    unsigned int usec, msec, last_msec;
    unsigned int msec_cnt, msec_cnt2, msec_cnt3;

    msec_cnt = 0;
    msec_cnt2 = 0;
    msec_cnt3 = 0;
    lswd = getMicrosecondsLSDW();
    last_msec = (lswd >> TASK_TIMESLICE_BITS);
    while (1) {
        //DisableInterrupts();
        lswd = getMicrosecondsLSDW();
        msec = (lswd >> TASK_TIMESLICE_BITS);
        usec = lswd & TASK_TIMESLICE_MASK;
        if (msec != last_msec) {
            last_msec = msec;
            /* loop working tasks */
            for (task = TaskListHeader; task; task = task->next) {
                if (task->period == 0 || ((msec & task->period) == task->delay)) {
                    if (task->cur_status < 2) {
                        tick_in = usec;
                        /* Task is waiting. Wake it up! */
                        //EnableInterrupts();
                        task->cur_status = task->task_func(task); /* Execute the task! */
                        //DisableInterrupts();
                        ++task->runtime_cnt;
                        lswd = getMicrosecondsLSDW();
                        msec = (lswd >> TASK_TIMESLICE_BITS);
                        usec = lswd & TASK_TIMESLICE_MASK;
                        if (msec == last_msec) {
                            tick_in = usec - tick_in;
                        } else if (msec > last_msec) {
                            tick_in = ((msec - last_msec) << TASK_TIMESLICE_BITS)
                                    + usec - tick_in;
                        } else {
                            tick_in = ((msec + (1 << (16 - TASK_TIMESLICE_BITS)) - last_msec) << TASK_TIMESLICE_BITS)
                                    + usec - tick_in;
                        }
                        task->runtime_microseconds += tick_in;
                        if (tick_in > task->load_max) {
                            task->load_max = tick_in;
                        }
                    } else {
                        // TODO free the task
                    }
                }
            }
            ++msec_cnt;
            ++msec_cnt2;
            ++msec_cnt3;
        } else if (msec_cnt >= 1000u) {
            msec_cnt = 0;
#if USE_LEDEXTBOARD
            mLED_1_Toggle();
            mLED_2_Off();
            mLED_3_Off();
            mLED_4_Off();
#endif
        } else if (msec_cnt2 >= 10) {
            msec_cnt2 = 0;
            ++offset_us;
        } else if (msec_cnt3 >= 289) {
            msec_cnt3 = 0;
            --offset_us;
        } else if (apply_time_offset()) {
            lswd = getMicrosecondsLSDW();
            last_msec = (lswd >> TASK_TIMESLICE_BITS);
        }
        //EnableInterrupts();
    }
}