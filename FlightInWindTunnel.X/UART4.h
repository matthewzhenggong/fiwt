/*
 * File:   UART4.h
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


#ifndef UART4_H
#define	UART4_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file UART4.h operationes of the first UART peripheral
 */

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     *  Config and initialize peripheral UART4
     */
    void UART4Init(void);

    /**
     * Start peripheral UART4
     *
     * @note Call it before read/write operations
     */
    void UART4Start(void);

    //void UART4Stop(void);

    /**
     * Write one byte into the output buffer of peripheral UART4
     */
    void UART4SendByte(uint8_t);

    /**
     * Send all data in current output buffer of peripheral UART4
     */
    void UART4Flush(void);

    /**
     * Test if there is any data in input buffer peripheral UART4 for read
     *
     * @return true if you can read.
     * @note Call this test before any read operation on UART4
     */
    bool UART4GetAvailable(void);

    /**
     * Read one byte from input buffer of peripheral UART4
     *
     * @note Call UART4GetAvailable() test before reading
     */
    /* please check UART4GetAvailable before call UART4GetByte */
    uint8_t UART4GetByte(void);

    /**
     * Read at most n bytes from input buffer of peripheral UART4
     *
     * @return number of read bytes
     */
    size_t UART4GetBytes(uint8_t *, size_t);

    /**
     * Send n bytes to output buffer of peripheral UART4
     */
    void UART4SendBytes(const uint8_t *, size_t);

    /**
     * Send a C-style string to output buffer of peripheral UART4
     */
    void UART4SendString(const char []);

#ifdef	__cplusplus
}
#endif

#endif	/* UART4_H */

