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
A Matplotlib-based profile window for Anatomist
'''

import anatomist.direct.api as ana
from soma import aims
import numpy, sys

from soma.qt_gui.qt_backend import init_matplotlib_backend
init_matplotlib_backend()

from matplotlib import pyplot
import pylab, sip

from soma.qt_gui.qt_backend import QtCore
from soma.qt_gui.qt_backend import QtGui


class AProfile( ana.cpp.QAWindow ):
  '''A Matplotlib-based profile window for Anatomist.
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
    self._fig = pyplot.figure()
    self.setAttribute( QtCore.Qt.WA_DeleteOnClose )
    self._fig.set_facecolor( str( self.palette().color( \
      QtGui.QPalette.Active, QtGui.QPalette.Window ).name() ) )
    wid = pyplot._pylab_helpers.Gcf.get_fig_manager(self._fig.number).window
    wid.setParent( self )
    self.setCentralWidget( wid )
    # keep a reference to the python object to prevent destruction of the
    # python part
    AProfile._instances.add( self )
    self.connect( self, QtCore.SIGNAL( 'destroyed()' ), self.destroyNotified )
    self._plots = {}
    self._orientation = aims.Quaternion( 0, 0, 0, 1 )
    self._cursorplot = None
    self._coordindex = 0 # x axis
    self._picker_installed = False
    # add toolbar
    toolbar = QtGui.QToolBar( wid )
    toolbar.addAction( 'X', self.muteOrientationX )
    toolbar.addAction( 'Y', self.muteOrientationY )
    toolbar.addAction( 'Z', self.muteOrientationZ )
    toolbar.addAction( 'T', self.muteOrientationT )
    wid.addToolBar( toolbar )
    # referential bar
    cw = wid.centralWidget()
    nw = QtGui.QWidget( wid )
    lay = QtGui.QVBoxLayout( nw )
    lay.setContentsMargins( 0, 0, 0, 0 )
    lay.setSpacing( 5 )
    nw.setLayout( lay )
    refbox = QtGui.QWidget( nw )
    lay.addWidget( refbox )
    cw.setParent( nw )
    lay.addWidget( cw )
    wid.setCentralWidget( nw )
    rlay = QtGui.QHBoxLayout( refbox )
    refbox.setLayout( rlay )
    rbut = QtGui.QPushButton( refbox )
    rlay.addWidget( rbut )
    icons = ana.cpp.IconDictionary.instance()
    directpix = icons.getIconInstance( 'direct_ref_mark' )
    refdirmark = QtGui.QLabel( refbox )
    if directpix is not None:
      refdirmark.setPixmap( directpix )
    rlay.addWidget( refdirmark )
    refdirmark.setFixedSize( QtCore.QSize( 21, 7 ) )
    rbut.setFixedHeight( 7 )
    refbox.setFixedHeight( refbox.sizeHint().height() )
    self._refbutton = rbut
    self._reflabel = refdirmark
    ana.cpp.anatomist.setQtColorStyle( rbut )
    self.paintRefLabel()
    self.connect( rbut, QtGui.SIGNAL( 'clicked()' ), self.changeReferential )
    # close shortcut
    ac = QtGui.QAction( 'Close', self )
    ac.setShortcut( QtCore.Qt.CTRL + QtCore.Qt.Key_W )
    self.connect( ac, QtGui.SIGNAL( 'triggered(bool)' ), self.closeAction )
    self.addAction( ac )


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
    #print 'AProfile.__del__'
    ana.cpp.QAWindow.__del__( self )

  def destroyNotified( self ):
    #print 'destroyNotified'
    # release internal reference which kept the python side of the object
    # alive - now the python object may be destroyed since the C++ side
    # will be also destroyed anyway.
    if self in AProfile._instances:
      AProfile._instances.remove( self )

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
    self.eraseObject( obj )
    ana.cpp.QAWindow.unregisterObject( self, obj )

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
      self._fig.canvas.draw()

  def plotObject( self, obj ):
    if obj.objectTypeName( obj.type() ) == 'VOLUME':
      if self._coordindex == 3:
        return self.plotTprofile( obj )
      figure = pyplot.figure( self._fig.number )
      vol = ana.cpp.AObjectConverter.aims( obj ).volume()
      ar = numpy.array( vol, copy=False )
      pos = self.getPosition()
      tpos = self.getTime()
      
      if len( vol.header()[ 'voxel_size' ] ) >= 4:
        tpos /= vol.header()[ 'voxel_size' ][3]
      if tpos < 0:
        tpos = 0
      elif tpos >= vol.getSizeT():
        tpos = vol.getSizeT() - 1
      opos = pos
      oref = obj.getReferential()
      wref = self.getReferential()
      a = ana.Anatomist()
      trans = a.getTransformation( wref, oref )
      uvect = self._orientation.transform( aims.Point3df( 1, 0, 0 ) )
      
      if trans is not None:
        # get in object space
        uvect = trans.transform( uvect ) - trans.transform( [ 0, 0, 0 ] )
        uvect.normalize()
        opos = trans.transform( pos )
      vs = vol.header()[ 'voxel_size' ][:3]
      dims = aims.Point3df( ( vol.getSizeX() - 1 ) * vs[0],
                            ( vol.getSizeY() - 1 ) * vs[1], 
                            ( vol.getSizeZ() - 1 ) * vs[2] )
      
      # get intersects of opos + x * uvect with bounding planes
      # (in object space)
      bounds = []
      # x plane, v=(1,0,0)
      bmin = aims.Point3df( -0.5 * vs[0], -0.5 * vs[1], -0.5 * vs[2] )
      bmax = aims.Point3df( dims[0] + vs[0] * 0.5, 
                            dims[1] + vs[1] * 0.5, 
                            dims[2] + vs[2] * 0.5 )
      v = aims.Point3df( 1, 0, 0 )
      uv = uvect.dot( v )
      if uv != 0:
        t0 = (aims.Point3df(0, 0, 0) - opos).dot( v ) / uv
        t1 = (dims - opos).dot( v ) / uv
        if t1 < t0:
          t0, t1 = t1, t0 # ensure t0 is the min
        bounds = [ t0, t1 ]
      else:
        # parallel to x plane: ensure opos is between both planes
        if (bmin - opos).dot( v ) * (bmax - opos).dot( v ) > 0:
          self.eraseObject( obj )
          return
      # y plane, v=(0,1,0)
      v = aims.Point3df( 0, 1, 0 )
      uv = uvect.dot( v )
      if uv != 0:
        t0 = (aims.Point3df( 0, 0, 0 ) - opos).dot( v ) / uv
        t1 = (dims - opos).dot( v ) / uv
        if t1 < t0:
          t0, t1 = t1, t0 # ensure t0 is the min
        if len( bounds ) == 0:
          bounds = [ t0, t1 ]
        else: # take minimum interval
          if t0 > bounds[1] or t1 < bounds[0]:
            self.eraseObject( obj )
            return
          if bounds[0] < t0:
            bounds[0] = t0
          if bounds[1] > t1:
            bounds[1] = t1
      else:
        # parallel to y plane: ensure opos is between both planes
        if (bmin - opos).dot( v ) * (bmax - opos).dot( v ) > 0:
          self.eraseObject( obj )
          return
      # z plane, v=(0,0,1)
      v = aims.Point3df( 0, 0, 1 )
      uv = uvect.dot( v )
      if uv != 0:
        t0 = (aims.Point3df( 0, 0, 0 ) - opos).dot( v ) / uv
        t1 = (dims - opos).dot( v ) / uv
        if t1 < t0:
          t0, t1 = t1, t0 # ensure t0 is the min
        if len( bounds ) == 0:
          bounds = [ t0, t1 ]
        else: # take minimum interval
          if t0 > bounds[1] or t1 < bounds[0]:
            self.eraseObject( obj )
            return
          if bounds[0] < t0:
            bounds[0] = t0
          if bounds[1] > t1:
            bounds[1] = t1
      else:
        # parallel to z plane: ensure opos is between both planes
        if (bmin - opos).dot( v ) * (bmax - opos).dot( v ) > 0:
          self.eraseObject( obj )
          return

      t0, t1 = bounds
      step = min( vs ) # not optimal.
      indices = [ opos+uvect*(t0+x*step) for x in \
        xrange( 0, int( (t1 - t0 + 1)/step ) ) ]

      besti = numpy.argmax( numpy.abs( self._orientation.transform( [ 1, 0, 0 ] ) ) )
      self._coordindex = besti
      avs = numpy.array(vs)
      aind = numpy.hstack( [ numpy.round( numpy.hstack( ( \
        ( numpy.array(x)/avs ), tpos ) ) ).astype( int ).reshape(4,1) \
        for x in indices ] )
      # clamp any index which may be out of bounds after rounding
      aind[ aind<0 ] = 0
      sz = numpy.reshape( ar.shape, (4,1) )
      clamped = numpy.where( aind >= sz )
      aind[ clamped ] = (sz-1)[clamped[0]].ravel()
      data = ar[ tuple( aind ) ]
      if trans is None:
        xdata = [ x[self._coordindex] for x in indices ]
      else:
        # display coords in window space
        trinv = trans.motion().inverse()
        trind = [ trinv.transform( x ) for x in indices ]
        xdata = [ x[self._coordindex] for x in trind ]
      kw = {}
      p = self._plots.get( ana.cpp.weak_ptr_AObject( obj ) )
      if p:
        kw['color'] = p[0].get_color()
        for x in p:
          x.remove()
        del self._plots[ ana.cpp.weak_ptr_AObject( obj ) ]
      h = pylab.plot( xdata, data, label=obj.name(), **kw )
      pylab.legend()
      self._fig.canvas.draw()
      self._plots[ ana.cpp.weak_ptr_AObject( obj ) ] = h

  def plotTprofile( self, obj ):
    figure = pyplot.figure( self._fig.number )
    vol = ana.cpp.AObjectConverter.aims( obj ).volume()
    ar = numpy.array( vol, copy=False )
    pos = self.getPosition()
    tpos = self.getTime()
    opos = pos
    oref = obj.getReferential()
    wref = self.getReferential()
    a = ana.Anatomist()
    trans = a.getTransformation( wref, oref )
    if trans:
      opos = trans.transform( pos )
    vs = vol.header()[ 'voxel_size' ]
    if len( vs ) >= 4:
      ts = vs[3]
    else:
      ts = 1.
    xdata = [ x * ts for x in xrange( vol.getSizeT() ) ]
    ipos = numpy.array( opos ) / vs[:3]
    if ipos[0] < 0 or ipos[1] < 0 or ipos[2] < 0 or ipos[0] >= vol.getSizeX() \
      or ipos[1] >= vol.getSizeY() or ipos[2] >= vol.getSizeZ():
      self.eraseObject( obj )
      return
    data = ar[ ipos[0], ipos[1], ipos[2], : ]
    kw = {}
    p = self._plots.get( ana.cpp.weak_ptr_AObject( obj ) )
    if p:
      kw['color'] = p[0].get_color()
      for x in p:
        x.remove()
      del self._plots[ ana.cpp.weak_ptr_AObject( obj ) ]
    h = pylab.plot( xdata, data, label=obj.name(), **kw )
    pylab.legend()
    self._fig.canvas.draw()
    self._plots[ ana.cpp.weak_ptr_AObject( obj ) ] = h

  def baseTitle( self ):
    return 'Profile'

  def Refresh( self ):
    ana.cpp.QAWindow.Refresh( self )
    for obj in self.Objects():
      self.plotObject( obj )
    self.drawCursor()
    self._fig.canvas.draw() # refresh plot with updated cursor

    # pick events
    if not self._picker_installed and len( self._fig.axes ) > 0:
      self._fig.axes[0].set_picker( True )
      self._fig.canvas.mpl_connect( 'pick_event', self.onPick )
      self._picker_installed = True

    self.paintRefLabel()

  def drawCursor( self ):
    pos = self.getPosition()[:3] + [ self.getTime() ]
    if self._cursorplot:
      for x in self._cursorplot:
        x.remove()
      del self._cursorplot
    if len( self._fig.axes ) == 0:
      return
    ax = self._fig.axes[0]
    if not ax:
      self._cursorplot = None
      return
    ax.grid( True )
    ax.xaxis.set_label_text( [ 'X (mm)', 'Y (mm)', 'Z (mm)',
      'T (s)' ][self._coordindex] )
    if not hasattr( ax, 'get_ylim' ):
      self._cursorplot = None
      return
    self._cursorplot = ax.plot( [pos[self._coordindex],
      pos[self._coordindex]], [ ax.get_ylim()[0], ax.get_ylim()[1] ], 'r' )

  def muteOrientationX( self ):
    self._orientation = aims.Quaternion( 0, 0, 0, 1 )
    self._coordindex = 0
    self.Refresh()

  def muteOrientationY( self ):
    self._orientation = aims.Quaternion( 0.5, 0.5, 0.5, 0.5 )
    self._coordindex = 1
    self.Refresh()

  def muteOrientationZ( self ):
    self._orientation = aims.Quaternion( 1, 0, 1, 0 ).normalized()
    self._coordindex = 2
    self.Refresh()

  def muteOrientationT( self ):
    self._coordindex = 3
    self.Refresh()

  def onPick( self, event ):
    a = ana.Anatomist()
    pos = list( self.getPosition() )
    pos.append( self.getTime() )
    if pos[ self._coordindex ] == event.mouseevent.xdata:
      return
    pos[ self._coordindex ] = event.mouseevent.xdata
    a.execute( 'LinkedCursor', window=self, position=pos )

  def paintRefLabel( self ):
    ref = self.getReferential()
    if ref is not None and ref.isDirect():
      col = ref.Color()
      pix = QtGui.QPixmap( 32,7 )
      pix.fill( QtGui.QColor( col.red(), col.green(), col.blue() ) )
      p = QtGui.QPainter()
      darken = 25;
      p.begin( pix )
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
      p.setPen( QtGui.QPen( QtGui.QColor( red, green, blue ), 5 ) )
      p.drawLine(3, 10, 25, -3)
      p.end()
      del p
      pal = QtGui.QPalette( QtGui.QColor( col.red(), col.green(),
        col.blue() ) )
      self._refbutton.setBackgroundRole( QtGui.QPalette.Window )
      # doesn't work... maybe due to styles forcing it ?
      pal.setBrush( self._refbutton.backgroundRole(), QtGui.QBrush( pix ) )
      self._refbutton.setPalette( pal )
      if self._reflabel is not None:
        self._reflabel.show()
    else:
      if self._reflabel is not None:
        self._reflabel.hide()
      if ref is not None:
        col = ref.Color()
        self._refbutton.setPalette( QtGui.QPalette( \
            QtGui.QColor( col.red(), col.green(), col.blue() ) ) )
      else:
        self._refbutton.setPalette( QtGui.QPalette( \
          QtGui.QColor(192, 192, 192) ) )

  def changeReferential( self ):
    sw = ana.cpp.set_AWindowPtr( [ self ] )
    w = ana.cpp.ChooseReferentialWindow( sw, 'Choose Referential Window' )
    w.setParent( self )
    w.setWindowFlags( QtCore.Qt.Window )
    w.show()

  def closeAction( self, dummy ):
    self.close()



class ProfileModule( ana.cpp.Module ):
  def name(self):
    return 'Profile window module'

  def description(self):
    return __doc__

class createProfileWindow( ana.cpp.AWindowCreator ):
  def __call__( self, dock, options ):
    h = AProfile()
    h.releaseref()
    h.show()
    return h

createprofile = createProfileWindow()

def init():
  AProfile._classType = AProfile.Type( ana.cpp.AWindowFactory.registerType( \
    'Matplotlib-profile', createprofile, True ) )
  ana.cpp.QAWindowFactory.loadDefaultPixmaps( 'Matplotlib-profile' )
  ana.cpp.AWindowFactory.setHasControlWindowButton( \
    ana.cpp.AWindowFactory.typeID( 'Profile' ), False )


hm = ProfileModule()
init()

