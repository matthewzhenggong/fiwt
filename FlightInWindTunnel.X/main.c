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

#include "sending.h"
#include "getting.h"
#include "echoTask.h"
#include "idle.h"
#include "task.h"
#include "user.h"          /* User funct/params, such as InitApp              */
#include "system.h"        /* System funct/params, like osc/peripheral config */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

idleParam_t idle_params;
sendingParam_t sending;
gettingParam_t getting1;
gettingParam_t getting2;
//echoParam_t echo;

int main(void) {

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize IO ports, peripherals */
    InitApp();

    /* Init Task */
    TaskInit();

    /* Add Task */
    sendingInit(&sending, &Xbee1, &Xbee2);
    TaskCreate(sendingLoop, "BISEND", (void *)&sending, sendingPERIOD, sendingDELAY, sendingPRIORITY);

    //echoInit(&echo, &Serial3);
    //TaskCreate(echoLoop, "ECHO", (void *)&echo, echoPERIOD, echoDELAY, echoPRIORITY);

    gettingInit(&getting1, &Xbee1);
    TaskCreate(gettingLoop, "GETMSG1", (void *)&getting1, gettingPERIOD, gettingDELAY, gettingPRIORITY);
    gettingInit(&getting2, &Xbee2);
    TaskCreate(gettingLoop, "GETMSG2", (void *)&getting2, gettingPERIOD, gettingDELAY+2u, gettingPRIORITY);

    idleInit(&idle_params);
    TaskSetIdleHook(idleLoop, (void *)&idle_params);

    /**
     * Start the RTOS scheduler, this function should not return.
     */
    TaskStartScheduler();

    /* Should never get here! */
    return 0;
}
