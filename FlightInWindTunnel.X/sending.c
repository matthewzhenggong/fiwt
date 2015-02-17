/*
 * File:   sending.c
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

#include "sending.h"
#include "idle.h"
#include "clock.h"
#include "AnalogInput.h"
#include "Servo.h"

#include <stddef.h>
#include <string.h>

void sendingInit(sendingParam_t *parameters, XBee_p xbee1, XBee_p xbee2) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->cnt = 0u;
    parameters->_xbee[0] = xbee2;
    parameters->_xbee[1] = xbee1;

    memcpy(parameters->tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
    parameters->tx_req._addr16 = 0xFFFE;
    parameters->tx_req._broadcastRadius = 1u;
    parameters->tx_req._option = 1u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';
}

struct SensorPack {
    clockType_t time;
    unsigned int ServoPos[SERVOPOSADCNUM];
    unsigned int BattCell[BATTCELLADCNUM];
    signed int ServoCtrl[SERVOPOSADCNUM];
    uint16_t loadmax[2];
};

extern TaskHandle_p servoTask;

PT_THREAD(sendingLoop)(TaskHandle_p task) {
    sendingParam_t *parameters;
    struct pt *pt;
    struct SensorPack *pack;
    parameters = (sendingParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        UpdateAnalogInputs();
        parameters->tx_req._payloadPtr[0] = '\x06';
        parameters->tx_req._payloadPtr[1] = '\x00';
        pack = (struct SensorPack *) (parameters->tx_req._payloadPtr + 2u);
        pack->time = AnalogInputTimeStamp;
        memcpy(pack->ServoPos, ServoPos, sizeof(ServoPos));
        memcpy(pack->BattCell, BattCell, sizeof(BattCell));
        pack->ServoCtrl[0] = Servos[0].Ctrl;
        pack->loadmax[0] = task->load_max;
        pack->loadmax[1] = servoTask->load_max;
        parameters->tx_req._payloadLength = sizeof(struct SensorPack)+2u;

        if (parameters->cnt & 1) {
            XBeeZBTxRequest(parameters->_xbee[0], &parameters->tx_req, 0u);
        } else {
            XBeeZBTxRequest(parameters->_xbee[1], &parameters->tx_req, 0u);
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}


