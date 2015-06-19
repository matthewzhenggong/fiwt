
import argparse
import numpy as np
import scipy.io as syio
import struct
import time
import glob
import ExpData
from ConfigParser import SafeConfigParser

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF

CODE_AC_MODEL_SERVO_POS = 0x22
CODE_AEROCOMP_SERVO_POS = 0x33
CODE_GNDBOARD_ADCM_READ = 0x44
CODE_GNDBOARD_MANI_READ = 0x45

CODE_AEROCOMP_SERV_CMD3 = 0xA7
CODE_AEROCOMP_SERV_CMD2 = 0xA5
CODE_AEROCOMP_SERV_CMD = 0xA6

CODE_GNDBOARD_STATS = 0x76
CODE_AC_MODEL_STATS = 0x77
CODE_AEROCOMP_STATS = 0x78

CODE_MANO_STATS = 0xE7

packCODE_GNDBOARD_ADCM_READ = struct.Struct('>B4Hi2hQ')
packCODE_GNDBOARD_MANI_READ = struct.Struct('>B2f')
packCODE_AC_MODEL_SERVO_POS = struct.Struct('>B6H3H6hQ6h6hf')
packCODE_AEROCOMP_SERVO_POS = struct.Struct('>B4H4HQ4h4hf')
packCODE_AEROCOMP_SERV_CMD = struct.Struct('>Bf7f')
packCODE_AEROCOMP_SERV_CMD2 = struct.Struct('>Bf8f')

packCODE_GNDBOARD_STATS = struct.Struct('>B2h2H')
packCODE_AEROCOMP_STATS = struct.Struct('>B2h3B3H')
packCODE_AC_MODEL_STATS = struct.Struct('>B2h3B3H')

packCODE_MANO_STATS = struct.Struct('>B3f')

class fileParser(object):
    def __init__(self, extra_simulink_inputs):
        self.expData = ExpData.ExpData(None,None, extra_simulink_inputs)
        self.packHdr = struct.Struct(">B3I2H")

        self.head22 = np.array(self.expData.getACMhdr(), dtype=np.object)
        self.head33 = np.array(self.expData.getCMPhdr(), dtype=np.object)
        self.headA7 = np.array(self.expData.getCMD3hdr(), dtype=np.object)
        self.headA5 = np.array(self.expData.getCMD2hdr(), dtype=np.object)
        self.headA6 = np.array(self.expData.getCMDhdr(), dtype=np.object)
        self.head44 = np.array(self.expData.getGNDhdr(), dtype=np.object)
        self.head76 = np.array(['ID', 'delay', 'offset',
            "gen_ts", "sent_ts", "recv_ts", "addr"], dtype=np.object)

    def parse_data(self, gen_ts, sent_ts, recv_ts, addr, rf_data):
        code = ord(rf_data[0])
        if code == CODE_AC_MODEL_SERVO_POS:
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
            self.data22.append(self.expData.getACMdata() + [gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AEROCOMP_SERVO_POS:
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
            self.data33.append(self.expData.getCMPdata() + [gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_GNDBOARD_MANI_READ:
            Id, Vel, DP = packCODE_GNDBOARD_MANI_READ.unpack(rf_data)
            self.expData.updateMani(Vel, DP)
        elif code == CODE_GNDBOARD_ADCM_READ:
            Id, RigPos1, RigPos2, RigPos3, RigPos4, \
                    RigRollPos, RigPitchPos, RigYawPos, \
                    ADC_TimeStamp = packCODE_GNDBOARD_ADCM_READ.unpack(rf_data)
            self.expData.updateRigPos(RigRollPos, RigPitchPos, RigYawPos, ADC_TimeStamp)
            self.data44.append(self.expData.getGNDdata() + [gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AEROCOMP_SERV_CMD2 :
            Id, TimeStamp, cmd_ts, dac, deac, dec, drc, dac_cmp, dec_cmp, drc_cmp = packCODE_AEROCOMP_SERV_CMD2.unpack(rf_data)
            TS = TimeStamp
            self.dataA5.append([TS, cmd_ts, dac, deac, dec, drc, dac_cmp, dec_cmp, drc_cmp,
                gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AEROCOMP_SERV_CMD3 :
            data = packCODE_AEROCOMP_SERV_CMD3.unpack(rf_data)
            self.dataA7.append(list(data[1:])+[gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AEROCOMP_SERV_CMD :
            Id, TimeStamp, dac, deac, dec, drc, dac_cmp, dec_cmp, drc_cmp = packCODE_AEROCOMP_SERV_CMD.unpack(rf_data)
            TS = TimeStamp
            self.dataA6.append([TS, dac, deac, dec, drc, dac_cmp, dec_cmp, drc_cmp,
                gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_GNDBOARD_STATS:
            Id, NTP_delay, NTP_offset, load_sen, load_msg = packCODE_GNDBOARD_STATS.unpack(rf_data)
            self.data76.append([CODE_GNDBOARD_STATS, NTP_delay, NTP_offset,
                gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AEROCOMP_STATS:
            Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AEROCOMP_STATS.unpack( rf_data)
            self.data76.append([CODE_AEROCOMP_STATS, NTP_delay, NTP_offset,
                gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_AC_MODEL_STATS:
            Id, NTP_delay, NTP_offset, B1, B2, B3, load_sen, load_rsen, load_msg = packCODE_AC_MODEL_STATS.unpack( rf_data)
            self.data76.append([CODE_AC_MODEL_STATS, NTP_delay, NTP_offset,
                gen_ts, sent_ts, recv_ts, addr])
        elif code == CODE_MANO_STATS:
            Id, TimeStamp, Vel, DP = packCODE_MANO_STATS.unpack( rf_data)
            self.expData.Vel = Vel
            self.expData.DP = DP
        else:
            print 'unkown code {:02x}'.format(code)

    def parse_file(self, filename):
        self.data22 = []
        self.data33 = []
        self.data44 = []
        self.dataA5 = []
        self.dataA6 = []
        self.dataA7 = []
        self.data76 = []
        with open(filename, 'rb') as f:
            head = f.read(17)
            while len(head) == 17:
                header,gen_ts, sent_ts, recv_ts, addr, length \
                        = self.packHdr.unpack(head)
                if header != 126 or addr not in [1,2,3,4] or length == 0:
                    print  ';'.join(['{:02x}'.format(ord(i)) for i in data])
                    c = f.read(1)
                    print  ':'.join(['{:02x}'.format(ord(i)) for i in head])
                    c = f.read(1)
                    print ':{:02x}'.format(ord(c)),
                    head = head[1:]+c
                    header,gen_ts, sent_ts, recv_ts, addr, length \
                            = self.packHdr.unpack(head)
                    while header != 126 or addr not in [1,2,3,4] or length == 0:
                        c = f.read(1)
                        print ':{:02x}'.format(ord(c)),
                        head = head[1:]+c
                        header,gen_ts, sent_ts, recv_ts, addr, length \
                                = self.packHdr.unpack(head)
                    print
                    data = f.read(length)
                    print  '.'.join(['{:02x}'.format(ord(i)) for i in data])
                    print
                else:
                    data = f.read(length)
                if len(data) == length:
                    self.parse_data(gen_ts, sent_ts, recv_ts, addr, data)
                else:
                    break
                head = f.read(17)
        self.data22 = np.array(self.data22)
        self.data33 = np.array(self.data33)
        self.dataA5 = np.array(self.dataA5)
        self.dataA6 = np.array(self.dataA6)
        self.dataA7 = np.array(self.dataA7)
        self.data44 = np.array(self.data44)
        self.data76 = np.array(self.data76)
        return {'data22':self.data22,'data33':self.data33,
                'head22':self.head22,'head33':self.head33,
                'headA5':self.headA5,'dataA5':self.dataA5,
                'headA6':self.headA6,'dataA6':self.dataA6,
                'headA7':self.headA7,'dataA7':self.dataA7,
                'head44':self.head44,'data44':self.data44,
                'head76':self.head76,'data76':self.data76,
                }

if __name__=='__main__' :
    parser = argparse.ArgumentParser(
        prog='recparse',
        description='parse rec data file')
    parser.add_argument('-c', '--config', metavar='file',
            nargs='?', default='config.ini', help='config filename')
    parser.add_argument('filenames', metavar='file',
            nargs='+', help='data filename')
    args = parser.parse_args()
    parser = SafeConfigParser()
    parser.read(args.config)
    extra_simulink_inputs = parser.get('simulink','extra').split()
    global packCODE_AEROCOMP_SERV_CMD3
    packCODE_AEROCOMP_SERV_CMD3 = struct.Struct('>Bf8f{}f'.format(len(extra_simulink_inputs)))
    p = fileParser(extra_simulink_inputs)
    for name in args.filenames :
        for filename in glob.glob(name):
            mat_data = p.parse_file(filename)
            syio.savemat(filename+'.mat', mat_data)
            np.savez_compressed(filename, **mat_data)

