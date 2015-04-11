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
    
    for (i=0;i<4;++i) {
        memcpy(parameters->node[i].tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
        parameters->node[i].tx_req._addr16 = 0xFFFE;
        parameters->node[i].tx_req._broadcastRadius = 1u;
        parameters->node[i].tx_req._option = 1u;
        parameters->node[i].tx_req._payloadLength = 1u;
        parameters->node[i].tx_req._payloadPtr[0] = 'P';
        parameters->node[i].valid = false;
    }
}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int i;
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    RemoteNode *node;

    parameters = (msgParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        if (SPIRX_RX_PCKT_PTR) {
            node = NULL;
            switch (SPIRX_RX_PCKT_PTR->RF_DATA[0]) {
                case  0xa5 :
                    if (parameters->cnt & 1 && parameters->node[0].valid) {
                        node = parameters->node;
                    } else if (parameters->node[1].valid) {
                        node = parameters->node+1;
                    }
                    break;
                case 0xa6 :
                    if (parameters->cnt & 1 && parameters->node[2].valid) {
                        node = parameters->node+2;
                    } else if (parameters->node[3].valid) {
                        node = parameters->node+3;
                    }
                    break;
            }
            if (node) {
                node->tx_req._payloadLength = (SPIRX_RX_PCKT_PTR->PCKT_LENGTH_MSB << 8) + SPIRX_RX_PCKT_PTR->PCKT_LENGTH_LSB;
                memcpy(node->tx_req._payloadPtr, SPIRX_RX_PCKT_PTR->RF_DATA, node->tx_req._payloadLength);
                SPIRX_RX_PCKT_PTR = NULL; //clear for sent
                XBeeZBTxRequest(parameters->node[0].xbee, &parameters->node[0].tx_req, 0u);
            }
        }
        if ((parameters->cnt & 0x1FE) == 0xFE) {
            SPIS_push((const uint8_t *)"\x79hello from ground board.",15);
        }

        for (i=0;i<4; ++i) {
                 node = parameters->node+i;
                packin = XBeeReadPacket(node->xbee);
                switch (packin) {
                    case ZB_RX_RESPONSE:
                    if (XBeeZBRxResponse(node->xbee, &parameters->rx_rsp)) {
                        if (memcmp(parameters->rx_rsp._addr64, node->tx_req._addr64,8)) {
                            memcpy(node->tx_req._addr64, parameters->rx_rsp._addr64, 8);
                            node->tx_req._addr16 = parameters->rx_rsp._addr16;
                            node->valid = true;
                        }
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\x22':
                                break;
                            case '\x33':
                                break;
                            case '\x88':
                                break;
                            case '\x99':
                                break;
                            case '\x77':
                                break;
                            case '\x78':
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

#endif /* GNDBOARD */
