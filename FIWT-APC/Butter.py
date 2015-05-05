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
    def __init__(self):
        self.A = np.array([[0.6742,  -0.2115],[0.2115,   0.9733]])
        self.B = np.array([[0.2991],[0.0378]])
        self.C = np.array([0.0748,   0.6977])
        self.D = 0.0134
        self.X = np.array([[0.0],[0.0]])

    def update(self, U):
        self.X = np.dot(self.A,self.X) + np.dot(self.B,U)
        y = np.dot(self.C,self.X) + np.dot(self.D,U)
        return y[0]

