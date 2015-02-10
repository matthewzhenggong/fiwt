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

#include "idle.h"
#include "clock.h"

void idleInit(idleParam_p parameters) {
    parameters->call_per_second = 0u;
    parameters->cnt = 0u;
    parameters->seconds = RTclock.seconds;
}

char idleLoop(TaskHandle_p task) {
    idleParam_t *parameters;
    parameters = (idleParam_t *)(task->parameters);

    ++(parameters->cnt);
    if (parameters->seconds == RTclock.seconds)
    {
        return 0;
    }
    else
    {
        parameters->seconds = RTclock.seconds;
        parameters->call_per_second = parameters->cnt;
        parameters->cnt = 0u;
        return 1;
    }
}
