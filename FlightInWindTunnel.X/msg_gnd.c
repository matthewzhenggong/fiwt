/*
 * File:   msg_gnd.c
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

#include "msg_gnd.h"
#include "LedExtBoard.h"
#include "SPIS.h"
#include <string.h>

void msgInit(msgParam_t *parameters, XBee_p xb1, XBee_p xb2, XBee_p xb3, XBee_p xb4) {
    struct pt *pt;
    int i;

    pt = &(parameters->PT);
    PT_INIT(pt);

    parameters->node[0].xbee = xb1;
    parameters->node[1].xbee = xb2;
    parameters->node[2].xbee = xb3;
    parameters->node[3].xbee = xb4;
    for (i = 0; i < 4; ++i) {
        //memcpy(parameters->node[i].tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
        parameters->node[i].tx_req._addr16 = 0xFFFF;
        parameters->node[i].tx_req._broadcastRadius = 1u;
        parameters->node[i].tx_req._option = 1u;
        //parameters->node[i].tx_req._payloadLength = 1u;
        //parameters->node[i].tx_req._payloadPtr[0] = 'P';
    }

    parameters->nodeAC[0] = NULL;
    parameters->nodeAC[1] = NULL;
    parameters->nodeCOMP[0] = NULL;
    parameters->nodeCOMP[1] = NULL;

    parameters->tx_cnt = 0u;
    parameters->rx_cnt = 0u;
}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int i;
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    RemoteNode *node;
    enum XbeeGroup flag;

    parameters = (msgParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {

        if (SPIRX_RX_PCKT_PTR) {
            node = NULL;
            switch (SPIRX_RX_PCKT_PTR->RF_DATA[0]) {
                case 0xa5:
                    if (parameters->tx_cnt & 1 && parameters->nodeAC[0]) {
                        node = parameters->nodeAC[0];
                    } else if (parameters->nodeAC[1]) {
                        node = parameters->nodeAC[1];
                    }
                    mLED_2_Toggle();
                    break;
                case 0xa6:
                    if (parameters->tx_cnt & 1 && parameters->nodeCOMP[0]) {
                        node = parameters->nodeCOMP[0];
                    } else if (parameters->nodeCOMP[1]) {
                        node = parameters->nodeCOMP[1];
                    }
                    mLED_3_Toggle();
                    break;
            }
            if (node) {
                ++parameters->tx_cnt;
                node->tx_req._payloadLength = (SPIRX_RX_PCKT_PTR->PCKT_LENGTH_MSB << 8) + SPIRX_RX_PCKT_PTR->PCKT_LENGTH_LSB;
                memcpy(node->tx_req._payloadPtr, SPIRX_RX_PCKT_PTR->RF_DATA, node->tx_req._payloadLength);
                SPIRX_RX_PCKT_PTR = NULL; //clear for sent
                XBeeZBTxRequest(parameters->node[0].xbee, &parameters->node[0].tx_req, 0u);
            }
        }

        ++parameters->rx_cnt;
        if ((parameters->rx_cnt & 0x1FE) == 0xFE) {
            SPIS_push((const uint8_t *) "\x79hello from ground board.", 15);
        }
        for (i = 0; i < 4; ++i) {
            node = parameters->node + i;
            packin = XBeeReadPacket(node->xbee);
            switch (packin) {
                case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(node->xbee, &parameters->rx_rsp)) {
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\x22':
                                flag = XB_AC;
                                mLED_4_Toggle();
                                break;
                            case '\x33':
                                flag = XB_COMP;
                                mLED_5_Toggle();
                                break;
                            case '\x88':
                                flag = XB_AC;
                                break;
                            case '\x99':
                                flag = XB_COMP;
                                break;
                            case '\x77':
                                flag = XB_AC;
                                break;
                            case '\x78':
                                flag = XB_COMP;
                                break;
                            default:
                                flag = XB_NON;
                        }
                        if (flag != XB_NON && node->tx_req._addr16 == 0xFFFF) {
                            memcpy(node->tx_req._addr64, parameters->rx_rsp._addr64, 8);
                            node->tx_req._addr16 = parameters->rx_rsp._addr16;
                            if (flag == XB_AC) {
                                if (!parameters->nodeAC[1]) {
                                    parameters->nodeAC[1] = node;
                                    mLED_6_On();
                                } else if (!parameters->nodeAC[0]) {
                                    parameters->nodeAC[0] = node;
                                    mLED_7_On();
                                }
                            } else if (flag == XB_COMP) {
                                if (!parameters->nodeCOMP[0]) {
                                    parameters->nodeCOMP[0] = node;
                                } else if (!parameters->nodeCOMP[1]) {
                                    parameters->nodeCOMP[1] = node;
                                }
                            }
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

#endif /* GNDBOARD */
