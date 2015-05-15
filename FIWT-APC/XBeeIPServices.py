#!/bin/env python
# -*- coding: utf-8 -*-

import socket
import struct

class XBeeApplicationService(object):
    def __init__(self, host):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.port = 0xBEE
        self.sock.bind((host,self.port))
        self.sock.setblocking(0)

        self.header = struct.Struct("!2H4B")
        self.header_RmtATCmd = self.header.pack(0x4242,0x0000,0x00,0x00,
                0x02,0x00)

    def getPacket(self):
        (data,address)=self.sock.recvfrom(1500)
        Number1,Number2,PacketID,EncPad,CommandID,CommandOptions = \
                self.header.unpack_from(data,offset=0)
        if CommandID == 0x00 :
            return {'id':'rx', 'source_addr':address, 'rf_data':data[self.header.size:]}
        elif CommandID == 0x82 :
            return {'id':'remote_at_response', 'source_addr':address,
                    'frame_id':ord(data[self.header.size]),
                    'command':data[self.header.size+1:self.header.size+3],
                    'status':ord(data[self.header.size+3]),
                    'parameter':data[self.header.size+4:],
                    }

    def sendConfigCommand(self, host, command, parameter=None,
            frame_id=0, options='\x02') :
        data = self.header_RmtATCmd+chr(frame_id)+options+command
        if parameter :
            data += parameter
        self.sock.sendto(data,(host,0xBEE))


if __name__ == '__main__' :
    service = XBeeApplicationService('192.168.191.1')
    while True:
        print service.getPacket()

