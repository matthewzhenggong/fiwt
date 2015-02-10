/*
 * File:   user.c
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
#include "user.h"            /* variables/params used by user.c               */
#include "clock.h"

/* Device header file */
#include <xc.h>

#include <stdint.h>          /* For uint16_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */

#if STARTKITBOARD
#include "StartKit.h"
#endif

/******************************************************************************/
/* Global Instances                                                             */
/******************************************************************************/

__near SerialStream_t Xbee2Serial;
__near SerialStream_t Xbee1Serial;
__near XBee_t Xbee1;
__near XBee_t Xbee2;

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

void InitApp(void) {
    /* Send Low Level Voltage to all ports */
    LATA = 0x0000;
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;
    LATE = 0x0000;
    LATF = 0x0000;
    LATG = 0x0000;
#if defined(__dsPIC33EP512MU814__)
    LATH = 0x0000;
    LATJ = 0x0000;
    LATK = 0x0000;
#endif

    /* Disable all analog inputs */
    ANSELA = 0x0000;
    ANSELB = 0x0000;
    ANSELC = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;

    /* Configure all digital ports as outputs */
    TRISA = 0x0000;
    TRISB = 0x0000;
    TRISC = 0x0000;
    TRISD = 0x0000;
    TRISE = 0x0000;
    TRISF = 0x0000;
    TRISG = 0x0000;
#if defined(__dsPIC33EP512MU814__)
    TRISH = 0x0000;
    TRISJ = 0x0000;
    TRISK = 0x0000;
#endif

#if STARTKITBOARD
    mInitAllLEDs();
    mInitAllSwitches();
#endif

    resetClock();

    serial_assign(&Xbee1Serial, 1u);
    Xbee1Serial.init();
    Xbee1Serial.open();
    XBeeInit(&Xbee1, &Xbee1Serial);

    serial_assign(&Xbee2Serial, 2u);
    Xbee2Serial.init();
    Xbee2Serial.open();
    XBeeInit(&Xbee2, &Xbee2Serial);
}
