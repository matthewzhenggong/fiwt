/*
 * File:   msg_gnd.c
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

#include "msg_gnd.h"

#if GNDBOARD

#include "AnalogInput.h"
#include "msg_code.h"
#include "msg_comm.h"

#include "clock.h"
#include <stdbool.h>
#include <string.h>

static bool updateCommPack(sendmsgParam_p msg) {
    uint8_t head[1+4+4];
    uint8_t *pack;
    msgParamGND_p p;

    p = (msgParamGND_p)(msg->_param);

    pack = head;

    *(pack++) = CODE_GNDBOARD_STATS;
    *(pack++) = ntp_delay >> 8;
    *(pack++) = ntp_delay & 0xFF;
    *(pack++) = ntp_offset >> 8;
    *(pack++) = ntp_offset & 0xFF;

    *(pack++) = p->sen_Task->load_max >> 8;
    *(pack++) = p->sen_Task->load_max & 0xFF;
    *(pack++) = p->msg_Task->load_max >> 8;
    *(pack++) = p->msg_Task->load_max & 0xFF;

    return pushMessage(msg->_msg, TargetAP, head, pack - head);
}

static sendmsgParam_t comm;
static msgParam_p _msg;

void msgRegistGND(msgParam_p msg, msgParamGND_p param, unsigned priority) {
    _msg = msg;

    sendmsgInit(&comm, msg, &updateCommPack, param);
    TaskCreate(sendmsgLoop, "COMM", (void *) &comm, 0x3FF, 0x101, priority);
}

bool sendRigPack(void) {
    uint8_t head[1+RIGPOSADCNUM*2+8+timestamp_size+2];
    uint8_t *pack;
    int i;

    pack = head;
    *(pack++) = CODE_GNDBOARD_ADCM_READ;
    for (i = 0; i < RIGPOSADCNUM; ++i) {
        *(pack++) = RigPos[i] >> 8;
        *(pack++) = RigPos[i] & 0xff;
    }

    *(pack++) = RigRollPos >> 24;
    *(pack++) = RigRollPos >> 16;
    *(pack++) = RigRollPos >> 8;
    *(pack++) = RigRollPos & 0xff;
    *(pack++) = RigPitchPos >> 8;
    *(pack++) = RigPitchPos & 0xff;
    *(pack++) = RigYawPos >> 8;
    *(pack++) = RigYawPos & 0xff;

    pack = packInt(pack, &ADC_TimeStamp, timestamp_size);

    return pushMessage(_msg, TargetAP, head, pack - head);
}


#endif /*GNDBOARD*/
