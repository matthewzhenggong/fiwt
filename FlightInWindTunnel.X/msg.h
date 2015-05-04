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
#define MSG_ID_POS (4)

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
        uint32_t remote_gen_timestamp;
        uint32_t remote_tx_timestamp;
        uint32_t rx_timestamp;
        uint16_t rx_cnt;
        uint16_t rx_port;
        uint16_t remote_tx_port;
        ProcessMessageHandleFunc_p func;
    } ProcessMessageHandle_t;

    struct PushMessageHandle;
    typedef struct PushMessageHandle* PushMessageHandle_p;
    typedef size_t (*PushMessageHandleFunc_p)(PushMessageHandle_p, uint8_t *head, size_t max_len);
    typedef struct PushMessageHandle {
        char name[MSG_NAME_MAX_LEN];
        void * parameters;
        unsigned int tx_cnt;
        unsigned int peroid;
        unsigned int offset;
        uint16_t target;
        uint32_t timestamp;
        PushMessageHandleFunc_p func;
    } PushMessageHandle_t;

    typedef struct {
        struct pt PT;
        XBee_p xbee;

        uint32_t rx_timestamp;
        uint32_t remote_tx_timestamp;
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
        uint16_t msg_port[DEST_MAX_NUM];

        PushMessageHandle_t push_handle_list[MSG_MAX_PUSH_FUNCS];
        size_t push_handle_list_num;
        uint8_t push_buff[MSG_TX_BUFF_LEN];
    } msgParam_t, *msgParam_p;

    bool pushMessage(msgParam_p parameters, uint16_t des_port, uint8_t *msg, size_t msg_len);

    bool registerProcessMessageHandle(msgParam_p msg, char name[MSG_NAME_MAX_LEN], uint8_t id, ProcessMessageHandleFunc_p func, void * parameters);

    bool registerPushMessageHandle(msgParam_p msg, char name[MSG_NAME_MAX_LEN],
            PushMessageHandleFunc_p func, void * parameters, uint16_t target,
            unsigned int circle, unsigned int offset);

    PT_THREAD(msgLoop)(TaskHandle_p task);

    void msgInit(msgParam_p parameters, XBee_p s6);


#ifdef	__cplusplus
}
#endif

#endif	/* MSG_H */

