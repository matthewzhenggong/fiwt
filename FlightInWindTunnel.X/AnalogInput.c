/*
 * File:   AnalogInput.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Original code from : Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
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

#include "AnalogInput.h"

#if USE_PWM && USE_ADC1

/* External variables declaration */
#if GNDBOARD
#if DEBUG == 0
extern unsigned int ADC_Input[8];
#endif
#else
__near unsigned int BattCell[BATTCELLADCNUM];
#if AC_MODEL
__near unsigned int ServoPos[SERVOPOSADCNUM];
#elif AEROCOMP
__near unsigned int ServoPos[SERVOPOSADCNUM];
#endif
#endif

__near clockType_t AnalogInputTimeStamp;

void UpdateAnalogInputs(void) {
    AnalogInputTimeStamp = RTclock;
    /* Read ADC conversion result */
#if AC_MODEL
    // Servo's Potentiometer readings
    ServoPos[0] = ADC1BUF7; // Read data from AN20 input.
    ServoPos[1] = ADC1BUF8; // Read data from AN21 input.
    ServoPos[2] = ADC1BUF3; // Read data from AN5 input.
    ServoPos[3] = ADC1BUF2; // Read data from AN3 input.
    ServoPos[4] = ADC1BUF1; // Read data from AN2 input.
    ServoPos[5] = ADC1BUF0; // Read data from AN1 input.
    // Battery Cells readings
    BattCell[0] = ADC1BUF4 >> 3; // Read data from AN12 input.
    BattCell[1] = ADC1BUF5 >> 3; // Read data from AN13 input.
    BattCell[2] = ADC1BUF6 >> 3; // Read data from AN14 input.
#elif AEROCOMP
    // Servo's Potentiometer readings
    ServoPos[0] = ADC1BUF5; // Read data from AN20 input.
    ServoPos[1] = ADC1BUF6; // Read data from AN21 input.
    ServoPos[2] = ADC1BUF1; // Read data from AN3 input.
    ServoPos[3] = ADC1BUF0; // Read data from AN2 input.
    // Battery Cells readings
    BattCell[0] = ADC1BUF2; // Read data from AN12 input.
    BattCell[1] = ADC1BUF3; // Read data from AN13 input.
    BattCell[2] = ADC1BUF4; // Read data from AN14 input.
#endif

}

#if AEROCOMP

void UpdateServoPosFromEnc(void) {
    // Correct Encoder 1 Position
    ServoPos[0] = 2692 - EncPos[0];
    ServoPos[0] += 650;
    // Correct Encoder 2 Position
    if ((EncPos[1] >= 7000) && (EncPos[1] <= 8191)) {
        ServoPos[1] = 2002 - (EncPos[1] - 8192);
    } else {
        ServoPos[1] = 2002 - EncPos[1];
    }
    ServoPos[1] += 650;
    // Correct Encoder 3 Position
    ServoPos[2] = 4806 - EncPos[2];
    ServoPos[2] += 650;
    // Correct Encoder 4 Position
    if ((EncPos[3] >= 7000) && (EncPos[3] <= 8191)) {
        ServoPos[3] = 1380 - (EncPos[3] - 8192);
    } else {
        ServoPos[3] = 1380 - EncPos[3];
    }
    ServoPos[3] += 650;
}
#endif

#endif /* USE_ADC1 */
