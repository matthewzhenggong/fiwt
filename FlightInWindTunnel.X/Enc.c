/*
 * File:   Enc.h
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Modified code from : Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
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

#include "Enc.h"

#if USE_ENC

#define ENC_CLOCK	LATHbits.LATH0
#define ENC1_DATA	PORTHbits.RH2
#define ENC2_DATA	PORTHbits.RH4
#define ENC3_DATA	PORTHbits.RH6
#if AEROCOMP
#define ENC4_DATA	PORTHbits.RH7
#endif

struct ENC {
    volatile unsigned int * port;
    unsigned int mask;
} enc_data[] = {
    {&PORTH, _PORTH_RH2_MASK},
    {&PORTH, _PORTH_RH4_MASK},
    {&PORTH, _PORTH_RH6_MASK},
#if AEROCOMP
    {&PORTH, _PORTH_RH7_MASK},
#endif
};

uint16_t EncPos[ENCNUM];
static int16_t LastEncPos[ENCNUM];

void EncInit(void) {
    /* 1) Configure ENC_CLOCK pin as output and ENC1_DATA, ENC2_DATA, ENC3_DATA pins as inputs*/
    TRISHbits.TRISH0 = 0b0; // ENC_CLOCK ouput in PORTH0
    TRISHbits.TRISH2 = 0b1; // ENC1_DATA input in PORTH2
    TRISHbits.TRISH4 = 0b1; // ENC2_DATA input in PORTH4
    TRISHbits.TRISH6 = 0b1; // ENC3_DATA input in PORTH6
#if AEROCOMP
    TRISHbits.TRISH7 = 0b1; // ENC4_DATA input in PORTH7
#endif
    ENC_CLOCK = 1;
}

void EncUpdate(void) {
    int i;
    /* Declare local variables */
    unsigned int EncBit_CNT;
    uint8_t sample[ENCNUM];
    int16_t CurEncPos[ENCNUM];

    /* Initialize local variables */

    for (i = 0; i < ENCNUM; ++i) {
        CurEncPos[i] = 0;
    }
    
    // pull down for start
    ENC_CLOCK = 0;
    asm ("repeat #32;"); Nop();

    /* Read 13-bit resolution Digital Encoders */
    /* 454.5 kHz Clock, Synchro-Serial Interface reading */
    for (EncBit_CNT = 0; EncBit_CNT < 13; ++EncBit_CNT) {
        // Set ENC_CLOCK to a logical high state
        ENC_CLOCK = 1;
        
        // Shift EncPos contents 1-bit to the left, to get correct bit alignment
        for (i = 0; i < ENCNUM; ++i) {
            CurEncPos[i] <<= 1;
            sample[i] = 0;
        }
        asm ("repeat #16;"); Nop();
        
        //first sample
        for (i = 0; i < ENCNUM; ++i) {
            if (*(enc_data[i].port) & enc_data[i].mask)
                ++sample[i];
        }

        // Set ENC_CLOCK to a logical low state
        ENC_CLOCK = 0;

        //second sample
        //asm ("repeat #4;"); Nop();
        for (i = 0; i < ENCNUM; ++i) {
            if (*(enc_data[i].port) & enc_data[i].mask)
                ++sample[i];
        }
        //third sample
        //asm ("repeat #3;"); Nop();
        for (i = 0; i < ENCNUM; ++i) {
            if (*(enc_data[i].port) & enc_data[i].mask)
                ++sample[i];
        }

        // Read Encoder data
        for (i = 0; i < ENCNUM; ++i) {
            if (sample[i] > 0b1)
                CurEncPos[i] |= 0b1;
        }
    }

    //End. Set ENC_CLOCK to a logical high state
    ENC_CLOCK = 1;

    // Speed limitation check
    for (i = 0; i < ENCNUM; ++i) {
        if (CurEncPos[i] > LastEncPos[i]-256u && CurEncPos[i] < LastEncPos[i]+256u) {
            EncPos[i] = CurEncPos[i];
        }
        LastEncPos[i] = CurEncPos[i];
    }
}

#endif /* USE_ENC */
