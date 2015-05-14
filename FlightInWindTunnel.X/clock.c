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

__near timestamp_t volatile offset_us = 0;
__near uint32_t volatile clock_us_MSW = 0;

bool _offset_set_req;
timestamp_t _offset_us_req;

void initClock(void) {
    offset_us = 0;
    clock_us_MSW = 0;
    _offset_set_req = false;

    T3CONbits.TON = 0; /* Disable Timer*/
    T2CONbits.TON = 0; /* Disable Timer*/
    T2CONbits.T32 = 1; /* Enable 32-bit Timer mode*/
    T2CONbits.TCS = 0; /* set internal clock source */
    T2CONbits.TGATE = 0; /* Disable Gated Timer mode */
    T2CONbits.TCKPS = 0b10; /* Select 1:64 Prescaler */

    TMR2 = 0; /* clear timer2 register */
    TMR3 = 0; /* clear timer3 register */

    /** Timer1 period for 1 ms */
    PR3 = 0xFFFF; /* set period3 register, MSW */
    PR2 = 0xFFFF; /* set period2 register, LSW */

    _T3IP = SCHEDULE_TIMER_PRIORITY_LEVEL; /* set priority level */
    _T3IF = 0; /* clear interrupt flag */
    _T3IE = 1; /* enable interrupts */

    T2CONbits.TON = 1; /* start the timer*/
}

__interrupt(no_auto_psv) void _T3Interrupt(void) {
    IFS0bits.T3IF = 0; /* clear interrupt flag */
    ++clock_us_MSW;
}

timestamp_t getMicroseconds() {
    timestamp_t tick_us;
    *((uint16_t*)(&tick_us)) = TMR2;
    *((uint16_t*)(&tick_us)+1) = TMR3HLD;
    *((uint32_t*)(&tick_us)+1) = clock_us_MSW;
    return tick_us+offset_us;
}

uint32_t getMicrosecondsLSDW() {
    uint32_t tick_us;
    *((uint16_t*)(&tick_us)) = TMR2;
    *((uint16_t*)(&tick_us)+1) = TMR3HLD;
    return tick_us+*((uint32_t*)(&offset_us));
}

void set_time_offset(timestamp_t microseconds) {
    _offset_us_req = microseconds + offset_us;
    _offset_set_req = true;
}

void setMicroseconds(timestamp_t microseconds) {
    timestamp_t tick_us;
    *((uint16_t*)(&tick_us)) = TMR2;
    *((uint16_t*)(&tick_us)+1) = TMR3HLD;
    *((uint32_t*)(&tick_us)+1) = clock_us_MSW;
    _offset_us_req = microseconds - tick_us;
    _offset_set_req = true;
}

bool apply_time_offset(void) {
    if (_offset_set_req) {
        _offset_set_req = false;
        offset_us = _offset_us_req;
        return true;
    }
    return false;
}
