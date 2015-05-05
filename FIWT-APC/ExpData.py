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

        self.RigScale = 360/4096.0

    def updateRigPos(self, RigRollPos,RigPitchPos,RigYawPos):
        self.RigRollRawPos = RigRollPos - self.RigRollPos0
        self.RigPitchRawPos = RigPitchPos - self.RigPitchPos0
        self.RigYawRawPos = RigYawPos - self.RigYawPos0

        self.RigRollPos = self.RigRollRawPos*self.RigScale
        self.RigPitchPos = self.RigPitchRawPos*self.RigScale
        self.RigYawPos = self.RigYawRawPos*self.RigScale

    def updateMani(self, vel, dp):
        self.Vel = vel
        self.DP = dp

    def updateACM(self, ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5, \
            ServoPos6, EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
            CmdTime):
        pass

    def updateCMP(self, ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
            EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
            ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
            ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
            CmdTime):
        pass

