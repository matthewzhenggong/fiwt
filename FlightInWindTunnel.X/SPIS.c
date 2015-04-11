/*
 * File:   SPIS.c
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


#include "SPIS.h"

#if USE_SPIS

#include <xc.h>
#include <string.h>

// GNDBOARD / dSPACE board SPI interface codes
#define DUMMY_DATA      '\x56'    // 0x12 -> "V": If connected to IMU-ADIS16360,
// the Product identification is retrieved.
#define SPI_DELIMITER   '\x7E'    // 0x7E -> "~": Message start delimiter
#define SPI_ESCAPE      '\x7D'    // 0x7D -> "}": Following data is ESCAPE
#define CONTINUE_TX     '\x11'    // 0x11 -> "Device Control 1": Continue Tx
#define CONTINUE_RX     '\x13'    // 0x13 -> "Device Control 3": Continue Rx
#define MESSAGE_END     '\x45'    // 0x45 -> "E": Message end delimiter
#define SPI_COMMSTEST   '\x43'    // 0x43 -> "C": Communications test
#define NEW_DATA        '\x3F'    // 0x3F -> "?": New Data?
#define DATA_RDY        '\x59'    // 0x59 -> "Y": Yes, Data ready
#define DATA_NRDY       '\x4E'    // 0x4E -> "N": No, Data NOT ready
#define NoF_PACKS       '\x50'    // 0x50 -> "P": Number of Packages
#define NoB_FRAME       '\x23'    // 0x23 -> "#": Number of Bytes in Frame
#define PRCS_STP        '\x53'    // 0x53 -> "S": Stop, Process stop

static const char SPI_Message[] = " Message from: dsPIC33EP MCU. If you can readthis, Comm workHave a nice day BYE ...";

__near uint8_t volatile SPI1RX_Data;
__near int volatile SPI1RX_Flow;
__near unsigned char volatile SPI1RX_DATA_ID;
__near uint16_t volatile SPI1RX_Bytes2RX;
__near uint16_t volatile SPI1RX_Bytes4RX;
__near uint8_t volatile SPI1RX_Checksum;
__near bool volatile SPI1RX_EscapedChar;
__near int volatile SPI1TX_Flow;
__near uint8_t volatile SPI1TX_Data;
__near uint8_t volatile SPI1TX_Bytes4TX;
__near uint8_t volatile SPI1TX_Bytes2TX;
uint8_t SPI1TX_Bytes[256];
__near int volatile Msg_idx;

static DATA_PCKT_Frame SPIRX_RX_PCKT[2];
__near DATA_PCKT_Frame_Ptr SPIRX_RX_PCKT_PCKT;
__near DATA_PCKT_Frame_Ptr SPIRX_RX_PCKT_PTR;


// Transmission Handling Queue struct data type definition
#define SPITX_MaxSize (10)
#define SPITX_Buffer_MaxSize (1700)
typedef struct QueueSPITX_{
        uint8_t Heads[SPITX_MaxSize];
	uint8_t Payloads[SPITX_Buffer_MaxSize];
	size_t Head_idx;
	size_t Payload_idx;
	size_t Tail_idx;
	size_t Payload_tail_idx;
	size_t Elem_count;
	size_t used_count;
}SPITX, *SPITX_Ptr;

SPITX SPI_TXHQ;

void QueueSPITX_Init(SPITX_Ptr p) {
    p->Elem_count = 0u;
    p->Head_idx = 0u;
    p->Payload_idx = 0u;
    p->Tail_idx = 0u;
    p->Payload_tail_idx = 0u;
    p->used_count = 0u;
}

size_t QueueSPITX_pop(SPITX_Ptr p, unsigned char *Payload){
    size_t length;
    size_t i;
    if (p->Elem_count > 0u) {
        --p->Elem_count;
        length = p->Heads[p->Head_idx];
        p->used_count -= length;
        if (++p->Head_idx >= SPITX_MaxSize) {
            p->Head_idx = 0u;
        }
        if (p->Payload_idx + length > SPITX_Buffer_MaxSize) {
            i = SPITX_Buffer_MaxSize - p->Payload_idx;
            memcpy(Payload, p->Payloads+p->Payload_idx, i);
            Payload += i;
            i = length - i;
            memcpy(Payload, p->Payloads, i);
            p->Payload_idx = i;
        } else {
          memcpy(Payload, p->Payloads+p->Payload_idx, length);
          p->Payload_idx += length;
          if (p->Payload_idx >= SPITX_Buffer_MaxSize) {
              p->Payload_idx = 0u;
          }
        }
        return length;
    } else {
        return 0u;
    }
}

bool QueueSPITX_push(SPITX_Ptr p, const uint8_t *Payload, size_t length){
    size_t i;

    if (length < 1u || p->used_count + length > SPITX_Buffer_MaxSize || p->Elem_count >= SPITX_MaxSize) {
        return false;
    } else {
        ++p->Elem_count;
        p->used_count += length;
        p->Heads[p->Tail_idx] = length;
        if (++p->Tail_idx >= SPITX_MaxSize) {
            p->Tail_idx = 0u;
        }
        if (p->Payload_tail_idx + length > SPITX_Buffer_MaxSize) {
            i = SPITX_Buffer_MaxSize - p->Payload_tail_idx;
            memcpy(p->Payloads+p->Payload_tail_idx, Payload, i);
            Payload += i;
            i = length - i;
            memcpy(p->Payloads, Payload, i);
            p->Payload_tail_idx = i;
        } else {
          memcpy(p->Payloads+p->Payload_tail_idx, Payload, length);
          p->Payload_tail_idx += length;
          if (p->Payload_tail_idx >= SPITX_Buffer_MaxSize) {
              p->Payload_tail_idx = 0u;
          }
        }
        return true;
    }
}

bool SPIS_push(const uint8_t *Payload, size_t length){
    return QueueSPITX_push(&SPI_TXHQ, Payload, length);
}

void SPISInit(void) {
    _SPI1IE = 0b0; /* SPI1 Interrupt disabled */
    _SPI1IP = HARDWARE_INTERRUPT_PRIORITY_LEVEL_MED; /*  SPI1 priority out 6 of 7 */
    _SPI1IF = 0b0; /* SPI1 Interrupt flag cleared */

    /* 1) Configure pin SDO1 as output and pins SCK1, SS1 and SDI1 as inputs*/
    TRISFbits.TRISF5 = 0b0; // SDO1 is assigned to RF5
    TRISFbits.TRISF3 = 0b1; // SCK1 is assigned to RF3
    TRISFbits.TRISF8 = 0b1; // SS1 is assigned to RF8
    TRISFbits.TRISF2 = 0b1; // SDI1 is assigned to RF2

    /* 2) Assign SPI1 pins through Peripherial Pin Select */
    RPOR9bits.RP101R = 0x05; // SDO1 is assigned to RP101,   0x05 = SDO1
    RPINR20bits.SCK1R = 0x63; // SCK1 is assigned to RP99,    0x63 = RP99
    RPINR21bits.SS1R = 0x68; // SS1 is assigned to RP104,    0x68 = RP104
    RPINR20bits.SDI1R = 0x62; // SDI1 Input tied to RP98,     0x62 = RP98

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
    SPI1CON1bits.MSTEN = 0b0; /*  Master Mode Enable bit: */
    /*                                          0b1 = Master mode. */
    /*                                          0b0 = Slave mode. */
    SPI1CON1bits.MODE16 = 0b0; /*  Word/Byte Communication Select bit: */
    /*                                          0b1 = Communication is word-wide (16 bits). */
    /*                                          0b0 = Communication is byte-wide (8 bits). */
    SPI1CON1bits.SMP = 0b0; /*  SPI1 Data Input Sample Phase bit: */
    /*                                          Master mode: */
    /*                                          0b1 = Input data is sampled at end of data output time. */
    /*                                          0b0 = Input data is sampled at middle of data output time. */
    /*                                          Slave mode: The SMPbit must be cleared when the SPI1 module is used in */
    /*                                          Slave mode. */
    SPI1CON1bits.SSEN = 0b1; /* Slave Select Enable bit (Slave mode): */
    /*                                           0b1 = SS1 pin is used for Slave mode. */
    /*                                           0b0 = SS1 pin is not used by the module. Pin is controlled by port */
    /*                                                 function. */
    SPI1CON1bits.CKE = 0b1; /*  SPI1 Clock Edge Select bit: */
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

}


void SPISStart(void) {
    SPI1RX_Data = 0;
    SPI1RX_Flow = 0;
    SPI1RX_DATA_ID = 0;
    SPI1RX_Data = 0;
    SPI1RX_Bytes2RX = 0;
    SPI1RX_Bytes4RX = 0;
    SPI1RX_Checksum = 0;
    SPI1RX_EscapedChar = false;
    SPI1TX_Flow = 0;
    SPI1TX_Data = 0;
    SPI1TX_Bytes4TX = 0;
    SPI1TX_Bytes2TX = 0;
    Msg_idx = 0;
    
    SPIRX_RX_PCKT_PCKT = SPIRX_RX_PCKT;
    SPIRX_RX_PCKT_PTR = NULL;

    QueueSPITX_Init(&SPI_TXHQ);

    SPI1BUF = 0;
    _SPI1IF = 0b0; /* SPI1 Interrupt flag cleared */
    _SPI1IE = 0b1; /* SPI1 Interrupt enabled */

    SPI1STATbits.SPIROV = 0b0;
    /*  Turn on the SP1 module */
    SPI1STATbits.SPIEN = 0b1; /*  SPI1 Enable bit: */
    /*                                  0b1 = Enables the module and configures SCK1, SDO1 and SDI1 as */
    /*                                  serial port pins. */
    /*                                  0b0 = Disables the module. */
}

__interrupt(auto_psv) void _SPI1Interrupt(void) {

    // Read SPI1 Serial Receive Buffer
    SPI1RX_Data = SPI1BUF;

    // Verify the received byte store in SPI1RX_Data
    switch (SPI1RX_Data) {
        case NoF_PACKS:
            // A NoPACKS byte has been received.
            // Send the number of Frames/Packages that are ready to be TX in the SPI_TXQ.

            // Extract NoPACKS from SPI_TXHQ
            SPI1TX_Data = SPI_TXHQ.Elem_count;
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;
            // Set SPI1RX_Flow = 1
            SPI1RX_Flow = 1;
            break;
        case NoB_FRAME:
            if (SPI1RX_Flow == 1) {
                // A NoB_FRAME byte has been received, and a NoPACKS byte has been
                // sent previously.
                // Send the number of bytes in the Frame/Package to be TX.

                // Extract Bytes4TX from SPI_TXHQ
                if (SPI_TXHQ.Elem_count) {
                   SPI1TX_Bytes4TX = SPI_TXHQ.Heads[SPI_TXHQ.Head_idx];
                } else {
                    SPI1TX_Bytes4TX = 0u;
                }
                // Store SPI1TX_Bytes4TX in SPI1TX_Data
                SPI1TX_Data = SPI1TX_Bytes4TX;
                // Send SPI1TX_Data
                SPI1BUF = SPI1TX_Data;
                // Set SPI1RX_Flow = 2
                SPI1RX_Flow = 2;
            } else {
                // Send PRCS_STP
                SPI1TX_Data = PRCS_STP;
                // Send SPI1TX_Data
                SPI1BUF = SPI1TX_Data;
                // Set SPI1RX_Flow = 0
                SPI1RX_Flow = 0;
            }
            break;
        case NEW_DATA:
            if (SPI1RX_Flow == 2) {
                // Verify if there is data in SPI_TXHQ.
                SPI1TX_Bytes4TX = QueueSPITX_pop(&SPI_TXHQ, SPI1TX_Bytes);
                if (SPI1TX_Bytes4TX == 0u) {
                    // There ISN'T data in SPI_TXHQ, send DATA_NRDY
                    SPI1TX_Data = DATA_NRDY;
                    // Set SPI1RX_Flow = 4
                    SPI1RX_Flow = 4;
                } else {
                    // There IS data in SPI_TXHQ.
                        // Data HAS been fully transfered, SPI_TXQ IS NOT empty,
                        // the current package IS NOT empty.
                        // Send DATA_RDY
                        SPI1TX_Data = DATA_RDY;
                        SPI1TX_Bytes2TX = 0u;
                        // Set SPI1RX_Flow = 3
                        SPI1RX_Flow = 3;
                }
                // Send SPI1TX_Data
                SPI1BUF = SPI1TX_Data;
            } else {
                // Send PRCS_STP
                SPI1TX_Data = PRCS_STP;
                // Send SPI1TX_Data
                SPI1BUF = SPI1TX_Data;
                // Set SPI1RX_Flow = 0
                SPI1RX_Flow = 0;
            }
            break;
        case SPI_ESCAPE:
            // Send DUMMY_DATA
            SPI1TX_Data = DUMMY_DATA;
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;
            // Set EscapedChar control byte
            SPI1RX_EscapedChar = true;
            break;
        case SPI_COMMSTEST:
            if (SPI1RX_Flow != SPI_COMMSTEST) {
                Msg_idx = 0;
            }
            // Reply to SPI_COMMSTEST with SPI_Message[Msg_idx];
            SPI1TX_Data = SPI_Message[Msg_idx];
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;
            // Increase Msg_idx counter
            if (++Msg_idx >= 83) {
                Msg_idx = 0;
            }
            // Set SPI1RX_Flow = SPI_COMMSTEST
            SPI1RX_Flow = SPI_COMMSTEST;
            break;
        case DUMMY_DATA:
            SPI1TX_Data = DUMMY_DATA;
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;
            break;
        case CONTINUE_TX:
            // Data will be transmitted only.
            // Verify if the transmission process was started.
            if (SPI1RX_Flow == 3) {
                // Data will be TX only.

                // A BEGIN_TX byte has been received after:
                // a) A NoF_PACKS was received, and the number of packages was sent as a reply,
                //      then a NoB_FRAME was received, and the number of byte in the frame was sent as a reply,
                //      then a NEW_DATA was, and a DATA_RDY byte was sent as a reply.
                // b) A previous BEGIN_TX has been received, and TX process is ongoing.

                // Discard received byte, and send corresponding DATA byte back.
                if (SPI1TX_Bytes2TX < SPI1TX_Bytes4TX) {
                    // Extract DATA byte from SPI_TXQ
                    SPI1TX_Data = SPI1TX_Bytes[SPI1TX_Bytes2TX];
                    // Send SPI1TX_Data
                    SPI1BUF = SPI1TX_Data;
                    // Transmission will continue.
                    ++SPI1TX_Bytes2TX;
                }
                if (SPI1TX_Bytes2TX == SPI1TX_Bytes4TX) {
                    // Set SPI1RX_Flow = 0
                    SPI1RX_Flow = 0;
                    // Transmission will stop.
                }
            } else {
                // Send PRCS_STP
                SPI1TX_Data = PRCS_STP;
                // Send SPI1TX_Data
                SPI1BUF = SPI1TX_Data;
                // Set SPI1RX_Flow = 0
                SPI1RX_Flow = 0;
            }
            break;
        case SPI_DELIMITER:
            // Store SPI_DELIMITER in SPI1TX_Data
            SPI1TX_Data = SPI_DELIMITER;
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;

            // Initialize Checksum
            SPI1RX_Checksum = 0x00u;
            SPI1RX_Bytes4RX = 0u;
            // Set SPI1TX_Flow = 1
            SPI1TX_Flow = 1;
            break;
        case MESSAGE_END:
            // Store MESSAGE_END in SPI1TX_Data
            SPI1TX_Data = MESSAGE_END;
            // Send SPI1TX_Data
            SPI1BUF = SPI1TX_Data;

            SPI1TX_Flow = 0;
            break;
        default:
                if (SPI1RX_EscapedChar) // If previous Char was ESCAPE
                {
                    // Apply a XOR with 0x20 to the incoming Char,
                    // and store byte in SPI1RX_Data
                    SPI1RX_Data ^= 0x20;
                    // Reset EscapedChar
                    SPI1RX_EscapedChar = false;
                }
                switch (SPI1TX_Flow) {
                    case 1:
                        // Send CONTINUE_RX
                        SPI1TX_Data = CONTINUE_RX;
                        // Send SPI1TX_Data
                        SPI1BUF = SPI1TX_Data;
                        // Store SPI1RX_Data in higher byte of SPI1RX_Bytes2RX
                        SPI1RX_Bytes2RX = SPI1RX_Data;
                        SPI1RX_Bytes2RX <<= 8;
                        // Store SPI1RX_Data in SPIRX_RX_PCKT_PCKT.PCKT_LENGTH_MSB
                        SPIRX_RX_PCKT_PCKT->PCKT_LENGTH_MSB = SPI1RX_Data;
                        // Set SPI1TX_Flow = 2
                        SPI1TX_Flow = 2;
                        break;
                    case 2:
                        // Store SPI1RX_Data in lower byte of SPI1RX_Bytes2RX
                        SPI1RX_Bytes2RX |= SPI1RX_Data;
                        if (SPI1RX_Bytes2RX > MaximumPayload) {
                            // Send PRCS_STP
                            SPI1TX_Data = PRCS_STP;
                            // Send SPI1TX_Data
                            SPI1BUF = SPI1TX_Data;
                            SPI1TX_Flow = 0;
                        } else {
                            // Send CONTINUE_RX
                            SPI1TX_Data = CONTINUE_RX;
                            // Send SPI1TX_Data
                            SPI1BUF = SPI1TX_Data;
                            // Store SPI1RX_Data in SPIRX_RX_PCKT_PCKT.PCKT_LENGTH_LSB
                            SPIRX_RX_PCKT_PCKT->PCKT_LENGTH_LSB = SPI1RX_Data;
                            // Set SPI1TX_Flow = 3
                            SPI1TX_Flow = 3;                            
                        }
                        break;
                    case 3:
                        // Send CONTINUE_RX
                        SPI1TX_Data = CONTINUE_RX;
                        // Send SPI1TX_Data
                        SPI1BUF = SPI1TX_Data;
                        // Store SPI1RX_Data in SPIRX_RX_PCKT_PCKT.RF_DATA[i]
                        SPIRX_RX_PCKT_PCKT->RF_DATA[SPI1RX_Bytes4RX] = SPI1RX_Data;
                        // Update Checksum
                        SPI1RX_Checksum += SPI1RX_Data;

                        // Verify that there is data left to be received
                        if (++SPI1RX_Bytes4RX >= SPI1RX_Bytes2RX) {
                            // This was the last byte to be received.
                            // Set SPI1TX_Flow = 4
                            SPI1TX_Flow = 4;
                        }
                        break;
                    case 4:
                        // Send CONTINUE_RX
                        SPI1TX_Data = CONTINUE_RX;
                        // Send SPI1TX_Data
                        SPI1BUF = SPI1TX_Data;

                        // Compute final value of SPI1RX_Checksum
                        SPI1RX_Checksum = 0xFF - SPI1RX_Checksum;
                        // Verify that the received data is valid by comparing
                        // SPIRX_RX_PCKT_PCKT.PCKT_CHECKSUM with SPI1RX_Checksum.
                        if (SPI1RX_Data == SPI1RX_Checksum) {
                            // A Valid Package has been received.
                            // Set SPI1RX_ValidPackage
                            SPIRX_RX_PCKT_PTR = SPIRX_RX_PCKT_PCKT;
                            if (SPIRX_RX_PCKT_PCKT == SPIRX_RX_PCKT) {
                                SPIRX_RX_PCKT_PCKT = SPIRX_RX_PCKT+1u;
                            } else {
                                SPIRX_RX_PCKT_PCKT = SPIRX_RX_PCKT;
                            }
                        }
                        // Set SPI1TX_Flow = 0
                        SPI1TX_Flow = 0;
                        break;
                    default :
                        // Send PRCS_STP
                        SPI1TX_Data = PRCS_STP;
                        // Send SPI1TX_Data
                        SPI1BUF = SPI1TX_Data;
                        // Set SPI1TX_Flow = 0
                        SPI1TX_Flow = 0;
                }
    }

    _SPI1IF = 0; /* clear interrupt flag */
}


#endif /* USE_SPIS */




