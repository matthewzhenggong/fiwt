/*
 * File:   clock.h
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

#ifndef CLOCK_H
#define	CLOCK_H

#include <xc.h>
#include <stdint.h>

/**
 * @file clock.h
 *
 * TIMER1 is configured as a clock for task schedule
 * It provide functions like providing time related information.
 */

#ifdef	__cplusplus
extern "C" {
#endif

    /** Clock providing seconds and milliseconds */
    struct clockType {
        uint16_t TimeStampLSW; /**< Store 32-Bit Timer LSW */
        uint16_t TimeStampMSW; /**< Store 32-Bit Timer MSW */
        unsigned int milliseconds; /**< absolute time, seconds */
        unsigned int seconds; /**< absolute time, milliseconds */
    };
    typedef struct clockType clockType_t;

    /** The global clock instance */
    extern clockType_t volatile RTclock;

    /**
     * Tick instance
     *
     * @note It will be counted up every milliseond by timer interrupt.
     *        the task schedule will count it down.
     */
    extern unsigned int volatile elapsed_ticks;

    /**
     * Get clock in microseconds
     *
     */
    #define microsec_ticks (TMR1)

    /**
     * Reset this global clock.
     */
    void resetClock(void);

#ifdef	__cplusplus
}
#endif

#endif	/* CLOCK_H */

