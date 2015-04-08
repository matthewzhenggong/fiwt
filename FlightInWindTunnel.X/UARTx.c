/*
 * File:   UARTx.c
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

#include <string.h>
#include <xc.h>
#include <libpic30.h>
#include <stdint.h>
#include <stdbool.h>

#include "config_uart.h"

/** Allocate two buffers for DMA transfers */
__eds__ static uint8_t bufferUARTTXA[UARTx(TXBUFFLEN)] __attribute__((eds, space(dma)));
__eds__ static uint8_t bufferUARTTXB[UARTx(TXBUFFLEN)] __attribute__((eds, space(dma)));
__near static volatile uint16_t DMASending;
__near static volatile uint16_t DMACntTx;

__eds__ static uint8_t UARTRXBUFF[UARTx(RXBUFFLEN)] __attribute__((eds, space(dma)));
__near static volatile unsigned int UARTRXBUFF_tail;
__near static unsigned int DMARxStart;
__near static unsigned int DMARxEnd;

void UARTx(Init)(void) {

    /* Configure UxTX as output, and UxRX as input */
    UARTx(TXTRIS) = 0b0;
    UARTx(RXTRIS) = 0b1;

    /* Assign UART1 pins through Peripherial Pin Select */
    _UxREG(RXR) = UARTx(RXPR);
    UARTx(TXPR) = UARTxTXPR;

    /* Initialize the appropiate baud rate */
    UxREG(BRG) = ((Fcy/UARTx(BAUdRATE))/16)-1;
    /* Configure UART module */
    UxREG(MODE) = UxMODEvalue;
    /* UART STATUS AND CONTROL REGISTER */
    UxREG(STA) = UxSTAvalue;

    /* Priorities of UART Interrupts */
    _UxREG(RXIP) = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Receiver priority out 5 of 7 */
    _UxREG(TXIP) = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  UART Trans priority out 5 of 7 */
    _UxREG(EIP) = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW;

    /* Disable UART Interrupts */
    _UxREG(RXIF) = 0;
    _UxREG(RXIE) = 0;
    _UxREG(TXIF) = 0;
    _UxREG(TXIE) = 0;
    _UxREG(EIF) = 0;
    _UxREG(EIE) = 0;

    /**   Associate DMA Channel with UART Tx */
    DMAxTX(REQ) = DMAxTXREQ; /*  Select UART Transmitter */
    DMAxTX(PAD) = (volatile uint16_t) & UxREG(TXREG);
    /*   Configure DMA Channel */
    DMAxTX(CON) = DMA_TO_UART_CONFIG;

    /**   Associate DMA Channel with UART Rx */
    DMAxRX(REQ) = DMAxRXREQ; /*  Select UART RX */
    DMAxRX(PAD) = (volatile uint16_t) & UxREG(RXREG);

    /*   Configure DMA Channel */
    DMAxRX(CON) = UART_TO_DMA_CONFIG;
    DMAxRX(CNT) = UARTx(RXBUFFLEN) - 1u;
    DMARxStart = __builtin_dmaoffset(UARTRXBUFF);
    DMARxEnd = DMARxStart + UARTx(RXBUFFLEN);
    DMAxRX(STAH) = __builtin_dmapage(UARTRXBUFF);

    /* Priorities of DMA Interrupts */
    _DMAxTX(IE) = 0; /*  Enable DMA interrupt */
    _DMAxTX(IP) = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMAxTX(IF) = 0; /*  Clear DMA Interrupt Flag */

    _DMAxRX(IE) = 0; /*  Enable DMA interrupt */
    _DMAxRX(IP) = HARDWARE_INTERRUPT_PRIORITY_LEVEL_LOW; /*  DMAx priority out 5 of 7 */
    _DMAxRX(IF) = 0; /*  Clear DMA Interrupt Flag */
}

void UARTx(Start)(void) {

    DMASending = 0;
    DMACntTx = 0;
    /*   Enable DMA Interrupts */
    _DMAxTX(IF) = 0; /*  Clear DMA Interrupt Flag */
    _DMAxTX(IE) = 1; /*  Enable DMA interrupt */

    DMAxRX(STAL) = UARTRXBUFF_tail = DMARxStart;
    DMAxRX(CONbits).CHEN = 1; /* Enable DMA channel */

    /* Enable the UARTx module (UxMODE<15>) */
    UxREG(MODEbits).UARTEN = 1; /*  UARTEN: UARTx Enable bit. */

    /* Enable the UARTx module transmission */
    UxREG(STAbits).UTXEN = 1; /*  The UTXEN bit is set after the UARTEN bit has been set; otherwise, UART transmissions will not be enabled. */
}

/** Setup DMA interrupt handlers */
__interrupt(no_auto_psv) void _DMAxTX(Interrupt)(void) {
    DMASending &= 0xFFFDu;
    _DMAxTX(IF) = 0; /*  Clear the DMAx Interrupt Flag; */
}

/**
 * Send all data in output buffer
 *
 * @note swap double-bufferes and send with DMAx
 */
void UARTx(Flush)(void) {

    if (DMACntTx == 0)
        return;

    if (_DMAxTX(IE) == 0u) {
        return;
    }
    while (DMASending & 0b10) {
        /* wait */
    }

    /*  buff swap */

    DMAxTX(CNT) = DMACntTx - 1u; /*  N+1 DMA requests */
    DMASending ^= 1u;
    DMACntTx = 0;

    if (DMASending & 1u) {
        DMAxTX(STAH) = __builtin_dmapage(bufferUARTTXA);
        DMAxTX(STAL) = __builtin_dmaoffset(bufferUARTTXA);
    } else {
        DMAxTX(STAH) = __builtin_dmapage(bufferUARTTXB);
        DMAxTX(STAL) = __builtin_dmaoffset(bufferUARTTXB);
    }
    DMAxTX(CONbits).CHEN = 1; /*  Re-enable DMA Channel */
    DMAxTX(REQbits).FORCE = 1; /*  Manual mode: Kick-start the first transfer */
    DMASending |= 0b10;
}

void UARTx(SendByte)(uint8_t inbyte) {
    if (DMASending & 0b01) {
        bufferUARTTXB[DMACntTx++] = inbyte;
    } else {
        bufferUARTTXA[DMACntTx++] = inbyte;
    }
    if (DMACntTx >= UARTx(TXBUFFLEN)) {
        UARTx(Flush)();
    }
}

bool UARTx(GetAvailable)(void) {
    if (DMAxRX(STAL) == UARTRXBUFF_tail) {
        return false;
    }
    return true;
}

uint8_t UARTx(GetByte)(void) {
    uint8_t c;
    c = UARTRXBUFF[UARTRXBUFF_tail - DMARxStart];
    if (++UARTRXBUFF_tail >= DMARxEnd)
        UARTRXBUFF_tail = DMARxStart;
    return c;
}

size_t UARTx(GetBytes)(uint8_t *output, size_t n) {
    size_t cnt;
    unsigned int head;
    signed int len;

    head = DMAxRX(STAL);
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

void UARTx(SendBytes)(const uint8_t *input, size_t n) {
    __eds__ uint8_t *buffer;
    size_t n1;

    while (n + DMACntTx >= UARTx(TXBUFFLEN)) {
        n1 = UARTx(TXBUFFLEN) - DMACntTx;
        if (DMASending & 0b01) {
            buffer = bufferUARTTXB;
        } else {
            buffer = bufferUARTTXA;
        }
        _memcpy_eds(buffer, (__eds__ void *) input, n1);
        DMACntTx += n1;
        UARTx(Flush)();
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

void UARTx(SendString)(const char input[]) {
    size_t n;
    n = strlen(input);
    UARTx(SendBytes)((const uint8_t *) input, n);
}
