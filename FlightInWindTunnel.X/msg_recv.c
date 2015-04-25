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

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

void msgRecvInit(msgRecvParam_p parameters, XBee_p xbee, XBeeSeries_t xbee_type,
        TaskHandle_p servoTask, msgSendParam_p sendParameters) {
    struct pt *pt;

    parameters->serov_Task = servoTask;
    parameters->sendParameters = sendParameters;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee = xbee;
    parameters->cnt = 0u;

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
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*8UL / 5;
            goto xxxx;
        case 8:
        case 9:
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*4UL / 5;
            goto xxxx;
yyyy:
            parameters->TimeDelta = ((cmd[5] << 8) | cmd[6]) / 5;
xxxx:
            parameters->InputType = cmd[1] - 1u;
            parameters->Srv2Move = cmd[2];
            parameters->StartTime = ((cmd[3] << 8) | cmd[4]) / 5;
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

void process_msg(msgRecvParam_p parameters, const uint8_t *msg_ptr, size_t msg_len) {
    switch (msg_ptr[0]) {
        case CODE_AC_MODEL_NEW_SERV_CMD:
        case CODE_AEROCOMP_NEW_SERV_CMD:
            servoProcA5Cmd((servoParam_p) (parameters->serov_Task->parameters), msg_ptr);
            break;
        case 'P':
            if (msg_len <= 256) {
                parameters->sendParameters->msg_len = msg_len;
                memcpy(parameters->sendParameters->msg_buff, msg_ptr, msg_len);
            }
            break;
    }
}

PT_THREAD(msgRecvLoop)(TaskHandle_p task) {
    int packin;
    msgRecvParam_p parameters;
    struct pt *pt;

    parameters = (msgRecvParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        packin = XBeeReadPacket(parameters->_xbee);
        if (packin > 0) {
            switch (packin) {
                case RX_IPV4_RESPONSE:
                    if (XBeeRxIPv4Response(parameters->_xbee, &parameters->rx_rsp.rxipv4)) {
                        ++parameters->cnt;
                        process_msg(parameters, parameters->rx_rsp.rxipv4._payloadPtr, parameters->rx_rsp.rxipv4._payloadLength);
                    }
                    break;
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(parameters->_xbee, &parameters->rx_rsp.zbrx)) {
                        ++parameters->cnt;
                        process_msg(parameters, parameters->rx_rsp.zbrx._payloadPtr, parameters->rx_rsp.zbrx._payloadLength);
                    }
                    break;
                case RX_A64_RESPONSE:
                    if (XBeeRxA64Response(parameters->_xbee, &parameters->rx_rsp.rxa64)) {
                        ++parameters->cnt;
                        process_msg(parameters, parameters->rx_rsp.rxa64._payloadPtr, parameters->rx_rsp.rxa64._payloadLength);
                    }
                    break;
                case RX_A16_RESPONSE:
                    if (XBeeRxA16Response(parameters->_xbee, &parameters->rx_rsp.rxa16)) {
                        ++parameters->cnt;
                        process_msg(parameters, parameters->rx_rsp.rxa16._payloadPtr, parameters->rx_rsp.rxa16._payloadLength);
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
