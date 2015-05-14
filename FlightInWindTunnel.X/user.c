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
#include "ADC1.h"
#include "PWMx.h"
#include "Servo.h"
#include "Enc.h"
#include "IMU.h"

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
#if USE_UART1
SerialStream_t Serial1;
XBee_t Xbee1;
#endif /*USE_UART1*/
#if USE_UART2
SerialStream_t Serial2;
XBee_t Xbee2;
#endif /*USE_UART2*/
#if USE_UART3
SerialStream_t Serial3;
XBee_t Xbee3;
#endif /*USE_UART3*/
#if USE_UART4
SerialStream_t Serial4;
XBee_t Xbee4;
#endif /*USE_UART4*/

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
    
    /* Configure Nested Interrupts */
    INTCON1bits.NSTDIS = 0b0; // Interrupt nesting enabled

    initClock();

#if USE_UART1
    serial_assign(&Serial1, 1u);
    Serial1.init();
    Serial1.open();
    XBeeInit(&Xbee1, XBEE1_ATAP, &Serial1);
#endif /*USE_UART1*/
#if USE_UART2
    serial_assign(&Serial2, 2u);
    Serial2.init();
    Serial2.open();
    XBeeInit(&Xbee2, XBEE2_ATAP, &Serial2);
#endif /*USE_UART2*/
#if USE_UART3
    serial_assign(&Serial3, 3u);
    Serial3.init();
    Serial3.open();
    XBeeInit(&Xbee3, XBEE3_ATAP, &Serial3);
#endif /*USE_UART3*/
#if USE_UART4
    serial_assign(&Serial4, 4u);
    Serial4.init();
    Serial4.open();
    XBeeInit(&Xbee4, XBEE4_ATAP, &Serial4);
#endif /*USE_UART4*/

#if USE_ADC1
    ADC1Init();
    ADC1Start();
#endif

#if USE_ENC
    EncInit();
#endif

#if USE_PWM
    PWMxInit();
    PWMxStart();
#endif

#if USE_PWM && USE_ADC1
    ServoInit();
    asm ("repeat #640;");
    Nop();
    ServoStart();
#endif /* USE_PWM && USE_ADC1 */

#if USE_IMU
    IMUInit();
    IMUStart();
#endif

#if USE_SPIS
    SPISInit();
    SPISStart();
#endif
}
