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
#include <stddef.h>

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
    {&ServoPos[0], &PWM1DC, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK},
    {&ServoPos[1], &PWM2DC, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK},
};

void ServoInit(void) {

}

void ServoStart(void) {
    size_t i;
    Servo_p servo;
    UpdateAnalogInputs();
    for (i = 0u, servo = Servos; i < SEVERONUM; ++i, ++servo) {
        servo->PrevPosition = *(servo->Position);
        servo->Reference = 2096;
        servo->butt1 = 0.0f;
        servo->butt2 = 0.0f;
        servo->Ctrl = 0;
    }
}

void MotorSet(unsigned int ch, signed int duty_circle) {
    Servo_p servo;
    servo = &Servos[ch];
    *(servo->lat_cw) &= servo->lat_cw_mask;
    *(servo->lat_ccw) &= servo->lat_ccw_mask;
    if (duty_circle == 0) {
        servo->Ctrl = 0;
    } else if (duty_circle > 0) {
        *(servo->lat_cw) |= servo->lat_cw_pos;
        if (duty_circle > PWM_PEROID) duty_circle = PWM_PEROID;
        servo->Ctrl = duty_circle;
    } else if (duty_circle < 0) {
        *(servo->lat_ccw) |= servo->lat_ccw_pos;
        if (duty_circle < -PWM_PEROID) duty_circle = -PWM_PEROID;
        servo->Ctrl = duty_circle;
        duty_circle = -duty_circle;
    }
    *(servo->DutyCycle) = duty_circle;
}

void ServoUpdate100Hz(unsigned int ch, unsigned int ref) {
    Servo_p servo;
    signed int duty_circle;
    signed int pos;
    signed int rate;
    float butt1;

    servo = Servos + ch;
    
    pos = *servo->Position;
    servo->Reference = ref;

    rate = (pos - servo->PrevPosition);
    if (rate > 41) rate = 41;
    else if (rate < -41) rate = -41;
    /** butterwolf filter */
    butt1 = servo->butt1;
    servo->butt1 = servo->butt1 * 0.2779 + servo->butt2*-0.4152 + rate * 0.5872;
    servo->butt2 = butt1 * 0.4152 + servo->butt2 * 0.8651 + rate * 0.1908;
    rate = servo->butt1 * 0.1468 + servo->butt2 * 0.6594 + rate * 0.0675;

//    ctrl = (15*PWM_PEROID/3.8f * 0.0007669904) * (servo->Reference - pos) /* Proportion */
//            + (15*0.04*PWM_PEROID/3.8*pi/4096*100) * (-rate); /* Difference */
    duty_circle = (19 * (servo->Reference - pos) >> 3) /* Proportion */
            + (19 * (-rate) >> 1); /* Difference */
    if (duty_circle > 0) {
        duty_circle += 250;
    } else {
        duty_circle -= 250;
    }

    *(servo->lat_cw) &= servo->lat_cw_mask;
    *(servo->lat_ccw) &= servo->lat_ccw_mask;
    if (duty_circle == 0) {
        servo->Ctrl = 0;
    } else if (duty_circle > 0) {
        *(servo->lat_cw) |= servo->lat_cw_pos;
        if (duty_circle > PWM_PEROID) duty_circle = PWM_PEROID;
        servo->Ctrl = duty_circle;
    } else if (duty_circle < 0) {
        *(servo->lat_ccw) |= servo->lat_ccw_pos;
        if (duty_circle < -PWM_PEROID) duty_circle = -PWM_PEROID;
        servo->Ctrl = duty_circle;
        duty_circle = -duty_circle;
    }
    *(servo->DutyCycle) = duty_circle;
    servo->PrevPosition = pos;
}

#endif /* USE_PWM && USE_ADC1 */
