/*
 * File:   ADC1.c
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


#include "config.h"

#if USE_ADC1

#include "config_adc.h"
#include "ADC1.h"

#include <xc.h>

void ADC1Init(void) {
    /* Function ConfigADC1
     * ADC1 module supports 10-bit and 12-bit operation.
     * In 10-bit operation mode; it can use up to 4 Sample and Hold (S&H) channels (Default Configuration).
     *        Simultaneous sampling of up to four analog input pins (channels), is supported.
     *        Additionally, conversions speeds up to 1.1 Msps are supported.
     * In 12-bit operation mode; ADC can use only 1 S&H channel.
     *        Simultaneous sampling of multiple channels is not supported.
     *        Conversions speeds up to 500 ksps are supported. */

    /* Analog Inputs used on Boards */
    // GNDBOARD up to 8 Analog Inputs
    //                AN0, AN1, AN2, AN3, AN4, AN5, AN20, AN21
    // AC_MODEL up to 6 Servo Potentiometers
    //                AN1, AN2, AN3, AN5, AN20, AN21
    // AEROCOMP up to 4 Servo Potentiometers
    //                AN2, AN3, AN20, AN21
    // Battery up to 3 cells
    //                AN12, AN13, AN14
    //
    // AN0        ->        ANSELBbits.ANSB0
    // AN1        ->        ANSELBbits.ANSB1
    // AN2        ->        ANSELBbits.ANSB2
    // AN3        ->        ANSELBbits.ANSB3
    // AN4        ->        ANSELBbits.ANSB4
    // AN5        ->        ANSELBbits.ANSB5
    // AN12        ->        ANSELBbits.ANSB12
    // AN13        ->        ANSELBbits.ANSB13
    // AN14        ->        ANSELBbits.ANSB14
    // AN20        ->        ANSELEbits.ANSE8
    // AN21        ->        ANSELEbits.ANSE9

    /* 1) Enable and configure analog inputs */
#if GNDBOARD
    _TRISB4 = 1; // AN4/RB4 is enabled as input
    _ANSB4 = 1; // AN4/RB4 is analog
    _TRISB5 = 1; // AN5/RB5 is enabled as input
    _ANSB5 = 1; // AN5/RB5 is analog
    _TRISE9 = 1; // AN21/RE9 is enabled as input
    _ANSE9 = 1; // AN21/RE9 is analog
    _TRISE8 = 1; // AN20/RE8 is enabled as input
    _ANSE8 = 1; // AN20/RE8 is analog
#else
#if AC_MODEL
    TRISBbits.TRISB1 = 1; // AN1/RB1 is enabled as input
    ANSELBbits.ANSB1 = 1; // AN1/RB1 is analog
    TRISBbits.TRISB5 = 1; // AN5/RB5 is enabled as input
    ANSELBbits.ANSB5 = 1; // AN5/RB5 is analog
#endif
    TRISBbits.TRISB2 = 1; // AN2/RB2 is enabled as input
    ANSELBbits.ANSB2 = 1; // AN2/RB2 is analog
    TRISBbits.TRISB3 = 1; // AN3/RB3 is enabled as input
    ANSELBbits.ANSB3 = 1; // AN3/RB3 is analog
    TRISBbits.TRISB12 = 1; // AN12/RB12 is enabled as input
    ANSELBbits.ANSB12 = 1; // AN12/RB12 is analog
    TRISBbits.TRISB13 = 1; // AN13/RB13 is enabled as input
    ANSELBbits.ANSB13 = 1; // AN13/RB13 is analog
    TRISBbits.TRISB14 = 1; // AN14/RB14 is enabled as input
    ANSELBbits.ANSB14 = 1; // AN14/RB14 is analog
    TRISEbits.TRISE8 = 1; // AN20/RE8 is enabled as input
    ANSELEbits.ANSE8 = 1; // AN20/RE8 is analog
    TRISEbits.TRISE9 = 1; // AN21/RE9 is enabled as input
    ANSELEbits.ANSE9 = 1; // AN21/RE9 is analog
#endif
    AD1CON1 = AD1CON1_CFG;
    AD1CON2 = AD1CON2_CFG;
    AD1CON3 = AD1CON3_CFG;
    AD1CON4 = AD1CON4_CFG;
    /* AD1CHS123bits: ADC1 Input Channel 1,2,3 Select Register.*/
    AD1CHS123bits.CH123NB = 0b00; /* Channel 1,2,3 Negative Input Select for Sample B bits:
                                     When AD12B = 1, CHxNB is Unimplemented, Read as 0. */
    AD1CHS123bits.CH123SB = 0b00; /* Channel 1,2,3 Positive Input Select for Sample B bit:
                                     When AD12B = 1, CHxSB is Unimplemented, Read as 0. */
    AD1CHS123bits.CH123NA = 0b00; /* Channel 1,2,3 Negative Input Select for Sample A bits:
                                     When AD12B = 1, CHxNA is Unimplemented, Read as 0. */
    AD1CHS123bits.CH123SA = 0b00; /* Channel 1,2,3 Positive Input Select for Sample A bit:
                                     When AD12B = 1, CHxSA is Unimplemented, Read as 0. */
    /* AD1CHS0: ADC1 Input Channel 0 Select Register. */
    AD1CHS0bits.CH0NB = 0b0; /* Channel 0 Negative Input Select for Sample B bit:
                                     0 = Channel 0 negative input is VREFL. */
    AD1CHS0bits.CH0NA = 0b0; /* Channel 0 Negative Input Select for Sample A bit
                                     0 = Channel 0 negative input is VREFL. */
    /* Analog inputs selection */
    /* AC_MODEL & AEROCOMP Common analog inputs*/
    AD1CSSH = 0x0030; /* ADC1 Input Scan Select Register High:
                         0x0030 = Scans AN20, AN21. */
    /* GNDBOARD, AC_MODEL & AEROCOMP Specific analog inputs*/
#if GNDBOARD
    AD1CSSL = 0x0030; /* ADC1 Input Scan Select Register Low:
                      0x0030 = Scans AN4, AN5. */
#elif AC_MODEL
    AD1CSSL = 0x702E; /* ADC1 Input Scan Select Register Low:
                      0x702E = Scans AN1, AN2, AN3, AN5, AN12, AN13, AN14. */
#endif

    /* 12) Select the interrupt rate or DMA buffer pointer increment rate (ADxCON2<6:2>) */
#if GNDBOARD
    AD1CON2bits.SMPI = 4-1; /* Sample and Conversion Operation bits
                                                              0b01000 = ADC interrupt is generated at the completion of every
                                                              8th sample/conversion operation. */
#elif AC_MODEL
    AD1CON2bits.SMPI = 9-1; /* Sample and Conversion Operation bits
                                                              0b01001 = ADC interrupt is generated at the completion of every
                                                               9th sample/conversion operation. */
#elif AEROCOMP
    AD1CON2bits.SMPI = 7-1; /* 0b00100 = ADC interrupt is generated at the completion of every
                                                               7th sample/conversion operation. */
#endif

    /* Disable Interupt*/
    _AD1IE = 0;
}

void ADC1Start(void) {
    /* 1) Turn on the ADC module (ADxCON1<15>) */
    AD1CON1bits.ADON = 0b01; // ADC Operating Mode bit: 1 = ADC module is operating.
}

#endif /*USE_ADC1*/
