/*
 * File:   PWMx.c
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

#include "PWMx.h"

#if USE_PWM

#include <xc.h>
#include "config_pwm.h"

#define RESET_PWM(idx,phase) \
    PWMCON##idx = PWMCONx_CFG; \
    IOCON##idx = IOCONx_CFG; \
    FCLCON##idx = FCLCONx_CFG; \
    PDC##idx = 0; \
    PHASE##idx = phase; \
    SDC##idx = 0; \
    SPHASE##idx = phase; \
    ALTDTR##idx = 0u; \
    TRIG##idx = TRIGx_CFG; \
    TRGCON##idx = 0u; \
    LEBCON##idx = LEBCONx_CFG; \
    LEBDLY##idx = LEBDLYx_CFG; \
    PWMCAP##idx = 0u; \
    AUXCON##idx = AUXCONx_CFG

void PWMxInit(void) {
    PTCON = PTCON_CFG;
    PTCON2 = PTCON2_CFG;
    PTPER = PTPER_CFG;
    STCON = STCON_CFG;
    STCON2 = STCON2_CFG;
    STPER = STPER_CFG;
    MDC = MDC_CFG;
    SEVTCMP = SEVTCMP_CFG;
    SSEVTCMP = SSEVTCMP_CFG;
    CHOP = CHOP_CFG;

#if GNDBOARD		// Only for GNDBOARD board
#else				// Only for AC_MODEL and AEROCOMP boards
    RESET_PWM(1, 0);
    IOCON1bits.PENH = 1;
    RESET_PWM(2, PWM_PEROID*6u/7u);
    IOCON2bits.PENL = 1;
    RESET_PWM(5, PWM_PEROID*3u/7u);
    IOCON5bits.PENL = 1;
    RESET_PWM(7, PWM_PEROID/7u);
    IOCON7bits.PENL = 1;
    #if AC_MODEL	// Only for AC_MODEL board
    RESET_PWM(3, PWM_PEROID*5u/7u);
    IOCON3bits.PENL = 1;
    RESET_PWM(6, PWM_PEROID*2u/7u);
    IOCON6bits.PENL = 1;
    #endif
#endif
}

void PWMxStart(void) {
    /* Turn on the High-Speed PWM module (PTEN<15>). */
    _PTEN = 0b1; // PWM Module Enable bit: 1 = PWM module is enabled.

    /* reset */
    PDC1 = 0;
    SDC2 = 0;
    SDC3 = 0;
    SDC5 = 0;
    SDC6 = 0;
    SDC7 = 0;
}

#endif /* USE_PWM*/
