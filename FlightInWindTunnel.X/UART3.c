/*
 * File:   UART3.c
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

#if USE_UART3

#include "UART3.h"

#include <string.h>
#include <xc.h>
#include <libpic30.h>

/** Compute baudrate value and store in BR_Value */
#define BR_Value ((Fcy/UART3BAUdRATE)/16)-1


/** Allocate two buffers for DMA transfers */
__eds__ static uint8_t bufferUARTTXA[UART3TXBUFFLEN] __attribute__((eds, space(dma)));
__eds__ static uint8_t bufferUARTTXB[UART3TXBUFFLEN] __attribute__((eds, space(dma)));
__near static volatile uint16_t DMASending;
__near static volatile uint16_t DMACntTx;

__eds__ static uint8_t UARTRXBUFF[UART3RXBUFFLEN] __attribute__((eds, space(dma)));
__near static volatile unsigned int UARTRXBUFF_tail;
__near static unsigned int DMARxStart;
__near static unsigned int DMARxEnd;

void UART3Init(void) {

    /* Configure UxTX as output, and UxRX as input */
    UART3TXTRIS = 0b0;
    UART3RXTRIS = 0b1;

    /* Assign UART3 pins through Peripherial Pin Select */
    _U3RXR = UART3RXPR;
    UART3TXPR = 0x1B;

    /* Initialize the appropiate baud rate */
    U3BRG = BR_Value;
    /* Configure UART module */
    U3MODE = UxMODEvalue;
    /* UART STATUS AND CONTROL REGISTER */
    U3STA = UxSTAvalue;

    /* Priorities of UART Interrupts */
    _U3RXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Receiver priority out 5 of 7 */
    _U3TXIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Trans priority out 5 of 7 */
    _U3EIP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW;

    /* Disable UART Interrupts */
    _U3RXIF = 0;
    _U3RXIE = 0;
    _U3TXIF = 0;
    _U3TXIE = 0;
    _U3EIF = 0;
    _U3EIE = 0;

    /**   Associate DMA Channel with UART Tx */
    DMA2REQ = 0b01010011; /*  Select UART Transmitter */
    DMA2PAD = (volatile uint16_t) & U3TXREG;
    /*   Configure DMA Channel */
    DMA2CON = DMA_TO_UART_CONFIG;

    /**   Associate DMA Channel with UART Rx */
    DMA6REQ = 0b01010010; /*  Select UART RX */
    DMA6PAD = (volatile uint16_t) & U3RXREG;

    /*   Configure DMA Channel */
    DMA6CON = UART_TO_DMA_CONFIG;
    DMA6CNT = UART3RXBUFFLEN - 1u;
    DMARxStart = __builtin_dmaoffset(UARTRXBUFF);
    DMARxEnd = DMARxStart + UART3RXBUFFLEN;
    DMA6STAH = __builtin_dmapage(UARTRXBUFF);

    /* Priorities of DMA Interrupts */
    _DMA2IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMA2IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA2IE = 0; /*  Enable DMA interrupt */

    _DMA6IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMA6IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA6IE = 0; /*  Enable DMA interrupt */
}

void UART3Start(void) {

    DMASending = 0;
    DMACntTx = 0;
    /*   Enable DMA Interrupts */
    _DMA2IF = 0; /*  Clear DMA Interrupt Flag */
    _DMA2IE = 1; /*  Enable DMA interrupt */

    DMA6STAL = UARTRXBUFF_tail = DMARxStart;
    DMA6CONbits.CHEN = 1; /* Enable DMA channel */

    /* Enable the UARTx module (UxMODE<15>) */
    U3MODEbits.UARTEN = 1; /*  UARTEN: UARTx Enable bit. */

    /* Enable the UARTx module transmission */
    U3STAbits.UTXEN = 1; /*  The UTXEN bit is set after the UARTEN bit has been set; otherwise, UART transmissions will not be enabled. */
}

/** Setup DMA interrupt handlers */
__interrupt(no_auto_psv) void _DMA2Interrupt(void) {
    DMASending &= 0xFFFDu;
    _DMA2IF = 0; /*  Clear the DMAx Interrupt Flag; */
}

/**
 * Send all data in output buffer
 *
 * @note swap double-bufferes and send with DMAx
 */
void UART3Flush(void) {

    if (DMACntTx == 0)
        return;

    if (_DMA2IE == 0u) {
        return;
    }
    while (DMASending & 0b10) {
        /* wait */
    }

    /*  buff swap */

    DMA2CNT = DMACntTx - 1u; /*  N+1 DMA requests */
    DMASending ^= 1u;
    DMACntTx = 0;

    if (DMASending & 1u) {
        DMA2STAH = __builtin_dmapage(bufferUARTTXA);
        DMA2STAL = __builtin_dmaoffset(bufferUARTTXA);
    } else {
        DMA2STAH = __builtin_dmapage(bufferUARTTXB);
        DMA2STAL = __builtin_dmaoffset(bufferUARTTXB);
    }
    DMA2CONbits.CHEN = 1; /*  Re-enable DMA Channel */
    DMA2REQbits.FORCE = 1; /*  Manual mode: Kick-start the first transfer */
    DMASending |= 0b10;
}

void UART3SendByte(uint8_t inbyte) {
    if (DMASending & 0b01) {
        bufferUARTTXB[DMACntTx++] = inbyte;
    } else {
        bufferUARTTXA[DMACntTx++] = inbyte;
    }
    if (DMACntTx >= UART3TXBUFFLEN) {
        UART3Flush();
    }
}

bool UART3GetAvailable(void) {
    if (DMA6STAL == UARTRXBUFF_tail) {
        return false;
    }
    return true;
}

uint8_t UART3GetByte(void) {
    uint8_t c;
    c = UARTRXBUFF[UARTRXBUFF_tail - DMARxStart];
    if (++UARTRXBUFF_tail >= DMARxEnd)
        UARTRXBUFF_tail = DMARxStart;
    return c;
}

size_t UART3GetBytes(uint8_t *output, size_t n) {
    size_t cnt;
    unsigned int head;
    signed int len;

    head = DMA6STAL;
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

void UART3SendBytes(const uint8_t *input, size_t n) {
#ifdef __HAS_DMA__
    __eds__ uint8_t *buffer;
#else
    volatile uint8_t *buffer;
#endif
    size_t n1;

    while (n + DMACntTx >= UART3TXBUFFLEN) {
        n1 = UART3TXBUFFLEN - DMACntTx;
        if (DMASending & 0b01) {
            buffer = bufferUARTTXB;
        } else {
            buffer = bufferUARTTXA;
        }
        _memcpy_eds(buffer, (__eds__ void *) input, n1);
        DMACntTx += n1;
        UART3Flush();
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

void UART3SendString(const char input[]) {
    size_t n;
    n = strlen(input);
    UART3SendBytes((const uint8_t *) input, n);
}

#endif /* UART3ENABLE */
