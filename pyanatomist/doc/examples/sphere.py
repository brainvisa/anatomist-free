
import anatomist.direct.api as anatomist
from soma import aims
import types

class ASphere( anatomist.cpp.ASurface_3 ):
  def __init__( self, mesh = None ):
    self._center = aims.Point3df( 0, 0, 0 )
    self._radius = 100
    if mesh is not None:
      if type( mesh ) is types.StringType:
        # mesh is a filename: read it
        anatomist.cpp.ASurface_3.__init__( self, mesh )
        r = aims.Reader()
        m = aims.rc_ptr_AimsTimeSurface_3( r.read( mesh ) )
        self.setSurface( m )
      else:
        # mesh should be an Aims mesh: assign it
        anatomist.cpp.ASurface_3.__init__( self )
        self.setSurface( mesh )
    else:
      # generate a sphere mesh
      anatomist.cpp.ASurface_3.__init__( self )
      m = aims.rc_ptr_AimsTimeSurface_3( aims.SurfaceGenerator.sphere( \
        self._center, self._radius, 100 ) )
      self.setSurface( m )
  def radius( self ):
    return self._radius
  def center( self ):
    return self._center
  def setCenter( self, c ):
    if type( c ) is not aims.Point3df:
      c = aims.Point3df( *c )
    cdiff = c - self._center
    self._center = c
    # translate all
    v = self.surface().vertex()
    for vi in v:
      vi += cdiff
    self.setChanged()
    self.UpdateMinAndMax()
    self.notifyObservers()
  def setRadius( self, r ):
    if r == 0:
      print 'can\'t assign radius 0'
      return
    scl = float(r) / self._radius
    self._radius = float(r)
    # scale all
    v = self.surface().vertex()
    c = self._center
    for vi in v:
      vi.assign( ( vi - c ) * scl + c )
    self.setChanged()
    self.UpdateMinAndMax()
    self.notifyObservers()

# example

if __name__ == '__main__':
  a = anatomist.Anatomist()
  s = ASphere()
  s.setName( 'sphere' )
  a.registerObject( s )

  #import qt
  #qt.qApp.exec_loop()

