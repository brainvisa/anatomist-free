#!/usr/bin/env python

import anatomist.cpp as anatomist, qt, operator, numpy
from soma import aims

r = aims.Reader()
vol = r.read( '/home/riviere/data/irm.ima' )

runloop = False
if __name__ == '__main__':
  # start events loop if it's not running
  if qt.QApplication.startingUp():
    print 'start events loop'
    runloop = True
  else:
    print 'events loop already running'

a = anatomist.Anatomist()
p = a.theProcessor()

#avol = anatomist.AObjectConverter.anatomist( vol )

#p.execute( 'SetObjectPalette', objects=[ avol ], palette='semitransparent2',
           #min=60./618., max=140./618 )
#p.execute( 'SetMaterial', objects=[ avol ], lighting=0 )
#p.execute( 'TexturingParams', objects=[ avol ], mode='geometric' )

# process gradients / normals
varr = vol.arraydata()
vol3d = varr.reshape( varr.shape[ 1: ] ) # see it as a 3D array rather than 4D
grads = numpy.gradient( vol3d )
norms = numpy.sqrt( grads[0] * grads[0] + grads[1] * grads[1] + grads[2] * grads[2] )
ngrad = [ x / norms for x in grads ]
del grads, norms
for x in ngrad:
  x[ numpy.isnan(x) ] = 1.



nslices = 150
direction = [ 1., 0., 0. ]

dscal = numpy.abs( numpy.dot( ngrad[0], direction[2] ) \
  + numpy.dot( ngrad[1], direction[1] ) + numpy.dot( ngrad[2], direction[0] ) )

vols = []
slices = []

h = vol.header()
vs = h[ 'voxel_size' ]
cmax = [ vol.getSizeX() * vs[0], vol.getSizeY() * vs[1],
         vol.getSizeZ() * vs[2] ]
z0 = aims.Point3df( 0, cmax[1] * 0.5, cmax[2] * 0.5 )
z1 = aims.Point3df( cmax[0]*0.7, z0[1], z0[2] )

#pal = reduce( operator.add, [ [ 0, 0, 0, i ] for i in xrange( 256 ) ] )
# + reduce( operator.add, [ [ 0, 0, 0, 255-i ] for i in xrange( 256 ) ] )
basecol = [ 255, 255, 255 ]
pal = reduce( operator.add, [ basecol + [ i ] for i in xrange( 256 ) ] ) \
  + reduce( operator.add, [ basecol + [ 255-i ] for i in xrange( 256 ) ] ) \
  + reduce( operator.add, [ basecol + [ i ] for i in xrange( 256 ) ] )

p.execute( 'NewPalette', name='black-transparent' )
p.execute( 'ChangePalette', name='black-transparent', color_mode='RGBA',
           colors=pal )

vscal = aims.Volume_DOUBLE( dscal )
vscal.header()[ 'voxel_size' ] = h[ 'voxel_size' ]

avol = anatomist.AObjectConverter.anatomist( vol )
avolc = anatomist.AObjectConverter.anatomist( vol )
apnorm = anatomist.AObjectConverter.anatomist( vscal )
c = anatomist.FusionObjectsCommand( [ avolc, avol, apnorm ], 'Fusion2DMethod' )
p.execute( c )
v = c.createdObject()
#p.execute( 'Fusion2DParams', object=[ v ], mode='geometric' )
p.execute( 'TexturingParams', objects=[ v ], texture_index=1, mode='geometric' )
p.execute( 'TexturingParams', objects=[ v ], texture_index=2, mode='geometric' )

c = anatomist.CreateWindowCommand( 'Sagittal' )
p.execute( c )
win = c.createdWindow()
p.execute( 'WindowConfig', windows=[ win ], fog=1 )

p.execute( 'SetObjectPalette', objects=[ avol ], palette='black-transparent',
           min=40./618., max=350./618 )
p.execute( 'SetObjectPalette', objects=[ apnorm ], palette='B-W LINEAR',
           min=-0.1, max=0.775 )
p.execute( 'SetMaterial', objects=[ v ], lighting=0 )
p.execute( 'TexturingParams', objects=[ v ], mode='geometric' )

for i in xrange( nslices ):
  l = 1. - float( i+1 ) / (nslices+1)
  z = z0 * (1.-l) + z1 * l
  # create a specific volume
  #v =  anatomist.AObjectConverter.anatomist( vol )
  #p.execute( 'SetObjectPalette', objects=[ v ], palette='semitransparent2',
           #min=60./618., max=(105.+(1.-l)*500)/618 )
  #p.execute( 'SetObjectPalette', objects=[ v ], palette='black-transparent',
           #min=40./618., max=(160.+(1.-l)*600)/618 )
  #p.execute( 'SetMaterial', objects=[ v ], lighting=0 ) #, diffuse=[ 0.8, 0.8, 0.8, l ] )
  #p.execute( 'TexturingParams', objects=[ v ], mode='geometric' )
  #vols.append( v )

  c = anatomist.FusionObjectsCommand( [ v ], 'FusionSliceMethod' )
  p.execute( c )
  s = c.createdObject()
  slices.append( s )
  p.execute( 'SliceParams', objects=[ s ], position=z.list(),
             quaternion=[ -0.5, -0.5, -0.5, 0.5 ] )
  p.execute( 'SetMaterial', objects=[ s ], lighting=0 )
  p.execute( 'TexturingParams', objects=[ s ], mode='geometric' )
  p.execute( 'AddObject', objects=[ s ], windows=[ win ] )

if runloop:
  qt.qApp.exec_loop()

