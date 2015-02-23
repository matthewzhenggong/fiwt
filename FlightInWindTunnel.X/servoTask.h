/*
 * File:   servoTask.h
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

#ifndef SERVOTASK_H
#define	SERVOTASK_H

#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        uint16_t _ticks;
        uint16_t _peroid;
        int16_t _dc;
        uint8_t _ch;
        uint8_t _loop;
        uint8_t _test_mode;
    } servoParam_t;

    void servoInit(servoParam_t *parameters);

    PT_THREAD(servoLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* SERVOTASK_H */

