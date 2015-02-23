/*
 * File:   IMU.c
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Modified code from : Sergio AraujoEstrada <S.AraujoEstrada@bristol.ac.uk>
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


#include "IMU.h"

#if USE_IMU

unsigned int IMU_Supply;
unsigned int IMU_XGyro;
unsigned int IMU_YGyro;
unsigned int IMU_ZGyro;
unsigned int IMU_XAccl;
unsigned int IMU_YAccl;
unsigned int IMU_ZAccl;
unsigned int IMU_XGyroTemp;
unsigned int IMU_YGyroTemp;
unsigned int IMU_ZGyroTemp;
unsigned int IMU_AUX_ADC;
clockType_t IMU_TimeStamp;

#define SS1 (LATFbits.LATF5)

void IMUInit(void) {
    /* 1) Configure SDO1,SCK1, SS1 pins as outputs and SDI1 pin as input*/
    TRISDbits.TRISD15 = 0b0;
    TRISFbits.TRISF4  = 0b0;
    TRISFbits.TRISF5  = 0b0;
    TRISDbits.TRISD14 = 0b1;

    /* 2) Assign SPI1 pins through Peripherial Pin Select */
    RPOR4bits.RP79R = 0x05;    /*  SDO1 is assigned to RP79,    0x05 = SDO1 */
    RPOR9bits.RP100R = 0x06;   /*  SCK1 is assigned to RP100,   0x06 = SCK1 */
    //RPOR9bits.RP101R  = 0x07;  /* SS1 is assigned to RP101,     0x07 = SS1 */
    RPINR20bits.SDI1R = 0x4E;  /*  SDI1 Input tied to RPI78,    0x4E = RPI78 */

    /* 3) Configure the SPI module status and control register */
    /* SPI1STAT: SPI1 Status and Control Register*/
    SPI1STATbits.SPIEN = 0b0; /*  SPI1 Enable bit: */
    /*                                  0b1 = Enables the module and configures SCK1, SDO1 and SDI1 as */
    /*                                  serial port pins. */
    /*                                  0b0 = Disables the module. */
    SPI1STATbits.SPISIDL = 0b0; /*  SPISIDL: Stop in Idle Mode bit: */
    /*                                          0b1 = Discontinue the module operation when device enters Idle */
    /*                                          mode. */
    /*                                          0b0 = Continue the module operation in Idle mode. */
    SPI1STATbits.SPIBEC = 0b000; /*  SPI1 Buffer Element Count bits (valid in Enhanced Buffer mode): */
    /*                                          Master mode: Number of SPI1 transfers are pending. */
    /*                                          Slave mode: Number of SPI1 transfers are unread. */
    SPI1STATbits.SRMPT = 0b1; /*  Shift Register (SPI1SR) Empty bit (valid in Enhanced Buffer */
    /*                                          mode): */
    /*                                          0b1 = SPI1 Shift register is empty and ready to send or receive */
    /*                                          the data. */
    /*                                          0b0 = SPI1 Shift register is not empty. */
    SPI1STATbits.SPIROV = 0b0; /*  Receive Overflow Flag */
    /*                                          0b1 = A new byte/word is completely received and discarded. The user */
    /*                                          user application has not read the previous data in the SPI1BUF */
    /*                                          register. */
    /*                                          0b0 = No overflow has ocurred. */
    SPI1STATbits.SRXMPT = 0b1; /*  Receive FIFO Empty bit (valid in Enhanced Buffer mode): */
    /*                                          0b1 = RX FIFO is empty. */
    /*                                          0b0 = RX FIFO is not empty. */
    SPI1STATbits.SISEL = 0b001; /*  SPI1 Buffer Interrupt Mode bits (valid in Enhanced Buffer mode): */
    /*                                          0b111 = Interrupt when the SPI1 transmit buffer is full (SPI1TBF bit */
    /*                                          is set). */
    /*                                          0b110 = Interrupt when last bit is shifted into SPI1SR, and as a */
    /*                                          result, the TX FIFO is empty. */
    /*                                          0b101 = Interrupt when the last bit is shifted out of SPI1SR, and */
    /*                                          the transmit is complete. */
    /*                                          0b100 = Interrupt when one data is shifted into the SPI1SR, and as a */
    /*                                          result, the TX FIFO has one open memory location. */
    /*                                          0b011 = Interrupt when the SPI1 receive buffer is full (SPI1RBF bit */
    /*                                          set). */
    /*                                          0b010 = Interrupt when the SPI1 receive buffer is 3/4 or more full. */
    /*                                          0b001 = Interrupt when data is received in the receive buffer (SRMPT */
    /*                                          bit is set). */
    /*                                          0b000 = Interrupt when the last data in the receive buffer is read, */
    /*                                          as a result, the buffer is empty (SRXMPT bit set). */

    /* 4) Configure SPI module */
    /*  SPI1CON1: SPI1 Control Register 1 */
    SPI1CON1bits.MSTEN = 0b1; /*  Master Mode Enable bit: */
    /*                                          0b1 = Master mode. */
    /*                                          0b0 = Slave mode. */
    SPI1CON1bits.DISSCK = 0b0; /*  DISSCK: Disable SCK1 Pin bit (SPI Master modes only): */
    /*                                          0b1 = Internal SPI clock is disabled; pin functions as I/O. */
    /*                                          0b0 = Internal SPI clock is enabled. */
    SPI1CON1bits.MODE16 = 0b1; /*  Word/Byte Communication Select bit: */
    /*                                          0b1 = Communication is word-wide (16 bits). */
    /*                                          0b0 = Communication is byte-wide (8 bits). */
    SPI1CON1bits.SMP = 0b0; /*  SPI1 Data Input Sample Phase bit: */
    /*                                          Master mode: */
    /*                                          0b1 = Input data is sampled at end of data output time. */
    /*                                          0b0 = Input data is sampled at middle of data output time. */
    /*                                          Slave mode: The SMPbit must be cleared when the SPI1 module is used in */
    /*                                          Slave mode. */
    SPI1CON1bits.CKE = 0b0; /*  SPI1 Clock Edge Select bit: */
    /*                                          0b1 = Serial output data changes on transition from active clock state */
    /*                                          to Idle clock state (refer to bit CKP). */
    /*                                          0b0 = Serial output data changes on transition from Idle clock state to */
    /*                                          active clock state (refer to bit CKP). */
    SPI1CON1bits.CKP = 0b1; /*  Clock Polarity Select bit: */
    /*                                          0b1 = Idle state for clock is a high level; active state is a low level. */
    /*                                          0b0 = Idle state for clock is a low level; active state is a high level. */
    SPI1CON1bits.DISSDO = 0b0; /*  Disable SDO1 Pin bit: */
    /*                                          0b1 = SDO1 pin is not used by the module; pin functions as I/O. */
    /*                                          0b0 = SDO1 pin is controlled by the module. */

    /* 5) Configure SPI module clock frequency */
    /*  Working at 64MIPS, and with SPRE = 0b100 and PPRE = 0b01, */
    /*  SPI module clock frequency = 1MHz => 64MIPS/(16*4) */
    SPI1CON1bits.SPRE = 0b100; /*  Secondary Prescale bits (Master mode): */
    /*                                          0b111 = Reserved. */
    /*                                          0b110 = Secondary prescale 2:1. */
    /*                                          0b101 = Secondary prescale 3:1. */
    /*                                          0b100 = Secondary prescale 4:1. */
    /*                                          0b011 = Secondary prescale 5:1. */
    /*                                          0b010 = Secondary prescale 6:1. */
    /*                                          0b001 = Secondary prescale 7:1. */
    /*                                          0b000 = Secondary prescale 8:1. */
    SPI1CON1bits.PPRE = 0b01; /*  Primary Prescale bits (Master mode): */
    /*                                          0b11 = Reserved. */
    /*                                          0b10 = Primary prescale 4:1. */
    /*                                          0b01 = Primary prescale 16:1. */
    /*                                          0b00 = Primary prescale 64:1. */
}

void IMUStart(void) {
    /* 1) Turn on the SP1 module */
    SPI1STATbits.SPIEN = 0b1; /*  SPI1 Enable bit: */
    /*                                  0b1 = Enables the module and configures SCK1, SDO1 and SDI1 as */
    /*                                  serial port pins. */
    /*                                  0b0 = Disables the module. */

    /* 2) De-assert SS1 */
    SS1 = 1;

    /* 3) Wait 1 usecond for clock pulse to stabilize */
    asm ("repeat #64;"); Nop();
}

static unsigned int SPI_WriteWord(unsigned int SPI_word)
{
	// Transmit message
	SPI1BUF = SPI_word;
	// Wait for transmission to finish
	while(! IFS0bits.SPI1IF);
	// Clear SPI1 Interrupt flag
  	IFS0bits.SPI1IF = 0b0;
        asm ("repeat #64;"); Nop();
	// Read SPI data in buffer, this is discardable data
	return SPI1BUF;
}

void IMUUpdate(void) {
        /* Store reading time */
        IMU_TimeStamp = RTclock;

        SS1 = 0;
        IFS0bits.SPI1IF = 0b0;

	// Read IMU Data
	// Initialize IMU Burst Read
	SPI_WriteWord(0x3E00);

	// Read IMU Output Data Registers
	// Store IMU Voltage Supply Data
	IMU_Supply = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU X Gyro Data
	IMU_XGyro = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU Y Gyro Data
	IMU_YGyro = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU Z Gyro Data
	IMU_ZGyro = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU X Accl Data
	IMU_XAccl = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU Y Accl Data
	IMU_YAccl = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU Z Accl Data
	IMU_ZAccl = SPI_WriteWord(0) & 0x3FFF;
	// Store IMU X Gyro Temperature Data
	IMU_XGyroTemp = SPI_WriteWord(0) & 0x0FFF;
	// Store IMU Y Gyro Temperature Data
	IMU_YGyroTemp = SPI_WriteWord(0) & 0x0FFF;
	// Store IMU Z Gyro Temperature Data
	IMU_ZGyroTemp = SPI_WriteWord(0) & 0x0FFF;
	// Store IMU Auxiliary ADC Output Data
	IMU_AUX_ADC = SPI_WriteWord(0) & 0x0FFF;

        SS1 = 1;
}

#endif /* USE_IMU */


