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
#include "system.h"
#include <stdbool.h>

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

unsigned int  EncPos[ENCNUM];

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
    int32_t timestamp0, timestamp1, timestamp;
    /* Declare local variables */
    unsigned int EncBit_CNT;
    unsigned int  CurEncPos[ENCNUM];
    bool phase_sync;

    phase_sync = true;
    DisableInterrupts();
    //trigger for start
    ENC_CLOCK = 0;
    timestamp0 = getMicroseconds();
    timestamp1 = timestamp0+ENC_HALF_PEROID;

    //work
    /* Initialize local variables */
    for (i = 0; i < ENCNUM; ++i) {
        CurEncPos[i] = 0;
    }
    //wait
    timestamp = getMicroseconds();
    if (timestamp>=timestamp1) {
       phase_sync = false;
    }
    while (timestamp<timestamp1) {
        timestamp = getMicroseconds();
    }

    /* Read 13-bit resolution Digital Encoders */
    /* Synchro-Serial Interface reading */
    for (EncBit_CNT = 0; EncBit_CNT < 13; ++EncBit_CNT) {
        // Trigger ENC_CLOCK to a logical high state
        ENC_CLOCK = 1;        
        timestamp1 += ENC_HALF_PEROID;

        //work
        // Read Encoder data
        for (i = 0; i < ENCNUM; ++i) {
                CurEncPos[i] <<= 0b1;
        }
        //wait
        timestamp = getMicroseconds();
        if (timestamp>=timestamp1) {
           phase_sync = false;
        }
        while (timestamp<timestamp1) {
            timestamp = getMicroseconds();
        }

        // Trigger ENC_CLOCK to a logical low state
        ENC_CLOCK = 0;
        timestamp1 += ENC_HALF_PEROID;

        //work
        // Read Encoder data
        for (i = 0; i < ENCNUM; ++i) {
            if (*(enc_data[i].port) & enc_data[i].mask)
                CurEncPos[i] |= 0b1;
        }

        //wait
        timestamp = getMicroseconds();
        if (timestamp>=timestamp1) {
           phase_sync = false;
           break;
        }
        while (timestamp<timestamp1) {
            timestamp = getMicroseconds();
        }
    }

    //End. Set ENC_CLOCK to a logical high state
    ENC_CLOCK = 1;
    EnableInterrupts();
    if (phase_sync) {
        for (i = 0; i < ENCNUM; ++i) {
                EncPos[i] = CurEncPos[i];
        }
    }
}

#endif /* USE_ENC */
