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

void msgInit(msgParam_p parameters, XBee_p s6, TaskHandle_p senTask, TaskHandle_p servoTask) {
    msgInitComm(parameters, s6);

    parameters->sen_Task = senTask;
    parameters->serov_Task = servoTask;
}

size_t updateBattPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_BAT_LEV);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_BAT_LEV);
#endif
    for (i = 0; i < BATTCELLADCNUM; ++i) {
        pack = EscapeByte(pack, BattCell[i]);
    }
    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    return pack - head;
}

size_t updateCommPack(TaskHandle_p sen_Task, TaskHandle_p serov_Task,
        TaskHandle_p task, uint8_t head[]) {
    uint8_t *pack;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_COM_STATS);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_COM_STATS);
#endif
    pack = EscapeByte(pack, sen_Task->load_max >> 8);
    pack = EscapeByte(pack, sen_Task->load_max & 0xFF);
    pack = EscapeByte(pack, serov_Task->load_max >> 8);
    pack = EscapeByte(pack, serov_Task->load_max & 0xFF);
    pack = EscapeByte(pack, task->load_max >> 8);
    pack = EscapeByte(pack, task->load_max & 0xFF);

    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    return pack - head;
}

size_t updateSensorPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_SERVO_POS);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_SERVO_POS);
#endif
    for (i = 0; i < SERVOPOSADCNUM; ++i) {
        pack = EscapeByte(pack, ServoPos[i] >> 8);
        pack = EscapeByte(pack, ServoPos[i] & 0xFF);
    }
    for (i = 0; i < ENCNUM; ++i) {
        pack = EscapeByte(pack, EncPos[i] >> 8);
        pack = EscapeByte(pack, EncPos[i] & 0xFF);
    }
#if USE_IMU
    pack = EscapeByte(pack, IMU_XGyro >> 8);
    pack = EscapeByte(pack, IMU_XGyro & 0xFF);
    pack = EscapeByte(pack, IMU_YGyro >> 8);
    pack = EscapeByte(pack, IMU_YGyro & 0xFF);
    pack = EscapeByte(pack, IMU_ZGyro >> 8);
    pack = EscapeByte(pack, IMU_ZGyro & 0xFF);
    pack = EscapeByte(pack, IMU_XAccl >> 8);
    pack = EscapeByte(pack, IMU_XAccl & 0xFF);
    pack = EscapeByte(pack, IMU_YAccl >> 8);
    pack = EscapeByte(pack, IMU_YAccl & 0xFF);
    pack = EscapeByte(pack, IMU_ZAccl >> 8);
    pack = EscapeByte(pack, IMU_ZAccl & 0xFF);
#endif
    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    for (i = 0; i < SEVERONUM; ++i) {
        pack = EscapeByte(pack, Servos[i].Ctrl >> 8);
        pack = EscapeByte(pack, Servos[i].Ctrl & 0xFF);
    }

    return pack - head;
}


void servoProcA5Cmd(servoParam_p parameters, const uint8_t cmd[]) {
    int i;

    switch (cmd[1]) {
        case 1:
            for (i = 0; i < SEVERONUM; ++i) {
                parameters->Servo_PrevRef[i] = parameters->ServoRef[i] = ((cmd[2 + i * 2] << 8) | cmd[2 + i * 2 + 1]);
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
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*8UL / TASK_PERIOD;
            goto xxxx;
        case 8:
        case 9:
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*4UL / TASK_PERIOD;
            goto xxxx;
yyyy:
            parameters->TimeDelta = ((cmd[5] << 8) | cmd[6]) / TASK_PERIOD;
xxxx:
            parameters->InputType = cmd[1] - 1u;
            parameters->Srv2Move = cmd[2];
            parameters->StartTime = ((cmd[3] << 8) | cmd[4]) / TASK_PERIOD;
            parameters->NofCycles = cmd[7];
            for (i = 0; i < 6; ++i) {
                parameters->MaxValue[i] = (cmd[8 + i] << 1);
            }
            for (i = 0; i < 6; ++i) {
                parameters->MinValue[i] = (cmd[14 + i] << 1);
            }
            for (i = 0; i < 6; ++i) {
                parameters->Sign[i] = cmd[20 + i];
            }
            parameters->GenerateInput_Flag = 1;
            parameters->cnt = 0u;
            break;
        case 6:
            break;
    }
}

#endif /*AC_MODEL || AEROCOMP*/