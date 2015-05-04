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

static uint8_t* EscapeByte(uint8_t* pack, uint8_t b) {
    if (b == MSG_DILIMITER || b == MSG_ESC) {
        *(pack++) = MSG_ESC;
        *(pack++) = b^0x20;
    } else {
        *(pack++) = b;
    }
    return pack;
}

/*
 * msg_ptr :
 * 0 - TS MSB remote gen-timestamp
 * 1 - TS
 * 2 - TS
 * 3 - TS LSB
 * 4 - PKG ID
 * 5 - Ctx first byte
 * ...
 * n - Ctx last byte
 */
static bool process_message(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    uint8_t id;
    ProcessMessageHandle_p h;
    uint32_t ts;

    id = msg_ptr[MSG_ID_POS];
    id = parameters->process_map[id];
    if (id < parameters->process_handle_list_num) {
        h = parameters->process_handle_list + id;
        h->rx_timestamp = parameters->rx_timestamp;
        ++(h->rx_cnt);
        h->remote_tx_timestamp = parameters->remote_tx_timestamp;
        ts = ((uint32_t)*(msg_ptr++) << 24);
        ts += ((uint32_t)*(msg_ptr++) << 16);
        ts += ((uint32_t)*(msg_ptr++) << 8);
        ts += (uint32_t)*(msg_ptr++);
        h->remote_gen_timestamp = ts;
        h->rx_port = parameters->rx_rsp._port;
        h->remote_tx_port = parameters->rx_rsp._src_port;
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
 * 4 - TS LSB
 * 5 - PKG ID
 * 6 - Ctx first byte
 * ...
 * n - Ctx last byte
 * n+1 - TS MSB remote send-timestamp
 * n+2 - TS
 * n+3 - TS
 * n+4 - TS LSB
 */
static bool process_packages(msgParam_p parameters, uint8_t *msg_ptr, size_t msg_len) {
    uint8_t *head;
    uint8_t *end;
    uint8_t *msg_tail;
    uint32_t ts;
    bool rslt;

    msg_tail = msg_ptr + msg_len;
    ts = (uint32_t) *(--msg_tail);
    ts += ((uint32_t) *(--msg_tail) << 8);
    ts += ((uint32_t) *(--msg_tail) << 16);
    ts += ((uint32_t)*(--msg_tail) << 24);
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

bool pushMessage(msgParam_p parameters, uint16_t des_port, uint8_t *msg, size_t msg_len) {
    uint8_t * pack;
    uint8_t * pack_tail;
    size_t i;
    uint32_t ts;
    int target;

    ts = getMicroseconds();

    if (des_port < DEST_MAX_NUM) {
        target = des_port;
    } else {
        i = 0;
        for (target=0; target < DEST_MAX_NUM; ++target) {
            if (parameters->msg_port[target] == des_port) {
                i = 1;
                break;
            }
        }
        if (!i) {
            ++parameters->bad_tx_pkg_cnt;
            return false;
        }
    }

    pack = parameters->msg_tail[target];
    pack_tail = parameters->msg_head[target + 1] - 1u;
    if (pack_tail - pack < 10 + msg_len) {
        ++parameters->bad_tx_pkg_cnt;
        return false;
    }
    //HEAD
    *(pack++) = MSG_DILIMITER;
    //GEN_TIME_STAMP
    pack = EscapeByte(pack, ts >> 24);
    pack = EscapeByte(pack, ts >> 16);
    pack = EscapeByte(pack, ts >> 8);
    pack = EscapeByte(pack, ts & 0xff);
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

bool registerPushMessageHandle(msgParam_p msg, char name[MSG_NAME_MAX_LEN],
        PushMessageHandleFunc_p func, void * parameters,
        uint16_t des_port, unsigned int circle, unsigned int offset) {
    PushMessageHandle_p h;
    uint16_t target;
    int flag;
    if (des_port < DEST_MAX_NUM) {
        target = des_port;
    } else {
        flag = 0;
        for (target=0; target < DEST_MAX_NUM; ++target) {
            if (msg->msg_port[target] == des_port) {
                flag = 1;
                break;
            }
        }
        if (!flag) {
            return false;
        }
    }

    if (msg->push_handle_list_num < MSG_MAX_PUSH_FUNCS) {
        h = msg->push_handle_list + msg->push_handle_list_num;
        strncpy(h->name, name, MSG_NAME_MAX_LEN);
        h->func = func;
        h->parameters = parameters;
        h->peroid = circle;
        h->offset = offset + 1;
        h->target = target;
        h->tx_cnt = 0;

        ++msg->push_handle_list_num;
        return true;
    }
    return false;
}

void msgInit(msgParam_p parameters, XBee_p s6) {
    struct pt * pt;
    int i;

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
    parameters->bad_rx_pkg_cnt = 0u;
    parameters->bad_tx_pkg_cnt = 0u;

    parameters->msg_tail[0] = parameters->msg_head[0] = parameters->msg_buff + 0;
    parameters->msg_port[0] = MSG_DEST_PORT;

    parameters->msg_tail[1] = parameters->msg_head[1] = parameters->msg_buff + (MSG_TX_BUFF_LEN - 200);
    parameters->msg_port[1] = MSG_DEST2_PORT;

    parameters->msg_tail[2] = parameters->msg_head[1] = parameters->msg_buff + (MSG_TX_BUFF_LEN - 100);
    parameters->msg_port[2] = MSG_DEST3_PORT;

    /*Last Empty Item used as stop-bit */
    parameters->msg_tail[3] = parameters->msg_head[3] = parameters->msg_buff + MSG_TX_BUFF_LEN;
    parameters->msg_port[3] = 0;

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

    for (i = 0; i < MSG_MAX_PUSH_FUNCS; ++i) {
        parameters->push_handle_list[i].parameters = NULL;
        parameters->push_handle_list[i].tx_cnt = 0;
        parameters->push_handle_list[i].target = 0;
        parameters->push_handle_list[i].peroid = 0;
        parameters->push_handle_list[i].func = NULL;
    }
    parameters->push_handle_list_num = 0;

}

PT_THREAD(msgLoop)(TaskHandle_p task) {
    int packin;
    msgParam_p parameters;
    struct pt *pt;
    uint32_t ts;
    uint8_t *pack;
    int i;
    PushMessageHandle_p pmh;
    size_t s;

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

        for (i = 0; i < parameters->push_handle_list_num; ++i) {
            pmh = parameters->push_handle_list + i;
            if (--pmh->offset == 0u) {
                pmh->offset = pmh->peroid;
                s = pmh->func(pmh, parameters->push_buff, parameters->msg_len[pmh->target]);
                if (s > 0) {
                    pushMessage(parameters, pmh->target, parameters->push_buff, s);
                    ++pmh->tx_cnt;
                }
            }
        }
        ++parameters->cnt;
        for (i = 0; i < DEST_MAX_NUM; ++i) {
            if (parameters->msg_tail[i] != parameters->msg_head[i]) {
                parameters->tx_req._des_port = parameters->msg_port[i];
                packin = parameters->msg_tail[i] - parameters->msg_head[i];
                memcpy(parameters->tx_req._payloadPtr, parameters->msg_head[i], packin);
                parameters->msg_tail[i] = parameters->msg_head[i];
                pack = parameters->tx_req._payloadPtr + packin;
                ts = getMicroseconds();
                pack = EscapeByte(pack, ts >> 24);
                pack = EscapeByte(pack, ts >> 16);
                pack = EscapeByte(pack, ts >> 8);
                pack = EscapeByte(pack, ts & 0xff);
                parameters->tx_req._payloadLength = pack - parameters->tx_req._payloadPtr;
                XBeeTxIPv4Request(parameters->xbee, &parameters->tx_req, 0u);
                // Reset to send to MSG_DEST_PORT
#if USE_LEDEXTBOARD
                mLED_3_Toggle();
#endif
                break;
            }
        }

        // clock adjust
        if (parameters->cnt % 10 == 0) {
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
