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
A Matplotlib-based histogram window for Anatomist
'''

import anatomist.direct.api as ana
from soma import aims
import numpy, sys

from soma.gui.api import chooseMatplotlibBackend
chooseMatplotlibBackend()

from matplotlib import pyplot
import pylab, sip

from PyQt4 import QtCore
from PyQt4 import QtGui
#qt = GtGui
## copy needed classes to fake qt (yes, it's a horrible hack)
#qt.QPoint = QtCore.QPoint
#qt.QSize = QtCore.QSize
#qt.QObject = QtCore.QObject
#qt.SIGNAL = QtCore.SIGNAL
#qt.PYSIGNAL = QtCore.SIGNAL
#Qt = QtCore.Qt


class AHistogram( ana.cpp.QAWindow ):
  '''A Matplotlib-based histogram window for Anatomist.
  It is designed in python, and python-inherited classes suffer from reference-
  counting problems. See the doc of the releaseref() method.
  '''
  _instances = set()
  _classType = ana.cpp.AWindow.Type( 0 )

  def __init__( self, parent=None, name=None, options=aims.Object(), f=None ):
    '''The releaseref() method should be called after the constructor - see
    the doc of this method.
    It is not called from the constructor for technical anatomist IDs problems
    (which may be solved).
    '''
    if f is None:
      f = QtCore.Qt.WindowFlags( QtCore.Qt.Window )
    ana.cpp.QAWindow.__init__( self, parent, name, options, f )
    self._histo = pyplot.figure()
    self.setAttribute( QtCore.Qt.WA_DeleteOnClose )
    self._histo.set_facecolor( str( self.palette().color( \
      QtGui.QPalette.Active, QtGui.QPalette.Window ).name() ) )
    wid = pyplot._pylab_helpers.Gcf.get_fig_manager(self._histo.number).window
    wid.setParent( self )
    self.setCentralWidget( wid )
    # keep a reference to the python object to prevent destruction of the
    # python part
    AHistogram._instances.add( self )
    self.connect( self, QtCore.SIGNAL( 'destroyed()' ), self.destroyNotified )
    self._plots = {}
    self._histo4d = True
    self._localHisto = False
    self._localSize = 20
    # additional toolbar
    toolbar = QtGui.QToolBar( wid )
    ac = QtGui.QAction( '3D', toolbar )
    ac.setCheckable( True )
    self.connect( ac, QtCore.SIGNAL( 'triggered(bool)' ), self.set3DHisto )
    toolbar.addAction( ac )
    ac = QtGui.QAction( 'Local', toolbar )
    ac.setCheckable( True )
    self.connect( ac, QtCore.SIGNAL( 'triggered(bool)' ), self.setLocalHisto )
    toolbar.addAction( ac )
    toolbar.addAction( 'Neighborhood...', self.setHistoNeighborhood )
    wid.addToolBar( toolbar )

  def releaseref( self ):
    '''WARNING:
    the instance in _instances shouldn't count on C++ side
    PROBLEM: all python refs are one unique ref for C++,
    all being of the same type, so later references will not be strong refs.
    the less annoying workaround at the moment is that python refs are
    'weak shared references': count as references to keep the object alive,
    but don't actually prevent its destruction whenever the close method
    or anatomist destroy command are called. In such case the python object
    will hold a deleted C++ object.
    This way, only C++ may destroy the object.
    When the C++ instance is destroyed, the QObject destroyed callback is
    used to cleanup the additional python reference in AHistogram._instances
    so that the python instance can also be destroyed when python doesn't
    use it any longer.
    That's the best I can do for now...
    This releaseref method should be called after the constructor: it is
    called from the createHistogramWindow factory class.
    this means you should _not_ create an instance of AHistogram directly.'''
    a = ana.Anatomist()
    a.execute( 'ExternalReference', elements=[self],
      action_type='TakeWeakSharedRef' )
    a.execute( 'ExternalReference', elements=[self],
      action_type='ReleaseStrongRef' )

  def __del__( self ):
    #print 'AHistogram.__del__'
    ana.cpp.QAWindow.__del__( self )

  def destroyNotified( self ):
    #print 'destroyNotified'
    # release internal reference which kept the python side of the object
    # alive - now the python object may be destroyed since the C++ side
    # will be also destroyed anyway.
    if self in AHistogram._instances:
      AHistogram._instances.remove( self )

  def type( self ):
    return self._classType;

  def registerObject( self, obj, temporaryObject=False, position=-1 ):
    if hasattr( obj, 'internalRep' ):
      obj = obj.internalRep
    if not self.hasObject( obj ):
      ana.cpp.QAWindow.registerObject( self, obj, temporaryObject, position )
      self.plotObject( obj )

  def unregisterObject( self, obj ):
    if hasattr( obj, 'internalRep' ):
      obj = obj.internalRep
    ana.cpp.QAWindow.unregisterObject( self, obj )
    p =  self._plots.get( ana.cpp.weak_ptr_AObject( obj ) )
    if p:
      for x in p:
        x.remove()
      del self._plots[ ana.cpp.weak_ptr_AObject( obj ) ]
      if len( self._plots ) >= 0:
        pylab.legend()
      self._histo.canvas.draw()

  def eraseObject( self, obj ):
    if hasattr( obj, 'internalRep' ):
      obj = obj.internalRep
    p =  self._plots.get( ana.cpp.weak_ptr_AObject( obj ) )
    if p:
      for x in p:
        x.remove()
      del self._plots[ ana.cpp.weak_ptr_AObject( obj ) ]
      if len( self._plots ) >= 0:
        pylab.legend()
      self._histo.canvas.draw()

  def plotObject( self, obj ):
    if obj.objectTypeName( obj.type() ) == 'VOLUME':
      figure = pyplot.figure( self._histo.number )
      kw = {}
      p = self._plots.get( ana.cpp.weak_ptr_AObject( obj ) )
      if p:
        col = p[0].get_facecolor()
        kw['facecolor'] = col
        kw[ 'color'] = col
        col = p[0].get_edgecolor()
        kw['edgecolor'] = col
        self.eraseObject( obj )
      vol = ana.cpp.AObjectConverter.aims( obj ).volume()
      ar = numpy.array( vol, copy=False )
      if not self._histo4d:
        ar = ar[ :, :, :, self.GetTime() ]
      if self._localHisto:
        pos = self.GetPosition()
        oref = obj.getReferential()
        wref = self.getReferential()
        a = ana.Anatomist()
        trans = a.getTransformation( wref, oref )
        if trans is not None:
          pos = trans.transform( pos )
        vs = vol.header()[ 'voxel_size' ]
        ipos = numpy.round( numpy.array( pos ) / vs[:3] ).astype( int )
        ipos0 = ipos - self._localSize / numpy.array( vs[:3] )
        ipos1 = ipos + self._localSize / numpy.array( vs[:3] )
        ipos0[ numpy.where( ipos0 < 0 ) ] = 0
        if ipos1[0] >= vol.getSizeX():
          ipos1[0] = vol.getSizeX() - 1
        if ipos1[1] >= vol.getSizeY():
          ipos1[1] = vol.getSizeY() - 1
        if ipos1[2] >= vol.getSizeZ():
          ipos1[2] = vol.getSizeZ() - 1
        ar = ar[ ipos0[0]:ipos1[0], ipos0[1]:ipos1[1], ipos0[2]:ipos1[2] ]
      h = pylab.hist( numpy.ravel( ar ), 256, label=obj.name(), **kw )
      pylab.legend()
      self._histo.canvas.draw()
      self._plots[ ana.cpp.weak_ptr_AObject( obj ) ] = h[2]

  def baseTitle( self ):
    return 'Histogram'

  def Refresh( self ):
    ana.cpp.QAWindow.Refresh( self )
    for obj in self.Objects():
      self.plotObject( obj )
    if len( self._histo.axes ) != 0:
      ax = self._histo.axes[0]
      ax.relim()
      ax.autoscale_view()

  def set3DHisto( self, is3d ):
    self._histo4d = not is3d
    self.Refresh()

  def setLocalHisto( self, islocal ):
    self._localHisto = islocal
    self.Refresh()

  def setHistoNeighborhood( self ):
    dia = QtGui.QDialog( self )
    dia.setWindowTitle( 'Neighborhood half-size (mm)' )
    l = QtGui.QVBoxLayout( dia )
    dia.setLayout( l )
    le = QtGui.QLineEdit( dia )
    v = QtGui.QIntValidator( le )
    v.setBottom( 0 )
    le.setValidator( v )
    le.setText( str( self._localSize ) )
    le.selectAll()
    l.addWidget( le )
    b = QtGui.QWidget( dia )
    l.addWidget( b )
    l2 = QtGui.QHBoxLayout( b )
    b.setLayout( l2 )
    ok = QtGui.QPushButton( 'OK', b )
    cancel = QtGui.QPushButton( 'Cancel', b )
    l2.addWidget( ok )
    l2.addWidget( cancel )
    dia.connect( ok, QtGui.SIGNAL( 'pressed()' ), dia.accept )
    dia.connect( cancel, QtGui.SIGNAL( 'pressed()' ), dia.reject )
    res = dia.exec_()
    if res:
      val = int( le.text() )
      self._localSize = val
      self.Refresh()


class HistogramModule( ana.cpp.Module ):
  def name(self):
    return 'Histogram window module'

  def description(self):
    return __doc__

class createHistogramWindow( ana.cpp.AWindowCreator ):
  def __call__( self, dock, options ):
    h = AHistogram()
    h.releaseref()
    h.show()
    return h

createhisto = createHistogramWindow()

def init():
  AHistogram._classType = AHistogram.Type( ana.cpp.AWindowFactory.registerType\
    ( 'Matplotlib-histogram', createhisto ) )
  ana.cpp.QAWindowFactory.loadDefaultPixmaps( 'Matplotlib-histogram' )


hm = HistogramModule()
init()

