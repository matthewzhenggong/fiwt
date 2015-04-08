/*
 * File:   config_pwm.h
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


#ifndef CONFIG_PWM_H
#define	CONFIG_PWM_H

/** For PWM */

/** PWM in 10kHz */
#define PWM_FREQ 10000u

/** PTCON: PWM TIME BASE CONTROL REGISTER
 * bit 15 PTEN = 0.  PWM Module Enable bit
 *                   1 = PWM module is enabled
 *                   0 = PWM module is disabled
 * bit 14 Unimplemented = 0.  Read as ?0?
 * bit 13 PTSIDL = 1.  PWM Time Base Stop in Idle Mode bit
 *                   1 = PWM time base halts in CPU Idle mode
 *                   0 = PWM time base runs in CPU Idle mode
 * bit 12 SESTAT = 0.  Special Event Interrupt Status bit
 *                   1 = Special Event Interrupt is pending
 *                   0 = Special Event Interrupt is not pending
 * bit 11 SEIEN = 0.  Special Event Interrupt Enable bit
 *                   1 = Special Event Interrupt is enabled
 *                   0 = Special Event Interrupt is disabled
 * bit 10 EIPU = 0.  Enable Immediate Period Updates bit(1)
 *                   1 = Active Period register is updated immediately
 *                   0 = Active Period register updates occur on PWM cycle boundaries
 * bit 9 SYNCPOL = 0.  Synchronize Input and Output Polarity bit(1)
 *                   1 = SYNCIx/SYNCO polarity is inverted (active-low)
 *                   0 = SYNCIx/SYNCO is active-high
 * bit 8 SYNCOEN = 0.  Primary Time Base Sync Enable bit(1)
 *                   1 = SYNCO output is enabled
 *                   0 = SYNCO output is disabled
 * bit 7 SYNCEN = 0.  External Time Base Synchronization Enable bit(1)
 *                   1 = External synchronization of primary time base is enabled
 *                   0 = External synchronization of primary time base is disabled
 * bit 6-4 SYNCSRC<2:0> = 000.  Synchronous Source Selection bits(1)
 *                   These bits select the SYNCIx or PTGOx input as the synchronous source. Refer to the ?High-Speed
 *                   PWM? chapter in the specific device data sheet for availability.
 * bit 3-0 SEVTPS<3:0> = 000.  PWM Special Event Trigger Output Postscaler Select bits(1)
 *                   1111 = 1:16 postscaler generates Special Event trigger at every 16th compare match event
 *                   ???
 *                   0001 = 1:2 postscaler generates Special Event trigger at every second compare match event
 *                   0000 = 1:1 postscaler generates Special Event trigger at every compare match event
 */
#define PTCON_CFG 0x2000u

/** PTCON2: PWM PRIMARY MASTER CLOCK DIVIDER SELECT REGISTER
 * bit 15-3 Unimplemented = 0.  Read as ?0?
 * bit 2-0 PCLKDIV<2:0> = 110.  PWM Input Clock Prescaler (Divider) Select bits(1)
 *                   111 = Reserved
 *                   110 = Divide by 64
 *                   101 = Divide by 32
 *                   100 = Divide by 16
 *                   011 = Divide by 8
 *                   010 = Divide by 4
 *                   001 = Divide by 2
 *                   000 = Divide by 1, maximum PWM timing resolution (power-on default)
 */
#define PTCON2_CFG 4

/**
 * Duty circle for each PWM generator should be less than PWM_PEROID and greater than zero.
 */
#define PWM_PEROID ((SYS_FREQ >> PTCON2_CFG) / PWM_FREQ)

/** PRIMARY MASTER TIME BASE PERIOD REGISTER */
#define PTPER_CFG PWM_PEROID

#define STCON_CFG PTCON_CFG
#define STCON2_CFG PTCON2_CFG
#define STPER_CFG PTPER_CFG

/** MDC: PWM MASTER DUTY CYCLE REGISTER */
#define MDC_CFG 0x0000u

/** SEVTCMP: PWM PRIMARY SPECIAL EVENT COMPARE REGISTER */
#define SEVTCMP_CFG 50u

/** SSEVTCMP: PWM SECONDARY SPECIAL EVENT COMPARE REGISTER */
#define SSEVTCMP_CFG 50u

/** CHOP: PWM CHOP CLOCK GENERATOR REGISTER */
#define CHOP_CFG 0x0000u

/** PWMCONx
 *
 * bit 15 FLTSTAT = 0.  Fault Interrupt Status bit(1)
 *                   1 = Fault interrupt is pending
 *                   0 = No fault interrupt is pending.
 *                   This bit is cleared by setting FLTIEN = 0.
 * bit 14 CLSTAT = 0.  Current-Limit Interrupt Status bit(1)
 *                   1 = Current-limit interrupt is pending
 *                   0 = No current-limit interrupt is pending.
 *                   This bit is cleared by setting CLIEN = 0.
 * bit 13 TRGSTAT = 0.  Trigger Interrupt Status bit
 *                   1 = Trigger interrupt is pending
 *                   0 = No trigger interrupt is pending.
 *                   This bit is cleared by setting TRGIEN = 0.
 * bit 12 FLTIEN = 0.  Fault Interrupt Enable bit
 *                   1 = Fault interrupt is enabled
 *                   0 = Fault interrupt is disabled and the FLTSTAT bit is cleared
 * bit 11 CLIEN = 0.  Current-Limit Interrupt Enable bit
 *                   1 = Current-limit interrupt enabled
 *                   0 = Current-limit interrupt disabled and the CLSTAT bit is cleared
 * bit 10 TRGIEN = 0.  Trigger Interrupt Enable bit
 *                   1 = A trigger event generates an interrupt request
 *                   0 = Trigger event interrupts are disabled and the TRGSTAT bit is cleared
 * bit 9 ITB = 0.  Independent Time Base Mode bit(3)
 *                   1 = PHASEx/SPHASEx registers provide time base period for this PWM generator
 *                   0 = PTPER register provides timing for this PWM generator
 * bit 8 MDCS = 0.  Master Duty Cycle Register Select bit(3)
 *                   1 = MDC register provides duty cycle information for this PWM generator
 *                   0 = PDCx and SDCx registers provide duty cycle information for this PWM generator
 * bit 7-6 DTC<1:0> = 10.  Dead Time Control bits
 *                   11 = Dead Time Compensation mode enabled
 *                   10 = Dead time function is disabled
 *                   01 = Negative dead time actively applied for Complementary Output mode(6)
 *                   00 = Positive dead time actively applied for all output modes
 * bit 5 DTCP = 0.  Dead Time Compensation Polarity bit(5)
 *                   1 = If DTCMPx pin = 0, PWMxL is shortened, and PWMxH is lengthened
 *                       If DTCMPx pin = 1, PWMxH is shortened, and PWMxL is lengthened
 *                   0 = If DTCMPx pin = 0, PWMxH is shortened, and PWMxL is lengthened
 *                       If DTCMPx pin = 1, PWMxL is shortened, and PWMxH is lengthened
 * bit 4 Unimplemented = 0.  Read as ?0?
 * bit 3 MTBS = 0.  Master Time Base Select bit
 *                   1 = PWM generator uses the secondary master time base for synchronization and the clock source
 *                        for the PWM generation logic (if secondary time base is available)
 *                   0 = PWM generator uses the primary master time base for synchronization and the clock source for
 *                        the PWM generation logic
 * bit 2 CAM = 0.  Center-Aligned Mode Enable bit(2,3)
 *                   1 = Center-Aligned mode is enabled
 *                   0 = Edge-Aligned mode is enabled
 * bit 1 XPRES = 0.  External PWM Reset Control bit(4)
 *                   1 = Current-limit source resets primary local time base for this PWM generator if it is in Independent
 *                       Time Base mode
 *                   0 = External pins do not affect PWM time base
 * bit 0 IUE = 1.  Immediate Update Enable bit(3)
 *                   1 = Updates to the active MDC/PDCx/SDCx/DTRx/ALTDTRx/PHASEx/SPHASEx registers are
 *                       immediate
 *                   0 = Updates to the active MDC/PDCx/SDCx/DTRx/ALTDTRx/PHASEx/SPHASEx registers are
 *                       synchronized to the PWM time base
 */
#define PWMCONx_CFG 0x0080u

/** IOCONx:
 *
 * bit 15 PENH = 0.  PWMxH Output Pin Ownership bit
 *                   1 = PWM module controls PWMxH pin
 *                   0 = GPIO module controls PWMxH pin
 * bit 14 PENL = 0.  PWMxL Output Pin Ownership bit
 *                   1 = PWM module controls PWMxL pin
 *                   0 = GPIO module controls PWMxL pin
 * bit 13 POLH = 1.  PWMxH Output Pin Polarity bit
 *                   1 = PWMxH pin is active-low
 *                   0 = PWMxH pin is active-high
 * bit 12 POLL = 1.  PWMxL Output Pin Polarity bit
 *                   1 = PWMxL pin is active-low
 *                   0 = PWMxL pin is active-high
 * bit 11-10 PMOD<1:0> = 11.  PWM # I/O Pin Mode bits
 *                   11 = PWM I/O pin pair is in True Independent PWM Output mode(3)
 *                   10 = PWM I/O pin pair is in Push-Pull Output mode
 *                   01 = PWM I/O pin pair is in Redundant Output mode
 *                   00 = PWM I/O pin pair is in Complementary Output mode
 * bit 9 OVRENH = 0.  Override Enable for PWMxH Pin bit
 *                   1 = OVRDAT<1> provides data for output on PWMxH pin
 *                   0 = PWM generator provides data for PWMxH pin
 * bit 8 OVRENL = 0.  Override Enable for PWMxL Pin bit
 *                   1 = OVRDAT<0> provides data for output on PWMxL pin
 *                   0 = PWM generator provides data for PWMxL pin
 * bit 7-6 OVRDAT<1:0> = 00.  State(2) for PWMxH, PWMxL Pins if Override is Enabled bits
 *                   If OVERENH = 1, OVRDAT<1> provides data for PWMxH
 *                   If OVERENL = 1, OVRDAT<0> provides data for PWMxL
 * bit 5-4 FLTDAT<1:0> = 00.  State(2) for PWMxH and PWMxL Pins if FLTMOD is Enabled bits(1)
 *                   IFLTMOD (FCLCONx<15>) = 0: Normal Fault mode:
 *                   If fault is active, FLTDAT<1> provides the state for PWMxH.
 *                   If fault is active, FLTDAT<0> provides the state for PWMxL.
 *                   IFLTMOD (FCLCONx<15>) = 1: Independent Fault mode:
 *                   If current-limit is active, FLTDAT<1> provides the state for PWMxH.
 *                   If fault is active, FLTDAT<0> provides the state for PWMxL.
 * bit 3-2 CLDAT<1:0> = 00.  State(2) for PWMxH and PWMxL Pins if CLMOD is Enabled bits
 *                   IFLTMOD (FCLCONx<15>) = 0: Normal Fault mode:
 *                   If current-limit is active, CLDAT<1> provides the state for PWMxH.
 *                   If current-limit is active, CLDAT<0> provides the state for PWMxL.
 *                   IFLTMOD (FCLCONx<15>) = 1: Independent Fault mode:
 *                   The CLDAT<1:0> bits are ignored.
 * bit 1 SWAP = 0.  SWAP PWMxH and PWMxL Pins bit
 *                   1 = PWMxH output signal is connected to PWMxL pin; PWMxL output signal is connected to PWMxH
pin
 *                   0 = PWMxH and PWMxL output signals pins are mapped to their respective pins
 * bit 0 OSYNC = 0.  Output Override Synchronization bit
 *                   1 = Output overrides via the OVRDAT<1:0> bits are synchronized to the PWM time base
 *                   0 = Output overrides via the OVRDAT<1:0> bits occur on next CPU clock boundary
 */
#define IOCONx_CFG 0x3C00u

/** FCLCONx
 * bit 15 IFLTMOD = 0.  Independent Fault Mode Enable bit(4)
 *                   1 = Independent Fault mode: Current-limit input maps FLTDAT<1> to PWMxH output, and fault input
 *                       maps FLTDAT<0> to PWMxL output. The CLDAT<1:0> bits are not used for override functions.
 *                   0 = Normal Fault mode: Current-limit and fault inputs map CLDAT<1:0> and FLTDAT<1:0> to PWMxH
 *                       and PWMxL outputs.
 * bit 14-10 CLSRC<4:0> = 00000.  Current-Limit Control Signal Source Select bits for PWM Generator #(2,3)
 *                   These bits specify the current-limit control signal source. Refer to the ?High-Speed PWM? chapter of
 *                   the specific device data sheet for available selections.
 * bit 9 CLPOL = 0.  Current-Limit Polarity bit for PWM Generator #(1)
 *                   1 = The selected current-limit source is active-low
 *                   0 = The selected current-limit source is active-high
 * bit 8 CLMOD = 0.  Current-Limit Mode Enable bit for PWM Generator #
 *                   1 = Current-Limit mode is enabled
 *                   0 = Current-Limit mode is disabled
 * bit 7-3 FLTSRC<4:0> = 00000.  Fault Control Signal Source Select bits for PWM Generator #(2,3)
 *                   These bits specify the Fault control source. Refer to the ?High-Speed PWM? chapter of the specific
 *                   device data sheet for available selections.
 * bit 2 FLTPOL = 0.  Fault Polarity bit for PWM Generator #(1)
 *                   1 = The selected fault source is active-low
 *                   0 = The selected fault source is active-high
 * bit 1-0 FLTMOD<1:0> = 11.  Fault Mode bits for PWM Generator #(4)
 *                   11 = Fault input is disabled
 *                   10 = Reserved
 *                   01 = The selected fault source forces PWMxH, PWMxL pins to FLTDAT values (cycle)
 *                   00 = The selected fault source forces PWMxH, PWMxL pins to FLTDAT values (latched condition)
 */
#define FCLCONx_CFG 0x0003u

/** TRIGx: PWM PRIMARY TRIGGER COMPARE VALUE REGISTER */
#define TRIGx_CFG 0x0000u

/** LEBCONx: LEADING-EDGE BLANKING CONTROL REGISTER */
#define LEBCONx_CFG 0x0000u

/** LEBDLYx: LEADING-EDGE BLANKING DELAY REGISTER */
#define LEBDLYx_CFG 0x0000u

/** AUXCONx: PWM AUXILIARY CONTROL REGISTER */
#define AUXCONx_CFG 0x0000u

#endif	/* CONFIG_PWM_H */

