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

#ifdef	__cplusplus
extern "C" {
#endif

    struct Servo {
        volatile unsigned int *Position;
        unsigned int PrevPosition;
        unsigned int Reference;
        signed int Ctrl;
        volatile unsigned int *DutyCycle;
        volatile unsigned int *lat_cw;
        unsigned int lat_cw_pos;
        unsigned int lat_cw_mask;
        volatile unsigned int *lat_ccw;
        unsigned int lat_ccw_pos;
        unsigned int lat_ccw_mask;
        unsigned int Enabled;
    };
    typedef struct Servo Servo_t;
    typedef Servo_t *Servo_p;

    extern Servo_t Servos[];

    void ServoInit(void);

    void ServoStart(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SERVO_H */

