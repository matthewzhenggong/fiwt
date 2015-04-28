/*
 * File:   msg.c
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
#include "msg_acm.h"
#elif GNDBOARD
#include "msg_gnd.h"
#include "SPIS.h"
#endif

#include "msg_code.h"
#include "LedExtBoard.h"

#include "clock.h"
#include <string.h>

void process_message(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    NTP_p ntp;
#if GNDBOARD
    PM_p pm;
    uint8_t * spis_pkg_buff;

    spis_pkg_buff = parameters->spis_pkg_buff;
#endif
    switch (msg_ptr[0]) {
        case 'S':
            switch (msg_ptr[1]) {
                case 0u :
                case 5u :
                    ntp = &(parameters->ntp);
                    ntp->TimeStampL0 = getMicroseconds();
                    ntp->TimeStampR0 = ((uint32_t)msg_ptr[2]<<24)+((uint32_t)msg_ptr[3]<<16)+((uint32_t)msg_ptr[4]<<8)+(uint32_t)msg_ptr[5];
                    ntp->stage = msg_ptr[1]+1u;
                    break;
                case 2u :
                    ntp = &(parameters->ntp);
                    ntp->TimeStampL4 = getMicroseconds();
                    ntp->TimeStampR2 = ((uint32_t)msg_ptr[2]<<24)+((uint32_t)msg_ptr[3]<<16)+((uint32_t)msg_ptr[4]<<8)+(uint32_t)msg_ptr[5];
                    ntp->TimeStampR3 = ((uint32_t)msg_ptr[6]<<24)+((uint32_t)msg_ptr[7]<<16)+((uint32_t)msg_ptr[8]<<8)+(uint32_t)msg_ptr[9];
                    if (ntp->stage == 2u) {
                        reset_clock(ntp, true);
                        parameters->cnt = 0;
                    } else {
                        reset_clock(ntp, false);
                    }
                    ntp->stage++;
                    break;
                case 0x11 :
                    ntp = &(parameters->ntp);
                    ntp->TimeStampLP = getMicroseconds();
                    ntp->stage = 0x11u;
                    break;
#if GNDBOARD
                case 0x12 :
                    pm  = &parameters->pm;
                    pm->TimeStampL4 = getMicroseconds();
                    pm->TimeStampR2 = ((uint32_t)msg_ptr[2]<<24)+((uint32_t)msg_ptr[3]<<16)+((uint32_t)msg_ptr[4]<<8)+(uint32_t)msg_ptr[5];
                    pm->TimeStampR3 = ((uint32_t)msg_ptr[6]<<24)+((uint32_t)msg_ptr[7]<<16)+((uint32_t)msg_ptr[8]<<8)+(uint32_t)msg_ptr[9];
                    pm->stage = 2u;
                    break;
#endif
            }
            break;
        case 'P':
            if (msg_ptr[1] == 0 && msg_len <= 256) {
#if AC_MODEL
                msg_ptr[1] = 2;
#elif AEROCOMP
                msg_ptr[1] = 3;
#elif GNDBOARD
                msg_ptr[1] = 1;
#endif
                memcpy(parameters->msg_buff+parameters->msg_len, msg_ptr, msg_len);
                parameters->msg_len += msg_len;
            }
            break;
#if AC_MODEL || AEROCOMP
        case CODE_AC_MODEL_NEW_SERV_CMD:
        case CODE_AEROCOMP_NEW_SERV_CMD:
            servoProcA5Cmd((servoParam_p) (parameters->serov_Task->parameters), msg_ptr);
            break;
#elif GNDBOARD
        case CODE_AC_MODEL_SERVO_POS:
            /* self.pack22 = struct.Struct(">B6H3H6HBH6h") */
            /* 1+12+6+12+3+12 = 46 */
            // DATA_ID == CODE_AC_MODEL_SERVO_POS
            *(spis_pkg_buff++) = CODE_AC_MODEL_SERVO_POS;
            //2-13	Data		SERVOx_H, SERVOx_L
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 1, 12);
            //14-16	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 31, 3);
            // DATA_ID ==  CODE_AC_MODEL_ENCOD_POS
            *(spis_pkg_buff++) = CODE_AC_MODEL_ENCOD_POS;
            //2- 7	Data		DIGENCx_H, DIGENCx_L
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 13, 6);
            //8-10	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 31, 3);
            //DATA_ID ==	CODE_AC_MODEL_IMU_DATA
            *(spis_pkg_buff++) = CODE_AC_MODEL_IMU_DATA;
            //2-21		Data    GxH,GxL,GyH,GyL...
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 19, 12);
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            *(spis_pkg_buff++) = 0;
            //22-24	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 31, 3);
            if (parameters->cnt % 5 == 1) {//TODO
                SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
                mLED_4_Toggle();
            }
            break;
        case CODE_AEROCOMP_SERVO_POS:
            /* self.pack33 = struct.Struct(">B4H4HBH4h") */
            /* 1+8+8+3+8 = 28 */
            //DATA_ID == 	CODE_AC_MODEL_SERVO_POS
            *(spis_pkg_buff++) = CODE_AEROCOMP_SERVO_POS;
            //2- 9	Data		SERVOx_H, SERVOx_L
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 1, 8);
            //10-12	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 17, 3);
            // DATA_ID ==  CODE_AEROCOMP_ENCOD_POS
            *(spis_pkg_buff++) = CODE_AEROCOMP_ENCOD_POS;
            //2-9	Data		DIGENCx_H, DIGENCx_L
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 9, 8);
            //10-12	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 17, 3);
            if (parameters->cnt % 5 == 3) {//TODO
                SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
                mLED_5_Toggle();
            }
            break;
        case CODE_AC_MODEL_BAT_LEV:
            /*self.pack88 = struct.Struct(">B3BBH")*/
            /* 1+3+3 */
            //DATA_ID == 	CODE_AC_MODEL_BAT_LEV
            *(spis_pkg_buff++) = CODE_AC_MODEL_BAT_LEV;
            //2- 4	Data		BATT_C1, BATT_C2, BATT_C3
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 1, 3);
            //5- 7	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 4, 3);
            SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
            break;
        case CODE_AEROCOMP_BAT_LEV:
            /*self.pack88 = struct.Struct(">B3HBH")*/
            /* 1+3+3 */
            //DATA_ID == 	CODE_AEROCOMP_BAT_LEV
            *(spis_pkg_buff++) = CODE_AEROCOMP_BAT_LEV;
            //2- 4	Data		BATT_C1, BATT_C2, BATT_C3
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 1, 3);
            //5- 7	Time Stamp	TimeStampH, TimeStampM, TimeStampL
            spis_pkg_buff = push_payload(spis_pkg_buff, msg_ptr + 4, 3);
            SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
            break;
        case CODE_AC_MODEL_COM_STATS:
            break;
        case CODE_AEROCOMP_COM_STATS:
            break;
#endif
        default:
            break;
    }
}

void process_packages(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    uint8_t *head;
    uint8_t *end;
    uint8_t *msg_tail;

    head = end = NULL;
    msg_tail = msg_ptr + msg_len;
    while (msg_ptr < msg_tail) {
        if (*msg_ptr == MSG_DILIMITER) {
            if (head) {
                process_message(parameters, head + 1u, end - head);
            }
            head = end = msg_ptr;
        } else if (*msg_ptr == MSG_ESC && head) {
            *(++end) = *(++msg_ptr) ^ 0x20;
        } else if (head && (++end) != msg_ptr) {
            *end = *msg_ptr;
        }
        ++msg_ptr;
    }
    msg_len = end - head;
    if (msg_len > 1u) {
        process_message(parameters, head + 1u, msg_len);
    }
}

static bool prepare_tx_data(TaskHandle_p task, msgParam_p parameters, size_t *_payloadLength, uint8_t *_payloadPtr, size_t max_payloadLength) {
    size_t pack_length;

    *_payloadLength = 0;

    switch (parameters->ntp.stage) {
        case 1u:
        case 6u:
            *(_payloadPtr++) = MSG_DILIMITER;
            parameters->tx_req._des_port = MSG_DEST_AP_PORT;
            pack_length = updateNTPPack(&parameters->ntp, _payloadPtr);
            _payloadPtr += pack_length;
            *_payloadLength += 1 + pack_length;
            return true;
        case 3u:
        case 8u:
            *(_payloadPtr++) = MSG_DILIMITER;
            parameters->tx_req._des_port = MSG_DEST_AP_PORT;
            pack_length = updateNTPPack3(&parameters->ntp, _payloadPtr);
            _payloadPtr += pack_length;
            *_payloadLength += 1 + pack_length;
            return true;
        case 0x11:
            *(_payloadPtr++) = MSG_DILIMITER;
            pack_length = updateNTPPack11(&parameters->ntp, _payloadPtr);
            _payloadPtr += pack_length;
            *_payloadLength += 1 + pack_length;
    }

#if AC_MODEL || AEROCOMP
    if ((parameters->cnt & 3) == 1) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateSensorPack(_payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    }
#if AC_MODEL
    if ((parameters->cnt & 0xFFF) == 0x3E9 && (*_payloadLength+1u+BATTPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateBattPack(_payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    } else if ((parameters->cnt & 0xFFF) == 0x7E9 && (*_payloadLength+1u+COMMPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateCommPack(parameters->sen_Task,
                    parameters->serov_Task, task, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    }
#else
    if ((parameters->cnt & 0xFFF) == 0xBE9 && (*_payloadLength+1u+BATTPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateBattPack(_payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    } else if ((parameters->cnt & 0xFFF) == 0xFE9 && (*_payloadLength+1u+COMMPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateCommPack(parameters->sen_Task,
                    parameters->serov_Task, task, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    }
#endif

#elif GNDBOARD
    if (SPIRX_RX_PCKT_PTR) {
        switch (SPIRX_RX_PCKT_PTR->RF_DATA[0]) {
            case CODE_AC_MODEL_NEW_SERV_CMD:
                parameters->tx_req._des_port = MSG_DEST_ACM_PORT;
                break;
            case CODE_AEROCOMP_NEW_SERV_CMD:
                parameters->tx_req._des_port = MSG_DEST_CMP_PORT;
                break;
        }
        pack_length = pull_payload(_payloadPtr, SPIRX_RX_PCKT_PTR->RF_DATA,
                (SPIRX_RX_PCKT_PTR->PCKT_LENGTH_MSB << 8) + SPIRX_RX_PCKT_PTR->PCKT_LENGTH_LSB)
                - parameters->tx_req._payloadPtr;
        SPIRX_RX_PCKT_PTR = NULL; //clear for sent
        _payloadPtr += pack_length;
        *_payloadLength += pack_length;
        return true;
    }
    if ((parameters->cnt & 0x1FFF) == 0x5DF) {
        parameters->tx_req._des_port = MSG_DEST_ACM_PORT;
        *(_payloadPtr++) = MSG_DILIMITER;
        parameters->pm.target = 'A';
        pack_length = updatePingPack(&parameters->pm, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1 + pack_length;
        return true;
    } else if ((parameters->cnt & 0x1FFF) == 0x9C7) {
        parameters->tx_req._des_port = MSG_DEST_CMP_PORT;
        *(_payloadPtr++) = MSG_DILIMITER;
        parameters->pm.target = 'C';
        pack_length = updatePingPack(&parameters->pm, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1 + pack_length;
        return true;
    }
    if ((parameters->cnt & 0xFFF) == 0x1F3 && (*_payloadLength + 1u + COMMPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateCommPack(task, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1 + pack_length;
    }
    if (parameters->pm.stage == 2u && (*_payloadLength + 1u + PINPACK2LEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updatePingPack2(&parameters->pm, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1 + pack_length;
    }
#endif

    if (parameters->msg_len > 0u && (*_payloadLength + 1u + parameters->msg_len) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = parameters->msg_len;
        parameters->msg_len = 0;
        memcpy(_payloadPtr, parameters->msg_buff, pack_length);
        _payloadPtr += pack_length;
        *_payloadLength += 1 + pack_length;
    }
    

    return (*_payloadLength > 2u);
}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int packin;
    msgParam_p parameters;
    struct pt *pt;

    parameters = (msgParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        packin = XBeeReadPacket(parameters->xbee);
        while (packin > 0) {
            switch (packin) {
                case RX_IPV4_RESPONSE:
                    if (XBeeRxIPv4Response(parameters->xbee, &parameters->rx_rsp)) {
                        ++parameters->rx_cnt;
                        process_packages(parameters, parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength);
#if USE_LEDEXTBOARD
                        mLED_2_Toggle();
#endif
                    }
                    break;
            }
            packin = XBeeReadPacket(parameters->xbee);
        }

        ++parameters->cnt;
        if (prepare_tx_data(task, parameters, &parameters->tx_req._payloadLength, parameters->tx_req._payloadPtr, MAX_S6_PAYLOAD_DATA_SIZE)) {
            XBeeTxIPv4Request(parameters->xbee, &parameters->tx_req, 0u);
            // Reset to send to MSG_DEST_PORT
#if USE_LEDEXTBOARD
            mLED_3_Toggle();
#endif
            if (parameters->tx_req._des_port != MSG_DEST_PORT) {
                parameters->tx_req._des_port = MSG_DEST_PORT;
            }
        }

        // clock adjust
        if (parameters->cnt %10 == 0) {
                offset_us += 1u;
        } else if (parameters->cnt % 289 == 0) {
                offset_us -= 1u;
        }

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}
