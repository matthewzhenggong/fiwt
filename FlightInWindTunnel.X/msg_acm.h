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

    typedef struct msgParamACM{
        TaskHandle_p sen_Task;
        TaskHandle_p servo_Task;
        TaskHandle_p msg_Task;
        servoParam_p serov_param;
    }msgParamACM_t, *msgParamACM_p;

    void msgRegistACM(msgParam_p, msgParamACM_p, unsigned);

    bool sendDataPack(uint32_t time_token);

#ifdef	__cplusplus
}
#endif

#endif /*AC_MODEL || AEROCOMP*/

#endif	/* MSG_ACM_H */

