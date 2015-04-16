/*
 * File:   msg_send.c
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

#include "msg_send.h"
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

void msgSendInit(msgSendParam_p parameters, XBee_p xbee) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee = xbee;
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
#if AEROCOMP
    *(pack++) = CODE_AC_MODEL_SERVO_POS;
#else 
    *(pack++) = CODE_AEROCOMP_SERVO_POS;
#endif
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
    for (i=0;i<SEVERONUM;++i) {
      *(pack++) = Servos[i].Ctrl >> 8;
      *(pack++) = Servos[i].Ctrl & 0xFF;
    }
    return pack-head;
}

PT_THREAD(msgSendLoop)(TaskHandle_p task) {
    msgSendParam_p parameters;
    struct pt *pt;

    parameters = (msgSendParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        parameters->tx_req._payloadLength = updateSensorPack(parameters->tx_req._payloadPtr);
        XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req, 0u);
        ++parameters->cnt;

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /* AC_MODEL */
