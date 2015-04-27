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

#include "msg.h"
#include "msg_code.h"
#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"
#include "Servo.h"
#include "LedExtBoard.h"
#include "SPIS.h"
#include "clock.h"

#if AC_MODEL || AEROCOMP
#include "servoTask.h"
#endif

#include <string.h>

static uint8_t* EscapeByte(uint8_t* pack, uint8_t b) {
    if (b == MSG_DILIMITER || b == MSG_ESC) {
        *(pack++) = MSG_ESC;
        *(pack++) = b^0x20;
    } else {
        *(pack++) = b;
    }
    return pack;
}

#if AC_MODEL || AEROCOMP
#define BATTPACKLEN (1+BATTCELLADCNUM+3)
size_t updateBattPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_BAT_LEV);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_BAT_LEV);
#endif
    for (i = 0; i < BATTCELLADCNUM; ++i) {
        pack = EscapeByte(pack, BattCell[i]);
    }
    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    return pack - head;
}

#define COMMPACKLEN (1+4*2+3)
size_t updateCommPack(TaskHandle_p sen_Task, TaskHandle_p serov_Task,
        TaskHandle_p task, uint8_t head[]) {
    uint8_t *pack;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_COM_STATS);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_COM_STATS);
#endif
    pack = EscapeByte(pack, sen_Task->load_max >> 8);
    pack = EscapeByte(pack, sen_Task->load_max & 0xFF);
    pack = EscapeByte(pack, serov_Task->load_max >> 8);
    pack = EscapeByte(pack, serov_Task->load_max & 0xFF);
    pack = EscapeByte(pack, task->load_max >> 8);
    pack = EscapeByte(pack, task->load_max & 0xFF);

    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    return pack - head;
}

size_t updateSensorPack(uint8_t head[]) {
    uint8_t *pack;
    int i;
    pack = head;
#if AEROCOMP
    pack = EscapeByte(pack, CODE_AEROCOMP_SERVO_POS);
#else
    pack = EscapeByte(pack, CODE_AC_MODEL_SERVO_POS);
#endif
    for (i = 0; i < SERVOPOSADCNUM; ++i) {
        pack = EscapeByte(pack, ServoPos[i] >> 8);
        pack = EscapeByte(pack, ServoPos[i] & 0xFF);
    }
    for (i = 0; i < ENCNUM; ++i) {
        pack = EscapeByte(pack, EncPos[i] >> 8);
        pack = EscapeByte(pack, EncPos[i] & 0xFF);
    }
#if USE_IMU
    pack = EscapeByte(pack, IMU_XGyro >> 8);
    pack = EscapeByte(pack, IMU_XGyro & 0xFF);
    pack = EscapeByte(pack, IMU_YGyro >> 8);
    pack = EscapeByte(pack, IMU_YGyro & 0xFF);
    pack = EscapeByte(pack, IMU_ZGyro >> 8);
    pack = EscapeByte(pack, IMU_ZGyro & 0xFF);
    pack = EscapeByte(pack, IMU_XAccl >> 8);
    pack = EscapeByte(pack, IMU_XAccl & 0xFF);
    pack = EscapeByte(pack, IMU_YAccl >> 8);
    pack = EscapeByte(pack, IMU_YAccl & 0xFF);
    pack = EscapeByte(pack, IMU_ZAccl >> 8);
    pack = EscapeByte(pack, IMU_ZAccl & 0xFF);
#endif
    pack = EscapeByte(pack, ADC_TimeStamp >>24);
    pack = EscapeByte(pack, ADC_TimeStamp >>16);
    pack = EscapeByte(pack, ADC_TimeStamp >> 8);
    pack = EscapeByte(pack, ADC_TimeStamp & 0xFF);
    for (i = 0; i < SEVERONUM; ++i) {
        pack = EscapeByte(pack, Servos[i].Ctrl >> 8);
        pack = EscapeByte(pack, Servos[i].Ctrl & 0xFF);
    }

    return pack - head;
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
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*8UL / TASK_PERIOD;
            goto xxxx;
        case 8:
        case 9:
            parameters->TimeDelta = ((uint32_t) ((cmd[5] << 8) | cmd[6]))*4UL / TASK_PERIOD;
            goto xxxx;
yyyy:
            parameters->TimeDelta = ((cmd[5] << 8) | cmd[6]) / TASK_PERIOD;
xxxx:
            parameters->InputType = cmd[1] - 1u;
            parameters->Srv2Move = cmd[2];
            parameters->StartTime = ((cmd[3] << 8) | cmd[4]) / TASK_PERIOD;
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

#elif GNDBOARD
#define COMMPACKLEN (1+4*2+3)
size_t updateCommPack(TaskHandle_p task, uint8_t head[]) {
    uint8_t *pack;
    pack = head;

    pack = EscapeByte(pack, CODE_GNDBOARD_COM_STATS);

    pack = EscapeByte(pack, task->load_max >> 8);
    pack = EscapeByte(pack, task->load_max & 0xFF);

    pack = EscapeByte(pack, RTclock.TimeStampMSW & 0xFF);
    pack = EscapeByte(pack, RTclock.TimeStampLSW >> 8);
    pack = EscapeByte(pack, RTclock.TimeStampLSW & 0xFF);
    return pack - head;
}


static bool Equal2DataHeader(uint8_t Byte2Vrfy) {
    if ((Byte2Vrfy == CODE_AC_MODEL_SERVO_POS) || (Byte2Vrfy == CODE_AEROCOMP_SERVO_POS) || (Byte2Vrfy == CODE_AC_MODEL_ENCOD_POS) ||
            (Byte2Vrfy == CODE_AEROCOMP_ENCOD_POS) || (Byte2Vrfy == CODE_AC_MODEL_BAT_LEV) || (Byte2Vrfy == CODE_AEROCOMP_BAT_LEV) ||
            (Byte2Vrfy == CODE_AC_MODEL_LOW_BAT) || (Byte2Vrfy == CODE_AEROCOMP_LOW_BAT) || (Byte2Vrfy == CODE_AC_MODEL_IMU_DATA) ||
            (Byte2Vrfy == CODE_AC_MODEL_COM_STATS) ||
            (Byte2Vrfy == CODE_AEROCOMP_COM_STATS) || (Byte2Vrfy == CODE_AC_MODEL_NEW_SERV_CMD) || (Byte2Vrfy == CODE_AEROCOMP_NEW_SERV_CMD) ||
            (Byte2Vrfy == CODE_GNDBOARD_ADCM_READ) || (Byte2Vrfy == CODE_GNDBOARD_ENCOD_POS) || (Byte2Vrfy == CODE_GNDBOARD_COM_STATS) ||
            (Byte2Vrfy == CODE_AC_MODEL_COMMAND) || (Byte2Vrfy == CODE_AEROCOMP_COMMAND) || (Byte2Vrfy == CODE_GNDBOARD_COMMAND) ||
            (Byte2Vrfy == MASK_BYTE) || (Byte2Vrfy == DUMMY_DATA)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t * push_payload(uint8_t *spis_pkg_buff, const uint8_t *buff, size_t length) {
    size_t i;
    for (i = 0u; i < length; ++i) {
        if (Equal2DataHeader(*buff)) {
            *(spis_pkg_buff++) = MASK_BYTE;
            *(spis_pkg_buff++) = *(buff++) ^ 0x3D;
        } else {
            *(spis_pkg_buff++) = *(buff++);
        }
    }
    return spis_pkg_buff;
}

uint8_t * pull_payload(uint8_t *spis_pkg_buff, const uint8_t *buff, size_t length) {
    //TODO
    size_t i;
    for (i = 0u; i < length; ++i) {
        if (*buff == MASK_BYTE) {
            ++buff;
            ++i;
            *(spis_pkg_buff++) = *(buff++) ^ 0x3D;
        } else {
            *(spis_pkg_buff++) = *(buff++);
        }
    }
    return spis_pkg_buff;
}


#endif

size_t updateNTPPack(NTP_p ntp, uint8_t *head) {
    uint8_t *pack;
    ntp->stage = 2u;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x01');
    pack = EscapeByte(pack, ntp->TimeStampL0 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL0 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL0 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL0 & 0xff);
    ntp->TimeStampL1 = getMicroseconds();
    pack = EscapeByte(pack, ntp->TimeStampL1 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL1 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL1 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL1 & 0xff);
    return pack - head;
}

size_t updateNTPPack3(NTP_p ntp, uint8_t *head) {
    uint8_t *pack;
    ntp->stage = 0u;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x03');
    pack = EscapeByte(pack, ntp->TimeStampL4 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL4 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL4 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL4 & 0xff);
    pack = EscapeByte(pack, ntp->delay >> 8);
    pack = EscapeByte(pack, ntp->delay & 0xff);
    pack = EscapeByte(pack, ntp->offset >> 24);
    pack = EscapeByte(pack, ntp->offset >> 16);
    pack = EscapeByte(pack, ntp->offset >> 8);
    pack = EscapeByte(pack, ntp->offset & 0xff);
    ntp->TimeStampL1 = getMicroseconds();
    pack = EscapeByte(pack, ntp->TimeStampL1 >>24);
    pack = EscapeByte(pack, ntp->TimeStampL1 >>16);
    pack = EscapeByte(pack, ntp->TimeStampL1 >> 8);
    pack = EscapeByte(pack, ntp->TimeStampL1 & 0xff);

    return pack - head;
}

void reset_clock(NTP_p ntp) {
    int32_t T;
    int32_t T1, T2, T3, T4;
    T1 = ntp->TimeStampL1;
    T2 = ntp->TimeStampR2;
    T3 = ntp->TimeStampR3;
    T4 = ntp->TimeStampL4;
    ntp->delay = (T4-T1) - (T3-T2);
    ntp->offset = ((T2-T1) + (T3-T4))/2;
    T = getMicroseconds();
    T += ntp->offset;
    setMicroseconds(T);
}

void process_message(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    NTP_p ntp;
#if GNDBOARD
    uint8_t * spis_pkg_buff;

    spis_pkg_buff = parameters->spis_pkg_buff;
#endif
    switch (msg_ptr[0]) {
        case 'S':
            switch (msg_ptr[1]) {
                case 0u :
                    ntp = &(parameters->ntp);
                    ntp->TimeStampL0 = getMicroseconds();
                    ntp->TimeStampR0 = ((uint32_t)msg_ptr[2]<<24)+((uint32_t)msg_ptr[3]<<16)+((uint32_t)msg_ptr[4]<<8)+(uint32_t)msg_ptr[5];
                    ntp->stage = 1u;
                    break;
                case 2u :
                    ntp = &(parameters->ntp);
                    ntp->TimeStampL4 = getMicroseconds();
                    ntp->TimeStampR2 = ((uint32_t)msg_ptr[2]<<24)+((uint32_t)msg_ptr[3]<<16)+((uint32_t)msg_ptr[4]<<8)+(uint32_t)msg_ptr[5];
                    ntp->TimeStampR3 = ((uint32_t)msg_ptr[6]<<24)+((uint32_t)msg_ptr[7]<<16)+((uint32_t)msg_ptr[8]<<8)+(uint32_t)msg_ptr[9];
                    reset_clock(ntp);
                    ntp->stage = 3u;
                    break;
            }
            break;
        case 'P':
            if (msg_len <= 256) {
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
            SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
            mLED_3_Toggle();
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
            SPIS_push(parameters->spis_pkg_buff, spis_pkg_buff - parameters->spis_pkg_buff);
            mLED_4_Toggle();
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
            *(_payloadPtr++) = MSG_DILIMITER;
            parameters->tx_req._des_port = MSG_DEST_AP_PORT;
            pack_length = updateNTPPack(&parameters->ntp, _payloadPtr);
            _payloadPtr += pack_length;
            *_payloadLength += 1 + pack_length;
            return true;
        case 3u:
            *(_payloadPtr++) = MSG_DILIMITER;
            parameters->tx_req._des_port = MSG_DEST_AP_PORT;
            pack_length = updateNTPPack3(&parameters->ntp, _payloadPtr);
            _payloadPtr += pack_length;
            *_payloadLength += 1 + pack_length;
            return true;
    }

#if AC_MODEL || AEROCOMP
    if ((parameters->cnt & 3) == 1) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateSensorPack(_payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    }
    if ((parameters->cnt & 0x7FF) == 1001 && (*_payloadLength+1u+BATTPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateBattPack(_payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    } else if ((parameters->cnt & 0x7FF) == 2001 && (*_payloadLength+1u+COMMPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateCommPack(parameters->sen_Task,
                    parameters->serov_Task, task, _payloadPtr);
        _payloadPtr += pack_length;
        *_payloadLength += 1+pack_length;
    }
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
        mLED_2_On();
        pack_length = pull_payload(_payloadPtr, SPIRX_RX_PCKT_PTR->RF_DATA,
                (SPIRX_RX_PCKT_PTR->PCKT_LENGTH_MSB << 8) + SPIRX_RX_PCKT_PTR->PCKT_LENGTH_LSB)
                - parameters->tx_req._payloadPtr;
        SPIRX_RX_PCKT_PTR = NULL; //clear for sent
        _payloadPtr += pack_length;
        *_payloadLength += pack_length;
        return true;
    }
    if ((parameters->cnt & 0x7FF) == 2001 && (*_payloadLength + 1u + COMMPACKLEN) < max_payloadLength) {
        *(_payloadPtr++) = MSG_DILIMITER;
        pack_length = updateCommPack(task, _payloadPtr);
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


static void msgInitComm(msgParam_p parameters, XBee_p s6) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->xbee = s6;
    parameters->tx_req._des_addr_msw = MSG_DEST_ADDR_MSW;
    parameters->tx_req._des_addr_lsw = MSG_DEST_ADDR_LSW;
    parameters->tx_req._des_port = MSG_DEST_PORT;
    parameters->tx_req._src_port = MSG_SRC_PORT;
    parameters->tx_req._protocol = 0u;
    parameters->tx_req._option = 0u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';

    parameters->cnt = 0u;
    parameters->rx_cnt = 0u;
    parameters->ntp.stage = 0u;
    parameters->msg_len = 0u;
}

#if AC_MODEL || AEROCOMP
void msgInit(msgParam_p parameters, XBee_p s6, TaskHandle_p senTask, TaskHandle_p servoTask) {
    msgInitComm(parameters, s6);

    parameters->sen_Task = senTask;
    parameters->serov_Task = servoTask;
}
#elif GNDBOARD
void msgInit(msgParam_p parameters, XBee_p s6) {
    msgInitComm(parameters, s6);
}
#endif

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
                    }
                    break;
            }
            packin = XBeeReadPacket(parameters->xbee);
        }

        if (++parameters->cnt % 10 == 0) {
                offset_us += 1u;
        }
        if (prepare_tx_data(task, parameters, &parameters->tx_req._payloadLength, parameters->tx_req._payloadPtr, MAX_S6_PAYLOAD_DATA_SIZE)) {
            XBeeTxIPv4Request(parameters->xbee, &parameters->tx_req, 0u);
            // Reset to send to MSG_DEST_PORT
            if (parameters->tx_req._des_port != MSG_DEST_PORT) {
                parameters->tx_req._des_port = MSG_DEST_PORT;
            }
        }

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}
