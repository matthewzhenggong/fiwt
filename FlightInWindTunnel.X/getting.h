/*
 * File:   getting.h
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

#ifndef GETTING_H
#define	GETTING_H

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#define gettingPERIOD (10u)
#define gettingDELAY (5u)
#define gettingPRIORITY (9u)

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        XBee_p _xbee;
        ZBRxResponse_t rx_rsp;
        ZBTxRequest_t rx_echo;
    } gettingParam_t;

    void gettingInit(gettingParam_t *parameters, XBee_p);

    PT_THREAD(gettingLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* GETTING_H */

