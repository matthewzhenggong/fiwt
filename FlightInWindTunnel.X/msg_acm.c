/*
 * File:   msg_acm.c
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

#include "msg_acm.h"

#if AC_MODEL || AEROCOMP

#include "Enc.h"
#include "AnalogInput.h"
#include "IMU.h"
#include "Servo.h"
#include "msg_code.h"
#include "msg_comm.h"

static bool updateCommPack(sendmsgParam_p msg) {
    uint8_t head[1+4+BATTCELLADCNUM+2*3];
    uint8_t *pack;
    msgParamACM_p p;
    int i;

    p = (msgParamACM_p)(msg->_param);

    pack = head;

#if AEROCOMP
    *(pack++) = CODE_AEROCOMP_STATS;
#else
    *(pack++) = CODE_AC_MODEL_STATS;
#endif
    *(pack++) = ntp_delay >> 8;
    *(pack++) = ntp_delay & 0xFF;
    *(pack++) = ntp_offset >> 8;
    *(pack++) = ntp_offset & 0xFF;
    for (i = 0; i < BATTCELLADCNUM; ++i) {
        *(pack++) = BattCell[i];
    }
    *(pack++) = p->sen_Task->load_max >> 8;
    *(pack++) = p->sen_Task->load_max & 0xFF;
    *(pack++) = p->servo_Task->load_max >> 8;
    *(pack++) = p->servo_Task->load_max & 0xFF;
    *(pack++) = p->msg_Task->load_max >> 8;
    *(pack++) = p->msg_Task->load_max & 0xFF;

    return pushMessage(msg->_msg, TargetAP, head, pack - head);
}

bool servoProcA5Cmd(ProcessMessageHandle_p msg_h, const uint8_t *cmd, size_t msg_len) {
    int i;
    msgParamACM_p p;
    servoParam_p parameters;
    p = (msgParamACM_p)(msg_h->parameters);
    parameters = p->serov_param;

    if (cmd[0] != 0xA5 && cmd[0] != 0xA6) {
        return false;
    }
    unpackInt(cmd+1, &(parameters->time_token), 4);

    switch (cmd[5]) {
        case 1:
            for (i = 0; i < SEVERONUM; ++i) {
                parameters->Servo_PrevRef[i] = parameters->ServoRef[i] = ((cmd[6 + i * 2] << 8) | cmd[6 + i * 2 + 1]);
            }
            parameters->InputType = 0;
            parameters->StartTime = 100;
            parameters->TimeDelta = 100;
            parameters->NofCycles = 1;
            parameters->Srv2Move = 0xFF;
            parameters->GenerateInput_Flag = 1;
            parameters->cnt = 0u;
            break;
        case 2:
        case 3:
        case 4:
        case 7:
            goto yyyy;
        case 5:
            parameters->TimeDelta = ((uint32_t) ((cmd[9] << 8) | cmd[10]))*8UL / TASK_PERIOD;
            goto xxxx;
        case 8:
        case 9:
            parameters->TimeDelta = ((uint32_t) ((cmd[9] << 8) | cmd[10]))*4UL / TASK_PERIOD;
            goto xxxx;
yyyy:
            parameters->TimeDelta = ((cmd[9] << 8) | cmd[10]) / TASK_PERIOD;
xxxx:
            parameters->InputType = cmd[5] - 1u;
            parameters->Srv2Move = cmd[6];
            parameters->StartTime = ((cmd[7] << 8) | cmd[8]) / TASK_PERIOD;
            parameters->NofCycles = cmd[11];
            for (i = 0; i < 6; ++i) {
                parameters->MaxValue[i] = (cmd[12 + i] << 2);
            }
            for (i = 0; i < 6; ++i) {
                parameters->MinValue[i] = (cmd[18 + i] << 1);
            }
            for (i = 0; i < 6; ++i) {
                parameters->Sign[i] = cmd[24 + i];
            }
            parameters->GenerateInput_Flag = 1;
            parameters->cnt = 0u;
            break;
        case 6:
            break;
        default :
            return false;
            break;
    }
    return true;
}

static sendmsgParam_t comm;
static msgParam_p _msg;

void msgRegistACM(msgParam_p msg, msgParamACM_p param, unsigned priority ) {
    _msg = msg;

    sendmsgInit(&comm, msg, &updateCommPack, param);
#if AC_MODEL
    TaskCreate(sendmsgLoop, "COMM", (void *) &comm, 0x3FF, 0x102, priority);
    registerProcessMessageHandle(msg, "CMDA5", CODE_AC_MODEL_SERV_CMD, servoProcA5Cmd, param);
#else
    TaskCreate(sendmsgLoop, "COMM", (void *) &comm, 0x3FF, 0x103, priority);
    registerProcessMessageHandle(msg, "CMDA6", CODE_AEROCOMP_SERV_CMD, servoProcA5Cmd, param);
#endif
}

bool sendDataPack(uint32_t T1) {
    uint8_t head[1+SERVOPOSADCNUM*2+ENCNUM*2+6*2+sizeof(timestamp_t)+SEVERONUM*4+4];
    uint8_t *pack;
    int i;

    pack = head;
#if AEROCOMP
    *(pack++) = CODE_AEROCOMP_SERVO_POS;
#else
    *(pack++) = CODE_AC_MODEL_SERVO_POS;
#endif
    for (i = 0; i < SERVOPOSADCNUM; ++i) {
        *(pack++) = ServoPos[i] >> 8;
        *(pack++) = ServoPos[i] & 0xFF;
    }
    for (i = 0; i < ENCNUM; ++i) {
        *(pack++) = EncPos[i] >> 8;
        *(pack++) = EncPos[i] & 0xFF;
    }
#if USE_IMU
    *(pack++) = IMU_XGyro >> 8;
    *(pack++) = IMU_XGyro & 0xFF;
    *(pack++) = IMU_YGyro >> 8;
    *(pack++) = IMU_YGyro & 0xFF;
    *(pack++) = IMU_ZGyro >> 8;
    *(pack++) = IMU_ZGyro & 0xFF;
    *(pack++) = IMU_XAccl >> 8;
    *(pack++) = IMU_XAccl & 0xFF;
    *(pack++) = IMU_YAccl >> 8;
    *(pack++) = IMU_YAccl & 0xFF;
    *(pack++) = IMU_ZAccl >> 8;
    *(pack++) = IMU_ZAccl & 0xFF;
#endif
    pack = packInt(pack, &ADC_TimeStamp, sizeof(timestamp_t));
    for (i = 0; i < SEVERONUM; ++i) {
        *(pack++) = Servos[i].Ctrl >> 8;
        *(pack++) = Servos[i].Ctrl & 0xFF;
    }
    for (i = 0; i < SEVERONUM; ++i) {
        *(pack++) = Servos[i].Reference >> 8;
        *(pack++) = Servos[i].Reference & 0xFF;
    }
    pack = packInt(pack, &T1, 4);

    return pushMessage(_msg, TargetAP, head, pack - head);
}


#endif /*AC_MODEL || AEROCOMP*/
