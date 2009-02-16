import anatomist.api as anatomist

# initialize Anatomist
a = anatomist.Anatomist()

# load a volume in anatomist
avol = a.loadObject('irm.ima' )
amesh = a.loadObject('test.mesh')

# fusion the objects
fusion=a.fusionObjects(objects=[avol, amesh], method="Fusion3DMethod")
# params of the fusion
a.execute("Fusion3DParams", object=fusion, method="line", submethod="mean", depth=4, step=1)

# open a window
win=a.createWindow('Axial')
# put volume in window
a.addObjects([ fusion ], [ win ] )

# export the fusion texture in a file.
fusion.exportTexture("fusion.tex")



