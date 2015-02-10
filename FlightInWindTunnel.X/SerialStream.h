/* 
 * File:   SerialStream.h
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

#ifndef SERIALSTREAM_H
#define	SERIALSTREAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     * HAL API stand for a serial port
     */
    typedef struct {
        /**
         *  Config and initialize the serial stream object
         */
        void (*init)(void);

        /**
         * Start the serial stream object
         *
         * @note Call it before read/write operations
         */
        void (*open)(void);

        /* void (*close)(void);*/

        /**
         * Test if there is any data in buffer for read
         *
         * @return true if you can read.
         * @note Call this test before any read operation
         */
        bool (*available)(void);

        /**
         * Read one byte from buffer
         *
         * @note Call available test before reading
         */
        uint8_t(*read)(void);

        /**
         * Read at most n bytes from buffer
         *
         * @param output : Buffer to store read bytes
         * @param n : maximum number to read
         * @return number of bytes read
         */
        size_t (*readBytes)(uint8_t *output, size_t n);

        /**
         * Send all data in output buffer and clear the buffer
         */
        void (*flush)(void);

        /** 
         * Write one byte into the output buffer
         */
        void (*write)(uint8_t input);

        /**
         * Write n bytes into the output buffer
         */
        void (*writeBytes)(const uint8_t *input, size_t n);

        /**
         * Write on C-style string into the output buffer
         */
        void (*writeString)(const char input[]);
    } SerialStream_t;

    typedef SerialStream_t * SerialStream_p;

    /**
     * Assign an UART device to the serial stream object
     *
     * @param _serial The object assigned to
     * @param UARTx The index number of UART device
     */
    void serial_assign(SerialStream_p _serial, unsigned int UARTx);

#ifdef	__cplusplus
}
#endif

#endif	/* SERIALSTREAM_H */

