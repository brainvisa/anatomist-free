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

# a test example for python plugin in Anatomist (Pyanatomist module)

"""Helpers / callback providing a new AObject menu entrie to show or hide the
   palettes related to the selected objects in selected windows.
"""

import sys, os, weakref
import anatomist.direct.api as ana
import anatomist.cpp as anatomist
from soma import aims

an = anatomist.Anatomist()
processor = an.theProcessor()

from soma.qt_gui.qt_backend import init_matplotlib_backend
init_matplotlib_backend()

from soma.qt_gui.qt_backend import QtCore
from soma.qt_gui.qt_backend import QtGui as qt
# copy needed classes to fake qt (yes, it's a horrible hack)
qt.QPoint = QtCore.QPoint
qt.QSize = QtCore.QSize
qt.QObject = QtCore.QObject
qt.SIGNAL = QtCore.SIGNAL
qt.PYSIGNAL = QtCore.SIGNAL
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg \
  as FigureCanvas
from matplotlib.figure import Figure


def setObjectId(object):
  import sip
  context = anatomist.CommandContext().defaultContext()
  context.mutex().lock()
  un = context.unserial.get()
  try:
    un.id(object)
  except RuntimeError:
    un.makeID(object)
  context.mutex().unlock()


def getObjectId(object):
  import sip
  context = anatomist.CommandContext().defaultContext()
  context.mutex().lock()
  un = context.unserial.get()
  try:
    id = un.id(object)
    return id
  except RuntimeError:
    return None


def convertVolumeToArray(img):
  import numpy
  def AimsRGBtoTuple(color):
    return (color.red(), color.green(), color.blue(), 255)
  l = [AimsRGBtoTuple(img.value(i)) for i in range(img.getSizeX())]
  return numpy.array(l, dtype='i')


class MplCanvas(FigureCanvas):
  def __init__(self, parent=None, name='mplCanvas',
               width=1, height=4, dpi=100, facecolor=None):
    from matplotlib import pylab
    self.fig = pylab.figure()
    self.number = self.fig.number
    FigureCanvas.__init__(self, self.fig)
    FigureCanvas.setSizePolicy(self,
      qt.QSizePolicy.Preferred,
      qt.QSizePolicy.Preferred)
    FigureCanvas.updateGeometry(self)
    self.setObjectName(name)
    self.setParent(parent)

  def sizeHint(self):
    w, h = self.get_width_height()
    w = 100
    return qt.QSize(w, h)

  def minimumSizeHint(self):
    return qt.QSize(1, 4)

class MoveAObjectFromAWindowEventHandler(anatomist.EventHandler):
  def __init__(self, groupwidget):
    anatomist.EventHandler.__init__(self)
    self._groupwidget = weakref.proxy( groupwidget )
    context = anatomist.CommandContext().defaultContext()
    outputEventContext = context.evfilter
    context.evfilter.setDefaultIsFiltering(True)
    outputEventContext.filter('RemoveObject')
    self.registerHandler('RemoveObject', self)

  def doit(self, ev):
    obj = ev.contents()['_object']
    win = ev.contents()['_window']
    try:
      aobj = anatomist.AObject.fromObject(obj)
    except RuntimeError:
      # this generally means obj has been destroyed
      return
    # The two next lines above are a small hack based on current
    # anatomist event system for objects deletion. In fact, these
    # lines allow to differentiate simple cases from objects
    # deletion while window closing.
    # If win is under deletion, then dynamic cast from AWindow to
    # QAWindow is impossible because its vtable is already
    # destroyed, thus, one obtains a Null pointer and then None
    # in python.
    qawin = anatomist.QAWindow.fromObject(win)
    if qawin == None: return
    groupwidget = qawin.parent().children()[1]
    if groupwidget != self._groupwidget: return
    try:
      self._groupwidget.remove(getObjectId(aobj))
    except KeyError: pass

class PaletteWidgetObserver(anatomist.Observer):
  def __init__(self, obj, palwidget):
    anatomist.Observer.__init__(self)
    self._palwidget = weakref.proxy( palwidget )
    obj.addObserver(self)

  def update(self, observable, arg):
    self._palwidget.update_palette()

class PaletteWidget(MplCanvas):
  def __init__(self, obj, parent):
    MplCanvas.__init__(self, parent, width=1, height = 4,
                       facecolor=None)
    bgcolor = self._init_bgcolor(parent)
    self.fig.set_facecolor(bgcolor)
    self.fig.subplots_adjust(left=0.35, top=0.95, bottom=0.05)
    if isinstance( obj, anatomist.AObject ):
      self._obj = anatomist.weak_shared_ptr_AObject( obj )
    else:
      self._obj = obj
    setObjectId(obj)
    self._observer = PaletteWidgetObserver(obj, self)
    self._size = None
    self._aobjectPalette = None
    self.update_palette()

  def _init_bgcolor(self, widget):
    import numpy
    qtbg = widget.palette().color( qt.QPalette.Active, qt.QPalette.Window )
    return (numpy.array(qtbg.getRgb(), dtype='f') / 255.).tolist()

  def update_palette(self):
    if not self._obj.palette():
      return
    self._aobjectPalette = self._obj.getOrCreatePalette()
    img = self._aobjectPalette.refPalette().volume()
    self._size = img.getSizeX()
    palette = self._compute_palette(img)
    self._display(palette)

  def _compute_palette(self, img):
    import numpy, matplotlib
    crange = numpy.linspace(0., 1., self._size)
    def scaleComponent(colors, id):
      l = [(x, c, c) for x, c in zip(crange, colors[:, id])]
      a = numpy.array(l, dtype = 'f')
      a[:, 1:3] /= 255.
      return a

    colors = convertVolumeToArray(img).tolist()
    colors.reverse()
    colors = numpy.array(colors)
    comps = [scaleComponent(colors, id) for id in range(3)]
    colorname = ['red', 'green', 'blue']
    cdict = dict(zip(colorname, comps))
    return matplotlib.colors.LinearSegmentedColormap(\
      'my_colormap', cdict, self._size)

  def _savePalette(self):
    fig_fn = qt.QFileDialog.getSaveFileName(self,"Save Figure", 
                                            os.getenv('HOME'), 
                                            "Images (*.png)")
    #print 'fig_fn:', fig_fn
    if fig_fn is not None:
      self._figure.savefig(str(fig_fn))

  def _display(self, palette):
    import pylab, numpy
    figure = pylab.figure(self.number)
    range = numpy.linspace(0, 1, self._size)[:, numpy.newaxis]
    te = self._obj.glAPI().glTexExtrema(0)
    mi = te.minquant[0]
    ma = te.maxquant[0]
    pmin = self._aobjectPalette.min1()
    pmax = self._aobjectPalette.max1()
    rmin = mi + (ma - mi) * pmin
    rmax = mi + (ma - mi) * pmax
    if rmin == rmax: return
    pylab.imshow(range, aspect='auto', cmap=palette, \
      extent=(0, 1, rmin, rmax))
    pylab.xticks([], [])
    self.draw()
    self._figure = figure

  def close(self):
    self.fig.set_canvas(None)
    return MplCanvas.close(self)

cross_img_data = [ "8 8 9 1", "# c #000000", "g c #f1f1f1", "f c #f3f3f3",
  "e c #f4f4f4", "d c #f6f6f6", "c c #f8f8f8", "b c #f9f9f9",
  "a c #fbfbfb", ". c #fdfdfd", ".#....#.", "###aa###", "b######b",
  "cc####cc", "dd####dd", "e######e", "###ff###", "g#gggg#g"]


class ClosableWidget(qt.QWidget):
  def __init__(self, parent, name = ''):
    qt.QWidget.__init__(self, parent)
    self.setObjectName( name )
    self._child = None
    biglayout = qt.QVBoxLayout(self)
    biglayout.setObjectName( "biglayout" )
    biglayout.setMargin( 11 )
    biglayout.setSpacing( 6 )
    layout = qt.QHBoxLayout(None)
    layout.setObjectName( "layout1" )
    layout.setMargin( 0 )
    layout.setSpacing( 6 )
    spacer = qt.QSpacerItem(10,10, qt.QSizePolicy.Expanding,
                            qt.QSizePolicy.Minimum)
    closeButton = qt.QPushButton(self)
    closeButton.setObjectName( "closeButton" )
    saveButton = qt.QPushButton(self)
    saveButton.setObjectName( "saveButton" )
    saveButton.setText('Save')
    saveButton.setSizePolicy(qt.QSizePolicy.Fixed,
                    qt.QSizePolicy.Fixed)
    closeButton.setText('x')
    closeButton.setSizePolicy(qt.QSizePolicy.Fixed,
                    qt.QSizePolicy.Fixed)
    closeButton.setFixedHeight(12)
    closeButton.setFixedWidth(12)
    pix = qt.QPixmap(cross_img_data)

    closeButton.setIcon( qt.QIcon( pix ) )
    layout.addItem(spacer)
    layout.addWidget(saveButton)
    layout.addWidget(closeButton)
    biglayout.addLayout(layout)
    label = qt.QLabel(self)
    f = qt.QFont("Sans", 10)
    label.setFont(f)
    label.setFixedHeight(20)
    label.setSizePolicy(qt.QSizePolicy.Ignored,
                    qt.QSizePolicy.Fixed)
    subwidget = qt.QWidget(self)
    sublayout = qt.QHBoxLayout(subwidget)
    sublayout.setObjectName( "sublayout" )
    sublayout.setMargin( 0 )
    sublayout.setSpacing( 0 )
    biglayout.addWidget(subwidget)
    biglayout.addWidget(label)
    self._closeButton = closeButton
    self._saveButton = saveButton
    self._sublayout = sublayout
    self._label = label
    self.subwidget = subwidget

  def close( self ):
    self._child.close()
    return qt.QWidget.close( self )

  def savePalette(self,):
    self._child._savePalette()

  def setChild(self, child):
    self._child = child
    self._sublayout.addWidget(child)

  def setName(self, name):
    self._label.setText(name)

  def getChild(self):
    return self._child

  def id(self):
    return getObjectId(self._child._obj)


class ReparentManager(object):
  _list = []

  def addWidget(self, widget):
    ReparentManager._list.append(widget)

  def removeWidget(self, widget):
    ReparentManager._list.remove(widget)

  addWidget = classmethod(addWidget)
  removeWidget = classmethod(removeWidget)


class GroupClosableWidget(qt.QWidget):
  class Clicked(object):
    def __init__(self, group, id):
      self._id = id
      self._group = weakref.ref( group )

    def doit(self):
      self._group().emit( qt.PYSIGNAL('clicked(int)'), self._id )

  def __init__(self, parent, name = ''):
    qt.QWidget.__init__(self, parent)
    self._layout = qt.QHBoxLayout(self)
    self._slots = {}
    self._widgets = {}

  def close( self ):
    for id in self._widgets.keys():
      self.remove( id )
    return qt.QWidget.close( self )

  def add(self, widget, id):
    self._layout.addWidget(widget)
    slot = GroupClosableWidget.Clicked(self, id)
    self._slots[id] = slot
    self._widgets[id] = widget
    qt.QObject.connect(widget._closeButton, qt.SIGNAL("clicked()"),
                       slot.doit)
    qt.QObject.connect(widget._saveButton, qt.SIGNAL("clicked()"),
                       widget.savePalette)
    self.show()

  def get(self, id):
    return self._widgets[id]

  def remove(self, id):
    widget = self._widgets[id]
    del self._slots[id]
    del self._widgets[id]
    widget.close()
    if len(self._widgets) == 0: self.hide()

  def has_key(self, id):
    return self._widgets.has_key(id)


class GroupPaletteWidget(GroupClosableWidget):
  def __init__(self, parent, name=''):
    GroupClosableWidget.__init__(self, parent, name)
    self._connect()
    self._eventHandler = MoveAObjectFromAWindowEventHandler(self)

  def __del__( self ):
    anatomist.EventHandler.unregisterHandler( 'RemoveObject',
      self._eventHandler )
    #GroupClosableWidget.__del__( self )

  def _connect(self):
    qt.QObject.connect(self, qt.PYSIGNAL("clicked(int)"),
                       self.closeOnePalette)

  def newPalette(self, object):
    p = ClosableWidget(self)
    p.setName(object.name())
    p.setToolTip(object.name())
    pw = PaletteWidget(object, p.subwidget)
    p.setChild(pw)
    self.add(p, getObjectId( object ))

  def closeOnePalette(self, id):
    self.remove(id)


class topWidgetWindow(qt.QSplitter):
  def __init__(self, parent=None, name=''):
    qt.QSplitter.__init__(self, parent)
    ReparentManager.addWidget(self)

  def close(self):
    ReparentManager.removeWidget(self)
    self.children()[1].close()
    return qt.QSplitter.close(self)


class ShowHidePaletteCallback(anatomist.ObjectMenuCallback):
  def __init__(self):
    anatomist.ObjectMenuCallback.__init__(self)

  def doit(self, objects):
    windows = an.getControlWindow().selectedWindows()
    if len(windows) == 0: windows = an.getWindows()
    for w in windows:
      if w.type() in [anatomist.AWindow.WINDOW_2D,
                      anatomist.AWindow.WINDOW_3D]:
        owin = [o for o in objects if self._winDisplaysObj(w, o)]
        if owin != []: self._togglePalettes(w, owin)

  def _togglePalettes(self, window, objects):
    topwidget = self._getOrCreateTopWidget(window)
    groupwidget = topwidget.findChild( qt.QWidget, 'paletteviewer_group' )
    layout = groupwidget.layout()
    for o in objects:
      if not o.palette():
        continue # do nothing on objects with no palette
      id = getObjectId(o)
      if groupwidget.has_key(id):
        groupwidget.remove(id)
      else:	groupwidget.newPalette(o)
    topwidget.show()

  def _getOrCreateTopWidget(self, window):
    parent = window.parent()
    layout = None
    lay_pos = None
    if parent is not None:
      if parent.objectName() == 'paletteviewer_top':
        return parent
      layout = parent.layout()
      if layout:
        if isinstance(layout, qt.QGridLayout):
          nc = layout.columnCount()
          nr = layout.rowCount()
          for r in xrange(nr):
            for c in xrange(nc):
              if layout.itemAtPosition(r, c).widget() == window:
                lay_pos = (r, c)
                break
            if lay_pos is not None:
              break
        else: # std layout
          nitem = layout.count()
          for c in xrange(nitem):
            if layout.itemAt(c).widget() == window:
              lay_pos = (c, )
              break
    topwidget = topWidgetWindow(parent)
    topwidget.setObjectName('paletteviewer_top')
    window.setParent( topwidget )
    if parent and lay_pos is not None:
      if len(lay_pos) == 2:
        layout.addWidget(topwidget, lay_pos[0], lay_pos[1])
      else:
        if lay_pos[0] == 0:
          layout.addWidget(topwidget)
        else:
          layout.insertWidget(lay_pos[0], topwidget)
    topwidget.setWindowTitle(window.Title())
    groupwidget = GroupPaletteWidget(topwidget)
    groupwidget.setObjectName( 'paletteviewer_group' )
    return topwidget

  @staticmethod
  def _winDisplaysObj(win, obj):
    if win.hasObject(obj):
      return True
    parents = obj.parents().list()
    done = set()
    while parents:
      parent = parents.pop(0)
      if parent in done:
        continue
      done.add(parent)
      parents += [p for p in parent.parents() \
        if p not in parents and p not in done]
      if win.hasObject(parent): # and parent.palette() == obj.palette():
        return True
    return False


class PaletteViewerModule(anatomist.Module):
  def name(self):
    return 'Palette viewer module'

  def description(self):
    return __doc__

callbacks_list = []

def addMenuEntryToOptionMenu(menu):
  '''Add menu to optionMenu (new menu system API)'''
  import sip
  showHidePalette = ShowHidePaletteCallback()
  callbacks_list.append(showHidePalette)
  menu.insertItem(['Color'], 'show/hide palette', showHidePalette)

def addMenuEntryToOptionTree(object):
  '''Add menu to optionTree (old menu system API)'''
  import sip
  m = anatomist.ObjectMenu(object.optionTree())
  addMenuEntryToOptionMenu(m)
  t = m.releaseTree()
  sip.transferto(t, None)

def init():
  import sip
  '''Add here entry to optionTree for ShowHidePaletteCallback'''

  # New menu API based on optionMenu()
  menumap = anatomist.AObject.getObjectMenuMap()
  menus = {}
  # Add palette menu to all menus but only once
  for k, v in menumap.items(): menus[v] = k
  for m in menus.keys(): addMenuEntryToOptionMenu(m)


def toggleShowPaletteForObject(aobject):
  # show/hide palette
  if hasattr(aobject, 'getInternalRep'):
    aobject = aobject.getInternalRep()
  ShowHidePaletteCallback().doit(ana.cpp.set_AObjectPtr([aobject]))

def savePaletteImage(aobject, filename):
  # save palette figure
  if hasattr(aobject, 'getInternalRep'):
    aobject = aobject.getInternalRep()
  a = ana.Anatomist()
  wins = a.getWindows()
  for w in wins:
    gw = w.parent().findChild(GroupPaletteWidget)
    if gw:
      break
  else:
    return
  fig = gw.get(getObjectId(aobject)).findChild(PaletteWidget).figure
  fig.savefig(filename)


pm = PaletteViewerModule()
init()
