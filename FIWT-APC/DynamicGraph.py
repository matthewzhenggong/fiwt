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
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from itertools import izip

def run(dat, queue, t,ax_data,lines):
    size = queue.qsize()
    if size < 1:
        size = 1
    for i in xrange(size):
        data = queue.get()

    data = data['states']
    t1 = data[0]
    tmin, tmax = ax_data[0][1].get_xlim()
    if t1 > tmax or t1 < tmin or len(t)>1 and t1 < t[-1]:
        del t[:]
        for i in ax_data:
            for j in i[0]:
                del j[:]
            i[1].set_xlim(t1,t1+10)
        ax_data[0][1].figure.canvas.draw()
    t.append(t1)
    d = ax_data[0][0]
    d[0].append(data[13])
    d[1].append(data[15])
    d[2].append(data[17])
    d = ax_data[1][0]
    #d[0].append(data[7])
    #d[1].append(data[9])
    d[0].append(data[53])
    d[1].append(data[54])
    d[2].append(data[11])
    d = ax_data[2][0]
    for i in xrange(8):
        d[i].append(data[31+i])
    d = ax_data[3][0]
    for i in xrange(4):
        d[i].append(data[41+i])
    d = ax_data[4][0]
    for i in xrange(4):
        d[i].append(data[19+i])
    for i in xrange(6):
        d[i+4].append(data[25+i])
    d = ax_data[5][0]
    for i in xrange(2):
        d[i].append(data[45+i])
    for i in xrange(3):
        d[i+2].append(data[48+i])
    d = ax_data[6][0]
    for i in range(3):
        d[i].append(data[14+i*2])
    d = ax_data[7][0]
    for i in range(3):
        #d[i].append(data[8+i*2])
        d[i].append(data[1+i])
    if len(t)>2:
        for i in ax_data:
            for j,k in izip(i[2],i[0]):
                j.set_data(t,k)
        return lines
    else:
        return ()


def drawer(gui2drawerQueue):
    """
    Worker process to draw data
    """
    fig1,axarr = plt.subplots(4,2,sharex=True)
    mngr = plt.get_current_fig_manager()
    mngr.window.wm_geometry("700x800+720+0")

    t = []

    rigpos = [[],[],[]]
    rigpos_ax = axarr[0][0]
    rigpos_l = rigpos_ax.plot(t, rigpos[0], 'r-',t, rigpos[1], 'g-',
            t, rigpos[2], 'b-')
    rigpos_ax.grid(True)
    rigpos_ax.set_xlim(0, 10)
    rigpos_ax.set_ylim(-180, 180)
    rigpos_ax.set_ylabel(r'$RigPos/^o$')

    rigrate = [[],[],[]]
    rigrate_ax = axarr[0][1]
    rigrate_l = rigrate_ax.plot(t, rigrate[0], 'r-',t, rigrate[1], 'g-',
            t, rigrate[2], 'b-')
    rigrate_ax.grid(True)
    rigrate_ax.set_xlim(0, 10)
    rigrate_ax.set_ylim(-380, 380)
    rigrate_ax.set_ylabel(r'$RigRate / ^o/s$')

    acmpos = [[],[],[]]
    acmpos_ax = axarr[1][0]
    acmpos_l = acmpos_ax.plot(t, acmpos[0], 'r-',t, acmpos[1], 'g-',
            t, acmpos[2], 'b-')
    acmpos_ax.grid(True)
    acmpos_ax.set_xlim(0, 10)
    acmpos_ax.set_ylim(-90, 90)
    acmpos_ax.set_ylabel(r'$AcmPos/^o$')

    acmrate = [[],[],[]]
    acmrate_ax = axarr[1][1]
    acmrate_l = acmrate_ax.plot(t, acmrate[0], 'r-',t, acmrate[1], 'g-',
            t, acmrate[2], 'b-')
    acmrate_ax.grid(True)
    acmrate_ax.set_xlim(0, 10)
    acmrate_ax.set_ylim(-380, 380)
    acmrate_ax.set_ylabel(r'$AcmPos / ^o/s$')

    cmpsvo = [[],[],[],[],[],[],[],[]]
    cmpsvo_ax = axarr[2][0]
    cmpsvo_l = cmpsvo_ax.plot(t, cmpsvo[0], 'r-',t, cmpsvo[1], 'r:',
            t, cmpsvo[2], 'b-', t, cmpsvo[3], 'b:',
            t, cmpsvo[4], 'g-', t, cmpsvo[5], 'g:',
            t, cmpsvo[6], 'm-', t, cmpsvo[7], 'm:')
    cmpsvo_ax.grid(True)
    cmpsvo_ax.set_xlim(0, 10)
    cmpsvo_ax.set_ylim(-50, 50)
    cmpsvo_ax.set_ylabel(r'$CmpServo/^o$')

    cmpmot = [[],[],[],[]]
    cmpmot_ax = axarr[2][1]
    cmpmot_l = cmpmot_ax.plot(t, cmpmot[0], 'r-', t, cmpmot[1], 'b-',
            t, cmpmot[2], 'g-', t, cmpmot[3], 'm-')
    cmpmot_ax.grid(True)
    cmpmot_ax.set_xlim(0, 10)
    cmpmot_ax.set_ylim(-1000, 1000)
    cmpmot_ax.set_ylabel(r'$CmpMot$')

    acmsvo = [[],[],[],[],[],[],[],[],[],[]]
    acmsvo_ax = axarr[3][0]
    acmsvo_l = acmsvo_ax.plot(t, acmsvo[0], 'r:',t, acmsvo[1], 'r-',
            t, acmsvo[2], 'b:', t, acmsvo[3], 'b-',
            t, acmsvo[4], 'g:', t, acmsvo[5], 'g-',
            t, acmsvo[6], 'm:', t, acmsvo[7], 'm-',
            t, acmsvo[8], 'm:', t, acmsvo[9], 'c-')
    acmsvo_ax.grid(True)
    acmsvo_ax.set_xlim(0, 10)
    acmsvo_ax.set_ylim(-30, 30)
    acmsvo_ax.set_ylabel(r'$AcmServo/^o$')
    acmsvo_ax.set_xlabel(r'$T/s$')

    acmmot = [[],[],[],[],[]]
    acmmot_ax = axarr[3][1]
    acmmot_l = acmmot_ax.plot(t, acmmot[0], 'r-', t, acmmot[1], 'b-',
            t, acmmot[2], 'g-', t, acmmot[3], 'm-', t, acmmot[4], 'c-')
    acmmot_ax.grid(True)
    acmmot_ax.set_xlim(0, 10)
    acmmot_ax.set_ylim(-1000, 1000)
    acmmot_ax.set_ylabel(r'$AcmMot$')
    acmmot_ax.set_xlabel(r'$T/s$')

    ax_data = [ [rigpos, rigpos_ax, rigpos_l],
                [acmpos, acmpos_ax, acmpos_l],
                [cmpsvo, cmpsvo_ax, cmpsvo_l],
                [cmpmot, cmpmot_ax, cmpmot_l],
                [acmsvo, acmsvo_ax, acmsvo_l],
                [acmmot, acmmot_ax, acmmot_l],
                [rigrate, rigrate_ax, rigrate_l],
                [acmrate, acmrate_ax, acmrate_l],
              ]
    lines = rigpos_l+rigrate_l+acmpos_l+acmrate_l\
            +cmpsvo_l+cmpmot_l+acmsvo_l+acmmot_l

    fig1.subplots_adjust(wspace=0.3,hspace=0.1)

    line_ani = animation.FuncAnimation(fig1, run, fargs=(gui2drawerQueue,
        t,ax_data, lines), interval=10, blit=True)

    plt.show()

