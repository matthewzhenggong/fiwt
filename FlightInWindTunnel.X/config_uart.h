/*
 * File:   config_uart.h
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

#ifndef CONFIG_UART_H
#define	CONFIG_UART_H

#include <xc.h>

/* Configure UARTx module to transmit 8 bit data with one stopbit. */
#define UxMODEvalue (0x0080)

#define UxSTAvalue (0x0000)

/*   Configure DMA Channel to: */
/*   Transfer data from RAM to UART */
/*   One-Shot mode */
/*   Register Indirect with Post-Increment */
/*   Using single buffer */
/*   8 transfers per buffer */
#define DMA_TO_UART_CONFIG (0x6001)

/*   Configure DMA Channel to: */
/*   Transfer data from RAM to UART */
/*   Continues mode */
/*   Register Indirect with Post-Increment */
/*   Using single buffer */
/*   8 transfers per buffer */
#define UART_TO_DMA_CONFIG (0x4000)

/***************************
 Fcy=64M
BRG=0 BD=8000000.00 7A1200
BRG=1 BD=4000000.00 3D0900
BRG=2 BD=2000000.00 1E8480
BRG=3 BD=1000000.00 0f4240
BRG=4 BD=800000.00 0xc3500
BRG=5 BD=666666.67 0xa2c2a
BRG=6 BD=571428.57 0x8b824
BRG=7 BD=500000.00 0x7a120
BRG=8 BD=444444.44 0x6c81c
BRG=9 BD=400000.00 0x61a80
BRG=10 BD=363636.36 0x58c74
BRG=11 BD=333333.33 0x51615
BRG=12 BD=307692.31 0x4b1ec
BRG=13 BD=285714.29 0x45c12
BRG=14 BD=266666.67 0x411aa
BRG=15 BD=250000.00 0x3d090
BRG=16 BD=235294.12 0x3971e
BRG=17 BD=222222.22 0x3640e
BRG=18 BD=210526.32 0x3365e
BRG=19 BD=200000.00 0x30d40
BRG=20 BD=190476.19 0x2e80c
BRG=21 BD=181818.18 0x2c63a
BRG=22 BD=173913.04 0x2a759
BRG=23 BD=166666.67 0x28b0a
BRG=24 BD=160000.00 0x27100
BRG=25 BD=153846.15 0x258f6
BRG=26 BD=148148.15 0x242b4
BRG=27 BD=142857.14 0x22e09
BRG=28 BD=137931.03 0x21acb
BRG=29 BD=133333.33 0x208d5
BRG=30 BD=129032.26 0x1f808
BRG=31 BD=125000.00 0x1e848
BRG=32 BD=121212.12 0x1d97c
BRG=33 BD=117647.06 0x1cb8f
********************************/

        
#if USE_UART1
/** UART1 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART1BAUdRATE  UARTBAUdRATE
#define UART1TXBUFFLEN (512)
#define UART1RXBUFFLEN (1024)
#if GNDBOARD  /* Only for GNDBOARD */
#define UART1TXTRIS (_TRISG1)
#define UART1RXTRIS (_TRISG0)
#define UART1RXPR (112) /* U1RX Input tied to RP112 */
#define UART1TXPR (_RP113R) /* U1TX is assigned to RP113 -> U1TX*/
#else    /* Only for AC_MODEL and AEROCOMP boards */
#define UART1TXTRIS (_TRISF0)
#define UART1RXTRIS (_TRISF1)
#define UART1RXPR (97) /* U1RX Input tied to RP97 */
#define UART1TXPR (_RP96R) /* U1TX is assigned to RP96 -> U1TX*/
#endif
#endif /*USE_UART1*/

#if USE_UART2
/** UART2 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART2BAUdRATE  UARTBAUdRATE
#define UART2TXBUFFLEN (512)
#define UART2RXBUFFLEN (1024)
#if GNDBOARD  /* Only for GNDBOARD */
#define UART2TXTRIS (_TRISF0)
#define UART2RXTRIS (_TRISF1)
#define UART2RXPR (97) /* U2RX Input tied to RP97 */
#define UART2TXPR (_RP96R) /* U2TX is assigned to RP96 -> U2TX*/
#else    /* Only for AC_MODEL and AEROCOMP boards */
#define UART2TXTRIS (_TRISG1)
#define UART2RXTRIS (_TRISG0)
#define UART2RXPR (112) /* U2RX Input tied to RP112 */
#define UART2TXPR (_RP113R) /* U2TX is assigned to RP113 -> U2TX*/
#endif
#endif /*USE_UART2*/

#if USE_UART3
/** UART3 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART3BAUdRATE  (0x3d090)//(0x3d090 0x61a80 0x7a120 0xc3500)
#define UART3TXBUFFLEN (32)
#define UART3RXBUFFLEN (128)
#define UART3TXTRIS (_TRISD6)
#define UART3RXTRIS (_TRISD7)
#define UART3RXPR (71) /* U3RX Input tied to RP71 */
#define UART3TXPR (_RP70R) /* U3TX is assigned to RP70 -> U3TX*/
#endif /*USE_UART3*/

#if USE_UART4
/** UART4 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART4BAUdRATE  (0x3d090)//(0x3d090 0x61a80 0x7a120 0xc3500)
#define UART4TXBUFFLEN (128)
#define UART4RXBUFFLEN (256)
#define UART4TXTRIS (_TRISD4)
#define UART4RXTRIS (_TRISD5)
#define UART4RXPR (69) /* U3RX Input tied to RP69 */
#define UART4TXPR (_RP68R) /* U3TX is assigned to RP68 -> U4TX*/
#endif /*USE_UART4*/


#endif	/* CONFIG_UART_H */

