/*
 * File:   remoteSenTask.c
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

#if GNDBOARD

#include "remoteSenTask.h"

#include "LedExtBoard.h"
#include <string.h>

void remoteSenInit(remoteSenParam_p parameters, msgParam_p msg, XBee_p s2) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->xbee = s2;
    memcpy(parameters->tx_req._addr64, "\x00\x13\xA2\x00\x40\x8A\x72\xDB", 8);
    parameters->tx_req._addr16 = 0xFFFE;
    parameters->tx_req._broadcastRadius = 0;
    parameters->tx_req._option = 0x01;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'T';

    parameters->cnt = 0u;
    parameters->rx_cnt = 0u;
    parameters->msg = msg;
}

PT_THREAD(remoteSenLoop)(TaskHandle_p task) {
    int packin;
    remoteSenParam_p parameters;
    struct pt *pt;

    parameters = (remoteSenParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        packin = XBeeReadPacket(parameters->xbee);
        while (packin > 0) {
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(parameters->xbee, &parameters->rx_rsp)) {
                        ++parameters->rx_cnt;
                        *(parameters->msg->msg_buff) = 'T';
                        memcpy(parameters->msg->msg_buff+1, parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength);
                        parameters->msg->msg_len = parameters->rx_rsp._payloadLength+1;
#if USE_LEDEXTBOARD
                        mLED_2_Toggle();
#endif
                    }
                    break;
            }
            packin = XBeeReadPacket(parameters->xbee);
        }

        ++parameters->cnt;
        if ((parameters->cnt & 0x7) == 0) {
            XBeeZBTxRequest(parameters->xbee, &parameters->tx_req, 0u);
        }

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}


#endif /*GNDBOARD*/

