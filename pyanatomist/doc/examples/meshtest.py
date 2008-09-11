from soma import aims
import time
from qt import *
import os
import anatomist.direct.api as anatomist

# Load a sphere mesh
r = aims.Reader()
mfile = 'test.mesh__'
if os.path.exists( mfile ):
  m = r.read( mfile )
else:
  # create a unit sphere of radius 1 and 500 vertices
  m = aims.SurfaceGenerator.sphere( aims.Point3df( 0,0,0 ), 1, 500 )

# Multiply the sphere size by 100
for p in xrange( m.vertex().size() ):
  m.vertex()[ p ] *= 100

# Open Anatomist
a = anatomist.Anatomist()

# Put the mesh in anatomist
am = a.toAObject( m )

# Create a new 3D window in Anatomist
aw=a.createWindow( '3D' )
#c = anatomist.CreateWindowCommand( '3D' )
#proc.execute( c )
#aw = c.createdWindow()

# Put the mesh in the created window
a.addObjects( [ am ], [ aw ] )
#c = anatomist.AddObjectCommand( [ am ], [ aw ] )
#proc.execute( c )

# keep a copy of original vertices
coords = [ aims.Point3df( m.vertex()[i] ) \
           for i in xrange( len( m.vertex() ) ) ]
# take one vertex out of 3
points = xrange( 0, len(coords), 3)

for i in xrange( 10 ):
  # shrink
  for s in reversed(xrange(100)):
    for p in points:
      m.vertex()[p] = coords[p] * s/100.
    am.setChanged()
    am.notifyObservers()
    qApp.processEvents()
    time.sleep( 0.01 )
  # expand
  for s in xrange(100):
    for p in points:
      m.vertex()[p] = coords[p] * s/100.
    am.setChanged()
    am.notifyObservers()
    qApp.processEvents()
    time.sleep( 0.01 )
