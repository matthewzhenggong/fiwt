
#include "config.h"
#include "Servo.h"
#include "PWMx.h"
#include "AnalogInput.h"
#include <xc.h>

    /* Servos pin layout */
    /************************************************************************
    System      Servo       Analog Input    PMW Output  Directioning Pins
                                                            (IN_B / IN_A)
    -------------------------------------------------------------------------
                Servo1		AN20        PMW1	LATJ4 	/ LATJ5
                Servo2		AN21        PMW2	LATJ6 	/ LATJ7
    AC_MODEL    Servo3		AN5         PMW3	LATG14	/ LATG12
                Servo4		AN3         PMW7	LATJ10	/ LATJ11
                Servo5		AN2         PMW5	LATJ12	/ LATJ13
                Servo6		AN1         PMW6	LATG6 	/ LATG7
    -------------------------------------------------------------------------
                Servo1		AN20        PMW1	LATJ4 	/ LATJ5
    AEROCOMP    Servo2		AN21        PMW2	LATJ6 	/ LATJ7
                Servo3		AN3         PMW7	LATJ10	/ LATJ11
                Servo4		AN2         PMW5	LATJ12	/ LATJ13
    ************************************************************************/

        unsigned int *Position;
        unsigned int PrevPosition;
        unsigned int Reference;
        signed int Ctrl;
        unsigned int *DutyCycle;
        unsigned int *lat_cw;
        unsigned int lat_cw_pos;
        unsigned int lat_cw_mask;
        unsigned int *lat_ccw;
        unsigned int lat_ccw_pos;
        unsigned int lat_ccw_mask;
        unsigned int Enabled;

Servo_t Servos[] = {
    {&ServoPos[0], 2048, 2048, 2048, &PWM1DC, &LATG, _LATG_LATG14_POSITION, _LATG_LATG14_MASK, &LATG, _LATG_LATG12_POSITION, _LATG_LATG12_MASK},
    {&ServoPos[0], 2048, 2048, 2048, &PWM1DC, &LATG, _LATG_LATG14_POSITION, _LATG_LATG14_MASK, &LATG, _LATG_LATG12_POSITION, _LATG_LATG12_MASK},
};

void ServoInit(void) {

}

void ServoStart(void) {

}



