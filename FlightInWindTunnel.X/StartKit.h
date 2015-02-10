/* 
 * File:   StartKit.h
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

#ifndef STARTKIT_H
#define	STARTKIT_H

/**
 * @file StartKit.h
 *
 * All macro used to manual the LEDs and SWs
 * on Microchip USB Starter kit<DM330012>.
 */

/** 4 LEDs *********************************************************/
#define mInitAllLEDs()      LATD &= 0xFFF8; TRISD &= 0xFFF8;

#define mLED_1              LATDbits.LATD0
#define mLED_2              LATDbits.LATD1
#define mLED_3              LATDbits.LATD2
#define mLED_4

#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2
#define mGetLED_3()         mLED_3
#define mGetLED_4()         1

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;
#define mLED_3_On()         mLED_3 = 1;
#define mLED_4_On()

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;
#define mLED_3_Off()        mLED_3 = 0;
#define mLED_4_Off()

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;
#define mLED_3_Toggle()     mLED_3 = !mLED_3;
#define mLED_4_Toggle()

/** SWITCH *********************************************************/
#define mInitSwitch2()      TRISDbits.TRISD6=1;
#define mInitSwitch3()      TRISDbits.TRISD7=1;
#define mInitAllSwitches()  mInitSwitch2();mInitSwitch3();
#define sw2                 PORTDbits.RD6
#define sw3                 PORTDbits.RD7

/** I/O pin definitions ********************************************/
#define INPUT_PIN 1
#define OUTPUT_PIN 0

#endif	/* STARTKIT_H */

