/*
 * File:   senTask.h
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

#ifndef SENTASK_H
#define	SENTASK_H

#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;

    } senParam_t, *senParam_p;

    void senInit(senParam_p parameters);

    PT_THREAD(senLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* SENTASK_H */

