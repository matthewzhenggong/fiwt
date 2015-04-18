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

#if AC_MODEL || AEROCOMP
#include "servoTask.h"
#include "senTask.h"
#include "ekfTask.h"
#include "msg_recv.h"
#include "msg_send.h"
#elif GNDBOARD
#include "msg_gnd.h"
#include <xc.h>
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


#if AC_MODEL || AEROCOMP
#define TASK_PERIOD (5u) // 200Hz
servoParam_t servo;
TaskHandle_p servoTask;
senParam_t sen;
TaskHandle_p senTask;
msgRecvParam_t msg_recv;
TaskHandle_p sendTask;
msgSendParam_t msg_send;
#if AC_MODEL
ekfParam_t ekf;
TaskHandle_p ekfTask;
#endif

#elif GNDBOARD
#define TASK_PERIOD (2u) // 500Hz
msgParam_t msg;

#endif

idleParam_t idle_params;

int main(void) {
    uint16_t i;

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    for (i = 0u; i < 10000u; ++i) {
        asm ("repeat #6400;");
        Nop();
    }

    /* Initialize IO ports, peripherals */
    InitApp();

    for (i = 0u; i < 5000u; ++i) {
        asm ("repeat #6400;");
        Nop();
    }

    /* Init Task */
    TaskInit();

    /* Add Task */
#if AC_MODEL || AEROCOMP
    senInit(&sen);
    senTask = TaskCreate(senLoop, "SENS", (void *) &sen, TASK_PERIOD, 0, 20);
    servoInit(&servo);
    servoTask = TaskCreate(servoLoop, "SERV", (void *) &servo, TASK_PERIOD, 0, 10);
    msgSendInit(&msg_send, &Xbee1);
    sendTask = TaskCreate(msgSendLoop, "MSGS", (void *) &msg_send, TASK_PERIOD*3, 0, 5);
#if AC_MODEL
    ekfInit(&ekf,TASK_PERIOD/1000.0f);
    ekfTask = TaskCreate(servoLoop, "EKF", (void *) &ekf, TASK_PERIOD, 0, 0);
#else
    ekfTask = NULL;
#endif
    msgRecvInit(&msg_recv, &Xbee2, senTask, servoTask, ekfTask, sendTask);
    TaskCreate(msgRecvLoop, "MSGR", (void *) &msg_recv, TASK_PERIOD, 0, 30);
#elif GNDBOARD
    msgInit(&msg, &Xbee1, &Xbee2, &Xbee3, &Xbee4);
    TaskCreate(msgLoop, "MSGC", (void *) &msg, TASK_PERIOD, 0, 10);
#elif STARTKITBOARD
    while (1) {
        asm ("repeat #4000;");
        Nop();
        IMUUpdate();
    }
#endif

    idleInit(&idle_params);
    TaskSetIdleHook(idleLoop, (void *) &idle_params);

    /**
     * Start the RTOS scheduler, this function should not return.
     */
    TaskStartScheduler();

    /* Should never get here! */
    return 0;
}
