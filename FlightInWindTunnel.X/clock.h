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
#include <stdbool.h>

/**
 * @file clock.h
 *
 * TIMER1 is configured as a clock for task schedule
 * It provide functions like providing time related information.
 */

#ifdef	__cplusplus
extern "C" {
#endif

    #define timestamp_size (8)
    typedef int64_t timestamp_t;

    extern timestamp_t volatile offset_us;

    timestamp_t getMicroseconds();

    uint32_t getMicrosecondsLSDW();

    void set_time_offset(timestamp_t microseconds);
    void setMicroseconds(timestamp_t microseconds);

    bool apply_time_offset(void);

    /**
     * Reset this global clock.
     */
    void initClock(void);

#ifdef	__cplusplus
}
#endif

#endif	/* CLOCK_H */

