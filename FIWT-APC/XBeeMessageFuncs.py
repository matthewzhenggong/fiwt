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

pack22 = struct.Struct('>B')
def process_pkg22(self, rf_data, gen_ts, sent_ts, recv_ts, addr):
    pass

process_funcs = {'\x22':process_pkg22,
}

