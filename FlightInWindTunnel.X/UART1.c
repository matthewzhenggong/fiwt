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

#if UART1ENABLE

#include "UART1.h"

#include <string.h>
#include <xc.h>
#include <libpic30.h>

/** Compute baudrate value and store in BR_Value */
#define BR_Value ((Fcy/UART1BAUdRATE)/16)-1


/** Allocate two buffers for DMA transfers */
__eds__ static uint8_t bufferUARTTXA[UART1TXBUFFLEN] __attribute__((eds, space(dma)));
__eds__ static uint8_t bufferUARTTXB[UART1TXBUFFLEN] __attribute__((eds, space(dma)));
__near static volatile uint16_t DMASending;
__near static volatile uint16_t DMACntTx;

__eds__ static uint8_t UARTRXBUFF[UART1RXBUFFLEN] __attribute__((eds, space(dma)));
__near static volatile unsigned int UARTRXBUFF_tail;
__near static unsigned int DMARxStart;
__near static unsigned int DMARxEnd;

void UART1Init(void) {

    /* Configure UxTX as output, and UxRX as input */
    UART1TXTRIS = 0b0;
    UART1RXTRIS = 0b1;

    /* Assign UART1 pins through Peripherial Pin Select */
    _U1RXR = UART1RXPR;
    UART1TXPR = 0x01;

    /* Initialize the appropiate baud rate */
    U1BRG = BR_Value;
    /* Configure UART module */
    U1MODE = UxMODEvalue;
    /* UART STATUS AND CONTROL REGISTER */
    U1STA = UxSTAvalue;

    /* Priorities of UART Interrupts */
    _U1RXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Receiver priority out 5 of 7 */
    _U1TXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Trans priority out 5 of 7 */
    _U1EIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW;

    /* Disable UART Interrupts */
    _U1RXIF = 0;
    _U1RXIE = 0;
    _U1TXIF = 0;
    _U1TXIE = 0;
    _U1EIF = 0;
    _U1EIE = 0;

    /**   Associate DMA Channel with UART Tx */
    DMA0REQ = 0b00001100; /*  Select UART Transmitter */
    DMA0PAD = (volatile uint16_t) & U1TXREG;
    /*   Configure DMA Channel */
    DMA0CON = DMA_TO_UART_CONFIG;

    /**   Associate DMA Channel with UART Rx */
    DMA4REQ = 0b00001011; /*  Select UART RX */
    DMA4PAD = (volatile uint16_t) & U1RXREG;

    /*   Configure DMA Channel */
    DMA4CON = UART_TO_DMA_CONFIG;
    DMA4CNT = UART1RXBUFFLEN - 1u;
    DMARxStart = __builtin_dmaoffset(UARTRXBUFF);
    DMARxEnd = DMARxStart + UART1RXBUFFLEN;
    DMA4STAH = __builtin_dmapage(UARTRXBUFF);

    /* Priorities of DMA Interrupts */
    _DMA0IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMA0IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA0IE = 0; /*  Enable DMA interrupt */

    _DMA4IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMA4IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA4IE = 0; /*  Enable DMA interrupt */
}

void UART1Start(void) {

    DMASending = 0;
    DMACntTx = 0;
    /*   Enable DMA Interrupts */
    _DMA0IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA0IE = 1; /*  Enable DMA interrupt */

    DMA4STAL = UARTRXBUFF_tail = DMARxStart;
    DMA4CONbits.CHEN = 1; /* Enable DMA channel */

    /* Enable the UARTx module (UxMODE<15>) */
    U1MODEbits.UARTEN = 1; /*  UARTEN: UARTx Enable bit. */

    /* Enable the UARTx module transmission */
    U1STAbits.UTXEN = 1; /*  The UTXEN bit is set after the UARTEN bit has been set; otherwise, UART transmissions will not be enabled. */
}

/** Setup DMA interrupt handlers */
__interrupt(no_auto_psv) void _DMA0Interrupt(void) {
    DMASending &= 0xFFFDu;
    _DMA0IF = 0; /*  Clear the DMAx Interrupt Flag; */
}

/**
 * Send all data in output buffer
 *
 * @note swap double-bufferes and send with DMAx
 */
void UART1Flush(void) {

    if (DMACntTx == 0)
        return;

    if (_DMA0IE == 0u) {
        return;
    }
    while (DMASending & 0b10) {
        /* wait */
    }

    /*  buff swap */

    DMA0CNT = DMACntTx - 1u; /*  N+1 DMA requests */
    DMASending ^= 1u;
    DMACntTx = 0;

    if (DMASending & 1u) {
        DMA0STAH = __builtin_dmapage(bufferUARTTXA);
        DMA0STAL = __builtin_dmaoffset(bufferUARTTXA);
    } else {
        DMA0STAH = __builtin_dmapage(bufferUARTTXB);
        DMA0STAL = __builtin_dmaoffset(bufferUARTTXB);
    }
    DMA0CONbits.CHEN = 1; /*  Re-enable DMA Channel */
    DMA0REQbits.FORCE = 1; /*  Manual mode: Kick-start the first transfer */
    DMASending |= 0b10;
}

void UART1SendByte(uint8_t inbyte) {
    if (DMASending & 0b01) {
        bufferUARTTXB[DMACntTx++] = inbyte;
    } else {
        bufferUARTTXA[DMACntTx++] = inbyte;
    }
    if (DMACntTx >= UART1TXBUFFLEN) {
        UART1Flush();
    }
}

bool UART1GetAvailable(void) {
    if (DMA4STAL == UARTRXBUFF_tail) {
        return false;
    }
    return true;
}

uint8_t UART1GetByte(void) {
    uint8_t c;
    c = UARTRXBUFF[UARTRXBUFF_tail - DMARxStart];
    if (++UARTRXBUFF_tail >= DMARxEnd)
        UARTRXBUFF_tail = DMARxStart;
    return c;
}

size_t UART1GetBytes(uint8_t *output, size_t n) {
    size_t cnt;
    unsigned int head;
    signed int len;

    head = DMA4STAL;
    len = head - UARTRXBUFF_tail;
    cnt = 0u;
    if (len < 0 && UARTRXBUFF_tail + n >= DMARxEnd) {
        len = DMARxEnd - UARTRXBUFF_tail;
        _memcpy_eds((__eds__ void *)output,  &UARTRXBUFF[UARTRXBUFF_tail - DMARxStart], len);
        cnt += len;
        n -= len;
        UARTRXBUFF_tail = DMARxStart;
        len = head - UARTRXBUFF_tail;
    }

    if (len < n) {
        _memcpy_eds((__eds__ void *)output,  &UARTRXBUFF[UARTRXBUFF_tail - DMARxStart], len);
        UARTRXBUFF_tail += len;
        cnt += len;
    } else {
        _memcpy_eds((__eds__ void *)output,  &UARTRXBUFF[UARTRXBUFF_tail - DMARxStart], n);
        UARTRXBUFF_tail += n;
        cnt += n;
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

    while (n + DMACntTx >= UART1TXBUFFLEN) {
        n1 = UART1TXBUFFLEN - DMACntTx;
        if (DMASending & 0b01) {
            buffer = bufferUARTTXB;
        } else {
            buffer = bufferUARTTXA;
        }
        _memcpy_eds(buffer, (__eds__ void *) input, n1);
        DMACntTx += n1;
        UART1Flush();
        n -= n1;
        input += n1;
    }
    if (n > 0) {
        if (DMASending & 0b01) {
            buffer = bufferUARTTXB;
        } else {
            buffer = bufferUARTTXA;
        }
        _memcpy_eds(buffer, (__eds__ void *) input, n);
        DMACntTx += n;
    }
}

void UART1SendString(const char input[]) {
    size_t n;
    n = strlen(input);
    UART1SendBytes((const uint8_t *) input, n);
}

#endif /* UART1ENABLE */
