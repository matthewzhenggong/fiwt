/*
 * File:   remoteSenTask.h
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

#ifndef REMOTESENTASK_H
#define	REMOTESENTASK_H

#include "config.h"

#include "msg.h"
#include "XBee.h"
#include "task.h"
#include <pt.h>

#ifdef	__cplusplus
extern "C" {
#endif

    extern float windtunnel_wind_speed; // m/s
    extern float windtunnel_dynamic_pressure; // pa

    typedef struct {
        struct pt PT;
        XBee_p xbee;
        ZBTxRequest_t tx_req;
        ZBRxResponse_t rx_rsp;
        unsigned int cnt;
    } remoteSenParam_t, *remoteSenParam_p;

    PT_THREAD(remoteSenLoop)(TaskHandle_p task);

    void remoteSenInit(remoteSenParam_p parameters, XBee_p s2);


#ifdef	__cplusplus
}
#endif

#endif	/* REMOTESENTASK_H */

