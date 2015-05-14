/*
 * File:   msg.h
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

#ifndef MSG_H
#define	MSG_H

#include "config.h"

#include "XBee.h"
#include "task.h"
#include "clock.h"
#include <pt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//#if GNDBOARD
//#define SPI_PKG_MAXLEN 86
//#endif

#define MSG_MAX_PROCESS_FUNCS (8)
#define MSG_MAX_PUSH_FUNCS (8)
#define MSG_NAME_MAX_LEN (8)
#define MSG_ID_POS (timestamp_size)

#ifdef	__cplusplus
extern "C" {
#endif

    struct ProcessMessageHandle;
    typedef struct ProcessMessageHandle* ProcessMessageHandle_p;
    typedef bool (*ProcessMessageHandleFunc_p)(ProcessMessageHandle_p, const uint8_t *msg, size_t msg_len);
    typedef struct ProcessMessageHandle {
        char name[MSG_NAME_MAX_LEN];
        uint8_t msg_id;
        void * parameters;
        timestamp_t remote_gen_timestamp;
        timestamp_t remote_tx_timestamp;
        timestamp_t rx_timestamp;
        uint16_t rx_cnt;
        uint8_t remote_tx_addr;
        ProcessMessageHandleFunc_p func;
    } ProcessMessageHandle_t;

    typedef struct {
        struct pt PT;
        XBee_p xbee;

        timestamp_t rx_timestamp;
        timestamp_t remote_tx_timestamp;
        RxIPv4Response_t rx_rsp;
        ProcessMessageHandle_t process_handle_list[MSG_MAX_PROCESS_FUNCS];
        size_t process_handle_list_num;
        uint8_t process_map[256];

        unsigned int cnt;
        unsigned int bad_rx_pkg_cnt;
        unsigned int bad_tx_pkg_cnt;

        TxIPv4Request_t tx_req;
        uint8_t msg_buff[MSG_TX_BUFF_LEN];
        uint8_t * msg_head[DEST_MAX_NUM+1];
        uint8_t * msg_tail[DEST_MAX_NUM+1];
        size_t msg_len[DEST_MAX_NUM];
        uint8_t msg_des_addr[DEST_MAX_NUM];

    } msgParam_t, *msgParam_p;

    struct SendMessageParam;
    typedef struct SendMessageParam* sendmsgParam_p;
    typedef bool (*SendMessageHandleFunc_p)(sendmsgParam_p);
    typedef struct SendMessageParam{
        struct pt PT;
        msgParam_p _msg;
        SendMessageHandleFunc_p _func;
        void * _param;
    } sendmsgParam_t;

    TargetIdx_t findTarget(msgParam_p parameters, uint8_t addr);
    bool pushMessage(msgParam_p parameters, TargetIdx_t target, uint8_t *msg, size_t msg_len);

    bool registerProcessMessageHandle(msgParam_p msg, char name[MSG_NAME_MAX_LEN], uint8_t id, ProcessMessageHandleFunc_p func, void * parameters);

    PT_THREAD(msgLoop)(TaskHandle_p task);
    PT_THREAD(sendmsgLoop)(TaskHandle_p task);

    void msgInit(msgParam_p parameters, XBee_p s6);
    void sendmsgInit(sendmsgParam_p parameters, msgParam_p msg, SendMessageHandleFunc_p func, void * param);

    uint8_t * packInt(uint8_t *pack, const void *data, size_t len);
    const uint8_t * unpackInt(const uint8_t *pack, void *data, size_t len);


#ifdef	__cplusplus
}
#endif

#endif	/* MSG_H */

