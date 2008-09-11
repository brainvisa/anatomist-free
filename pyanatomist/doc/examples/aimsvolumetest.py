import anatomist.direct.api as anatomist
from soma import aims

# load any volume as a aims.Volume_* object
r = aims.Reader()
vol = r.read( 'irm.ima' )

# initialize Anatomist
a = anatomist.Anatomist()

# convert the AimsData volume to Anatomist API
avol = a.toAObject( vol )

win=a.createWindow('Axial')

# put volume in window
a.addObjects([ avol ], [ win ] )

##  volume change and update test

# using Numeric API
arr = vol.arraydata()
# we're just printing a white square in the middle of the volume
arr[0,50:70,100:130,100:130] = 255

# update Anatomist object and its views
avol.setChanged()
avol.notifyObservers()


##  fusion test

# have a second volume. We could have loaded another one
# (and it would have been nicer on screen). This example also
# shows the ability to share the volume data with multiple anatomist/aims
# objects
avol2 = a.toAObject( vol )

# set a different palette on the second object so we can see something
a.setObjectPalette([ avol2 ], 'Blue-Red-fusion')

# another view
win2=a.createWindow( 'Axial' )

a.addObjects([ avol2 ], [ win2 ])

# create the fusion object
fus=a.fusionObjects([ avol, avol2 ], 'Fusion2DMethod')

# show it
win3=a.createWindow('Sagittal' )

a.addObjects([ fus ], [ win3 ] )

