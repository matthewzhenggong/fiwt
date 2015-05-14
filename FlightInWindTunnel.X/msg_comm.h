/*
 * File:   msg_acm.h
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

#ifndef MSG_COMM_H
#define	MSG_COMM_H

#include "msg.h"

#ifdef	__cplusplus
extern "C" {
#endif

    extern  int16_t ntp_offset;
    extern  int16_t ntp_delay;

    void msgRegistNTP(msgParam_p, unsigned);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_COMM_H */

