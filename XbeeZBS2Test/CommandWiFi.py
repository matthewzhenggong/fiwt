#!/bin/env python
# -*- coding: utf-8 -*-
""" @package XBee Zigbee API Test Programme

Funtions include:
1) AT command;
2) Remote AT command
3) Send single TX request with response in const frequency
4) Send continuous TX requests with/without response in const frequency
5) Flow rate predict/measurement in Pps(Packet per Second)
   and bps(Bit per Second)
6) Echo RX response for range test
"""

import os
import time
import wx
import string
import logging
import sys
import traceback
import threading
import struct
import socket
from ConfigParser import SafeConfigParser
from butter import Butter

from wx.lib.newevent import NewEvent
import XBeeIPServices
import PayloadPackage as pp

RxEvent, EVT_RSLT1 = NewEvent()
Rx2Event, EVT_RSLT2 = NewEvent()
RxStaEvent, EVT_STAT = NewEvent()
LogEvent, EVT_LOG = NewEvent()
RxCmpEvent, EVT_RSLT1C = NewEvent()
Rx2CmpEvent, EVT_RSLT2C = NewEvent()
RxGndEvent, EVT_RSLT1G = NewEvent()
RxAirEvent, EVT_RSLT1AIR = NewEvent()

log = logging.getLogger(__name__)

def Get14bit(val) :
    if val & 0x2000 :
        return -(((val & 0x1FFF)^0x1FFF)+1)
    else :
        return val & 0x1FFF



at_status = {
    0: 'OK',
    1: 'ERROR',
    2: 'Invalid Command',
    3: 'Invalid Parameter',
    4: 'Tx Failure',
}

moderm_status = {
    0: 'Hardware reset',
    1: 'Watchdog timer reset',
    2: 'Joined network (routers and end devices)',
    3: 'Disassociated',
    6: 'Coordinator started',
    7: 'Network security key was updated',
    0x0D: 'Voltage supply limit exceeded (PRO S2B only)',
    0x0E: 'Device Cloud connected',
    0x0F: 'Device Cloud disconnected',
    0x11: 'Modem configuration changed while join in progress',
    0x80: 'stack error',
}

discovery_status = {
    0x00: 'No Discovery Overhead',
    0x01: 'Address Discovery',
    0x02: 'Route Discovery',
    0x03: 'Address and Route',
    0x40: 'Extended Timeout Discovery',
}

delivery_status = {
    0x00: 'Success',
    0x01: 'MAC ACK Failure',
    0x02: 'CCA Failure',
    0x03: 'Transmission was purged because it was attempted before stack was completely up',
    0x15: 'Invalid destination endpoint',
    0x21: 'Network ACK Failure',
    0x22: 'Not Joined to Network',
    0x23: 'Self-addressed',
    0x24: 'Address Not Found',
    0x25: 'Route Not Found',
    0x26: 'Broadcast source failed to hear a neighbor relay the message',
    0x2B: 'Invalid binding table index',
    0x2C: 'Resource error lack of free buffers, timers, etc.',
    0x2D: 'Attempted broadcast with APS transmission',
    0x2E: 'Attempted unicast with APS transmission, but EE=0',
    0x32: 'Resource error lack of free buffers, timers, etc.',
    0x74: 'Data payload too large',
    0x76: 'Attempt to create a client socket fail',
    0x77: 'TCP connection to given IP address and port doesn\'t exist',
    0x78: 'Source port on a UDP transmission does not match a listening port on the transmitting module',
}

tx_status = {
    0x00: 'Success',
    0x01: 'No ACK received',
    0x02: 'CCA failure',
    0x03: 'Purged',
}

recv_opts = {
    0x01: 'Packet Acknowledged',
    0x02: 'Packet was a broadcast packet',
    0x20: 'Packet encrypted with APS encryption',
    0x21: 'Packet encrypted with APS encryption',
    0x22: 'Broadcast packet encrypted with APS encryption',
    0x40: 'Packet was sent from an end device',
    0x41: 'Packet was sent from an end device',
    0x42: 'Broadcast packet was sent from an end device',
    0x61: 'APS-encrypted Packet was sent from an end device',
    0x62: 'APS-encrypted Broadcast packet was sent from an end device',
}

ALPHA_ONLY = 1
DIGIT_ONLY = 2
HEX_ONLY = 3


class MyValidator(wx.PyValidator):
    def __init__(self, flag=None, pyVar=None):
        wx.PyValidator.__init__(self)
        self.flag = flag
        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.hexs = string.digits + 'abcdefABCDEF'

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

        if self.flag == HEX_ONLY and chr(key) in self.hexs:
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

    def write(self, string):
        string = string.strip('\r\n\t ')
        if string:
            log.error(string)


class RedirectInfo(object):
    def __init__(self):
        pass

    def write(self, string):
        string = string.strip('\r\n\t ')
        if string:
            log.info(string)


class RedirectText(object):
    def __init__(self, parent):
        self.parent = parent

    def write(self, string):
        wx.PostEvent(self.parent, LogEvent(log=string))

class MyFrame(wx.Frame):
    def __init__(self, parent, ID, title,
                 pos=wx.DefaultPosition,
                 size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)

        parser = SafeConfigParser()
        parser.read('config.ini')
        self.parser = parser

        panel = wx.Panel(self, -1)
        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnStart = wx.Button(panel, -1, "Start", size=(100, -1))
        box.Add(self.btnStart, 0, wx.ALIGN_CENTER, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Host:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtHost = wx.TextCtrl(panel, -1, parser.get('host','AP'), size=(100, -1))
        box.Add(self.txtHost, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.btnBaseTime = wx.Button(panel, -1, "Set Base Time", size=(100, -1))
        self.btnBaseTime.Enable(False)
        box.Add(self.btnBaseTime, 0, wx.ALIGN_CENTER, 5)
        self.txtRecName = wx.TextCtrl(panel, -1, parser.get('rec','prefix'), )
        box.Add(self.txtRecName, 1, wx.ALIGN_CENTER|wx.LEFT, 5)
        self.btnALLrec = wx.ToggleButton(panel, -1, "REC")
        self.btnALLrec.Enable(False)
        box.Add(self.btnALLrec, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        AT_CMD = ['MY', 'MK', 'GW', 'SH', 'SL', 'DL', 'C0', 'ID', 'AH', 'MA',
                'PL', 'BD', 'AI', 'WR', 'FR',]
        HOST_LIST = ["192.168.191.2", "192.168.191.3", "192.168.191.4"]
        self.PORT_LIST = ["2616", "2267", "2677", "2000"]


        box = wx.BoxSizer(wx.HORIZONTAL)
        self.target = 'GND'
        self.rbGND = wx.RadioButton(panel, wx.ID_ANY, "GND:",
                                  style=wx.RB_GROUP)
        box.Add(self.rbGND, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtGNDhost = wx.ComboBox(panel, -1, parser.get('host','GND'),
                                          choices=HOST_LIST)
        box.Add(self.txtGNDhost, 0, wx.ALIGN_CENTER, 5)
        self.txtGNDport = wx.ComboBox(panel, -1, "2616",
                choices=self.PORT_LIST[:-1], validator=MyValidator(HEX_ONLY))
        box.Add(self.txtGNDport, 0, wx.ALIGN_CENTER, 5)
        self.chkGNDsynct = wx.CheckBox(panel, -1, "")
        self.chkGNDsynct.SetValue(True)
        box.Add(self.chkGNDsynct, 0, wx.ALIGN_CENTER, 5)
        self.btnGNDsynct = wx.Button(panel, -1, "Sync Time")
        self.btnGNDsynct.Enable(False)
        box.Add(self.btnGNDsynct, 0, wx.ALIGN_CENTER, 5)
        self.txtGNDinfo = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 16))
        self.txtGNDinfo.SetForegroundColour((255, 55, 0))
        box.Add(self.txtGNDinfo, 1, wx.ALIGN_CENTER|wx.LEFT, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.rbACM = wx.RadioButton(panel, wx.ID_ANY, "ACM:")
        box.Add(self.rbACM, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtACMhost = wx.ComboBox(panel, -1, parser.get('host','ACM'),
                                          choices=HOST_LIST)
        box.Add(self.txtACMhost, 0, wx.ALIGN_CENTER, 5)
        self.txtACMport = wx.ComboBox(panel, -1, "2267",
                choices=self.PORT_LIST[:-1], validator=MyValidator(HEX_ONLY))
        box.Add(self.txtACMport, 0, wx.ALIGN_CENTER, 5)
        self.chkACMsynct = wx.CheckBox(panel, -1, "")
        self.chkACMsynct.SetValue(True)
        box.Add(self.chkACMsynct, 0, wx.ALIGN_CENTER, 5)
        self.btnACMsynct = wx.Button(panel, -1, "Sync Time")
        self.btnACMsynct.Enable(False)
        box.Add(self.btnACMsynct, 0, wx.ALIGN_CENTER, 5)
        self.txtACMbat = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 16))
        self.txtACMbat.SetForegroundColour((255, 55, 0))
        box.Add(self.txtACMbat, 1, wx.ALIGN_CENTER|wx.LEFT, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.rbCMP = wx.RadioButton(panel, wx.ID_ANY, "CMP:")
        box.Add(self.rbCMP, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtCMPhost = wx.ComboBox(panel, -1, parser.get('host','CMP'),
                                          choices=HOST_LIST)
        box.Add(self.txtCMPhost, 0, wx.ALIGN_CENTER, 5)
        self.txtCMPport = wx.ComboBox(panel, -1, "2677",
                choices=self.PORT_LIST[:-1], validator=MyValidator(HEX_ONLY))
        box.Add(self.txtCMPport, 0, wx.ALIGN_CENTER, 5)
        self.chkCMPsynct = wx.CheckBox(panel, -1, "")
        self.chkCMPsynct.SetValue(True)
        box.Add(self.chkCMPsynct, 0, wx.ALIGN_CENTER, 5)
        self.btnCMPsynct = wx.Button(panel, -1, "Sync Time")
        self.btnCMPsynct.Enable(False)
        box.Add(self.btnCMPsynct, 0, wx.ALIGN_CENTER, 5)
        self.txtCMPbat = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 16))
        self.txtCMPbat.SetForegroundColour((255, 55, 0))
        box.Add(self.txtCMPbat, 1, wx.ALIGN_CENTER|wx.LEFT, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnRmtAT = wx.Button(panel, -1, "Send RemoteAT", size=(100, -1))
        self.btnRmtAT.Enable(False)
        box.Add(self.btnRmtAT, 0, wx.ALIGN_CENTER, 5)
        self.txtRmtATcmd = wx.ComboBox(panel, -1, "MY",
                                       choices=AT_CMD,
                                       size=(50, -1))
        self.txtRmtATcmd.SetToolTip(wx.ToolTip('''AT Command in TWO characters :
MY - IP Network Address
MK - IP Address Mask
GW - Gateway IP address
SH - Serial Number High
SL - Serial Number Low
DL - Destination Address Low
C0 - source IP port
ID - SSID
AH - Network Type
MA - IP Addressing Mode. 0=DHCP;1=Static
PL - Power Level
BD - baudrate
AI - Association Indication
WR - write to flash
FR - Software Reset
'''))
        box.Add(self.txtRmtATcmd, 0, wx.ALIGN_CENTER, 5)
        self.txtRmtATpar = wx.TextCtrl(panel, -1, "",
                                       size=(100, -1),
                                       validator=MyValidator(HEX_ONLY))
        self.txtRmtATpar.SetToolTip(wx.ToolTip(
            'Hexadecimal Parameter for remote AT Command to set.\n'
            'If blanked, just get the parameter.'))
        box.Add(self.txtRmtATpar, 0, wx.ALIGN_CENTER, 5)
        self.txtRmtATopt = wx.TextCtrl(panel, -1, "02",
                                       size=(30, -1),
                                       validator=MyValidator(HEX_ONLY))
        self.txtRmtATopt.SetToolTip(
            wx.ToolTip('''Bitfield of supported transmission options
Supported values include the following:
0x00 - Disable retries and route repair
0x02 - Apply changes. '''))
        box.Add(self.txtRmtATopt, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)

        self.btnTX = wx.Button(panel, -1, "Send Ping", size=(100, -1))
        self.btnTX.Enable(False)
        box.Add(self.btnTX, 0, wx.ALIGN_CENTER, 5)
        self.txtTX = wx.TextCtrl(panel, -1, "", size=(130, -1))
        self.txtTX.SetToolTip(wx.ToolTip(
            'Text to be sent\nIf in continoous mode, the sent text will be prefixed with "P" and 5-digital index number.'))
        box.Add(self.txtTX, 1, wx.ALIGN_CENTER, 5)
        self.txtTXrad = wx.TextCtrl(panel, -1, "01",
                                    size=(30, -1),
                                    validator=MyValidator(HEX_ONLY))
        self.txtTXrad.SetToolTip(wx.ToolTip(
            '''Sets maximum number of hops a broadcast transmission can occur.
If set to 0, the broadcast radius will be set to the maximum hops value.'''))
        box.Add(self.txtTXrad, 0, wx.ALIGN_CENTER, 5)
        self.txtTXopt = wx.TextCtrl(panel, -1, "01",
                                    size=(30, -1),
                                    validator=MyValidator(HEX_ONLY))
        self.txtTXopt.SetToolTip(wx.ToolTip(
            '''Bitfield of supported transmission options. Supported values include the following:
0x01 - Disable retries and route repair
0x20 - Enable APS encryption (if EE=1)
0x40 - Use the extended transmission timeout
Enabling APS encryption presumes the source and destination have been authenticated.  I also decreases the maximum number of RF payload bytes by 4 (below the value reported by NP).
The extended transmission timeout is needed when addressing sleeping end devices.It also increases the retry interval between retries to compensate for end device polling.See Chapter 4, Transmission Timeouts, Extended Timeout for a description.
Unused bits must be set to 0.  '''))
        box.Add(self.txtTXopt, 0, wx.ALIGN_CENTER, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)

        self.btnTM = wx.Button(panel, -1, "Servo Command", size=(100, -1))
        self.btnTM.Enable(False)
        box.Add(self.btnTM, 0, wx.ALIGN_CENTER, 5)
        boxV = wx.BoxSizer(wx.VERTICAL)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.InputType = wx.Choice(panel, wx.ID_ANY,
            choices=['Reset','Step','Doublet','3-2-1-1','Ramp',
                'pitch rate','open loop','LinFreq Sweep','ExpFreq Sweep'])
        self.InputType.SetSelection(0)
        boxH.Add(self.InputType, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "StartTime"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.StartTime = wx.TextCtrl(panel, -1, "500",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        self.StartTime.SetToolTip(wx.ToolTip('milliseconds'))
        boxH.Add(self.StartTime, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "TimeDelta"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.TimeDelta = wx.TextCtrl(panel, -1, "500",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        self.TimeDelta.SetToolTip(wx.ToolTip('milliseconds'))
        boxH.Add(self.TimeDelta, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "NofCycles"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.NofCycles = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.NofCycles, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move1 = wx.CheckBox(panel, -1, "CH1")
        boxH.Add(self.Srv2Move1, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef1 = wx.TextCtrl(panel, -1, "1967",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef1, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue1 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue1, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue1 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue1, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign1 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign1, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move2 = wx.CheckBox(panel, -1, "CH2")
        boxH.Add(self.Srv2Move2, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef2 = wx.TextCtrl(panel, -1, "2259",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef2, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue2 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue2, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue2 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue2, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign2 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign2, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move3 = wx.CheckBox(panel, -1, "CH3")
        boxH.Add(self.Srv2Move3, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef3 = wx.TextCtrl(panel, -1, "2000",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef3, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue3 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue3, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue3 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue3, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign3 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign3, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move4 = wx.CheckBox(panel, -1, "CH4")
        boxH.Add(self.Srv2Move4, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef4 = wx.TextCtrl(panel, -1, "1700",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef4, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue4 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue4, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue4 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue4, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign4 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign4, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move5 = wx.CheckBox(panel, -1, "CH5")
        boxH.Add(self.Srv2Move5, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef5 = wx.TextCtrl(panel, -1, "1820",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef5, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue5 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue5, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue5 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue5, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign5 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign5, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        boxH = wx.BoxSizer(wx.HORIZONTAL)
        self.Srv2Move6 = wx.CheckBox(panel, -1, "CH6")
        boxH.Add(self.Srv2Move6, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "ServoRef"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef6 = wx.TextCtrl(panel, -1, "2067",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef6, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue6 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue6, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue6 = wx.TextCtrl(panel, -1, "100",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MinValue6, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Sign"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.Sign6 = wx.TextCtrl(panel, -1, "1",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.Sign6, 0, wx.ALIGN_CENTER, 5)
        boxV.Add(boxH, 0, wx.ALIGN_CENTER, 5)

        box.Add(boxV, 0, wx.ALIGN_CENTER, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        sub_panel = wx.Panel(panel, -1)
        sub_panel.SetDoubleBuffered(True)
        sub_sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)

        self.txtRXSta = wx.StaticText(sub_panel, wx.ID_ANY, "")
        box.Add(self.txtRXSta, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 32))
        self.txtRX.SetForegroundColour((0, 0, 255))
        box.Add(self.txtRX, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX2 = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 16))
        self.txtRX2.SetForegroundColour((255, 55, 0))
        box.Add(self.txtRX2, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX_CMP = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 32))
        self.txtRX_CMP.SetForegroundColour((0, 0, 255))
        box.Add(self.txtRX_CMP, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX2_CMP = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 16))
        self.txtRX2_CMP.SetForegroundColour((255, 55, 0))
        box.Add(self.txtRX2_CMP, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX_GND = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 16))
        self.txtRX_GND.SetForegroundColour((155, 55, 0))
        box.Add(self.txtRX_GND, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)
        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX_AIR = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(32, 16))
        self.txtRX_AIR.SetForegroundColour((155, 55, 0))
        box.Add(self.txtRX_AIR, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sub_sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        sub_panel.SetSizer(sub_sizer)
        #sub_sizer.Fit(sub_panel)
        sizer.Add(sub_panel, 0, wx.ALL | wx.EXPAND, 1)

        self.log_txt = wx.TextCtrl(
            panel, -1, "",
            size=(300, 300),
            style=wx.TE_MULTILINE | wx.TE_READONLY | wx.TE_RICH2)
        self.log_txt.SetFont(wx.Font(10, wx.FONTFAMILY_TELETYPE,
                                     wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL))
        self.log = logging.getLogger(__name__)
        self.log.setLevel(logging.INFO)
        self.log_handle = logging.StreamHandler(RedirectText(self))
        self.log_handle.setFormatter(
            logging.Formatter('%(asctime)s:%(message)s'))
        self.log.addHandler(self.log_handle)
        # redirect stdout to log
        sys.stdout = RedirectInfo()
        sys.stderr = RedirectError()
        sizer.Add(self.log_txt, 1, wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnClr = wx.Button(panel, -1, "Clear")
        box.Add(self.btnClr, 1, wx.ALIGN_CENTER, 5)
        self.btnSaveLog = wx.Button(panel, -1, "Save Log")
        box.Add(self.btnSaveLog, 1, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        panel.SetSizer(sizer)
        sizer.Fit(panel)
        self.Bind(wx.EVT_BUTTON, self.OnStart, self.btnStart)
        self.Bind(wx.EVT_BUTTON, self.OnRmtAT, self.btnRmtAT)
        self.Bind(wx.EVT_BUTTON, self.OnSyncACM, self.btnACMsynct)
        self.Bind(wx.EVT_BUTTON, self.OnSyncCMP, self.btnCMPsynct)
        self.Bind(wx.EVT_BUTTON, self.OnSyncGND, self.btnGNDsynct)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.OnRecALL, self.btnALLrec)
        self.Bind(wx.EVT_BUTTON, self.OnSetBaseTime, self.btnBaseTime)
        self.Bind(wx.EVT_BUTTON, self.OnTX, self.btnTX)
        self.Bind(wx.EVT_BUTTON, self.OnTestMotor, self.btnTM)
        self.Bind(wx.EVT_BUTTON, self.OnClr, self.btnClr)
        self.Bind(wx.EVT_BUTTON, self.OnSaveLog, self.btnSaveLog)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(EVT_RSLT1, self.OnRX)
        self.Bind(EVT_RSLT2, self.OnRX2)
        self.Bind(EVT_RSLT1C, self.OnRX_CMP)
        self.Bind(EVT_RSLT2C, self.OnRX2_CMP)
        self.Bind(EVT_RSLT1G, self.OnRX_GND)
        self.Bind(EVT_RSLT1AIR, self.OnRX_AIR)
        self.Bind(EVT_STAT, self.OnRXSta)
        self.Bind(EVT_LOG, self.OnLog)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseACM, self.rbACM)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseCMP, self.rbCMP)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseGND, self.rbGND)

        self.fileALL = None
        self.butt = [Butter()]*4

    def OnRecALL(self, event) :
        if event.IsChecked():
            self.filename = time.strftime('{}%Y%m%d%H%M%S.dat'.format(
                self.txtRecName.GetValue()))
            self.fileALL = open(self.filename, 'wb')
            self.log.info('Recording to {}.'.format(self.filename))
        else:
            self.fileALL.close()
            self.log.info('Stop Recording to {}.'.format(self.filename))
            self.fileALL = None

    def OnLog(self, event) :
        self.log_txt.AppendText(event.log)

    def OnSetBaseTime(self, event) :
        self.ntp_tick0 = time.clock()
        self.log.info('Set Local T0')

    def OnSyncGND(self, event) :
        self.rbGND.SetValue(True)
        self.target = 'GND'

        code = 0 if self.rbGND.GetValue() else 5
        self.btnALLrec.Enable(True)

        if not hasattr(self, 'ntp_tick0') :
            self.OnSetBaseTime(None)
        self.ntp_T0 = int((time.clock() - self.ntp_tick0)*1e6)
        self.send(self.packNTP.pack(ord('S'),code,self.ntp_T0))
        self.log.info('Local T0={}us'.format(self.ntp_T0))

    def OnSyncACM(self, event) :
        self.rbACM.SetValue(True)
        self.target = 'ACM'

        code = 0 if self.rbACM.GetValue() else 5
        self.btnALLrec.Enable(True)

        if not hasattr(self, 'ntp_tick0') :
            self.OnSetBaseTime(None)
        self.ntp_T0 = int((time.clock() - self.ntp_tick0)*1e6)
        self.send(self.packNTP.pack(ord('S'),code,self.ntp_T0))
        self.log.info('Local T0={}us'.format(self.ntp_T0))

    def OnSyncCMP(self, event) :
        self.rbCMP.SetValue(True)
        self.target = 'CMP'

        code = 0 if self.rbCMP.GetValue() else 5
        self.btnALLrec.Enable(True)

        if not hasattr(self, 'ntp_tick0') :
            self.OnSetBaseTime(None)
        self.ntp_T0 = int((time.clock() - self.ntp_tick0)*1e6)
        self.send(self.packNTP.pack(ord('S'),code,self.ntp_T0))
        self.log.info('Local T0={}us'.format(self.ntp_T0))

    def OnRX(self, event) :
        self.txtRX.SetLabel(event.txt)

    def OnRX2(self, event) :
        self.txtRX2.SetLabel(event.txt)

    def OnRX_AIR(self, event) :
        self.txtRX_AIR.SetLabel(event.txt)

    def OnRX_GND(self, event) :
        self.txtRX_GND.SetLabel(event.txt)

    def OnRX_CMP(self, event) :
        self.txtRX_CMP.SetLabel(event.txt)

    def OnRX2_CMP(self, event) :
        self.txtRX2_CMP.SetLabel(event.txt)

    def OnRXSta(self, event) :
        self.txtRXSta.SetLabel(event.txt)

    def OnChooseACM(self, event):
        self.target = 'ACM'
        self.log.info('Target {}'.format(self.target))

    def OnChooseCMP(self, event):
        self.target = 'CMP'
        self.log.info('Target {}'.format(self.target))

    def OnChooseGND(self, event):
        self.target = 'GND'
        self.log.info('Target {}'.format(self.target))

    def OnSaveLog(self, event):
        dlg = wx.FileDialog(
            self, message="Save log as ...", defaultDir=os.getcwd(),
            defaultFile="log.txt", wildcard="Text file(*.txt)|*.txt",
            style=wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK:
            self.log_txt.SaveFile(dlg.GetPath())

    def OnClr(self, event):
        self.log_txt.Clear()
        self.txtRXSta.SetLabel('')
        self.txtRX.SetLabel('')
        self.txtRX2.SetLabel('')
        self.txtRX_AIR.SetLabel('')
        self.txtRX_GND.SetLabel('')
        self.txtRX_CMP.SetLabel('')
        self.txtRX2_CMP.SetLabel('')
        self.txtACMbat.SetLabel('')
        self.txtCMPbat.SetLabel('')
        self.txtGNDinfo.SetLabel('')
        self.first_cnt = True
        self.arrv_cnt = 0
        self.arrv_cnt_22 = 0
        self.arrv_cnt_33 = 0
        self.arrv_cnt_44 = 0
        self.last_arrv_cnt = 0
        self.arrv_bcnt = 0
        self.periodic_count = 0

    def OnClose(self, event):
        self.log.info("clean_up" + traceback.format_exc())
        try:
            self.halting = True
            time.sleep(0.2)
        except:
            pass

        parser = self.parser
        parser.set('host','AP', self.txtHost.GetValue())
        parser.set('host','GND', self.txtGNDhost.GetValue())
        parser.set('host','ACM', self.txtACMhost.GetValue())
        parser.set('host','CMP', self.txtCMPhost.GetValue())
        parser.set('rec','prefix', self.txtRecName.GetValue())
        cfg = open('config.ini', 'w')
        parser.write(cfg)
        cfg.close()

        if self.fileALL:
            self.fileALL.close()

        self.log.removeHandler(self.log_handle)
        event.Skip()

    def OnRmtAT(self, event):
        try:
            if self.target == 'ACM' :
                remote_host = self.txtACMhost.GetValue().encode()
            elif self.target == 'CMP' :
                remote_host = self.txtCMPhost.GetValue().encode()
            else :
                remote_host = self.txtGNDhost.GetValue().encode()
            options = self.txtRmtATopt.GetValue().encode()[:2].decode('hex')[0]
            command = self.txtRmtATcmd.GetValue().encode()[:2]
            parameter = self.txtRmtATpar.GetValue().encode()
            if len(parameter) == 0:
                parameter = None
                self.log.info('get AT ' + command + ' from ' + remote_host +
                              ' with option {:02x}'.format(ord(options)))
            else:
                if len(parameter) % 2 == 1:
                    parameter = '0' + parameter
                parameter = parameter.decode('hex')
                self.log.debug('send AT ' + command + '=' + ':'.join(
                    '{:02x}'.format(ord(c)) for c in parameter) + ' to '
                    + remote_host + ' with option {:02x}'.format(ord(options)))
            self.frame_id = self.getFrameId()
            self.service.sendConfigCommand(remote_host, command, parameter,
                    frame_id=self.frame_id, options=options)
        except:
            traceback.print_exc()

    def OnTestMotor(self, event):
        InputType = self.InputType.GetSelection()+1
        if InputType == 1:
            ServoRef = [int(self.ServoRef1.GetValue()),
                        int(self.ServoRef2.GetValue()),
                        int(self.ServoRef3.GetValue()),
                        int(self.ServoRef4.GetValue()),
                        int(self.ServoRef5.GetValue()),
                        int(self.ServoRef6.GetValue()) ]
            data = struct.pack('>2B6H', 0xA5, InputType, *ServoRef)
        else :
            Srv2Move = (1 if self.Srv2Move1.GetValue() else 0) \
                     | (2 if self.Srv2Move2.GetValue() else 0) \
                     | (4 if self.Srv2Move3.GetValue() else 0) \
                     | (8 if self.Srv2Move4.GetValue() else 0) \
                     | (16 if self.Srv2Move5.GetValue() else 0) \
                     | (32 if self.Srv2Move6.GetValue() else 0)
            others = [ int(self.MaxValue1.GetValue()),
                       int(self.MaxValue2.GetValue()),
                       int(self.MaxValue3.GetValue()),
                       int(self.MaxValue4.GetValue()),
                       int(self.MaxValue5.GetValue()),
                       int(self.MaxValue6.GetValue()),
                       int(self.MinValue1.GetValue()),
                       int(self.MinValue2.GetValue()),
                       int(self.MinValue3.GetValue()),
                       int(self.MinValue4.GetValue()),
                       int(self.MinValue5.GetValue()),
                       int(self.MinValue6.GetValue()),
                       int(self.Sign1.GetValue()),
                       int(self.Sign2.GetValue()),
                       int(self.Sign3.GetValue()),
                       int(self.Sign4.GetValue()),
                       int(self.Sign5.GetValue()),
                       int(self.Sign6.GetValue()),
                     ]
            starttime = int(self.StartTime.GetValue())
            deltatime = int(self.TimeDelta.GetValue())
            nofcyc = int(self.NofCycles.GetValue())
            data = struct.pack('>3B2HB6B6B6B', 0xA5, InputType, Srv2Move,
                    starttime, deltatime, nofcyc, *others)
            if InputType == 1 :
                self.OutputCnt = starttime*nofcyc/20+3+10
            elif InputType == 2 :
                self.OutputCnt = (starttime+deltatime)*nofcyc/20+3+10
            elif InputType == 7 :
                self.OutputCnt = starttime*nofcyc/20+3+10
            self.OutputSrv2Move = Srv2Move
            txt = '#Time,'
            if self.OutputSrv2Move & 1 :
                txt += 'Servo1,Ctrl1,'
            if self.OutputSrv2Move & 2 :
                txt += 'Servo2,Ctrl2,'
            if self.OutputSrv2Move & 4 :
                txt += 'Servo3,Ctrl3,'
            if self.OutputSrv2Move & 8 :
                txt += 'Servo4,Ctrl4,'
            if self.OutputSrv2Move & 16 :
                txt += 'Servo5,Ctrl5,'
            if self.OutputSrv2Move & 32 :
                txt += 'Servo6,Ctrl6,'
            self.log.info(txt)
        self.send(data)

    def OnTX(self, event):
        data = self.txtTX.GetValue().encode()
        self.send('P\x00'+data)
        self.ping_tick = time.clock()

    def send(self, data):
        try:
            if data:
                if self.target == 'ACM' :
                    remote_host = self.txtACMhost.GetValue().encode()
                    remote_port = self.port_struct.unpack(
                            self.txtACMport.GetValue().decode('hex'))[0]
                elif self.target == 'CMP' :
                    remote_host = self.txtCMPhost.GetValue().encode()
                    remote_port = self.port_struct.unpack(
                            self.txtCMPport.GetValue().decode('hex'))[0]
                else :
                    remote_host = self.txtGNDhost.GetValue().encode()
                    remote_port = self.port_struct.unpack(
                            self.txtGNDport.GetValue().decode('hex'))[0]
                self.tx_socket.sendto(pp.pack(data), (remote_host, remote_port))
                self.tick = time.clock()
        except:
            traceback.print_exc()
            return False
        else:
            return True

    def getFrameId(self):
        fid = self.frame_id
        self.frame_id += 1
        if self.frame_id > 255:
            self.frame_id = 1
        return fid

    def OnStart(self, event):
        self.btnStart.Enable(False)
        self.txtHost.Enable(False)

        self.starting = True

        self.frame_id = 0
        self.first_cnt = True
        self.arrv_cnt = 0
        self.arrv_cnt_22 = 0
        self.arrv_cnt_44 = 0
        self.arrv_cnt_33 = 0
        self.last_arrv_cnt = 0
        self.arrv_bcnt = 0
        self.periodic_count = 0

        self.frame_id = 1

        self.port_struct = struct.Struct("!H")
        self.pack22 = struct.Struct(">B6H3H6HI6h")
        self.pack77 = struct.Struct(">B3HI")
        self.pack78 = struct.Struct(">B3HI")
        self.packAA = struct.Struct(">BHI")
        self.pack88 = struct.Struct(">B3BI")
        self.pack33 = struct.Struct(">B4H4HI4h")
        self.pack44 = struct.Struct(">B4HI")
        self.packNTP = struct.Struct(">2BI")
        self.packNTP1 = struct.Struct(">2I")
        self.packNTP2 = struct.Struct(">2B2I")
        self.packNTP3 = struct.Struct(">IhiI")
        self.packNTP13 = struct.Struct(">bhi")
        self.packHdr = struct.Struct(">BH")
        self.packT = struct.Struct(">I")
        self.packMeter = struct.Struct(">B2f")
        self.ch = 0
        self.test_motor_ticks = 0
        self.starting = False

        self.OutputSrv2Move = 0
        self.OutputCnt = 0

        self.btnRmtAT.Enable(True)
        self.btnACMsynct.Enable(True)
        self.btnCMPsynct.Enable(True)
        self.btnGNDsynct.Enable(True)
        self.btnBaseTime.Enable(True)
        self.btnTX.Enable(True)
        self.btnTM.Enable(True)

        self.halting = False
        try:
            host = self.txtHost.GetValue().encode()
            self.service = XBeeIPServices.XBeeApplicationService(host)
            self.thread = threading.Thread(target=self.run)
            self.thread.daemon = True
            self.thread.start()
            print '{} started on {}'.format(self.thread.name, host)
        except:
            self.log.error(traceback.format_exc())

        all_ports = [(self.port_struct.unpack(i.decode('hex'))[0],i)
                    for i in self.PORT_LIST]
        for i,port_name in all_ports :
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                self.tx_socket = sock
                sock.bind((host,i))
                sock.settimeout(0.1)
                thread = threading.Thread(target=self.monitor, args=(sock, port_name))
                thread.daemon = True
                thread.start()
                print '{} started, listening on {}'.format(thread.name,
                        sock.getsockname())
            except:
                self.log.error(traceback.format_exc())

    def monitor(self, sock, port_name):
        while not self.halting:
            try:
                (rf_data,address)=sock.recvfrom(1500)
                data = {'id':'rx', 'source_addr':address, 'rf_data':rf_data}
                self.process(data)
            except socket.timeout :
                pass

    def run(self):
        while not self.halting:
            data = self.service.getPacket()
            if data :
                self.process(data)

    def updateStatistics(self, bcnt):
        if self.first_cnt:
            self.first_cnt = False
            self.start_cnt = time.clock()
        else:
            self.arrv_cnt += 1
            self.arrv_bcnt += bcnt
            elapsed = time.clock() - self.start_cnt
            if (self.arrv_cnt % 100) == 0 :
                wx.PostEvent(self, RxStaEvent(txt=
                'C{:0>5d}/T{:<.2f} {:03.0f}Pps/{:05.0f}bps'.format(
                    self.arrv_cnt, elapsed, self.arrv_cnt / elapsed,
                    self.arrv_bcnt * 10 / elapsed)))

    def process(self, data) :
        if data['id'] == 'rx':
            try:
              addr = data['source_addr']
              data_group = data['rf_data']
              self.updateStatistics(len(data_group))
              '''
              self.log.info( 'rf_data:{}'.format(
                            ':'.join('{:02x}'.format(ord(c)) for c in data_group)))
              '''
              rf_data_group = pp.unpack(data_group)
              for rf_data in rf_data_group :
                if rf_data[0] == 'S':
                    if rf_data[1] == '\x01' :
                        T2 = int((time.clock() -
                            self.ntp_tick0)*1e6)
                        rslt = self.packNTP1.unpack(rf_data[2:])
                        T0 = rslt[0]
                        T1 = rslt[1]
                        T3 = int((time.clock() -
                            self.ntp_tick0)*1e6)
                        self.send(self.packNTP2.pack(ord('S'),2,T2,T3))
                        time.sleep(0.01)
                        delay = (T2-self.ntp_T0)-(T1-T0)
                        offset = ((T0-self.ntp_T0)+(T1-T2))/2
                        self.log.info(('Remote Time0={}us\n'
                            'Remote Time1={}us\n'
                            'Local Time2={}us\nLocal Time3={}us\n'
                            'Delay={}us, Offset={}us'
                            ).format(T0,T1,T2,T3,delay,offset))
                    elif rf_data[1] == '\x03' :
                        T6 = int((time.clock() -
                            self.ntp_tick0)*1e6)
                        rslt = self.packNTP3.unpack(rf_data[2:])
                        T4 = rslt[0]
                        self.log.info('Remote Time4={}us'.format(T4))
                        delay = rslt[1]
                        offset = rslt[2]
                        self.log.info('Delay={}us, Offset={}us'.format(delay,offset))
                        T5 = rslt[3]
                        self.log.info('Remote Time={}us, Local Time={}us'.format(T5,T6))
                    elif rf_data[1] == '\x13' :
                        rslt = self.packNTP13.unpack(rf_data[2:])
                        target = rslt[0]
                        delay = rslt[1]
                        offset = rslt[2]
                        self.log.info('{} Delay={}us, Offset={}us'.format(
                            {ord('A'):'ACM',ord('C'):'CMP'}[target], 
                            delay,offset))
                elif rf_data[0] == 'P':
                    deltaT = (time.clock() - self.ping_tick)*1000
                    if rf_data[1] == '\x01':
                        self.log.info('Ping back {} in {:.1f}ms, from GND {}'.format(
                            rf_data[2:], deltaT, addr))
                    elif rf_data[1] == '\x02':
                        self.log.info('Ping back {} in {:.1f}ms, from ACM {}'.format(
                            rf_data[2:], deltaT, addr))
                    elif rf_data[1] == '\x03':
                        self.log.info('Ping back {} in {:.1f}ms, from CMP {}'.format(
                            rf_data[2:], deltaT, addr))
                elif rf_data[0] == '\x22':
                    if self.fileALL:
                        self.fileALL.write(self.packHdr.pack(0x7e,
                            len(rf_data)))
                        self.fileALL.write(rf_data)
                    if self.OutputCnt > 0 or \
                        self.arrv_cnt_22 > 10 :
                        rslt = self.pack22.unpack(rf_data)
                        T = rslt[16]*1e-6
                        GX = Get14bit(rslt[10])*0.05
                        GY = Get14bit(rslt[11])*-0.05
                        GZ = Get14bit(rslt[12])*-0.05
                        AX = Get14bit(rslt[13])*-0.003333
                        AY = Get14bit(rslt[14])*0.003333
                        AZ = Get14bit(rslt[15])*0.003333
                    if self.OutputCnt > 0 :
                        self.OutputCnt -= 1
                        txt = '{:.2f},'.format(T)
                        if self.OutputSrv2Move & 1 :
                            txt += '{},{},'.format(rslt[1], rslt[17])
                        if self.OutputSrv2Move & 2 :
                            txt += '{},{},'.format(rslt[2], rslt[18])
                        if self.OutputSrv2Move & 4 :
                            txt += '{},{},'.format(rslt[3], rslt[19])
                        if self.OutputSrv2Move & 8 :
                            txt += '{},{},'.format(rslt[4], rslt[20])
                        if self.OutputSrv2Move & 16 :
                            txt += '{},{},'.format(rslt[5], rslt[21])
                        if self.OutputSrv2Move & 32 :
                            txt += '{},{},'.format(rslt[6], rslt[22])
                        self.log.info(txt)
                    if self.arrv_cnt_22 > 10 :
                        self.arrv_cnt_22 = 0
                        txt = ('T{0:08.3f} SenPack '
                            '1S{1:04d}/{16:+04d} 2S{2:04d}/{17:+04d} '
                            '3S{3:04d}/{18:+04d} 4S{4:04d}/{19:+04d} '
                            '5S{5:04d}/{20:+04d} 6S{6:04d}/{21:+04d}\n'
                            '1E{7:04d} 2E{8:04d} 3E{9:04d} '
                            'GX{10:6.1f} GY{11:6.1f} GZ{12:6.1f} '
                            'AX{13:6.2f} AY{14:6.2f} AZ{15:6.2f} ').format(T,
                                    rslt[1],rslt[2],rslt[3],
                                    rslt[4],rslt[5],rslt[6],
                                    rslt[7],rslt[8],rslt[9],
                                    GX,GY,GZ, AX,AY,AZ,
                                    rslt[17],rslt[18],rslt[19],rslt[20],
                                    rslt[21],rslt[22])
                        wx.PostEvent(self, RxEvent(txt=txt))
                        self.log.debug(txt)
                    else:
                        self.arrv_cnt_22 += 1
                elif rf_data[0] == '\x33':
                    if self.fileALL:
                        self.fileALL.write(self.packHdr.pack(0x7e,
                            len(rf_data)))
                        self.fileALL.write(rf_data)
                    if self.OutputCnt > 0 or \
                            self.arrv_cnt_33 > 10:
                        rslt = self.pack33.unpack(rf_data)
                        T = rslt[9]*1e-6
                    if self.OutputCnt > 0 :
                        self.OutputCnt -= 1
                        txt = '{:.2f},'.format(T)
                        if self.OutputSrv2Move & 1 :
                            txt += '{},{},'.format(rslt[1], rslt[10])
                        if self.OutputSrv2Move & 2 :
                            txt += '{},{},'.format(rslt[2], rslt[11])
                        if self.OutputSrv2Move & 4 :
                            txt += '{},{},'.format(rslt[3], rslt[12])
                        if self.OutputSrv2Move & 8 :
                            txt += '{},{},'.format(rslt[4], rslt[13])
                        self.log.info(txt)
                    if self.arrv_cnt_33 > 10:
                        self.arrv_cnt_33 = 0
                        self.last_arrv_cnt = self.arrv_cnt
                        txt = ('T{0:08.2f} SenPack '
                            '1S{1:04d}/{9:+04d} 2S{2:04d}/{10:+04d} '
                            '3S{3:04d}/{11:+04d} 4S{4:04d}/{12:+04d}\n'
                            '1E{5:04d} 2E{6:04d} 3E{7:04d} 4E{8:04d} '
                            ).format(T, rslt[1],rslt[2],rslt[3], rslt[4],
                                    rslt[5],rslt[6], rslt[7],rslt[8],
                                    rslt[10],rslt[11],rslt[12],rslt[13])
                        wx.PostEvent(self, RxCmpEvent(txt=txt))
                        self.log.debug(txt)
                    else:
                        self.arrv_cnt_33 += 1
                elif rf_data[0] == '\xA5':
                    #rslt = self.packAA.unpack(rf_data)
                    if self.fileALL:
                        self.fileALL.write(self.packHdr.pack(0x7e,
                            len(rf_data)))
                        self.fileALL.write(rf_data)
                    txt = ('A5 cmd')
                    self.txtGNDinfo.SetLabel(txt)
                elif rf_data[0] == '\xA6':
                    #rslt = self.packAA.unpack(rf_data)
                    if self.fileALL:
                        self.fileALL.write(self.packHdr.pack(0x7e,
                            len(rf_data)))
                        self.fileALL.write(rf_data)
                    txt = ('A6 cmd')
                    self.txtGNDinfo.SetLabel(txt)
                elif rf_data[0] == '\x77':
                    rslt = self.pack77.unpack(rf_data)
                    T = rslt[4]*1e-6
                    txt = ('T{0:08.3f} ACM CommStat senTask{2:d}us svoTask{3:d}us '
                           'msgTask{4:d}us').format(T,*rslt)
                    self.log.debug(txt)
                    wx.PostEvent(self, Rx2Event(txt=txt))
                elif rf_data[0] == '\x78' :
                    rslt = self.pack78.unpack(rf_data)
                    T = rslt[4]*1e-6
                    txt = ('T{0:08.3f} CMP CommStat senTask{2:d}us svoTask{3:d}us '
                           'msgTask{4:d}us').format(T,*rslt)
                    self.log.debug(txt)
                    wx.PostEvent(self, Rx2CmpEvent(txt=txt))
                elif rf_data[0] == '\xAA':
                    rslt = self.packAA.unpack(rf_data)
                    txt = ('msgTask{1:d}').format(*rslt)
                    self.txtGNDinfo.SetLabel(txt)
                elif rf_data[0] == '\x88':
                    rslt = self.pack88.unpack(rf_data)
                    B1 = rslt[1]*1.294e-2*1.515
                    B2 = rslt[2]*1.294e-2*3.0606
                    B3 = rslt[3]*1.294e-2*4.6363
                    B2 -= B1
                    if B2 < 0 :
                        B2 = 0
                    B2 =0 #TODO
                    B3 -= B1+B2
                    if B3 < 0 :
                        B3 = 0
                    txt = '{:.2f}V {:.2f}V {:.2f}V'.format(B1,B2,B3)
                    self.txtACMbat.SetLabel(txt)
                elif rf_data[0] == '\x99':
                    rslt = self.pack88.unpack(rf_data)
                    B1 = rslt[1]*1.294e-2*1.515
                    B2 = rslt[2]*1.294e-2*3.0606
                    B3 = rslt[3]*1.294e-2*4.6363
                    B2 -= B1
                    if B2 < 0 :
                        B2 = 0
                    B2 =0 #TODO
                    B3 -= B1+B2
                    if B3 < 0 :
                        B3 = 0
                    txt = '{:.2f}V {:.2f}V {:.2f}V'.format(B1,B2,B3)
                    self.txtCMPbat.SetLabel(txt)
                elif rf_data[0] == '\x44':
                    if self.fileALL:
                        self.fileALL.write(self.packHdr.pack(0x7e,
                            len(rf_data)))
                        self.fileALL.write(rf_data)
                    if self.arrv_cnt_44 > 10:
                        self.arrv_cnt_44 = 0
                        rslt = self.pack44.unpack(rf_data)
                        T = rslt[5]*1e-6
                        #ADC = [self.butt[i].update(rslt[1+i]) for i in range(4)]
                        ADC = [rslt[1+i] for i in range(4)]
                        txt = ('RIG {:.3f}s ADC:{:04.0f}|{:04.0f}'
                                '|{:04.0f}|{:04.0f}(unfilted)').format(T,*ADC)
                        wx.PostEvent(self, RxGndEvent(txt=txt))
                    else:
                        self.arrv_cnt_44 += 1
                elif rf_data[0] == 'T':
                    try:
                        txt = rf_data[5:-2]
                        items = txt.split(',')
                        vel = float(items[1])
                        dp = float(items[4])
                        T = self.packT.unpack(rf_data[1:5])[0]*1e-6
                        if self.fileALL:
                            data = self.packMeter.pack(ord('A'),vel,dp)
                            self.fileALL.write(self.packHdr.pack(0x7e,
                                len(data)))
                            self.fileALL.write(data)
                        txt = 'Get Vel{:.2f}m/s {:.1f}pa from {} at {:.3f}'.format(
                                vel, dp, addr, T)
                        wx.PostEvent(self, RxAirEvent(txt=txt))
                    except :
                        traceback.print_exc()
                else:
                    self.log.debug('Get {} from {}'.format(
                        rf_data.__repr__(), addr))
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        elif data['id'] == 'tx_status':
            try:
                if self.use_ZB :
                    del_sta = ord(data['deliver_status'])
                    dis_sta = ord(data['discover_status'])
                    retries = ord(data['retries'])
                    if self.frame_id != ord(data['frame_id']):
                        self.log.error("TXResponse frame_id mismatch"
                            "{}!={}".format(self.frame_id,
                                ord(data['frame_id'])))
                    addr = data['dest_addr']
                    del_sta = delivery_status[del_sta]
                    dis_sta = discovery_status[dis_sta]
                    self.log.info(
                        'TXResponse:{} to {} ({:d} retries,{})'.format(
                            del_sta, ':'.join('{:02x}'.format(ord(c))
                                              for c in addr), retries,
                            dis_sta))
                else :
                    tx_sta = ord(data['status'])
                    if self.frame_id != ord(data['frame_id']):
                        self.log.error("TXResponse frame_id mismatch"
                            "{}!={}".format(self.frame_id,
                                ord(data['frame_id'])))
                    tx_sta = tx_status[tx_sta]
                    self.log.info( 'TXResponse:{}'.format(tx_sta))
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        elif data['id'] == 'remote_at_response':
            try:
                s = data['status']
                addr = data['source_addr']
                parameter = data['parameter']
                if self.frame_id != data['frame_id']:
                    self.log.error("Remote ATResponse frame_id mismatch")
                self.log.info('ATResponse:{} {}={} from {}'.format(
                    at_status[s], data['command'],
                    ':'.join('{:02x}'.format(ord(c)) for c in parameter),
                     addr))
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        else:
            self.log.info(repr(data))

if __name__ == '__main__':
    app = wx.App(False)
    frame = MyFrame(None, wx.ID_ANY, 'Monitor Station', size=(650, 800))
    frame.Show(True)
    app.MainLoop()

