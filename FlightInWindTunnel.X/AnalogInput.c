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
int16_t RigRollRate=0;
int16_t RigPitchRate=0;
int16_t RigYawRate=0;
#else
uint8_t BattCell[BATTCELLADCNUM];
#if AC_MODEL
unsigned int ServoPos[SERVOPOSADCNUM];
#elif AEROCOMP
unsigned int ServoPos[SERVOPOSADCNUM];
#endif
#endif

uint32_t ADC_TimeStamp;

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
//#elif GNDBOARD
#elif 0

/** two-order butterwolf filter 10Hz */
#define BUTTER_ORDER (2)
//    0.6012  -0.2536 0.3586
//    0.2536   0.9598 0.0568
//    0.08966  0.6929 0.02008
// x 2^15 =
static fractional _butter_mat_frac[] = { \
     19700, -8310, 11752,
      8310, 31452,  1861,
      2938, 22705,   658,
};

static fractional _butter_update(fractional input, fractional butt[BUTTER_ORDER + 1]) {
    fractional dstM[BUTTER_ORDER];
    int i;

    butt[BUTTER_ORDER] = input;
    MatrixMultiply(BUTTER_ORDER, BUTTER_ORDER + 1, 1, dstM, _butter_mat_frac, butt);
    for (i = 0; i < BUTTER_ORDER; ++i) {
        butt[i] = dstM[i];
    }
    MatrixMultiply(1, BUTTER_ORDER + 1, 1, dstM, _butter_mat_frac + BUTTER_ORDER * (BUTTER_ORDER + 1), butt);
    return dstM[0];
}


static int16_t LastRigRollPos=0;
static int16_t LastRigPitchPos=0;
static int16_t LastRigYawPos=0;
static int16_t RigRollLoop=0;

void UpdateRigPosAndRate(void) {
        int16_t CurRigRollPos;
        int16_t CurRigPitchPos;
        int16_t CurRigYawPos;
//        int32_t diff;

        CurRigRollPos = 0;
        CurRigPitchPos = 0;
        CurRigYawPos = 0;
//
//        diff = CurRigRollPos-LastRigRollPos;
//        if (diff < 256+RigRollRate && diff > -256+RigRollRate)
//        {
//            diff = (-CurRigRollPos+RigRollLoop*120)-(RigRollPos);
//            /* Find the right loop index if there is a jump. */
//            if (diff > 683)
//            {
//                --RigRollLoop;
//                diff = (-CurRigRollPos+RigRollLoop*120)-(RollDataOut);
//            }
//            else if (diff < -60)
//            {
//                ++circle_cnt;
//                diff = (-RollData+circle_cnt*120)-(RollDataOut);
//            }
//            RollDataOut += diff;
//            /* I would not like to output this odd value.
//             * So I select to save it in the bank.
//             * And I hope the opposite value comming soon.
//             */
///*
//            if (diff > 5+LastRollDataFilterDiff)
//            {
//                RollDataOutSaved += diff;
//            }
//            else if (diff < -5+LastRollDataFilterDiff)
//            {
//                RollDataOutSaved += diff;
//            }
//*/
//        }
//        else
//        {
//            RollDataOut += 0.7*LastRollDataFilterDiff;
//        }
//

    
}

#endif

#endif /* USE_ADC1 */
