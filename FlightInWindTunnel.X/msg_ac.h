/*
 * File:   msg.h
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

#ifndef MSG_AC_H
#define	MSG_AC_H

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        XBee_p _xbee[2];
        TaskHandle_p serov_Task;
        unsigned int cnt;
        ZBRxResponse_t rx_rsp;
        ZBTxRequest_t tx_req;
    } msgParam_t, *msgParam_p;

    void msgInit(msgParam_p parameters, XBee_p, XBee_p, TaskHandle_p);

    PT_THREAD(msgLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_AC_H */

