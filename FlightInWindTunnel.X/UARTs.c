/*
 * File:   UARTs.c
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

#if USE_UART1

/** Compute baudrate value and store in BR_Value */
#define UARTxTXPR 0x01
#define DMAxTXREQ 0b00001100
#define DMAxRXREQ 0b00001011

#define UARTx(PARAM) UART1##PARAM
#define UxREG(REG) U1##REG
#define _UxREG(REG) _U1##REG
#define DMAxTX(REG) DMA0##REG
#define DMAxRX(REG) DMA4##REG
#define _DMAxTX(REG) _DMA0##REG
#define _DMAxRX(REG) _DMA4##REG

#include "UARTx.c"

#undef UARTxTXPR
#undef DMAxTXREQ
#undef DMAxRXREQ

#undef UARTx
#undef UxREG
#undef _UxREG
#undef DMAxTX
#undef DMAxRX
#undef _DMAxTX
#undef _DMAxRX

#endif /* UART1ENABLE */

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

#undef UARTxTXPR
#undef DMAxTXREQ
#undef DMAxRXREQ

#undef UARTx
#undef UxREG
#undef _UxREG
#undef DMAxTX
#undef DMAxRX
#undef _DMAxTX
#undef _DMAxRX

#endif /* UART2ENABLE */

#if USE_UART3

/** Compute baudrate value and store in BR_Value */
#define UARTxTXPR 0x1B
#define DMAxTXREQ 0b01010011
#define DMAxRXREQ 0b01010010

#define UARTx(PARAM) UART3##PARAM
#define UxREG(REG) U3##REG
#define _UxREG(REG) _U3##REG
#define DMAxTX(REG) DMA2##REG
#define DMAxRX(REG) DMA6##REG
#define _DMAxTX(REG) _DMA2##REG
#define _DMAxRX(REG) _DMA6##REG

#include "UARTx.c"

#undef UARTxTXPR
#undef DMAxTXREQ
#undef DMAxRXREQ

#undef UARTx
#undef UxREG
#undef _UxREG
#undef DMAxTX
#undef DMAxRX
#undef _DMAxTX
#undef _DMAxRX

#endif /* UART3ENABLE */

#if USE_UART4

/** Compute baudrate value and store in BR_Value */
#define UARTxTXPR 0x1D
#define DMAxTXREQ 0b01011001
#define DMAxRXREQ 0b01011000

#define UARTx(PARAM) UART4##PARAM
#define UxREG(REG) U4##REG
#define _UxREG(REG) _U4##REG
#define DMAxTX(REG) DMA3##REG
#define DMAxRX(REG) DMA7##REG
#define _DMAxTX(REG) _DMA3##REG
#define _DMAxRX(REG) _DMA7##REG

#include "UARTx.c"

#undef UARTxTXPR
#undef DMAxTXREQ
#undef DMAxRXREQ

#undef UARTx
#undef UxREG
#undef _UxREG
#undef DMAxTX
#undef DMAxRX
#undef _DMAxTX
#undef _DMAxRX

#endif	/* UART4ENABLE */