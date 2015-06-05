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

// IP=192.169.191.255
#define MSG_NETWORK_MSW (0xc0a8)
#define MSG_NETWORK_LSW (0xbf00)
#define MSG_APC_ADDR (0x01)
#define MSG_GND_ADDR (0x02)
#define MSG_CMP_ADDR (0x03)
#define MSG_ACM_ADDR (0x04)

#define MSG_DES_ADDR MSG_APC_ADDR

#define MSG_SRC_PORT 0x2000
#define MSG_DES_PORT MSG_SRC_PORT
#define DEST_MAX_NUM (3)
#define MSG_TX_BUFF_LEN (600)

#if AC_MODEL || AEROCOMP
#define TASK_PERIOD (4u)
#define XBEE1_ATAP 2
#define ENC_HALF_PEROID (5)

#if AC_MODEL
typedef enum TargetIdx {
    TargetAP=0,
    TargetGND, TargetCMP
} TargetIdx_t;
#define MSG_DES2_ADDR MSG_GND_ADDR
#define MSG_DES3_ADDR MSG_CMP_ADDR

#elif AEROCOMP
typedef enum TargetIdx {
    TargetAP=0,
    TargetGND, TargetACM
} TargetIdx_t;
#define MSG_DES2_ADDR MSG_GND_ADDR
#define MSG_DES3_ADDR MSG_ACM_ADDR
#endif

#elif GNDBOARD
#define TASK_PERIOD (4u) // 250Hz
#define XBEE2_ATAP 2
typedef enum TargetIdx {
    TargetAP=0,
    TargetCMP, TargetACM
} TargetIdx_t;
#define MSG_DES2_ADDR MSG_CMP_ADDR
#define MSG_DES3_ADDR MSG_ACM_ADDR

#endif


/*************************
 * Hardware configuration
 *************************/
#define UARTBAUdRATE (0x3D0900)


/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        (128000000L)
#define Fcy             (SYS_FREQ/2L)

#if AC_MODEL
#define USE_UART1 1
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   1
#define USE_EKF   0
#define USE_TESTBOARD 0
#define ENCNUM (3)
#define BATTCELLADCNUM (3)
#define SERVOPOSADCNUM (6)
#define SEVERONUM (6)

#elif AEROCOMP
#define USE_UART1 1
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   1
#define USE_ENC   1
#define USE_IMU   0
#define ENCNUM (4)
#define USE_TESTBOARD (0)
#define BATTCELLADCNUM (3)
#define SERVOPOSADCNUM (4)
#define SEVERONUM (4)

#elif GNDBOARD
#define USE_UART1 0
#define USE_UART2 1
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  1
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   0
#define USE_LEDEXTBOARD 1
#define NOT_USE_EXTOSC 0
#define ENCNUM (0)
#define RIGPOSADCNUM (4)
#define BATTCELLADCNUM (0)
#define SERVOPOSADCNUM (0)
#define SEVERONUM (0)

#elif STARTKITBOARD
#define USE_UART1 0
#define USE_UART2 0
#define USE_UART3 0
#define USE_UART4 0
#define USE_ADC1  0
#define USE_PWM   0
#define USE_ENC   0
#define USE_IMU   1
#define ENCNUM (0)

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
#define ENCNUM (0)
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
#define TASK_TIMESLICE_BITS (10)
#define TASK_TIMESLICE_MASK (0x3FF)

#define MSG_DILIMITER (0x9E) //0x80+0x1E(RS)
#define MSG_ESC (0x9B) //0x80+0x1B(ESC)


#endif 	/* CONFIG_H */