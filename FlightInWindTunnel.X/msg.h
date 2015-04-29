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

#ifndef MSG_H
#define	MSG_H

#include "config.h"

#include "XBeeZBS2.h"
#include "task.h"
#include <pt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if GNDBOARD
#define SPI_PKG_MAXLEN 86
#endif

#ifdef	__cplusplus
extern "C" {
#endif

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
        uint32_t TimeStampLP;
        uint32_t TimeStampRP;
    } NTP_t, *NTP_p;

    typedef struct PM {
        int32_t offset;
        int16_t delay;
        uint8_t target;
        int stage;
        uint32_t TimeStampL1;
        uint32_t TimeStampR2;
        uint32_t TimeStampR3;
        uint32_t TimeStampL4;
    } PM_t, *PM_p;

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
        PM_t pm;
#endif
    } msgParam_t, *msgParam_p;

    PT_THREAD(msgLoop)(TaskHandle_p task);

    uint8_t* EscapeByte(uint8_t* pack, uint8_t b);

    size_t updateNTPPack(NTP_p ntp, uint8_t *head);

    size_t updateNTPPack3(NTP_p ntp, uint8_t *head);

    size_t updateNTPPack11(NTP_p ntp, uint8_t *head);

    void reset_clock(NTP_p ntp, int apply);

    void msgInitComm(msgParam_p parameters, XBee_p s6);


#ifdef	__cplusplus
}
#endif

#endif	/* MSG_H */

