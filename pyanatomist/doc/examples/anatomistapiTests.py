#!/usr/bin/env python

"""
Anatomist api tests.
"""

import anatomist
import threading
import qt
from qt import QApplication
import sys

def testDirectImpl():
  import anatomist.direct.api as pyanatomist
  a=pyanatomist.Anatomist()
  return testAnatomist(a)
    
def testSocketImpl():
  # test socket impl, not threaded
  import anatomist.socket.api as pyanatomist
  a=pyanatomist.Anatomist()
  return testAnatomist(a)

def testAnatomist(a):
  print "--- CreateWindowBlock ---"
  block=a.createWindowsBlock(3)
  print block.internalRep, block.nbCols
  print "--- CreateWindow ---"
  w1=a.createWindow(wintype = 'Axial', block = block)
  print w1, block.internalRep
  w2=a.createWindow(wintype = 'Sagittal', geometry = [10,20,200,500], block = block)
  w3=a.createWindow(wintype = 'Coronal', block=block, no_decoration=True)
  w4=a.createWindow(wintype = '3D', geometry = [10,20,200,500])
  w5=a.createWindow(wintype='3D', no_decoration=True)
  print "--- LoadObject ---"
  
  o=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lhemi.mesh", "objetO")
  o2=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lwhite_curv.tex", "objetO2")
  o3=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/roi/basal_ganglia.arg")
  o4=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/roi/basal_ganglia.hie")
  o5=a.loadObject(filename="/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/3d/gis/subject01.ima", restrict_object_types = {'Volume' : [ 'FLOAT' ] })
  o6=a.loadObject(filename="/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/3d/gis/subject01.ima", restrict_object_types = {'Volume' : [ 'S16', 'FLOAT' ] })
  cur=a.loadCursor("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lhemi.mesh")
  print "--- FusionObjects ---"
  fus=a.fusionObjects([o, o2], "FusionTexSurfMethod", True)
  print "--- CreateReferential ---"
  r=a.createReferential("/home/appli/shared-main/registration/Talairach-MNI_template-SPM.referential")
  r2=a.createReferential()
  cr=a.centralRef
  print "central referential", cr
  print "--- LoadTransformation ---"
  t=a.loadTransformation("/neurospin/lnao/Panabase/demosII/data_for_anatomist/referential/ref_TO_talairach/chaos_TO_talairach.trm", cr, r2)
  print "--- CreatePalette ---"
  p=a.createPalette("maPalette") 
  print "--- GroupObjects ---"
  g=a.groupObjects([o, o2])
  print "--- linkWindows ---"
  wg=a.linkWindows([w1,w2])
  print wg
  print "--- GetInfos ---"
  lo=a.getObjects()
  lio=a.importObjects(False)
  print "objects", lo, lio
  lw=a.getWindows()
  liw=a.importWindows()
  print "windows", lw, liw
  print "palettes", a.getPalettes() 
  lr=a.getReferentials()
  lir=a.importReferentials()
  print "referentials", lr, lir
  lt=a.getTransformations()
  lit=a.importTransformations()
  print "transformations", lt, lit
  sel=a.getSelection()
  print "selections in default group", sel
  print "cursor last pos", a.linkCursorLastClickedPosition()
  print "cursor last pos dans ref r2", a.linkCursorLastClickedPosition(r2)
  print "--- AddObjects ---"
  a.addObjects([fus], [w1,w2,w3])
  print "--- RemoveObjects ---"
  a.removeObjects([fus], [w2])
  print "--- DeleteObjects ---"
  a.deleteObjects([g])
  print "--- AssignReferential ---"
  a.assignReferential(r, [o6, w2])
  print "--- camera ---"
  a.camera([w1], 1.5)
  print "--- closeWindows ---"
  a.closeWindows([w4])
  print "--- setMaterial ---"
  o.addInWindows([w2])
  w2.assignReferential(cr)
  mat=a.Material([0.5,0.1,0.1,1], smooth_shading=1)
  a.setMaterial([o], mat)
  print "New material :", o.material
  print "--- setObjectPalette ---"
  pal=a.getPalette("Blue-Red") 
  w6=a.createWindow('Axial')
  o6.addInWindows([w6])
  a.setObjectPalette([o6], pal,0, 0.2) 
  print "--- setGraphParams ---"
  a.setGraphParams(display_mode="mesh")
  print "\n--- AObject Methods---"
  w4=a.createWindow(wintype='3D')
  o3.addInWindows([w4])
  print "object attributes :",o3, o3.filename, o3.material, o3.objectType, o3.children
  o4.addInWindows([w4])
  o4.removeFromWindows([w4])
  o6.delete()
  o4.assignReferential(r)
  fus.setMaterial(mat)
  o.setPalette(pal) 
  tex=fus.extractTexture() 
  print "extracted texture:", tex
  tex=fus.generateTexture() 
  fus2=a.fusionObjects([o, tex])
  fus2.addInWindows([w5])
  print "generated texture: ", tex
  fus.exportTexture("/tmp/exportedTexture.tex") 
  o.save("/tmp/savedObject.mesh")
  print "\n--- AWindow Methods---" 
  w4.addObjects([o4])
  print "window attributes: ", w4.windowType, w4.group 
  w4.removeObjects([o4])
  w2.camera(2)
  w2.assignReferential(r2)
  w2.assignReferential(cr)
  w2.addObjects([fus])
  w2.moveLinkedCursor([10,10,0])
  w4.showToolbox()
  w4.close()
  print "\n--- AWindowsGroup Methods---" 
  wg.setSelection([fus])
  print "selection in default group:", a.getSelection()
  print "selection:", a.getSelection(wg)
  wg.unSelect([fus])
  print "selection:", a.getSelection(wg)
  w4=a.createWindow('3D')
  w4.addObjects([o3])
  g0=a.AWindowsGroup(a, 0)
  g0.setSelectionByNomenclature(o4, ['Caude_droit']) 
  print "selection default group:", g0.getSelection()
  g0.addToSelectionByNomenclature(o4, ['Caude_gauche']) 
  print "selection default group:", g0.getSelection()
  g0.toggleSelectionByNomenclature(o4, ['Caude_gauche']) 
  print "selection default group:", g0.getSelection()
  print "\n--- APalette Methods---"
  pal.setColors(colors=[1,0,0]) # don't know what parameters to give...
  print "\n--- ATransformation Methods---"
  t.save("/tmp/savedTransformation.trm")
  objects=a.getObjects()
  windows=a.getWindows()
  return (objects, windows)

def testBase():
  # base module : simple interface, methods are not implemented
  import anatomist.base as pyanatomist
  a=pyanatomist.Anatomist()
  w=a.createWindow('Axial')
  print w

print "\n****  TEST ANATOMIST API DIRECT IMPLEMENTATION ****\n"
res1=testDirectImpl()
print "\n****  TEST ANATOMIST API SOCKET IMPLEMENTATION ****\n"
res2=testSocketImpl()