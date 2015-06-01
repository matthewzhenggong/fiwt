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
#include "Enc.h"

#if USE_ADC1

/* External variables declaration */
#if GNDBOARD
#include <dsp.h>
unsigned int RigPos[RIGPOSADCNUM];
int32_t RigRollPos=0;
int16_t RigPitchPos=0;
int16_t RigYawPos=0;
#else
uint8_t BattCell[BATTCELLADCNUM];
#if AC_MODEL
unsigned int ServoPos[SERVOPOSADCNUM];
#elif AEROCOMP
unsigned int ServoPos[SERVOPOSADCNUM];
#endif
#endif

timestamp_t ADC_TimeStamp;

void UpdateAnalogInputs(void) {
    ADC_TimeStamp = getMicroseconds();
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
    BattCell[0] = ADC1BUF4 >> 4; // Read data from AN12 input.
    BattCell[1] = ADC1BUF5 >> 4; // Read data from AN13 input.
    BattCell[2] = ADC1BUF6 >> 4; // Read data from AN14 input.
#elif AEROCOMP
    // Servo's Potentiometer readings
    ServoPos[0] = ADC1BUF5; // Read data from AN20 input.
    ServoPos[1] = ADC1BUF6; // Read data from AN21 input.
    ServoPos[2] = ADC1BUF1; // Read data from AN3 input.
    ServoPos[3] = ADC1BUF0; // Read data from AN2 input.
    // Battery Cells readings
    BattCell[0] = ADC1BUF2 >> 4; // Read data from AN12 input.
    BattCell[1] = ADC1BUF3 >> 4; // Read data from AN13 input.
    BattCell[2] = ADC1BUF4 >> 4; // Read data from AN14 input.
#elif GNDBOARD
    // Servo's Potentiometer readings
    RigPos[0] = ADC1BUF3; // Read data from AN20 input.
    RigPos[1] = ADC1BUF2; // Read data from AN21 input.
    RigPos[2] = ADC1BUF1; // Read data from AN5 input.
    RigPos[3] = ADC1BUF0; // Read data from AN4 input.
#endif

}

#if AEROCOMP

void UpdateServoPosFromEnc(void) {
    // Correct Encoder 1 Position
    EncPos[0] = 3342 - EncPos[0];
    // Correct Encoder 2 Position
    if (EncPos[1] >= 7000) {
        EncPos[1] = 2652 - (EncPos[1] - 8192);
    } else {
        EncPos[1] = 2652 - EncPos[1];
    }
    // Correct Encoder 3 Position
    EncPos[2] = 5456 - EncPos[2];
    // Correct Encoder 4 Position
    if (EncPos[3] >= 7000) {
        EncPos[3] = 2030 - (EncPos[3] - 8192);
    } else {
        EncPos[3] = 2030 - EncPos[3];
    }
//    static int16_t  LastEncPos[ENCNUM];
//
//    // Speed limitation check
//    for (i = 0; i < ENCNUM; ++i) {
//        if (CurEncPos[i] > LastEncPos[i]-256u && CurEncPos[i] < LastEncPos[i]+256u) {
//            EncPos[i] = CurEncPos[i];
//        }
//        LastEncPos[i] = CurEncPos[i];
//    }
}
#elif GNDBOARD

static int32_t LastRigRollPos=0;
static int16_t LastRigYawPos=0;
static int RigRollLoop=0;
static int RigYawLoop=0;

int32_t updateAngle32(int16_t CurRigRollPos, int *RigRollLoop, int32_t RigRollPos) {
    int32_t diff;
    diff = (CurRigRollPos+(*RigRollLoop)*3873)-(RigRollPos);
    /* Find the right loop index if there is a jump. */
    if (diff > 1936)
    {
        --(*RigRollLoop);
        diff = (CurRigRollPos+(*RigRollLoop)*3873)-(RigRollPos);
    }
    else if (diff < -1936)
    {
        ++(*RigRollLoop);
        diff = (CurRigRollPos+(*RigRollLoop)*3873)-(RigRollPos);
    }
    return RigRollPos + diff;
}

void UpdateRigPos(void) {
    int16_t diff;
        int16_t CurRigRollPos;
        int16_t CurRigPitchPos;
        int16_t CurRigYawPos;

        CurRigRollPos = -(RigPos[0]-2048)-112;
        CurRigPitchPos = (RigPos[2]-2657);
        CurRigYawPos = -(RigPos[3]-1610);

//        diff = CurRigRollPos-LastRigRollPos;
//        if (diff < 256 && diff > -256)
//        {
//            //RigRollPos = updateAngle32(CurRigRollPos, &RigRollLoop, RigRollPos);
//            RigRollPos = CurRigRollPos;
//        }
//        LastRigRollPos = CurRigRollPos;
        RigRollPos = CurRigRollPos;

        RigPitchPos = CurRigPitchPos;

        diff = CurRigYawPos-LastRigYawPos;
        if (diff < 256 && diff > -256)
        {
            RigYawPos = updateAngle32(CurRigYawPos, &RigYawLoop, RigYawPos);
        }
        LastRigYawPos = CurRigYawPos;
}

#endif

#endif /* USE_ADC1 */
