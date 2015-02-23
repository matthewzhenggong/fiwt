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
#define AEROCOMP 1

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
#elif AEROCOMP
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   0
#elif GNDBOARD
#define USE_UART1 1
#define USE_UART2 1
#define USE_UART3 1
#define USE_UART4 1
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
#else
#define USE_UART1 0
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
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
#define TASK_PERIOD (10u) // 100Hz

/** common hardware config*/
#include "config_uart.h"
#include "config_adc.h"

/** PWM in 10kHz */
#define PWM_FREQ 10000u
#include "config_pwm.h"

#if AC_MODEL
#define ENCNUM (3)
#elif AEROCOMP
#define ENCNUM (4)
#elif GNDBOARD
#define ENCNUM (3)
#endif


#if AC_MODEL
#define SERVO_ACCEL_LIMIT (35)
#define SEVERONUM (4)
#define SERVO_K (15)
#define SERVO_S (3)
#define SERVO_DIFF_LMT ((2^15)/(SERVO_K+1))
#define SERVO_SHAKE (330)
#define SERVO_SHAKE_TICKS (20)
#elif AEROCOMP
#define SERVO_ACCEL_LIMIT (35)
#define SEVERONUM (4)
#define SERVO_K (19)
#define SERVO_S (3)
#define SERVO_DIFF_LMT ((2^15)/(SERVO_K+1))
#define SERVO_SHAKE (343)
#define SERVO_SHAKE_TICKS (20)
#endif


#endif 	/* CONFIG_H */