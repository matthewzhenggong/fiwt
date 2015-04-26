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

import time
import wx
import string
import logging
import serial
import xbee
import sys
import traceback
import threading
import struct

from wx.lib.newevent import NewEvent

RxEvent, EVT_RSLT1 = NewEvent()
Rx2Event, EVT_RSLT2 = NewEvent()
RxStaEvent, EVT_STAT = NewEvent()
LogEvent, EVT_LOG = NewEvent()

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
    0x00: 'Packet Acknowledged',
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

        panel = wx.Panel(self, -1)
        panel.SetDoubleBuffered(True)
        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.rb1 = wx.RadioButton(panel, wx.ID_ANY, "Port1:",
                                  style=wx.RB_GROUP)
        self.rb1.Enable(False)
        box.Add(self.rb1, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtPort = wx.TextCtrl(panel, -1, "COM3", size=(100, -1))
        box.Add(self.txtPort, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.rb2 = wx.RadioButton(panel, wx.ID_ANY, "Port2:")
        self.rb2.Enable(False)
        box.Add(self.rb2, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtPort2 = wx.TextCtrl(panel, -1, "COM6", size=(100, -1))
        box.Add(self.txtPort2, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Baudrate:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtBR = wx.TextCtrl(panel, -1, "1000000",
                                 size=(100, -1),
                                 validator=MyValidator(DIGIT_ONLY))
        box.Add(self.txtBR, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.cbZB = wx.CheckBox(panel, -1, "ZB")
        self.cbZB.SetValue(True)
        self.cbZB.SetToolTip(wx.ToolTip('Use Zigbee APIs'))
        box.Add(self.cbZB, 0, wx.ALIGN_CENTER, 5)
        self.cbEsc = wx.CheckBox(panel, -1, "Esc")
        self.cbEsc.SetValue(True)
        self.cbEsc.SetToolTip(wx.ToolTip('Use escape characters(ATAP=2)'))
        box.Add(self.cbEsc, 0, wx.ALIGN_CENTER, 5)
        self.btnStart = wx.Button(panel, -1, "Start", size=(100, -1))
        box.Add(self.btnStart, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Host:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtHost = wx.TextCtrl(panel, -1, "192.168.191.1", size=(100, -1))
        box.Add(self.txtHost, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.rb3 = wx.RadioButton(panel, wx.ID_ANY, "Remote:")
        box.Add(self.rb3, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtRemote = wx.TextCtrl(panel, -1, "192.168.191.2", size=(100, -1))
        box.Add(self.txtRemote, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        AT_CMD = ['MY', 'BD']
        ADDR64_LIST = ["0013a200408a72d2", "0013A200408A72AA",
                       "0000000000000000", "000000000000FFFF",
                       "0013a20040c1c44a", "0013a20040c15313",
                       "00000000C0A8BF02", "00000000C0A8BFFF",
                       "0013a200408A72A8", "0013A200408A72BC",
                       "0013A200408A72BF", "0013A200408A72DD"]
        ADDR16_LIST = ["0000", "FFFE"]

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnAT = wx.Button(panel, -1, "Send AT", size=(100, -1))
        self.btnAT.Enable(False)
        box.Add(self.btnAT, 0, wx.ALIGN_CENTER, 5)
        self.txtATcmd = wx.ComboBox(panel, -1, "MY",
                                    choices=AT_CMD,
                                    size=(50, -1))
        self.txtATcmd.SetToolTip(wx.ToolTip('AT Command in TWO characters'))
        box.Add(self.txtATcmd, 0, wx.ALIGN_CENTER, 5)
        self.txtATpar = wx.TextCtrl(panel, -1, "",
                                    size=(100, -1),
                                    validator=MyValidator(HEX_ONLY))
        self.txtATpar.SetToolTip(wx.ToolTip(
            'Hexadecimal Parameter for AT Command to set.\n'
            'If blanked, just get the parameter.'))
        box.Add(self.txtATpar, 0, wx.ALIGN_CENTER, 5)
        self.cbATqu = wx.CheckBox(panel, -1, "queued")
        box.Add(self.cbATqu, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Remote 64bit Addr:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtRmtATaddr64 = wx.ComboBox(panel, -1, "0013a200408a72d2",
                                          choices=ADDR64_LIST,
                                          validator=MyValidator(HEX_ONLY))
        self.txtRmtATaddr64.SetToolTip(wx.ToolTip(
'''Set to the 64-bit address of the destination device in hexadecimal.
The following addresses are also supported:
0x0000000000000000 - Reserved 64-bit address for the coordinator
0x000000000000FFFF - Broadcast address '''))
        box.Add(self.txtRmtATaddr64, 0, wx.ALIGN_CENTER, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "16bit Addr:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtRmtATaddr16 = wx.ComboBox(panel, -1, "FFFE",
                                          choices=ADDR16_LIST,
                                          validator=MyValidator(HEX_ONLY))
        self.txtRmtATaddr16.SetToolTip(wx.ToolTip(
'Set to the 16-bit address of the destination device in hexadecimal,'
'if known. Set to 0xFFFE if the address is unknown,'
'or if sending a broadcast.'))
        box.Add(self.txtRmtATaddr16, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnRmtAT = wx.Button(panel, -1, "Send RemoteAT", size=(100, -1))
        self.btnRmtAT.Enable(False)
        box.Add(self.btnRmtAT, 0, wx.ALIGN_CENTER, 5)
        self.txtRmtATcmd = wx.ComboBox(panel, -1, "MY",
                                       choices=AT_CMD,
                                       size=(50, -1))
        self.txtRmtATcmd.SetToolTip(wx.ToolTip('AT Command in TWO characters'))
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
Unused bits must be set to 0.  '''))
        box.Add(self.txtRmtATopt, 0, wx.ALIGN_CENTER, 5)
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

        self.btnTXc = wx.ToggleButton(panel, -1, "Send TX in", size=(100, -1))
        self.btnTXc.Enable(False)
        box.Add(self.btnTXc, 0, wx.ALIGN_CENTER, 5)
        self.txtTXfreq = wx.TextCtrl(panel, -1, "50",
                                     size=(50, -1),
                                     validator=MyValidator(DIGIT_ONLY))
        self.txtTXfreq.SetToolTip(
            wx.ToolTip('Frequency to send ping pack periodically.'))
        box.Add(self.txtTXfreq, 0, wx.ALIGN_CENTER, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Hz"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.txtTXBR = wx.StaticText(panel, wx.ID_ANY, "")
        box.Add(self.txtTXBR, 1, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRXSta = wx.StaticText(panel, wx.ID_ANY, "")
        box.Add(self.txtRXSta, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 32))
        self.txtRX.SetForegroundColour((0, 0, 255))
        box.Add(self.txtRX, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.txtRX2 = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 16))
        self.txtRX2.SetForegroundColour((255, 55, 0))
        box.Add(self.txtRX2, 1, wx.ALIGN_CENTER_VERTICAL | wx.EXPAND, 1)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

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
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        panel.SetSizer(sizer)
        sizer.Fit(panel)
        self.Bind(wx.EVT_BUTTON, self.OnStart, self.btnStart)
        self.Bind(wx.EVT_BUTTON, self.OnAT, self.btnAT)
        self.Bind(wx.EVT_BUTTON, self.OnRmtAT, self.btnRmtAT)
        self.Bind(wx.EVT_BUTTON, self.OnTX, self.btnTX)
        self.Bind(wx.EVT_BUTTON, self.OnTestMotor, self.btnTM)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.OnTXc, self.btnTXc)
        self.Bind(wx.EVT_BUTTON, self.OnClr, self.btnClr)
        self.Bind(wx.EVT_TEXT, self.OnCalBD, self.txtTX)
        self.Bind(wx.EVT_TEXT, self.OnCalBD, self.txtTXfreq)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChoosePort1, self.rb1)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChoosePort2, self.rb2)
        self.Bind(wx.EVT_TEXT_ENTER, self.OnTX, self.txtTX)
        self.Bind(wx.EVT_TEXT_ENTER, self.OnAT, self.txtATpar)
        self.Bind(wx.EVT_TEXT_ENTER, self.OnRmtAT, self.txtRmtATpar)
        self.Bind(EVT_RSLT1, self.OnRX)
        self.Bind(EVT_RSLT2, self.OnRX2)
        self.Bind(EVT_STAT, self.OnRXSta)
        self.Bind(EVT_LOG, self.OnLog)

        self.timer = None

    def OnLog(self, event) :
        self.log_txt.AppendText(event.log)

    def OnRX(self, event) :
        self.txtRX.SetLabel(event.txt)

    def OnRX2(self, event) :
        self.txtRX2.SetLabel(event.txt)

    def OnRXSta(self, event) :
        self.txtRXSta.SetLabel(event.txt)

    def OnChoosePort1(self, event):
        self.xbee = self.xbee1

    def OnChoosePort2(self, event):
        self.xbee = self.xbee2

    def OnCalBD(self, event):
        try:
            dt = 1 / (float(self.txtTXfreq.GetValue().encode()))
            if dt > 0:
                data_len = len(self.txtTX.GetValue().encode())
                br2 = int((6 + data_len) / dt + 1) * 10
                br1 = int((28 + 6 + data_len) / dt + 1) * 10
                self.txtTXBR.SetLabel('Pack len={}+{}={} BR={}/{}'.format(
                    28, 6 + data_len, 28 + 6 + data_len, br2, br1))
                br = int(self.txtBR.GetValue().encode())
                if br1 >= br:
                    self.txtTXBR.SetForegroundColour((255, 0, 0))
                else:
                    self.txtTXBR.SetForegroundColour((0, 0, 255))
        except:
            #traceback.print_exc()
            pass

    def OnClr(self, event):
        self.log_txt.Clear()
        self.txtRXSta.SetLabel('')
        self.txtRX.SetLabel('')
        self.txtRX2.SetLabel('')
        self.first_cnt = True
        self.arrv_cnt = 0
        self.last_arrv_cnt = 0
        self.arrv_bcnt = 0
        self.periodic_count = 0

    def OnClose(self, event):
        try:
            if self.timer:
                self.timer.cancel()
            self.halting = True
            time.sleep(1)
            if self.xbee1:
                self.xbee1.halt()
            if self.xbee2:
                self.xbee2.halt()
            if self.serial_port:
                self.serial_port.close()
            if self.serial_port2:
                self.serial_port2.close()
        except:
            pass
        self.log.removeHandler(self.log_handle)
        event.Skip()

    def OnAT(self, event):
        command = self.txtATcmd.GetValue().encode()[:2]
        parameter = self.txtATpar.GetValue().encode()
        if len(parameter) == 0:
            parameter = None
            self.log.info('get AT ' + command)
        else:
            if len(parameter) % 2 == 1:
                parameter = '0' + parameter
            try:
                parameter = parameter.decode('hex')
            except:
                traceback.print_exc()
                return
            self.log.debug(
                'set AT ' + command + '=' + ':'.join('{:02x}'.format(ord(c))
                                                     for c in parameter))
        self.frame_id = self.getFrameId()
        if self.cbATqu.GetValue():
            self.xbee.queued_at(command=command,
                                parameter=parameter,
                                frame_id=chr(self.frame_id))
        else:
            self.xbee.at(command=command,
                         parameter=parameter,
                         frame_id=chr(self.frame_id))

    def OnRmtAT(self, event):
        try:
            addr64 = self.txtRmtATaddr64.GetValue().encode()[:16].decode('hex')
            addr16 = self.txtRmtATaddr16.GetValue().encode()[:4].decode('hex')
            options = self.txtRmtATopt.GetValue().encode()[:2].decode('hex')
            command = self.txtRmtATcmd.GetValue().encode()[:2]
            parameter = self.txtRmtATpar.GetValue().encode()
            if len(parameter) == 0:
                parameter = None
                self.log.info('get AT ' + command + ' from ' + ':'.join(
                    '{:02x}'.format(ord(c))
                    for c in addr16) + '(' + ':'.join('{:02x}'.format(ord(c))
                                                      for c in addr64) +
                              ') with option {:02x}'.format(ord(options)))
            else:
                if len(parameter) % 2 == 1:
                    parameter = '0' + parameter
                parameter = parameter.decode('hex')
                self.log.debug('send AT ' + command + '=' + ':'.join(
                    '{:02x}'.format(ord(c))
                    for c in parameter) + ' to ' + ':'.join(
                        '{:02x}'.format(ord(c))
                        for c in addr16) + '(' + ':'.join('{:02x}'.format(ord(c))
                                                          for c in addr64) +
                               ') with option {:02x}'.format(ord(options)))
            self.frame_id = self.getFrameId()
            self.xbee.remote_at(dest_addr_long=addr64,
                                dest_addr=addr16,
                                options=options,
                                command=command,
                                parameter=parameter,
                                frame_id=chr(self.frame_id))
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
        self.send(data, no_response=True)

    def OnTX(self, event):
        data = self.txtTX.GetValue().encode()
        self.send('P'+data, no_response=True)
        self.ping_tick = time.clock()

    def OnTXc(self, event):
        if event.IsChecked():
            self.periodic_sending = 1
            self.periodic_sending_time_all = 0.0
            self.periodic_sending_time_max = 0.0
            self.periodic_sending_time_min = 99e99
            self.periodic_sending_cnt = 0.0
            self.periodic_dt = 1.0 / float(
                self.txtTXfreq.GetValue().encode())
            self.periodic_data = self.txtTX.GetValue().encode()
            self.txtTXfreq.Disable()
            self.txtTX.Disable()
            self.periodic_send()
        else:
            self.periodic_sending = 0
            self.txtTXfreq.Enable()
            self.txtTX.Enable()

    def periodic_send(self):
        tick = time.clock()
        self.periodic_count += 1
        self.send('P{:05d}'.format(self.periodic_count) + self.periodic_data,
                  True)
        self.ping_tick = time.clock()
        if self.periodic_sending:
            threading.Timer(self.periodic_dt -
                            (time.clock() - tick), self.periodic_send).start()

    def send(self, data, no_response=False):
        try:
            if data:
                if no_response:
                    self.frame_id = 0
                elif self.sending:
                    return False
                else:
                    self.frame_id = self.getFrameId()
                    self.sending = True

                addr64 = self.txtRmtATaddr64.GetValue().encode()[:16].decode(
                    'hex')
                addr16 = self.txtRmtATaddr16.GetValue().encode()[:4].decode(
                    'hex')
                broadcast_radius = self.txtTXrad.GetValue().encode()[:2].decode(
                    'hex')
                options = self.txtTXopt.GetValue().encode()[:2].decode('hex')
                if self.use_ZB:
                    if not self.periodic_sending and not no_response:
                        self.log.debug(
                            'send TX ' + data.__repr__() + ' to ' + ':'.join(
                                '{:02x}'.format(ord(c)) for c in addr16) + '(' +
                            ':'.join('{:02x}'.format(ord(c)) for c in addr64) +
                            ') with option {:02x}'.format(ord(options)) +
                            ' & radius {:02x}'.format(ord(broadcast_radius)))
                    self.xbee.tx(dest_addr_long=addr64,
                                 dest_addr=addr16,
                                 broadcast_radius=broadcast_radius,
                                 options=options,
                                 data=data,
                                 frame_id=chr(self.frame_id))
                elif addr16 == '\xff\xfe':
                    if not self.periodic_sending and not no_response:
                        self.log.debug(
                            'send TX ' + data.__repr__() + ' to ' +
                            ':'.join('{:02x}'.format(ord(c)) for c in addr64) +
                            ' with option {:02x}'.format(ord(options)))
                    self.xbee.tx_long_addr(dest_addr=addr64,
                                 options=options,
                                 data=data,
                                 frame_id=chr(self.frame_id))
                else:
                    if not self.periodic_sending and not no_response:
                        self.log.debug(
                            'send TX ' + data.__repr__() + ' to ' +
                            ':'.join('{:02x}'.format(ord(c)) for c in addr16) +
                            ' with option {:02x}'.format(ord(options)))
                    self.xbee.tx(dest_addr=addr16,
                                 options=options,
                                 data=data,
                                 frame_id=chr(self.frame_id))
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
        self.txtPort.Enable(False)
        self.txtPort2.Enable(False)
        self.txtHost.Enable(False)
        self.txtRemote.Enable(False)
        self.txtBR.Enable(False)
        self.cbEsc.Enable(False)
        self.cbZB.Enable(False)
        self.use_ZB = self.cbZB.GetValue()

        self.rec = open(time.strftime('rec%Y%m%d%H%M%S.dat'), 'wb')

        self.starting = True
        try:
            self.serial_port = serial.Serial(self.txtPort.GetValue().encode(),
                                             int(self.txtBR.GetValue()))
        except:
            port1flag = False
            self.serial_port1 = None
        else:
            port1flag = True

        try:
            self.serial_port2 = serial.Serial(
                self.txtPort2.GetValue().encode(), int(self.txtBR.GetValue()))
        except:
            self.serial_port2 = None
            port2flag = False
        else:
            port2flag = True

        self.halting = False
        if port2flag:
            if self.cbZB.GetValue() :
                self.xbee2 = xbee.zigbee.ZigBee(self.serial_port2,
                                         callback=self.get_frame_data,
                                         escaped=self.cbEsc.GetValue())
            else :
                self.xbee2 = xbee.XBee(self.serial_port2,
                                         callback=self.get_frame_data,
                                         escaped=self.cbEsc.GetValue())
            self.xbee = self.xbee2
            self.rb2.Enable(True)
            self.rb2.SetValue(True)
        else :
            self.xbee2 = None
        if port1flag:
            if self.cbZB.GetValue() :
                self.xbee1 = xbee.zigbee.ZigBee(self.serial_port,
                                         callback=self.get_frame_data,
                                         escaped=self.cbEsc.GetValue())
            else :
                self.xbee1 = xbee.XBee(self.serial_port,
                                         callback=self.get_frame_data,
                                         escaped=self.cbEsc.GetValue())
            self.xbee = self.xbee1
            self.rb1.Enable(True)
            self.rb1.SetValue(True)
        else :
            self.xbee1 = None

        self.frame_id = 0
        self.sending = False
        self.first_cnt = True
        self.arrv_cnt = 0
        self.last_arrv_cnt = 0
        self.arrv_bcnt = 0
        self.periodic_count = 0
        self.periodic_sending = 0

        if port2flag or port1flag:
            self.btnAT.Enable(True)
            self.btnRmtAT.Enable(True)
            self.btnTX.Enable(True)
            self.btnTM.Enable(True)
            self.btnTXc.Enable(True)

        self.frame_id = 1

        self.pack06 = struct.Struct("<H2H6H3H6h4H6H4H")
        self.pack22 = struct.Struct(">B6H3H6HBH6h2f")
        self.pack77 = struct.Struct(">B5HBH")
        self.pack78 = struct.Struct(">B4HBH")
        self.pack88 = struct.Struct(">B3BBH")
        self.pack33 = struct.Struct(">B4H4HBH4h")
        self.ch = 0
        self.test_motor_ticks = 0
        self.starting = False

        self.OutputSrv2Move = 0
        self.OutputCnt = 0

        print 'start'

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

    def get_frame_data(self, data):
        if self.starting:
            return
        if self.halting:
            return
        if data['id'] == 'rx':
            try:
                if self.use_ZB :
                    addr64 = data['source_addr_long']
                else :
                    rssi = data['rssi']
                addr16 = data['source_addr']
                options = ord(data['options'])
                rf_data = data['rf_data']
                self.updateStatistics(len(rf_data))
                if rf_data[0] == 'P':
                    deltaT = (time.clock() - self.ping_tick)*1000
                    if self.periodic_sending == 0:
                        self.log.info('Ping back in {:.1f}ms'.format(deltaT))
                    else :
                        self.periodic_sending_time_all += deltaT
                        self.periodic_sending_cnt += 1.0
                        if deltaT > self.periodic_sending_time_max:
                            self.periodic_sending_time_max = deltaT
                        if deltaT < self.periodic_sending_time_min:
                            self.periodic_sending_time_min = deltaT
                        txt = 'Ping back in {:.1f}/{:.1f}/{:.1f}ms'.format(
                                self.periodic_sending_time_all/self.periodic_sending_cnt,
                                self.periodic_sending_time_max,
                                self.periodic_sending_time_min)
                        wx.PostEvent(self, RxEvent(txt=txt))
                elif rf_data[0] == '\x22':
                    rslt = self.pack22.unpack(rf_data)
                    T = (rslt[16]*0x10000+rslt[17])*0.001
                    GX = Get14bit(rslt[10])*0.05
                    GY = Get14bit(rslt[11])*-0.05
                    GZ = Get14bit(rslt[12])*-0.05
                    AX = Get14bit(rslt[13])*-0.003333
                    AY = Get14bit(rslt[14])*0.003333
                    AZ = Get14bit(rslt[15])*0.003333
                    phi = rslt[24]*57.3
                    tht = rslt[25]*57.3
                    if self.OutputCnt > 0 :
                        self.OutputCnt -= 1
                        txt = '{:.2f},'.format(T)
                        if self.OutputSrv2Move & 1 :
                            txt += '{},{},'.format(rslt[1], rslt[18])
                        if self.OutputSrv2Move & 2 :
                            txt += '{},{},'.format(rslt[2], rslt[19])
                        if self.OutputSrv2Move & 4 :
                            txt += '{},{},'.format(rslt[3], rslt[20])
                        if self.OutputSrv2Move & 8 :
                            txt += '{},{},'.format(rslt[4], rslt[21])
                        if self.OutputSrv2Move & 16 :
                            txt += '{},{},'.format(rslt[5], rslt[22])
                        if self.OutputSrv2Move & 32 :
                            txt += '{},{},'.format(rslt[6], rslt[23])
                        self.log.info(txt)
                    if self.arrv_cnt > self.last_arrv_cnt+4 :
                        self.last_arrv_cnt = self.arrv_cnt
                        txt = ('T{0:08.2f} SenPack '
                            '1S{1:04d}/{16:+04d} 2S{2:04d}/{17:+04d} '
                            '3S{3:04d}/{18:+04d} 4S{4:04d}/{19:+04d} '
                            '5S{5:04d}/{20:+04d} 6S{6:04d}/{21:+04d}\n'
                            '1E{7:04d} 2E{8:04d} 3E{9:04d} '
                            'GX{10:6.1f} GY{11:6.1f} GZ{12:6.1f} '
                            'AX{13:6.2f} AY{14:6.2f} AZ{15:6.2f} '
                            'phi{22:6.2f} tht{23:6.2f}').format(T,
                                    rslt[1],rslt[2],rslt[3],
                                    rslt[4],rslt[5],rslt[6],
                                    rslt[7],rslt[8],rslt[9],
                                    GX,GY,GZ, AX,AY,AZ,
                                    rslt[18],rslt[19],rslt[20],rslt[21],
                                    rslt[22],rslt[23], phi,tht )
                        wx.PostEvent(self, RxEvent(txt=txt))
                        self.log.debug(txt)
                elif rf_data[0] == '\x33':
                    rslt = self.pack33.unpack(rf_data)
                    T = (rslt[9]*0x10000+rslt[10])*0.001
                    if self.OutputCnt > 0 :
                        self.OutputCnt -= 1
                        txt = '{:.2f},'.format(T)
                        if self.OutputSrv2Move & 1 :
                            txt += '{},{},'.format(rslt[1], rslt[11])
                        if self.OutputSrv2Move & 2 :
                            txt += '{},{},'.format(rslt[2], rslt[12])
                        if self.OutputSrv2Move & 4 :
                            txt += '{},{},'.format(rslt[3], rslt[13])
                        if self.OutputSrv2Move & 8 :
                            txt += '{},{},'.format(rslt[4], rslt[14])
                        self.log.info(txt)
                    if self.arrv_cnt > self.last_arrv_cnt+4 :
                        self.last_arrv_cnt = self.arrv_cnt
                        txt = ('T{0:08.2f} SenPack '
                            '1S{1:04d}/{9:+04d} 2S{2:04d}/{10:+04d} '
                            '3S{3:04d}/{11:+04d} 4S{4:04d}/{12:+04d}\n'
                            '1E{5:04d} 2E{6:04d} 3E{7:04d} 4E{8:04d} '
                            ).format(T, rslt[1],rslt[2],rslt[3], rslt[4],
                                    rslt[5],rslt[6], rslt[7],rslt[8],
                                    rslt[11],rslt[12],rslt[13],rslt[14])
                        wx.PostEvent(self, RxEvent(txt=txt))
                        self.log.debug(txt)
                elif rf_data[0] == '\x77':
                    rslt = self.pack77.unpack(rf_data)
                    T = rslt[4]*0x10000+rslt[5]
                    T = (rslt[4]*0x10000+rslt[5])*0.001
                    txt = ('T{0:08.2f} CommPack revTask{2:d}us '
                           'senTask{3:d}us svoTask{4:d}us '
                           'ekfTask{5:d}us sndTask{6:d}us').format(T,*rslt)
                    self.log.debug(txt)
                    wx.PostEvent(self, Rx2Event(txt=txt))
                elif rf_data[0] == '\x78' :
                    rslt = self.pack78.unpack(rf_data)
                    T = rslt[4]*0x10000+rslt[5]
                    T = (rslt[4]*0x10000+rslt[5])*0.001
                    txt = ('T{0:08.2f} CommPack revTask{2:d}us '
                           'senTask{3:d}us svoTask{4:d}us '
                           'sndTask{5:d}us').format(T,*rslt)
                    self.log.debug(txt)
                    wx.PostEvent(self, Rx2Event(txt=txt))
                elif rf_data[0] == '\x88' or rf_data[0] == '\x99':
                    rslt = self.pack88.unpack(rf_data)
                    B1 = rslt[1]*1.294e-2*1.515
                    B2 = rslt[2]*1.294e-2*3.0606
                    B3 = rslt[3]*1.294e-2*4.6363
                    B2 -= B1
                    if B2 < 0 :
                        B2 = 0
                    B3 -= B1+B2
                    if B3 < 0 :
                        B3 = 0
                    T = (rslt[4]*0x10000+rslt[5])*0.001
                    txt = ('T{:08.2f} BattPack '
                        'B{:.2f} B{:.2f} B{:.2f} ').format(T,B1,B2,B3)
                    self.log.debug(txt)
                    wx.PostEvent(self, Rx2Event(txt=txt))
                elif rf_data[0] == '\x06':
                    rslt = self.pack06.unpack(rf_data)
                    wx.PostEvent(self, RxEvent(txt=
                        'Sensor {1}.{2:03d} B{9}B{10}B{11} 1S{3:04d} 2S{4:04d} 3S{5:04d} 4S{6:04d} 5S{7:04d} 6S{8:04d}\n1E{18:04d} 2E{19:04d} 3E{20:04d} 4E{21:04d} 1L{28:03d} 2L{29:03d} 3L{30:03d} 4L{31:03d}'.format(
                            *rslt)))
                    if self.test_motor_ticks > 0:
                        sec = rslt[1]
                        msec = rslt[2]
                        pos = rslt[3 + self.ch]
                        ctrl = rslt[12 + self.ch]
                        self.log.info('Sensor\t{0}.{1:03d}\t{2}\t{3}'.format(
                            sec, msec, pos, ctrl))
                        self.test_motor_ticks -= 1
                else:
                    if self.use_ZB :
                        self.log.info('RX:{}. Get {} from {}({})'.format(
                            recv_opts[options], rf_data.__repr__(),
                            ':'.join('{:02x}'.format(ord(c)) for c in addr16),
                            ':'.join('{:02x}'.format(ord(c)) for c in addr64)))
                    else :
                        self.log.info('RX:{}. Get {} from {}({})'.format(
                            recv_opts[options], rf_data.__repr__(),
                            ':'.join('{:02x}'.format(ord(c)) for c in addr16),
                            rssi))
                self.rec.write(rf_data)
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        elif data['id'] == 'tx_status':
            self.sending = False
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
        elif data['id'] == 'at_response':
            try:
                s = ord(data['status'])
                try:
                    parameter = data['parameter']
                    if not parameter:
                        parameter = ''
                except KeyError:
                    parameter = ''
                if self.frame_id != ord(data['frame_id']):
                    self.log.error("ATResponse frame_id mismatch")
                self.log.info('ATResponse:{} {}={}'.format(
                    at_status[s], data['command'],
                    ':'.join('{:02x}'.format(ord(c)) for c in parameter)))
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        elif data['id'] == 'remote_at_response':
            try:
                s = ord(data['status'])
                addr16 = data['source_addr']
                addr64 = data['source_addr_long']
                try:
                    parameter = data['parameter']
                    if not parameter:
                        parameter = ''
                except KeyError:
                    parameter = ''
                if self.frame_id != ord(data['frame_id']):
                    self.log.error("Remote ATResponse frame_id mismatch")
                self.log.info('ATResponse:{} {}={} from {}({})'.format(
                    at_status[s], data['command'],
                    ':'.join('{:02x}'.format(ord(c)) for c in parameter),
                    ':'.join('{:02x}'.format(ord(c))
                             for c in addr16), ':'.join('{:02x}'.format(ord(c))
                                                        for c in addr64)))
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        elif data['id'] == 'status':
            try:
                s = ord(data['status'])
                if s > 0x80:
                    s = 0x80
                s = moderm_status[s]
                self.log.info('ModemStatus:' + s)
            except:
                traceback.print_exc()
                self.log.error(repr(data))
        else:
            self.log.info(repr(data))



if __name__ == '__main__':
    app = wx.App(False)
    frame = MyFrame(None, wx.ID_ANY, 'XBee ZigBee Station', size=(650, 800))
    frame.Show(True)
    app.MainLoop()
