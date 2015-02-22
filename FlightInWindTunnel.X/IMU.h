/*
 * File:   IMU.h
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

#ifndef IMU_H
#define	IMU_H

#include "config.h"
#include "clock.h"

#if AC_MODEL

#ifdef	__cplusplus
extern "C" {
#endif

extern unsigned int IMU_Supply;
extern unsigned int IMU_XGyro;
extern unsigned int IMU_YGyro;
extern unsigned int IMU_ZGyro;
extern unsigned int IMU_XAccl;
extern unsigned int IMU_YAccl;
extern unsigned int IMU_ZAccl;
extern unsigned int IMU_XGyroTemp;
extern unsigned int IMU_YGyroTemp;
extern unsigned int IMU_ZGyroTemp;
extern unsigned int IMU_AUX_ADC;
extern clockType_t IMU_TimeStamp;

    /**
     *  Config and initialize peripheral SPI1
     */
    void IMUInit(void);

    /**
     * Start peripheral SPI1
     *
     * @note Call it before read
     */
    void IMUStart(void);

    /**
     * Update IMU data
     */
    void IMUUpdate(void);

#ifdef	__cplusplus
}
#endif

#endif /* AC_MODEL */

#endif	/* IMU_H */

