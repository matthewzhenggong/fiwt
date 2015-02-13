/*
 * File:   echoTask.c
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

#include "echoTask.h"
#include "SerialStream.h"


void echoInit(echoParam_t *parameters, SerialStream_p s) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_serial = s;
}

PT_THREAD(echoLoop)(TaskHandle_p task) {
    echoParam_t *parameters;
    struct pt *pt;
    SerialStream_p s;
    uint8_t *buff;
    size_t n;
    
    parameters = (echoParam_t *) (task->parameters);
    pt = &(parameters->PT);
    s = parameters->_serial;
    buff = parameters->_buff;

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        if (s->available()) {
            n = s->readBytes(buff, 300);
            s->writeBytes(buff, n);
            s->flush();
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}
