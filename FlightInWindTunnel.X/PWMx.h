/*
 * File:   PWMx.h
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


#ifndef PWMX_H
#define	PWMX_H

#include "config.h"

#if USE_PWM

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * PWM Duty Circle Register
     */
#define PWM1DC PDC1
#define PWM2DC SDC2
#define PWM3DC SDC3
#define PWM5DC SDC5
#define PWM6DC SDC6
#define PWM7DC SDC7

    /**
     *  Config and initialize peripheral PWMx
     */
    void PWMxInit(void);

    /**
     * Start peripheral PWMx
     */
    void PWMxStart(void);

#ifdef	__cplusplus
}
#endif

#endif /* USE_PWM */

#endif	/* PWMX_H */

