/*
 * File:   Servo.c
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


#include "Servo.h"

#if USE_PWM && USE_ADC1

#include "PWMx.h"
#include "AnalogInput.h"
#include <xc.h>

/* Servos pin layout */
/************************************************************************
System      Servo       Analog Input    PMW Output  Directioning Pins
                                                        (IN_B / IN_A)
-------------------------------------------------------------------------
            Servo1		AN20        PMW1	LATJ4 	/ LATJ5
            Servo2		AN21        PMW2	LATJ6 	/ LATJ7
AC_MODEL    Servo3		AN5         PMW3	LATG14	/ LATG12
            Servo4		AN3         PMW7	LATJ10	/ LATJ11
            Servo5		AN2         PMW5	LATJ12	/ LATJ13
            Servo6		AN1         PMW6	LATG6 	/ LATG7
-------------------------------------------------------------------------
            Servo1		AN20        PMW1	LATJ4 	/ LATJ5
AEROCOMP    Servo2		AN21        PMW2	LATJ6 	/ LATJ7
            Servo3		AN3         PMW7	LATJ10	/ LATJ11
            Servo4		AN2         PMW5	LATJ12	/ LATJ13
 ************************************************************************/

Servo_t Servos[] = {
    {&ServoPos[0], 2048, 2048, 2048, &PWM1DC, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK, 0},
    {&ServoPos[1], 2048, 2048, 2048, &PWM2DC, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK, 0},
};

void ServoInit(void) {

}

void ServoStart(void) {

}

#define MotorSetImpl(idx) \
void MotorSet##idx(signed int duty_circle) { \
    *(Servos[idx].lat_cw) &= Servos[idx].lat_cw_mask; \
    *(Servos[idx].lat_ccw) &= Servos[idx].lat_ccw_mask; \
    if (duty_circle==0) { \
        Servos[idx].Ctrl = 0; \
    }else if (duty_circle>0) { \
        *(Servos[idx].lat_cw) |= Servos[idx].lat_cw_pos; \
        if (duty_circle > PWM_PEROID) duty_circle = PWM_PEROID; \
        Servos[idx].Ctrl = duty_circle; \
    }else if (duty_circle<0) { \
        *(Servos[idx].lat_ccw) |= Servos[idx].lat_ccw_pos; \
        if (duty_circle < -PWM_PEROID) duty_circle = -PWM_PEROID; \
        Servos[idx].Ctrl = duty_circle; \
        duty_circle = -duty_circle; \
    } \
    *(Servos[idx].DutyCycle) = duty_circle; \
}

MotorSetImpl(0);

MotorSetImpl(1);

#endif /* USE_PWM && USE_ADC1 */
