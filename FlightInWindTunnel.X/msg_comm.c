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

bool requestNTP(sendmsgParam_p msg) {
    uint8_t head[1+3];
    uint8_t *pack;

    pack = head;
    *(pack++) = CODE_NTP_REQUEST;
    ntp_token = getMicrosecondsLSDW()^(MSG_SRC_PORT);
    *(pack++) = ntp_token >> 8;
    *(pack++) = ntp_token & 0xff;

    return pushMessage(msg->_msg, TargetAP, head, pack - head);
}


bool processNTPreq(ProcessMessageHandle_p msg_h, const uint8_t *cmd, size_t max_len) {
    timestamp_t T1,T2,T3,offset;
    uint8_t head[19];
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
        pack = packInt(pack, &T1, timestamp_size);
        pack = packInt(pack, &T2, timestamp_size);
        return pushMessage((msgParam_p)msg_h->parameters, findTarget((msgParam_p)msg_h->parameters, msg_h->remote_tx_addr), head, pack - head);
    }
    return false;
}

bool processNTPrsp(ProcessMessageHandle_p msg_h, const uint8_t *msg_ptr, size_t msg_len) {
    timestamp_t T1,T2,T3,T4,T5;
    timestamp_t offset, delay;

    if (msg_ptr[0] == CODE_NTP_RESPONSE && msg_ptr[1] == *((uint8_t*)&ntp_token+1) && msg_ptr[2] == *((uint8_t*)&ntp_token)) {
        T3 = msg_h->remote_tx_timestamp;
        T5 = msg_h->remote_gen_timestamp;
        offset = T3-T5;
        if (offset > 1000 || offset < 0) {
            return false;
        }
        unpackInt(msg_ptr+11, &T2, timestamp_size);
        offset = T5-T2;
        if (offset > 1000 || offset < 0) {
            return false;
        }

        T4 = msg_h->rx_timestamp;        
        unpackInt(msg_ptr+3, &T1, timestamp_size);
        offset = T4-T1;
        if (offset > 5000 || offset < 0) {
            return false;
        }

        delay = (T4-T1) - (T3-T2);
        offset = (((T2-T1) + (T3-T4))>>1);
        if (offset < 4000 && offset > -4000) {
            offset >>= 3;
        }
        set_time_offset(offset);

        ntp_delay = delay;
        ntp_offset = offset;
        return true;
    }
    return false;
}

static sendmsgParam_t ntpreq;

void msgRegistNTP(msgParam_p msg, unsigned priority) {
    sendmsgInit(&ntpreq, msg, &requestNTP, NULL);
#if AC_MODEL
    TaskCreate(sendmsgLoop, "NTPREQ", (void *) &ntpreq, 0x7FF, 2, priority);
#elif AEROCOMP
    TaskCreate(sendmsgLoop, "NTPREQ", (void *) &ntpreq, 0x7FF, 3, priority);
#else
    TaskCreate(sendmsgLoop, "NTPREQ", (void *) &ntpreq, 0x7FF, 1, priority);
#endif
    registerProcessMessageHandle(msg, "NTPREQ", CODE_NTP_REQUEST, processNTPreq, msg);
    registerProcessMessageHandle(msg, "NTPRSP", CODE_NTP_RESPONSE, processNTPrsp, msg);
}


