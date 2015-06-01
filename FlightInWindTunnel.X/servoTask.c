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

#include "config.h"

#if AC_MODEL || AEROCOMP

#include "servoTask.h"
#include "Servo.h"
#include "msg_acm.h"


void servoInit(servoParam_p parameters) {
    struct pt *pt;
    int i;

    pt = &(parameters->PT);
    PT_INIT(pt);

    for (i=0;i<SEVERONUM;++i) {
        parameters->Servo_PrevRef[i] = parameters->ServoRef[i] = 0x800;
    }
    parameters->GenerateInput_Flag = 0u;
    parameters->InputType = 0u;
    parameters->time_token = 0u;
}

PT_THREAD(servoLoop)(TaskHandle_p task) {
    servoParam_t *parameters;
    struct pt *pt;
    unsigned int i;
    int s;
    float sfun;
    static float tht;
    static float last_omega;
    static float min_omega=0.0f;
    static float max_omega=5.12f;
    float omega;
    float k;

    parameters = (servoParam_t *) (task->parameters);
    pt = &(parameters->PT);

    /* A protothread function must begin with PT_BEGIN() which takes a
     pointer to a struct pt. */
    PT_BEGIN(pt);

    /* We loop forever here. */
    while (1) {
        if (parameters->GenerateInput_Flag == 1u) {

            switch (parameters->InputType) {
                case 0u:
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            ServoUpdate(i, parameters->ServoRef[i]);
                        }
                    }
                    if (++parameters->cnt > parameters->StartTime) {
                        parameters->GenerateInput_Flag = 0u;
                    }
                    break;
                case 1u: //Step
                    if (parameters->cnt < parameters->StartTime) {
                        s = 0;
                    } else {
                        s = 1;
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                ServoUpdate(i, parameters->ServoRef[i]-s*parameters->MaxValue[i]);
                            } else {
                                ServoUpdate(i, parameters->ServoRef[i]+s*parameters->MaxValue[i]);
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+parameters->TimeDelta)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->cnt = 0u;
                            parameters->StartTime = 100u;
                            parameters->InputType = 0u;
                        }
                    }
                    break;
                case 2u: //Doublet
                    if (parameters->cnt < parameters->StartTime) {
                        s = 0;
                    } else if (parameters->cnt < parameters->StartTime+parameters->TimeDelta) {
                        s = 1;
                    } else {
                        s = -1;
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                ServoUpdate(i, parameters->ServoRef[i]-s*parameters->MaxValue[i]);
                            } else {
                                ServoUpdate(i, parameters->ServoRef[i]+s*parameters->MaxValue[i]);
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+2*parameters->TimeDelta)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->cnt = 0u;
                            parameters->StartTime = 100u;
                            parameters->InputType = 0u;
                        }
                    }
                    break;
                case 3u: //3-2-1-1
                    if (parameters->cnt < parameters->StartTime) {
                        s = 0;
                    } else if (parameters->cnt < parameters->StartTime+3*parameters->TimeDelta) {
                        s = 1;
                    } else if (parameters->cnt < parameters->StartTime+5*parameters->TimeDelta) {
                        s = -1;
                    } else if (parameters->cnt < parameters->StartTime+6*parameters->TimeDelta) {
                        s = 1;
                    } else {
                        s = -1;
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                ServoUpdate(i, parameters->ServoRef[i]-s*parameters->MaxValue[i]);
                            } else {
                                ServoUpdate(i, parameters->ServoRef[i]+s*parameters->MaxValue[i]);
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+7*parameters->TimeDelta)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->cnt = 0u;
                            parameters->StartTime = 100u;
                            parameters->InputType = 0u;
                        }
                    }
                    break;
                case 4u: //ramp
                    if (parameters->cnt < parameters->StartTime) {
                        s = 0;
                    } else if (parameters->cnt < parameters->StartTime+parameters->TimeDelta) {
                        s = (parameters->cnt - parameters->StartTime);
                    } else {
                        s = (parameters->StartTime+2*parameters->TimeDelta - parameters->cnt);
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                ServoUpdate(i, parameters->ServoRef[i]-(uint16_t)(((uint32_t)s*parameters->MaxValue[i])/(uint32_t)parameters->TimeDelta));
                            } else {
                                ServoUpdate(i, parameters->ServoRef[i]+(uint16_t)(((uint32_t)s*parameters->MaxValue[i])/(uint32_t)parameters->TimeDelta));
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+2*parameters->TimeDelta)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->cnt = 0u;
                            parameters->StartTime = 100u;
                            parameters->InputType = 0u;
                        }
                    }
                    break;
                case 6u: //Open Loop Step
                    if (parameters->cnt < parameters->StartTime) {
                        s = 2;
                    } else {
                        s = -2;
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                MotorSet(i, -s*parameters->MaxValue[i]);
                            } else {
                                MotorSet(i, s*parameters->MaxValue[i]);
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+parameters->StartTime)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->GenerateInput_Flag = 0u;
                        }
                    }
                    break;
                case 7u: //Frequency Sweep
                case 8u: //Frequency Sweep
                    if (parameters->cnt < parameters->StartTime) {
                        sfun = 0;
                    }
                    else if (parameters->cnt == parameters->StartTime) {
                        sfun = 0;
                        tht = 0;
                        last_omega = 0;
                        min_omega = parameters->MinValue[0]*0.125663706f;  // Hz -> rad/s  0-255:0-10.2Hz
                        max_omega = parameters->MinValue[1]*0.125663706f;  // Hz -> rad/s  0-255:0-10.2Hz
                    } else {
                        if (parameters->InputType == 8u) {
                            k = 0.0187f*(exp(4.0f*(parameters->cnt - parameters->StartTime)/(float)parameters->TimeDelta)-1);
                        } else {
                            k = (parameters->cnt - parameters->StartTime)/(float)parameters->TimeDelta;
                        }
                        if (k < 0.0f) k = 0.0f; else if (k > 1.0f) k = 1.0f;
                        omega = min_omega+(max_omega-min_omega)*k;

                        tht += (omega+last_omega) * 0.002048;
                        sfun = sin(tht);
                        last_omega = omega;
                    }
                    for (i = 0u; i < SEVERONUM; ++i) {
                        if ((1<<i) &parameters->Srv2Move) {
                            if (parameters->Sign[i] == 2u) {
                                ServoUpdate(i, parameters->ServoRef[i]-(int16_t)(sfun*parameters->MaxValue[i]));
                            } else {
                                ServoUpdate(i, parameters->ServoRef[i]+(int16_t)(sfun*parameters->MaxValue[i]));
                            }
                        }
                    }
                    if (++parameters->cnt > (parameters->StartTime+parameters->TimeDelta)) {
                        --parameters->NofCycles;
                        if (parameters->NofCycles > 0u) {
                            parameters->cnt = 0u;
                        } else {
                            parameters->cnt = 0u;
                            parameters->StartTime = 100u;
                            parameters->InputType = 0u;
                        }
                    }
                    break;
                default:
                    parameters->GenerateInput_Flag = 0u;
            }
        } else {
            for (i = 0u; i < SEVERONUM; ++i) {
                MotorSet(i, 0);
            }
        }
        sendDataPack(parameters->time_token);
        PT_YIELD(pt);
    }

    /* All protothread functions must end with PT_END() which takes a
     pointer to a struct pt. */
    PT_END(pt);
}

#endif /*AC_MODEL or AEROCOMP*/