/*
 * File:   Servo.h
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

#ifndef SERVO_H
#define	SERVO_H

#include "config.h"

#if USE_PWM && USE_ADC1

#include <dsp.h>

#define SEVERONUM 2

#ifdef	__cplusplus
extern "C" {
#endif

    struct Servo {
        volatile unsigned int *Position;
        volatile unsigned int *DutyCycle;
        volatile unsigned int *lat_cw;
        unsigned int lat_cw_pos;
        unsigned int lat_cw_mask;
        volatile unsigned int *lat_ccw;
        unsigned int lat_ccw_pos;
        unsigned int lat_ccw_mask;
        signed int  PrevPosition;
        signed int  Reference;
        fractional butt[3];
        signed int Ctrl;
    };
    typedef struct Servo Servo_t;
    typedef Servo_t *Servo_p;

    extern Servo_t Servos[];

    void ServoInit(void);

    void ServoStart(void);

    /**
     * Set the duty circle and direction for servo
     * @param ch Servo channel
     * @param duty_circle for pwm gen and sign means direction
     */
    void MotorSet(unsigned int ch, signed int duty_circle);

    /**
     * Set Servo reference and update the control law
     * @param ch Servo channel
     * @param ref
     */
    void ServoUpdate100Hz(unsigned int ch, unsigned int ref);

#ifdef	__cplusplus
}
#endif

#endif /* USE_PWM && USE_ADC1 */

#endif	/* SERVO_H */

