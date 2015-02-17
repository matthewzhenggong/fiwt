/*
 * File:   UARTs.h
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


#ifndef UARTS_H
#define	UARTS_H

#include "config.h"


#if USE_UART1
#ifdef UARTX_H
#undef UARTX_H
#endif
#ifdef UARTxFunc
#undef UARTxFunc
#endif
#define UARTxFunc(FUN) UART1##FUN
#include "UARTx.h"
#endif /* USE_UART1 */

#if USE_UART2
#ifdef UARTX_H
#undef UARTX_H
#endif
#ifdef UARTxFunc
#undef UARTxFunc
#endif
#define UARTxFunc(FUN) UART2##FUN
#include "UARTx.h"
#endif /* USE_UART2 */

#if USE_UART3
#ifdef UARTX_H
#undef UARTX_H
#endif
#ifdef UARTxFunc
#undef UARTxFunc
#endif
#define UARTxFunc(FUN) UART3##FUN
#include "UARTx.h"
#endif /* USE_UART3 */

#if USE_UART4
#ifdef UARTX_H
#undef UARTX_H
#endif
#ifdef UARTxFunc
#undef UARTxFunc
#endif
#define UARTxFunc(FUN) UART4##FUN
#include "UARTx.h"
#endif /* USE_UART4 */

#endif	/* UARTS_H */

