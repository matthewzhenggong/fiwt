/*
 * File:   config_adc.h
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

#ifndef CONFIG_ADC_H
#define	CONFIG_ADC_H

/** For ADC1 */
#if GNDBOARD
#define BATTCELLADCNUM (0)
#define SERVOPOSADCNUM (0)
#else
#define BATTCELLADCNUM (3)
#if AC_MODEL
#define SERVOPOSADCNUM (6)
#elif AEROCOMP
#define SERVOPOSADCNUM (4)
#endif
#endif

#endif	/* CONFIG_ADC_H */

