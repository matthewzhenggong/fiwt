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

import math

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF

class ExpData(object):
    def __init__(self):
        self.RigRollRawPos = 0
        self.RigRollPos0 = 0
        self.RigPitchRawPos = 0
        self.RigPitchPos0 = 0
        self.RigYawRawPos = 0
        self.RigYawPos0 = 0
        self.Vel = 0
        self.DP = 0

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
        self.RigScale = 180/4096.0

    def updateRigPos(self, RigRollPos,RigPitchPos,RigYawPos, ts_ADC):
        self.RigRollRawPos = RigRollPos - self.RigRollPos0
        self.RigPitchRawPos = RigPitchPos - self.RigPitchPos0
        self.RigYawRawPos = RigYawPos - self.RigYawPos0
        self.GND_ADC_TS = ts_ADC*1e-6

        self.RigRollPos = self.RigRollRawPos*self.RigScale
        self.RigPitchPos = self.RigPitchRawPos*self.RigScale
        self.RigYawPos = self.RigYawRawPos*self.RigScale

    def updateMani(self, vel, dp):
        self.Vel = vel
        self.DP = dp

    def getCMDhdr(self):
        return []

    def getGNDhdr(self):
        return ["GND_ADC_TS", "RigRollRawPos", "RigRollPos",
                "RigPitchRawPos", "RigPitchPos",
                "RigYawRawPos", "RigYawPos", "Vel", "DP",
                "gen_ts", "sent_ts", "recv_ts", "port"]

    def getGNDdata(self):
        return [self.GND_ADC_TS, self.RigRollRawPos, self.RigRollPos,
                self.RigPitchRawPos, self.RigPitchPos,
                self.RigYawRawPos, self.RigYawPos, self.Vel, self.DP]

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
        self.ACM_roll = (EncPos1 - self.ACM_roll0)*self.RigScale
        self.ACM_pitch = (EncPos2 - self.ACM_pitch0)*self.RigScale
        self.ACM_yaw = (EncPos3 - self.ACM_yaw0)*self.RigScale
        self.GX = Get14bit(Gx)*0.05
        self.GY = Get14bit(Gy)*-0.05
        self.GZ = Get14bit(Gz)*-0.05
        self.AX = Get14bit(Nx)*-0.003333
        self.AY = Get14bit(Ny)*0.003333
        self.AZ = Get14bit(Nz)*0.003333
        self.ACM_ADC_TS = ts_ADC*1e-6
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

    def getACMdata(self):
        return [self.ACM_ADC_TS, self.ACM_CmdTime, self.ACM_svoref1, self.ACM_servo1,
                self.ACM_svoref2, self.ACM_servo2, self.ACM_svoref3, self.ACM_servo3,
                self.ACM_svoref4, self.ACM_servo4, self.ACM_svoref5, self.ACM_servo5,
                self.ACM_svoref6, self.ACM_servo6, self.ACM_roll,
                self.ACM_pitch, self.ACM_yaw, self.GX, self.GY, self.GZ,
                self.AX, self.AY, self.AZ, self.ACM_mot1, self.ACM_mot2,
                self.ACM_mot3, self.ACM_mot4, self.ACM_mot5, self.ACM_mot6]

    def getACMhdr(self):
        return ["ACM_ADC_TS", "ACM_CmdTime", "ACM_svoref1", "ACM_servo1",
                "ACM_svoref2", "ACM_servo2", "ACM_svoref3", "ACM_servo3",
                "ACM_svoref4", "ACM_servo4", "ACM_svoref5", "ACM_servo5",
                "ACM_svoref6", "ACM_servo6", "ACM_roll",
                "ACM_pitch", "ACM_yaw", "GX", "GY", "GZ",
                "AX", "AY", "AZ", "ACM_mot1", "ACM_mot2",
                "ACM_mot3", "ACM_mot4", "ACM_mot5", "ACM_mot6",
                "gen_ts", "sent_ts", "recv_ts", "port"]


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

    def getCMPdata(self):
        return [self.CMP_ADC_TS, self.CMP_CmdTime, self.CMP_svoref1, self.CMP_servo1,
                self.CMP_svoref2, self.CMP_servo2, self.CMP_svoref3, self.CMP_servo3,
                self.CMP_svoref4, self.CMP_servo4,
                self.CMP_mot1, self.CMP_mot2, self.CMP_mot3, self.CMP_mot4]

    def getCMPhdr(self):
        return ["CMP_ADC_TS", "CMP_CmdTime", "CMP_svoref1", "CMP_servo1",
                "CMP_svoref2", "CMP_servo2", "CMP_svoref3", "CMP_servo3",
                "CMP_svoref4", "CMP_servo4",
                "CMP_mot1", "CMP_mot2", "CMP_mot3", "CMP_mot4",
                "gen_ts", "sent_ts", "recv_ts", "port"]



