/*
 * File:   main.c
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

/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include "config.h"

#if AC_MODEL
#include "servoTask.h"
#include "senTask.h"
#include "msg_ac.h"
#elif AEROCOMP
#include "servoTask.h"
#include "senTask.h"
#include "msg_comp.h"
#elif GNDBOARD
#include "msg_gnd.h"
#elif STARTKITBOARD
#include "IMU.h"
#endif


#include "idle.h"
#include "task.h"
#include "user.h"          /* User funct/params, such as InitApp              */
#include "system.h"        /* System funct/params, like osc/peripheral config */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


#if AC_MODEL
#define TASK_PERIOD (10u) // 100Hz
servoParam_t servo;
TaskHandle_p servoTask;
senParam_t sen;
TaskHandle_p senTask;
msgParam_t msg;

#elif AEROCOMP
#define TASK_PERIOD (10u) // 100Hz
servoParam_t servo;
TaskHandle_p servoTask;
senParam_t sen;
TaskHandle_p senTask;
msgParam_t msg;

#elif GNDBOARD
#define TASK_PERIOD (5u) // 200Hz
msgParam_t msg;

#endif

idleParam_t idle_params;

int main(void) {

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize IO ports, peripherals */
    InitApp();

    /* Init Task */
    TaskInit();

    /* Add Task */
#if AC_MODEL
    senInit(&sen);
    senTask = TaskCreate(senLoop, "SENS", (void *)&sen, TASK_PERIOD, 0, 20);
    servoInit(&servo);
    servoTask = TaskCreate(servoLoop, "SERV", (void *)&servo, TASK_PERIOD, 0, 10);
    msgInit(&msg, &Xbee1, &Xbee2, servoTask, senTask);
    TaskCreate(msgLoop, "MSGA", (void *)&msg, TASK_PERIOD, 3, 10);
#elif AEROCOMP
    senInit(&sen);
    senTask = TaskCreate(senLoop, "SENS", (void *)&sen, TASK_PERIOD, 0, 20);
    servoInit(&servo);
    servoTask = TaskCreate(servoLoop, "SERV", (void *)&servo, TASK_PERIOD, 0, 10);
    msgInit(&msg, &Xbee1, &Xbee2, servoTask, senTask);
    TaskCreate(msgLoop, "MSGC", (void *)&msg, TASK_PERIOD, 3, 10);
#elif GNDBOARD
    msgInit(&msg, &Xbee1, &Xbee2, &Xbee3, &Xbee4);
    TaskCreate(msgLoop, "MSGC", (void *)&msg, TASK_PERIOD, 3, 10);
#elif STARTKITBOARD
    while (1) {
        asm ("repeat #4000;"); Nop();
       IMUUpdate();
    }
#endif

    idleInit(&idle_params);
    TaskSetIdleHook(idleLoop, (void *)&idle_params);

    /**
     * Start the RTOS scheduler, this function should not return.
     */
    TaskStartScheduler();

    /* Should never get here! */
    return 0;
}
