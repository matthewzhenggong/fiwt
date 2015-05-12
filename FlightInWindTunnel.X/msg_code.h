/* 
 * File:   msg_code.h
 * Author: Matt
 *
 * Created on April 16, 2015, 9:05 PM
 */

#ifndef MSG_CODE_H
#define	MSG_CODE_H

#ifdef	__cplusplus
extern "C" {
#endif

/* Data Identifier Codes */
// Servos Read Data
#define CODE_AC_MODEL_SERVO_POS		0x22
#define CODE_AEROCOMP_SERVO_POS		0x33
#define CODE_GNDBOARD_ADCM_READ		0x44
#define CODE_GNDBOARD_MANI_READ		0x45
#define CODE_GNDBOARD_MANI_RAW_READ	0x46

// State Statistics
#define CODE_GNDBOARD_STATS		0x76
#define CODE_AC_MODEL_STATS		0x77
#define CODE_AEROCOMP_STATS		0x78

// Servos New Position
#define CODE_AC_MODEL_SERV_CMD	0xA5
#define CODE_AEROCOMP_SERV_CMD	0xA6

//NTP
#define CODE_NTP_REQUEST		0x01
#define CODE_NTP_RESPONSE		0x02

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_CODE_H */

