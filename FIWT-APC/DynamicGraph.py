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
import wx

from dynamic_chart import HistChart

class MyFrame(wx.Frame):
    def __init__(self, parent, id, title, gui2drawerQueue):
        """
        Initialise the Frame.
        """
        self.gui2drawerQueue = gui2drawerQueue

        wx.Frame.__init__(self, parent, id, title, wx.Point(650, 0),
                          wx.Size(650, 800))
        panel = wx.Panel(self, -1)
        sizer = wx.BoxSizer(wx.VERTICAL)

        self.hpanel = HistChart(panel)
        sizer.Add(self.hpanel, 1, wx.ALL|wx.EXPAND, 1)

        panel.SetSizer(sizer)
        sizer.Fit(panel)

        # Set some program flags
        self.keepgoing = True
        self.msg_thread = threading.Thread(target=self.processMsgTask)
        self.msg_thread.daemon = True
        self.msg_thread.start()

    def processMsgTask(self):
        """
        Start the execution of tasks by the processes.
        """
        while self.keepgoing:
            try:
                output = self.gui2drawerQueue.get(block=True,timeout=0.2)
                if output['ID'] == 'ExpData':
                    states = output['states']
                    ht = self.hpanel
                    ht.data_t.append(states[0])
                    ht.data_y.append(states[19])
                    ht.data_y2.append(states[20])
                    ht.draw_plot()
                elif output['ID'] == 'STOP':
                    self.Destroy()
            except Queue.Empty:
                pass

class MyApp(wx.App):
    """
    A simple App class, modified to hold the processes and task queues.
    """

    def __init__(self,
                 redirect=True,
                 filename=None,
                 useBestVisual=False,
                 clearSigInt=True,
                 gui2drawerQueue=None):
        """
        Initialise the App.
        """
        self.gui2drawerQueue=gui2drawerQueue
        wx.App.__init__(self, redirect, filename, useBestVisual, clearSigInt)

    def OnInit(self):
        """
        Initialise the App with a Frame.
        """
        self.frame = MyFrame(None, -1, 'Drawer', self.gui2drawerQueue)
        self.frame.Show(True)
        return True

def drawer(gui2drawerQueue):
    """
    Worker process to draw data
    """
    # Create the app
    app = MyApp(redirect=True,
                filename='DynamicGraph.stderr.log',
                gui2drawerQueue=gui2drawerQueue)
    app.MainLoop()

