/*
 * File:   msg_gnd.h
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

#ifndef MSG_GND_H
#define	MSG_GND_H

#include "config.h"

#if GNDBOARD

#include "msg.h"
#include "task.h"
#include <stdint.h>
#include <stddef.h>

#define COMMPACKLEN (1+4*2+3)
#define PINPACK2LEN (10)

#ifdef	__cplusplus
extern "C" {
#endif

    void msgInit(msgParam_p parameters, XBee_p);

    size_t updateRigPack(uint8_t head[]);

    size_t updateCommPack(TaskHandle_p task, uint8_t head[]);

    uint8_t * push_payload(uint8_t *spis_pkg_buff, const uint8_t *buff, size_t length);
    
    uint8_t * push_timestamp(uint8_t *spis_pkg_buff, const uint8_t *timestamp, size_t length);

    uint8_t * pull_payload(uint8_t *spis_pkg_buff, const uint8_t *buff, size_t length);

    size_t updatePingPack(PM_p pm, uint8_t *head);

    size_t updatePingPack2(PM_p pm, uint8_t *head);



#ifdef	__cplusplus
}
#endif

#endif /*GNDBOARD*/

#endif	/* MSG_GND_H */

