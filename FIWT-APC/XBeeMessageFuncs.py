#!/bin/env python
# -*- coding: utf-8 -*-
"""
XBee Message Process Functions in Python
----------------------------------------

Author: Zheng GONG(matthewzhenggong@gmail.com)

This file is part of FIWT.

FIWT is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
"""

import struct, math, time, traceback
import numpy as np
from scipy import interpolate


bat_acm_volt = np.array([3,3.58, 3.7, 3.74, 3.78, 3.8, 3.88, 4.09, 4.18, 4.2,4.3])
bat_acm_pct = np.array([5, 5, 9, 15, 17, 23, 55, 95, 99, 100,100])
bat_acm_interp1d = interpolate.interp1d(bat_acm_volt, bat_acm_pct, bounds_error=False, fill_value=0)

bat_cmp_volt = np.array([3,3.5, 3.6, 3.66, 3.7, 3.72, 4.0, 4.2, 4.5])
bat_cmp_pct = np.array( [5,  5,  10,  44,   52,  57,   92, 100,100])
bat_cmp_interp1d = interpolate.interp1d(bat_cmp_volt, bat_cmp_pct, bounds_error=False, fill_value=0)

process_funcs = {}

CODE_AC_MODEL_SERVO_POS = 0x22
CODE_AEROCOMP_SERVO_POS = 0x33
CODE_GNDBOARD_ADCM_READ = 0x44
CODE_GNDBOARD_MANI_READ = 0x45
CODE_GNDBOARD_MANI_RAW_READ = 0x46

# State Statistics
CODE_GNDBOARD_STATS = 0x76
CODE_AC_MODEL_STATS = 0x77
CODE_AEROCOMP_STATS = 0x78

# Servos New Position
CODE_AC_MODEL_SERV_CMD = 0xA5
CODE_AEROCOMP_SERV_CMD = 0xA6

#NTP
CODE_NTP_REQUEST = 0x01
CODE_NTP_RESPONSE = 0x02

packCODE_NTP_REQUEST = struct.Struct('>BH')
packCODE_NTP_RESPONSE = struct.Struct('>BH2q')

cnt = [0,0,0]

def process_CODE_NTP_REQUEST(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_Token = packCODE_NTP_REQUEST.unpack(rf_data)
    if Id == CODE_NTP_REQUEST and sent_ts-gen_ts < 1000 and self.response_ntp \
            and sent_ts-gen_ts > 0:
        #self.log.info('NTP request {} from {}'.format(
        #    sent_ts-recv_ts, addr.__repr__()))
        resp = packCODE_NTP_RESPONSE.pack(CODE_NTP_RESPONSE, NTP_Token,
                                          sent_ts, recv_ts)
        self.send(resp, addr)


process_funcs[CODE_NTP_REQUEST] = process_CODE_NTP_REQUEST

packCODE_GNDBOARD_STATS = struct.Struct('>B2h2H')


def process_CODE_GNDBOARD_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, load_sen, load_msg = packCODE_GNDBOARD_STATS.unpack(
        rf_data)
    if Id == CODE_GNDBOARD_STATS:
        info = 'GND states NTP{}/{} Load{}/{}'.format(
            NTP_delay, NTP_offset, load_sen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'GND_STA', 'info':info})
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_GNDBOARD_STATS] = process_CODE_GNDBOARD_STATS

packCODE_GNDBOARD_ADCM_READ = struct.Struct('>B4Hi2hq')


def process_CODE_GNDBOARD_ADCM_READ(self, rf_data, gen_ts, sent_ts, recv_ts,
                                    addr):
    Id, RigPos1, RigPos2, RigPos3, RigPos4, RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp = packCODE_GNDBOARD_ADCM_READ.unpack(
        rf_data)
    if Id == CODE_GNDBOARD_ADCM_READ:
        self.expData.updateRigPos(RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp)
        if cnt[2] > 25:
            cnt[2] = 0
            info = ('RIG {:.3f} rawdat {}/{}/{}').format( ADC_TimeStamp*1e-6,
                    RigRollPos, RigPitchPos, RigYawPos)
            self.msgc2guiQueue.put_nowait({'ID':'GND_DAT', 'info':info})
        else:
            cnt[2] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_GNDBOARD_ADCM_READ] = process_CODE_GNDBOARD_ADCM_READ


packCODE_AEROCOMP_STATS = struct.Struct('>B2h3B3H')


def process_CODE_AEROCOMP_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AEROCOMP_STATS.unpack(
        rf_data)
    if Id == CODE_AEROCOMP_STATS:
        B1v = 0.0199*B1-0.0772
        B3v = (0.0377*B3+0.3571)-B1v
        batpct = int(bat_cmp_interp1d(B1v))
        info = 'CMP states NTP{}/{} B{:d}/{:d}/{:d}(ADC) B{:04.2f}/{:04.2f}(V) Load{}/{}/{}'.format(
            NTP_delay, NTP_offset, B1, B2, B3, B1v, B3v, load_sen, load_rsen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'CMP_STA', 'info':info, 'batpct':batpct})
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_AEROCOMP_STATS] = process_CODE_AEROCOMP_STATS

packCODE_AC_MODEL_STATS = struct.Struct('>B2h3B3H')


def process_CODE_AC_MODEL_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AC_MODEL_STATS.unpack(
        rf_data)
    if Id == CODE_AC_MODEL_STATS:
        B1v = 0.0192*B1+0.28065
        B3v = (0.0589*B3+0.456946)-B1v
        batpct = int(bat_acm_interp1d(B1v))
        info = 'ACM states NTP{}/{} B{:04.2f}/{:04.2f}(V) Load{}/{}/{}'.format(
            NTP_delay, NTP_offset, B1v, B3v, load_sen, load_rsen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'ACM_STA', 'info':info, 'batpct':batpct})
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_AC_MODEL_STATS] = process_CODE_AC_MODEL_STATS

packCODE_AC_MODEL_SERVO_POS = struct.Struct('>B6H3H6hq6h6hf')

def process_CODE_AC_MODEL_SERVO_POS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5,ServoPos6, \
            EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
            CmdTime = packCODE_AC_MODEL_SERVO_POS.unpack(rf_data)
    if Id == CODE_AC_MODEL_SERVO_POS:
        self.expData.updateACM(ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5, \
            ServoPos6, EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
            CmdTime)
        if cnt[0] > 25:
            cnt[0] = 0
            info = ('ACM {:.3f} rawdat S{:04d}/{:04d}/{:04d}/{:04d}/{:04d}/{:04d} '
            'E{:04d}/{:04d}/{:04d} '
            'Nx{:d}/Gx{:d} ').format(ts_ADC*1e-6,
                ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5,ServoPos6,
                EncPos1,EncPos2,EncPos3, Nx,Gx)
            self.msgc2guiQueue.put_nowait({'ID':'ACM_DAT', 'info':info})
        else:
            cnt[0] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_AC_MODEL_SERVO_POS] = process_CODE_AC_MODEL_SERVO_POS

packCODE_AEROCOMP_SERVO_POS = struct.Struct('>B4H4Hq4h4hf')

def process_CODE_AEROCOMP_SERVO_POS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
            EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
            CmdTime = packCODE_AEROCOMP_SERVO_POS.unpack(rf_data)
    if Id == CODE_AEROCOMP_SERVO_POS:
        self.expData.updateCMP(ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
            EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
            CmdTime)
        if cnt[1] > 25:
            cnt[1] = 0
            info = ('CMP {:.3f} rawdat S{:04d}/{:04d}/{:04d}/{:04d} '
            'E{:04d}/{:04d}/{:04d}/{:04d} ').format(ts_ADC*1e-6,
                ServoPos1,ServoPos2,ServoPos3,ServoPos4,
                EncPos1,EncPos2,EncPos3,EncPos4,
                )
            self.msgc2guiQueue.put_nowait({'ID':'CMP_DAT', 'info':info})
        else:
            cnt[1] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)

process_funcs[CODE_AEROCOMP_SERVO_POS] = process_CODE_AEROCOMP_SERVO_POS

