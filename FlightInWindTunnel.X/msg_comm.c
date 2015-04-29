/*
 * File:   msg_comm.c
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

#include "msg.h"
#include "clock.h"

uint8_t* EscapeByte(uint8_t* pack, uint8_t b) {
    if (b == MSG_DILIMITER || b == MSG_ESC) {
        *(pack++) = MSG_ESC;
        *(pack++) = b^0x20;
    } else {
        *(pack++) = b;
    }
    return pack;
}


size_t updateNTPPack(NTP_p ntp, uint8_t *head) {
    uint8_t *pack;
    ntp->stage++;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x01');
    pack = EscapeByte(pack, ntp->TimeStampL0 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL0 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL0 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL0 & 0xff);
    ntp->TimeStampL1 = getMicroseconds();
    pack = EscapeByte(pack, ntp->TimeStampL1 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL1 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL1 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL1 & 0xff);
    return pack - head;
}

size_t updateNTPPack3(NTP_p ntp, uint8_t *head) {
    uint8_t *pack;
    ntp->stage = 0u;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x03');
    pack = EscapeByte(pack, ntp->TimeStampL4 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL4 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL4 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL4 & 0xff);
    pack = EscapeByte(pack, ntp->delay >> 8);
    pack = EscapeByte(pack, ntp->delay & 0xff);
    pack = EscapeByte(pack, ntp->offset >> 24);
    pack = EscapeByte(pack, ntp->offset >> 16);
    pack = EscapeByte(pack, ntp->offset >> 8);
    pack = EscapeByte(pack, ntp->offset & 0xff);
    ntp->TimeStampL1 = getMicroseconds();
    pack = EscapeByte(pack, ntp->TimeStampL1 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL1 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL1 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL1 & 0xff);

    return pack - head;
}

size_t updateNTPPack11(NTP_p ntp, uint8_t *head) {
    uint8_t *pack;
    uint32_t ts;
    ntp->stage = 0u;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x12');
    pack = EscapeByte(pack, ntp->TimeStampLP >>24);
    pack = EscapeByte(pack, ntp->TimeStampLP >>16);
    pack = EscapeByte(pack, ntp->TimeStampLP >> 8);
    pack = EscapeByte(pack, ntp->TimeStampLP & 0xff);
    ts = getMicroseconds();
    pack = EscapeByte(pack, ts >>24);
    pack = EscapeByte(pack, ts >>16);
    pack = EscapeByte(pack, ts >> 8);
    pack = EscapeByte(pack, ts & 0xff);
    return pack - head;
}

void reset_clock(NTP_p ntp, int apply) {
    int32_t T;
    int32_t T1, T2, T3, T4;
    if (apply == 2) {
        ntp->offset = ntp->TimeStampRP - ntp->TimeStampLP;
        ntp->offset += 3000; //TODO
        if (ntp->offset < 5000 && ntp->offset > -5000) {
            ntp->offset >>= 2;
        }
        T = getMicroseconds();
        T += ntp->offset;
        setMicroseconds(T);
    } else {
        T1 = ntp->TimeStampL1;
        T2 = ntp->TimeStampR2;
        T3 = ntp->TimeStampR3;
        T4 = ntp->TimeStampL4;
        ntp->delay = (T4-T1) - (T3-T2);
        ntp->offset = ((T2-T1) + (T3-T4))/2;
        if (ntp->offset < 5000 && ntp->offset > -5000) {
            ntp->offset >>= 2;
        }
        T = getMicroseconds();
        T += ntp->offset;
        if (apply) {
            setMicroseconds(T);
        }
    }
}

void msgInitComm(msgParam_p parameters, XBee_p s6) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->xbee = s6;
    parameters->tx_req._des_addr_msw = MSG_DEST_ADDR_MSW;
    parameters->tx_req._des_addr_lsw = MSG_DEST_ADDR_LSW;
    parameters->tx_req._des_port = MSG_DEST_PORT;
    parameters->tx_req._src_port = MSG_SRC_PORT;
    parameters->tx_req._protocol = 0u;
    parameters->tx_req._option = 0u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';

    parameters->cnt = 0u;
    parameters->rx_cnt = 0u;
    parameters->ntp.stage = 0u;
    parameters->msg_len = 0u;
}
