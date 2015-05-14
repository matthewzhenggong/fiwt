#!/bin/env python
# -*- coding: utf-8 -*-
"""
Payload Packet pack/unpack functions in Python
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

import struct

MSG_DILIMITER = '\x9E' #0x80+0x1E(RS)
MSG_ESC = '\x9B'     #0x80+0x1B(ESC)
ESCAPE_BYTES = (MSG_DILIMITER, MSG_ESC)
TS = struct.Struct('>q')
lenTS = TS.size


def unpack(data) :
    packs = []
    group = data.split(MSG_DILIMITER)[1:]
    for pkg in group:
        s = pkg.split(MSG_ESC)
        packs.append(''.join([s[0]]+[chr(ord(i[0])^0x20)+i[1:]
            for i in s[1:]]))
    if len(packs):
        last = packs[-1]
        sent_timestamp = TS.unpack(last[-lenTS:])[0]
        packs[-1] = last[:-lenTS]
        packs = [(TS.unpack(i[:lenTS])[0], i[lenTS:]) for i in packs]
        return packs, sent_timestamp
    else:
        return None

def pack(data, timestamp) :
    data = TS.pack(timestamp)+data
    return "".join([MSG_DILIMITER]+[byte if byte not in ESCAPE_BYTES
        else MSG_ESC+chr(0x20^ord(byte)) for byte in data])

def packs(timestamp,*data_list) :
    ts = TS.pack(timestamp)
    ts = [byte if byte not in ESCAPE_BYTES else MSG_ESC+chr(0x20^ord(byte))
            for byte in ts]
    ts = "".join(ts)
    return "".join(data_list)+ts

if __name__ == '__main__' :
    p1 = pack('Phello',1000000)
    print ':'.join(['{:02x}'.format(ord(i)) for i in p1])
    px = packs(1000001,p1)
    print ':'.join(['{:02x}'.format(ord(i)) for i in px])
    print unpack(px)

