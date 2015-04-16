/*
 * File:   msg_gnd.h
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

#ifndef MSG_GND_H
#define	MSG_GND_H

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define SPI_PKG_MAXLEN 86

    enum XbeeGroup {
        XB_AC_TX,
        XB_AC_RX,
        XB_COMP_TX,
        XB_COMP_RX,
        XB_NON
    };
    
    typedef struct remote {
        XBee_p xbee;
        ZBTxRequest_t tx_req;
    } RemoteNode;

    typedef struct {
        struct pt PT;
        RemoteNode node[4];
        RemoteNode* nodeACTx;
        RemoteNode* nodeACRx;
        RemoteNode* nodeCOMPTx;
        RemoteNode* nodeCOMPRx;
        unsigned int tx_cnt;
        unsigned int rx_cnt;
        ZBRxResponse_t rx_rsp;
        uint8_t spis_pkg_buff[SPI_PKG_MAXLEN];
    } msgParam_t, *msgParam_p;

    void msgInit(msgParam_t *parameters, XBee_p, XBee_p, XBee_p, XBee_p);

    PT_THREAD(msgLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_GND_H */

