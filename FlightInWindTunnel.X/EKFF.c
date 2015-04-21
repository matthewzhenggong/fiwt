/*
 * File:   EKFF.c
 * Author: Matt
 *
 * Created on April 18, 2015, 7:10 PM
 */

#include "config.h"

#if USE_EKF

#include "EKFF.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

/*
 * the postion order of Kalman Filter
 */
#define PPOS (3u)
/*
 * the velocity order of Kalman Filter
 */
#define PVEL (3u)
/*
 * the magnet order of Kalman Filter
 */
#define PCMP (1u)
/*
 * 重量加速度
 */
#define AccG0 (9.81)

/// 2pi
#define TWOPI (2.0*M_PI)

/*
 * Matrix Multiplier C=AxB
 * @param rowA row number of matrix A
 * @param colA column number of matrix A and row number of B
 * @param colB column number of matrix B
 */
void MultMat(float *A, float *B, float *C, unsigned rowA, unsigned colA, unsigned colB) {
    float *c;
    float *a;
    unsigned i, j, k;
    for (i = 0u; i < rowA; ++i) {
        for (j = 0u; j < colB; ++j) {
            c = C + i * colB + j;
            *c = 0.0;
            a = A + i*colA;
            for (k = 0u; k < colA; ++k) {
                *c += (*a++) * B[k * colB + j];
            }
        }
    }
}

/*
 * Matrix transferation B=A^{T}
 * @param rowA row number of matrix A
 * @param colA column number of matrix A
 */
void TransposeMatrix(float *A, float *B, unsigned rowA, unsigned colA) {
    unsigned i;
    for (i = 0u; i < rowA * colA; ++i) {
        *B++ = A[i / colA + (i % colA) * colA];
    }
}

/*
 * Euler angles to Quaternion
 */
void RPY2ABCD(EKFF_p ekf) {
    float u0, u1, u2, cr, cp, cy, sr, sp, sy;
    u0 = ekf->rpy[0]*0.5;
    u1 = ekf->rpy[1]*0.5;
    u2 = ekf->rpy[2]*0.5;
    cr = cos(u0);
    sr = sin(u0);
    cp = cos(u1);
    sp = sin(u1);
    cy = cos(u2);
    sy = sin(u2);
    ekf->xEKF[6] = cr * cp * cy + sr * sp*sy;
    ekf->xEKF[7] = sr * cp * cy - cr * sp*sy;
    ekf->xEKF[8] = cr * sp * cy + sr * cp*sy;
    ekf->xEKF[9] = cr * cp * sy - sr * sp*cy;
}

/*
 * Quaternion to Euler angles
 */
void ABCD2RPY(EKFF_p ekf) {
    float a, b, c, d, a2c2, b2d2, t1, t2;
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    a2c2 = a * a - c*c;
    b2d2 = b * b - d*d;
    ekf->rpy[0] = atan2(2.0 * (a * b + c * d), a2c2 - b2d2);
    t1 = 2.0L * (a * c - b * d);
    t2 = sqrt(1.0L - t1 * t1);
    ekf->rpy[1] = atan(t1 / t2);
    ekf->rpy[2] = atan2(2.0 * (a * d + b * c), a2c2 + b2d2);
}

/*
 * Quaternion to Euler orientation cosine matrix
 */
void ABCD2CBN(EKFF_p ekf) {
    float a, b, c, d, ab, ac, ad, bb, bc, bd, cc, cd, dd;
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    ab = a*b;
    ac = a*c;
    ad = a*d;
    bb = b*b;
    bc = b*c;
    bd = b*d;
    cc = c*c;
    cd = c*d;
    dd = d*d;
    ekf->cbn11 = 1.0 - 2.0 * (cc + dd);
    ekf->cbn12 = 2.0 * (bc - ad);
    ekf->cbn13 = 2.0 * (bd + ac);
    ekf->cbn21 = 2.0 * (bc + ad);
    ekf->cbn22 = 1.0 - 2.0 * (bb + dd);
    ekf->cbn23 = 2.0 * (cd - ab);
    ekf->cbn31 = 2.0 * (bd - ac);
    ekf->cbn32 = 2.0 * (cd + ab);
    ekf->cbn33 = 1.0 - 2.0 * (bb + cc);
}

/*
 * normalize Quaternion
 */
void NormalizeABCD(EKFF_p ekf) {
    float a, b, c, d, n;
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    n = 1.0 / sqrt(a * a + b * b + c * c + d * d);
    ekf->xEKF[6] = a*n;
    ekf->xEKF[7] = b*n;
    ekf->xEKF[8] = c*n;
    ekf->xEKF[9] = d*n;
}

void UpdateFilterState(EKFF_p ekf) {
    float a, b, c, d, da, db, dc;
    int i;

    for (i = 0; i < 6; ++i) {
        ekf->xEKF[i] += ekf->dxEKF[i];
    }
    da = ekf->dxEKF[6];
    db = ekf->dxEKF[7];
    dc = ekf->dxEKF[8];
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    ekf->xEKF[6] += 0.5 * (b * da + c * db + d * dc);
    ekf->xEKF[7] += 0.5 * (c * dc - a * da - d * db);
    ekf->xEKF[8] += 0.5 * (d * da - a * db - b * dc);
    ekf->xEKF[9] += 0.5 * (b * db - c * da - a * dc);
    NormalizeABCD(ekf);
    for (i = 10; i < 16; ++i) {
        ekf->xEKF[i] += ekf->dxEKF[i - 1];
    }
}

/*
 * Predict new States
 */
void PredictState(EKFF_p ekf) {
    float p0, q0, r0, a, b, c, d, dt;
    dt = ekf->dt;
    p0 = ekf->p - ekf->xEKF[13];
    q0 = ekf->q - ekf->xEKF[14];
    r0 = ekf->r - ekf->xEKF[15];
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    /// new position
    ekf->xEKF[0] += dt * ekf->xEKF[3];
    ekf->xEKF[1] += dt * ekf->xEKF[4];
    ekf->xEKF[2] += dt * ekf->xEKF[5];
    /// new velocity
    ekf->xEKF[3] = dt * ekf->fN;
    ekf->xEKF[4] = dt * ekf->fE;
    ekf->xEKF[5] = dt * (ekf->fD + AccG0);
    /// 姿态变化(四元素积分)
    ekf->xEKF[6] = a + dt * 0.5 * (-b * p0 - c * q0 - d * r0);
    ekf->xEKF[7] = b + dt * 0.5 * (a * p0 - d * q0 + c * r0);
    ekf->xEKF[8] = c + dt * 0.5 * (d * p0 + a * q0 - b * r0);
    ekf->xEKF[9] = d + dt * 0.5 * (-c * p0 + b * q0 + a * r0);
    /* xEKF[10] ... xEKF[15] unchanged */
}

/*
 * Output Jacobian Matrix DFDX to y
 */
void JacobianDFDX(EKFF_p ekf, float y[DIMNUM2]) {
    float ax0, ay0, az0;
    float dt;
    dt = ekf->dt;
    ax0 = ekf->ax - ekf->xEKF[10];
    ay0 = ekf->ay - ekf->xEKF[11];
    az0 = ekf->az - ekf->xEKF[12];
    ekf->fN = ekf->cbn11 * ax0 + ekf->cbn12 * ay0 + ekf->cbn13*az0;
    ekf->fE = ekf->cbn21 * ax0 + ekf->cbn22 * ay0 + ekf->cbn23*az0;
    ekf->fD = ekf->cbn31 * ax0 + ekf->cbn32 * ay0 + ekf->cbn33*az0;
    y[3] = dt;
    y[19] = dt;
    y[35] = dt;
    y[52] = -dt * ekf->fD;
    y[53] = dt * ekf->fE;
    y[54] = -dt * ekf->cbn11;
    y[55] = -dt * ekf->cbn12;
    y[56] = -dt * ekf->cbn13;
    y[66] = dt * ekf->fD;
    y[68] = -dt * ekf->fN;
    y[69] = -dt * ekf->cbn21;
    y[70] = -dt * ekf->cbn22;
    y[71] = -dt * ekf->cbn23;
    y[81] = -dt * ekf->fE;
    y[82] = dt * ekf->fN;
    y[84] = -dt * ekf->cbn31;
    y[85] = -dt * ekf->cbn32;
    y[86] = -dt * ekf->cbn33;
    y[102] = dt * ekf->cbn11;
    y[103] = dt * ekf->cbn12;
    y[104] = dt * ekf->cbn13;
    y[117] = dt * ekf->cbn21;
    y[118] = dt * ekf->cbn22;
    y[119] = dt * ekf->cbn23;
    y[132] = dt * ekf->cbn31;
    y[133] = dt * ekf->cbn32;
    y[134] = dt * ekf->cbn33;
}

/*
 * Calculate heading in rad based on Quaternion
 */
float OutputEquationH3(EKFF_p ekf) {
    float a, b, c, d, t1, t2;
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    t1 = 2.0 * (a * d + b * c);
    t2 = a * a + b * b - c * c - d*d;
    return atan2(t1, t2);
}

/*
 * Output Jacobian Matrix DH3DX to y
 */
void JacobianDH3DX(EKFF_p ekf, float y[3]) {
    float a, b, c, d, c11, c21, c31, tp, w;
    a = ekf->xEKF[6];
    b = ekf->xEKF[7];
    c = ekf->xEKF[8];
    d = ekf->xEKF[9];
    c31 = 2.0 * (b * d - a * c);
    c21 = 2.0 * (b * c + a * d);
    c11 = 1.0 - 2.0 * (c * c + d * d);
    tp = c31 / sqrt(1 - c31 * c31);
    w = sqrt(c21 * c21 + c11 * c11);
    y[0] = tp * c11 / w;
    y[1] = tp * c21 / w;
    y[2] = -1.0;
}

/*
 * Get invert matrix 3x3
 */
void Inverse3(float m[9], float minv[9]) {
    float invdet;
    invdet = 1.0 / (m[0] * m[4] * m[8] + m[1] * m[5] * m[6] + m[2] * m[3] * m[7] -
            m[2] * m[4] * m[6] - m[1] * m[3] * m[8] - m[0] * m[5] * m[7]);
    minv[0] = invdet * (m[4] * m[8] - m[5] * m[7]);
    minv[1] = invdet * (m[2] * m[7] - m[1] * m[8]);
    minv[2] = invdet * (m[1] * m[5] - m[2] * m[4]);
    minv[3] = invdet * (m[5] * m[6] - m[3] * m[8]);
    minv[4] = invdet * (m[0] * m[8] - m[2] * m[6]);
    minv[5] = invdet * (m[2] * m[3] - m[0] * m[5]);
    minv[6] = invdet * (m[3] * m[7] - m[4] * m[6]);
    minv[7] = invdet * (m[1] * m[6] - m[0] * m[7]);
    minv[8] = invdet * (m[0] * m[4] - m[1] * m[3]);
}

void UpdateSigmaRPos(EKFF_p ekf) {
    ekf->R[0] = ekf->Rpos[0];
    ekf->R[1] = ekf->Rpos[1];
    ekf->R[2] = ekf->Rpos[2];
}

void UpdateSigmaRVel(EKFF_p ekf) {
    ekf->R[0] = ekf->Rvel[0];
    ekf->R[1] = ekf->Rvel[1];
    ekf->R[2] = ekf->Rvel[2];
}

void Initialize(EKFF_p ekf, float y[16]) {
    int i;
    /* set initial states */
    for (i = 0; i < 6; ++i) ekf->xEKF[i] = y[i];

    ekf->rpy[0] = y[6];
    ekf->rpy[1] = y[7];
    ekf->rpy[2] = y[8];
    RPY2ABCD(ekf);

    for (i = 10; i < 16; ++i) ekf->xEKF[i] = y[i - 1];

    /* set initial covariance matrix */
    for (i = 0; i < DIMNUM2; ++i) ekf->sigkf[i] = 0.0;
    ekf->sigkf[0] = 25.0;
    ekf->sigkf[16] = 25.0;
    ekf->sigkf[32] = 25.0;
    ekf->sigkf[48] = 4.0;
    ekf->sigkf[64] = 4.0;
    ekf->sigkf[80] = 4.0;
    ekf->sigkf[96] = 0.2;
    ekf->sigkf[112] = 0.2;
    ekf->sigkf[128] = 0.5;
    ekf->sigkf[144] = 0.05;
    ekf->sigkf[160] = 0.05;
    ekf->sigkf[176] = 0.01;
    ekf->sigkf[192] = 0.01;
    ekf->sigkf[208] = 0.01;
    ekf->sigkf[224] = 0.01;

    /* set all jacobian matrices equal zero */
    for (i = 0; i < DIMNUM2; ++i) ekf->dfdx[i] = 0.0;

    /* process noise covariances */
    ekf->Q[0] = 0.5;
    ekf->Q[1] = 0.5;
    ekf->Q[2] = 0.5; /* position */
    ekf->Q[3] = 0.05;
    ekf->Q[4] = 0.05;
    ekf->Q[5] = 0.05; /* velocity, 0.01 */
    ekf->Q[6] = 3.0E-3;
    ekf->Q[7] = 3.0E-3;
    ekf->Q[8] = 3.0E-4; /* attitude angles */
    ekf->Q[9] = 0.5E-7;
    ekf->Q[10] = 0.5E-7;
    ekf->Q[11] = 0.5E-7; /* bias accelerations */
    ekf->Q[12] = 1.0E-7;
    ekf->Q[13] = 1.0E-7;
    ekf->Q[14] = 1.0E-7; /* bias angular rates, 1e-6 */

    /* measurement noise covariance */
    ekf->Rpos[0] = 50.0;
    ekf->Rpos[1] = 50.0;
    ekf->Rpos[2] = 25.0;
    ekf->Rvel[0] = 2.0;
    ekf->Rvel[1] = 2.0;
    ekf->Rvel[2] = 1.0;
    ekf->Rcmp = 1.5;

    ekf->dt = y[15];
}

void Extrapolate(EKFF_p ekf) {
    int i, j;
    float temp1[DIMNUM2], temp2[DIMNUM2];

    ABCD2CBN(ekf);
    JacobianDFDX(ekf, ekf->dfdx);
    /* SigEKF = SigEKF + T*(dfdx*SigEKF + SigEKF*dfdx' + Q) */
    MultMat(ekf->dfdx, ekf->sigkf, temp1, DIMNUM, DIMNUM, DIMNUM);
    TransposeMatrix(temp1, temp2, DIMNUM, DIMNUM);
    for (i = 0; i < DIMNUM2; ++i) {
        ekf->sigkf[i] += temp1[i] + temp2[i];
    }
    j = 0;
    for (i = 0; i < DIMNUM; ++i) {
        ekf->sigkf[j] += ekf->dt * ekf->Q[i];
        j += DIMNUM + 1;
    }
    /* xEKF = f(xEKF,p,q,r,ax,ay,az) */
    PredictState(ekf);
    NormalizeABCD(ekf);
}

void UpdatePos(EKFF_p ekf, float y[3]) {
    float tempNN[DIMNUM2];
    float temp33[9], inv33[9];
    float L[DIMNUM * PPOS];
    float s0, s1, s2;
    int i, j, k, m;

    /* inv33 = Inverse3(R+dh1dx*SigEKF*dh1dx') */
    temp33[0] = ekf->sigkf[0] + ekf->R[0];
    temp33[1] = ekf->sigkf[1];
    temp33[2] = ekf->sigkf[2];
    temp33[3] = ekf->sigkf[15];
    temp33[4] = ekf->sigkf[16] + ekf->R[1];
    temp33[5] = ekf->sigkf[17];
    temp33[6] = ekf->sigkf[30];
    temp33[7] = ekf->sigkf[31];
    temp33[8] = ekf->sigkf[32] + ekf->R[2];
    Inverse3(temp33, inv33);
    /* L = SigEKF*dh1dx'*inv33 = [p11;p21;p31;p41;p51]*inv33 */
    for (i = 0, j = 0; i < DIMNUM2; i += DIMNUM) {
        s0 = ekf->sigkf[i];
        s1 = ekf->sigkf[i + 1];
        s2 = ekf->sigkf[i + 2];
        L[j] = s0 * inv33[0] + s1 * inv33[3] + s2 * inv33[6];
        ++j;
        L[j] = s0 * inv33[1] + s1 * inv33[4] + s2 * inv33[7];
        ++j;
        L[j] = s0 * inv33[2] + s1 * inv33[5] + s2 * inv33[8];
        ++j;
    }
    /* dxEKF = L*(y - h1(xEKF,p,q,r,ax,ay,az)) */
    s0 = y[0] - ekf->xEKF[0];
    s1 = y[1] - ekf->xEKF[1];
    s2 = y[2] - ekf->xEKF[2];
    i = 0;
    j = 0;
    for (i = 0, j = 0; i < DIMNUM; ++i, j += PPOS) {
        ekf->dxEKF[i] = L[j] * s0 + L[j + 1] * s1 + L[j + 2] * s2;
    }
    /* "add" dxEKF to xEKF */
    UpdateFilterState(ekf);
    /* SigEKF = (eye(15) - L*dh1dx)*SigEKF */
    /* tempNN =  [L11;L21;L31;L41;L51]*[p11 p12 p13 p14 p15] */
    for (i = 0, j = 0; i < DIMNUM * PPOS;) {
        s0 = L[i];
        ++i;
        s1 = L[i];
        ++i;
        s2 = L[i];
        ++i;
        for (k = 0; k < DIMNUM; ++k) {
            tempNN[j] = s0 * ekf->sigkf[k] + s1 * ekf->sigkf[k + 15] + s2 * ekf->sigkf[k + 30];
            ++j;
        }
    }
    /* SigEKF = SigEKF - tempNN */
    for (i = 0; i < DIMNUM; ++i) {
        k = (DIMNUM + 1) * i;
        m = k;
        for (j = i; j < DIMNUM; ++j) {
            ekf->sigkf[k] = 0.5 * (ekf->sigkf[k] - tempNN[k] + ekf->sigkf[m] - tempNN[m]);
            ekf->sigkf[m] = ekf->sigkf[k];
            ++k;
            m += DIMNUM;
        }
    }
}

void UpdateVel(EKFF_p ekf, float y[3]) {
    float tempNN[DIMNUM2];
    float temp33[9], inv33[9];
    float L[DIMNUM * PVEL];
    float s0, s1, s2;
    int i, j, k, m;

    /* inv33 = Inverse3(R+dh2dx*SigEKF*dh2dx') */
    temp33[0] = ekf->sigkf[48] + ekf->R[0];
    temp33[1] = ekf->sigkf[49];
    temp33[2] = ekf->sigkf[50];
    temp33[3] = ekf->sigkf[63];
    temp33[4] = ekf->sigkf[64] + ekf->R[1];
    temp33[5] = ekf->sigkf[65];
    temp33[6] = ekf->sigkf[78];
    temp33[7] = ekf->sigkf[79];
    temp33[8] = ekf->sigkf[80] + ekf->R[2];
    Inverse3(temp33, inv33);
    /* L = SigEKF*dh2dx'*inv33 = [P21;P22;P32;P42;P52]*inv33 */
    for (i = 0, j = 0; i < DIMNUM2; i += DIMNUM) {
        s0 = ekf->sigkf[i + 3];
        s1 = ekf->sigkf[i + 4];
        s2 = ekf->sigkf[i + 5];
        L[j] = s0 * inv33[0] + s1 * inv33[3] + s2 * inv33[6];
        ++j;
        L[j] = s0 * inv33[1] + s1 * inv33[4] + s2 * inv33[7];
        ++j;
        L[j] = s0 * inv33[2] + s1 * inv33[5] + s2 * inv33[8];
        ++j;
    }
    /* dxEKF = L*(y - h2(xEKF,p,q,r,ax,ay,az)) */
    s0 = y[0] - ekf->xEKF[3];
    s1 = y[1] - ekf->xEKF[4];
    s2 = y[2] - ekf->xEKF[5];
    for (i = 0, j = 0; i < DIMNUM; ++i, j += PVEL) {
        ekf->dxEKF[i] = L[j] * s0 + L[j + 1] * s1 + L[j + 2] * s2;
    }
    /* "add" dxEKF to xEKF */
    UpdateFilterState(ekf);
    /* SigEKF = (eye(15) - L*dh2dx)*SigEKF */
    /* tempNN = [L11;L21;L31;L41;L51]*[p21 p22 p23 p24 p25] */
    for (i = 0, j = 0; i < DIMNUM * PVEL;) {
        s0 = L[i];
        ++i;
        s1 = L[i];
        ++i;
        s2 = L[i];
        ++i;
        for (k = 0; k < DIMNUM; ++k) {
            tempNN[j] = s0 * ekf->sigkf[k + 45] + s1 * ekf->sigkf[k + 60] + s2 * ekf->sigkf[k + 75];
            ++j;
        }
    }
    /* SigEKF = SigEKF - tempNN */
    for (i = 0; i < DIMNUM; ++i) {
        k = (DIMNUM + 1) * i;
        m = k;
        for (j = i; j < DIMNUM; ++j) {
            ekf->sigkf[k] = 0.5 * (ekf->sigkf[k] - tempNN[k] + ekf->sigkf[m] - tempNN[m]);
            ekf->sigkf[m] = ekf->sigkf[k];
            ++k;
            m += DIMNUM;
        }
    }
}

void UpdateCmp(EKFF_p ekf, float y[1]) {
    float tempNN[DIMNUM2];
    float dh3dx[3];
    float pdh3dxt[DIMNUM], L[DIMNUM];
    float inv11;
    float h, s0, s1, s2;
    int i, j, k, m;

    /* inv11 = 1/(R+dh3dx*SigEKF*dh3dx') */
    /* pdh3dxt = SigEKF*dh3dx' */
    JacobianDH3DX(ekf, dh3dx);
    s0 = dh3dx[0];
    s1 = dh3dx[1];
    s2 = dh3dx[2];
    for (i = 0, j = 0; i < DIMNUM; ++i, j += DIMNUM) {
        pdh3dxt[i] = ekf->sigkf[j + 6] * s0 + ekf->sigkf[j + 7] * s1 + ekf->sigkf[j + 8] * s2;
    }
    h = pdh3dxt[6] * s0 + pdh3dxt[7] * s1 + pdh3dxt[8] * s2;
    inv11 = 1.0L / (ekf->Rcmp + h);
    /* L = SigEKF*dh3dx'*inv11 */
    for (i = 0; i < DIMNUM; ++i) {
        L[i] = pdh3dxt[i] * inv11;
    }
    /* dxEKF = L*(y - h3(xEKF,p,q,r,ax,ay,az)) */
    h = y[0] - OutputEquationH3(ekf);
    if (h > M_PI) {
        h -= TWOPI;
    } else if (h <= -M_PI) {
        h += TWOPI;
    }
    for (i = 0; i < DIMNUM; ++i) {
        ekf->dxEKF[i] = L[i] * h;
    }
    /* "add" dxEKF to xEKF */
    UpdateFilterState(ekf);
    /* SigEKF = (eye(15) - L*dh3dx)*SigEKF */
    /* tempNN = L*pdh3dxt' */
    for (i = 0, j = 0; i < DIMNUM; ++i) {
        for (k = 0; k < DIMNUM; ++k) {
            tempNN[j] = L[i] * pdh3dxt[k];
            ++j;
        }
    }
    /* SigEKF = SigEKF - tempNN */
    for (i = 0; i < DIMNUM; ++i) {
        k = (DIMNUM + 1) * i;
        m = k;
        for (j = i; j < DIMNUM; ++i) {
            ekf->sigkf[k] = 0.5 * (ekf->sigkf[k] - tempNN[k] + ekf->sigkf[m] - tempNN[m]);
            ekf->sigkf[m] = ekf->sigkf[k];
            ++k;
            m += DIMNUM;
        }
    }
}

void EKF_Filter(EKFF_p ekf, enum OP_TYPE op, float y[], float pqr[3], float accbi[3]) {
    ekf->p = pqr[0];
    ekf->q = pqr[1];
    ekf->r = pqr[2];
    ekf->ax = accbi[0];
    ekf->ay = accbi[1];
    ekf->az = accbi[2];

    if (op == INITIALIZE) {
        Initialize(ekf, y);
    } else {
        Extrapolate(ekf);
        /*
        if (op == UPDATEPOS) {
            UpdateSigmaRPos(ekf);
            UpdatePos(ekf, y);
        } else if (op == UPDATEVEL) {
            UpdateSigmaRVel(ekf);
            UpdateVel(ekf, y);
        } else if (op == UPDATECMP) {
            UpdateCmp(ekf, y);
        }
         */
    }
    ABCD2RPY(ekf);
}


/*
int main()
{
  EKFF ekff;
  float y0[15];
  y0[0]=y0[1]=y0[2]=0; // px py pz
  y0[3]=y0[4]=y0[5]=0; // vx vy vz
  y0[6]=y0[7]=y0[8]=0; // roll pitch yaw
  y0[9]=y0[10]=y0[11]=0; // bax bay baz
  y0[12]=y0[13]=y0[14]=0; // bwx bwy bwz

  float pqr[3];
  pqr[0]=pqr[1]=pqr[2]=0;
  float acc[3];
  acc[0]=acc[1]=0;
  acc[2]=-9.81;

  ekff.Filter(EKFF::INITIALIZE, y0, pqr, acc);

  float pos[3];
  pos[0]=pos[1]=pos[2]=0; // px py pz
  float vel[3];
  vel[0]=vel[1]=vel[2]=0; // vx vy vz
  float cmp[1];
  cmp[0]=0;

  for (int i=0; i < 250; ++i)
  {
      ekff.Filter(EKFF::UPDATEPOS, pos, pqr, acc);
      ekff.Filter(EKFF::UPDATEPOS, vel, pqr, acc);
      ekff.Filter(EKFF::UPDATECMP, cmp, pqr, acc);
  }
}
 */

#endif /*USE_EKF*/
