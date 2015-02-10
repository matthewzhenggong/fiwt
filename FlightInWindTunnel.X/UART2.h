/*
 * File:   UART2.h
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


#ifndef UART2_H
#define	UART2_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file UART2.h operationes of the first UART peripheral
 */

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     *  Config and initialize peripheral UART2
     */
    void UART2Init(void);

    /**
     * Start peripheral UART2
     *
     * @note Call it before read/write operations
     */
    void UART2Start(void);

    //void UART2Stop(void);

    /**
     * Write one byte into the output buffer of peripheral UART2
     */
    void UART2SendByte(uint8_t);

    /**
     * Send all data in current output buffer of peripheral UART2
     */
    void UART2Flush(void);

    /**
     * Test if there is any data in input buffer peripheral UART2 for read
     *
     * @return true if you can read.
     * @note Call this test before any read operation on UART2
     */
    bool UART2GetAvailable(void);

    /**
     * Read one byte from input buffer of peripheral UART2
     *
     * @note Call UART2GetAvailable() test before reading
     */
    /* please check UART2GetAvailable before call UART2GetByte */
    uint8_t UART2GetByte(void);

    /**
     * Read at most n bytes from input buffer of peripheral UART2
     *
     * @return number of read bytes
     */
    size_t UART2GetBytes(uint8_t *, size_t);

    /**
     * Send n bytes to output buffer of peripheral UART2
     */
    void UART2SendBytes(const uint8_t *, size_t);

    /**
     * Send a C-style string to output buffer of peripheral UART2
     */
    void UART2SendString(const char []);

#ifdef	__cplusplus
}
#endif

#endif	/* UART2_H */

