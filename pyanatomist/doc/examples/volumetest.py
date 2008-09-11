import anatomist.api as anatomist # works with all implementations

# initialize Anatomist
a = anatomist.Anatomist()

# load a volume in anatomist
avol = a.loadObject('irm.ima' )

win=a.createWindow('Axial')

# put volume in window
a.addObjects([ avol ], [ win ] )

# change palette
avol.setPalette('Blue-Red-fusion')

print "object and window are not deletable since there is a reference on it. Execute 'del win' and 'del avol' to delete them."
