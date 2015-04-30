/*
 * File:   user.h
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

#ifndef USER_H
#define	USER_H

/******************************************************************************/
/* User Level Headers                                                         */
/******************************************************************************/

#include "SerialStream.h"
#include "XBee.h"

/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/


#ifdef	__cplusplus
extern "C" {
#endif

    /**************************************************************************/
    /* User Instance Prototypes                                               */
    /**************************************************************************/
#if USE_UART1
    extern XBee_t Xbee1;
#endif
#if USE_UART2
    extern XBee_t Xbee2;
#endif
#if USE_UART3
    extern XBee_t Xbee3;
#endif /*USE_UART3*/
#if USE_UART4
    extern XBee_t Xbee4;
#endif /*USE_UART4*/
    /**************************************************************************/
    /* User Function Prototypes                                               */
    /**************************************************************************/
    void InitApp(void); /* I/O and Peripheral Initialization */

#ifdef	__cplusplus
}
#endif

#endif	/* USER_H */

