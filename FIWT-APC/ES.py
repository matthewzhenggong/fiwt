
import socket
import PayloadPackage
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr1=('192.168.191.3',0x2000)
addr2=('192.168.191.4',0x2000)

A5 = struct.Struct('>BfB6H')

CMP_servo1_0 = 2020
CMP_servo2_0 = 2050
CMP_servo3_0 = 2000
CMP_servo4_0 = 2020
pack1 = A5.pack(0xA6, 0, 1, CMP_servo1_0, CMP_servo2_0, CMP_servo3_0,
        CMP_servo4_0,2000,2000)

ACM_servo1_0 = 1967
ACM_servo2_0 = 2259
ACM_servo3_0 = 2000
ACM_servo4_0 = 2200
ACM_servo5_0 = 1820
ACM_servo6_0 = 2110
pack2 = A5.pack(0xA5, 0, 1, ACM_servo1_0, ACM_servo2_0, ACM_servo3_0,
        ACM_servo4_0,ACM_servo5_0,ACM_servo6_0)

ts = 0
data1 = PayloadPackage.pack(pack1,ts)
data2 = PayloadPackage.pack(pack2,ts)

while True:
    sock.sendto(data1, addr1)
    sock.sendto(data2, addr2)

