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

/* Data Identifier Codes */
// Servos Read Data
#define CODE_AC_MODEL_SERVO_POS		0x22
#define CODE_AEROCOMP_SERVO_POS		0x33

// Encoders Read Data
#define CODE_AC_MODEL_ENCOD_POS 	0x55
#define CODE_AEROCOMP_ENCOD_POS 	0x66

// Battery Read Data
#define CODE_AC_MODEL_BAT_LEV		0x88
#define CODE_AEROCOMP_BAT_LEV		0x99

// Low Battery codes
#define CODE_AC_MODEL_LOW_BAT		0xBB
#define CODE_AEROCOMP_LOW_BAT	 	0xCC

// IMU Read Data
#define CODE_AC_MODEL_IMU_DATA		0xEE

// Communication Statistics
#define CODE_AC_MODEL_COM_STATS		0x75
#define CODE_AEROCOMP_COM_STATS		0x76

// GNDBOARD Sensors Read
#define CODE_GNDBOARD_ADCM_READ		0x44
#define CODE_GNDBOARD_ENCOD_POS 	0x77
#define CODE_GNDBOARD_COM_STATS		0xAA

// Servos New Position
#define CODE_AC_MODEL_NEW_SERV_CMD	0xA5
#define CODE_AEROCOMP_NEW_SERV_CMD	0xA6

// Command Message
#define CODE_AC_MODEL_COMMAND		0x45
#define CODE_AEROCOMP_COMMAND		0x46
#define CODE_GNDBOARD_COMMAND		0x47

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
    uint8_t *spis_pkg_buff;

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
                    mLED_2_On();
                    break;
                case 0xa6:
                    if (parameters->tx_cnt & 1 && parameters->nodeCOMP[0]) {
                        node = parameters->nodeCOMP[0];
                    } else if (parameters->nodeCOMP[1]) {
                        node = parameters->nodeCOMP[1];
                    }
                    mLED_2_On();
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
                    spis_pkg_buff = parameters->spis_pkg_buff;
                    if (XBeeZBRxResponse(node->xbee, &parameters->rx_rsp)) {
                        switch (parameters->rx_rsp._payloadPtr[0]) {
                            case '\x22':
                                /* self.pack22 = struct.Struct(">B6H3H6HBH6h") */
                                /* 1+12+6+12+3+12 = 46 */
                                flag = XB_AC;
                                // DATA_ID == CODE_AC_MODEL_SERVO_POS
                                spis_pkg_buff[0] = CODE_AC_MODEL_SERVO_POS;
                                //2-13	Data		SERVOx_H, SERVOx_L
                                memcpy(spis_pkg_buff+1, parameters->rx_rsp._payloadPtr+1, 12);
                                //14-16	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+13, parameters->rx_rsp._payloadPtr+31, 3);
                                // DATA_ID ==  CODE_AC_MODEL_ENCOD_POS
                                spis_pkg_buff[16] = CODE_AC_MODEL_ENCOD_POS;
                                //2- 7	Data		DIGENCx_H, DIGENCx_L
                                memcpy(spis_pkg_buff+17, parameters->rx_rsp._payloadPtr+13, 6);
                                //8-10	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+23, parameters->rx_rsp._payloadPtr+31, 3);
                                //DATA_ID ==	CODE_AC_MODEL_IMU_DATA
                                spis_pkg_buff[26] = CODE_AC_MODEL_ENCOD_POS;
                                //2-21		Data    GxH,GxL,GyH,GyL...
                                memcpy(spis_pkg_buff+27+2, parameters->rx_rsp._payloadPtr+19, 12);
                                //22-24	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+47, parameters->rx_rsp._payloadPtr+31, 3);
                                SPIS_push(spis_pkg_buff, 50);
                                mLED_3_Toggle();
                                break;
                            case '\x33':
                                /* self.pack33 = struct.Struct(">B4H4HBH4h") */
                                /* 1+8+8+3+8 = 28 */
                                flag = XB_COMP;
                                //DATA_ID == 	CODE_AC_MODEL_SERVO_POS
                                spis_pkg_buff[0] = CODE_AEROCOMP_SERVO_POS;
                                //2- 9	Data		SERVOx_H, SERVOx_L
                                memcpy(spis_pkg_buff+1, parameters->rx_rsp._payloadPtr+1, 8);
                                //10-12	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+9, parameters->rx_rsp._payloadPtr+17, 3);
                                // DATA_ID ==  CODE_AEROCOMP_ENCOD_POS
                                spis_pkg_buff[12] = CODE_AEROCOMP_ENCOD_POS;
                                //2-9	Data		DIGENCx_H, DIGENCx_L
                                memcpy(spis_pkg_buff+13, parameters->rx_rsp._payloadPtr+9, 8);
                                //10-12	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+21, parameters->rx_rsp._payloadPtr+17, 3);
                                SPIS_push(spis_pkg_buff, 24);
                                mLED_4_Toggle();
                                break;
                            case '\x88':
                                /*self.pack88 = struct.Struct(">B3HBH")*/
                                /* 1+6+3 */
                                flag = XB_AC;
                                //DATA_ID == 	CODE_AC_MODEL_BAT_LEV
                                spis_pkg_buff[0] = CODE_AC_MODEL_BAT_LEV;
                                //2- 4	Data		BATT_C1, BATT_C2, BATT_C3
                                spis_pkg_buff[1] = parameters->rx_rsp._payloadPtr[2];
                                spis_pkg_buff[2] = parameters->rx_rsp._payloadPtr[4];
                                spis_pkg_buff[3] = parameters->rx_rsp._payloadPtr[6];
                                //5- 7	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+4, parameters->rx_rsp._payloadPtr+7, 3);
                                SPIS_push(spis_pkg_buff, 7);
                                break;
                            case '\x99':
                                /*self.pack88 = struct.Struct(">B3HBH")*/
                                /* 1+6+3 */
                                flag = XB_COMP;
                                //DATA_ID == 	CODE_AC_MODEL_BAT_LEV
                                spis_pkg_buff[0] = CODE_AEROCOMP_BAT_LEV;
                                //2- 4	Data		BATT_C1, BATT_C2, BATT_C3
                                spis_pkg_buff[1] = parameters->rx_rsp._payloadPtr[2];
                                spis_pkg_buff[2] = parameters->rx_rsp._payloadPtr[4];
                                spis_pkg_buff[3] = parameters->rx_rsp._payloadPtr[6];
                                //5- 7	Time Stamp	TimeStampH, TimeStampM, TimeStampL
                                memcpy(spis_pkg_buff+4, parameters->rx_rsp._payloadPtr+7, 3);
                                SPIS_push(spis_pkg_buff, 7);
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
                                } else if (!parameters->nodeAC[0]) {
                                    parameters->nodeAC[0] = node;
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
