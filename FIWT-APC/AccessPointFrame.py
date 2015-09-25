#!/bin/env python
# -*- coding: utf-8 -*-
"""
AccessPoint(AP) GUI frame in wxPython
----------------------------------------

Author: Zheng GONG(matthewzhenggong@gmail.com)

This file is part of FIWT.

FIWT is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
"""

import math, os, sys, time, types, string, wx
import threading, logging, struct
import Queue
from ConfigParser import SafeConfigParser
from math import atan2,sqrt,sin,cos

import socket
import json

from wx.lib.newevent import NewEvent
import wx.lib.agw.pygauge as PG

from embed_images import ES

# New Event Declarations
LogEvent, EVT_LOG = NewEvent()
RxStaEvent, EVT_STAT = NewEvent()
ACM_StaEvent, EVT_ACM_STAT = NewEvent()
CMP_StaEvent, EVT_CMP_STAT = NewEvent()
GND_StaEvent, EVT_GND_STAT = NewEvent()
ACM_DatEvent, EVT_ACM_DAT = NewEvent()
CMP_DatEvent, EVT_CMP_DAT = NewEvent()
GND_DatEvent, EVT_GND_DAT = NewEvent()
EXP_DatEvent, EVT_EXP_DAT = NewEvent()

ALPHA_ONLY = 1
DIGIT_ONLY = 2
HEX_ONLY = 3

def _(ori_string):
    return ori_string

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


class RedirectText(object):
    def __init__(self, parent):
        self.parent = parent

    def write(self, string):
        wx.PostEvent(self.parent, LogEvent(log=string))

class MyFrame(wx.Frame):
    """
    Main Frame class.
    """

    def __init__(self, parent, id, title, process, gui2msgcQueue,
            msgc2guiQueue, gui2drawerQueue):
        """
        Initialise the Frame.
        """
        self.msg_process = process[0]
        self.graph_process = process[1]
        self.gui2msgcQueue = gui2msgcQueue
        self.msgc2guiQueue = msgc2guiQueue
        self.gui2drawerQueue = gui2drawerQueue

        parser = SafeConfigParser()
        parser.read('config.ini')
        self.parser = parser

        self.aclink = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.aclink.settimeout(0.01)
        self.aclink.bind(('127.0.0.1', 7131))
        self.aclink_addr = ('127.0.0.1', 7132)

        wx.Frame.__init__(self, parent, id, title, wx.Point(0, 0),
                          wx.Size(720, 800))

        # Prepare the menu bar
        menuBar = wx.MenuBar()

        # 1st menu from left
        menu1 = wx.Menu()
        menu_ES = menu1.Append(wx.ID_ANY, "&Emergency Stop\tCTRL+E", "Emergency Stop")
        menu_ER = menu1.Append(wx.ID_ANY, "Emergency &Cancel", "Emergency Canceled")
        menu1.AppendSeparator()
        menu_close = menu1.Append(wx.ID_ANY, "&Close\tCTRL+Q", "Close this frame")
        # Add menu to the menu bar
        menuBar.Append(menu1, "&File")

        self.SetMenuBar(menuBar)

        panel = wx.Panel(self, -1)
        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnStart = wx.Button(panel, -1, "Start", size=(100, -1))
        box.Add(self.btnStart, 0, wx.ALIGN_CENTER, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "Host:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtHost = wx.TextCtrl(panel, -1, parser.get('host','AP'), size=(100, -1))
        box.Add(self.txtHost, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        box.Add(wx.StaticText(panel, wx.ID_ANY, "ManoSer:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtCOM = wx.TextCtrl(panel, -1, parser.get('host','COM'), size=(100, -1))
        box.Add(self.txtCOM, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 5)
        self.btnBaseTime = wx.Button(panel, -1, "Set Base Time", size=(100, -1))
        self.btnBaseTime.Enable(False)
        box.Add(self.btnBaseTime, 0, wx.ALIGN_CENTER, 5)
        self.btnGNDsynct = wx.ToggleButton(panel, -1, "Sync Time")
        self.btnGNDsynct.SetValue(True)
        self.btnGNDsynct.Enable(False)
        box.Add(self.btnGNDsynct, 0, wx.ALIGN_CENTER, 5)
        self.txtRecName = wx.TextCtrl(panel, -1, parser.get('rec','prefix'),
                validator=MyValidator(DIGIT_ONLY))
        box.Add(self.txtRecName, 1, wx.ALIGN_CENTER|wx.LEFT, 5)
        self.btnALLrec = wx.ToggleButton(panel, -1, "REC")
        self.btnALLrec.Enable(False)
        box.Add(self.btnALLrec, 0, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        AT_CMD = ['MY', 'MK', 'GW', 'SH', 'SL', 'DL', 'C0', 'ID', 'AH', 'MA',
                'PL', 'BD', 'AI', 'WR', 'FR',]
        HOST_LIST = ["192.168.191.2", "192.168.191.3", "192.168.191.4"]

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.target = 'GND'
        self.rbGND = wx.RadioButton(panel, wx.ID_ANY, "GND:",
                                  style=wx.RB_GROUP)
        box.Add(self.rbGND, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtGNDhost = wx.ComboBox(panel, -1, parser.get('host','GND'),
                                          choices=HOST_LIST)
        box.Add(self.txtGNDhost, 0, wx.ALIGN_CENTER, 5)
        self.txtGNDinfo = wx.StaticText(panel, wx.ID_ANY, "", size=(32, 16))
        self.txtGNDinfo.SetForegroundColour((255, 55, 0))
        box.Add(self.txtGNDinfo, 1, wx.ALIGN_CENTER|wx.LEFT, 5)

        box.Add(wx.StaticText(panel, wx.ID_ANY, "Simulink Tx port:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtMatlabRx = wx.TextCtrl(panel, -1,
                parser.get('simulink','tx'), size=(100,-1),
                validator=MyValidator(DIGIT_ONLY))
        box.Add(self.txtMatlabRx, 0, wx.ALIGN_CENTER, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.rbACM = wx.RadioButton(panel, wx.ID_ANY, "ACM:")
        box.Add(self.rbACM, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtACMhost = wx.ComboBox(panel, -1, parser.get('host','ACM'),
                                          choices=HOST_LIST)
        box.Add(self.txtACMhost, 0, wx.ALIGN_CENTER, 5)
        self.ggACMbat = PG.PyGauge(panel, wx.ID_ANY, size=(50, 16))
        self.ggACMbat.SetValue(0)
        self.ggACMbat.SetBorderColor(wx.BLACK)
        self.ggACMbat.SetBorderPadding(2)
        self.ggACMbat.SetDrawValue(draw=True, drawPercent=True, font=wx.SMALL_FONT, colour=wx.BLUE)
        box.Add(self.ggACMbat, 0, wx.ALIGN_CENTER|wx.LEFT, 5)
        box.AddStretchSpacer()

        box.Add(wx.StaticText(panel, wx.ID_ANY, "Simulink Rx port:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtMatlabTx = wx.TextCtrl(panel, -1,
                parser.get('simulink','rx'), size=(100,-1),
                validator=MyValidator(DIGIT_ONLY))
        box.Add(self.txtMatlabTx, 0, wx.ALIGN_CENTER, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.rbCMP = wx.RadioButton(panel, wx.ID_ANY, "CMP:")
        box.Add(self.rbCMP, 0, wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtCMPhost = wx.ComboBox(panel, -1, parser.get('host','CMP'),
                                          choices=HOST_LIST)
        box.Add(self.txtCMPhost, 0, wx.ALIGN_CENTER, 5)
        self.ggCMPbat = PG.PyGauge(panel, wx.ID_ANY, size=(50, 16))
        self.ggCMPbat.SetValue(0)
        self.ggCMPbat.SetBorderColor(wx.BLACK)
        self.ggCMPbat.SetBorderPadding(2)
        self.ggCMPbat.SetDrawValue(draw=True, drawPercent=True, font=wx.SMALL_FONT, colour=wx.BLUE)
        box.Add(self.ggCMPbat, 0, wx.ALIGN_CENTER|wx.LEFT, 5)
        box.AddStretchSpacer()

        box.Add(wx.StaticText(panel, wx.ID_ANY, "Simulink ExtraInputs:"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, 1)
        self.txtMatlabExtra = wx.TextCtrl(panel, -1,
                parser.get('simulink','extra'), size=(100,-1))
        box.Add(self.txtMatlabExtra, 0, wx.ALIGN_CENTER, 5)

        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnRmtAT = wx.Button(panel, -1, "Send RemoteAT", size=(100, -1))
        self.btnRmtAT.Enable(False)
        box.Add(self.btnRmtAT, 0, wx.ALIGN_CENTER, 5)
        self.txtRmtATcmd = wx.ComboBox(panel, -1, "MY", choices=AT_CMD)
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

        self.btnTX = wx.Button(panel, -1, "Send Command", size=(100, -1))
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

        boxV = wx.BoxSizer(wx.VERTICAL)
        self.btnTM = wx.Button(panel, -1, "Servo Command", size=(100, -1))
        self.btnTM.Enable(False)
        boxV.Add(self.btnTM, 0, wx.ALIGN_CENTER, 5)
        box.Add(boxV, 0, wx.ALIGN_CENTER, 5)

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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Da="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef1 = wx.TextCtrl(panel, -1, "0.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef1, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move1 = wx.CheckBox(panel, -1, "CH1")
        boxH.Add(self.Srv2Move1, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue1 = wx.TextCtrl(panel, -1, "5.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue1, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MinFreq"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue1 = wx.TextCtrl(panel, -1, "1.0",
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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "De="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef2 = wx.TextCtrl(panel, -1, "4.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef2, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move2 = wx.CheckBox(panel, -1, "CH2")
        boxH.Add(self.Srv2Move2, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue2 = wx.TextCtrl(panel, -1, "5.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.MaxValue2, 0, wx.ALIGN_CENTER, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxFreq"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MinValue2 = wx.TextCtrl(panel, -1, "1.0",
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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Dr="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef3 = wx.TextCtrl(panel, -1, "0.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef3, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move3 = wx.CheckBox(panel, -1, "CH3")
        boxH.Add(self.Srv2Move3, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue3 = wx.TextCtrl(panel, -1, "5.0",
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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Ca="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef4 = wx.TextCtrl(panel, -1, "0.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef4, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move4 = wx.CheckBox(panel, -1, "CH4")
        boxH.Add(self.Srv2Move4, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue4 = wx.TextCtrl(panel, -1, "5.0",
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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Ce="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef5 = wx.TextCtrl(panel, -1, "0.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef5, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move5 = wx.CheckBox(panel, -1, "CH5")
        boxH.Add(self.Srv2Move5, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue5 = wx.TextCtrl(panel, -1, "5.0",
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
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "Cr="), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.ServoRef6 = wx.TextCtrl(panel, -1, "0.0",
                                   size=(50, -1),
                                   validator=MyValidator(DIGIT_ONLY))
        boxH.Add(self.ServoRef6, 0, wx.ALIGN_CENTER, 5)
        self.Srv2Move6 = wx.CheckBox(panel, -1, "CH6")
        boxH.Add(self.Srv2Move6, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        boxH.Add(wx.StaticText(panel, wx.ID_ANY, "MaxValue"), 0,
                wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 5)
        self.MaxValue6 = wx.TextCtrl(panel, -1, "5.0",
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

        boxV = wx.BoxSizer(wx.VERTICAL)
        self.btnES = wx.BitmapButton(panel, -1, bitmap=ES.getBitmap(), size=(100, -1))
        self.btnES.Enable(False)
        self.btnES.SetBackgroundColour(wx.RED)
        boxV.Add(self.btnES, 1, wx.ALIGN_CENTER|wx.EXPAND, 5)
        box.Add(boxV, 1, wx.ALIGN_CENTER|wx.EXPAND, 5)


        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        self.btnResetRig = wx.Button(panel, -1, "Reset Rig Position")
        self.btnResetRig.Enable(False)
        sizer.Add(self.btnResetRig, 0, wx.ALIGN_LEFT, 5)

        sub_panel = wx.Panel(panel, -1)
        sub_panel.SetDoubleBuffered(True)
        sub_sizer = wx.BoxSizer(wx.VERTICAL)

        self.txtRXSta = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtRXSta, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        self.txtGNDSta = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtGNDSta, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)
        self.txtGNDDat = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtGNDDat, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        self.txtACMSta = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtACMSta, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)
        self.txtACMDat = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtACMDat, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        self.txtCMPSta = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtCMPSta, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)
        self.txtCMPDat = wx.StaticText(sub_panel, wx.ID_ANY, "")
        sub_sizer.Add(self.txtCMPDat, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        self.txtExpDat = wx.StaticText(sub_panel, wx.ID_ANY, "", size=(100,32))
        sub_sizer.Add(self.txtExpDat, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

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
        sizer.Add(self.log_txt, 1, wx.ALL | wx.EXPAND, 1)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.btnClr = wx.Button(panel, -1, "Clear")
        box.Add(self.btnClr, 1, wx.ALIGN_CENTER, 5)
        self.btnSaveLog = wx.Button(panel, -1, "Save Log")
        box.Add(self.btnSaveLog, 1, wx.ALIGN_CENTER, 5)
        sizer.Add(box, 0, wx.ALIGN_CENTRE | wx.ALL | wx.EXPAND, 1)

        panel.SetSizer(sizer)
        sizer.Fit(panel)

        # bind events to functions
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_MENU, self.OnClose, menu_close)
        self.Bind(EVT_LOG, self.OnLog)
        self.Bind(wx.EVT_BUTTON, self.OnStart, self.btnStart)
        self.Bind(wx.EVT_BUTTON, self.OnRmtAT, self.btnRmtAT)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.OnSyncGND, self.btnGNDsynct)
        self.Bind(wx.EVT_TOGGLEBUTTON, self.OnRecALL, self.btnALLrec)
        self.Bind(wx.EVT_BUTTON, self.OnSetBaseTime, self.btnBaseTime)
        self.Bind(wx.EVT_BUTTON, self.OnTX, self.btnTX)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseACM, self.rbACM)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseCMP, self.rbCMP)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnChooseGND, self.rbGND)
        self.Bind(wx.EVT_BUTTON, self.OnClr, self.btnClr)
        self.Bind(wx.EVT_BUTTON, self.OnSaveLog, self.btnSaveLog)
        self.Bind(EVT_STAT, self.OnRXSta)
        self.Bind(EVT_ACM_STAT, self.OnACMSta)
        self.Bind(EVT_CMP_STAT, self.OnCMPSta)
        self.Bind(EVT_GND_STAT, self.OnGNDSta)
        self.Bind(EVT_ACM_DAT, self.OnACMDat)
        self.Bind(EVT_CMP_DAT, self.OnCMPDat)
        self.Bind(EVT_GND_DAT, self.OnGNDDat)
        self.Bind(EVT_EXP_DAT, self.OnExpDat)
        self.Bind(wx.EVT_BUTTON, self.OnTestMotor, self.btnTM)
        self.Bind(wx.EVT_BUTTON, self.OnES, self.btnES)
        self.Bind(wx.EVT_MENU, self.OnES, menu_ES)
        self.Bind(wx.EVT_MENU, self.OnER, menu_ER)
        self.Bind(wx.EVT_BUTTON, self.OnRstRig, self.btnResetRig)
        self.Bind(wx.EVT_CHOICE, self.OnInputType, self.InputType)

        self.OnInputType(None)

        # Set some program flags
        self.keepgoing = True
        self.msg_thread = threading.Thread(target=self.processMsgTask)
        self.msg_thread.daemon = True
        self.msg_thread.start()

    def OnClose(self, event):
        """
        Stop the task queue, terminate processes and close the window.
        """
        busy = wx.BusyInfo("Waiting for processes to terminate...", self)
        # Stop processing tasks and terminate the processes
        self.processTerm()
        self.keepgoing = False
        self.msg_thread.join()
        self.saveConfig()
        self.Destroy()

    def OnLog(self, event) :
        self.log_txt.AppendText(event.log)

    def saveConfig(self):
        parser = self.parser
        parser.set('host','AP', self.txtHost.GetValue())
        parser.set('host','COM', self.txtCOM.GetValue())
        parser.set('host','GND', self.txtGNDhost.GetValue())
        parser.set('host','ACM', self.txtACMhost.GetValue())
        parser.set('host','CMP', self.txtCMPhost.GetValue())
        parser.set('rec','prefix', self.txtRecName.GetValue())
        parser.set('simulink','tx', self.txtMatlabRx.GetValue())
        parser.set('simulink','rx', self.txtMatlabTx.GetValue())
        parser.set('simulink','extra', self.txtMatlabExtra.GetValue())
        cfg = open('config.ini', 'w')
        parser.write(cfg)
        cfg.close()

    def processMsgTask(self):
        """
        Start the execution of tasks by the processes.
        """
        while self.keepgoing:
            try:
                output = self.msgc2guiQueue.get(block=True,timeout=0.2)
                if output['ID'] == 'ExpData':
                    wx.PostEvent(self, EXP_DatEvent(states=output['states']))
                    self.gui2drawerQueue.put_nowait(output)
                elif output['ID'] == 'info':
                    self.log.info(':'.join(['MSGC:',output['content']]))
                elif output['ID'] == 'Statistics':
                    arrv_cnt = output['arrv_cnt']
                    arrv_bcnt = output['arrv_bcnt']
                    elapsed = output['elapsed']
                    wx.PostEvent(self, RxStaEvent(txt=
                    'C{:0>5d}/T{:<.2f} {:03.0f}Pps/{:05.0f}bps'.format(
                        arrv_cnt, elapsed, arrv_cnt / elapsed,
                        arrv_bcnt * 10 / elapsed)))
                elif output['ID'] == 'ACM_STA':
                    wx.PostEvent(self, ACM_StaEvent(txt=output['info'],
                        batpct=output['batpct']))
                elif output['ID'] == 'ACM_DAT':
                    wx.PostEvent(self, ACM_DatEvent(txt=output['info']))
                elif output['ID'] == 'CMP_STA':
                    wx.PostEvent(self, CMP_StaEvent(txt=output['info'],
                        batpct=output['batpct']))
                elif output['ID'] == 'CMP_DAT':
                    wx.PostEvent(self, CMP_DatEvent(txt=output['info']))
                elif output['ID'] == 'GND_STA':
                    wx.PostEvent(self, GND_StaEvent(txt=output['info']))
                elif output['ID'] == 'GND_DAT':
                    wx.PostEvent(self, GND_DatEvent(txt=output['info']))
            except Queue.Empty:
                pass

    def processTerm(self):
        """
        Stop the execution of tasks by the processes.
        """
        # Terminate message center processes
        while self.msg_process.is_alive():
            self.gui2msgcQueue.put_nowait({'ID': 'STOP'})
            self.msg_process.join(0.5)

        if self.graph_process.is_alive():
            self.graph_process.terminate()
            self.graph_process.join()

    def OnStart(self, event):
        self.gui2msgcQueue.put({'ID': 'START',
            'xbee_hosts': [self.txtHost.GetValue(), self.txtGNDhost.GetValue(),
                self.txtACMhost.GetValue(), self.txtCMPhost.GetValue()],
            'xbee_port': 0x2000, 'mano_port': self.txtCOM.GetValue(),
            'matlab_ports': [int(self.txtMatlabRx.GetValue()),
                int(self.txtMatlabTx.GetValue()),
                self.txtMatlabExtra.GetValue()],
                })

        self.btnStart.Enable(False)
        self.txtHost.Enable(False)
        self.txtCOM.Enable(False)
        self.txtGNDhost.Enable(False)
        self.txtACMhost.Enable(False)
        self.txtCMPhost.Enable(False)
        self.txtMatlabRx.Enable(False)
        self.txtMatlabTx.Enable(False)
        self.txtMatlabExtra.Enable(False)
        self.btnALLrec.Enable(True)
        self.btnBaseTime.Enable(True)
        self.btnGNDsynct.Enable(True)
        self.btnRmtAT.Enable(True)
        self.btnTX.Enable(True)
        self.btnTM.Enable(True)
        self.btnES.Enable(True)
        self.btnResetRig.Enable(True)

    def OnRecALL(self, event) :
        if event.IsChecked():
            filename = time.strftime(
                    'FIWT_Exp{:03d}_%Y%m%d%H%M%S.dat'.format(
                        int(self.txtRecName.GetValue()[:3])))
            self.gui2msgcQueue.put({'ID': 'REC_START',
                'filename': filename})
        else:
            self.gui2msgcQueue.put({'ID': 'REC_STOP'})

    def OnSetBaseTime(self, event) :
        self.gui2msgcQueue.put({'ID': 'SET_BASE_TIME'})

    def OnRmtAT(self, event):
        options = self.txtRmtATopt.GetValue().encode()[:2].decode('hex')[0]
        command = self.txtRmtATcmd.GetValue().encode()[:2]
        parameter = self.txtRmtATpar.GetValue().encode()
        self.gui2msgcQueue.put({'ID': 'AT', 'target':self.target,
            'options':options, 'command':command, 'parameter':parameter})

    def OnChooseACM(self, event):
        self.target = 'ACM'
        self.log.info('Target {}'.format(self.target))

    def OnChooseCMP(self, event):
        self.target = 'CMP'
        self.log.info('Target {}'.format(self.target))

    def OnChooseGND(self, event):
        self.target = 'GND'
        self.log.info('Target {}'.format(self.target))

    def OnSyncGND(self, event) :
        self.gui2msgcQueue.put({'ID': 'NTP', 'enable':event.IsChecked()})

    def OnTX(self, event):
        data = self.txtTX.GetValue().encode()
        self.gui2msgcQueue.put({'ID': 'CMD', 'target':self.target, 'command':data})

    def OnRXSta(self, event) :
        self.txtRXSta.SetLabel(event.txt)

    def OnACMSta(self, event) :
        self.txtACMSta.SetLabel(event.txt)
        self.ggACMbat.SetValue(event.batpct)
        if event.batpct < 10:
            self.ggACMbat.SetBarColor(wx.RED)
        elif event.batpct < 20:
            self.ggACMbat.SetBarColor(wx.YELLOW)
        else:
            self.ggACMbat.SetBarColor(wx.GREEN)
        self.ggACMbat.Refresh()

    def OnCMPSta(self, event) :
        self.txtCMPSta.SetLabel(event.txt)
        self.ggCMPbat.SetValue(event.batpct)
        if event.batpct < 10:
            self.ggCMPbat.SetBarColor(wx.RED)
        elif event.batpct < 20:
            self.ggCMPbat.SetBarColor(wx.YELLOW)
        else:
            self.ggCMPbat.SetBarColor(wx.GREEN)
        self.ggCMPbat.Refresh()

    def OnGNDSta(self, event) :
        self.txtGNDSta.SetLabel(event.txt)

    def OnACMDat(self, event) :
        self.txtACMDat.SetLabel(event.txt)

    def OnCMPDat(self, event) :
        self.txtCMPDat.SetLabel(event.txt)

    def OnGNDDat(self, event) :
        self.txtGNDDat.SetLabel(event.txt)

    def OnExpDat(self, event) :
        states = event.states
        txt = ('ACRoll{7:.2f} ACRollRate{1:.2f} '
        'RigRoll{13:07.2f} RigRollRate{14:07.2f} '
        'RigPitch{15:07.2f} RigPitchRate{16:07.2f}\n').format(*states)

        Ax = states[4]
        Ay = states[5]
        Az = states[6]
        roll_s = atan2(-Ay, -Az)*57.3
        pitch_s = atan2(Ax,sqrt(Ay*Ay+Az*Az))*57.3
        txt2 = 'AoA{:-5.1f}/AoS{:-5.1f} Pitch{:-5.1f}/Roll{:-5.1f} PitchS{:-5.1f}/RollS{:-5.1f}'.format(states[51],states[52], states[54],states[53], pitch_s,roll_s)

        self.txtExpDat.SetLabel(txt+txt2)
        msgs = {'data': {
                    'VC': states[39],
                    'VG': states[40],
                    'heading': states[13],
                    'NAV':{ 'SLG': states[15]*10,
                        'CTE': states[17]*10 },
                    'roll': states[53],
                    'pitch': states[54],
                    'AoS': states[52],
                    'AoA': states[51],
                    'Ma': states[52],
                    'GLoad':-states[6],
                    'ASL': states[15],
                    'AGL': states[17],
                    'T': states[0],
                    'mode': 'FIWT',
                    } }
        self.aclink.sendto(json.dumps(msgs), self.aclink_addr)

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
        self.txtACMSta.SetLabel('')
        self.txtCMPSta.SetLabel('')
        self.txtGNDSta.SetLabel('')
        self.txtACMDat.SetLabel('')
        self.txtCMPDat.SetLabel('')
        self.txtGNDDat.SetLabel('')

        self.ggACMbat.SetValue(0)
        self.ggACMbat.Refresh()
        self.ggCMPbat.SetValue(0)
        self.ggCMPbat.Refresh()
        self.txtGNDinfo.SetLabel('')

        self.gui2msgcQueue.put({'ID': 'CLEAR'})

    def OnRstRig(self, event):
        self.gui2msgcQueue.put({'ID': 'RESET_RIG'})

    def OnES(self, event):
        self.gui2msgcQueue.put({'ID': 'EMERGENCY_STOP'})

    def OnER(self, event):
        self.gui2msgcQueue.put({'ID': 'EMERGENCY_CANCELED'})

    def OnInputType(self, event):
        InputType = self.InputType.GetSelection()+1
        if InputType == 1:
            self.StartTime.Enable(False)
            self.TimeDelta.Enable(False)
            self.NofCycles.Enable(False)
            self.ServoRef1.Enable(True)
            self.Srv2Move1.Enable(False)
            self.MaxValue1.Enable(False)
            self.MinValue1.Enable(False)
            self.Sign1.Enable(False)
            self.ServoRef2.Enable(True)
            self.Srv2Move2.Enable(False)
            self.MaxValue2.Enable(False)
            self.MinValue2.Enable(False)
            self.Sign2.Enable(False)
            self.ServoRef3.Enable(True)
            self.Srv2Move3.Enable(False)
            self.MaxValue3.Enable(False)
            self.MinValue3.Enable(False)
            self.Sign3.Enable(False)
            self.ServoRef4.Enable(True)
            self.Srv2Move4.Enable(False)
            self.MaxValue4.Enable(False)
            self.MinValue4.Enable(False)
            self.Sign4.Enable(False)
            self.ServoRef5.Enable(True)
            self.Srv2Move5.Enable(False)
            self.MaxValue5.Enable(False)
            self.MinValue5.Enable(False)
            self.Sign5.Enable(False)
            self.ServoRef6.Enable(True)
            self.Srv2Move6.Enable(False)
            self.MaxValue6.Enable(False)
            self.MinValue6.Enable(False)
            self.Sign6.Enable(False)
        else:
            self.StartTime.Enable(True)
            self.TimeDelta.Enable(True)
            self.NofCycles.Enable(True)
            self.ServoRef1.Enable(False)
            self.Srv2Move1.Enable(True)
            self.MaxValue1.Enable(True)
            self.MinValue1.Enable(False)
            self.Sign1.Enable(True)
            self.ServoRef2.Enable(False)
            self.Srv2Move2.Enable(True)
            self.MaxValue2.Enable(True)
            self.MinValue2.Enable(False)
            self.Sign2.Enable(True)
            self.ServoRef3.Enable(False)
            self.Srv2Move3.Enable(True)
            self.MaxValue3.Enable(True)
            self.MinValue3.Enable(False)
            self.Sign3.Enable(True)
            self.ServoRef4.Enable(False)
            self.Srv2Move4.Enable(True)
            self.MaxValue4.Enable(True)
            self.MinValue4.Enable(False)
            self.Sign4.Enable(True)
            self.ServoRef5.Enable(False)
            self.Srv2Move5.Enable(True)
            self.MaxValue5.Enable(True)
            self.MinValue5.Enable(False)
            self.Sign5.Enable(True)
            self.ServoRef6.Enable(False)
            self.Srv2Move6.Enable(True)
            self.MaxValue6.Enable(True)
            self.MinValue6.Enable(False)
            self.Sign6.Enable(True)
            if InputType in [8,9]:
                self.MinValue1.Enable(True)
                self.MinValue2.Enable(True)

    def OnTestMotor(self, event):
        InputType = self.InputType.GetSelection()+1
        if self.target == 'CMP' :
            Id = 0xA6
        else:
            Id = 0xA5
        if InputType == 1:
            cmd = {'ID': 'CMD',
                   'da': float(self.ServoRef1.GetValue()),
                   'de': float(self.ServoRef2.GetValue()),
                   'dr': float(self.ServoRef3.GetValue()),
                   'da_cmp': float(self.ServoRef4.GetValue()),
                   'de_cmp': float(self.ServoRef5.GetValue()),
                   'dr_cmp': float(self.ServoRef6.GetValue()) }
            self.gui2msgcQueue.put(cmd)
        else :
            Srv2Move = (1 if self.Srv2Move1.GetValue() else 0) \
                     | (2 if self.Srv2Move2.GetValue() else 0) \
                     | (4 if self.Srv2Move3.GetValue() else 0) \
                     | (8 if self.Srv2Move4.GetValue() else 0) \
                     | (16 if self.Srv2Move5.GetValue() else 0) \
                     | (32 if self.Srv2Move6.GetValue() else 0)
            others = [ int(float(self.MaxValue1.GetValue())*4096/180.0/4.0),
                       int(float(self.MaxValue2.GetValue())*4096/180.0/4.0),
                       int(float(self.MaxValue3.GetValue())*4096/180.0/4.0),
                       int(float(self.MaxValue4.GetValue())*4096/180.0/4.0),
                       int(float(self.MaxValue5.GetValue())*4096/180.0/4.0),
                       int(float(self.MaxValue6.GetValue())*4096/180.0/4.0),
                       float(self.MinValue1.GetValue()),
                       float(self.MinValue2.GetValue()),
                       float(self.MinValue3.GetValue()),
                       float(self.MinValue4.GetValue()),
                       float(self.MinValue5.GetValue()),
                       float(self.MinValue6.GetValue()),
                       int(self.Sign1.GetValue()),
                       int(self.Sign2.GetValue()),
                       int(self.Sign3.GetValue()),
                       int(self.Sign4.GetValue()),
                       int(self.Sign5.GetValue()),
                       int(self.Sign6.GetValue()),
                     ]
            starttime = int(int(self.StartTime.GetValue())*1000/1024.0)
            if InputType == 5:
                deltatime = int(int(self.TimeDelta.GetValue())*1000/(1024.0*8))
            elif InputType in [8,9]:
                deltatime = int(int(self.TimeDelta.GetValue())*1000/(1024.0*4))
                others[6] = int(others[6]*25)
                others[7] = int(others[7]*25)
            else:
                deltatime = int(int(self.TimeDelta.GetValue())*1000/1024.0)
            nofcyc = int(self.NofCycles.GetValue())
            data = struct.pack('>Bf2B2HB6B6B6B', Id, 0.0, InputType, Srv2Move,
                    starttime, deltatime, nofcyc, *others)
            self.gui2msgcQueue.put({'ID': 'A5', 'target':self.target,
                'data':data})


