/*
 * File:   AnalogInput.h
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


#ifndef ANALOGINPUT_H
#define	ANALOGINPUT_H

#include "config.h"

#if USE_PWM && USE_ADC1

#include "clock.h"
#include "config_adc.h"

#ifdef	__cplusplus
extern "C" {
#endif

    /** BattCell ADC Value
     * Unit : 10mV
     */
    extern uint8_t BattCell[BATTCELLADCNUM];

    /** ServoPos ADC Value
     * from 0-180deg, 0-4096 output
     */
    extern unsigned int ServoPos[SERVOPOSADCNUM];

    /** Analog Input Time Stamp
     *  Update when call function UpdateAnalogInputs()
     */
    extern uint16_t ADC_TimeStamp[2];

    /** Update Analog Inputs from SFRs
     *  call it before visit ADC values
     */
    void UpdateAnalogInputs(void);

#if AEROCOMP
    void UpdateServoPosFromEnc(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* USE_ADC1 */


#endif	/* ANALOGINPUT_H */

