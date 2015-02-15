/*
 * File:   UARTx.h
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


#ifndef UARTX_H
#define	UARTX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file UARTx.h operationes of the first UART peripheral
 */

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     *  Config and initialize peripheral UARTx
     */
    void UARTxFunc(Init)(void);

    /**
     * Start peripheral UARTx
     *
     * @note Call it before read/write operations
     */
    void UARTxFunc(Start)(void);

    //void UARTxStop(void);

    /**
     * Write one byte into the output buffer of peripheral UARTx
     */
    void UARTxFunc(SendByte)(uint8_t);

    /**
     * Send all data in current output buffer of peripheral UARTx
     */
    void UARTxFunc(Flush)(void);

    /**
     * Test if there is any data in input buffer peripheral UARTx for read
     *
     * @return true if you can read.
     * @note Call this test before any read operation on UARTx
     */
    bool UARTxFunc(GetAvailable)(void);

    /**
     * Read one byte from input buffer of peripheral UARTx
     *
     * @note Call UARTxGetAvailable() test before reading
     */
    uint8_t UARTxFunc(GetByte)(void);

    /**
     * Read at most n bytes from input buffer of peripheral UARTx
     *
     * @return number of read bytes
     */
    size_t UARTxFunc(GetBytes)(uint8_t *, size_t);

    /**
     * Send n bytes to output buffer of peripheral UARTx
     */
    void UARTxFunc(SendBytes)(const uint8_t *, size_t);

    /**
     * Send a C-style string to output buffer of peripheral UARTx
     */
    void UARTxFunc(SendString)(const char []);

#ifdef	__cplusplus
}
#endif

#endif	/* UARTX_H */

