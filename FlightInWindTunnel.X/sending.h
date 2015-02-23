/*
 * File:   sending.h
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

#ifndef SENDING_H
#define	SENDING_H

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        ZBTxRequest_t tx_req;
        struct pt PT;
        unsigned int cnt;
        XBee_p _xbee[2];
    } sendingParam_t;

    void sendingInit(sendingParam_t *parameters, XBee_p, XBee_p);

    PT_THREAD(sendingLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* SENDING_H */

