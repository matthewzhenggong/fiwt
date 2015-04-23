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
#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"
#include "Servo.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#if USE_EKF
#include "ekfTask.h"
extern ekfParam_t ekf;
#endif

void msgSendInit(msgSendParam_p parameters, XBee_p xbee, XBeeSeries_t xbee_type,
        TaskHandle_p recvTask, TaskHandle_p senTask, TaskHandle_p servoTask,
        TaskHandle_p ekfTask) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee = xbee;
    parameters->cnt = 0u;

    parameters->serov_Task = servoTask;
    parameters->sen_Task = senTask;
    parameters->ekf_Task = ekfTask;
    parameters->recv_Task = recvTask;

    parameters->xbee_type = xbee_type;
    switch (xbee_type) {
        case XBeeS1:
            memcpy(parameters->tx_req.txa64._addr64, MSG_TARGET_ADDR, 8);
            parameters->tx_req.txa64._option = 1u;
            parameters->tx_req.txa64._payloadLength = 1u;
            parameters->tx_req.txa64._payloadPtr[0] = 'P';
            break;
        default:
            memcpy(parameters->tx_req.zbtx._addr64, MSG_TARGET_ADDR, 8);
            parameters->tx_req.zbtx._addr16 = 0xFFFE;
            parameters->tx_req.zbtx._broadcastRadius = 1u;
            parameters->tx_req.zbtx._option = 1u;
            parameters->tx_req.zbtx._payloadLength = 1u;
            parameters->tx_req.zbtx._payloadPtr[0] = 'P';
    }

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

size_t updateCommPack(TaskHandle_p msg_recv_Task, TaskHandle_p sen_Task,
        TaskHandle_p serov_Task, TaskHandle_p task, TaskHandle_p ekf_Task, uint8_t head[]) {
    uint8_t *pack;
    pack = head;
#if AEROCOMP
    *(pack++) = CODE_AEROCOMP_COM_STATS;
#else
    *(pack++) = CODE_AC_MODEL_COM_STATS;
#endif
    *(pack++) = msg_recv_Task->load_max >> 8;
    *(pack++) = msg_recv_Task->load_max & 0xFF;
    *(pack++) = sen_Task->load_max >> 8;
    *(pack++) = sen_Task->load_max & 0xFF;
    *(pack++) = serov_Task->load_max >> 8;
    *(pack++) = serov_Task->load_max & 0xFF;
    *(pack++) = task->load_max >> 8;
    *(pack++) = task->load_max & 0xFF;
#if AC_MODEL
    *(pack++) = ekf_Task->load_max >> 8;
    *(pack++) = ekf_Task->load_max & 0xFF;
#endif

    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    return pack - head;
}

size_t updateSensorPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
#if USE_EKF
    uint8_t *ptr;
#endif
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
    *(pack++) = ADC_TimeStamp[0] & 0xFF;
    *(pack++) = ADC_TimeStamp[1] >> 8;
    *(pack++) = ADC_TimeStamp[1] & 0xFF;
    for (i = 0; i < SEVERONUM; ++i) {
        *(pack++) = Servos[i].Ctrl >> 8;
        *(pack++) = Servos[i].Ctrl & 0xFF;
    }
#if USE_EKF
    ptr = (uint8_t *) (ekf.ekff.rpy);
    *(pack++) = *(ptr + 3);
    *(pack++) = *(ptr + 2);
    *(pack++) = *(ptr + 1);
    *(pack++) = *(ptr);
    *(pack++) = *(ptr + 7);
    *(pack++) = *(ptr + 6);
    *(pack++) = *(ptr + 5);
    *(pack++) = *(ptr + 4);
#else
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
    *(pack++) = 0;
#endif
    return pack - head;
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
        ++parameters->cnt;
        switch (parameters->xbee_type) {
            case XBeeS1:
                if ((parameters->cnt & 0x1FF) == 200) {
                    parameters->tx_req.txa64._payloadLength = updateBattPack(parameters->tx_req.txa64._payloadPtr);
                    XBeeTxA64Request(parameters->_xbee, &parameters->tx_req.txa64, 0u);
                } else if ((parameters->cnt & 0x1FF) == 400) {
                    parameters->tx_req.txa64._payloadLength = updateCommPack(
                            parameters->recv_Task, parameters->sen_Task,
                            parameters->serov_Task, task, parameters->ekf_Task,
                            parameters->tx_req.txa64._payloadPtr);
                    XBeeTxA64Request(parameters->_xbee, &parameters->tx_req.txa64, 0u);
                } else {
                    parameters->tx_req.txa64._payloadLength = updateSensorPack(parameters->tx_req.txa64._payloadPtr);
                    XBeeTxA64Request(parameters->_xbee, &parameters->tx_req.txa64, 0u);
                }
                break;
            default:
                if ((parameters->cnt & 0x1FF) == 200) {
                    parameters->tx_req.zbtx._payloadLength = updateBattPack(parameters->tx_req.zbtx._payloadPtr);
                    XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req.zbtx, 0u);
                } else if ((parameters->cnt & 0x1FF) == 400) {
                    parameters->tx_req.zbtx._payloadLength = updateCommPack(
                            parameters->recv_Task, parameters->sen_Task,
                            parameters->serov_Task, task, parameters->ekf_Task,
                            parameters->tx_req.zbtx._payloadPtr);
                    XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req.zbtx, 0u);
                } else {
                    parameters->tx_req.zbtx._payloadLength = updateSensorPack(parameters->tx_req.zbtx._payloadPtr);
                    XBeeZBTxRequest(parameters->_xbee, &parameters->tx_req.zbtx, 0u);
                }
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /* AC_MODEL */
