/*
 * File:   sending.c
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

#include "sending.h"
#include "idle.h"
#include "clock.h"

#include <xc.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "StartKit.h"

void sendingInit(sendingParam_t *parameters, XBee_p xbee1, XBee_p xbee2) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->cnt = 0u;
    parameters->_xbee[0] = xbee2;
    parameters->_xbee[1] = xbee1;

    memcpy(parameters->tx_req._addr64, "\00\00\00\00\00\00\00\00", 8);
    parameters->tx_req._addr16 = 0xFFFE;
    parameters->tx_req._broadcastRadius = 1u;
    parameters->tx_req._option = 1u;
    parameters->tx_req._payloadLength = 1u;
    parameters->tx_req._payloadPtr[0] = 'P';
}

extern idleParam_t idle_params;

static void test(void) {
    int i;
    float f;
    for (i=0,f=1.0f; i<100; ++i) {
        f = f/1.2f;
    }
    return;
}

PT_THREAD(sendingLoop)(TaskHandle_p task) {
    sendingParam_t *parameters;
    struct pt *pt;
    unsigned int t1,t2,t3,i;
    parameters = (sendingParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        ++parameters->cnt;
        if (parameters->cnt & 1) {
            /** Test One */
            sprintf((char *) (parameters->tx_req._payloadPtr + 1u), "%05d %05u.%03u.%03u S%03u C%lu T%lu I%lu AD%u", \
                    (parameters->cnt) >> 1u, RTclock.seconds, RTclock.ticks, microsec_ticks, \
                    task->load_max, task->runtime_cnt, task->runtime_microseconds, idle_params.call_per_second, \
                    ADC1BUF0);
            parameters->tx_req._payloadLength = strlen((const char *) parameters->tx_req._payloadPtr);
            XBeeZBTxRequest(parameters->_xbee[0], &parameters->tx_req, 0u);
        } else {
            /** Test Two */
            t1 = microsec_ticks;
            test();
            t2 = microsec_ticks;
            asm volatile ("REPEAT, #640"); ++i;
            t3 = microsec_ticks;
            sprintf((char *) (parameters->tx_req._payloadPtr + 1u), "%05d %05u.%03u.%03u T%03u T%03u T%03u", (parameters->cnt) >> 1u, RTclock.seconds, RTclock.ticks, microsec_ticks, t3, t2, t1);
            parameters->tx_req._payloadLength = strlen((const char *) parameters->tx_req._payloadPtr);
            XBeeZBTxRequest(parameters->_xbee[1], &parameters->tx_req, 0u);
            mLED_2_Toggle();
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}


