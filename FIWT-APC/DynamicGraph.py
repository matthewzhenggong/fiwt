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

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def run(dat, queue, ax, t,y,line):
    size = queue.qsize()
    if size < 1:
        size = 1
    for i in xrange(size):
        data = queue.get()

    data = data['states']
    t1 = data[0]
    y1 = data[13]
    tmin, tmax = ax.get_xlim()
    if t1 > tmax or t1 < tmin or len(t)>1 and t1 < t[-1]:
        del t[:]
        del y[:]
        ax.set_xlim(t1,t1+20)
        ax.figure.canvas.draw()
    t.append(t1)
    y.append(y1)
    line.set_data(t,y)
    return line,


def drawer(gui2drawerQueue):
    """
    Worker process to draw data
    """
    fig1,ax = plt.subplots()

    t = []
    y = []
    l, = plt.plot(t, y, 'r-')
    plt.xlim(0, 20)
    plt.ylim(-180, 180)
    plt.xlabel('T/s')

    line_ani = animation.FuncAnimation(fig1, run, fargs=(gui2drawerQueue,ax,t,y,l),
        interval=10, blit=True)

    plt.show()

