#!/bin/env python
# -*- coding: utf-8 -*-
"""
Message Process Center in Python
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
import select
import Queue, threading
import logging

from MessageFuncs import process_funcs

class RedirectText(object):
    def __init__(self, msg_queue):
        self.msg_queue = msg_queue

    def write(self, string):
        try:
            self.msg_queue.put_nowait({'ID': 'info',
                'content': string.rstrip('\n')})
        except Queue.Full:
            pass

class Worker(object):
    def __init__(self, gui2msgcQueue, msgc2guiQueue):
        self.gui2msgcQueue = gui2msgcQueue
        self.msgc2guiQueue = msgc2guiQueue
        self.socklist = []
        self.fileALL = None

        #logging
        self.log = logging.getLogger(__name__)
        self.log.setLevel(logging.INFO)
        self.log_handle = logging.StreamHandler(RedirectText(self.msgc2guiQueue))
        self.log.addHandler(self.log_handle)

        self.ready = False
        self.main_thread_running = True
        self.msg_thread_running = True
        self.msg_thread = threading.Thread(target=self.processGUImsg)
        self.msg_thread.daemon = True
        self.msg_thread.start()

    def processGUImsg(self):
        while self.msg_thread_running:
            try:
                output = self.gui2msgcQueue.get(block=True,timeout=0.2)
                try:
                    process_funcs[output['ID']](self, output)
                except:
                    self.log.error(traceback.format_exc())
            except Queue.Empty:
                pass

    def MainLoop(self):
        while self.main_thread_running and not self.ready:
            time.sleep(1)
            self.log.info('Waiting for start...')

        self.log.info('Started.')
        while self.main_thread_running:
            rlist,wlist,elist=select.select(self.socklist,[],[],0.2)
            if rlist:
                recv_ts = (time.clock()-self.T0)*1e6
                self.xbee_network.read(rlist, recv_ts)
                self.matlab_link.read(rlist, recv_ts)
        self.log.info('Work end.')
        if self.fileALL:
            self.fileALL.close()
            self.log.info('Stop Recording to {}.'.format(self.filename))
            self.fileALL = None


def worker(gui2msgcQueue, msgc2guiQueue):
    """
    Worker process to manage all messages
    """
    w = Worker(gui2msgcQueue, msgc2guiQueue)
    w.MainLoop()

