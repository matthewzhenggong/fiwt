#!/bin/env python
# -*- coding: utf-8 -*-
"""
Exp data process in Python
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

import math, struct
from Butter import Butter

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF

class ExpData(object):
    def __init__(self, msgc2guiQueue):
        self.RigRollRawPos = 0
        self.RigRollPos0 = 0
        self.RigPitchRawPos = 0
        self.RigPitchPos0 = 0
        self.RigYawRawPos = 0
        self.RigYawPos0 = 0
        self.Vel = 0
        self.DP = 0
        self.GND_ADC_TS = 0
        self.msgc2guiQueue = msgc2guiQueue

        self.ACM_servo1 = 0
        self.ACM_servo2 = 0
        self.ACM_servo3 = 0
        self.ACM_servo4 = 0
        self.ACM_servo5 = 0
        self.ACM_servo6 = 0

        self.CMP_servo1 = 0
        self.CMP_servo2 = 0
        self.CMP_servo3 = 0
        self.CMP_servo4 = 0

        self.ACM_servo1_0 = 1967
        self.ACM_servo2_0 = 2259
        self.ACM_servo3_0 = 2000
        self.ACM_servo4_0 = 2200
        self.ACM_servo5_0 = 1820
        self.ACM_servo6_0 = 2210

        self.CMP_servo1_0 = 2020
        self.CMP_servo2_0 = 2050
        self.CMP_servo3_0 = 2000
        self.CMP_servo4_0 = 2020

        self.GX = 0
        self.GY = 0
        self.GZ = 0
        self.AX = 0
        self.AY = 0
        self.AZ = 0
        self.ACM_svoref1 = 0
        self.ACM_svoref2 = 0
        self.ACM_svoref3 = 0
        self.ACM_svoref4 = 0
        self.ACM_svoref5 = 0
        self.ACM_svoref6 = 0
        self.ACM_ADC_TS = 0
        self.ACM_CmdTime = 0

        self.ACM_pitch = 0
        self.ACM_roll = 0
        self.ACM_yaw = 0

        self.ACM_pitch0 = 236
        self.ACM_roll0 = 4964
        self.ACM_yaw0 = 0

        self.ACMScale = 180/4096.0
        self.CMPScale = 180/4096.0
        self.EncScale = 180/4096.0
        self.RigScale = 120/3873.0

        self.RigRollPos = 0
        self.RigPitchPos = 0
        self.RigYawPos = 0

        self.RigRollPosRate = 0
        self.RigPitchPosRate = 0
        self.RigYawPosRate = 0
        self.RigRollPosFiltered = 0
        self.RigPitchPosFiltered = 0
        self.RigYawPosFiltered = 0
        self.RigRollPosButt = Butter()
        self.RigPitchPosButt = Butter()
        self.RigYawPosButt = Butter()

        self.ACM_pitch_rate = 0
        self.ACM_roll_rate = 0
        self.ACM_yaw_rate = 0
        self.ACM_pitch_filtered = 0
        self.ACM_roll_filtered = 0
        self.ACM_yaw_filtered = 0
        self.ACM_pitch_butt = Butter()
        self.ACM_roll_butt = Butter()
        self.ACM_yaw_butt = Butter()

        self.A5 = struct.Struct('>BfB6H')
        self.last_update_ts = 0

    def updateRigPos(self, RigRollPos,RigPitchPos,RigYawPos, ts_ADC):
        self.RigRollRawPos = RigRollPos - self.RigRollPos0
        self.RigPitchRawPos = RigPitchPos - self.RigPitchPos0
        self.RigYawRawPos = RigYawPos - self.RigYawPos0
        ts = self.GND_ADC_TS
        self.GND_ADC_TS = ts_ADC*1e-6
        dt = self.GND_ADC_TS - ts

        self.RigRollPos = self.RigRollRawPos*self.RigScale
        self.RigPitchPos = self.RigPitchRawPos*self.RigScale
        self.RigYawPos = self.RigYawRawPos*self.RigScale
        roll = self.RigRollPosButt.update(self.RigRollPos)
        pitch = self.RigPitchPosButt.update(self.RigPitchPos)
        yaw = self.RigYawPosButt.update(self.RigYawPos)
        self.RigRollPosRate = (roll - self.RigRollPosFiltered)/dt
        self.RigPitchPosRate = (pitch - self.RigPitchPosFiltered)/dt
        self.RigYawPosRate = (yaw - self.RigYawPosFiltered)/dt
        self.RigRollPosFiltered = roll
        self.RigPitchPosFiltered = pitch
        self.RigYawPosFiltered = yaw

        self.update2GUI(ts_ADC)

    def updateMani(self, vel, dp):
        self.Vel = vel
        self.DP = dp

    def getCMDhdr(self):
        return []

    def getGNDhdr(self):
        return ["GND_ADC_TS", "RigRollRawPos", "RigRollPos",
                "RigRollPosFiltered", "RigRollPosRate",
                "RigPitchRawPos", "RigPitchPos",
                "RigPitchPosFiltered", "RigPitchPosRate",
                "RigYawRawPos", "RigYawPos",
                "RigYawPosFiltered", "RigYawPosRate",
                "Vel", "DP"] \
                        + ["gen_ts", "sent_ts", "recv_ts", "port"]

    def getGNDdata(self):
        return [self.GND_ADC_TS, self.RigRollRawPos, self.RigRollPos,
                self.RigRollPosFiltered, self.RigRollPosRate,
                self.RigPitchRawPos, self.RigPitchPos,
                self.RigPitchPosFiltered, self.RigPitchPosRate,
                self.RigYawRawPos, self.RigYawPos,
                self.RigYawPosFiltered, self.RigYawPosRate,
                self.Vel, self.DP]

    def updateACM(self, ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5, \
            ServoPos6, EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
            CmdTime):
        self.ACM_servo1 = (ServoPos1-self.ACM_servo1_0)*self.ACMScale
        self.ACM_servo2 = (ServoPos2-self.ACM_servo2_0)*self.ACMScale
        self.ACM_servo3 = (ServoPos3-self.ACM_servo3_0)*self.ACMScale
        self.ACM_servo4 = (ServoPos4-self.ACM_servo4_0)*self.ACMScale
        self.ACM_servo5 = (ServoPos5-self.ACM_servo5_0)*self.ACMScale
        self.ACM_servo6 = (ServoPos6-self.ACM_servo6_0)*self.ACMScale
        self.ACM_roll = (EncPos1 - self.ACM_roll0)*self.EncScale
        self.ACM_pitch = (EncPos2 - self.ACM_pitch0)*self.EncScale
        self.ACM_yaw = (EncPos3 - self.ACM_yaw0)*self.EncScale
        self.GX = Get14bit(Gx)*0.05
        self.GY = Get14bit(Gy)*-0.05
        self.GZ = Get14bit(Gz)*-0.05
        self.AX = Get14bit(Nx)*-0.003333
        self.AY = Get14bit(Ny)*0.003333
        self.AZ = Get14bit(Nz)*0.003333
        ts = self.ACM_ADC_TS
        self.ACM_ADC_TS = ts_ADC*1e-6
        dt = self.ACM_ADC_TS - ts
        self.ACM_svoref1 = (ServoRef1-self.ACM_servo1_0)*self.ACMScale
        self.ACM_svoref2 = (ServoRef2-self.ACM_servo2_0)*self.ACMScale
        self.ACM_svoref3 = (ServoRef3-self.ACM_servo3_0)*self.ACMScale
        self.ACM_svoref4 = (ServoRef4-self.ACM_servo4_0)*self.ACMScale
        self.ACM_svoref5 = (ServoRef5-self.ACM_servo5_0)*self.ACMScale
        self.ACM_svoref6 = (ServoRef6-self.ACM_servo6_0)*self.ACMScale
        self.ACM_mot1 = ServoCtrl1
        self.ACM_mot2 = ServoCtrl2
        self.ACM_mot3 = ServoCtrl3
        self.ACM_mot4 = ServoCtrl4
        self.ACM_mot5 = ServoCtrl5
        self.ACM_mot6 = ServoCtrl6
        self.ACM_CmdTime = CmdTime


        pitch = self.ACM_pitch_butt.update(self.ACM_pitch)
        roll = self.ACM_roll_butt.update(self.ACM_roll)
        yaw = self.ACM_yaw_butt.update(self.ACM_yaw)
        self.ACM_pitch_rate = (pitch - self.ACM_pitch_filtered)/dt
        self.ACM_roll_rate = (roll - self.ACM_roll_filtered)/dt
        self.ACM_yaw_rate = (yaw - self.ACM_yaw_filtered)/dt
        self.ACM_pitch_filtered = pitch
        self.ACM_roll_filtered = roll
        self.ACM_yaw_filtered = yaw

        self.update2GUI(ts_ADC)

    def getACMdata(self):
        return [self.ACM_ADC_TS, self.ACM_CmdTime, self.ACM_svoref1,
                self.ACM_servo1, self.ACM_svoref2, self.ACM_servo2,
                self.ACM_svoref3, self.ACM_servo3, self.ACM_svoref4,
                self.ACM_servo4, self.ACM_svoref5, self.ACM_servo5,
                self.ACM_svoref6, self.ACM_servo6, self.ACM_roll,
                self.ACM_roll_filtered, self.ACM_roll_rate,
                self.ACM_pitch, self.ACM_pitch_filtered, self.ACM_pitch_rate,
                self.ACM_yaw, self.ACM_yaw_filtered, self.ACM_yaw_rate,
                self.GX, self.GY, self.GZ, self.AX, self.AY, self.AZ,
                self.ACM_mot1, self.ACM_mot2, self.ACM_mot3, self.ACM_mot4,
                self.ACM_mot5, self.ACM_mot6]

    def getACMhdr(self):
        return ["ACM_ADC_TS", "ACM_CmdTime", "ACM_svoref1",
                "ACM_servo1", "ACM_svoref2", "ACM_servo2",
                "ACM_svoref3", "ACM_servo3", "ACM_svoref4",
                "ACM_servo4", "ACM_svoref5", "ACM_servo5",
                "ACM_svoref6", "ACM_servo6", "ACM_roll",
                "ACM_roll_filtered", "ACM_roll_rate",
                "ACM_pitch", "ACM_pitch_filtered", "ACM_pitch_rate",
                "ACM_yaw", "ACM_yaw_filtered", "ACM_yaw_rate",
                "GX", "GY", "GZ", "AX", "AY", "AZ",
                "ACM_mot1", "ACM_mot2", "ACM_mot3", "ACM_mot4",
                "ACM_mot5", "ACM_mot6"] \
                        + ["gen_ts", "sent_ts", "recv_ts", "port"]


    def updateCMP(self, ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
            EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
            CmdTime):
        self.CMP_servo1 = (EncPos1-self.CMP_servo1_0)*self.CMPScale
        self.CMP_servo2 = (EncPos2-self.CMP_servo2_0)*self.CMPScale
        self.CMP_servo3 = (EncPos3-self.CMP_servo3_0)*self.CMPScale
        self.CMP_servo4 = (EncPos4-self.CMP_servo4_0)*self.CMPScale
        self.CMP_ADC_TS = ts_ADC*1e-6
        self.CMP_svoref1 = (ServoRef1-self.CMP_servo1_0)*self.CMPScale
        self.CMP_svoref2 = (ServoRef2-self.CMP_servo2_0)*self.CMPScale
        self.CMP_svoref3 = (ServoRef3-self.CMP_servo3_0)*self.CMPScale
        self.CMP_svoref4 = (ServoRef4-self.CMP_servo4_0)*self.CMPScale
        self.CMP_mot1 = ServoCtrl1
        self.CMP_mot2 = ServoCtrl2
        self.CMP_mot3 = ServoCtrl3
        self.CMP_mot4 = ServoCtrl4
        self.CMP_CmdTime = CmdTime

        self.update2GUI(ts_ADC)

    def getCMPdata(self):
        return [self.CMP_ADC_TS, self.CMP_CmdTime, self.CMP_svoref1,
                self.CMP_servo1, self.CMP_svoref2, self.CMP_servo2,
                self.CMP_svoref3, self.CMP_servo3, self.CMP_svoref4,
                self.CMP_servo4, self.CMP_mot1, self.CMP_mot2,
                self.CMP_mot3, self.CMP_mot4]

    def getCMPhdr(self):
        return ["CMP_ADC_TS", "CMP_CmdTime", "CMP_svoref1",
                "CMP_servo1", "CMP_svoref2", "CMP_servo2",
                "CMP_svoref3", "CMP_servo3", "CMP_svoref4",
                "CMP_servo4", "CMP_mot1", "CMP_mot2",
                "CMP_mot3", "CMP_mot4"] \
                        + ["gen_ts", "sent_ts", "recv_ts", "port"]

    def sendCommand(self, time_token, da, de, dr, da_cmp, de_cmp, dr_cmp):
        da = int(da/self.ACMScale)
        de = int(de/self.ACMScale)
        dr = int(dr/self.ACMScale)

        da_cmp = int(da_cmp/self.CMPScale)
        de_cmp = int(de_cmp/self.CMPScale)
        dr_cmp = int(dr_cmp/self.CMPScale)

        self.ACM_servo1_cmd = self.ACM_servo1_0 - da
        self.ACM_servo2_cmd = self.ACM_servo2_0 - da
        self.ACM_servo3_cmd = self.ACM_servo3_0 + dr
        self.ACM_servo4_cmd = self.ACM_servo4_0 + dr
        self.ACM_servo5_cmd = self.ACM_servo5_0 + de
        self.ACM_servo6_cmd = self.ACM_servo6_0 - de
        data = self.A5.pack(0xA5, time_token, 1, self.ACM_servo1_cmd,
                self.ACM_servo2_cmd, self.ACM_servo3_cmd, self.ACM_servo4_cmd,
                self.ACM_servo5_cmd,self.ACM_servo6_cmd)
        self.xbee_network.send(data,self.ACM_node)

        self.CMP_servo1_cmd = self.CMP_servo1_0 + da_cmp +de_cmp
        self.CMP_servo2_cmd = self.CMP_servo2_0 + da_cmp +dr_cmp
        self.CMP_servo3_cmd = self.CMP_servo3_0 + da_cmp -de_cmp
        self.CMP_servo4_cmd = self.CMP_servo4_0 + da_cmp -dr_cmp

        data = self.A5.pack(0xA6, time_token, 1, self.CMP_servo1_cmd,
                self.CMP_servo2_cmd, self.CMP_servo3_cmd, self.CMP_servo4_cmd,
                2000,2000)
        self.xbee_network.send(data,self.CMP_node)

    def update2GUI(self, ts_ADC):
        deltaT = ts_ADC - self.last_update_ts
        if deltaT > 50000 or deltaT < 0:
            self.last_update_ts = ts_ADC
            self.msgc2guiQueue.put_nowait({'ID':'ExpData',
                'states':[self.GND_ADC_TS,
                        self.GX, self.GY, self.GZ, self.AX, self.AY,
                        self.AZ, self.ACM_roll_filtered, self.ACM_roll_rate,
                        self.ACM_pitch_filtered, self.ACM_pitch_rate,
                        self.ACM_yaw_filtered, self.ACM_yaw_rate,
                        self.RigRollPosFiltered, self.RigRollPosRate,
                        self.RigPitchPosFiltered, self.RigPitchPosRate,
                        self.RigYawPosFiltered, self.RigYawPosRate,
                        self.ACM_svoref1, self.ACM_servo1,
                        self.ACM_svoref2, self.ACM_servo2,
                        self.ACM_svoref3, self.ACM_servo3,
                        self.ACM_svoref4, self.ACM_servo4,
                        self.ACM_svoref5, self.ACM_servo5,
                        self.ACM_svoref6, self.ACM_servo6,
                        self.CMP_servo1, self.CMP_svoref1,
                        self.CMP_servo2, self.CMP_svoref2,
                        self.CMP_servo3, self.CMP_svoref3,
                        self.CMP_servo4, self.CMP_svoref4,
                        ]})

