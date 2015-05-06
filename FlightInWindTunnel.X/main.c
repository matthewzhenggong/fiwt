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
#include "msg_acm.h"
#include "servoTask.h"

#elif GNDBOARD
#include "msg_gnd.h"
#include "remoteSenTask.h"
#include <xc.h>

#elif STARTKITBOARD
#include "IMU.h"
#endif
#include "msg_comm.h"
#include "senTask.h"

#include "idle.h"
#include "task.h"
#include "user.h"          /* User funct/params, such as InitApp              */
#include "system.h"        /* System funct/params, like osc/peripheral config */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/


#if AC_MODEL || AEROCOMP
servoParam_t servo;
senParam_t sen;
msgParamACM_t msgACM;
#elif GNDBOARD
senParam_t sen;
remoteSenParam_t remoteSen;
msgParamGND_t msgGND;
#endif
msgParam_t msg;

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
    msgACM.servo_Task = TaskCreate(servoLoop, "SVO", (void *) &servo, TASK_PERIOD, 0, 20);

    senInit(&sen);
    msgACM.sen_Task = TaskCreate(senLoop, "SEN", (void *) &sen, 1, 0, 30);

    msgInit(&msg, &Xbee1);
    msgACM.msg_Task = TaskCreate(msgLoop, "MSG", (void *) &msg, 1, 0, 0);
    msgACM.serov_param = &servo;
    msgRegistNTP(&msg);
    msgRegistACM(&msg, &msgACM);
#elif GNDBOARD
    senInit(&sen);
    msgGND.sen_Task = TaskCreate(senLoop, "SEN", (void *) &sen, TASK_PERIOD, 0, 30);
    remoteSenInit(&remoteSen, &Xbee3);
    msgGND.rsen_Task = TaskCreate(remoteSenLoop, "MANIMET", (void *) &remoteSen, 100, 0, 10);
    msgInit(&msg, &Xbee2);
    msgGND.msg_Task = TaskCreate(msgLoop, "MSG", (void *) &msg, 1, 0, 0);
    msgRegistNTP(&msg);
    msgRegistGND(&msg, &msgGND);
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
