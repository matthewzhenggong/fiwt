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

__near unsigned int volatile elapsed_ticks = 0;
unsigned int volatile elapsed_hours = 0;
__near int32_t volatile offset_us = 0;

void initClock(void) {
    elapsed_ticks = 0; /* clear software registers */
    elapsed_hours = 0;
    offset_us = 0;

    T1CONbits.TON = 0; /* Disable Timer*/
    T1CONbits.TCS = 0; /* set internal clock source */
    T1CONbits.TGATE = 0; /* Disable Gated Timer mode */
    T1CONbits.TCKPS = 0b10; /* Select 1:64 Prescaler */

    T3CONbits.TON = 0; /* Disable Timer*/
    T2CONbits.TON = 0; /* Disable Timer*/
    T2CONbits.T32 = 1; /* Enable 32-bit Timer mode*/
    T2CONbits.TCS = 0; /* set internal clock source */
    T2CONbits.TGATE = 0; /* Disable Gated Timer mode */
    T2CONbits.TCKPS = 0b10; /* Select 1:64 Prescaler */

    TMR1 = 0; /* clear timer1 register */
    TMR2 = 0; /* clear timer2 register */
    TMR3 = 0; /* clear timer3 register */
    /** Timer1 period for 1 ms */
    PR1 = 999; /* set period1 register */

    /** Timer1 period for 1 ms */
    PR3 = 0xD693; /* set period3 register, MSW */
    PR2 = 0xA3FF; /* set period2 register, LSW */

    _T1IP = SCHEDULE_TIMER_PRIORITY_LEVEL; /* set priority level */
    _T1IF = 0; /* clear interrupt flag */
    _T1IE = 1; /* enable interrupts */

    _T3IP = SCHEDULE_TIMER_PRIORITY_LEVEL; /* set priority level */
    _T3IF = 0; /* clear interrupt flag */
    _T3IE = 1; /* enable interrupts */

    T2CONbits.TON = 1; /* start the timer*/
    T1CONbits.TON = 1; /* start the timer*/
}

__interrupt(no_auto_psv) void _T1Interrupt(void) {
    IFS0bits.T1IF = 0; /* clear interrupt flag */
    ++elapsed_ticks;
}

__interrupt(no_auto_psv) void _T3Interrupt(void) {
    IFS0bits.T3IF = 0; /* clear interrupt flag */
    ++elapsed_hours;
}

uint32_t getMicroseconds() {
    uint32_t lsw, msw;
    lsw = TMR2;
    msw = TMR3HLD;
    return (msw<<16)+lsw+offset_us;
}

void setMicroseconds(uint32_t microseconds) {
    uint32_t lsw, msw;
    lsw = TMR2;
    msw = TMR3HLD;
    offset_us = microseconds - ((msw<<16)+lsw);
}
