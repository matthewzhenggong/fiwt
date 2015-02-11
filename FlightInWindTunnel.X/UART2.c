/*
 * File:   UART2.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Original author: Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
 * Some code may also be from Microchip's DataSheet Documents.
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
#include "UART2.h"
/* TODO include for test only */
#if STARTKITBOARD
#include "StartKit.h"
#endif

#include <string.h>
#include <xc.h>
#include <libpic30.h>

/** Compute baudrate value and store in BR_Value */
#define BR_Value ((FCY/UART2BAUdRATE)/16)-1


/** Allocate two buffers for DMA1 transfers */
#ifdef __HAS_DMA__
__eds__ uint8_t bufferUART2TXA[UART2TXBUFFLEN] __attribute__((eds, space(dma)));
__eds__ uint8_t bufferUART2TXB[UART2TXBUFFLEN] __attribute__((eds, space(dma)));
#else
uint8_t bufferUART2TXA[UART2TXBUFFLEN] __attribute__((space(xmemory)));
uint8_t bufferUART2TXB[UART2TXBUFFLEN] __attribute__((space(xmemory)));
#endif

__near volatile uint16_t DMA1Sending;
__near volatile uint16_t DMA1Cnt;

#ifdef __HAS_DMA__
__eds__ uint8_t UART2RXBUFF[UART2RXBUFFLEN] __attribute__((eds, space(dma)));
#else
uint8_t UART2RXBUFF[UART2RXBUFFLEN] __attribute__((space(xmemory)));
#endif
__near volatile unsigned int UART2RXBUFF_tail;
__near unsigned int DMA5START;
__near unsigned int DMA5END;

void UART2Init(void) {

    /* 1) Configure U2TX as output, and U2RX as input */
#if GNDBOARD        /* Only for GNDBOARD */
    _TRISF0 = 0b0;
    _TRISF1 = 0b1;
#else                        /* Only for AC_MODEL and AEROCOMP boards */
    _TRISG0 = 0b1;
    _TRISG1 = 0b0;
#endif

    /* 2) Assign UART2 pins through Peripherial Pin Select */
#if GNDBOARD        /* Only for GNDBOARD */
    _U2RXR = 0x61;        /* Input tied to RP97,                0x61 ->   97 U2RX */
    _RP96R = 0x03;        /* U2TX is assigned to RP96,        0x03 -> U2TX      */
#else /* Only for AC_MODEL and AEROCOMP boards */
    _U2RXR = 0x70;        /* U2RX        Input tied to RP112,        0x70 ->  112 */
    _RP113R = 0x03;        /* U2TX is assigned to RP113,        0x03 -> U2TX */
#endif

    /* 3) Initialize the U2BRG register for the appropiate baud rate */
    U2BRG = BR_Value;

    /* 4) Configure UART2 module: Set number of data bits, number of Stop bits, and parity bits */
    U2MODEbits.UARTEN = 0; /*  UARTEN: UART2 Enable bit. */
    U2STAbits.UTXEN = 0; /*  The UTXEN bit is set after the UARTEN bit has been set */
    U2MODEbits.USIDL = 0b0; /*  USIDL: Stop in Idle Mode bit.
                                1 = Discontinue module operation when device enters Idle mode.
                                0 = Continue module operation in Idle mode. */
    U2MODEbits.IREN = 0b0; /*  IREN: IrDA?Encoder and Decoder Enable bit(2).
                               1 = IrDA encoder and decoder enabled.
                               0 = IrDA encoder and decoder disabled. */
    U2MODEbits.RTSMD = 0b0; /*  RTSMD: Mode Selection for U2RTS Pin bit.
                                1 = U2RTS pin in Simplex mode.
                                0 = U2RTS pin in Flow Control mode. */
    U2MODEbits.UEN = 0b00; /*  UEN<1:0>: UART2 Pin Enable bits.
                               11 = U2TX, U2RX and BCLK pins are enabled and used; U2CTS pin
                               controlled by PORT latches.
                               10 = U2TX, U2RX, U2CTS and U2RTS pins are enabled and used.
                               01 = U2TX, U2RX and U2RTS pins are enabled and used; U2CTS pin
                               controlled by PORT latches.
                               00 = U2TX and U2RX pins are enabled and used; U2CTS and U2RTS/BCLK pins
                               controlled by PORT latches. */
    U2MODEbits.WAKE = 0b1; /*  WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit.
                               1 = UART2 continues to sample the U2RX pin; interrupt generated on
                               falling edge; bit cleared in hardware on following rising edge.
                               0 = No wake-up enabled. */
    U2MODEbits.LPBACK = 0b0; /*  LPBACK: UART2 Loopback Mode Select bit.
                               1 = Enable Loopback mode.
                               0 = Loopback mode is disabled. */
    U2MODEbits.ABAUD = 0b0; /*  ABAUD: Auto-Baud Enable bit.
                                1 = Enable baud rate measurement on the next character ?requires
                                reception of a Sync field (55h) before other data; cleared in hardware
                                upon completion.
                                0 = Baud rate measurement disabled or completed. */
    U2MODEbits.URXINV = 0b0; /*  URXINV: Receive Polarity Inversion bit.
                                 1 = U2RX Idle state is ??
                                 0 = U2RX Idle state is ?? */
    U2MODEbits.BRGH = 0b0; /*  BRGH: High Baud Rate Enable bit.
                               1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed
                               mode).
                               0 = BRG generates 16 clocks per bit period (16x baud clock, Standard
                               mode). */
    U2MODEbits.PDSEL = 0b00; /*  PDSEL<1:0>: Parity and Data Selection bits.
                                 11 = 9-bit data, no parity.
                                 10 = 8-bit data, odd parity.
                                 01 = 8-bit data, even parity.
                                 00 = 8-bit data, no parity. */
    U2MODEbits.STSEL = 0b0; /*  STSEL: Stop Bit Selection bit.
                                1 = Two Stop bits.
                                0 = One Stop bit. */

    /* 4. Configure Transmit & Receive Interrupt for UART2 */
    /*  U2STA: UART2 STATUS AND CONTROL REGISTER */
    U2STAbits.UTXISEL1 = 0b0; /*  UTXISEL<1:0>: Transmission Interrupt Mode Selection bits. */
    U2STAbits.UTXISEL0 = 0b0; /*  11 = Reserved; do not use.
                                  10 = Interrupt when a character is transferred to the Transmit Shift
                                  Register, and as a result, the transmit buffer becomes empty.
                                  01 = Interrupt when the last character is shifted out of the Transmit
                                  Shift Register; all transmit operations are completed.
                                  00 = Interrupt when a character is transferred to the Transmit Shift
                                  Register (this implies there is at least one character open in the */
    /*  transmit buffer). */
    U2STAbits.UTXINV = 0b0; /*  UTXINV: Transmit Polarity Inversion bit.
                                If IREN = 0:
                                1 = U2TX Idle state is ??
                                0 = U2TX Idle state is ??
                                If IREN = 1:
                                1 = IrDA encoded U2TX Idle state is ??
                                0 = IrDA encoded U2TX Idle state is ?? */
    U2STAbits.UTXBRK = 0b0; /*  UTXBRK: Transmit Break bit.
                                1 = Send Sync Break on next transmission ?Start bit, followed by
                                twelve ??bits, followed by Stop bit; cleared by hardware upon completion.
                                0 = Sync Break transmission disabled or completed. */
    U2STAbits.URXISEL = 0b00; /*  URXISEL<1:0>: Receive Interrupt Mode Selection bits.
                                  11 = Interrupt is set on U2RSR transfer making the receive buffer full
                                  (i.e., has 4 data characters).
                                  10 = Interrupt is set on U2RSR transfer making the receive buffer 3/4
                                  full (i.e., has 3 data characters).
                                  0x = Interrupt is set when any character is received and transferred
                                  from the U2RSR to the receive buffer. Receive buffer has one or more
                                  characters. */
    U2STAbits.ADDEN = 0b0; /*  ADDEN: Address Character Detect bit (bit 8 of received data = 1).
                               1 = Address Detect mode enabled. If 9-bit mode is not selected, this
                               does not take effect.
                               0 = Address Detect mode disabled. */

    /* Priorities of UART2 Interrupts */
    _U2RXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART2 Receiver priority out 5 of 7 */
    _U2TXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART2 Trans priority out 5 of 7 */
    _U2EIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW;

    /**   Associate DMA Channel 1 with UART Tx */
    DMA1REQ = 0b00011111; /*  Select UART2 Transmitter */
    DMA1PAD = (volatile uint16_t) & U2TXREG;

    /*   Configure DMA Channel 0 to: */
    /*   Transfer data from RAM to UART */
    /*   One-Shot mode */
    /*   Register Indirect with Post-Increment */
    /*   Using single buffer */
    /*   8 transfers per buffer */
    DMA1CONbits.CHEN = 0;
    DMA1CONbits.AMODE = 0;
    DMA1CONbits.MODE = 1;
    DMA1CONbits.DIR = 1;
    DMA1CONbits.SIZE = 1;
    DMA1CNT = 0;

    /**   Associate DMA Channel 4 with UART Rx */
    DMA5REQ = 0b00011110; /*  Select UART2 RX */
    DMA5PAD = (volatile uint16_t) & U2RXREG;

    /*   Configure DMA Channel 4 to: */
    /*   Transfer data from RAM to UART */
    /*   Continues mode */
    /*   Register Indirect with Post-Increment */
    /*   Using single buffer */
    /*   8 transfers per buffer */
    DMA5CONbits.CHEN = 0;
    DMA5CONbits.AMODE = 0;
    DMA5CONbits.MODE = 0;
    DMA5CONbits.DIR = 0;
    DMA5CONbits.SIZE = 1;
    DMA5CNT = UART2RXBUFFLEN-1u;

    /* Priorities of DMA Interrupts */
    _DMA1IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMA1 priority out 5 of 7 */
    _DMA5IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMA5 priority out 5 of 7 */
}

void UART2Start(void) {
    /* 1. Enable the UART2 module (U2MODE<15>) */
    U2MODEbits.UARTEN = 1; /*  UARTEN: UART2 Enable bit.
                               1 = UART2 is enabled; all UART2 pins are controlled by UART2 as defined
                               by UEN<1:0>.
                               0 = UART2 is disabled; all UART2 pins are controlled by PORT latches;
                               UART2 power consumption minimal. */

    /* 2. Enable the UART2 module transmission */
    U2STAbits.UTXEN = 1; /*  The UTXEN bit is set after the UARTEN bit has been set; otherwise, UART
                             transmissions will not be enabled.
                             UTXEN: Transmit Enable bit(1).
                             1 = Transmit enabled, U2TX pin controlled by UART2.
                             0 = Transmit disabled, any pending transmission is aborted and buffer
                             is reset. U2TX pin controlled by port. */

    /* Enable UART2 Interrupts */
    _U2RXIF = 0; /*  UART2 Receiver Interrupt flag cleared */
    _U2RXIE = 0; /*  UART2 Receiver Interrupt enabled */
    _U2TXIF = 0; /*  UART2 Receiver Interrupt flag cleared */
    _U2TXIE = 0; /*  UART2 Receiver Interrupt disabled */
    _U2EIF = 0; /*  UART2 Receiver Interrupt flag cleared */
    _U2EIE = 0; /*  UART2 Receiver Interrupt enabled */

    /*   Enable DMA Interrupts */
    _DMA1IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA1IE = 1; /*  Enable DMA interrupt */

    DMA5STAL = UART2RXBUFF_tail = DMA5START =__builtin_dmaoffset(UART2RXBUFF);
    DMA5END = DMA5START+UART2RXBUFFLEN;
    DMA5STAH = __builtin_dmapage(UART2RXBUFF);

    _DMA5IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA5IE = 0; /*  Enable DMA interrupt */
    DMA5CONbits.CHEN = 1; /* Enable DMA channel */

    DMA1Sending = 0;
    DMA1Cnt = 0;
}

/** Setup DMA interrupt handlers */
__interrupt(no_auto_psv) void _DMA1Interrupt(void) {
#ifdef STARTKITBOARD
    mLED_2_Toggle();
#endif
    DMA1Sending &= 0xFFFDu;
    _DMA1IF = 0; /*  Clear the DMA1 Interrupt Flag; */
}


/**
 * Send all data in output buffer
 *
 * @note swap double-bufferes and send with DMA1
 */
void UART2Flush(void) {

    if (DMA1Cnt == 0)
        return;

    while (DMA1Sending & 0b10) {
        /* wait */
    }

    /*  buff swap */

    DMA1CNT = DMA1Cnt - 1u; /*  N+1 DMA requests */
    DMA1Sending ^= 1u;
    DMA1Cnt = 0;

    DMA1STAH = 0x0000u;
    if (DMA1Sending & 1u) {
#ifdef __HAS_DMA__
        DMA1STAL = __builtin_dmaoffset(bufferUART2TXA);
#else
        DMA1STAL = (volatile uint16_t) & bufferUART2TXA;
#endif
    } else {
#ifdef __HAS_DMA__
        DMA1STAL = __builtin_dmaoffset(bufferUART2TXB);
#else
        DMA1STAL = (volatile uint16_t) & bufferUART2TXB;
#endif
    }
    DMA1CONbits.CHEN = 1; /*  Re-enable DMA1 Channel */
    DMA1REQbits.FORCE = 1; /*  Manual mode: Kick-start the first transfer */
    DMA1Sending |= 0b10;
}

void UART2SendByte(uint8_t inbyte) {
    if (DMA1Sending & 0b01) {
        bufferUART2TXB[DMA1Cnt++] = inbyte;
    } else {
        bufferUART2TXA[DMA1Cnt++] = inbyte;
    }
    if (DMA1Cnt >= UART2TXBUFFLEN) {
        UART2Flush();
    }
}


bool UART2GetAvailable(void) {
    if (DMA5STAL == UART2RXBUFF_tail) {
        return false;
    }
    return true;
}

uint8_t UART2GetByte(void) {
    uint8_t c;
    c = UART2RXBUFF[UART2RXBUFF_tail-DMA5START];
    if (++UART2RXBUFF_tail >= DMA5END)
        UART2RXBUFF_tail = DMA5START;
    return  c;
}

size_t UART2GetBytes(uint8_t *output, size_t n) {
    size_t cnt;
    cnt = 0u;
    while (cnt < n && DMA5STAL != UART2RXBUFF_tail) {
        output [cnt++] = UART2RXBUFF[UART2RXBUFF_tail-DMA5START];
        if (++UART2RXBUFF_tail >= DMA5END)
            UART2RXBUFF_tail = DMA5START;
    }
    return cnt;
}

void UART2SendBytes(const uint8_t *input, size_t n) {
    #ifdef __HAS_DMA__
    __eds__ uint8_t *buffer;
    #else
    volatile uint8_t *buffer;
    #endif
    size_t n1;

    while (n+DMA1Cnt >= UART2TXBUFFLEN) {
        n1 = UART2TXBUFFLEN-DMA1Cnt;
        if (DMA1Sending & 0b01) {
            buffer = bufferUART2TXB;
        } else {
            buffer = bufferUART2TXA;
        }
#ifdef __HAS_DMA__
        _memcpy_eds(buffer, (__eds__ void *)input, n1);
#else
        memcpy(buffer, input, n1);
#endif
        DMA1Cnt += n1;
        UART2Flush();
        n -= n1;
        input += n1;
    }
    if (n > 0) {
        if (DMA1Sending & 0b01) {
            buffer = bufferUART2TXB;
        } else {
            buffer = bufferUART2TXA;
        }
#ifdef __HAS_DMA__
        _memcpy_eds(buffer, (__eds__ void *)input, n);
#else
        memcpy(buffer, input, n);
#endif
        DMA1Cnt += n;
    }
}

void UART2SendString(const char input[]) {
    size_t n;
    n = strlen(input);
    UART2SendBytes((const uint8_t *)input, n);
}

