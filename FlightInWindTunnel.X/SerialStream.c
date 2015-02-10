/*
 * File:   SerialStream.c
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

#include "SerialStream.h"
#include "UART1.h"
#include "UART2.h"

void serial_assign(SerialStream_p _serial, unsigned int UARTx) {
    switch (UARTx) {
        case 2u:
            /** associate fuctions of UART1 to the serial object*/
            _serial->init = &UART2Init;
            _serial->open = &UART2Start;
            _serial->available = &UART2GetAvailable;
            _serial->read = &UART2GetByte;
            _serial->write = &UART2SendByte;
            _serial->flush = &UART2Flush;
            _serial->readBytes = &UART2GetBytes;
            _serial->writeBytes = &UART2SendBytes;
            _serial->writeString = &UART2SendString;
            break;
        case 1u:
        default:
            /** associate fuctions of UART1 to the serial object*/
            _serial->init = &UART1Init;
            _serial->open = &UART1Start;
            _serial->available = &UART1GetAvailable;
            _serial->read = &UART1GetByte;
            _serial->write = &UART1SendByte;
            _serial->flush = &UART1Flush;
            _serial->readBytes = &UART1GetBytes;
            _serial->writeBytes = &UART1SendBytes;
            _serial->writeString = &UART1SendString;
    }

}
