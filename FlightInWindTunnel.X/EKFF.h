/* 
 * File:   EKFF.h
 * Author: Matt
 *
 * Created on April 18, 2015, 7:10 PM
 */

#ifndef EKFF_H
#define	EKFF_H

#include "config.h"

#if USE_EKF

/*
 * the order of Kalman Filter
 */
#define DIMNUM (15u)
#define DIMNUM2 (DIMNUM*DIMNUM)

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct EKFF {
        /*
         * Kalman Filter states
         * in the order of
         * px py pz vx vy vz a b c d bax bay baz bwx bwy bwz
         * include :
         * px py pz position in NED axises(m)
         * vx vy vz velocity in NED axises(m/s)
         * a b c d  attitude in Quaternion
         * bax bay baz biases of accelerometer
         * bwx bwy bwz biases of gyro
         */
        float xEKF[DIMNUM + 1];

        /*
         * Kalman Filter state derivatives
         */
        float dxEKF[DIMNUM + 1];

        /*
         * attitude vector in rad
         * include Roll, Pitch, Yaw respectively
         */
        float rpy[3];

        /*
         * Euler orientation cosine matrix
         * from body axies to NED axies
         */
        float cbn11, cbn12, cbn13, cbn21, cbn22, cbn23, cbn31, cbn32, cbn33;

        /*
         * roll rates in body axies(rad/s)
         */
        float p, q, r;

        /*
         * measured accelerations in body axies(m/s2)
         */
        float ax, ay, az;

        /*
         * accelerations in NED axies(m/s2)
         * gravity excluded
         */
        float fN, fE, fD;

        /*
         * Jaccobi Matrix
         */
        float dfdx[DIMNUM2];

        /*
         * CONV Matrix of state matrix
         */
        float sigkf[DIMNUM2];

        /*
         * delta time for one step
         */
        float dt;


        /*
         * conv matrix for input noise (diagonal matrix)
         */
        float Q[DIMNUM];

        /*
         * conv matrix for estimate values (diagonal matrix)
         */
        float R[3];
        float Rpos[3];
        float Rvel[3];
        float Rcmp;


    } EKFF_t, *EKFF_p;

    enum OP_TYPE {
        INITIALIZE=0,
        UPDATEPOS,
        UPDATEVEL,
        UPDATECMP
    };

    void EKF_Filter(EKFF_p ekf, enum OP_TYPE op, float y[], float pqr[3], float accbi[3]);

#ifdef	__cplusplus
}
#endif

#endif

#endif	/* EKFF_H */

