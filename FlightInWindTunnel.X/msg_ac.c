/*
 * File:   msg_ac.c
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

#if AC_MODEL

#include "msg_ac.h"
#include "servoTask.h"
#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

void msgInit(msgParam_p parameters, XBee_p xbee1, XBee_p xbee2,
        TaskHandle_p servoTask, TaskHandle_p senTask) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee[0] = xbee1;
    parameters->_xbee[1] = xbee2;
    parameters->serov_Task = servoTask;
    parameters->sen_Task = senTask;
    parameters->cnt = 0u;

    memcpy(parameters->tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
    parameters->tx_req._addr16 = 0xFFFE;
    parameters->tx_req._broadcastRadius = 1u;
    parameters->tx_req._option = 1u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';
}


size_t updateSensorPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
    *(pack++) = 0x22;
    for (i=0;i<SERVOPOSADCNUM;++i) {
      *(pack++) = ServoPos[i] >> 8;
      *(pack++) = ServoPos[i] & 0xFF;
    }
    for (i=0;i<ENCNUM;++i) {
      *(pack++) = EncPos[i] >> 8;
      *(pack++) = EncPos[i] & 0xFF;
    }
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
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

size_t updateBattPack(uint8_t head[]){
    uint8_t *pack;
    int i;
    pack = head;
    *(pack++) = 0x88;
    for (i=0;i<BATTCELLADCNUM;++i) {
      *(pack++) = BattCell[i] >> 8;
      *(pack++) = BattCell[i] & 0xFF;
    }
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

size_t updateCommPack(TaskHandle_p task, TaskHandle_p sen_Task, TaskHandle_p serov_Task, uint8_t head[]){
    uint8_t *pack;
    pack = head;
    *(pack++) = 0x77;
    *(pack++) = sen_Task->load_max >> 8;
    *(pack++) = sen_Task->load_max & 0xFF;
    *(pack++) = serov_Task->load_max >> 8;
    *(pack++) = serov_Task->load_max & 0xFF;
    *(pack++) = task->load_max >> 8;
    *(pack++) = task->load_max & 0xFF;

    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

void servoProcA5Cmd(servoParam_p parameters, const uint8_t cmd[]) {
    int i;

    if (cmd[0] == '\xA5' || cmd[0] == '\xA6') {
        switch (cmd[1]) {
            case 1 :
                for (i=0;i<SEVERONUM;++i) {
                    parameters->Servo_PrevRef[i] = parameters->ServoRef[i] = ((cmd[2+i*2]<<8) | cmd[2+i*2+1]);
                }
                parameters->InputType = 0;
                parameters->StartTime = 100;
                parameters->TimeDelta = 100;
                parameters->NofCycles = 1;
                parameters->Srv2Move = 0xFF;
                parameters->GenerateInput_Flag = 1;
                parameters->cnt = 0u;
                break;
            case 2 :
            case 3 :
            case 4 :
            case 5 :
            case 7 :
                parameters->InputType = cmd[1] - 1u;
                parameters->Srv2Move = cmd[2];
                parameters->StartTime = ((cmd[3] << 8) | cmd[4])/10;
                parameters->TimeDelta = ((cmd[5] << 8) | cmd[6])/10;
                parameters->NofCycles = cmd[7];
                for (i=0;i<6;++i) {
                    parameters->MaxValue[i] = (cmd[8+i]<<1);
                }
                for (i=0;i<6;++i) {
                    parameters->MinValue[i] = (cmd[14+i]<<1);
                }
                for (i=0;i<6;++i) {
                    parameters->Sign[i] = cmd[20+i];
                }
                parameters->GenerateInput_Flag = 1;
                parameters->cnt = 0u;
                break;
            case 6 :
                break;
        }
    }
}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    XBee_p _xbee_in;
    XBee_p _xbee_out;
    servoParam_p servo;

    parameters = (msgParam_p) (task->parameters);
    servo = (servoParam_p) (parameters->serov_Task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        if ((parameters->cnt & 0x1FE) == 0xFE) {
            parameters->tx_req._payloadLength = updateBattPack(parameters->tx_req._payloadPtr);
        } else if ((parameters->cnt & 0x1FE) == 0x1FE) {
            parameters->tx_req._payloadLength = updateCommPack(task, parameters->sen_Task, parameters->serov_Task, parameters->tx_req._payloadPtr);
        } else {
            parameters->tx_req._payloadLength = updateSensorPack(parameters->tx_req._payloadPtr);
        }
        if (parameters->cnt & 1) {
            XBeeZBTxRequest(parameters->_xbee[0], &parameters->tx_req, 0u);
            _xbee_out = parameters->_xbee[1];
        } else {
            XBeeZBTxRequest(parameters->_xbee[1], &parameters->tx_req, 0u);
            _xbee_out = parameters->_xbee[0];
        }

        packin = XBeeReadPacket(parameters->_xbee[0]);
        _xbee_in = parameters->_xbee[0];
        if (packin <= 0) {
            packin = XBeeReadPacket(parameters->_xbee[1]);
            _xbee_in = parameters->_xbee[1];
        }
        if (packin > 0) {
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(_xbee_in, &parameters->rx_rsp)) {
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\xa5':
                            case '\xa6':
                                servoProcA5Cmd(servo, parameters->rx_rsp._payloadPtr);
                                break;
                            case 'P' :
                                parameters->tx_req._payloadLength = parameters->rx_rsp._payloadLength;
                                memcpy(parameters->tx_req._payloadPtr ,  parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength);
                                XBeeZBTxRequest(_xbee_out, &parameters->tx_req, 0u);
                                break;
                        }
                    }
                    break;
            }
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /* AC_MODEL */
