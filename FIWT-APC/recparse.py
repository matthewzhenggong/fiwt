
import argparse
import numpy as np
import scipy.io as syio
import struct
import time

import ExpData

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF

CODE_AC_MODEL_SERVO_POS = 0x22
CODE_AEROCOMP_SERVO_POS = 0x33
CODE_GNDBOARD_ADCM_READ = 0x44
CODE_GNDBOARD_MANI_READ = 0x45

CODE_AC_MODEL_SERV_CMD = 0xA5
CODE_AEROCOMP_SERV_CMD = 0xA6

packCODE_GNDBOARD_ADCM_READ = struct.Struct('>B4Hi2hI')
packCODE_GNDBOARD_MANI_READ = struct.Struct('>B2f')
packCODE_AC_MODEL_SERVO_POS = struct.Struct('>B6H3H6hI6h6hf')
packCODE_AEROCOMP_SERVO_POS = struct.Struct('>B4H4HI4h4hf')

class fileParser(object):
    def __init__(self):
        self.expData = ExpData.ExpData()
        self.packHdr = struct.Struct(">B3I2H")

        self.head22 = np.array(self.expData.getACMhdr(), dtype=np.object)
        self.head33 = np.array(self.expData.getCMPhdr(), dtype=np.object)
        self.headA6 = np.array(self.expData.getCMDhdr(), dtype=np.object)
        self.head44 = np.array(self.expData.getGNDhdr(), dtype=np.object)

    def parse_data(self, gen_ts, sent_ts, recv_ts, port, rf_data):
        if ord(rf_data[0]) == CODE_AC_MODEL_SERVO_POS:
            Id, ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5,ServoPos6, \
                EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
                ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
                ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
                CmdTime = packCODE_AC_MODEL_SERVO_POS.unpack(rf_data)
            self.expData.updateACM(ServoPos1,ServoPos2,ServoPos3,ServoPos4,ServoPos5, \
                ServoPos6, EncPos1,EncPos2,EncPos3, Gx,Gy,Gz, Nx,Ny,Nz, ts_ADC, \
                ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4,ServoCtrl5,ServoCtrl6, \
                ServoRef1,ServoRef2,ServoRef3,ServoRef4,ServoRef5,ServoRef6, \
                CmdTime)
            self.data22.append(self.expData.getACMdata() + [gen_ts, sent_ts, recv_ts, port])
        elif ord(rf_data[0]) == CODE_AEROCOMP_SERVO_POS:
            Id, ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
                EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
                ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
                ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
                CmdTime = packCODE_AEROCOMP_SERVO_POS.unpack(rf_data)
            self.expData.updateCMP(ServoPos1,ServoPos2,ServoPos3,ServoPos4, \
                EncPos1,EncPos2,EncPos3,EncPos4,ts_ADC, \
                ServoCtrl1,ServoCtrl2,ServoCtrl3,ServoCtrl4, \
                ServoRef1,ServoRef2,ServoRef3,ServoRef4, \
                CmdTime)
            self.data33.append(self.expData.getCMPdata() + [gen_ts, sent_ts, recv_ts, port])
        elif ord(rf_data[0]) == CODE_GNDBOARD_MANI_READ:
            Id, Vel, DP = packCODE_GNDBOARD_MANI_READ.unpack(rf_data)
            self.expData.updateMani(Vel, DP)
        elif ord(rf_data[0]) == CODE_GNDBOARD_ADCM_READ:
            Id, RigPos1, RigPos2, RigPos3, RigPos4, \
                    RigRollPos, RigPitchPos, RigYawPos, \
                    ADC_TimeStamp = packCODE_GNDBOARD_ADCM_READ.unpack(rf_data)
            self.expData.updateRigPos(RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp)
            self.data44.append(self.expData.getGNDdata() + [gen_ts, sent_ts, recv_ts, port])
        elif ord(rf_data[0]) == CODE_AC_MODEL_SERV_CMD:
            pass
        elif ord(rf_data[0]) == CODE_AEROCOMP_SERV_CMD:
            pass

    def parse_file(self, filename):
        self.data22 = []
        self.data33 = []
        self.data44 = []
        self.dataA6 = []
        with open(filename, 'rb') as f:
            head = f.read(17)
            while len(head) == 17:
                header,gen_ts, sent_ts, recv_ts, port, length \
                        = self.packHdr.unpack(head)
                data = f.read(length)
                if len(data) == length:
                    self.parse_data(gen_ts, sent_ts, recv_ts, port, data)
                else:
                    break
                head = f.read(17)
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

