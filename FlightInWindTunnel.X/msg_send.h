/*
 * File:   msg_send.h
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

#ifndef MSG_SEND_H
#define	MSG_SEND_H

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
        TaskHandle_p ekf_Task;
        TaskHandle_p recv_Task;
        unsigned int cnt;
        XBeeSeries_t xbee_type;
        uint8_t msg_buff[256];
        uint8_t msg_len;
        union {
           ZBTxRequest_t zbtx;
           TxA64Request_t txa64;
        } tx_req;
    } msgSendParam_t, *msgSendParam_p;

    void msgSendInit(msgSendParam_p parameters, XBee_p, XBeeSeries_t, TaskHandle_p, TaskHandle_p, TaskHandle_p, TaskHandle_p);

    PT_THREAD(msgSendLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_SEND_H */

