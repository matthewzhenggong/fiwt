/* 
 * File:   config_servo.h
 * Author: Matt
 *
 * Created on April 8, 2015, 9:50 AM
 */

#ifndef CONFIG_SERVO_H
#define	CONFIG_SERVO_H

#if AC_MODEL
#define SERVO_ACCEL_LIMIT (30)
//#define SERVO_KP (10)
//#define SERVO_SP (3)
//#define SERVO_KD (11)
//#define SERVO_SD (0)
//#define SERVO_DIFF_LMT  (4681) //((2^15)/(SERVO_K+1))
//#define SERVO_SHAKE (455)
#define SERVO_SHAKE_TICKS (50)
#define SERVO_SHAKE_DZ (1)
#define SERVO_SHAKE_RDZ (1)
#elif AEROCOMP
#define SERVO_ACCEL_LIMIT (35)
//#define SERVO_KP (19)
//#define SERVO_SP (3)
//#define SERVO_KD (20)
//#define SERVO_SD (0)
//#define SERVO_DIFF_LMT (1724) //((2^15)/(SERVO_K+1))
//#define SERVO_SHAKE (343)
#define SERVO_SHAKE_TICKS (20)
#define SERVO_SHAKE_DZ (1)
#define SERVO_SHAKE_RDZ (1)
#endif

#endif	/* CONFIG_SERVO_H */

