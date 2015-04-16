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

// Data masking, MASK_BYTE definition
#define MASK_BYTE       		0xEF

/* Data Identifier Codes */
// Servos Read Data
#define CODE_AC_MODEL_SERVO_POS		0x22
#define CODE_AEROCOMP_SERVO_POS		0x33

// Encoders Read Data
#define CODE_AC_MODEL_ENCOD_POS 	0x55
#define CODE_AEROCOMP_ENCOD_POS 	0x66

// Battery Read Data
#define CODE_AC_MODEL_BAT_LEV		0x88
#define CODE_AEROCOMP_BAT_LEV		0x99

// Low Battery codes
#define CODE_AC_MODEL_LOW_BAT		0xBB
#define CODE_AEROCOMP_LOW_BAT	 	0xCC

// IMU Read Data
#define CODE_AC_MODEL_IMU_DATA		0xEE

// Communication Statistics
#define CODE_AC_MODEL_COM_STATS		0x77
#define CODE_AEROCOMP_COM_STATS		0x78

// GNDBOARD Sensors Read
#define CODE_GNDBOARD_ADCM_READ		0x44
#define CODE_GNDBOARD_ENCOD_POS 	0x77
#define CODE_GNDBOARD_COM_STATS		0xAA

// Servos New Position
#define CODE_AC_MODEL_NEW_SERV_CMD	0xA5
#define CODE_AEROCOMP_NEW_SERV_CMD	0xA6

// Command Message
#define CODE_AC_MODEL_COMMAND		0x45
#define CODE_AEROCOMP_COMMAND		0x46
#define CODE_GNDBOARD_COMMAND		0x47

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_CODE_H */

