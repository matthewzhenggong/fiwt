/*
 * File:   echoTask.c
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

#include "servoTask.h"
#include "Servo.h"
#include "AnalogInput.h"
#include "Enc.h"

void servoInit(servoParam_t *parameters) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);
    parameters->_ticks = 0u;
    parameters->_peroid = 0u;
    parameters->_dc = 0u;
    parameters->_ch = 0u;
    parameters->_loop = 0u;
    parameters->_test_mode = 0u;
}

PT_THREAD(servoLoop)(TaskHandle_p task) {
    servoParam_t *parameters;
    struct pt *pt;
    unsigned int i;

    parameters = (servoParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        UpdateAnalogInputs();
        EncUpdate();
#if AEROCOMP
        UpdateServoPosFromEnc();
#endif /*AEROCOMP*/
        if (parameters->_loop) {
            switch (parameters->_test_mode) {
                case 1u:
                    ServoUpdate100Hz(parameters->_ch, parameters->_dc);
                    break;
                default:
                    MotorSet(parameters->_ch, parameters->_dc);
            }
            if (--parameters->_ticks == 0u) {
                --parameters->_loop;
                parameters->_ticks = parameters->_peroid;

                switch (parameters->_test_mode) {
                    case 1u:
                        if (parameters->_loop & 1) {
                            parameters->_dc -= 23;
                        } else {
                            parameters->_dc += 23;
                        }
                        break;
                    default:
                        parameters->_dc = -parameters->_dc;
                }
            }
        } else {
            for (i = 0u; i < SEVERONUM; ++i) {
                MotorSet(i, 0);
            }
        }
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

