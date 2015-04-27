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

#include "config.h"

#include "XBeeZBS2.h"
#include "task.h"
#include "pt.h"

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define SPI_PKG_MAXLEN 86

    typedef struct NTP {
        int32_t offset;
        int16_t delay;
        int stage;
        uint32_t TimeStampR0;
        uint32_t TimeStampL0;
        uint32_t TimeStampL1;
        uint32_t TimeStampR2;
        uint32_t TimeStampR3;
        uint32_t TimeStampL4;
    } NTP_t, *NTP_p;

    typedef struct {
        struct pt PT;
        XBee_p xbee;
        TxIPv4Request_t tx_req;
        RxIPv4Response_t rx_rsp;
        unsigned int rx_cnt;
        unsigned int cnt;

        //time sync
        NTP_t ntp;

        //send general message
        uint8_t msg_buff[256];
        uint8_t msg_len;

#if AC_MODEL || AEROCOMP
        TaskHandle_p serov_Task;
        TaskHandle_p sen_Task;
        TaskHandle_p recv_Task;
#elif GNDBOARD
        uint8_t spis_pkg_buff[SPI_PKG_MAXLEN];
#endif
    } msgParam_t, *msgParam_p;

#if AC_MODEL || AEROCOMP
    void msgInit(msgParam_p parameters, XBee_p, TaskHandle_p, TaskHandle_p);
#elif GNDBOARD
    void msgInit(msgParam_p parameters, XBee_p);
#endif

    PT_THREAD(msgLoop)(TaskHandle_p task);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_GND_H */

