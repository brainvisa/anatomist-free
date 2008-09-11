
import anatomist.direct.api as anatomist
from soma import aims
import qt, sys, math
sys.path.insert( 0, '.' )
import sphere

class MyAction( anatomist.cpp.Action ):
  def name( self ):
    return 'MyAction'

  def resetRadius( self ):
    print 'reset radius to 1'
    s.setRadius( 1. )

  def startMoveRadius( self, x, y, globx, globy ):
    print 'start move radius', x, y
    self._initial = ( x, y )
    self._radius = s.radius()

  def endMoveRadius( self, x, y, globx, globy ):
    print 'end move radius', x, y

  def moveRadius( self, x, y, globx, globy ):
    print 'move radius', x, y
    s.setRadius( math.exp( 0.01 * ( self._initial[1] - y ) ) * self._radius )

class MyControl( anatomist.cpp.Control ):
  def __init__( self, prio = 25 ):
    anatomist.cpp.Control.__init__( self, prio, 'MyControl' )

  def eventAutoSubscription( self, pool ):
    self.keyPressEventSubscribe( qt.Qt.Key_Escape, qt.Qt.NoButton, 
                                 pool.action( 'MyAction' ).resetRadius )
    self.mouseLongEventSubscribe( \
      qt.Qt.LeftButton, qt.Qt.NoButton,
      pool.action( 'MyAction' ).startMoveRadius,
      pool.action( 'MyAction' ).moveRadius,
      pool.action( 'MyAction' ).endMoveRadius,
      False )


a = anatomist.Anatomist()

pix = qt.QPixmap( 'control.xpm' )
anatomist.cpp.IconDictionary.instance().addIcon( 'MyControl',
  pix )
ad = anatomist.cpp.ActionDictionary.instance()
ad.addAction( 'MyAction', lambda: MyAction() )
cd = anatomist.cpp.ControlDictionary.instance()
cd.addControl( 'MyControl', lambda: MyControl(), 25 )
cm = anatomist.cpp.ControlManager.instance()
cm.addControl( 'QAGLWidget3D', '', 'MyControl' )

s = sphere.ASphere()
a.registerObject( s )
aw = a.createWindow( '3D' )
a.addObjects( [ s ], [ aw ] )

