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
#if USE_EKF
#include "ekfTask.h"
#endif
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
TaskHandle_p msgRecvTask;
msgSendParam_t msg_send;
#if USE_EKF
ekfParam_t ekf;
#endif
TaskHandle_p ekfTask;

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
    servoInit(&servo);
    servoTask = TaskCreate(servoLoop, "SERV", (void *) &servo, TASK_PERIOD, 1, 10);

    msgRecvInit(&msg_recv, &Xbee1, XBEE1_SERIES, servoTask, &msg_send);
    msgRecvTask = TaskCreate(msgRecvLoop, "MSGR", (void *) &msg_recv, TASK_PERIOD, 0, 10);

    senInit(&sen);
    senTask = TaskCreate(senLoop, "SENS", (void *) &sen, TASK_PERIOD, 1, 20);

    msgSendInit(&msg_send, &Xbee1,XBEE1_SERIES, msgRecvTask, senTask, servoTask, ekfTask);
    TaskCreate(msgSendLoop, "MSGS", (void *) &msg_send, TASK_PERIOD, 1, 5);
#if USE_EKF
    ekfInit(&ekf,TASK_PERIOD/1000.0f);
    ekfTask = TaskCreate(ekfLoop, "EKF", (void *) &ekf, TASK_PERIOD, 0, 0);
#else
    ekfTask = NULL;
#endif
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
