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

/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include "config.h"
#include "system.h"          /* variables/params used by system.c             */

/* Device header file */
#include <xc.h>

/******************************************************************************/
/* System Level Functions                                                     */
/*                                                                            */
/* Custom oscillator configuration funtions, reset source evaluation          */
/* functions, and other non-peripheral microcontroller initialization         */
/* functions get placed in system.c.                                          */
/*                                                                            */
/******************************************************************************/

/* Refer to the device Family Reference Manual Oscillator section for
information about available oscillator configurations.  Typically
this would involve configuring the oscillator tuning register or clock
switching useing the compiler's __builtin_write_OSCCON functions.
Refer to the C Compiler for PIC24 MCUs and dsPIC DSCs User Guide in the
compiler installation directory /doc folder for documentation on the
__builtin functions.*/

void ConfigureOscillator(void)
{
#if STARTKITBOARD
    // Configure Oscillator to operate the device at 64Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 8M*64/(2*2)=128Mhz for 8M input clock
    PLLFBD = 62;                                // M=64
    CLKDIVbits.PLLPRE = 0;                      // N1=2
    CLKDIVbits.PLLPOST = 0;                     // N2=2

    OSCTUN = 0;                                 // Tune FRC oscillator, if FRC is used
    RCONbits.SWDTEN = 0;                        /* Disable Watch Dog Timer*/

    // Clock switch to incorporate PLL
    __builtin_write_OSCCONH( 0x03 );            // Initiate Clock Switch to Primary

    // Oscillator with PLL (NOSC=0b011)
    __builtin_write_OSCCONL( OSCCON || 0x01 );  // Start clock switching
    while( OSCCONbits.COSC != 0b011 );

    // Wait for Clock switch to occur
    while( OSCCONbits.LOCK != 1 )
    { };
#else
    // Configure Oscillator to operate the device at 64Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
#if NOT_USE_EXTOSC // Fosc= 7.4M*104/(3*2)=128.27Mhz for 7.4M input clock
    OSCTUNbits.TUN = 1                          // Fin = 7.4M
    PLLFBD = 102;                               // M=104
    CLKDIVbits.PLLPRE = 1;                      // N1=3
    CLKDIVbits.PLLPOST = 0;                     // N2=2
#else  // Fosc= 10M*128/(5*2)=128Mhz for 10M input clock
    PLLFBD = 126;                               // M=128
    CLKDIVbits.PLLPRE = 3;                      // N1=5
    CLKDIVbits.PLLPOST = 0;                     // N2=2
#endif
    asm ("repeat #6400;");Nop();
#endif
}

void DisableInterrupts(void) {
    SET_CPU_IPL(SCHEDULE_TIMER_PRIORITY_LEVEL);
}

void EnableInterrupts(void) {
    SET_CPU_IPL(0);
}

