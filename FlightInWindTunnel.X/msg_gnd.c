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

#include "msg_gnd.h"

#if GNDBOARD

#include "SPIS.h"
#include "msg_code.h"
#include "clock.h"
#include <stdbool.h>

void msgInit(msgParam_p parameters, XBee_p s6) {
    msgInitComm(parameters, s6);
    parameters->pm.stage = 0;
}

size_t updateCommPack(TaskHandle_p task, uint8_t head[]) {
    uint8_t *pack;
    uint32_t t;
    pack = head;

    pack = EscapeByte(pack, CODE_GNDBOARD_COM_STATS);

    pack = EscapeByte(pack, task->load_max >> 8);
    pack = EscapeByte(pack, task->load_max & 0xFF);

    t = getMicroseconds();
    pack = EscapeByte(pack, t >> 24);
    pack = EscapeByte(pack, t >> 16);
    pack = EscapeByte(pack, t >> 8);
    pack = EscapeByte(pack, t & 0xFF);
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

static bool Equal2CmdDataHeader(uint8_t Byte2Vrfy) {
    if ((Byte2Vrfy == CODE_AC_MODEL_NEW_SERV_CMD) || (Byte2Vrfy == CODE_AEROCOMP_NEW_SERV_CMD)) {
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

uint8_t * push_timestamp(uint8_t *spis_pkg_buff, const uint8_t *timestamp, size_t length) {
    size_t i;
    uint32_t t;
    uint8_t buff[3];
    t = ((uint32_t)timestamp[0] << 24) + ((uint32_t)timestamp[1] << 16)
             + ((uint32_t)timestamp[3] << 8)  + ((uint32_t)timestamp[4] & 0xff);
    t /= 1000;
    buff[0] = t >> 16;
    buff[1] = t >> 8;
    buff[2] = t && 0xff;
    for (i = 0u; i < length; ++i) {
        if (Equal2DataHeader(buff[i])) {
            *(spis_pkg_buff++) = MASK_BYTE;
            *(spis_pkg_buff++) = buff[i] ^ 0x3D;
        } else {
            *(spis_pkg_buff++) = buff[i];
        }
    }
    return spis_pkg_buff;
}

uint8_t * pull_payload(uint8_t *spis_pkg_buff, const uint8_t *buff, size_t length) {
    //TODO
    size_t i;
    bool escape;
    escape = false;
    for (i = 0u; i < length; ++i) {
        if (escape) {
            spis_pkg_buff = EscapeByte(spis_pkg_buff, *(buff++) ^ 0x3D);
            escape = false;
        }
        else if (Equal2CmdDataHeader(*buff)) {
            *(spis_pkg_buff++) = MSG_DILIMITER;
            spis_pkg_buff = EscapeByte(spis_pkg_buff, *(buff++));
        }
        else if (*buff == MASK_BYTE) {
            ++buff;
            escape = true;
        } else {
            spis_pkg_buff = EscapeByte(spis_pkg_buff, *(buff++));
        }
    }
    return spis_pkg_buff;
}

size_t updatePingPack(PM_p pm, uint8_t *head) {
    uint8_t *pack;
    uint32_t ts;
    pm->stage = 1u;
    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x11');
    ts = getMicroseconds();
    pm->TimeStampL1 = ts;
    pack = EscapeByte(pack, ts >> 24);
    pack = EscapeByte(pack, ts >> 16);
    pack = EscapeByte(pack, ts >> 8);
    pack = EscapeByte(pack, ts & 0xff);
    return pack - head;
}

size_t updatePingPack2(PM_p pm, uint8_t *head) {
    int32_t T1, T2, T3, T4;
    uint8_t *pack;
    pm->stage = 0u;
    T1 = pm->TimeStampL1;
    T2 = pm->TimeStampR2;
    T3 = pm->TimeStampR3;
    T4 = pm->TimeStampL4;
    pm->delay = (T4-T1) - (T3-T2);
    pm->offset = ((T2-T1) + (T3-T4))/2;

    pack = head;
    pack = EscapeByte(pack, 'S');
    pack = EscapeByte(pack, '\x13');
    pack = EscapeByte(pack, pm->target);
    pack = EscapeByte(pack, pm->delay >> 8);
    pack = EscapeByte(pack, pm->delay & 0xff);
    pack = EscapeByte(pack, pm->offset >> 24);
    pack = EscapeByte(pack, pm->offset >> 16);
    pack = EscapeByte(pack, pm->offset >> 8);
    pack = EscapeByte(pack, pm->offset & 0xff);

    return pack - head;
}

#endif /*GNDBOARD*/
