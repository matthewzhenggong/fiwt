#!/bin/env python
# -*- coding: utf-8 -*-
"""
Message Process Functions in Python
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

import struct, math, time, traceback

from MatlabLink import MatlabLink
from XBeeWifiNetwork import XBeeNetwork
from utils import getMicroseconds

import Manimeter
from ExpData import ExpData

def msg_start(self, cmd):
    if not self.ready:
        self.log.info('Starting...')
        self.extra_simulink_inputs = cmd['matlab_ports'][2].split()
        self.extra_simulink_input_number = len(self.extra_simulink_inputs)
        self.zero_vector = [0]*self.extra_simulink_input_number
        self.expData = ExpData(self, self.msgc2guiQueue,
                self.extra_simulink_inputs)
        self.host = cmd['xbee_hosts'][0]
        self.node_addr = { 'GND': (cmd['xbee_hosts'][1],cmd['xbee_port']),
                'ACM': (cmd['xbee_hosts'][2],cmd['xbee_port']),
                'CMP': (cmd['xbee_hosts'][3],cmd['xbee_port']) }
        self.xbee_network = XBeeNetwork(self,cmd['xbee_hosts'],cmd['xbee_port'])
        self.socklist += self.xbee_network.getReadList()
        self.matlab_link = MatlabLink(self, cmd['matlab_ports'])
        self.socklist += self.matlab_link.getReadList()
        self.T0 = getMicroseconds()
        self.ready = True
        self.expData.xbee_network = self.xbee_network
        self.expData.ACM_node = self.node_addr['ACM']
        self.expData.CMP_node = self.node_addr['CMP']
        self.mano = Manimeter.Manimeter(self.expData,cmd['mano_port'])

def msg_stop(self, cmd):
    self.main_thread_running = False
    self.msg_thread_running = False
    if self.mano:
        self.mano.running = False

def cmd_rec_start(self, cmd):
    self.filename = cmd['filename']
    self.fileALL = open(self.filename, 'wb')
    self.log.info('Recording to {}.'.format(self.filename))

def cmd_rec_stop(self, cmd):
    if self.fileALL:
        while self.writing :
            pass
        a = self.fileALL
        self.fileALL = None
        a.close()
        self.log.info('Stop Recording to {}.'.format(self.filename))

def cmd_set_base_time(self, cmd):
    self.T0 = getMicroseconds()
    self.log.info('Reset T0')

def cmd_at(self, cmd):
    target = cmd['target']
    remote_host = self.node_addr[target][0]
    options = cmd['options']
    command = cmd['command']
    parameter = cmd['parameter']
    if len(parameter) == 0:
        parameter = None
        self.log.info('get AT ' + command + ' from ' + remote_host +
            ' with option {:02x}'.format(ord(options)))
    else:
        if len(parameter) % 2 == 1:
            parameter = '0' + parameter
        parameter = parameter.decode('hex')
        self.log.info('send AT ' + command + '=' + ':'.join(
            '{:02x}'.format(ord(c)) for c in parameter) + ' to '
            + remote_host + ' with option {:02x}'.format(ord(options)))
    self.xbee_network.service.sendConfigCommand(remote_host,
        command, parameter, frame_id=1, options=options)

def cmd_ntp(self, cmd):
    self.xbee_network.response_ntp = cmd['enable']

def cmd_command(self, cmd):
    da = cmd['da']
    dea = 0
    de = cmd['de']
    dr = cmd['dr']
    da_cmp = cmd['da_cmp']
    de_cmp = cmd['de_cmp']
    dr_cmp = cmd['dr_cmp']
    self.expData.sendCommand(0, da, dea, de, dr, da_cmp, de_cmp, dr_cmp,
            *self.zero_vector)

def cmd_clear(self, cmd):
    self.xbee_network.arrv_cnt = -1
    self.max_dt = 0

def cmd_A5(self, cmd):
    target = cmd['target']
    remote_host = self.node_addr[target]
    data = cmd['data']
    self.xbee_network.send(data,remote_host)

def cmd_reset_rig(self, cmd):
    self.expData.resetRigAngel()

def cmd_emergency_stop(self, cmd):
    self.emergency_stop = False
    self.expData.sendCommand(0, 0, 0, 0, 0, 0, 0, 0,*self.zero_vector)
    self.emergency_stop = True

def cmd_emergency_cancelled(self, cmd):
    self.emergency_stop = False

process_funcs = {'START':msg_start,
    'STOP':msg_stop,
    'REC_START':cmd_rec_start,
    'REC_STOP':cmd_rec_stop,
    'SET_BASE_TIME':cmd_set_base_time,
    'AT':cmd_at,
    'NTP':cmd_ntp,
    'CMD':cmd_command,
    'CLEAR':cmd_clear,
    'RESET_RIG':cmd_reset_rig,
    'A5':cmd_A5,
    'EMERGENCY_STOP':cmd_emergency_stop,
    'EMERGENCY_CANCELED':cmd_emergency_cancelled,
    }

