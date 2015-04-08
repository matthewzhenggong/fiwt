/*
 * File:   config_adc.h
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

#ifndef CONFIG_ADC_H
#define	CONFIG_ADC_H

/** For ADC1 */

//    /* 0) Turn off the ADC module (ADxCON1<15>) */
//    AD1CON1bits.ADON = 0b00; // ADC Operating Mode bit: 1 = ADC module is operating.
//
//    /* 2) Select 10-bit or 12-bit mode (ADxCON1<10>) */
//    AD1CON1bits.AD12B = 0b1; /* 10-bit or 12-bit Operation Mode bit: 1 = 12-bit */
//
//    /* 3) Select the voltage reference source to match the expected range on analog inputs
//     (ADxCON2<15:13>) */
//    AD1CON2bits.VCFG = 0b000; /* Converter Voltage Reference Configuration bits:
//                                 000 = VREFH -> AVDD, VREFL -> AVSS. */
//
//    /* 4) Select the analog conversion clock to match the desired data rate with processor clock
//     (ADxCON3<7:0>) */
//    AD1CON3bits.ADRC = 0b0; /* ADC Conversion Clock Source bit: 0 = Clock Derived from
//                               System Clock. */
//    /* ADC Conversion Clock Select bits. Minimum TAD = 117.6 ns. */
//    AD1CON3bits.ADCS = 80; /* At 64 MIPS, TCY = 15.6 ns.
//                                With ADCS = 80 = 80*TCY = 1248 ns > minimumm TAD.
//                             * T = (31+14)*CSS*ADCS*TCY= 505.44 us
//                             * Freq = 2kHz */
//
//    /* 5) Determine how inputs will be allocated to S&H channels (ADxCHS0<15:0> and ADxCHS123<15:0>) */
//    /* 6) Determine how many S&H channels will be used (ADxCON2<9:8>) */
//    AD1CON2bits.CHPS = 0b00; /* Channel Select bits: 00 = Converts CH0.
//                                     When AD12B = 0b1, CHPS is unimplemented. */
//    /* 7) Determine how sampling will occur (ADxCON1<3>, ADxCSSH<15:0> and ADxCSSL<15:0>) */
//    AD1CON1bits.SIMSAM = 0b00; /* Simultaneous Sample Select bit: 0 = Samples multiple channels
//                                  individually in sequence.
//                                  When AD12B = 0b1, SIMSAM is unimplemented. */
//    AD1CON2bits.CSCNA = 0b01; /* Input Scan Select bit: 1 = Scan Inputs for CH0+ during
//                                 Sample A bit. */
//    AD1CON2bits.BUFM = 0b00; /* Buffer Fill Mode Select bit: 00 = Always starts filling the
//                                buffer from the start address. */
//    AD1CON2bits.ALTS = 0b00; /* Alternate Input Sample Mode Select bit: 00 = Always uses
//                                channel input selects for Sample A. */
//    /* 8) Select Manual or Auto Sampling */
//    AD1CON1bits.ASAM = 0b01; /* ADC Sample Auto-Start bit:
//                                                                  0 = Sampling begins when SAMP bit is set. Manual Sampling.
//                                                                  1 = Sampling begins immediately after last conversion. SAMP bit
//                                                                  is auto-set. Auto Sampling. */
//
//    /* 9) Select the conversion trigger and sampling time */
//    AD1CON1bits.SSRC = 0b111; /* Sample Clock Source Select bits: 111 = Internal counter ends
//                                                           sampling and starts conversion (auto-convert). */
//    AD1CON1bits.SSRCG = 0b00; /* Sample Clock Source Group bit: 0 = Group 2. */
//    AD1CON3bits.SAMC = 31; /* Auto Sample Time bits:  max TAD. */
//
//    /* 10) Select how the data format for the conversion results are stored in the buffer
//     (ADxCON1<9:8>) */
//    AD1CON1bits.FORM = 0b00; /* Data Output Format bits: 00 = Integer. */
//
//    /* 11) Set the ADDMAEN bit to configure the ADC module to use DMA */
//    AD1CON4bits.ADDMAEN = 0b0; /* ADC DMA Enable bit:
//                                                           0b1 = ADC DMA enabled.
//                                                           0b0 = ADC DMA disabled. */
//
//    /* 13) Select the number of samples in DMA buffer for each ADC module input (ADxCON4<2:0>) */
//    AD1CON4bits.DMABL = 0b000; /* Selects Number of DMA Buffer Location per Analog Input bits:
//                                                            000 = Allocates 1 words of buffer to each analog input. */
//
//    /* 14) Configure the DMA channel (if needed) */
//    AD1CON1bits.ADDMABM = 0b00; /* DMA Buffer Build Mode bit: 0 = DMA buffers are written in the
//                                                              order of conversion. */
//
//    /* 18) Additional configuration parameters */
//    AD1CON1bits.ADSIDL = 0b00; /* Stop in Idle Mode bit: 0 = Continue module operation when
//                                                              device enters Idle mode. */
#define AD1CON1_CFG 0x04E4u
#define AD1CON2_CFG 0x0400u
#define AD1CON3_CFG 0x1f50u
#define AD1CON4_CFG 0x0000u

#endif	/* CONFIG_ADC_H */

