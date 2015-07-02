#!/bin/env python
# -*- coding: utf-8 -*-
"""
Matlab UDP Link Process in Python
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

import socket
import struct

class MatlabLink(object):
    def __init__(self, parent, ports):
        self.parent = parent
        self.expData = parent.expData
        self.log = parent.log
        self.host = '127.0.0.1'
        local_port = ports[0]
        remote_port = ports[1]
        extra_inputs = len(ports[2].split())

        self.rx_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.rx_udp.bind((self.host,local_port))
        self.rx_udp.setblocking(0)
        self.socklist = [self.rx_udp]

        self.tx_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.tx_udp.setblocking(0)
        self.tx_udp_addr = (self.host,remote_port)

        self.rx_pack = struct.Struct("<d4d3d{}d".format(extra_inputs))
        self.tx_pack = struct.Struct("<44d")

    def getReadList(self):
        return self.socklist

    def write(self, exp):
        data = self.tx_pack.pack(exp.ACM_CmdTime,
                exp.GX, exp.GY, exp.GZ, exp.AX, exp.AY,
                exp.AZ, exp.ACM_roll_filtered, exp.ACM_roll_rate,
                exp.ACM_pitch_filtered, exp.ACM_pitch_rate,
                exp.ACM_yaw_filtered, exp.ACM_yaw_rate,
                exp.RigRollPosFiltered, exp.RigRollPosRate,
                exp.RigPitchPosFiltered, exp.RigPitchPosRate,
                exp.RigYawPosFiltered, exp.RigYawPosRate,
                exp.ACM_svoref1, exp.ACM_servo1, #19 20
                exp.ACM_svoref2, exp.ACM_servo2,
                exp.ACM_svoref3, exp.ACM_servo3,
                exp.ACM_svoref4, exp.ACM_servo4,
                exp.ACM_svoref5, exp.ACM_servo5,
                exp.ACM_svoref6, exp.ACM_servo6,
                exp.CMP_servo1, exp.CMP_svoref1, #31 32
                exp.CMP_servo2, exp.CMP_svoref2,
                exp.CMP_servo3, exp.CMP_svoref3,
                exp.CMP_servo4, exp.CMP_svoref4,
                exp.Vel, exp.AoA, exp.AoS, exp.roll_ac, exp.pitch_ac)
        self.tx_udp.sendto(data, self.tx_udp_addr)

    def read(self, rlist, recv_ts):
        if self.rx_udp in rlist:
            (dat,address)=self.rx_udp.recvfrom(1000)
            data = self.rx_pack.unpack(dat)
            self.expData.sendCommand(*data)

