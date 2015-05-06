#!/bin/env python
# -*- coding: utf-8 -*-
"""
AccessPoint(AP) Center in wxPython
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

import wx
from multiprocessing import Process, Queue, freeze_support
from AccessPointFrame import MyFrame
from MessageCenter import worker
from DynamicGraph import drawer

class MyApp(wx.App):
    """
    A simple App class, modified to hold the processes and task queues.
    """

    def __init__(self,
                 redirect=True,
                 filename=None,
                 useBestVisual=False,
                 clearSigInt=True,
                 process=None,
                 gui2drawerQueue=None,
                 gui2msgcQueue=None,
                 msgc2guiQueue=None):
        """
        Initialise the App.
        """
        self.process = process
        self.gui2msgcQueue = gui2msgcQueue
        self.msgc2guiQueue = msgc2guiQueue
        self.gui2drawerQueue=gui2drawerQueue
        wx.App.__init__(self, redirect, filename, useBestVisual, clearSigInt)

    def OnInit(self):
        """
        Initialise the App with a Frame.
        """
        self.frame = MyFrame(None, -1, 'AccessPointCenter', self.process,
                             self.gui2msgcQueue, self.msgc2guiQueue,
                             self.gui2drawerQueue)
        self.frame.Show(True)
        return True


if __name__ == '__main__':

    freeze_support()

    # Create the queues
    gui2msgcQueue = Queue()
    msgc2guiQueue = Queue()
    gui2drawerQueue = Queue()

    # Create the worker process
    msg_process = Process(target=worker, args=(gui2msgcQueue, msgc2guiQueue))
    msg_process.start()

    graph_process = Process(target=drawer, args=(gui2drawerQueue,))
    graph_process.start()

    # Create the app
    app = MyApp(redirect=True,
                filename='AccessPointCenter.stderr.log',
                process=[msg_process, graph_process],
                gui2drawerQueue=gui2drawerQueue,
                gui2msgcQueue=gui2msgcQueue,
                msgc2guiQueue=msgc2guiQueue)
    app.MainLoop()
