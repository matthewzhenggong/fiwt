/*
 * File:   idle.h
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

#ifndef IDLE_H
#define	IDLE_H

#include "task.h"
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct{
        uint32_t call_per_second;
        uint32_t cnt;
        unsigned int seconds;
    } idleParam_t;
    typedef idleParam_t * idleParam_p;

    void idleInit(idleParam_p parameters);

    char idleLoop(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* IDLE_H */

