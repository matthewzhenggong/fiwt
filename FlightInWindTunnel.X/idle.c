/*
 * File:   idle.c
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

#include "config.h"

#include "idle.h"
#include "clock.h"
#include "IMU.h"

#include "LedExtBoard.h"

void idleInit(idleParam_p parameters) {
    parameters->call_per_second = 0u;
    parameters->cnt = 0u;
    parameters->secticks = (getMicroseconds()>>20);
}

char idleLoop(TaskHandle_p task) {
    idleParam_t *parameters;
    unsigned sec;
    parameters = (idleParam_t *)(task->parameters);

    ++(parameters->cnt);
    sec = (getMicroseconds()>>20);
    
#if USE_IMU
    if (microsec_ticks < 960) {
        IMURead2DMA();
    }
#endif
    if (parameters->secticks == sec)
    {
        return 0;
    }
    else
    {
        parameters->secticks = sec;
        parameters->call_per_second = parameters->cnt;
        parameters->cnt = 0u;
#if USE_LEDEXTBOARD
        mLED_1_Toggle();
        mLED_2_Off();
        mLED_3_Off();
        mLED_4_Off();
#endif
        return 1;
    }
}
