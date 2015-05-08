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

class Manimeter(object):
    def __init__(self, port_name, baudrate=9600):
        self.Vel = 0
        self.DP = 0
        try:
            self.serial_port = serial.Serial(port=port_name,
                    baudrate=baudrate, timeout=1)
        except:
            self.serial_port = None
            traceback.print_exc()
        if self.serial_port:
            self.running = True
            self.msg_thread = threading.Thread(target=self.processMsg)
            self.msg_thread.daemon = True
            self.msg_thread.start()

    def processMsg(self):
        ser = self.serial_port
        while self.running:
            ser.write('T')
            data = ser.read(64)
            seg = data.split(',')
            num = len(seg)
            props = {}
            for i in xrange(0,num,2):
                try:
                    props[seg[i].strip('\"')] = float(seg[i+1])
                except:
                    pass
            print data
            print props


if __name__ == '__main__':
    import time
    m = Manimeter('COM6')
    while True:
        time.sleep(1)

