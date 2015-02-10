/*
 * File:   UART1.c
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
#include "UART1.h"
/* TODO include for test only */
#if STARTKITBOARD
#include "StartKit.h"
#endif

#include <string.h>
#include <xc.h>
#include <libpic30.h>


/** Compute baudrate value and store in BR_Value */
#define BR_Value ((FCY/UART1BAUdRATE)/16)-1


/** Allocate two buffers for DMA0 transfers */
#ifdef __HAS_DMA__
__eds__ uint8_t bufferUART1TXA[UART1TXBUFFLEN] __attribute__((eds, space(dma)));
__eds__ uint8_t bufferUART1TXB[UART1TXBUFFLEN] __attribute__((eds, space(dma)));
#else
volatile uint8_t bufferUART1TXA[UART1TXBUFFLEN] __attribute__((space(xmemory)));
volatile uint8_t bufferUART1TXB[UART1TXBUFFLEN] __attribute__((space(xmemory)));
#endif

__near volatile uint16_t DMA0Sending;
__near volatile uint16_t DMA0Cnt;

__near uint8_t UART1RXBUFF[UART1RXBUFFLEN];
__near volatile uint16_t UART1RXBUFF_head;
__near volatile uint16_t UART1RXBUFF_tail;

void UART1Init(void) {
    /* 1) Configure U1TX as output, and U1RX as input */
#if GNDBOARD        /*  Only for GNDBOARD */
    _TRISG0 = 0b1;
    _TRISG1 = 0b0;
#else                        /*  Only for AC_MODEL and AEROCOMP boards */
    _TRISF0 = 0b0;
    _TRISF1 = 0b1;
#endif

    /* 2) Assign UART1 pins through Peripherial Pin Select */
#if GNDBOARD        /*  Only for GNDBOARD */
    _U1RXR = 0x70; /*  U1RX        Input tied to RP112,   0x70 ->  112 */
    _RP113R = 0x01; /*  U1TX is assigned to RP113,        0x01 -> U1TX */
#else              /*  Only for AC_MODEL and AEROCOMP boards */
    _U1RXR = 0x61; /*  U1RX        Input tied to RP97,  0x61 ->   97 */
    _RP96R = 0x01; /*  U1TX is assigned to RP96,        0x01 -> U1TX */
#endif

    /* 3) Initialize the U1BRG register for the appropiate baud rate */
    U1BRG = BR_Value;

    /* 4) Configure UART1 module: Set number of data bits, number of Stop bits, and parity bits */
    /*  U1MODE: UART1 MODE REGISTER */
    U1MODEbits.USIDL = 0b0; /*  USIDL: Stop in Idle Mode bit.
                                1 = Discontinue module operation when device enters Idle mode.
                                0 = Continue module operation in Idle mode. */
    U1MODEbits.IREN = 0b0; /*  IREN: IrDA?Encoder and Decoder Enable bit(2).
                               1 = IrDA encoder and decoder enabled.
                               0 = IrDA encoder and decoder disabled. */
    U1MODEbits.RTSMD = 0b0; /*  RTSMD: Mode Selection for U1RTS Pin bit.
                                1 = U1RTS pin in Simplex mode.
                                0 = U1RTS pin in Flow Control mode. */
    U1MODEbits.UEN = 0b00; /*  UEN<1:0>: UART1 Pin Enable bits.
                               11 = U1TX, U1RX and BCLK pins are enabled and used; U1CTS pin
                               controlled by PORT latches.
                               10 = U1TX, U1RX, U1CTS and U1RTS pins are enabled and used.
                               01 = U1TX, U1RX and U1RTS pins are enabled and used; U1CTS pin
                               controlled by PORT latches.
                               00 = U1TX and U1RX pins are enabled and used; U1CTS and U1RTS/BCLK pins
                               controlled by PORT latches. */
    U1MODEbits.WAKE = 0b1; /*  WAKE: Wake-up on Start bit Detect During Sleep Mode Enable bit.
                               1 = UART1 continues to sample the U1RX pin; interrupt generated on
                               falling edge; bit cleared in hardware on following rising edge.
                               0 = No wake-up enabled. */
    U1MODEbits.LPBACK = 0b0; /*  LPBACK: UART1 Loopback Mode Select bit.
                               1 = Enable Loopback mode.
                               0 = Loopback mode is disabled. */
    U1MODEbits.ABAUD = 0b0; /*  ABAUD: Auto-Baud Enable bit.
                                1 = Enable baud rate measurement on the next character ?requires
                                reception of a Sync field (55h) before other data; cleared in hardware
                                upon completion.
                                0 = Baud rate measurement disabled or completed. */
    U1MODEbits.URXINV = 0b0; /*  URXINV: Receive Polarity Inversion bit.
                                 1 = U1RX Idle state is ??
                                 0 = U1RX Idle state is ?? */
    U1MODEbits.BRGH = 0b0; /*  BRGH: High Baud Rate Enable bit.
                               1 = BRG generates 4 clocks per bit period (4x baud clock, High-Speed
                               mode).
                               0 = BRG generates 16 clocks per bit period (16x baud clock, Standard
                               mode). */
    U1MODEbits.PDSEL = 0b00; /*  PDSEL<1:0>: Parity and Data Selection bits.
                                 11 = 9-bit data, no parity.
                                 10 = 8-bit data, odd parity.
                                 01 = 8-bit data, even parity.
                                 00 = 8-bit data, no parity. */
    U1MODEbits.STSEL = 0b0; /*  STSEL: Stop Bit Selection bit.
                                1 = Two Stop bits.
                                0 = One Stop bit. */

    /* 4. Configure Transmit & Receive Interrupt for UART1 */
    /*  U1STA: UART1 STATUS AND CONTROL REGISTER */
    U1STAbits.UTXISEL1 = 0b0; /*  UTXISEL<1:0>: Transmission Interrupt Mode Selection bits. */
    U1STAbits.UTXISEL0 = 0b0; /*  11 = Reserved; do not use.
                                  10 = Interrupt when a character is transferred to the Transmit Shift
                                  Register, and as a result, the transmit buffer becomes empty.
                                  01 = Interrupt when the last character is shifted out of the Transmit
                                  Shift Register; all transmit operations are completed.
                                  00 = Interrupt when a character is transferred to the Transmit Shift
                                  Register (this implies there is at least one character open in the */
    /*  transmit buffer). */
    U1STAbits.UTXINV = 0b0; /*  UTXINV: Transmit Polarity Inversion bit.
                                If IREN = 0:
                                1 = U1TX Idle state is ??
                                0 = U1TX Idle state is ??
                                If IREN = 1:
                                1 = IrDA encoded U1TX Idle state is ??
                                0 = IrDA encoded U1TX Idle state is ?? */
    U1STAbits.UTXBRK = 0b0; /*  UTXBRK: Transmit Break bit.
                                1 = Send Sync Break on next transmission ?Start bit, followed by
                                twelve ??bits, followed by Stop bit; cleared by hardware upon completion.
                                0 = Sync Break transmission disabled or completed. */
    U1STAbits.URXISEL = 0b00; /*  URXISEL<1:0>: Receive Interrupt Mode Selection bits.
                                  11 = Interrupt is set on U1RSR transfer making the receive buffer full
                                  (i.e., has 4 data characters).
                                  10 = Interrupt is set on U1RSR transfer making the receive buffer 3/4
                                  full (i.e., has 3 data characters).
                                  0x = Interrupt is set when any character is received and transferred
                                  from the U1RSR to the receive buffer. Receive buffer has one or more
                                  characters. */
    U1STAbits.ADDEN = 0b0; /*  ADDEN: Address Character Detect bit (bit 8 of received data = 1).
                               1 = Address Detect mode enabled. If 9-bit mode is not selected, this
                               does not take effect.
                               0 = Address Detect mode disabled. */

    /* Priorities of UART1 Interrupts */
    _U1RXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART1 Receiver priority out 5 of 7 */
    _U1TXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART1 Trans priority out 5 of 7 */
    _U1EIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW;

    /**   Associate DMA Channel 0 with UART Tx */
    DMA0REQ = 0b00001100; /*  Select UART1 Transmitter */
    DMA0PAD = (volatile uint16_t) & U1TXREG;

    /*   Configure DMA Channel 0 to: */
    /*   Transfer data from RAM to UART */
    /*   One-Shot mode */
    /*   Register Indirect with Post-Increment */
    /*   Using single buffer */
    /*   8 transfers per buffer */
    DMA0CONbits.AMODE = 0;
    DMA0CONbits.MODE = 1;
    DMA0CONbits.DIR = 1;
    DMA0CONbits.SIZE = 1;
    DMA0CNT = 0;

    /* Priorities of DMA Interrupts */
    _DMA0IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMA0 priority out 5 of 7 */
}

void UART1Start(void) {
    /* 1. Enable the UART1 module (U1MODE<15>) */
    U1MODEbits.UARTEN = 1; /*  UARTEN: UART1 Enable bit.
                               1 = UART1 is enabled; all UART1 pins are controlled by UART1 as defined
                               by UEN<1:0>.
                               0 = UART1 is disabled; all UART1 pins are controlled by PORT latches;
                               UART1 power consumption minimal. */

    /* 2. Enable the UART1 module transmission */
    U1STAbits.UTXEN = 1; /*  The UTXEN bit is set after the UARTEN bit has been set; otherwise, UART
                             transmissions will not be enabled.
                             UTXEN: Transmit Enable bit(1).
                             1 = Transmit enabled, U1TX pin controlled by UART1.
                             0 = Transmit disabled, any pending transmission is aborted and buffer
                             is reset. U1TX pin controlled by port. */

    /* Enable UART1 Interrupts */
    _U1RXIF = 0; /*  UART1 Receiver Interrupt flag cleared */
    _U1RXIE = 1; /*  UART1 Receiver Interrupt enabled */
    _U1TXIF = 0; /*  UART1 Receiver Interrupt flag cleared */
    _U1TXIE = 0; /*  UART1 Receiver Interrupt disabled */
    _U1EIF = 0; /*  UART1 Receiver Interrupt flag cleared */
    _U1EIE = 1; /*  UART1 Receiver Interrupt enabled */

    /*   Enable DMA Interrupts */
    _DMA0IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA0IE = 1; /*  Enable DMA interrupt */

    DMA0Sending = 0;
    DMA0Cnt = 0;
    UART1RXBUFF_head = UART1RXBUFF_tail = 0;
}

/** Setup DMA interrupt handlers */
__interrupt(no_auto_psv) void _DMA0Interrupt(void) {
#ifdef STARTKITBOARD
    mLED_3_Toggle();
#endif
    DMA0Sending &= 0xFFFDu;
    _DMA0IF = 0; /*  Clear the DMA0 Interrupt Flag; */
}

void __attribute__((__interrupt__, no_auto_psv, shadow)) _U1ErrInterrupt(void) {
    /*  An error has occurred on the last reception. Check the last received word. */
    _U1EIF = 0;
}

/**
 * Send all data in output buffer
 *
 * @note swap double-bufferes and send with DMA0
 */
void UART1Flush(void) {

    if (DMA0Cnt == 0)
        return;

    while (DMA0Sending & 0b10) {
        /* wait */
    }

    /*  buff swap */

    DMA0CNT = DMA0Cnt - 1u; /*  N+1 DMA requests */
    DMA0Sending ^= 1u;
    DMA0Cnt = 0;

    if (DMA0Sending & 1u) {
#ifdef __HAS_DMA__
        DMA0STAL = __builtin_dmaoffset(bufferUART1TXA);
        DMA0STAH = __builtin_dmapage(bufferUART1TXA);
#else
        DMA0STAL = (volatile uint16_t) & bufferUART1TXA;
        DMA0STAH = (volatile uint16_t) & bufferUART1TXA;
#endif
    } else {
#ifdef __HAS_DMA__
        DMA0STAL = __builtin_dmaoffset(bufferUART1TXB);
        DMA0STAH = __builtin_dmapage(bufferUART1TXB);
#else
        DMA0STAL = (volatile uint16_t) & bufferUART1TXB;
        DMA0STAH = (volatile uint16_t) & bufferUART1TXB;
#endif
    }
    DMA0CONbits.CHEN = 1; /*  Re-enable DMA0 Channel */
    DMA0REQbits.FORCE = 1; /*  Manual mode: Kick-start the first transfer */
    DMA0Sending |= 0b10;
}

void UART1SendByte(uint8_t inbyte) {
    if (DMA0Sending & 0b01) {
        bufferUART1TXB[DMA0Cnt++] = inbyte;
    } else {
        bufferUART1TXA[DMA0Cnt++] = inbyte;
    }
    if (DMA0Cnt >= UART1TXBUFFLEN) {
        UART1Flush();
    }
}

__interrupt(no_auto_psv) void _U1RXInterrupt(void) {
    if (++UART1RXBUFF_head >= UART1RXBUFFLEN)
        UART1RXBUFF_head = 0;
    UART1RXBUFF[UART1RXBUFF_head] = U1RXREG;
    if (UART1RXBUFF_head == UART1RXBUFF_tail) {
        ++UART1RXBUFF_tail;
        if (UART1RXBUFF_tail >= UART1RXBUFFLEN)
            UART1RXBUFF_tail = 0;
    }
    _U1RXIF = 0; /*  Clear the UART1RX Interrupt Flag; */
}

bool UART1GetAvailable(void) {
    if (UART1RXBUFF_head == UART1RXBUFF_tail) {
        return false;
    }

    return true;
}

uint8_t UART1GetByte(void) {
    if (++UART1RXBUFF_tail >= UART1RXBUFFLEN)
        UART1RXBUFF_tail = 0;
    return UART1RXBUFF[UART1RXBUFF_tail];
}

size_t UART1GetBytes(uint8_t *output, size_t n) {
    size_t cnt;
    cnt = 0u;
    while (cnt < n && UART1RXBUFF_head == UART1RXBUFF_tail) {
        if (++UART1RXBUFF_tail >= UART1RXBUFFLEN)
            UART1RXBUFF_tail = 0;
        output [cnt++] = UART1RXBUFF[UART1RXBUFF_tail];
    }
    return cnt;
}

void UART1SendBytes(const uint8_t *input, size_t n) {
    #ifdef __HAS_DMA__
    __eds__ uint8_t *buffer;
    #else
    volatile uint8_t *buffer;
    #endif
    size_t n1;

    while (n+DMA0Cnt >= UART1TXBUFFLEN) {
        n1 = UART1TXBUFFLEN-DMA0Cnt;
        if (DMA0Sending & 0b01) {
            buffer = bufferUART1TXB;
        } else {
            buffer = bufferUART1TXA;
        }
#ifdef __HAS_DMA__
        _memcpy_eds(buffer, (__eds__ void *)input, n1);
#else
        memcpy(buffer, input, n1);
#endif
        DMA0Cnt += n1;
        UART1Flush();
        n -= n1;
        input += n1;
    }
    if (n > 0) {
        if (DMA0Sending & 0b01) {
            buffer = bufferUART1TXB;
        } else {
            buffer = bufferUART1TXA;
        }
#ifdef __HAS_DMA__
        _memcpy_eds(buffer, (__eds__ void *)input, n);
#else
        memcpy(buffer, input, n);
#endif
        DMA0Cnt += n;
    }
}

void UART1SendString(const char input[]) {
    size_t n;
    n = strlen(input);
    UART1SendBytes((const uint8_t *)input, n);
}
