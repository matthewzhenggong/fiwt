/*
 * File:   ekfTask.c
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


#if USE_EKF

#include "ekfTask.h"
#include "IMU.h"

void ekfInit(ekfParam_p parameters, float dt) {
    struct pt *pt;

    pt = &(parameters->PT);
    PT_INIT(pt);

    parameters->op = INITIALIZE;
    parameters->y0[0] = parameters->y0[1] = parameters->y0[2] = 0; // px py pz
    parameters->y0[3] = parameters->y0[4] = parameters->y0[5] = 0; // vx vy vz
    parameters->y0[6] = parameters->y0[7] = parameters->y0[8] = 0; // roll pitch yaw
    parameters->y0[9] = parameters->y0[10] = parameters->y0[11] = 0; // bax bay baz
    parameters->y0[12] = parameters->y0[13] = parameters->y0[14] = 0; // bwx bwy bwz
    parameters->y0[15] = dt;

}

int16_t Get14bit(unsigned int val) {
    if (val & 0x2000) {
        return val | 0xC000;
    } else {
        return val;
    }
}

PT_THREAD(ekfLoop)(TaskHandle_p task) {
    ekfParam_t *parameters;
    struct pt *pt;
    float pqr[3];
    float acc[3];
    float val[3];

    parameters = (ekfParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        pqr[0] = Get14bit(IMU_XGyro)*0.05f;
        pqr[1] = Get14bit(IMU_YGyro)*-0.05f;
        pqr[2] = Get14bit(IMU_ZGyro)*-0.05f;
        acc[0] = Get14bit(IMU_XAccl)*-0.003333f;
        acc[1] = Get14bit(IMU_YAccl)*0.003333f;
        acc[2] = Get14bit(IMU_ZAccl)*0.003333f;
        switch (parameters->op) {
            case INITIALIZE:
                EKF_Filter(&parameters->ekff, INITIALIZE, parameters->y0, pqr, acc);
                parameters->op = UPDATEPOS;
                break;
            case UPDATEPOS:
                val[0] = val[1] = val[2] = 0.0;
                //EKF_Filter(&parameters->ekff, UPDATEPOS, val, pqr, acc);
                parameters->op = UPDATEVEL;
                break;
            case UPDATEVEL:
                val[0] = val[1] = val[2] = 0.0;
                EKF_Filter(&parameters->ekff, UPDATEVEL, val, pqr, acc);
                parameters->op = UPDATECMP;
                break;
            case UPDATECMP:
                val[0] = 0.0;
                //EKF_Filter(&parameters->ekff, UPDATECMP, val, pqr, acc);
                parameters->op = UPDATEPOS;
                break;
            default:
                parameters->op = INITIALIZE;
        }

        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /*USE_EKF*/
