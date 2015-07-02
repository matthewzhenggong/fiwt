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

import numpy as np

class Butter(object):
    def __init__(self,rate_lim=1000):
        self.A = np.array([[0.8299,  -0.1151],[0.1151,   0.9928]])
        self.B = np.array([[0.1628],[0.0102]])
        self.C = np.array([0.0407,   0.7045])
        self.D = 0.0036
        self.X = np.array([[0.0],[0.0]])
        self.circle = 0
        self.U0 = 0
        self.U0_raw = 0
        self.rate_lim = rate_lim
        self.count = 0
        self.Y0 = 0
        self.rate = 0
        self.last_rate = 0

    def update(self, U, dt):
        if dt < 1e06:
            dt = 1/244.0
        rate_raw = (U-self.U0_raw)/dt
        self.U0_raw = U
        if abs(rate_raw) > self.rate_lim:
            self.count = 1
        if self.count > 0:
            U = self.U0-self.circle+self.rate*dt
            self.count -= 1

        while U+self.circle - self.U0 > 180:
            self.circle -= 360
        while U+self.circle - self.U0 < -180:
            self.circle += 360
        self.U0 = U+self.circle

        self.X = np.dot(self.A,self.X) + np.dot(self.B,self.U0)
        y = np.dot(self.C,self.X) + np.dot(self.D,self.U0)
        Y = y[0]
        rate = (Y - self.Y0)/dt
        if abs(rate-self.last_rate) > 100:
            #rate = self.rate
            pass
        else:
            self.rate = rate
        self.last_rate = rate
        self.Y0 = Y
        YC = Y%360
        if YC > 180:
            YC -= 360
        return rate,YC


