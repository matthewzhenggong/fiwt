#!/bin/env python
# -*- coding: utf-8 -*-

MSG_DILIMITER = '\x9E' #0x80+0x1E(RS)
MSG_ESC = '\x9B'     #0x80+0x1B(ESC)
ESCAPE_BYTES = (MSG_DILIMITER, MSG_ESC)

def unpack(data) :
    packs = []
    group = data.split(MSG_DILIMITER)[1:]
    for pkg in group:
        s = pkg.split(MSG_ESC)
        packs.append(''.join([s[0]]+[chr(ord(i[0])^0x20)+i[1:]
            for i in s[1:]]))
    return packs

def pack(data) :
    return "".join([MSG_DILIMITER]+[byte if byte not in ESCAPE_BYTES
        else MSG_ESC+chr(0x20^ord(byte)) for byte in data])

def packs(*data_list) :
    return "".join([pack(data) for data in data_list])

if __name__ == '__main__' :
    data = '\x9E"\x9B\x00\x00\x04I\x00\x00\x00\x00\x00\x00\x1f\xff\x1f\xff\x1f\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\r\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    print unpack(data)
    print ':'.join(['{:02x}'.format(ord(i)) for i in pack('Phello')])
    print ':'.join(['{:02x}'.format(ord(i)) for i in pack('Phello')])
    print ':'.join(['{:02x}'.format(ord(i))
        for i in packs('Phello','Phello','Phello')])

