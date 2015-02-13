/*
 * File:   config.h
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


#ifndef CONFIG_H
#define	CONFIG_H

/** 
 * @file Global configureation macros and constants
 */

#define GNDBOARD 0
#define AC_MODEL 0
#define AEROCOMP 0
#define STARTKITBOARD 1

/* Microcontroller MIPs (FCY) */
#if STARTKITBOARD
#define SYS_FREQ        (128000000L)
#endif

#define Fcy             (SYS_FREQ/2L)

#if STARTKITBOARD
#define UART1ENABLE 1
#define UART2ENABLE 1
#define UART3ENABLE 1
#define UART4ENABLE 1
#else
#define UART1ENABLE 0
#define UART2ENABLE 0
#define UART3ENABLE 0
#define UART4ENABLE 0
#endif

#include <xc.h>

/**
 * Interrupt LEVEL
 *
 * @note
 * 1-3 : software int
 * 4   : schedule timer int
 * 5-7 : peripheral and DMA int
 */
#define SOFTWARE_INTERRUPT_PRIORITY_LEVEL_LOW (1)
#define SOFTWARE_INTERRUPT_PRIORITY_LEVEL_MED (2)
#define SOFTWARE_INTERRUPT_PRIORITY_LEVEL_HIG (3)
#define SCHEDULE_TIMER_PRIORITY_LEVEL         (4)
#define HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW (5)
#define HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED (6)
#define HARDWARE_INTERRUPT_PRIORITY_LEVEL_HIG (7)

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

/** UART1 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART1BAUdRATE  (0x03d090)
#define UART1TXBUFFLEN (128)
#define UART1RXBUFFLEN (256)
#if GNDBOARD  /* Only for GNDBOARD */
#define UART1TXTRIS (_TRISG0)
#define UART1RXTRIS (_TRISG1)
#define UART1RXPR (112) /* U1RX Input tied to RP112 */
#define UART1TXPR (_RP113R) /* U1TX is assigned to RP113 -> U1TX*/
#else    /* Only for AC_MODEL and AEROCOMP boards */
#define UART1TXTRIS (_TRISF0)
#define UART1RXTRIS (_TRISF1)
#define UART1RXPR (97) /* U1RX Input tied to RP97 */
#define UART1TXPR (_RP96R) /* U1TX is assigned to RP96 -> U1TX*/
#endif

/** UART2 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART2BAUdRATE  (0x03d090)
#define UART2TXBUFFLEN (128)
#define UART2RXBUFFLEN (256)
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

/** UART3 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART3BAUdRATE  (0x03d090)
#define UART3TXBUFFLEN (128)
#define UART3RXBUFFLEN (256)
#define UART3TXTRIS (_TRISD6)
#define UART3RXTRIS (_TRISD7)
#define UART3RXPR (71) /* U3RX Input tied to RP71 */
#define UART3TXPR (_RP70R) /* U3TX is assigned to RP70 -> U3TX*/

/** UART4 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART4BAUdRATE  (0x03d090)
#define UART4TXBUFFLEN (128)
#define UART4RXBUFFLEN (256)
#define UART4TXTRIS (_TRISD4)
#define UART4RXTRIS (_TRISD5)
#define UART4RXPR (69) /* U3RX Input tied to RP69 */
#define UART4TXPR (_RP68R) /* U3TX is assigned to RP68 -> U4TX*/

/**
 * Task related definitions
 */
#define TASK_NAME_MAX (8)
#define MAX_NUM_TASKS (8)

#endif 	/* CONFIG_H */