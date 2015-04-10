/*
 * File:   msg_comp.c
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

#if AEROCOMP

#include "msg_comp.h"
#include "servoTask.h"
#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

void msgInit(msgParam_p parameters, XBee_p xbee1, XBee_p xbee2, TaskHandle_p servoTask) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee[0] = xbee1;
    parameters->_xbee[1] = xbee2;
    parameters->serov_Task = servoTask;
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
    *(pack++) = 0x33;
    for (i=0;i<SERVOPOSADCNUM;++i) {
      *(pack++) = ServoPos[i] >> 8;
      *(pack++) = ServoPos[i] & 0xFF;
    }
    for (i=0;i<ENCNUM;++i) {
      *(pack++) = EncPos[i] >> 8;
      *(pack++) = EncPos[i] & 0xFF;
    }
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

size_t updateBattPack(uint8_t head[]){
    uint8_t *pack;
    int i;
    pack = head;
    *(pack++) = 0x99;
    for (i=0;i<BATTCELLADCNUM;++i) {
      *(pack++) = BattCell[i] >> 8;
      *(pack++) = BattCell[i] & 0xFF;
    }
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

size_t updateCommPack(TaskHandle_p task, TaskHandle_p serov_Task, uint8_t head[]){
    uint8_t *pack;
    pack = head;
    *(pack++) = 0x78;
    *(pack++) = serov_Task->load_max >> 8;
    *(pack++) = serov_Task->load_max & 0xFF;
    *(pack++) = task->load_max >> 8;
    *(pack++) = task->load_max & 0xFF;

    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack-head;
}

extern void servoProcA5Cmd(servoParam_p parameters, const uint8_t cmd[]);

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    XBee_p _xbee;
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
            parameters->tx_req._payloadLength = updateCommPack(task, parameters->serov_Task, parameters->tx_req._payloadPtr);
        } else {
            parameters->tx_req._payloadLength = updateSensorPack(parameters->tx_req._payloadPtr);
        }
        if (parameters->cnt & 1) {
            XBeeZBTxRequest(parameters->_xbee[0], &parameters->tx_req, 0u);
        } else {
            XBeeZBTxRequest(parameters->_xbee[1], &parameters->tx_req, 0u);
        }

        packin = 0;
        if (parameters->_xbee[0]) {
            packin = XBeeReadPacket(parameters->_xbee[0]);
            _xbee = parameters->_xbee[0];
        }
        if (packin <= 0 && parameters->_xbee[1]) {
            packin = XBeeReadPacket(parameters->_xbee[1]);
            _xbee = parameters->_xbee[1];
        }
        if (packin > 0) {
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(_xbee, &parameters->rx_rsp)) {
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\xa5':
                            case '\xa6':
                                servoProcA5Cmd(servo, parameters->rx_rsp._payloadPtr);
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

#endif /* AEROCOMP*/

