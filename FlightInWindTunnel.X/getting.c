/*
 * File:   getting.c
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

#include "getting.h"
#include "servoTask.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

extern servoParam_t servo;

void gettingInit(gettingParam_t *parameters, XBee_p xbee) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_xbee = xbee;

    memcpy(parameters->rx_echo._addr64, "\00\00\00\00\00\00\00\00", 8);
    parameters->rx_echo._addr16 = 0xFFFE;
    parameters->rx_echo._broadcastRadius = 1u;
    parameters->rx_echo._option = 1u;
    parameters->rx_echo._payloadLength = 1u;
    parameters->rx_echo._payloadPtr[0] = 'P';
}

PT_THREAD(gettingLoop)(TaskHandle_p task) {
    int packin;
    gettingParam_t *parameters;
    struct pt *pt;
    parameters = (gettingParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        packin = XBeeReadPacket(parameters->_xbee);
        if (packin > 0) {
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(parameters->_xbee, &parameters->rx_rsp)) {
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\x02':
                            {
                                servo._ch = parameters->rx_rsp._payloadPtr[2];
                                servo._dc = parameters->rx_rsp._payloadPtr[3]+((uint16_t)(parameters->rx_rsp._payloadPtr[4]) << 8);
                                servo._peroid = parameters->rx_rsp._payloadPtr[5];
                                servo._loop = parameters->rx_rsp._payloadPtr[6];
                                servo._ticks = servo._peroid;
                            }
                                break;
                            default:
                            {
                                parameters->rx_echo._addr16 = parameters->rx_rsp._addr16;
                                memcpy(parameters->rx_echo._addr64, parameters->rx_rsp._addr64, 8);
                                parameters->rx_echo._broadcastRadius = 1u;
                                parameters->rx_echo._option = 1u;
                                parameters->rx_echo._payloadLength = parameters->rx_rsp._payloadLength;
                                memcpy(parameters->rx_echo._payloadPtr, parameters->rx_rsp._payloadPtr,
                                        parameters->rx_rsp._payloadLength);
                                XBeeZBTxRequest(parameters->_xbee, &(parameters->rx_echo), 0u);
                            }
                        }
                    }
                    break;
                default:
                    ;
            }
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}