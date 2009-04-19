import anatomist.direct.api as anatomist
from soma import aims

# Open Anatomist
a = anatomist.Anatomist()

# create a sphere mesh
m = aims.SurfaceGenerator.sphere( aims.Point3df( 0 ), 100, 100 )
mesh = a.toAObject( m )

# Create a new 3D window in Anatomist
aw = a.createWindow( '3D' )

# Put the mesh in the created window
a.addObjects( mesh, aw )

g=a.getDefaultWindowsGroup()
#sel = anatomist.SelectFactory.factory()
print 'mesh isSelected:', g.isSelected( mesh )
print 'selecting it'
g.setSelection( mesh )
print "selection in default group", a.getSelection()
print "selection de", g, g.getSelection()
sel=g.getSelection()
#print mesh, sel, mesh == sel[0], mesh is sel[0]
print 'mesh isSelected:', g.isSelected( mesh )

