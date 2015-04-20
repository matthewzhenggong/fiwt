/*
 * File:   msg_recv.c
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

#include "config.h"

#if AC_MODEL || AEROCOMP

#include "msg_recv.h"
#include "msg_code.h"
#include "servoTask.h"
#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"
#include "Servo.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

void msgRecvInit(msgRecvParam_p parameters, XBee_p xbee,
        TaskHandle_p senTask, TaskHandle_p servoTask,
        TaskHandle_p ekfTask, TaskHandle_p sendTask) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee = xbee;
    parameters->serov_Task = servoTask;
    parameters->sen_Task = senTask;
    parameters->ekf_Task = ekfTask;
    parameters->send_Task = sendTask;
    parameters->rx_cnt = 0u;
    parameters->tx_cnt = 0u;
    parameters->cnt = 0u;

    memcpy(parameters->tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
    parameters->tx_req._addr16 = 0xFFFE;
    parameters->tx_req._broadcastRadius = 1u;
    parameters->tx_req._option = 1u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';
}

size_t updateBattPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
#if AEROCOMP
    *(pack++) = CODE_AEROCOMP_BAT_LEV;
#else
    *(pack++) = CODE_AC_MODEL_BAT_LEV;
#endif
    for (i = 0; i < BATTCELLADCNUM; ++i) {
        *(pack++) = BattCell[i];
    }
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack - head;
}

size_t updateCommPack(TaskHandle_p task, TaskHandle_p sen_Task,
TaskHandle_p serov_Task, TaskHandle_p ekf_Task, TaskHandle_p send_Task, uint8_t head[]) {
    uint8_t *pack;
    pack = head;
#if AEROCOMP
    *(pack++) = CODE_AEROCOMP_COM_STATS;
#else
    *(pack++) = CODE_AC_MODEL_COM_STATS;
#endif
    *(pack++) = task->load_max >> 8;
    *(pack++) = task->load_max & 0xFF;
    *(pack++) = sen_Task->load_max >> 8;
    *(pack++) = sen_Task->load_max & 0xFF;
    *(pack++) = serov_Task->load_max >> 8;
    *(pack++) = serov_Task->load_max & 0xFF;
#if AC_MODEL
    *(pack++) = ekf_Task->load_max >> 8;
    *(pack++) = ekf_Task->load_max & 0xFF;
#endif
    *(pack++) = send_Task->load_max >> 8;
    *(pack++) = send_Task->load_max & 0xFF;

    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
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
        case 5:
        case 7:
        case 8:
        case 9:
            parameters->InputType = cmd[1] - 1u;
            parameters->Srv2Move = cmd[2];
            parameters->StartTime = ((cmd[3] << 8) | cmd[4]) / 5;
            if (parameters->InputType == 4) {
                parameters->TimeDelta = (uint32_t)((cmd[5] << 8) | cmd[6])*8 / 5;
            } else {
                parameters->TimeDelta = ((cmd[5] << 8) | cmd[6]) / 5;
            }
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

PT_THREAD(msgRecvLoop)(TaskHandle_p task) {
    int packin;
    msgRecvParam_p parameters;
    struct pt *pt;
    bool sent;

    parameters = (msgRecvParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        packin = XBeeReadPacket(parameters->_xbee);
        sent = false;
        if (packin > 0) {
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(parameters->_xbee, &parameters->rx_rsp)) {
                        ++parameters->rx_cnt;
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case CODE_AC_MODEL_NEW_SERV_CMD:
                            case CODE_AEROCOMP_NEW_SERV_CMD:
                                servoProcA5Cmd((servoParam_p) (parameters->serov_Task->parameters), parameters->rx_rsp._payloadPtr);
                                break;
                            case 'P':
                                parameters->tx_req._payloadLength = parameters->rx_rsp._payloadLength;
                                memcpy(parameters->tx_req._addr64, parameters->rx_rsp._addr64, 8);
                                parameters->tx_req._addr16 = parameters->rx_rsp._addr16;
                                memcpy(parameters->tx_req._payloadPtr, parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength);
                                XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req, 0u);
                                sent = true;
                                break;
                        }
                    }
                    break;
            }
        }
        if (!sent) {
            if ((parameters->cnt & 0x1FF) == 200) {
                parameters->tx_req._payloadLength = updateBattPack(parameters->tx_req._payloadPtr);
                XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req, 0u);
            } else if ((parameters->cnt & 0x1FF) == 400) {
                parameters->tx_req._payloadLength = updateCommPack(task, parameters->sen_Task, 
                        parameters->serov_Task,  parameters->ekf_Task, parameters->send_Task,
                        parameters->tx_req._payloadPtr);
                XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req, 0u);
            }
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /* AC_MODEL */
