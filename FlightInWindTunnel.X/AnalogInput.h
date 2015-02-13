/*
 * File:   AnalogInput.h
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


#ifndef ANALOGINPUT_H
#define	ANALOGINPUT_H

#include "config.h"
#include "clock.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /* External variables declaration */
#if GNDBOARD
#if DEBUG == 0
    extern unsigned int ADC_Input[8];
#endif
#else
    extern unsigned int BattCell[3];
#if AC_MODEL
    extern unsigned int ServoPos[6];
#elif AEROCOMP
    extern unsigned int ServoPos[4];
#endif
#endif
    extern clockType_t AnalogInputTimeStamp;
    extern unsigned int AnalogInputTimeMicroSecStamp;

    void UpdateAnalogInputs(void);


#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGINPUT_H */

