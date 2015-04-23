
#include <math.h>

typedef struct AHRS{
    float _omega_I[3];
    float _omega_P[3];
    float _omega_yaw_P[3];
    float _omega[3];
    float _omega_bias[3];

    float _omega_I_sum_time;

    float _quaternion[4];
    float _euler[3];
    float cbn11, cbn12, cbn13;
    float cbn21, cbn22, cbn23;
    float cbn31, cbn32, cbn33;

} AHRS_t, *AHRS_p;

void AHRS_reset(AHRS_p ahrs);

void AHRS_recalc_gyro_offsets(AHRS_p ahrs, float gyro[3]);

void AHRS_update(AHRS_p ahrs, float acc[3], float gyro[3], float vel[3]);

static void AHRS_rpy2abcd(AHRS_p ahrs) {
    float u0, u1, u2, cr, cp, cy, sr, sp, sy;
    u0 = ahrs->_euler[0]*0.5;
    u1 = ahrs->_euler[1]*0.5;
    u2 = ahrs->_euler[2]*0.5;
    cr = cos(u0);
    sr = sin(u0);
    cp = cos(u1);
    sp = sin(u1);
    cy = cos(u2);
    sy = sin(u2);
    ahrs->_quaternion[0] = cr * cp * cy + sr * sp*sy;
    ahrs->_quaternion[1] = sr * cp * cy - cr * sp*sy;
    ahrs->_quaternion[2] = cr * sp * cy + sr * cp*sy;
    ahrs->_quaternion[3] = cr * cp * sy - sr * sp*cy;
}

void AHRS_reset(AHRS_p ahrs) {
    int i;

    for (i=0;i<3;++i) {
       ahrs->_omega_I[i] = 0;
       ahrs->_omega_P[i] = 0;
       ahrs->_omega_yaw_P[i] = 0;
       ahrs->_omega[i] = 0;
       ahrs->_omega_bias[i] = 0;
       ahrs->_euler[i] = 0;
    }
    AHRS_rpy2abcd(ahrs);
}

void AHRS_recalc_gyro_offsets(AHRS_p ahrs, float gyro[3]) {
    int i;
    for (i=0;i<3;++i) {
       ahrs->_omega_I[i] = 0;
       ahrs->_omega_I_sum[i] = 0;
       ahrs->_omega_bias[i] = 0.1*ahrs->_omega_bias[i]+0.9*gyro[i];
    }
    ahrs->_omega_I_sum_time = 0;
}

static void AHRS_drift_correction(AHRS_p ahrs, float dt,
        float acc[3], float velocity[3]) {

}

static void AHRS_normalize(AHRS_p ahrs) {
    float a, b, c, d, n;
    a = ahrs->_quaternion[0];
    b = ahrs->_quaternion[1];
    c = ahrs->_quaternion[2];
    d = ahrs->_quaternion[3];
    n = 1.0 / sqrt(a * a + b * b + c * c + d * d);
    ahrs->_quaternion[0] = a*n;
    ahrs->_quaternion[1] = b*n;
    ahrs->_quaternion[2] = c*n;
    ahrs->_quaternion[3] = d*n;
}

static void AHRS_euler_angles(AHRS_p ahrs) {
    float a, b, c, d, a2c2, b2d2, t1, t2;
    a = ahrs->_quaternion[0];
    b = ahrs->_quaternion[1];
    c = ahrs->_quaternion[2];
    d = ahrs->_quaternion[3];
    a2c2 = a * a - c*c;
    b2d2 = b * b - d*d;
    ahrs->_euler[0] = atan2(2.0 * (a * b + c * d), a2c2 - b2d2);
    t1 = 2.0L * (a * c - b * d);
    t2 = sqrt(1.0L - t1 * t1);
    ahrs->_euler[1] = atan(t1 / t2);
    ahrs->_euler[2] = atan2(2.0 * (a * d + b * c), a2c2 + b2d2);
}

void AHRS_quat_update(AHRS_p ahrs, float dt, float gyro[3]) {
    int i;
    float p0, q0, r0, a, b, c, d;

    for (i=0;i<3;++i) {
       ahrs->_omega[i] = (gyro[i]-ahrs->_omega_bias[i]) +ahrs->_omega_I[i];
    }

    p0 = ahrs->_omega[0]+ahrs->_omega_P[0]+ahrs->_omega_yaw_P[0];
    q0 = ahrs->_omega[1]+ahrs->_omega_P[1]+ahrs->_omega_yaw_P[1];
    r0 = ahrs->_omega[2]+ahrs->_omega_P[2]+ahrs->_omega_yaw_P[2];

    a = ahrs->_quaternion[0];
    b = ahrs->_quaternion[1];
    c = ahrs->_quaternion[2];
    d = ahrs->_quaternion[3];
    ahrs->_quaternion[0] = a + dt * 0.5 * (-b * p0 - c * q0 - d * r0);
    ahrs->_quaternion[1] = b + dt * 0.5 * (a * p0 - d * q0 + c * r0);
    ahrs->_quaternion[2] = c + dt * 0.5 * (d * p0 + a * q0 - b * r0);
    ahrs->_quaternion[3] = d + dt * 0.5 * (-c * p0 + b * q0 + a * r0);
}

void AHRS_abcd2cbn(AHRS_p ahrs) {
    float a, b, c, d, ab, ac, ad, bb, bc, bd, cc, cd, dd;
    a = ahrs->_quaternion[0];
    b = ahrs->_quaternion[1];
    c = ahrs->_quaternion[2];
    d = ahrs->_quaternion[3];
    ab = a*b;
    ac = a*c;
    ad = a*d;
    bb = b*b;
    bc = b*c;
    bd = b*d;
    cc = c*c;
    cd = c*d;
    dd = d*d;
    ahrs->cbn11 = 1.0 - 2.0 * (cc + dd);
    ahrs->cbn12 = 2.0 * (bc - ad);
    ahrs->cbn13 = 2.0 * (bd + ac);
    ahrs->cbn21 = 2.0 * (bc + ad);
    ahrs->cbn22 = 1.0 - 2.0 * (bb + dd);
    ahrs->cbn23 = 2.0 * (cd - ab);
    ahrs->cbn31 = 2.0 * (bd - ac);
    ahrs->cbn32 = 2.0 * (cd + ab);
    ahrs->cbn33 = 1.0 - 2.0 * (bb + cc);
}

void AHRS_update(AHRS_p ahrs, float dt, float acc[3], float gyro[3], float vel[3]) {

    // Integrate the DCM matrix using gyro inputs
    AHRS_quat_update(ahrs, dt, gyro);

    // Normalize the DCM matrix
    AHRS_normalize(ahrs);

    AHRS_abcd2cbn(ahrs);

    // Perform drift correction
    AHRS_drift_correction(ahrs, dt, acc, vel);

    // Calculate pitch, roll, yaw for stabilization and navigation
    AHRS_euler_angles(ahrs);
}

