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

#include "config_servo.h"
#include "config_pwm.h"

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

//Position;DutyCycle;lat_cw;lat_cw_mask;lat_cw_pos;lat_ccw;lat_ccw_mask;lat_ccw_pos;MaxPosition;MinPosition;
//Kp;ShiftKp;Kd;ShiftKd;shake;

Servo_t Servos[] = {
#if AC_MODEL
#if USE_TESTBOARD
    {&ServoPos[0], &PWM1DC, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[1], &PWM2DC, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[2], &PWM3DC, &LATG, _LATG_LATG12_MASK, ~_LATG_LATG12_MASK, &LATG, _LATG_LATG14_MASK, ~_LATG_LATG14_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[3], &PWM7DC, &LATJ, _LATJ_LATJ11_MASK, ~_LATJ_LATJ11_MASK, &LATJ, _LATJ_LATJ10_MASK, ~_LATJ_LATJ10_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[4], &PWM5DC, &LATJ, _LATJ_LATJ13_MASK, ~_LATJ_LATJ13_MASK, &LATJ, _LATJ_LATJ12_MASK, ~_LATJ_LATJ12_MASK, 3800, 200,
            10, 3, 11, 0,440},
    {&ServoPos[5], &PWM6DC, &LATG, _LATG_LATG7_MASK, ~_LATG_LATG7_MASK, &LATG, _LATG_LATG6_MASK, ~_LATG_LATG6_MASK, 3800, 200,
            10, 3, 11, 0,455},
#else
    {&ServoPos[0], &PWM1DC, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[1], &PWM2DC, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK, 3800, 200,
            10, 3, 11, 0,520},
    {&ServoPos[2], &PWM3DC, &LATG, _LATG_LATG14_MASK, ~_LATG_LATG14_MASK, &LATG, _LATG_LATG12_MASK, ~_LATG_LATG12_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[3], &PWM7DC, &LATJ, _LATJ_LATJ10_MASK, ~_LATJ_LATJ10_MASK, &LATJ, _LATJ_LATJ11_MASK, ~_LATJ_LATJ11_MASK, 3800, 200,
            10, 3, 11, 0,440},
    {&ServoPos[4], &PWM5DC, &LATJ, _LATJ_LATJ12_MASK, ~_LATJ_LATJ12_MASK, &LATJ, _LATJ_LATJ13_MASK, ~_LATJ_LATJ13_MASK, 3800, 200,
            10, 3, 11, 0,440},
    {&ServoPos[5], &PWM6DC, &LATG, _LATG_LATG6_MASK, ~_LATG_LATG6_MASK, &LATG, _LATG_LATG7_MASK, ~_LATG_LATG7_MASK, 3800, 200,
            10, 3, 11, 0,465},
#endif
#elif AEROCOMP
#if USE_TESTBOARD
    {&ServoPos[0], &PWM1DC, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[1], &PWM2DC, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[2], &PWM3DC, &LATG, _LATG_LATG12_MASK, ~_LATG_LATG12_MASK, &LATG, _LATG_LATG14_MASK, ~_LATG_LATG14_MASK, 3800, 200,
            10, 3, 11, 0,455},
    {&ServoPos[3], &PWM7DC, &LATJ, _LATJ_LATJ11_MASK, ~_LATJ_LATJ11_MASK, &LATJ, _LATJ_LATJ10_MASK, ~_LATJ_LATJ10_MASK, 3800, 200,
            10, 3, 11, 0,455},
#else
    {&EncPos[0], &PWM1DC, &LATJ, _LATJ_LATJ5_MASK, ~_LATJ_LATJ5_MASK, &LATJ, _LATJ_LATJ4_MASK, ~_LATJ_LATJ4_MASK, 3000, 1100,
            19, 3, 20, 0,343},
    {&EncPos[1], &PWM2DC, &LATJ, _LATJ_LATJ7_MASK, ~_LATJ_LATJ7_MASK, &LATJ, _LATJ_LATJ6_MASK, ~_LATJ_LATJ6_MASK, 3000, 1100,
            19, 3, 20, 0,343},
    {&EncPos[2], &PWM7DC, &LATJ, _LATJ_LATJ11_MASK, ~_LATJ_LATJ11_MASK, &LATJ, _LATJ_LATJ10_MASK, ~_LATJ_LATJ10_MASK, 3000, 1100,
            19, 3, 20, 0,343},
    {&EncPos[3], &PWM5DC, &LATJ, _LATJ_LATJ13_MASK, ~_LATJ_LATJ13_MASK, &LATJ, _LATJ_LATJ12_MASK, ~_LATJ_LATJ12_MASK, 3000, 1100,
            19, 3, 20, 0,343},
#endif
#endif
};

/** two-order butterwolf filter 10Hz */
#define BUTTER_ORDER (2)
//    0.6012  -0.2536 0.3586
//    0.2536   0.9598 0.0568
//    0.08966  0.6929 0.02008
// x 2^15 = 
static fractional _butter_mat_frac[] = { \
     19700, -8310, 11752,
      8310, 31452,  1861,
      2938, 22705,   658,
};

static fractional _butter_update(fractional input, fractional butt[BUTTER_ORDER + 1]) {
    fractional dstM[BUTTER_ORDER];
    int i;

    butt[BUTTER_ORDER] = input;
    MatrixMultiply(BUTTER_ORDER, BUTTER_ORDER + 1, 1, dstM, _butter_mat_frac, butt);
    for (i = 0; i < BUTTER_ORDER; ++i) {
        butt[i] = dstM[i];
    }
    MatrixMultiply(1, BUTTER_ORDER + 1, 1, dstM, _butter_mat_frac + BUTTER_ORDER * (BUTTER_ORDER + 1), butt);
    return dstM[0];
}

void ServoInit(void) {

}

void ServoStart(void) {
    size_t i;
    Servo_p servo;
    UpdateAnalogInputs();
    for (i = 0u, servo = Servos; i < SEVERONUM; ++i, ++servo) {
        servo->PrevPosition = *(servo->Position);
        servo->PrevRate = 0;
        servo->Reference = 2096;
        servo->butt[0] = 0;
        servo->butt[1] = 0;
        servo->butt[2] = 0;
        servo->Ctrl = 0;
        servo->SERVO_DIFF_LMT = ((0x8000)/(servo->Kp+1));
    }
}

static void _motor_set(Servo_p servo, signed int duty_circle) {
    *(servo->lat_cw) |= servo->lat_cw_mask;
    *(servo->lat_ccw) |= servo->lat_ccw_mask;
    if (duty_circle == 0) {
        servo->Ctrl = 0;
    } else if (duty_circle > 0) {
        *(servo->lat_cw) &= servo->lat_cw_pos;
        if (duty_circle > PWM_PEROID) duty_circle = PWM_PEROID;
        servo->Ctrl = duty_circle;
    } else if (duty_circle < 0) {
        *(servo->lat_ccw) &= servo->lat_ccw_pos;
        if (duty_circle < -PWM_PEROID) duty_circle = -PWM_PEROID;
        servo->Ctrl = duty_circle;
        duty_circle = -duty_circle;
    }
    *(servo->DutyCycle) = duty_circle;
}

void MotorSet(unsigned int ch, signed int duty_circle) {
    Servo_p servo;
    if (ch < SEVERONUM) {
        servo = Servos + ch;
        _motor_set(servo, duty_circle);
    }
}

void ServoUpdate(unsigned int ch, unsigned int ref) {
    Servo_p servo;
    signed int duty_circle;
    signed int pos;
    signed int rate;
    signed int diff;
    signed int accel;
    signed int SERVO_DIFF_LMT;

    if (ch < SEVERONUM) {
        servo = Servos + ch;

        pos = _butter_update(*servo->Position, servo->butt);
        *servo->Position = pos;
        if (ref > servo->MaxPosition) {
            ref = servo->MaxPosition;
        } else if (ref < servo->MinPosition) {
            ref = servo->MinPosition;
        }
        servo->Reference = ref;

        rate = pos - servo->PrevPosition;
        servo->PrevPosition = pos;
        accel = rate - servo->PrevRate;
        servo->PrevRate = rate;
        /* Accel limitation */
        if (accel > SERVO_ACCEL_LIMIT || accel < -SERVO_ACCEL_LIMIT) {
            rate = 0;
        }

        diff = servo->Reference - pos;
        SERVO_DIFF_LMT = servo->SERVO_DIFF_LMT;
        if (diff > SERVO_DIFF_LMT) diff = SERVO_DIFF_LMT;
        else if (diff < -SERVO_DIFF_LMT) diff = -SERVO_DIFF_LMT;
        /** python code to generate feedback coefficients
def pval(fb_coeff, shifts) :
     # unit convert
     fb_coeff_f = fb_coeff*(800-343)/3.8*pi/4096.0 * 2**shifts
     # round-off
     fb_coeff_i = int(fb_coeff_f)
     # error percentage
     err = (fb_coeff_i-fb_coeff_f)/fb_coeff_f*100
     # max input
     max_fb_val = 2**15 / fb_coeff_i
     return (fb_coeff_i,err,max_fb_val)
         */
        duty_circle = ((servo->Kp) * diff >> (servo->ShiftKp)) /* Proportion */
                + ((servo->Kd) * -rate >> (servo->ShiftKd)); /* Difference */
        if (diff > SERVO_SHAKE_DZ || diff < -SERVO_SHAKE_DZ
                || rate > SERVO_SHAKE_RDZ || rate < -SERVO_SHAKE_RDZ) {
            servo->tick = 0u;
            if (duty_circle > 0) {
                duty_circle += servo->shake;
            } else {
                duty_circle -= servo->shake;
            }
        } else {
            if (servo->tick < SERVO_SHAKE_TICKS) {
                if (++servo->tick & 1) {
                    duty_circle = servo->shake;
                } else {
                    duty_circle = -servo->shake;
                }
            } else {
                duty_circle = 0;
            }
        }

        _motor_set(servo, duty_circle);
    }
}

#endif /* USE_PWM && USE_ADC1 */
