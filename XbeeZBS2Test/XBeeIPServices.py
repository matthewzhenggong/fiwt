#!/bin/env python
# -*- coding: utf-8 -*-

import socket
import struct

class XBeeApplicationService(object):
    def __init__(self, address, ports, timeout=1e-7):
        all_ports = []
        self.port_struct = struct.Struct("!H")
        for i in ports :
            all_ports.append(self.port_struct.unpack(i.decode('hex'))[0])
        self.socket = []
        for i in all_ports :
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.bind((address,i))
            sock.settimeout(timeout)
            self.socket.append(sock)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((address,0xBEE))
        self.sock.settimeout(timeout)

        self.header = struct.Struct("!2H4B")
        self.header_RmtATCmd = self.header.pack(0x4242,0x0000,0x00,0x00,
                0x02,0x00)
        self.header_SerDatCmd = self.header.pack(0x4242,0x0000,0x00,0x00,
                0x00,0x00)

    def getPacket(self):
        while True :
            flag = False
            for sock in self.socket :
                try:
                    (data,address)=sock.recvfrom(1500)
                    yield {'id':'rx', 'source_addr':address, 'rf_data':data}
                    flag = True
                    break
                except socket.timeout :
                    pass
            if not flag :
                try:
                    (data,address)=self.sock.recvfrom(1500)
                    if len(data) <= 8 :
                        continue
                    Number1,Number2,PacketID,EncPad,CommandID,CommandOptions = \
                            self.header.unpack_from(data,offset=0)
                    if CommandID == 0x00 :
                        yield {'id':'rx', 'source_addr':address, 'rf_data':data[self.header.size:]}
                        break
                    elif CommandID == 0x82 :
                        yield {'id':'remote_at_response', 'source_addr':address,
                                'frame_id':ord(data[self.header.size]),
                                'command':data[self.header.size+1:self.header.size+3],
                                'status':ord(data[self.header.size+3]),
                                'parameter':data[self.header.size+4:],
                                }
                        break
                except socket.timeout :
                    yield None

    def sendSerialDataCommand(self, data) :
        data = self.header_SerDatCmd+data
        self.socket.sendto(data,(host,0xBEE))

    def sendConfigCommand(self, host, frame_id, options, command, parameter) :
        data = self.header_RmtATCmd+chr(frame_id)+options+command
        if parameter :
            data += parameter
        self.socket.sendto(data,(host,0xBEE))


if __name__ == '__main__' :
    service = XBeeApplicationService('192.168.191.1',1.0)
    while True:
        print service.getPacket()

