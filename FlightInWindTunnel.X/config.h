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

#define FCY             (SYS_FREQ/2L)

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


/** UART1 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART1BAUdRATE  (0x03d090)
#define UART1TXBUFFLEN (128)
#define UART1RXBUFFLEN (256)

/** UART2 Baudrate definition (Posible values: 9600, 57600, 115200) */
#define UART2BAUdRATE  (0x03d090)
#define UART2TXBUFFLEN (128)
#define UART2RXBUFFLEN (256)

/**
 * Task related definitions
 */
#define TASK_NAME_MAX (8)
#define MAX_NUM_TASKS (8)

#endif 	/* CONFIG_H */