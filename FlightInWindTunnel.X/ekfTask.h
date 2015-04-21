/*
 * File:   ekfTask.h
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

#ifndef EKFTASK_H
#define	EKFTASK_H

#include "config.h"

#if USE_EKF

#include "EKFF.h"
#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        EKFF_t ekff;
        enum OP_TYPE op;
        float y0[16];
    } ekfParam_t, *ekfParam_p;

    void ekfInit(ekfParam_p parameters, float dt);

    PT_THREAD(ekfLoop)(TaskHandle_p task);



#ifdef	__cplusplus
}
#endif

#endif

#endif	/* EKFTASK_H */

