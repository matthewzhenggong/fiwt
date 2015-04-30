
import argparse
import numpy as np
import scipy.io as syio
import struct
import time

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF

class fileParser(object):
    def __init__(self):
        self.packHdr = struct.Struct(">BH")
        self.packHdr = struct.Struct(">BH")
        self.pack22 = struct.Struct(">B6H3H6HI6h")
        self.pack33 = struct.Struct(">B4H4HI4h")
        self.packA6 = struct.Struct(">BB4H16BI")
        self.pack44 = struct.Struct(">B4HI")

        self.head22 = np.array([ "T", "Enc1","Enc2","Enc3",
                    "GX","GY","GZ","AX","AY","AZ",
                    "Svo1","Svo2","Svo3","Svo4","Svo5","Svo6",
                    "Mot1","Mot2","Mot3","Mot4","Mot5","Mot6"
                    ], dtype=np.object)
        self.head33 = np.array([ "T", "Enc1","Enc2","Enc3","Enc4",
                "Mot1","Mot2","Mot3","Mot4" ], dtype=np.object)
        self.headA6 = np.array([ "T", "Svo1","Svo2","Svo3","Svo4", "Type" ],
                dtype=np.object)
        self.head44 = np.array([ "T", "ADC1","ADC2","ADC3","ADC4" ],
                dtype=np.object)

    def parse_data(self, data):
        if data[0] == '\x22':
            rslt = self.pack22.unpack(data)
            Svo1 = rslt[1]
            Svo2 = rslt[2]
            Svo3 = rslt[3]
            Svo4 = rslt[4]
            Svo5 = rslt[5]
            Svo6 = rslt[6]
            Enc1 = rslt[7]
            Enc2 = rslt[8]
            Enc3 = rslt[9]
            GX = Get14bit(rslt[10])*0.05
            GY = Get14bit(rslt[11])*-0.05
            GZ = Get14bit(rslt[12])*-0.05
            AX = Get14bit(rslt[13])*-0.003333
            AY = Get14bit(rslt[14])*0.003333
            AZ = Get14bit(rslt[15])*0.003333
            T = rslt[16]*1e-6
            Mot1 = rslt[17]
            Mot2 = rslt[18]
            Mot3 = rslt[19]
            Mot4 = rslt[20]
            Mot5 = rslt[21]
            Mot6 = rslt[22]
            self.data22.append( [T, Enc1,Enc2,Enc3,
                    GX,GY,GZ,AX,AY,AZ,
                    Svo1,Svo2,Svo3,Svo4,Svo5,Svo6,
                    Mot1,Mot2,Mot3,Mot4,Mot5,Mot6] )
        elif data[0] == '\x33':
            rslt = self.pack33.unpack(data)
            Svo1 = rslt[1]
            Svo2 = rslt[2]
            Svo3 = rslt[3]
            Svo4 = rslt[4]
            Enc1 = rslt[5]
            Enc2 = rslt[6]
            Enc3 = rslt[7]
            Enc4 = rslt[8]
            T = rslt[9]*1e-6
            Mot1 = rslt[10]
            Mot2 = rslt[11]
            Mot3 = rslt[12]
            Mot4 = rslt[13]
            self.data33.append( [T, Enc1,Enc2,Enc3,Enc4,
                Mot1,Mot2,Mot3,Mot4] )
        elif data[0] == '\xa6':
            rslt = self.packA6.unpack(data)
            itype = rslt[1]
            Svo1 = rslt[2]
            Svo2 = rslt[3]
            Svo3 = rslt[4]
            Svo4 = rslt[5]
            T = rslt[5+17]*1e-6
            self.dataA6.append( [T, Svo1,Svo2,Svo3,Svo4,itype] )
        elif data[0] == '\x44':
            rslt = self.pack44.unpack(data)
            ADC1 = rslt[1]
            ADC2 = rslt[2]
            ADC3 = rslt[3]
            ADC4 = rslt[4]
            T = rslt[5]*1e-6
            self.data44.append( [T, ADC1,ADC2,ADC3,ADC4] )

    def parse_file(self, filename):
        self.data22 = []
        self.data33 = []
        self.data44 = []
        self.dataA6 = []
        with open(filename, 'rb') as f:
            head = f.read(3)
            while len(head) == 3:
                header,length = self.packHdr.unpack(head)
                data = f.read(length)
                if len(data) == length:
                    self.parse_data(data)
                else:
                    break
                head = f.read(3)
        self.data22 = np.array(self.data22)
        self.data33 = np.array(self.data33)
        self.dataA6 = np.array(self.dataA6)
        self.data44 = np.array(self.data44)
        return {'data22':self.data22,'data33':self.data33,
                'head22':self.head22,'head33':self.head33,
                'headA6':self.headA6,'dataA6':self.dataA6,
                'head44':self.head44,'data44':self.data44,
                }

if __name__=='__main__' :
    parser = argparse.ArgumentParser(
        prog='recparse',
        description='parse rec data file')
    parser.add_argument('filenames', metavar='file',
            nargs='+', help='data filename')
    args = parser.parse_args()
    p = fileParser()
    for filename in args.filenames :
        mat_data = p.parse_file(filename)
        syio.savemat(filename+'.mat', mat_data)

