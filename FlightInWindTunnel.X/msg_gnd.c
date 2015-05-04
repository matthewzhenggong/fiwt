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
#include "remoteSenTask.h"

#include "clock.h"
#include <stdbool.h>

static size_t updateCommPack(PushMessageHandle_p msg_h, uint8_t *head, size_t max_len) {
    uint8_t *pack;
    msgParamGND_p p;

    p = (msgParamGND_p)(msg_h->parameters);

    if (max_len < 1+6+4) {
        return 0;
    }
    pack = head;

    *(pack++) = CODE_GNDBOARD_STATS;
    *(pack++) = ntp_delay >> 8;
    *(pack++) = ntp_delay & 0xFF;
    *(pack++) = ntp_offset >> 8;
    *(pack++) = ntp_offset & 0xFF;

    *(pack++) = p->rsen_Task->load_max >> 8;
    *(pack++) = p->rsen_Task->load_max & 0xFF;
    *(pack++) = p->rsen_Task->load_max >> 8;
    *(pack++) = p->rsen_Task->load_max & 0xFF;
    *(pack++) = p->msg_Task->load_max >> 8;
    *(pack++) = p->msg_Task->load_max & 0xFF;

    return pack - head;
}

static msgParam_p _msg;
uint16_t _sen_data_target;

void msgRegistGND(msgParam_p msg, msgParamGND_p param) {
    _msg = msg;
    _sen_data_target = TargetAP;

    registerPushMessageHandle(msg, "COMM", &updateCommPack, param,
            MSG_DEST_PORT, 1000, 508);
}

bool sendRigPack(void) {
    uint8_t head[1+RIGPOSADCNUM*2+8+4];
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

//    *(pack++) = RigRollRate >> 8;
//    *(pack++) = RigRollRate & 0xff;
//    *(pack++) = RigPitchRate >> 8;
//    *(pack++) = RigPitchRate & 0xff;
//    *(pack++) = RigYawRate >> 8;
//    *(pack++) = RigYawRate & 0xff;

    *(pack++) = ADC_TimeStamp >>24;
    *(pack++) = ADC_TimeStamp >>16;
    *(pack++) = ADC_TimeStamp >> 8;
    *(pack++) = ADC_TimeStamp & 0xFF;

    return pushMessage(_msg, _sen_data_target, head, pack - head);
}

bool sendSpeedPack(void) {
    uint8_t head[1+RIGPOSADCNUM*2+4];
    uint8_t *pack;

    uint8_t *float_addr;

    pack = head;
    *(pack++) = CODE_GNDBOARD_ADCM_READ;

    float_addr = (uint8_t *)&windtunnel_wind_speed;
    *(pack++) = *(float_addr+3);
    *(pack++) = *(float_addr+2);
    *(pack++) = *(float_addr+1);
    *(pack++) = *(float_addr);

    float_addr = (uint8_t *)&windtunnel_dynamic_pressure;
    *(pack++) = *(float_addr+3);
    *(pack++) = *(float_addr+2);
    *(pack++) = *(float_addr+1);
    *(pack++) = *(float_addr);

    return pushMessage(_msg, _sen_data_target, head, pack - head);
}


#endif /*GNDBOARD*/
