/*
 * File:   system.h
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

/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */

/**
 * Disable interrupts which's prioritiy is less than or equal to clock timer's level
 * Call this funtion to block all software interrupts and clock interrupt.
 * Only hardware interrupts are responsive.
 */
void DisableInterrupts(void);

/**
 * Enable all levels of interrupts.
 * All hardware, software and clock interrupts are responsive.
 */
void EnableInterrupts(void);

