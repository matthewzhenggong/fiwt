/*
 * File:   task.c
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

#ifndef TASK_H
#define	TASK_H

#include "config.h"
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

    struct TaskHandle_t;
    typedef struct TaskHandle_t * TaskHandle_p;
    typedef char (*TaskFunction_t)(TaskHandle_p);

    /**
     * Task Structure
     */
    struct TaskHandle_t {
        char task_name[TASK_NAME_MAX];
        unsigned task_num;
        TaskFunction_t task_func; /**< Pointer to the task function. */
        void * parameters; /**< Pointer to the parameters to be called with. */
        unsigned period; /**< Period to execute with in milliseconds. */
        unsigned delay; /**< Delay before first call. */
        uint16_t load_max; /**< maximum CPU occupation time in microseconds*/
        /**
         * Task Status
         * The value may be :
         *  PT_WAITING = 0
         *  PT_YIELDED = 1
         *  PT_EXITED  = 2
         *  PT_ENDED   = 3
         */
        int cur_status;
        unsigned priority; /**< priority in list */
        TaskHandle_p next; /**< pointer to next task */
        uint32_t runtime_microseconds;
        uint32_t runtime_cnt;
    };

    /** Header pointing to task list */
    extern TaskHandle_p TaskListHeader;

    /**
     * Task Flags
     * bit0 : OverRun Flag
     *
     */
    extern uint16_t TaskFlags;

    extern unsigned TaskNumber;

    /** Initialize task list */
    void TaskInit(void);

    /**
     * Create one task
     * @param task_func : Pointer to task function
     * @param parameters : Pointer to task parameters
     * @param period : Loop period in milliseconds(greater than zero)
     * @param delay : Call displacement in milliseconds
     * @param priority : priority in task list, greater means higher priority
     * @return Task pointer, NULL for failure
     * 
     * @note If peroid is one, the task will be called every millisecond.
     *
     * @note The return value of Task funtion may be one of followings :
     *  PT_WAITING = 0
     *  PT_YIELDED = 1
     *  PT_EXITED  = 2
     *  PT_ENDED   = 3
     */
    TaskHandle_p TaskCreate(TaskFunction_t task_func,
            const char * name,
            void * parameters,
            unsigned period,
            unsigned delay,
            unsigned priority
            );

    /**
     * Schedule tasks in main loop
     */
    void TaskStartScheduler(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TASK_H */

