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
#include "msg_gnd.h"

#include "LedExtBoard.h"
#include "clock.h"
#include <string.h>
#include <stdlib.h>

float windtunnel_wind_speed = 0; // m/s
float windtunnel_dynamic_pressure = 0; // pa

static uint8_t Velocity_head[] = "Velocity\",";
static uint8_t DP_head[] = "D.P.\",";

static char vel_buff[32];
static int flag, idx;

static bool process_data(uint8_t *msg, size_t msg_len) {
    size_t i;
    uint8_t c;
    bool rslt;

    rslt = false;
    for (i = 0; i < msg_len; ++i) {
        c = *(msg++);
        switch (flag) {
            case 0:
                if (c == Velocity_head[0]) {
                    flag = 10;
                    idx = 1;
                } else if (c == DP_head[0]) {
                    flag = 20;
                    idx = 1;
                }
                break;
            case 10:
                if (c == Velocity_head[idx++]) {
                    if (idx == 10) {
                        flag = 11;
                        idx = 0;
                    }
                } else {
                    flag = 0;
                }
                break;
            case 11:
                switch (c) {
                    case ',':
                        vel_buff[idx] = '\0';
                        windtunnel_wind_speed = (float) atof(vel_buff);
                        flag = 0;
                        break;
                    case ' ':
                    case '\t':
                        break;
                    case '+':
                    case '-':
                    case '.':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        vel_buff[idx++] = c;
                        break;
                    default :
                        flag = 0;
                }
                break;
            case 20:
                if (c == Velocity_head[idx++]) {
                    if (idx == 6) {
                        flag = 21;
                        idx = 0;
                    }
                } else {
                    flag = 0;
                }
                break;
            case 21:
                switch (c) {
                    case ',':
                        vel_buff[idx] = '\0';
                        windtunnel_dynamic_pressure = (float) atof(vel_buff);
                        flag = 0;
                        break;
                    case ' ':
                    case '\t':
                        break;
                    case '+':
                    case '-':
                    case '.':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        vel_buff[idx++] = c;
                        break;
                    default :
                        flag = 0;
                }
                break;
            default:
                flag = 0;

        }
    }
    return rslt;
}

void remoteSenInit(remoteSenParam_p parameters, XBee_p s2) {
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

    windtunnel_wind_speed = 0; // m/s
    windtunnel_dynamic_pressure = 0; // pa
    flag = 0;
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
                        if (process_data(parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength)) {
                            sendSpeedPack();
#if USE_LEDEXTBOARD
                            mLED_2_Toggle();
#endif
                        }
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

