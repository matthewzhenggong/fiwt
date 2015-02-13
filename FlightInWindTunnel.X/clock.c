/*
 * File:   clock.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Original author: Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
 * Some code is also from Microchip's DataSheet Documents.
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

#include "config.h"
#include "clock.h"

#include <xc.h>
#include <stdint.h>

/**
 * @file Real Time Clock for dsPIC
 *
 * Uses Timer1, TCY clock timer mode
 * and interrupt on period match
 */

__near clockType_t volatile RTclock;

__near unsigned int volatile elapsed_ticks = 0;

void resetClock(void) {
    elapsed_ticks = 0; /* clear software registers */
    RTclock.ticks = 0;
    RTclock.seconds = 0;

    T1CONbits.TON = 0; /* Disable Timer*/
    T1CONbits.TCS = 0; /* set internal clock source */
    T1CONbits.TGATE = 0; /* Disable Gated Timer mode */
    T1CONbits.TCKPS = 0b10; /* Select 1:64 Prescaler */

    TMR1 = 0; /* clear timer1 register */
    /** Timer1 period for 1 ms */
    PR1 = 1000; /* set period1 register */

    IPC0bits.T1IP = SCHEDULE_TIMER_PRIORITY_LEVEL; /* set priority level */
    IFS0bits.T1IF = 0; /* clear interrupt flag */
    IEC0bits.T1IE = 1; /* enable interrupts */

    T1CONbits.TON = 1; /* start the timer*/
}

__interrupt(no_auto_psv) void _T1Interrupt(void) {
    ++elapsed_ticks;
    if (++RTclock.ticks >= 1000) { /* increment ticks counter */
        /* if time to rollover */
        RTclock.ticks = 0u; /* clear seconds ticks */
        ++RTclock.seconds; /* and increment seconds */
    }

    IFS0bits.T1IF = 0; /* clear interrupt flag */
}

