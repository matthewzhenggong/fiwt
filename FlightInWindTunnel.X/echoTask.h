/*
 * File:   echo.h
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

#ifndef ECHOTASK_H
#define	ECHOTASK_H

#include "SerialStream.h"
#include "task.h"
#include "pt.h"

#define echoPERIOD (10u)
#define echoDELAY (4u)
#define echoPRIORITY (1u)

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        SerialStream_p _serial;
        uint8_t _buff[300];
    } echoParam_t;

    void echoInit(echoParam_t *parameters, SerialStream_p);

    PT_THREAD(echoLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* ECHOTASK_H */

