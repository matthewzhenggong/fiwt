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

#ifndef MSG_ACM_H
#define	MSG_ACM_H

#include "config.h"

#if AC_MODEL || AEROCOMP

#include "msg.h"
#include "servoTask.h"
#include "task.h"
#include <stdint.h>
#include <stddef.h>

#define BATTPACKLEN (1+BATTCELLADCNUM+4)
#define COMMPACKLEN (1+4*2+3)

#ifdef	__cplusplus
extern "C" {
#endif

    void msgInit(msgParam_p parameters, XBee_p, TaskHandle_p, TaskHandle_p);

    size_t updateBattPack(uint8_t head[]);

    size_t updateCommPack(TaskHandle_p sen_Task, TaskHandle_p serov_Task,
            TaskHandle_p task, uint8_t head[]);

    size_t updateSensorPack(uint8_t head[]);

    void servoProcA5Cmd(servoParam_p parameters, const uint8_t cmd[]);


#ifdef	__cplusplus
}
#endif

#endif /*AC_MODEL || AEROCOMP*/

#endif	/* MSG_ACM_H */

