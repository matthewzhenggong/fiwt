/*
 * File:   senTask.c
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

#include "config.h"

#include "senTask.h"
#if GNDBOARD
#include "msg_gnd.h"
#endif

#include "AnalogInput.h"
#include "Enc.h"
#include "IMU.h"

void senInit(senParam_p parameters) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);

}

PT_THREAD(senLoop)(TaskHandle_p task) {
    senParam_t *parameters;
    struct pt *pt;

    parameters = (senParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
#if USE_ENC
        EncUpdate();
#endif
#if USE_IMU
        IMUUpdate();
#endif
#if USE_ADC1
        UpdateAnalogInputs();
#endif
#if AEROCOMP
        UpdateServoPosFromEnc();
#elif GNDBOARD
        UpdateRigPos();
        sendRigPack();
#endif /*AEROCOMP*/
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}
