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

process_funcs = {}

CODE_AC_MODEL_SERVO_POS = 0x22
CODE_AEROCOMP_SERVO_POS = 0x33
CODE_GNDBOARD_ADCM_READ = 0x44
CODE_GNDBOARD_MANI_READ = 0x45

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
packCODE_NTP_RESPONSE = struct.Struct('>BH2I')

cnt = [0,0,0]

def process_CODE_NTP_REQUEST(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_Token = packCODE_NTP_REQUEST.unpack(rf_data)
    if Id == CODE_NTP_REQUEST and sent_ts-gen_ts < 1000 \
            and sent_ts-gen_ts > 0:
        #self.log.info('NTP request {} from {}'.format(
        #    sent_ts-recv_ts, addr.__repr__()))
        resp = packCODE_NTP_RESPONSE.pack(CODE_NTP_RESPONSE, NTP_Token,
                                          sent_ts, recv_ts)
        self.send(resp, addr)


process_funcs[CODE_NTP_REQUEST] = process_CODE_NTP_REQUEST

packCODE_GNDBOARD_STATS = struct.Struct('>B2h3H')


def process_CODE_GNDBOARD_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, load_sen, load_rsen, load_msg = packCODE_GNDBOARD_STATS.unpack(
        rf_data)
    if Id == CODE_GNDBOARD_STATS:
        info = 'GND states NTP{}/{} Load{}/{}/{}'.format(
            NTP_delay, NTP_offset, load_sen, load_rsen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'GND_STA', 'info':info})


process_funcs[CODE_GNDBOARD_STATS] = process_CODE_GNDBOARD_STATS

packCODE_GNDBOARD_ADCM_READ = struct.Struct('>B4Hi2hI')


def process_CODE_GNDBOARD_ADCM_READ(self, rf_data, gen_ts, sent_ts, recv_ts,
                                    addr):
    Id, RigPos1, RigPos2, RigPos3, RigPos4, RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp = packCODE_GNDBOARD_ADCM_READ.unpack(
        rf_data)
    if Id == CODE_GNDBOARD_ADCM_READ:
        self.expData.updateRigPos(RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp)
        if cnt[2] > 12:
            cnt[2] = 0
            info = ('RIG rawdat {}/{}/{}').format( RigRollPos, RigPitchPos, RigYawPos)
            self.msgc2guiQueue.put_nowait({'ID':'GND_DAT', 'info':info})
        else:
            cnt[2] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_GNDBOARD_ADCM_READ] = process_CODE_GNDBOARD_ADCM_READ

packCODE_GNDBOARD_MANI_READ = struct.Struct('>B2f')


def process_CODE_GNDBOARD_MANI_READ(self, rf_data, gen_ts, sent_ts, recv_ts,
                                    addr):
    Id, Vel, DP = packCODE_GNDBOARD_MANI_READ.unpack(rf_data)
    if Id == CODE_GNDBOARD_MANI_READ:
        self.expData.updateMani(Vel, DP)
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)
        self.log.info('Manimeter Vel{:.2f} DP{:.1f}'.format(Vel,DP))


process_funcs[CODE_GNDBOARD_MANI_READ] = process_CODE_GNDBOARD_MANI_READ

packCODE_AEROCOMP_STATS = struct.Struct('>B2h3B3H')


def process_CODE_AEROCOMP_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AEROCOMP_STATS.unpack(
        rf_data)
    if Id == CODE_AEROCOMP_STATS:
        info = 'CMP states NTP{}/{} B{}/{}/{} Load{}/{}/{}'.format(
            NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'CMP_STA', 'info':info})


process_funcs[CODE_AEROCOMP_STATS] = process_CODE_AEROCOMP_STATS

packCODE_AC_MODEL_STATS = struct.Struct('>B2h3B3H')


def process_CODE_AC_MODEL_STATS(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AC_MODEL_STATS.unpack(
        rf_data)
    if Id == CODE_AC_MODEL_STATS:
        info = 'ACM states NTP{}/{} B{}/{}/{} Load{}/{}/{}'.format(
            NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg)
        self.msgc2guiQueue.put_nowait({'ID':'ACM_STA', 'info':info})


process_funcs[CODE_AC_MODEL_STATS] = process_CODE_AC_MODEL_STATS

packCODE_AC_MODEL_SERVO_POS = struct.Struct('>B6H3H6hI6h6hf')

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
        if cnt[0] > 12:
            cnt[0] = 0
            info = ('ACM rawdat S{:4d}/{:4d}/{:4d}/{:4d}/{:4d}/{:4d} '
            'E{:4d}/{:4d}/{:4d} '
            'GX{:6d} GY{:6d} GZ{:6d} '
            'AX{:6d} AY{:6d} AZ{:6d} ').format(
                ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5,ServoPos6,
                EncPos1,EncPos2,EncPos3,
                Gx,Gy,Gz, Nx,Ny,Nz
                )
            self.msgc2guiQueue.put_nowait({'ID':'ACM_DAT', 'info':info})
        else:
            cnt[0] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)


process_funcs[CODE_AC_MODEL_SERVO_POS] = process_CODE_AC_MODEL_SERVO_POS

packCODE_AEROCOMP_SERVO_POS = struct.Struct('>B4H4HI4h4hf')

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
        if cnt[1] > 12:
            cnt[1] = 0
            info = ('CMP rawdat S{:4d}/{:4d}/{:4d}/{:4d} '
            'E{:4d}/{:4d}/{:4d}/{:4d} ').format(
                ServoPos1,ServoPos2,ServoPos3,ServoPos4,
                EncPos1,EncPos2,EncPos3,EncPos4,
                )
            self.msgc2guiQueue.put_nowait({'ID':'CMP_DAT', 'info':info})
        else:
            cnt[1] += 1
        self.parent.save(rf_data, gen_ts, sent_ts, recv_ts, addr)

process_funcs[CODE_AEROCOMP_SERVO_POS] = process_CODE_AEROCOMP_SERVO_POS

