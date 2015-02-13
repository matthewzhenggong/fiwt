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

#include "AnalogInput.h"


/* External variables declaration */
#if GNDBOARD
#if DEBUG == 0
extern unsigned int ADC_Input[8];
#endif
#else
__near unsigned int BattCell[3];
#if AC_MODEL
__near unsigned int ServoPos[6];
#elif AEROCOMP
__near unsigned int ServoPos[4];
#endif
#endif

__near clockType_t AnalogInputTimeStamp;
__near unsigned int AnalogInputTimeMicroSecStamp;

void UpdateAnalogInputs(void) {

}