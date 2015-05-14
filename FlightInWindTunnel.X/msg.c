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

#include "msg.h"
#include "LedExtBoard.h"

#include "clock.h"
#include <string.h>

uint8_t * packInt(uint8_t *pack, const void *data, size_t len) {
    size_t i;
    for (i=0;i<len;++i) {
        pack[i] = *((uint8_t *)data+(len-1-i));
    }
    return pack+len;
}

const uint8_t * unpackInt(const uint8_t *pack, void *data, size_t len) {
    size_t i;
    for (i=0;i<len;++i) {
        *((uint8_t *)data+(len-1-i)) = pack[i];
    }
    return pack+len;
}

static uint8_t* EscapeByte(uint8_t* pack, uint8_t b) {
    if (b == MSG_DILIMITER || b == MSG_ESC) {
        *(pack++) = MSG_ESC;
        *(pack++) = b^0x20;
    } else {
        *(pack++) = b;
    }
    return pack;
}

uint8_t * packIntEsc(uint8_t *head, const void *data, size_t len) {
    size_t i;
    uint8_t *pack;
    pack = head;
    for (i=0;i<len;++i) {
        pack = EscapeByte(pack, *((uint8_t *)data+(len-1-i)));
    }
    return pack;
}

/*
 * msg_ptr :
 * 0 - TS MSB remote gen-timestamp
 * 1 - TS
 * 2 - TS
 * 3 - TS
 * 4 - TS
 * 5 - TS
 * 6 - TS
 * 7 - TS LSB
 * 8 - PKG ID
 * 9 - Ctx first byte
 * ...
 * n - Ctx last byte
 */
static bool process_message(msgParam_p parameters, const uint8_t *msg_ptr, size_t msg_len) {
    uint8_t id;
    ProcessMessageHandle_p h;
    timestamp_t ts;

    id = msg_ptr[MSG_ID_POS];
    id = parameters->process_map[id];
    if (id < parameters->process_handle_list_num) {
        h = parameters->process_handle_list + id;
        h->rx_timestamp = parameters->rx_timestamp;
        ++(h->rx_cnt);
        h->remote_tx_timestamp = parameters->remote_tx_timestamp;
        msg_ptr = unpackInt(msg_ptr, &ts, timestamp_size);
        h->remote_gen_timestamp = ts;
        h->remote_tx_addr = parameters->rx_rsp._src_addr_lsw;
        return h->func(h, msg_ptr, msg_len - MSG_ID_POS);
    }
    return false;
}

/*
 * msg_ptr :
 * 0 - MSG_DILIMITER
 * 1 - TS MSB remote gen-timestamp
 * 2 - TS
 * 3 - TS
 * 4 - TS
 * 5 - TS
 * 6 - TS
 * 7 - TS
 * 8 - TS LSB
 * 9 - PKG ID
 * 10 - Ctx first byte
 * ...
 * n - Ctx last byte
 * n+1 - TS MSB remote send-timestamp
 * n+2 - TS
 * n+3 - TS
 * n+4 - TS
 * n+5 - TS
 * n+6 - TS
 * n+7 - TS
 * n+8 - TS LSB
 */
static bool process_packages(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    uint8_t *head;
    uint8_t *end;
    uint8_t *msg_tail;
    timestamp_t ts;
    bool rslt;

    msg_tail = msg_ptr + msg_len;
    unpackInt(msg_tail-8u, &ts, timestamp_size);
    parameters->remote_tx_timestamp = ts;
    rslt = true;
    head = end = NULL;
    while (msg_ptr <= msg_tail) {
        if (*msg_ptr == MSG_DILIMITER) {
            if (head) {
                rslt = rslt && process_message(parameters, head + 1u, end - head);
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
    if (msg_len > MSG_ID_POS + 1) {
        rslt = rslt && process_message(parameters, head + 1u, msg_len);
    }
    return rslt;
}

bool registerProcessMessageHandle(msgParam_p msg, char name[MSG_NAME_MAX_LEN], uint8_t id, ProcessMessageHandleFunc_p func, void * parameters) {
    size_t s;
    ProcessMessageHandle_p h;

    if (msg->process_handle_list_num < MSG_MAX_PROCESS_FUNCS) {
        s = msg->process_handle_list_num;
        h = msg->process_handle_list + s;
        strncpy(h->name, name, MSG_NAME_MAX_LEN);
        h->msg_id = id;
        h->func = func;
        h->parameters = parameters;
        h->rx_cnt = 0;

        msg->process_map[id] = s;
        ++msg->process_handle_list_num;
        return true;
    }
    return false;
}

TargetIdx_t findTarget(msgParam_p parameters, uint8_t addr) {
    size_t i;
    for (i=0u;i<DEST_MAX_NUM;++i) {
        if (parameters->msg_des_addr[i] == addr) {
            return i;
        }
    }
    return 0;
}

bool pushMessage(msgParam_p parameters, TargetIdx_t target, uint8_t *msg, size_t msg_len) {
    uint8_t * pack;
    uint8_t * pack_tail;
    size_t i;
    timestamp_t ts;

    ts = getMicroseconds();

    pack = parameters->msg_tail[target];
    pack_tail = parameters->msg_head[target + 1] - 1u;
    if (pack_tail - pack < 10 + msg_len) {
        ++parameters->bad_tx_pkg_cnt;
        return false;
    }
    //HEAD
    *(pack++) = MSG_DILIMITER;
    //GEN_TIME_STAMP
    pack = packIntEsc(pack, &ts, timestamp_size);
    //BODY
    for (i = 0; i < msg_len && pack < pack_tail; ++i) {
        pack = EscapeByte(pack, *(msg++));
    }
    if (i < msg_len) {
        ++parameters->bad_tx_pkg_cnt;
        return false;
    }

    parameters->msg_tail[target] = pack;
    return true;
}


void msgInit(msgParam_p parameters, XBee_p s6) {
    struct pt * pt;
    int i;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->xbee = s6;
    parameters->tx_req._des_addr_msw = MSG_NETWORK_MSW;
    parameters->tx_req._des_addr_lsw = MSG_NETWORK_LSW;
    parameters->tx_req._des_port = MSG_DES_PORT;
    parameters->tx_req._src_port = MSG_SRC_PORT;
    parameters->tx_req._protocol = 0u;
    parameters->tx_req._option = 0u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';

    parameters->cnt = 0u;
    parameters->bad_rx_pkg_cnt = 0u;
    parameters->bad_tx_pkg_cnt = 0u;

    parameters->msg_tail[0] = parameters->msg_head[0] = parameters->msg_buff + 0;
    parameters->msg_des_addr[0] = MSG_DES_ADDR;

    parameters->msg_tail[1] = parameters->msg_head[1] = parameters->msg_buff + (MSG_TX_BUFF_LEN - 200);
    parameters->msg_des_addr[1] = MSG_DES2_ADDR;

    parameters->msg_tail[2] = parameters->msg_head[2] = parameters->msg_buff + (MSG_TX_BUFF_LEN - 100);
    parameters->msg_des_addr[2] = MSG_DES3_ADDR;

    /*Last Empty Item used as stop-bit */
    parameters->msg_tail[3] = parameters->msg_head[3] = parameters->msg_buff + MSG_TX_BUFF_LEN;
    parameters->msg_des_addr[3] = 0;

    parameters->msg_len[0] = parameters->msg_head[1] - parameters->msg_head[0];
    parameters->msg_len[1] = parameters->msg_head[2] - parameters->msg_head[1];
    parameters->msg_len[2] = parameters->msg_head[3] - parameters->msg_head[2];

    for (i = 0; i < MSG_MAX_PROCESS_FUNCS; ++i) {
        parameters->process_handle_list[i].msg_id = 0xFF;
        parameters->process_handle_list[i].rx_cnt = 0;
        parameters->process_handle_list[i].parameters = NULL;
        parameters->process_handle_list[i].func = NULL;
    }
    parameters->process_handle_list_num = 0;
    for (i = 0; i < 256; ++i) {
        parameters->process_map[i] = 0xFF;
    }
}

void sendmsgInit(sendmsgParam_p parameters, msgParam_p msg, SendMessageHandleFunc_p func, void *param) {
    struct pt * pt;

    pt = &(parameters->PT);
    PT_INIT(pt);

    parameters->_msg = msg;
    parameters->_func = func;
    parameters->_param = param;
}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    timestamp_t ts;
    uint8_t *pack;
    int i;

    parameters = (msgParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        parameters->rx_timestamp = getMicroseconds();
        packin = XBeeReadPacket(parameters->xbee);
        while (packin > 0) {
            switch (packin) {
                case RX_IPV4_RESPONSE:
                    if (XBeeRxIPv4Response(parameters->xbee, &parameters->rx_rsp)) {
#if USE_LEDEXTBOARD
                        mLED_2_Toggle();
#endif
                        if (!process_packages(parameters, parameters->rx_rsp._payloadPtr, parameters->rx_rsp._payloadLength)) {
                            ++parameters->bad_rx_pkg_cnt;
                        }
                    }
                    break;
            }
            packin = XBeeReadPacket(parameters->xbee);
        }

        ++parameters->cnt;
        for (i = 0; i < DEST_MAX_NUM; ++i) {
            if (parameters->msg_tail[i] != parameters->msg_head[i]) {
                (*(uint8_t*)&(parameters->tx_req._des_addr_lsw)) = parameters->msg_des_addr[i];
                packin = parameters->msg_tail[i] - parameters->msg_head[i];
                memcpy(parameters->tx_req._payloadPtr, parameters->msg_head[i], packin);
                parameters->msg_tail[i] = parameters->msg_head[i];
                pack = parameters->tx_req._payloadPtr + packin;
                ts = getMicroseconds();
                pack = packIntEsc(pack, &ts, timestamp_size);
                parameters->tx_req._payloadLength = pack - parameters->tx_req._payloadPtr;
                XBeeTxIPv4Request(parameters->xbee, &parameters->tx_req, 0u);
                // Reset to send to MSG_DEST_PORT
#if USE_LEDEXTBOARD
                mLED_3_Toggle();
#endif
                break;
            }
        }

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

PT_THREAD(sendmsgLoop)(TaskHandle_p task) {
    sendmsgParam_p parameters;
    struct pt *pt;

    parameters = (sendmsgParam_p) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        parameters->_func(parameters);
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}
