/*
 * File:   UART2.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Original author: Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
 * Some code may also be from Microchip's DataSheet Documents.
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

#if USE_UART2

/** Compute baudrate value and store in BR_Value */
#define UARTxTXPR 0x03
#define DMAxTXREQ 0b00011111
#define DMAxRXREQ 0b00011110

#define UARTx(PARAM) UART2##PARAM
#define UxREG(REG) U2##REG
#define _UxREG(REG) _U2##REG
#define DMAxTX(REG) DMA1##REG
#define DMAxRX(REG) DMA5##REG
#define _DMAxTX(REG) _DMA1##REG
#define _DMAxRX(REG) _DMA5##REG

#include "UARTx.c"

#endif /* UART2ENABLE */
