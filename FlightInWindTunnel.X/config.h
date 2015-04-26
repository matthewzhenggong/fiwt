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

#define AC_MODEL 1

#define AEROCOMP 0

#define STARTKITBOARD 0

#define XBEE1_ATAP 2
#define XBEE1_SERIES XBeeS6

// IP=192.169.191.255
#define MSG_DEST_ADDR_MSW (0xc0a8)
#define MSG_DEST_ADDR_LSW (0xbfff)
#define MSG_DEST_ADDR "\x00\x00\x00\x00\xc0\xa8\xbf\xff"
#if AC_MODEL || AEROCOMP
#define MSG_DEST_PORT 0x2616
#elif GNDBOARD
#define MSG_DEST_ACM_PORT 0x2267
#define MSG_DEST_CMP_PORT 0x2677
#endif
#if AC_MODEL
#define MSG_SRC_PORT 0x2267
#elif AEROCOMP
#define MSG_SRC_PORT 0x2677
#endif


/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        (128000000L)

#define Fcy             (SYS_FREQ/2L)

#if AC_MODEL
#define TASK_PERIOD (4u) // 250Hz
#define USE_UART1 1
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   1
#define USE_SPIS  0
#define USE_EKF   0
#define USE_TESTBOARD 1
#elif AEROCOMP
#define TASK_PERIOD (4u) // 250Hz
#define USE_UART1 1
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   0
#define USE_SPIS  0
#define USE_EKF   0
#define XBEE1_ATAP 2
#define XBEE2_ATAP 2

#elif GNDBOARD
#define TASK_PERIOD (2u) // 500Hz
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 1
#define USE_UART4 1
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
#define USE_SPIS  1
#define USE_EKF   0
#define USE_LEDEXTBOARD 1
#define NOT_USE_EXTOSC 0
#define XBEE1_ATAP 2
#define XBEE2_ATAP 2
#define XBEE3_ATAP 2
#define XBEE4_ATAP 2

#elif STARTKITBOARD
#define USE_UART1 0
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   1
#define USE_SPIS  0
#define USE_EKF   0
#else
#define USE_UART1 0
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
#define USE_SPIS  0
#define USE_EKF   0
#endif

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


/**
 * Task related definitions
 */
#define TASK_NAME_MAX (8)
#define MAX_NUM_TASKS (8)

#if GNDBOARD
#define BATTCELLADCNUM (0)
#define SERVOPOSADCNUM (0)
#define SEVERONUM (0)
#else
#define BATTCELLADCNUM (3)
#if AC_MODEL
#define SERVOPOSADCNUM (6)
#define SEVERONUM (6)
#elif AEROCOMP
#define SERVOPOSADCNUM (4)
#define SEVERONUM (4)
#endif
#endif

#define MSG_DILIMITER (0x9E) //0x80+0x1E(RS)
#define MSG_ESC (0x9B) //0x80+0x1B(ESC)
#define MSG_ESC (0x9B) //0x80+0x1B(ESC)


#endif 	/* CONFIG_H */