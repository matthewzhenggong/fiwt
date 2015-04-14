/* 
 * File:   LedExtBoard.h
 * Author: Matt
 *
 * Created on April 14, 2015, 10:47 AM
 */

#ifndef LEDEXTBOARD_H
#define	LEDEXTBOARD_H

#include "config.h"

#if USE_LEDEXTBOARD

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif

/** 8 LEDs *********************************************************/
#define mInitAllLEDs()      TRISB &= 0xFFC0; TRISE &= 0xFCFF; LATB &= 0xFFC0; LATE &= 0xFCFF; 

#define mLED_1              LATBbits.LATB0
#define mLED_2              LATBbits.LATB1
#define mLED_3              LATBbits.LATB2
#define mLED_4              LATBbits.LATB3
#define mLED_5              LATBbits.LATB4
#define mLED_6              LATBbits.LATB5
#define mLED_7              LATEbits.LATE9
#define mLED_8              LATEbits.LATE8

#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2
#define mGetLED_3()         mLED_3
#define mGetLED_4()         mLED_4
#define mGetLED_5()         mLED_5
#define mGetLED_6()         mLED_6
#define mGetLED_7()         mLED_7
#define mGetLED_8()         mLED_8

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;
#define mLED_3_On()         mLED_3 = 1;
#define mLED_4_On()         mLED_4 = 1;
#define mLED_5_On()         mLED_5 = 1;
#define mLED_6_On()         mLED_6 = 1;
#define mLED_7_On()         mLED_7 = 1;
#define mLED_8_On()         mLED_8 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;
#define mLED_3_Off()        mLED_3 = 0;
#define mLED_4_Off()        mLED_4 = 0;
#define mLED_5_Off()        mLED_5 = 0;
#define mLED_6_Off()        mLED_6 = 0;
#define mLED_7_Off()        mLED_7 = 0;
#define mLED_8_Off()        mLED_8 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;
#define mLED_3_Toggle()     mLED_3 = !mLED_3;
#define mLED_4_Toggle()     mLED_4 = !mLED_4;
#define mLED_5_Toggle()     mLED_5 = !mLED_5;
#define mLED_6_Toggle()     mLED_6 = !mLED_6;
#define mLED_7_Toggle()     mLED_7 = !mLED_7;
#define mLED_8_Toggle()     mLED_8 = !mLED_8;

/** SWITCH *********************************************************/

/** I/O pin definitions ********************************************/
#define INPUT_PIN 1
#define OUTPUT_PIN 0



#ifdef	__cplusplus
}
#endif

#endif /* USE_LEDEXTBOARD */

#endif	/* LEDEXTBOARD_H */

