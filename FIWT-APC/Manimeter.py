#!/bin/env python
# -*- coding: utf-8 -*-
"""
Manimeter serial process in Python
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

import serial
import threading
import traceback
import struct
from utils import getMicroseconds

class Manimeter(object):
    def __init__(self, parent, port_name, baudrate=4800):
        self.parent = parent
        self.log = parent.parent.log
        self.AA = struct.Struct('>B3f')
        try:
            self.serial_port = serial.Serial(port=port_name,
                    baudrate=baudrate, timeout=2)
        except:
            self.serial_port = None
            self.log.error(traceback.format_exc())
        if self.serial_port:
            self.running = True
            self.msg_thread = threading.Thread(target=self.processMsg)
            self.msg_thread.daemon = True
            self.msg_thread.start()

    def processMsg(self):
        ser = self.serial_port
        while self.running:
            ser.write('T\n')
            data = ser.read(128)
            seg = data.split(',')
            num = len(seg)
            props = {}
            for i in xrange(0,num,3):
                try:
                    props[seg[i].strip('\"')] = (float(seg[i+1]), seg[i+2])
                except:
                    pass
            if self.parent:
                if 'D.P.' in props and 'Velocity' in props:
                    ts1 = getMicroseconds()-self.parent.parent.T0

                    self.parent.DP = props['D.P.'][0]
                    self.parent.Vel = props['Velocity'][0]
                    pkgdata = self.AA.pack(0xE7, ts1*1e-6, self.parent.Vel, self.parent.DP)
                    self.parent.parent.save(pkgdata, ts1, ts1, ts1, ['192.168.191.1',0])
                    self.log.info(data)


if __name__ == '__main__':
    import time
    m = Manimeter(None, '/dev/ttyUSB0')
    while True:
        time.sleep(1)

