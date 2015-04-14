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

#define GNDBOARD 1
#define AC_MODEL 0
#define AEROCOMP 0
#define STARTKITBOARD 0

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        (128000000L)

#define Fcy             (SYS_FREQ/2L)

#if AC_MODEL
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   1
#define USE_SPIS  0
#elif AEROCOMP
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   0
#define USE_SPIS  0
#elif GNDBOARD
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 1
#define USE_UART4 1
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
#define USE_SPIS  1
#define USE_LEDEXTBOARD 1
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


#endif 	/* CONFIG_H */