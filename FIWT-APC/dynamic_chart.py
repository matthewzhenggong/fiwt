#!/bin/env python
# -*- coding: utf-8 -*-
"""
AccessPoint(AP) GUI graph in wxPython
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

# The recommended way to use wx with mpl is with the WXAgg
# backend.
#
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab
import math

class HistChart(FigCanvas) :

    def __init__(self,parent):
        self.data_t = [0]
        self.data_y = [0]
        self.data_y2 = [0]

        self.dpi = 100
        self.fig = Figure((3.0, 3.0), dpi=self.dpi)

        self.axes = self.fig.add_subplot(211)

        pylab.setp(self.axes.get_xticklabels(), fontsize=18)
        pylab.setp(self.axes.get_yticklabels(), fontsize=18)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        #
        self.plot_data = self.axes.plot(
            self.data_t,self.data_y,'y',
            self.data_t,self.data_y2,'c',
            )

        ymin = -180
        ymax = 180
        self.axes.set_ybound(lower=ymin, upper=ymax)
        self.axes.set_xbound(lower=0, upper=20)
        self.axes.grid(True)

        FigCanvas.__init__(self, parent, -1, self.fig)
        self.cnt = 0


    def draw_plot(self):
        self.cnt = self.cnt+1
        if self.cnt %6 == 3 :
            pass
        else :
            return

        xmin, xmax = self.axes.get_xlim()

        if self.data_t[-1] > xmax:
            self.axes.set_xlim(xmax,xmax+20)
            del data_t[:-2]
            del data_y[:-2]
            del data_y2[:-2]
            pylab.setp(self.axes.get_xticklabels(), visible=True)
            self.draw()

        self.plot_data[0].set_data(self.data_t, self.data_y)
        self.plot_data[1].set_data(self.data_t, self.data_y2)
        #self.draw()


