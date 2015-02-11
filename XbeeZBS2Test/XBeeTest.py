#!/bin/env python
# -*- coding: utf-8 -*-

""" @package XBee Zigbee API Test Programme

Funtions include:
1) AT command;
2) Remote AT command
3) Send single TX request with response in const frequency
4) Send continuous TX requests with/without response in const frequency
5) Flow rate predict/measurement in Pps(Packet per Second) and bps(Bit per Second)
5) Echo RX response for range test
"""

import time
import wx
import string
import logging
import serial
import xbee
import sys
import traceback
import threading

log = logging.getLogger(__name__)

at_status = {
0 : 'OK',
1 : 'ERROR',
2 : 'Invalid Command',
3 : 'Invalid Parameter',
4 : 'Tx Failure',
}

moderm_status = {
0 : 'Hardware reset',
1 : 'Watchdog timer reset',
2 : 'Joined network (routers and end devices)',
3 : 'Disassociated',
6 : 'Coordinator started',
7 : 'Network security key was updated',
0x0D : 'Voltage supply limit exceeded (PRO S2B only)',
0x11 : 'Modem configuration changed while join in progress',
0x80 : 'stack error',
}

discovery_status = {
0x00 : 'No Discovery Overhead',
0x01 : 'Address Discovery',
0x02 : 'Route Discovery',
0x03 : 'Address and Route',
0x40 : 'Extended Timeout Discovery',
}

delivery_status = {
0x00 : 'Success',
0x01 : 'MAC ACK Failure',
0x02 : 'CCA Failure',
0x15 : 'Invalid destination endpoint',
0x21 : 'Network ACK Failure',
0x22 : 'Not Joined to Network',
0x23 : 'Self-addressed',
0x24 : 'Address Not Found',
0x25 : 'Route Not Found',
0x26 : 'Broadcast source failed to hear a neighbor relay the message',
0x2B : 'Invalid binding table index',
0x2C : 'Resource error lack of free buffers, timers, etc.',
0x2D : 'Attempted broadcast with APS transmission',
0x2E : 'Attempted unicast with APS transmission, but EE=0',
0x32 : 'Resource error lack of free buffers, timers, etc.',
0x74 : 'Data payload too large',
}

recv_opts = {
0x01 : 'Packet Acknowledged',
0x02 : 'Packet was a broadcast packet',
0x20 : 'Packet encrypted with APS encryption',
0x21 : 'Packet encrypted with APS encryption',
0x22 : 'Broadcast packet encrypted with APS encryption',
0x40 : 'Packet was sent from an end device',
0x41 : 'Packet was sent from an end device',
0x42 : 'Broadcast packet was sent from an end device',
0x61 : 'APS-encrypted Packet was sent from an end device',
0x62 : 'APS-encrypted Broadcast packet was sent from an end device',
}

ALPHA_ONLY = 1
DIGIT_ONLY = 2
HEX_ONLY = 3

class MyValidator(wx.PyValidator):
        def __init__(self, flag=None, pyVar=None):
                wx.PyValidator.__init__(self)
                self.flag = flag
                self.Bind(wx.EVT_CHAR, self.OnChar)
                self.hexs = string.digits+'abcdefABCDEF'

        def Clone(self):
                return MyValidator(self.flag)

        def Validate(self, win):
                tc = self.GetWindow()
                val = tc.GetValue()
                
                if self.flag == ALPHA_ONLY:
                        return all([i in string.letters for i in val])

                elif self.flag == DIGIT_ONLY:
                        return all([i in string.digits for i in val])

                elif self.flag == HEX_ONLY:
                        return all([i in self.hexs for i in val])

                return True


        def OnChar(self, event):
                key = event.GetKeyCode()

                if key < wx.WXK_SPACE or key == wx.WXK_DELETE or key > 255:
                        event.Skip()
                        return

                if self.flag == HEX_ONLY and chr(key) in self.hexs :
                        event.Skip()
                        return

                if self.flag == ALPHA_ONLY and chr(key) in string.letters:
                        event.Skip()
                        return

                if self.flag == DIGIT_ONLY and chr(key) in string.digits:
                        event.Skip()
                        return

                if self.flag == DIGIT_ONLY and chr(key) in '-':
                        event.Skip()
                        return

                if self.flag == DIGIT_ONLY and chr(key) in '.':
                        event.Skip()
                        return

                if not wx.Validator_IsSilent():
                        wx.Bell()

                # Returning without calling even.Skip eats the event before it
                # gets to the text control
                return

class RedirectError(object):
    def __init__(self):
        pass

    def write(self,string):
        string = string.strip('\r\n\t ')
        if string :
            log.error(string)

class RedirectInfo(object):
    def __init__(self):
        pass

    def write(self,string):
        string = string.strip('\r\n\t ')
        if string :
            log.info(string)

class RedirectText(object):
    def __init__(self,aWxTextCtrl):
        self.out=aWxTextCtrl

    def write(self,string):
        self.out.AppendText(string)

class MyFrame(wx.Frame):
        def __init__(
                        self, parent, ID, title, pos=wx.DefaultPosition,
                        size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE
                        ):

                wx.Frame.__init__(self, parent, ID, title, pos, size, style)

                panel = wx.Panel(self,-1)
                sizer = wx.BoxSizer(wx.VERTICAL)

                box = wx.BoxSizer(wx.HORIZONTAL)

                box.Add(wx.StaticText(panel, wx.ID_ANY, "Port:"), 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 1)
                self.txtPort = wx.TextCtrl(panel, -1, "COM3", size=(100,-1))
                box.Add(self.txtPort, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5)
                box.Add(wx.StaticText(panel, wx.ID_ANY, "Baudrate:"), 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 1)
                self.txtBR = wx.TextCtrl(panel, -1, "115200", size=(100,-1), validator = MyValidator(DIGIT_ONLY))
                box.Add(self.txtBR, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5)
                self.cbEsc = wx.CheckBox(panel, -1, "Esc")
                self.cbEsc.SetValue(True)
                self.cbEsc.SetToolTip( \
                     wx.ToolTip('Use escape characters(ATAP=2)') )
                box.Add(self.cbEsc, 0, wx.ALIGN_CENTER, 5)
                self.btnStart = wx.Button(panel, -1, "Start", size=(100,-1))
                box.Add(self.btnStart, 0, wx.ALIGN_CENTER, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                AT_CMD = ['MY','BD']
                ADDR64_LIST = ["0000000000000000","000000000000FFFF", \
                        "0013a200408A72F0", "0013a200408a72d2",
                        "0013A200408A72BB", "0013A200408A72AA"]
                ADDR16_LIST = ["0000","FFFE"]

                box = wx.BoxSizer(wx.HORIZONTAL)
                self.btnAT = wx.Button(panel, -1, "Send AT", size=(100,-1))
                self.btnAT.Enable(False)
                box.Add(self.btnAT, 0, wx.ALIGN_CENTER, 5)
                self.txtATcmd = wx.ComboBox(panel, -1, "MY", choices=AT_CMD, size=(50,-1))
		self.txtATcmd.SetToolTip(wx.ToolTip('AT Command in TWO characters'))
                box.Add(self.txtATcmd, 0, wx.ALIGN_CENTER, 5)
                self.txtATpar = wx.TextCtrl(panel, -1, "", size=(100,-1), validator = MyValidator(HEX_ONLY))
		self.txtATpar.SetToolTip(wx.ToolTip('Hexadecimal Parameter for AT Command to set.\nIf blanked, just get the parameter.'))
                box.Add(self.txtATpar, 0, wx.ALIGN_CENTER, 5)
                self.cbATqu = wx.CheckBox(panel, -1, "queued")
                box.Add(self.cbATqu, 0, wx.ALIGN_CENTER, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                box = wx.BoxSizer(wx.HORIZONTAL)
                box.Add(wx.StaticText(panel, wx.ID_ANY, "Remote 64bit Addr:"), 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 1)
                self.txtRmtATaddr64 = wx.ComboBox(panel, -1, \
                        "0000000000000000", choices=ADDR64_LIST, \
                        validator = MyValidator(HEX_ONLY))
                self.txtRmtATaddr64.SetToolTip( \
                        wx.ToolTip('''Set to the 64-bit address of the destination device in hexadecimal. The following addresses are also supported: 
0x0000000000000000 - Reserved 64-bit address for the coordinator
0x000000000000FFFF - Broadcast address ''')
                )
                box.Add(self.txtRmtATaddr64, 0, wx.ALIGN_CENTER, 5)
                box.Add(wx.StaticText(panel, wx.ID_ANY, "16bit Addr:"), 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 1)
                self.txtRmtATaddr16 = wx.ComboBox(panel, -1, \
                        "FFFE", choices=ADDR16_LIST, \
                        validator = MyValidator(HEX_ONLY))
                self.txtRmtATaddr16.SetToolTip( \
                        wx.ToolTip('''Set to the 16-bit address of the destination device in hexadecimal, if known. Set to 0xFFFE if the address is unknown, or if sending a broadcast.''')
                )
                box.Add(self.txtRmtATaddr16, 0, wx.ALIGN_CENTER, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                box = wx.BoxSizer(wx.HORIZONTAL)
                self.btnRmtAT = wx.Button(panel, -1, "Send RemoteAT", size=(100,-1))
                self.btnRmtAT.Enable(False)
                box.Add(self.btnRmtAT, 0, wx.ALIGN_CENTER, 5)
                self.txtRmtATcmd = wx.ComboBox(panel, -1, "MY", choices=AT_CMD, size=(50,-1))
		self.txtRmtATcmd.SetToolTip(wx.ToolTip('AT Command in TWO characters'))
                box.Add(self.txtRmtATcmd, 0, wx.ALIGN_CENTER, 5)
                self.txtRmtATpar = wx.TextCtrl(panel, -1, "", size=(100,-1), validator = MyValidator(HEX_ONLY))
		self.txtRmtATpar.SetToolTip(wx.ToolTip('Hexadecimal Parameter for remote AT Command to set.\nIf blanked, just get the parameter.'))
                box.Add(self.txtRmtATpar, 0, wx.ALIGN_CENTER, 5)
                self.txtRmtATopt = wx.TextCtrl(panel, -1, "02", size=(30,-1), validator = MyValidator(HEX_ONLY))
                self.txtRmtATopt.SetToolTip( \
                        wx.ToolTip('''Bitfield of supported transmission options
Supported values include the following:
0x01 - Disable retries and route repair
0x02 - Apply changes.
0x20 - Enable APS encryption (if EE=1)
0x40 - Use the extended transmission timeout
If apply changes is not enabled, then an AC command
must be sent before changes will take effect.
Enabling APS encryption presumes the source and
destination have been authenticated.
It also decreases the maximum number of RF payload
bytes by 4 (below the value reported by NP).
The extended transmission timeout is needed when
addressing sleeping end devices.
It increases the retry interval between retries to
compensate for end device polling.
See Chapter 4, Transmission Timeouts, Extended
Timeout for a description.
Unused bits must be set to 0.  ''')
                )
                box.Add(self.txtRmtATopt, 0, wx.ALIGN_CENTER, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)


                box = wx.BoxSizer(wx.HORIZONTAL)

                self.btnTX = wx.Button(panel, -1, "Send TX", size=(100,-1))
                self.btnTX.Enable(False)
                box.Add(self.btnTX, 0, wx.ALIGN_CENTER, 5)
                self.txtTX = wx.TextCtrl(panel, -1, "", size=(130,-1))
		self.txtTX.SetToolTip(wx.ToolTip('Text to be sent\nIf in continoous mode, the sent text will be prefixed with "P" and 5-digital index number.'))
                box.Add(self.txtTX, 1, wx.ALIGN_CENTER, 5)
                self.txtTXrad = wx.TextCtrl(panel, -1, "01", size=(30,-1), validator = MyValidator(HEX_ONLY))
                self.txtTXrad.SetToolTip( \
                        wx.ToolTip('''Sets maximum number of hops a broadcast transmission can occur.
If set to 0, the broadcast radius will be set to the maximum hops value.''')
                )
                box.Add(self.txtTXrad, 0, wx.ALIGN_CENTER, 5)
                self.txtTXopt = wx.TextCtrl(panel, -1, "01", size=(30,-1), validator = MyValidator(HEX_ONLY))
                self.txtTXopt.SetToolTip( \
                        wx.ToolTip('''Bitfield of supported transmission options. Supported values include the following:
0x01 - Disable retries and route repair
0x20 - Enable APS encryption (if EE=1)
0x40 - Use the extended transmission timeout
Enabling APS encryption presumes the source and destination have been authenticated.  I also decreases the maximum number of RF payload bytes by 4 (below the value reported by NP).
The extended transmission timeout is needed when addressing sleeping end devices.It also increases the retry interval between retries to compensate for end device polling.See Chapter 4, Transmission Timeouts, Extended Timeout for a description.
Unused bits must be set to 0.  ''')
                )
                box.Add(self.txtTXopt, 0, wx.ALIGN_CENTER, 5)

                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)


                box = wx.BoxSizer(wx.HORIZONTAL)

                self.btnTXc = wx.ToggleButton(panel, -1, "Send TX in", size=(100,-1))
                self.btnTXc.Enable(False)
                box.Add(self.btnTXc, 0, wx.ALIGN_CENTER, 5)
                self.txtTXfreq = wx.TextCtrl(panel, -1, "50", size=(50,-1), validator = MyValidator(DIGIT_ONLY))
		self.txtTXfreq.SetToolTip(wx.ToolTip('Frequency to send ping pack periodically.'))
                box.Add(self.txtTXfreq, 0, wx.ALIGN_CENTER, 5)
                box.Add(wx.StaticText(panel, wx.ID_ANY, "Hz"), 0,
                        wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 5)
                self.cbUseFrameId = wx.CheckBox(panel, -1, "Demand TX Response")
                self.cbUseFrameId.SetToolTip(wx.ToolTip("Use FrameID != 0"))
                box.Add(self.cbUseFrameId, 0, wx.ALIGN_CENTER|wx.RIGHT, 5)
                self.txtTXBR = wx.StaticText(panel, wx.ID_ANY, "")
                box.Add(self.txtTXBR, 1, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 1)

                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                box = wx.BoxSizer(wx.HORIZONTAL)
                self.txtRXSta = wx.StaticText(panel, wx.ID_ANY, "")
                box.Add(self.txtRXSta, 1, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND, 1)
                self.cbEcho = wx.CheckBox(panel, -1, "Echo RX pack")
		self.cbEcho.SetToolTip(wx.ToolTip('If checked, any received ping pack will be sent back.'))
                box.Add(self.cbEcho, 0, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                box = wx.BoxSizer(wx.HORIZONTAL)
                self.txtRX = wx.StaticText(panel, wx.ID_ANY, "")
                self.txtRX.SetForegroundColour((0,0,255))
                box.Add(self.txtRX, 1, wx.ALIGN_CENTER_VERTICAL|wx.EXPAND, 1)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)


                self.log_txt = wx.TextCtrl(panel, -1, "", size=(300,300), style=wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_RICH2)
                self.log_txt.SetFont(wx.Font(10, wx.FONTFAMILY_TELETYPE, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL))
                self.log = logging.getLogger(__name__)
                self.log.setLevel(logging.DEBUG)
                self.log_handle = logging.StreamHandler(RedirectText(self.log_txt))
                self.log_handle.setFormatter(logging.Formatter('%(asctime)s:%(message)s'))
                self.log.addHandler(self.log_handle)
                # redirect stdout to log
                sys.stdout=RedirectInfo()
                sys.stderr=RedirectError()
                sizer.Add(self.log_txt, 1, wx.ALL|wx.EXPAND, 1)

                box = wx.BoxSizer(wx.HORIZONTAL)
                self.btnClr = wx.Button(panel, -1, "Clear")
                box.Add(self.btnClr, 1, wx.ALIGN_CENTER, 5)
                sizer.Add(box, 0, wx.ALIGN_CENTRE|wx.ALL|wx.EXPAND, 1)

                panel.SetSizer(sizer)
                sizer.Fit(panel)
                self.Bind(wx.EVT_BUTTON, self.OnStart, self.btnStart)
                self.Bind(wx.EVT_BUTTON, self.OnAT, self.btnAT)
                self.Bind(wx.EVT_BUTTON, self.OnRmtAT, self.btnRmtAT)
                self.Bind(wx.EVT_BUTTON, self.OnTX, self.btnTX)
                self.Bind(wx.EVT_TOGGLEBUTTON, self.OnTXc, self.btnTXc)
                self.Bind(wx.EVT_BUTTON, self.OnClr, self.btnClr)
                self.Bind(wx.EVT_TEXT, self.OnCalBD, self.txtTX)
                self.Bind(wx.EVT_TEXT, self.OnCalBD, self.txtTXfreq)
                self.Bind(wx.EVT_CLOSE, self.OnClose)

                self.timer = None

        def OnCalBD(self, event):
            try :
                dt=1/(float(self.txtTXfreq.GetValue().encode()))
                if dt > 0 :
                    data_len=len(self.txtTX.GetValue().encode())
                    br2 = int((6+data_len)/dt+1)*10
                    br1 = int((28+6+data_len)/dt+1)*10
                    self.txtTXBR.SetLabel('Pack len={}+{}={} BR={}/{}'.format(\
                            28,6+data_len,28+6+data_len,br2,br1))
                    br=int(self.txtBR.GetValue().encode())
                    if br1 >= br :
                        self.txtTXBR.SetForegroundColour((255,0,0))
                    else :
                        self.txtTXBR.SetForegroundColour((0,0,255))
            except :
                #traceback.print_exc()
                pass

        def OnClr(self, event):
            self.log_txt.Clear()
            self.txtRXSta.SetLabel('')
            self.txtRX.SetLabel('')
            self.first_cnt = True
            self.last_cnt = 0
            self.arrv_cnt = 0
            self.arrv_bcnt = 0
            self.lost_cnt = 0
            self.dup_cnt = 0
            self.periodic_count = 0

        def OnClose(self, event):
            try :
                if self.timer :
                    self.timer.cancel()
                self.xbee.halt()
                self.serial_port.close()
            except :
                pass
            self.log.removeHandler(self.log_handle)
            event.Skip()

        def OnAT(self, event):
            command=self.txtATcmd.GetValue().encode()[:2]
            parameter = self.txtATpar.GetValue().encode()
            if len(parameter) == 0 :
                parameter = None
                self.log.info('get AT '+command)
            else :
                if len(parameter) % 2 == 1 :
                    parameter = '0'+parameter
                try :
                    parameter = parameter.decode('hex')
                except :
                    traceback.print_exc()
                    return
                self.log.debug('set AT '+command+'='+ \
                        ':'.join('{:02x}'.format(ord(c)) for c in parameter))
            self.at_frame_id=self.getFrameId()
            if self.cbATqu.GetValue() :
                self.xbee.queued_at(command=command, parameter=parameter,
                        frame_id=chr(self.at_frame_id))
            else :
                self.xbee.at(command=command, parameter=parameter,
                        frame_id=chr(self.at_frame_id))

        def OnRmtAT(self, event):
          try :
            addr64 = self.txtRmtATaddr64.GetValue().encode()[:16].decode('hex')
            addr16 = self.txtRmtATaddr16.GetValue().encode()[:4].decode('hex')
            options = self.txtRmtATopt.GetValue().encode()[:2].decode('hex')
            command=self.txtRmtATcmd.GetValue().encode()[:2]
            parameter = self.txtRmtATpar.GetValue().encode()
            if len(parameter) == 0 :
                parameter = None
                self.log.info('get AT '+command+' from ' \
                        + ':'.join('{:02x}'.format(ord(c)) for c in addr16)
                        + '('+':'.join('{:02x}'.format(ord(c)) for c in addr64)
                        + ') with option {:02x}'.format(ord(options))
                )
            else :
                if len(parameter) % 2 == 1 :
                    parameter = '0'+parameter
                parameter = parameter.decode('hex')
                self.log.debug('send AT '+command +'='+ \
                        ':'.join('{:02x}'.format(ord(c)) for c in parameter) \
                        +' to ' \
                        + ':'.join('{:02x}'.format(ord(c)) for c in addr16) \
                        + '('+':'.join('{:02x}'.format(ord(c)) for c in addr64)\
                        + ') with option {:02x}'.format(ord(options)) \
                        )
            self.remote_at_frame_id=self.getFrameId()
            self.xbee.remote_at(dest_addr_long=addr64, dest_addr=addr16,
                    options=options, command=command, parameter=parameter,
                    frame_id=chr(self.remote_at_frame_id))
          except :
            traceback.print_exc()

        def OnTX(self, event):
            data=self.txtTX.GetValue().encode()
            self.send(data)

        def OnTXc(self, event):
            if self.cbUseFrameId.GetValue() :
                if event.IsChecked() :
                    self.periodic_sending = 2
                    self.periodic_dt = 1.0 / float(self.txtTXfreq.GetValue().encode())
                    self.periodic_data=self.txtTX.GetValue().encode()
                    self.txtTXfreq.Disable()
                    self.txtTX.Disable()
                    self.periodic_count += 1
                    self.send('P{:05d}'.format(self.periodic_count)+self.periodic_data,
                            False)
                else :
                    self.periodic_sending = 0
                    self.txtTXfreq.Enable()
                    self.txtTX.Enable()
            else :
                if event.IsChecked() :
                    self.periodic_sending = 1
                    self.periodic_dt = 1.0 / float(self.txtTXfreq.GetValue().encode())
                    self.periodic_data=self.txtTX.GetValue().encode()
                    self.txtTXfreq.Disable()
                    self.txtTX.Disable()
                    self.periodic_send()
                else :
                    self.periodic_sending = 0
                    self.txtTXfreq.Enable()
                    self.txtTX.Enable()

        def periodic_send(self):
            tick = time.clock()
            self.periodic_count += 1
            self.send('P{:05d}'.format(self.periodic_count)+self.periodic_data,
                    True)
            if self.periodic_sending :
                threading.Timer(self.periodic_dt-(time.clock()-tick), self.periodic_send).start()

        def periodic_send2(self):
            tick = time.clock()
            dt = self.periodic_dt-(tick-self.tick)
            self.periodic_count += 1
            data = 'P{:05d}'.format(self.periodic_count)+self.periodic_data
            if dt < 0.000 :
                self.send(data, False)
                self.log.warning('Can not catch up given freq.(dt={:.4f})'.format(-dt))
            else :
                threading.Timer(dt, self.send, [data, False]).start()

        def send(self, data, no_response=False):
          try :
            if data :
                if no_response :
                    self.tx_frame_id=0
                elif self.sending :
                    return False
                else :
                    self.tx_frame_id=self.getFrameId()
                    self.sending = True

                addr64 = self.txtRmtATaddr64.GetValue().encode()[:16].decode('hex')
                addr16 = self.txtRmtATaddr16.GetValue().encode()[:4].decode('hex')
                broadcast_radius = self.txtTXrad.GetValue().encode()[:2]\
                        .decode('hex')
                options = self.txtTXopt.GetValue().encode()[:2].decode('hex')
                if not self.periodic_sending :
                    self.log.debug('send TX "'+data+'" to ' \
                            + ':'.join('{:02x}'.format(ord(c)) for c in addr16)
                            + '('+':'.join('{:02x}'.format(ord(c)) for c in addr64)
                            + ') with option {:02x}'.format(ord(options))
                            + ' & radius {:02x}'.format(ord(broadcast_radius))
                    )
                self.xbee.tx(dest_addr_long=addr64, dest_addr=addr16,
                        broadcast_radius=broadcast_radius, 
                        options=options, data=data,
                        frame_id=chr(self.tx_frame_id))
                self.tick = time.clock()
          except :
            traceback.print_exc()
            return False
          else :
            return True

        def getFrameId(self):
            fid = self.frame_id
            self.frame_id += 1
            if self.frame_id > 255 :
                self.frame_id = 1
            return fid

        def OnStart(self, event):
            self.btnStart.Enable(False)
            self.txtPort.Enable(False)
            self.txtBR.Enable(False)
            self.cbEsc.Enable(False)

            self.rec = open(time.strftime('rec%Y%m%d%H%M%S.dat'), 'wb')

            self.serial_port = serial.Serial( \
                    self.txtPort.GetValue().encode(), \
                    int(self.txtBR.GetValue()) )
            self.xbee = xbee.zigbee.ZigBee(self.serial_port, \
                    callback=self.get_frame_data, escaped=self.cbEsc.GetValue())

            self.at_frame_id = 0
            self.tx_frame_id = 0
            self.remote_at_frame_id =0
            self.sending = False
            self.first_cnt = True
            self.last_cnt = 0
            self.arrv_cnt = 0
            self.arrv_bcnt = 0
            self.lost_cnt = 0
            self.dup_cnt = 0
            self.periodic_count = 0
            self.periodic_sending = 0

            self.btnAT.Enable(True)
            self.btnRmtAT.Enable(True)
            self.btnTX.Enable(True)
            self.btnTXc.Enable(True)

            self.frame_id = 1
            print 'start'

        def get_frame_data(self, data):
            if data['id'] == 'rx' :
                try :
                    addr64 = data['source_addr_long']
                    addr16 = data['source_addr']
                    options = ord(data['options'])
                    rf_data = data['rf_data']
                    if rf_data[0] == 'P' :
                      cnt = int(rf_data[1:6])
                      if self.first_cnt :
                        self.first_cnt = False
                        self.start_cnt = time.clock()
                      else :
                        pnum = cnt - self.last_cnt
                        if pnum <= 0 :
                            self.dup_cnt += 1
                        else :
                            self.arrv_cnt += 1
                            self.arrv_bcnt += len(rf_data)
                            self.lost_cnt += pnum - 1
                            elapsed = time.clock() - self.start_cnt
                            self.txtRXSta.SetLabel('Ping {:0>5d}/{:0>3d}/{:0>3d} C{}/T{:<.2f} {:.1f}Pps/{:.0f}bps'\
					    .format(self.arrv_cnt, self.lost_cnt, self.dup_cnt, \
                                            cnt, elapsed, self.arrv_cnt/elapsed, self.arrv_bcnt*10/elapsed))
                            self.txtRX.SetLabel(rf_data[6:])
                      if self.cbEcho.GetValue() and options & 0x01 :
                            broadcast_radius = self.txtTXrad.GetValue().encode()[:2]\
                                    .decode('hex')
                            options = self.txtTXopt.GetValue().encode()[:2].decode('hex')
                            self.xbee.tx(dest_addr_long=addr64, dest_addr=addr16,
                                    broadcast_radius=broadcast_radius, 
                                    options=options, data=rf_data,
                                    frame_id='\x00')
                      self.last_cnt = cnt
                    else :
                        self.log.info( \
                    'RX:{}. Get "{}" from {}({})'.format(recv_opts[options], \
                    rf_data, \
                    ':'.join('{:02x}'.format(ord(c)) for c in addr16), \
                    ':'.join('{:02x}'.format(ord(c)) for c in addr64) )
                            )
                    self.rec.write(rf_data)
                except :
                    traceback.print_exc()
                    self.log.error(repr(data))
            elif data['id'] == 'tx_status' :
                self.sending = False
                try :
                    del_sta = ord(data['deliver_status'])
                    dis_sta = ord(data['discover_status'])
                    retries = ord(data['retries'])
                    if self.tx_frame_id != ord(data['frame_id']) :
                        self.log.error("TXResponse frame_id mismatch")
                    if self.periodic_sending == 2 :
                        self.periodic_send2()
                    else :
                        addr = data['dest_addr']
                        del_sta = delivery_status[del_sta]
                        dis_sta = discovery_status[dis_sta]
                        self.log.info('TXResponse:{} to {} ({:d} retries,{})'.format(\
                                del_sta, ':'.join('{:02x}'.format(ord(c)) for c in addr),
                                retries, dis_sta))

                except :
                    traceback.print_exc()
                    self.log.error(repr(data))
            elif data['id'] == 'at_response' :
              try :
                s = ord(data['status'])
                try :
                    parameter = data['parameter']
                    if not parameter :
                      parameter = ''
                except KeyError :
                    parameter = ''
                if self.at_frame_id != ord(data['frame_id']) :
                    self.log.error("ATResponse frame_id mismatch")
                self.log.info('ATResponse:{} {}={}'.format(at_status[s], \
                        data['command'], \
                        ':'.join('{:02x}'.format(ord(c)) for c in parameter))
                        )
              except :
                    traceback.print_exc()
                    self.log.error(repr(data))
            elif data['id'] == 'remote_at_response' :
              try :
                s = ord(data['status'])
                addr16 = data['source_addr']
                addr64 = data['source_addr_long']
                try :
                    parameter = data['parameter']
                    if not parameter :
                      parameter = ''
                except KeyError :
                    parameter = ''
                if self.remote_at_frame_id != ord(data['frame_id']) :
                    self.log.error("Remote ATResponse frame_id mismatch")
                self.log.info( \
            'ATResponse:{} {}={} from {}({})'.format(at_status[s], \
            data['command'], \
            ':'.join('{:02x}'.format(ord(c)) for c in parameter), \
            ':'.join('{:02x}'.format(ord(c)) for c in addr16), \
            ':'.join('{:02x}'.format(ord(c)) for c in addr64))
                        )
              except :
                traceback.print_exc()
                self.log.error(repr(data))
            elif data['id'] == 'status' :
                try :
                    s = ord(data['status'])
                    if s > 0x80 :
                        s = 0x80
                    s = moderm_status[s]
                    self.log.info('ModemStatus:'+s)
                except :
                    traceback.print_exc()
                    self.log.error(repr(data))
            else :
                self.log.info(repr(data))


if __name__ == '__main__' :
        app = wx.App(False)
        frame = MyFrame(None, wx.ID_ANY,'XBee ZigBee Station', size=(650,800))
        frame.Show(True)
        app.MainLoop()



