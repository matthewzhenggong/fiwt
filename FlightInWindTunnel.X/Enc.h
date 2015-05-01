/*
 * File:   Enc.h
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

#ifndef ENC_H
#define	ENC_H

#include "config.h"

#if USE_ENC

#include "clock.h"
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

    extern unsigned int  EncPos[ENCNUM];

    /**
     * Start Encodes
     */
    void EncInit(void);

    /**
     * Read Encodes
     */
    void EncUpdate(void);

#ifdef	__cplusplus
}
#endif

#endif /* USE_ENC */

#endif	/* ENC_H */

