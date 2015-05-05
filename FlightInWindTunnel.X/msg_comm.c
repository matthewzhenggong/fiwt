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
#include "msg_code.h"
#include "clock.h"

int16_t ntp_offset;
int16_t ntp_delay;

static uint16_t ntp_token;

size_t requestNTP(PushMessageHandle_p msg_h, uint8_t *head, size_t max_len) {
    uint8_t *pack;
    pack = head;

    *(pack++) = CODE_NTP_REQUEST;
    ntp_token = (microsec_ticks)^(MSG_SRC_PORT);
    *(pack++) = ntp_token >> 8;
    *(pack++) = ntp_token & 0xff;
    return pack - head;
}

bool processNTPreq(ProcessMessageHandle_p msg_h, const uint8_t *cmd, size_t max_len) {
    int32_t T1,T2,T3,offset;
    uint8_t head[8];
    uint8_t *pack;
    if (cmd[0] == CODE_NTP_REQUEST) {
        T1 = msg_h->remote_tx_timestamp;
        T3 = msg_h->remote_gen_timestamp;
        offset = T1-T3;
        if (offset > 1000 || offset < 0) {
            return false;
        }
        pack = head;
        *(pack++) = CODE_NTP_RESPONSE;
        *(pack++) = cmd[1];
        *(pack++) = cmd[2];
        *(pack++) = (T1 >> 24);
        *(pack++) = (T1 >> 16);
        *(pack++) = (T1 >> 8);
        *(pack++) = (T1 & 0xff);
        T2 = msg_h->rx_timestamp;
        *(pack++) = (T2 >> 24);
        *(pack++) = (T2 >> 16);
        *(pack++) = (T2 >> 8);
        *(pack++) = (T2 & 0xff);
        return pushMessage((msgParam_p)msg_h->parameters, msg_h->remote_tx_port, head, 8);
    }
    return false;
}

bool processNTPrsp(ProcessMessageHandle_p msg_h, const uint8_t *msg_ptr, size_t msg_len) {
    int32_t T1,T2,T3,T4,T5;
    int32_t offset;
    int32_t delay;
    int32_t T;

    if (msg_ptr[0] == CODE_NTP_RESPONSE && msg_ptr[1] == (ntp_token >> 8) && msg_ptr[2] == (ntp_token & 0xFF)) {
        T3 = msg_h->remote_tx_timestamp;
        T5 = msg_h->remote_gen_timestamp;
        offset = T3-T5;
        if (offset > 1000 || offset < 0) {
            return false;
        }
        T2 = ((uint32_t)msg_ptr[7]<<24)+((uint32_t)msg_ptr[8]<<16)+((uint32_t)msg_ptr[9]<<8)+(uint32_t)msg_ptr[10];
        offset = T5-T2;
        if (offset > 1000 || offset < 0) {
            return false;
        }

        T4 = msg_h->rx_timestamp;        
        T1 = ((uint32_t)msg_ptr[3]<<24)+((uint32_t)msg_ptr[4]<<16)+((uint32_t)msg_ptr[5]<<8)+(uint32_t)msg_ptr[6];
        offset = T4-T1;
        if (offset > 5000 || offset < 0) {
            return false;
        }

        delay = (T4-T1) - (T3-T2);
        offset = (((T2-T1) + (T3-T4))/2);
        if (offset < 4000 && offset > -4000) {
            offset >>= 2;
        }
        T = getMicroseconds();
        T += offset;
        setMicroseconds(T);
        ntp_delay = delay;
        ntp_offset = offset;
        return true;
    }
    return false;
}

void msgRegistNTP(msgParam_p msg) {
#if AC_MODEL || AEROCOMP
    registerPushMessageHandle(msg, "NTPREQ", &requestNTP, msg,
            TargetAP, 1000, 108);
#else
    registerPushMessageHandle(msg, "NTPREQ", &requestNTP, msg,
            TargetAP, 1000, 108);
#endif
    registerProcessMessageHandle(msg, "NTPREQ", CODE_NTP_REQUEST, processNTPreq, msg);
    registerProcessMessageHandle(msg, "NTPRSP", CODE_NTP_RESPONSE, processNTPrsp, msg);
}


