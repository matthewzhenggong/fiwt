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
//#include <xc.h>

#elif STARTKITBOARD
#include "IMU.h"
#endif
#include "msg_comm.h"
#include "senTask.h"

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
msgParamGND_t msgGND;
#endif
msgParam_t msg;


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
#if AC_MODEL
    senInit(&sen);
    msgACM.sen_Task = TaskCreate(senLoop, "SEN", (void *) &sen, 0x3, 0x2, 30);
    servoInit(&servo);
    msgACM.servo_Task = TaskCreate(servoLoop, "SVO", (void *) &servo, 0x3, 0x2, 20);

    msgInit(&msg, &Xbee1);
    msgACM.msg_Task = TaskCreate(msgLoop, "MSG", (void *) &msg, 0x0, 0x0, 0);
    msgACM.serov_param = &servo;
    msgRegistNTP(&msg,2);
    msgRegistACM(&msg, &msgACM,1);
#elif AEROCOMP
    senInit(&sen);
    msgACM.sen_Task = TaskCreate(senLoop, "SEN", (void *) &sen, 0x3, 0x3, 30);
    servoInit(&servo);
    msgACM.servo_Task = TaskCreate(servoLoop, "SVO", (void *) &servo, 0x3, 0x3, 20);

    msgInit(&msg, &Xbee1);
    msgACM.msg_Task = TaskCreate(msgLoop, "MSG", (void *) &msg, 0x0, 0x0, 0);
    msgACM.serov_param = &servo;
    msgRegistNTP(&msg,2);
    msgRegistACM(&msg, &msgACM,1);
#elif GNDBOARD
    senInit(&sen);
    msgGND.sen_Task = TaskCreate(senLoop, "SEN", (void *) &sen, 0x3, 0x1, 30);
    msgInit(&msg, &Xbee2);
    msgGND.msg_Task = TaskCreate(msgLoop, "MSG", (void *) &msg, 0x0, 0x0, 0);
    msgRegistNTP(&msg, 2);
    msgRegistGND(&msg, &msgGND, 1);
#elif STARTKITBOARD
    while (1) {
        asm ("repeat #4000;");
        Nop();
        IMUUpdate();
    }
#endif

    /**
     * Start the RTOS scheduler, this function should not return.
     */
    TaskStartScheduler();

    /* Should never get here! */
    return 0;
}
