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

import math, sys, time, types, wx
import Queue
import threading

class MyFrame(wx.Frame):
    """
    Main Frame class.
    """

    def __init__(self, parent, id, title, msg_process, gui2msgcQueue, msgc2guiQueue):
        """
        Initialise the Frame.
        """
        self.msg_process = msg_process
        self.gui2msgcQueue = gui2msgcQueue
        self.msgc2guiQueue = msgc2guiQueue

        wx.Frame.__init__(self, parent, id, title, wx.Point(700, 500),
                          wx.Size(300, 200))

        # Create the panel, sizer and controls
        self.panel = wx.Panel(self, wx.ID_ANY)
        self.sizer = wx.GridBagSizer(5, 5)

        self.output_tc = wx.TextCtrl(self.panel, wx.ID_ANY,
                                     style=wx.TE_MULTILINE | wx.TE_READONLY)

        # Add the controls to the sizer
        self.sizer.Add(self.output_tc, (1, 0),
                       flag=wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM,
                       border=5)
        self.sizer.AddGrowableCol(0)
        self.sizer.AddGrowableRow(1)

        self.panel.SetSizer(self.sizer)

        self.Bind(wx.EVT_CLOSE, self.OnClose)

        # Set some program flags
        self.keepgoing = True
        self.msg_thread = threading.Thread(target=self.processMsgTask)
        self.msg_thread.daemon = True
        self.msg_thread.start()

    def OnClose(self, event):
        """
        Stop the task queue, terminate processes and close the window.
        """
        self.keepgoing = False
        busy = wx.BusyInfo("Waiting for processes to terminate...")
        # Stop processing tasks and terminate the processes
        self.processTerm()
        self.Destroy()

    def processMsgTask(self):
        """
        Start the execution of tasks by the processes.
        """
        while self.keepgoing:
            try:
                output = self.msgc2guiQueue.get(block=True,timeout=0.2)
                self.output_tc.AppendText(''.join([output.__repr__(),'\n']))
            except Queue.Empty:
                pass

    def processTerm(self):
        """
        Stop the execution of tasks by the processes.
        """
        self.keepgoing = False

        # Terminate any running processes
        self.msg_process.terminate()

        # Wait for all processes to stop
        isalive = 1
        while isalive:
            isalive = 0
            isalive = isalive + self.msg_process.is_alive()
            time.sleep(0.5)

