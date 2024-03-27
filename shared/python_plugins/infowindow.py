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
Information window for Anatomist
'''

import anatomist.direct.api as ana
from soma import aims
import atexit

from soma.qt_gui.qt_backend import QtCore
from soma.qt_gui.qt_backend import Qt
from soma.qt_gui.qt_backend import QtGui
from soma.qt_gui.qt_backend import sip


class InfoWindow(ana.cpp.QAWindow):

    '''Info window for Anatomist.
    It is designed in python, and python-inherited classes suffer from
    reference-counting problems. See the doc of the releaseref() method.
    '''
    _instances = {}
    _classType = ana.cpp.AWindow.Type(0)

    def __init__(self, parent=None, name=None, options=aims.Object(), f=None):
        '''The releaseref() method should be called after the constructor - see
        the doc of this method.
        It is not called from the constructor for technical anatomist IDs
        problems
        (which may be solved).
        '''
        if f is None:
            f = QtCore.Qt.WindowType(QtCore.Qt.Window)
        ana.cpp.QAWindow.__init__(self, parent, name, options, f)

        self._objects_rows = {}

        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        wid = Qt.QWidget()
        self.setCentralWidget(wid)

        layout = Qt.QVBoxLayout()
        wid.setLayout(layout)

        # referential bar
        refbox = QtGui.QWidget(wid)
        layout.addWidget(refbox)
        rlay = QtGui.QHBoxLayout(refbox)
        refbox.setLayout(rlay)
        rbut = QtGui.QPushButton(refbox)
        rlay.addWidget(rbut)
        icons = ana.cpp.IconDictionary.instance()
        directpix = icons.getIconInstance('direct_ref_mark')
        refdirmark = QtGui.QLabel(refbox)
        if directpix is not None:
            refdirmark.setPixmap(directpix)
        rlay.addWidget(refdirmark)
        refdirmark.setFixedSize(QtCore.QSize(21, 7))
        rbut.setFixedHeight(7)
        refbox.setFixedHeight(refbox.sizeHint().height())
        self._refbutton = rbut
        self._reflabel = refdirmark
        ana.cpp.anatomist.setQtColorStyle(rbut)
        self.paintRefLabel()
        rbut.clicked.connect(self.changeReferential)

        coord_grp = Qt.QGroupBox('Coords:')
        layout.addWidget(coord_grp)
        clay = Qt.QGridLayout()
        coord_grp.setLayout(clay)
        self.coord_lay = clay
        self.coord_items = []

        values_grp = Qt.QLabel('Values:')
        layout.addWidget(values_grp)
        vlist = Qt.QTableWidget()
        layout.addWidget(vlist)
        vlist.setColumnCount(4)
        vlist.setHorizontalHeaderLabels(['Object:', 'Vertex:', 'Value:',
                                         'Label:'])
        self.val_table = vlist

        # keep a reference to the python object to prevent destruction of the
        # python part. We keep a "raw pointer" to it, because the destroyed
        # signal is emitted when higher-levels of the C++ object are already
        # destroyed: we just get a QWidget object pointer, not a QAWindow. But
        # the raw pointer allows to identify the object.
        InfoWindow._instances[sip.unwrapinstance(self)] = self
        self.destroyed.connect(InfoWindow.destroyNotified)

        # close shortcut
        ac = QtGui.QAction('Close', self)
        ac.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL | QtCore.Qt.Key_W))
        ac.triggered.connect(self.closeAction)
        self.addAction(ac)

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
        InfoWindow._instances
        so that the python instance can also be destroyed when python doesn't
        use it any longer.
        That's the best I can do for now...
        This releaseref method should be called after the constructor: it is
        called from the createHistogramWindow factory class.
        this means you should _not_ create an instance of InfoWindow
        directly.'''
        a = ana.Anatomist()
        a.execute('ExternalReference', elements=[self],
                  action_type='TakeWeakSharedRef')
        a.execute('ExternalReference', elements=[self],
                  action_type='ReleaseStrongRef')

    def __del__(self):
        # print('InfoWindow.__del__')
        super().__del__()

    @staticmethod
    def destroyNotified(obj):
        # print('destroyNotified')
        # release internal reference which kept the python side of the object
        # alive - now the python object may be destroyed since the C++ side
        # will be also destroyed anyway.
        # obj is the C++ object, already partly deleted, thus it is not a
        # QAWindow, but a QWidget. We use a raw pointer to identify the window
        # we have registered in the constructor.
        ptr = sip.unwrapinstance(obj)
        if ptr in InfoWindow._instances:
            del InfoWindow._instances[ptr]

    def type(self):
        return self._classType

    def registerObject(self, obj, temporaryObject=False, position=-1):
        if hasattr(obj, 'internalRep'):
            obj = obj.internalRep
        if not self.hasObject(obj):
            first = (len(self.Objects()) == 0)
            ana.cpp.QAWindow.registerObject(
                self, obj, temporaryObject, position)
            row = self.val_table.rowCount()
            self.val_table.setRowCount(row + 1)
            self._objects_rows[obj] = row
            self.val_table.setItem(
                row, 0, Qt.QTableWidgetItem(self.get_object_label(obj)))

            # set the referential of the first object
            if first:
                oref = obj.getReferential()
                if oref != self.getReferential():
                    self.setReferential(oref)

            self.Refresh()

    def unregisterObject(self, obj):
        if hasattr(obj, 'internalRep'):
            obj = obj.internalRep
        row = self._objects_rows.get(obj)
        if row is not None:
            self.val_table.removeRow(row)
            del self._objects_rows[obj]
            for oid, r in self._objects_rows.items():
                if r > row:
                    self._objects_rows[oid] -= 1
        ana.cpp.QAWindow.unregisterObject(self, obj)
        self.Refresh()

    def baseTitle(self):
        return 'Info'

    def Refresh(self):
        ''' Redraw
        '''
        ana.cpp.QAWindow.Refresh(self)

        fpos = self.getFullPosition()
        wref = self.getReferential()

        for obj in self.Objects():
            row = self._objects_rows.get(obj)
            if row is None:
                continue

            tex = obj.texValues(fpos, wref)
            labels = aims.vector_STRING()
            textype = 'no_type'
            obj.getTextureLabels(tex, labels, textype)
            no, vertex, dist = obj.nearestVertex(fpos)
            if len(tex) == 0:
                tex = ''
            elif len(tex) == 1:
                tex = tex[0]
            item = self.val_table.item(row, 2)
            if item is None:
                item = Qt.QTableWidgetItem()
                self.val_table.setItem(row, 2, item)
            item.setText(str(tex))
            if labels:
                item = self.val_table.item(row, 3)
                if item is None:
                    item = Qt.QTableWidgetItem()
                    self.val_table.setItem(row, 3, item)
                if len(labels) == 1:
                    labels = labels[0]
                item.setText(str(labels))
            if no is not None:
                item = self.val_table.item(row, 1)
                if item is None:
                    item = Qt.QTableWidgetItem()
                    self.val_table.setItem(row, 1, item)
                item.setText(str(vertex))

        self.paintRefLabel()

    def closeAction(self, dummy):
        self.close()

    def get_object_label(self, obj):
        return obj.name()

    def update(self, observable, arg):
        self.Refresh()

    def setPosition(self, pos, ref=None):
        super().setPosition(pos, ref)
        self.update_position_ui()

    def update_position_ui(self):
        coords = ('X', 'Y', 'Z', 'T', 'X5', 'X6', 'X7', 'X8')
        mpos = self.getFullPosition()
        for i in range(len(self.coord_items), len(mpos)):
            ctext = coords[i]
            label = Qt.QLabel(ctext)
            self.coord_lay.addWidget(label, i, 0)
            ledit = Qt.QLineEdit()
            self.coord_lay.addWidget(ledit, i, 1)
            self.coord_items.append((label, ledit))
            ledit.setValidator(Qt.QDoubleValidator(ledit))
            ledit.editingFinished.connect(self.coords_changed)
        for i, x in enumerate(mpos):
            self.coord_items[i][1].setText(str(x))

    def changeReferential(self):
        sw = ana.cpp.set_AWindowPtr([self])
        w = ana.cpp.ChooseReferentialWindow(
            sw, [], 'Choose Referential Window')
        w.setParent(self)
        w.setWindowFlags(QtCore.Qt.Window)
        w.show()
        self.update_position_ui()

    def paintRefLabel(self):
        ref = self.getReferential()
        if ref is not None and ref.isDirect():
            col = ref.Color()
            pix = QtGui.QPixmap(32, 7)
            pix.fill(QtGui.QColor(col.red(), col.green(), col.blue()))
            p = QtGui.QPainter()
            darken = 25
            p.begin(pix)
            red = col.red()
            if red > darken:
                red = col.red() - darken
            else:
                red += darken
            green = col.green()
            if green > darken:
                green = col.green() - darken
            else:
                green += darken
            blue = col.blue()
            if blue > darken:
                blue = col.blue() - darken
            else:
                blue += darken
            p.setPen(QtGui.QPen(QtGui.QColor(red, green, blue), 5))
            p.drawLine(3, 10, 25, -3)
            p.end()
            del p
            pal = QtGui.QPalette(QtGui.QColor(col.red(), col.green(),
                                              col.blue()))
            self._refbutton.setBackgroundRole(QtGui.QPalette.Window)
            # doesn't work... maybe due to styles forcing it ?
            pal.setBrush(self._refbutton.backgroundRole(), QtGui.QBrush(pix))
            self._refbutton.setPalette(pal)
            if self._reflabel is not None:
                self._reflabel.show()
        else:
            if self._reflabel is not None:
                self._reflabel.hide()
            if ref is not None:
                col = ref.Color()
                self._refbutton.setPalette(QtGui.QPalette(
                    QtGui.QColor(col.red(), col.green(), col.blue())))
            else:
                self._refbutton.setPalette(QtGui.QPalette(
                                           QtGui.QColor(192, 192, 192)))

    def coords_changed(self):
        pos = []
        for item in self.coord_items:
            ledit = item[1]
            c = float(ledit.text())
            pos.append(c)
        a = ana.Anatomist()
        a.execute('LinkedCursor', window=self, position=pos)


class InfoWindowModule(ana.cpp.Module):

    def name(self):
        return 'Info window module'

    def description(self):
        return __doc__


class createInfoWindow(ana.cpp.AWindowCreator):

    def __call__(self, dock, options):
        h = InfoWindow()
        h.releaseref()
        h.show()
        return h


createinfowin = createInfoWindow()


def init():
    InfoWindow._classType \
        = InfoWindow.Type(ana.cpp.AWindowFactory.registerType(
            'Info', createinfowin, True))
    ana.cpp.QAWindowFactory.loadDefaultPixmaps('info')
    ana.cpp.AWindowFactory.setHasControlWindowButton(
        ana.cpp.AWindowFactory.typeID('Info'), False)


def clean_wincreator():
    ana.cpp.AWindowFactory.unregisterType('Info')
    global createinfowin
    createinfowin = None


hm = InfoWindowModule()
init()
atexit.register(clean_wincreator)
