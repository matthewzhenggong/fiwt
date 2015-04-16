/*
 * File:   msg_recv.h
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

#ifndef MSG_RECV_H
#define	MSG_RECV_H

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        struct pt PT;
        XBee_p _xbee;
        TaskHandle_p serov_Task;
        TaskHandle_p sen_Task;
        unsigned int rx_cnt;
        unsigned int tx_cnt;
        unsigned int cnt;
        ZBRxResponse_t rx_rsp;
        ZBTxRequest_t tx_req;
    } msgRecvParam_t, *msgRecvParam_p;

    void msgRecvInit(msgRecvParam_p parameters, XBee_p, TaskHandle_p, TaskHandle_p);

    PT_THREAD(msgRecvLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_RECV_H */

