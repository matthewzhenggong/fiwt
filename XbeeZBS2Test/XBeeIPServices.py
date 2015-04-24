#!/bin/env python
# -*- coding: utf-8 -*-

import socket
import struct

class XBeeApplicationService(object):
    def __init__(self, address, timeout=0.0001):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((address,0xBEE))
        self.socket.settimeout(timeout)
        self.header = struct.Struct("!2H4B")

    def getPacket(self):
        try:
            (data,address)=self.socket.recvfrom(1500)
            Number1,Number2,PacketID,EncPad,CommandID,CommandOptions = \
                    self.header.unpack_from(data,offset=0)
            if CommandID == 0x00 :
                return data[self.header.size:]
        except socket.timeout :
            pass
        return None


if __name__ == '__main__' :
    service = XBeeApplicationService('192.168.191.1',1.0)
    while True:
        print service.getPacket()

