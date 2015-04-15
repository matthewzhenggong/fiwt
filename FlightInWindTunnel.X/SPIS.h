/*
 * File:   SPIS.h
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

#ifndef SPIS_H
#define	SPIS_H

#include "config.h"

#if USE_SPIS

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

// GNDBOARD / dSPACE board SPI interface codes
#define DUMMY_DATA      '\x56'

#define MaximumPayload 86
/* Define struct DATA_PCKT_Frame data type */
typedef struct {
	unsigned char PCKT_LENGTH_MSB;
	unsigned char PCKT_LENGTH_LSB;
	unsigned char RF_DATA[MaximumPayload];
} DATA_PCKT_Frame, *DATA_PCKT_Frame_Ptr;

    /**
     * New SPI RX pakage pointer
     * Set it to NULL after reading
     */
    extern DATA_PCKT_Frame_Ptr SPIRX_RX_PCKT_PTR;

    /**
     *  Config and initialize peripheral SPI1
     */
    void SPISInit(void);

    /**
     * Start peripheral SPI1
     *
     * @note Call it before read
     */
    void SPISStart(void);

    /**
     * Push a message into queue to dspace
     */
    void SPIS_push(const uint8_t *Payload, size_t length);

#ifdef	__cplusplus
}
#endif

#endif /* USE_SPIS */

#endif	/* SPIS_H */

