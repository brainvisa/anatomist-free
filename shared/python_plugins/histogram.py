

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

if sys.modules.has_key( 'PyQt4' ):
  qt4 = True
  from PyQt4 import QtCore
  from PyQt4 import QtGui as qt
  # copy needed classes to fake qt (yes, it's a horrible hack)
  qt.QPoint = QtCore.QPoint
  qt.QSize = QtCore.QSize
  qt.QObject = QtCore.QObject
  qt.QString = QtCore.QString
  qt.SIGNAL = QtCore.SIGNAL
  qt.PYSIGNAL = QtCore.SIGNAL
  Qt = QtCore.Qt
else:
  qt4 = False
  import qt, qtui
  Qt = qt.Qt


class AHistogram( ana.cpp.QAWindow ):
  '''A Matplotlib-based histogram window for Anatomist.
  It is designed in python, and python-inherited classes suffer from reference-
  counting problems. See the doc of the releaseref() method.
  '''
  _instances = set()
  _classType = ana.cpp.AWindow.Type( ana.cpp.AWindowFactory.types().size() )

  def __init__( self, parent=None, name=None, options=aims.Object(), f=None ):
    '''The releaseref() method should be called after the constructor - see
    the doc of this method.
    It is not called from the constructor for technical anatomist IDs problems
    (which may be solved).
    '''
    if f is None:
      if qt4:
        f = Qt.WindowFlags( Qt.Window )
      else:
        f = Qt.WType_TopLevel | Qt.WDestructiveClose
    ana.cpp.QAWindow.__init__( self, parent, name, options, f )
    self._histo = pyplot.figure()
    if qt4:
      self.setAttribute( Qt.WA_DeleteOnClose )
      self._histo.set_facecolor( str( self.palette().color( \
        qt.QPalette.Active, qt.QPalette.Window ).name() ) )
    else:
      self._histo.set_facecolor( str( self.palette().color( \
        qt.QPalette.Active, qt.QColorGroup.Background ).name() ) )
    wid = pyplot._pylab_helpers.Gcf.get_fig_manager(self._histo.number).window
    if qt4:
      wid.setParent( self )
    else:
      wid.reparent( self, qt.QPoint(0, 0) )
    self.setCentralWidget( wid )
    # keep a reference to the python object to prevent destruction of the
    # python part
    AHistogram._instances.add( self )
    self.connect( self, qt.SIGNAL( 'destroyed()' ), self.destroyNotified )
    self._plots = {}

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
    print 'AHistogram.__del__'
    if not qt4:
      try:
        # try to cleanup things
        pyplot.close( self._histo )
      except RuntimeError:
        pass
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
      if obj.objectTypeName( obj.type() ) == 'VOLUME':
        figure = pyplot.figure( self._histo.number )
        h = pylab.hist( numpy.array( ana.cpp.AObjectConverter.aims( \
          obj ).volume(), copy=False ), 256, label=obj.name() )
        pylab.legend()
        self._histo.canvas.draw()
        self._plots[ ana.cpp.weak_ptr_AObject( obj ) ] = h[2]

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

  def baseTitle( self ):
    return 'Histogram'

  #if qt4:
    #def close(self):
      #print 'AHistogram.close'
      #x = ana.cpp.QAWindow.close( self )
      #if x:
        #pyplot.close( self._histo )
        ##self._histo.set_canvas(None)
      #return x
  if not qt4:
    def close(self, alsoDelete=True):
      if alsoDelete and self.testDeletable():
        pyplot.close( self._histo )
      return ana.cpp.QAWindow.close( self, True )


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
  ana.cpp.AWindowFactory.registerType( 'Matplotlib-histogram',
    createhisto )


hm = HistogramModule()
init()

