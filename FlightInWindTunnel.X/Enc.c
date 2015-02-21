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

unsigned int EncPos[ENCNUM];

void EncStart(void) {
    /* 1) Configure ENC_CLOCK pin as output and ENC1_DATA, ENC2_DATA, ENC3_DATA pins as inputs*/
    TRISHbits.TRISH0 = 0b0; // ENC_CLOCK ouput in PORTH0
    TRISHbits.TRISH2 = 0b1; // ENC1_DATA input in PORTH2
    TRISHbits.TRISH4 = 0b1; // ENC2_DATA input in PORTH4
    TRISHbits.TRISH6 = 0b1; // ENC3_DATA input in PORTH6
#if AEROCOMP
    TRISHbits.TRISH7 = 0b1; // ENC4_DATA input in PORTH7
#endif
}

void EncUpdate(void) {
    int i;
    /* Declare local variables */
    unsigned int EncBit_CNT;

    /* Initialize local variables */

    for (i = 0; i < ENCNUM; ++i) {
        EncPos[i] = 0;
    }

    /* Store reading time */
    ENC_TimeStamp = RTclock;

    /* Read 13-bit resolution Digital Encoders */
    /* 454.5 kHz Clock, Synchro-Serial Interface reading */
    for (EncBit_CNT = 0; EncBit_CNT < 13; ++EncBit_CNT) {
        // Set ENC_CLOCK to a logical low state
        ENC_CLOCK = 0;

        // Shift EncPos contents 1-bit to the left, to get correct bit alignment
        for (i = 0; i < ENCNUM; ++i) {
            EncPos[i] <<= 1;
        }

        // Stop mcu operation, hold ENC_CLOCK value for 1 us 
        //TODO Check Encode SSI frequency
        asm ("repeat #64;"); Nop();

        // Set ENC_CLOCK to a logical high state
        ENC_CLOCK = 1;

        // Stop mcu operation, to account for delay in Encoder signal
        asm ("repeat #64;"); Nop();

        // Read Encoder data
        for (i = 0; i < ENCNUM; ++i) {
            EncPos[i] |= *(enc_data[i].port) & enc_data[i].mask;
        }
    }
    /* Finally when the LSB is transferred (end of transmission)
     *  an additional rising clock will set the data output to LOW level.
     * This will be held low for 20 ±1 ?s (monoflop time) */
    // Set ENC_CLOCK to a logical low state
    ENC_CLOCK = 0;
    // Stop mcu operation, hold ENC_CLOCK value for 1 us
    asm ("repeat #64;"); Nop();

    // Set ENC_CLOCK to a logical high state
    ENC_CLOCK = 1;

#if AEROCOMP
    // Correct Encoder 1 Position
    EncPos[0] = 2692 - EncPos[0];
    EncPos[0] += 650;
    // Correct Encoder 2 Position
    if ((EncPos[1] >= 7000) && (EncPos[1] <= 8191)) {
        EncPos[1] = 2002 - (EncPos[1] - 8192);
    } else {
        EncPos[1] = 2002 - EncPos[1];
    }
    EncPos[1] += 650;
    // Correct Encoder 3 Position
    EncPos[2] = 4806 - EncPos[2];
    EncPos[2] += 650;
    // Correct Encoder 4 Position
    if ((EncPos[3] >= 7000) && (EncPos[3] <= 8191)) {
        EncPos[3] = 1380 - (EncPos[3] - 8192);
    } else {
        EncPos[3] = 1380 - EncPos[3];
    }
    EncPos[3] += 650;
#endif
}