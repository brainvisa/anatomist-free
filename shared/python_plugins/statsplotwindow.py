# -*- coding: utf-8 -*-
#  This software and supporting documentation are distributed by
#      Institut Federatif de Recherche 49
#      CEA/NeuroSpin, Batiment 145,
#      91191 Gif-sur-Yvette cedex
#      France
#
# This software is governed by the CeCILL-B license under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL-B license as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL-B license and that you accept its terms.


'''
A Matplotlib-based stats window for Anatomist
'''

import anatomist.direct.api as ana
from soma import aims
import numpy as np
import atexit
from functools import partial

from soma.qt_gui.qt_backend import init_matplotlib_backend
init_matplotlib_backend()

from matplotlib import pyplot
import pylab

from soma.qt_gui.qt_backend import QtCore
from soma.qt_gui.qt_backend import QtGui
from soma.qt_gui.qt_backend import QtWidgets
from soma.qt_gui.qt_backend import sip
from soma.utils.weak_proxy import proxy_method


class StatsPlotWindow(ana.cpp.QAWindow):

    '''A Matplotlib-based stats plot window for Anatomist.
    It is designed in python, and python-inherited classes suffer from
    reference-counting problems. See the doc of the releaseref() method.
    '''
    _instances = {}
    _classType = ana.cpp.AWindow.Type(0)

    colors_list = [
        (182, 0, 0),
        (78, 182, 68),
        (39, 53, 242),
        (237, 255, 114),
        (109, 179, 174),
        (175, 109, 179),
        (125, 25, 25),
        (21, 108, 21),
        (28, 44, 92),
        (181, 100, 20),
        (20, 181, 103),
        (138, 39, 189),
        (243, 142, 142),
        (130, 177, 99),
        (47, 116, 159),
        (132, 68, 45),
        (22, 91, 68),
        (147, 37, 112),
    ]

    def __init__(self, parent=None, name=None, options=aims.Object(), f=None):
        '''The releaseref() method should be called after the constructor - see
        the doc of this method.
        It is not called from the constructor for technical anatomist IDs problems
        (which may be solved).
        '''
        if f is None:
            f = QtCore.Qt.WindowType(QtCore.Qt.Window)
        ana.cpp.QAWindow.__init__(self, parent, name, options, f)
        self._fig = pyplot.figure()
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self._fig.set_facecolor(str(self.palette().color(
            QtGui.QPalette.Active, QtGui.QPalette.Window).name()))
        wid = pyplot._pylab_helpers.Gcf.get_fig_manager(
            self._fig.number).window
        wid.setParent(self)
        self.setCentralWidget(wid)
        # keep a reference to the python object to prevent destruction of the
        # python part
        StatsPlotWindow._instances[sip.unwrapinstance(self)] = self
        self.destroyed.connect(StatsPlotWindow.destroyNotified)
        self._plots = {}
        self._orientation = aims.Quaternion(0, 0, 0, 1)
        self._cursorplot = None
        self._coordindex = 0  # x axis
        self._display_method_index = 0
        self._display_methods = [
            proxy_method(self.draw_boxplot),
            proxy_method(self.draw_errorbars),
            proxy_method(self.draw_violinplot),
        ]
        self._display_method = self._display_methods[
            self._display_method_index]
        self._orientation = 'vertical'

        self.color_modes = {
            'color_list': proxy_method(self.get_value_color_colorlist),
            'value_palette': proxy_method(self.get_value_color_valuepalette),
            'object': proxy_method(self.get_value_color_object),
            'object_palette': proxy_method(self.get_value_color_objpalette),
        }

        self._color_mode = 'color_list'
        self.get_value_color = self.color_modes[self._color_mode]
        self._palette = None
        self._palette_obj = None
        self._direction = 3

        # add toolbar
        toolbar = QtWidgets.QToolBar(wid)
        toolbar.addAction('>', self.mute_plot_type)
        toolbar.addAction('↺', self.mute_orientation)
        toolbar.addAction('⚙', self.open_settings)

        tb = QtWidgets.QToolButton()
        tb.setText('T')
        toolbar.addWidget(tb)
        menu = QtWidgets.QMenu()
        menu.addAction('X', partial(self.set_direction, 0))
        menu.addAction('Y', partial(self.set_direction, 1))
        menu.addAction('Z', partial(self.set_direction, 2))
        menu.addAction('T', partial(self.set_direction, 3))
        menu.addAction('X5', partial(self.set_direction, 4))
        menu.addAction('X6', partial(self.set_direction, 5))
        menu.addAction('X7', partial(self.set_direction, 6))
        menu.addAction('X8', partial(self.set_direction, 7))
        tb.setMenu(menu)
        tb.setPopupMode(tb.InstantPopup)
        menu.triggered.connect(tb.setDefaultAction)

        wid.addToolBar(toolbar)

        # no referential bar button: the ref is managed automatically

        # close shortcut
        ac = QtGui.QAction('Close', self)
        ac.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL | QtCore.Qt.Key_W))
        ac.triggered.connect(self.closeAction)
        self.addAction(ac)
        self.fcolors_list = [[x/256. for x in y] for y in self.colors_list]

    def releaseref(self):
        '''WARNING:
        the instance in _instances shouldn't count on C++ side
        PROBLEM: all python refs are one unique ref for C++,
        all being of the same type, so later references will not be strong
        refs.
        the less annoying workaround at the moment is that python refs are
        'weak shared references': count as references to keep the object alive,
        but don't actually prevent its destruction whenever the close method
        or anatomist destroy command are called. In such case the python object
        will hold a deleted C++ object.
        This way, only C++ may destroy the object.
        When the C++ instance is destroyed, the QObject destroyed callback is
        used to cleanup the additional python reference in
        StatsPlotWindow._instances
        so that the python instance can also be destroyed when python doesn't
        use it any longer.
        That's the best I can do for now...
        This releaseref method should be called after the constructor: it is
        called from the createHistogramWindow factory class.
        this means you should _not_ create an instance of StatsPlotWindow
        directly.'''
        a = ana.Anatomist()
        a.execute('ExternalReference', elements=[self],
                  action_type='TakeWeakSharedRef')
        a.execute('ExternalReference', elements=[self],
                  action_type='ReleaseStrongRef')

    def __del__(self):
        # print 'StatsPlotWindow.__del__'
        ana.cpp.QAWindow.__del__(self)

    @staticmethod
    def destroyNotified(self):
        # print 'destroyNotified'
        # release internal reference which kept the python side of the object
        # alive - now the python object may be destroyed since the C++ side
        # will be also destroyed anyway.
        ptr = sip.unwrapinstance(self)
        if ptr in StatsPlotWindow._instances:
            del StatsPlotWindow._instances[ptr]

    def type(self):
        return self._classType

    def registerObject(self, obj, temporaryObject=False, position=-1):
        if hasattr(obj, 'internalRep'):
            obj = obj.internalRep
        if not self.hasObject(obj):
            ana.cpp.QAWindow.registerObject(
                self, obj, temporaryObject, position)
            self.Refresh()

    def unregisterObject(self, obj):
        if hasattr(obj, 'internalRep'):
            obj = obj.internalRep
        ana.cpp.QAWindow.unregisterObject(self, obj)
        self.Refresh()

    def plotObject(self, obj):
        ''' Prepare a data point for a single object
        '''
        if obj.objectTypeName(obj.type()) == 'VOLUME':
            vol = ana.cpp.AObjectConverter.aims(obj)
            ar = vol.np
            pos = self.getFullPosition()
            vs = vol.getVoxelSize()
            while len(vs) < len(pos):
                vs.append(1.)

            opos = pos
            oref = obj.getReferential()
            wref = self.getReferential()
            a = ana.Anatomist()
            trans = a.getTransformation(wref, oref)
            # print('pos:', pos)
            if trans is not None:
                # get in object space
                opos = trans.transform(pos[:3])
                pos[:3] = opos
            vpos = np.round(pos.np / vs.np).astype(int)
            if len(vpos) > self._direction:
                vpos[self._direction] = 0
            dims = vol.getSize()
            if np.min(vpos) < 0 or np.min(dims.np - 1 - vpos) < 0:
                self.data.append(np.array([]))
                self._obj_indices.append(1)
                return
            # print('obj:', obj.name(), ', pos:', pos, vpos, ', v:', ar[tuple(vpos)])
            index = list(vpos)
            if len(vpos) > self._direction:
                index[self._direction] = slice(dims[self._direction])
            else:
                # use a slice anyway in order to get an array, not a scalar
                index[-1] = slice(index[-1], index[-1] + 1)
            self.data.append(ar[tuple(index)])
            self._obj_indices.append(1)  # for now 1 value per object
        else:
            data = obj.texValuesSeries(self.getFullPosition(), self._direction)
            data = data.np[:, :, 0, 0]
            if data.shape[1] == 1:
                data = data[:, 0]
            self.data.append(data)
            self._obj_indices.append(1)

    def baseTitle(self):
        return 'Stats plot'

    def Refresh(self):
        ''' Redraw the full graph
        '''
        # set the referential of the first object
        objects = [o for o in self.Objects()
                   if self._palette_obj is None
                   or o is not self._palette_obj.internalRep]
        if len(objects) != 0:
            oref = objects[0].getReferential()
            if oref != self.getReferential():
                self.setReferential(oref)
        ana.cpp.QAWindow.Refresh(self)
        self.data = []
        self._obj_indices = []
        xlabels = []
        colors = []
        i = 0
        for obj in objects:
            self.plotObject(obj)
            xlabels.append(self.get_object_label(obj, self.data[i], i))
            if len(self.data[-1]) == 0:
                colors.append([0., 0., 0.])
            else:
                colors.append(self.get_value_color(obj, self.data[i], i))
            i += self._obj_indices[-1]

        figure = pyplot.figure(self._fig.number)
        figure.clear()

        self._display_method(np.arange(len(self.data)), self.data, xlabels,
                             colors)
        self._fig.canvas.draw()

    def draw_boxplot(self, x, y, labels, colors):
        if len(x) == 0 or len(y) == 0:
            return  # plot fails if no data is given
        bplot = pylab.boxplot(x=y, labels=labels, patch_artist=True,
                              vert=(self._orientation == 'vertical'))
        if self._orientation == 'vertical':
            pylab.xticks(x + 1, labels, rotation='vertical')
        else:
            pylab.yticks(x + 1, labels)
        for patch, color in zip(bplot['boxes'], colors):
            patch.set_facecolor(color)

    def draw_errorbars(self, x, y, labels, colors):
        avg = []
        std = []
        for d in y:
            if len(d) == 2:
                avg.append(d[0])
                std.append(d[1])
            else:
                avg.append(np.average(d))
                std.append(np.std(d))

        if self._orientation == 'vertical':
            pylab.errorbar(x, y=avg, yerr=std, fmt='o', linewidth=2, capsize=6)
            pylab.xticks(x, labels, rotation='vertical')
        else:
            pylab.errorbar(x=avg, y=x, xerr=std, fmt='o', linewidth=2,
                           capsize=6)
            pylab.yticks(x, labels)

    def draw_violinplot(self, x, y, labels, colors):
        invalid = set(i for i in range(len(y)) if len(y[i]) == 0)
        if len(invalid) != 0:
            x = [z for i, z in enumerate(x) if i not in invalid]
            y = [z for i, z in enumerate(y) if i not in invalid]
            labels = [z for i, z in enumerate(labels) if i not in invalid]
        pylab.violinplot(dataset=y, positions=x, showmeans=True,
                         showextrema=True,
                         vert=(self._orientation == 'vertical'))
        if self._orientation == 'vertical':
            pylab.xticks(x, labels, rotation='vertical')
        else:
            pylab.yticks(x, labels)

    def mute_orientation(self):
        if self._orientation == 'vertical':
            self._orientation = 'horizontal'
        else:
            self._orientation = 'vertical'
        self.Refresh()

    def mute_plot_type(self):
        self._display_method_index \
            = (self._display_method_index + 1) % len(self._display_methods)
        self._display_method \
            = self._display_methods[self._display_method_index]
        self.Refresh()

    def closeAction(self, dummy):
        self.close()

    def get_value_color_colorlist(self, obj, value, index):
        return self.fcolors_list[index % len(self.fcolors_list)]

    def get_value_color_valuepalette(self, obj, value, index):
        if self._palette_obj is None:
            return self.get_value_color_colorlist(obj, value, index)
        pal = self._palette_obj.palette()
        if pal is None:
            return self.get_value_color_colorlist(obj, value, index)
        glc = self._palette_obj.glAPI()
        te = glc.glTexExtrema(0)
        value = np.average(value)
        if te.minquant[0] != te.maxquant[0]:
            value = (value - te.minquant[0]) \
                / (te.maxquant[0] - te.minquant[0])
        rgb = pal.normColor(value, 0)
        return [x / 256. for x in rgb[:3]]

    def get_value_color_object(self, obj, value, index):
        return self.fcolors_list[index % len(self.fcolors_list)]

    def get_value_color_objpalette(self, obj, value, index):
        pal = obj.palette()
        if pal is None:
            return self.get_value_color_colorlist(obj, value, index)
        glc = obj.glAPI()
        if glc is not None:
            te = glc.glTexExtrema(0)
            value = np.average(value)
            if te.minquant[0] != te.maxquant[0]:
                value = (value - te.minquant[0]) \
                    / (te.maxquant[0] - te.minquant[0])
        rgb = pal.normColor(value, 0)
        return [x / 256. for x in rgb[:3]]

    def get_object_label(self, obj, value, index):
        return obj.name()

    def open_settings(self):
        settingsw = QtWidgets.QDialog()
        lay = QtWidgets.QVBoxLayout()
        settingsw.setLayout(lay)

        colmode = QtWidgets.QGroupBox('Color mode:')
        cml = QtWidgets.QVBoxLayout()
        colmode.setLayout(cml)
        colmodes = list(self.color_modes.keys())
        rbut = []
        for cm in colmodes:
            cmr = QtWidgets.QRadioButton(cm)
            rbut.append(cmr)
            if cm == self._color_mode:
                cmr.setChecked(True)
            cml.addWidget(cmr)
        lay.addWidget(colmode)

        palgrp = QtWidgets.QGroupBox('Palette:')
        pl = QtWidgets.QVBoxLayout()
        palgrp.setLayout(pl)
        palb = QtWidgets.QPushButton('Edit')
        pl.addWidget(palb)
        palb.clicked.connect(self.edit_palette)
        palb.clicked.connect(settingsw.accept)
        lay.addWidget(palgrp)

        blay = QtWidgets.QHBoxLayout()
        lay.addLayout(blay)
        blay.addStretch(1)
        ok = QtWidgets.QPushButton('OK')
        blay.addWidget(ok)
        ok.clicked.connect(settingsw.accept)
        cancel = QtWidgets.QPushButton('Cancel')
        blay.addWidget(cancel)
        cancel.clicked.connect(settingsw.reject)

        res = settingsw.exec()
        if res != QtWidgets.QDialog.Accepted:
            return
        sel = [i for i, b in enumerate(rbut) if b.isChecked()]
        self._color_mode = colmodes[sel[0]]
        del rbut
        self.get_value_color = self.color_modes[self._color_mode]
        self.Refresh()

    def update(self, observable, arg):
        self.Refresh()

    def edit_palette(self):
        ''' Edit a palette (associated with a fake object) for the
        "value_palette" color mode
        '''
        a = ana.Anatomist()
        if self._palette_obj is None:
            vol = aims.Volume_FLOAT(2, 1)
            self._palette_obj = a.toAObject(vol)
            a.unregisterObject(self._palette_obj)
            a.registerObject(self._palette_obj, False)
            self._palette_obj.releaseAppRef()
            self.registerObject(self._palette_obj)
        vol = a.toAimsObject(self._palette_obj)
        vol[0, 0, 0, 0] = 0.
        vol[1, 0, 0, 0] = 1.
        objects = [o for o in self.Objects()
                   if o is not self._palette_obj.internalRep]
        i = 0
        for o in objects:
            glc = o.glAPI()
            if glc is not None:
                te = glc.glTexExtrema(0)
                if i == 0:
                    vol[0, 0, 0, 0] = te.minquant[0]
                    vol[1, 0, 0, 0] = te.maxquant[0]
                else:
                    vol[0, 0, 0, 0] = np.min((te.minquant[0], vol[0, 0, 0, 0]))
                    vol[1, 0, 0, 0] = np.max((te.maxquant[0], vol[1, 0, 0, 0]))
                i += 1
        self._palette_obj.setInternalsChanged()
        self._palette_obj.internalUpdate()
        self._palette_obj.notifyObservers()
        a.execute('PopupPalette', objects=[self._palette_obj])

    def set_direction(self, direction):
        self._direction = direction
        self.Refresh()


class StatsPlotModule(ana.cpp.Module):

    def name(self):
        return 'Stats plot window module'

    def description(self):
        return __doc__


class createStatsPlotWindow(ana.cpp.AWindowCreator):

    def __call__(self, dock, options):
        h = StatsPlotWindow()
        h.releaseref()
        h.show()
        return h


createstatsplot = createStatsPlotWindow()


def init():
    StatsPlotWindow._classType \
        = StatsPlotWindow.Type(ana.cpp.AWindowFactory.registerType(
            'StatsPlot', createstatsplot, True))
    ana.cpp.QAWindowFactory.loadDefaultPixmaps('stats')
    ana.cpp.AWindowFactory.setHasControlWindowButton(
        ana.cpp.AWindowFactory.typeID('StatsPlot'), False)


def clean_wincreator():
    ana.cpp.AWindowFactory.unregisterType('StatsPlot')
    global createstatsplot
    createstatsplot = None


hm = StatsPlotModule()
init()
atexit.register(clean_wincreator)
