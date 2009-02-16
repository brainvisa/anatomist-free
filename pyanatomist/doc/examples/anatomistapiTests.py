#!/usr/bin/env python

"""
Anatomist api tests.
This example uses data of i2bm platform. 
"""

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
  print "\n--- CreateWindowsBlock ---"
  block=a.createWindowsBlock(3) # a block of windows with 3 columns
  # in direct api the block object is really created only when the first window is added to it
  print "block.internalRep = ", block.internalRep, ", block.nbCols = ", block.nbCols
  
  print "\n--- CreateWindow ---"
  w1=a.createWindow(wintype = 'Axial', block = block)
  print "w1 : Axial window, added to the block. w1 = ",w1, ", block.internalRep = ", block.internalRep
  w2=a.createWindow(wintype = 'Sagittal', geometry = [10,20,200,500], block = block)
  print "w2 : Sagittal window, added to the block. The geometry attribute is not taken into account because of the block, the window is resized to fit into the block."
  w3=a.createWindow(wintype = 'Coronal', block=block, no_decoration=True)
  print "w3 : Coronal window, added to the block (3rd column), thanks to the attribute no decoration, the window has no menus, buttons and so on..."
  w4=a.createWindow(wintype = '3D', geometry = [10,20,200,500])
  print "w4 : 3D window, not in the block, with geometry attribute."
  w5=a.createWindow(wintype='3D', no_decoration=True)
  print "w5 : 3D window, not in the block, with no decoration."
  
  print "\n--- LoadObject ---"
  o=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lhemi.mesh", "objetO")
  print "o : mesh Lhemi.mesh, renamed objecO, internalRep = ", o, ", o.__class__ = ", o.__class__
  o2=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lwhite_curv.tex", "objetO2")
  print "o2 : texture Lwhite_curv.tex, renamed objetO2, internalRep = ", o2
  o4=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/roi/basal_ganglia.hie")
  print "o4 : nomenclature basal_ganglia.hie, internalRep = ", o4
  o3=a.loadObject("/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/roi/basal_ganglia.arg")
  print "o3 : graph basal_ganglia.arg, internalRep = ", o3
  # this should fail
  print 'o5 : trying to load an object with wrong type (field restrict_object_type doesn\'t match the type of the object): it should fail.'
  o5=a.loadObject(filename="/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/3d/gis/subject01.ima", restrict_object_types = {'Volume' : [ 'FLOAT' ] })
  print 'The error message is normal, o5 = ', o5
  o6=a.loadObject(filename="/neurospin/lnao/Panabase/demosII/data_for_anatomist/objects/3d/gis/subject01.ima", restrict_object_types = {'Volume' : [ 'S16', 'FLOAT' ] })
  print "o6 : volume subject01.ima, restrict_object_type match the object type. internalRep = ", o6
  o7=a.loadObject(filename="/neurospin/lnao/Panabase/demosII/data_for_anatomist/fusion/volume_volume/no_bias_subject01.ima")
  print "o7 : same object"
  cur=a.loadCursor("/neurospin/lnao/Panabase/demosII/data_for_anatomist/functional/Lhemi.mesh")
  print "cur : load Lhemi.mesh as a cursor. The object is not in the list of objects in Anatomist main window but can be selected as cursor in preferences. "
  
  print "\n--- FusionObjects ---"
  fus=a.fusionObjects([o, o2], "FusionTexSurfMethod", True)
  print "fus : fusion of o and o2, method is FusionTexSurfMethod and askOrder is True, so a window opens to let the user choose the order of the objects in the fusion. internalRep = ", fus
  
  print "\n--- CreateReferential ---"
  r=a.createReferential("/home/a-sac-ns-research/shared-main/registration/Talairach-MNI_template-SPM.referential")
  print "r : Referential loaded from the Talairach-MNI_template-SPM.referential. r.__class__ = ", r.__class__, ", internalRep = ", r, ", r.refUuid = ", r.refUuid,". This should not create a new referential because Talairach-MNI_template-SPM referential is already loaded in Anatomist. "
  r2=a.createReferential()
  print "r2 : new referential. internalRep = ", r2, ", refUuid = ", r2.refUuid
  cr=a.centralRef
  print "cr : central referential, ", cr, ", refUuid = ", cr.refUuid, ". This referential is already loaded in Anatomist."
  
  print "\n--- LoadTransformation ---"
  t=a.loadTransformation("/neurospin/lnao/Panabase/demosII/data_for_anatomist/referential/ref_TO_talairach/chaos_TO_talairach.trm", r2, cr)
  print "t : loaded from the file chaos_TO_talairach.trm, as a transformation between r2 and cr. t.__class__ = ", t.__class__, ", internalRep = ", t
  
  print "\n--- CreatePalette ---"
  p=a.createPalette("maPalette")
  print "p : new palette named maPalette, added Anatomist list of palettes. p.__class__ = ", p.__class__, ", internalRep = ", p
  
  print "\n--- GroupObjects ---"
  g=a.groupObjects([o, o2])
  print "g : new group of objects containing o and o2. g.__class__ = ", g.__class__, ", internalRep = ", g
  
  print "\n--- linkWindows ---"
  wg=a.linkWindows([w1,w2])
  print "wg : new group of windows containing w1 and w2. wg.__class__ = ", wg.__class__, ", internalRep = ", wg
  
  print "\n--- GetInfos ---"
  lo=a.getObjects()
  print "\nObjects refererenced in current context : ", lo
  print "Total : ", len(lo)
  lio=a.importObjects(False) #top_level_only = False -> all objects
  print "All objects (importing those that were not referenced in current context) : ", lio
  print "Total : ", len(lio) 
  print "-> Should be the same in direct implementation."
  
  lw=a.getWindows()
  liw=a.importWindows()
  print "\nGetWindows : ", len(lw), ", ImportWindows : ", len(liw)
  print liw
  
  print "\nPalettes : ", a.getPalettes()
  
  lr=a.getReferentials()
  lir=a.importReferentials()
  print "\ngetReferentials : ", len(lr), ", importReferentials : ",  len(lir)
  print lir
  
  lt=a.getTransformations()
  lit=a.importTransformations()
  print "\ngetTransformations : ", len(lt), ", importTransformations : ", len(lit)
  
  sel=a.getSelection()
  print "\nSelections in default group", sel
  
  print "\nCursor last pos", a.linkCursorLastClickedPosition()
  print "Cursor last pos dans ref r2", a.linkCursorLastClickedPosition(r2)
  
  print "\n--- AddObjects ---"
  a.addObjects([fus], [w1,w2,w3])
  print "Object fus added in windows w1, w2, and w3."
  
  print "\n--- RemoveObjects ---"
  a.removeObjects([fus], [w2, w1])
  print "Object fus removed from window w1 and w2."
  
  print "\n--- DeleteObjects ---"
  # delete the list of objects to avoid keeping a reference on object that prevent from deleting it
  del lo
  del lio
  a.deleteObjects([o7])
  print "Delete object o7."
  
  print "\n--- AssignReferential ---"
  a.assignReferential(r, [o6, w2])
  print "Referential r assigned to object o6 and window w2."
  
  print "\n--- camera ---"
  a.camera([w3], zoom=1.5)
  print "Set zoom to 1.5 in window w3."
  
  print "\n--- closeWindows ---"
  # delete lists of windows to avoid keeping a reference that prevent from closing the window.
  del lw
  del liw
  a.closeWindows([w4])
  print "Close window w4."
  
  print "\n--- setMaterial ---"
  o.addInWindows([w1])
  mat=a.Material([0.5,0.1,0.1,1], smooth_shading=1)
  a.setMaterial([o], mat)
  print "Add object o to the window w1 and change its material : ", o.material

  print "\n--- setObjectPalette ---"
  pal=a.getPalette("Blue-Red") 
  w6=a.createWindow('Axial')
  o6.addInWindows([w6])
  a.setObjectPalette([o6], pal, minVal=0, maxVal=0.2) 
  print "Put object o6 in a new Axial window w6 and change its palette to Blue-Red with min and max values to 0 and 0.2"
  
  print "\n--- setGraphParams ---"
  a.setGraphParams(display_mode="mesh")
  print "Set display mode (paint mode of objects in graph nodes) to mesh."
  
  print "\n--- AObject Methods---"
  w7=a.createWindow('3D')
  o3.addInWindows([w7])
  print "Put object o3 in new 3D window (w7). o3 attributes : filename : ", o3.filename, ", material : ", o3.material, "objectType : ", o3.objectType, ", children : ", o3.children
  
  o.addInWindows([w6])
  o.removeFromWindows([w6])
  print "\nAdd and remove object o from window w6."
  print "Try to delete o2. Should fail because the object is used in a fusion :"
  o2.delete()
  
  # Some methods are available in Anatomist class and Objects classes
  # Anatomist.assignReferential -> AObject.assignReferential
  # Anatomist.setMaterial -> AObject.setMaterial
  # Anatomist.setObjectPalette -> AObject.setPalette
  o4.assignReferential(r)
  fus.setMaterial(mat)
  o.setPalette(pal) 
  
  tex=fus.extractTexture() 
  print "\nExtract texture from object fus :", tex
  tex=fus.generateTexture() 
  print "Generate a texture: tex =", tex
  fus2=a.fusionObjects([o, tex])
  fus2.addInWindows([w5])
  print "Fusion the generated texture with object o : fus2 = ", fus2
  fus.exportTexture("/tmp/exportedTexture.tex")
  print "fus texture is saved in file /tmp/exportedTexture.tex."
  o.save("/tmp/savedObject.mesh")
  print "The object o is saved in the file /tmp/savedObject.mesh."
  
  print "\n--- AWindow Methods---" 
  print "Window attributes: w2.windowType = ", w2.windowType,", w2.group = ", w2.group 
  # Some methods available in Anatomist class are also available directly in AWindows class.
  w2.addObjects([o])
  w2.removeObjects([o])
  w2.camera(2)
  w2.assignReferential(cr)
  w2.addObjects([fus])
  w2.moveLinkedCursor([150,100,60])
  w6.showToolbox() # opens the toolbox window. This toolbox will be empty if the window is empty. If there is an object on which it is possible to draw a roi, the roi toolbox will be shown.
  
  print "\n--- AWindowsGroup Methods---" 
  wg.setSelection([fus])
  print "Set fus object as selected in the group of windows wg : "
  print "-> selection in default group has not changed :", a.getSelection()
  print "-> selection in the group of window wg :", a.getSelection(wg)
  wg.unSelect([fus])
  print "After unselect, selection in wg :", a.getSelection(wg)
  
  g0=a.getDefaultWindowsGroup() # the default group has the id 0
  g0.setSelectionByNomenclature(o4, ['Caude_droit']) 
  print "Selection by nomenclature in default group - add 'Caude_droit':", g0.getSelection()
  g0.addToSelectionByNomenclature(o4, ['Caude_gauche']) 
  print "Selection by nomenclature  default group - add 'Caude_gauche' :", g0.getSelection()
  g0.toggleSelectionByNomenclature(o4, ['Caude_gauche']) 
  print "Toggle selection by nomenclature in default group - toggle 'Caude_gauche' : ", g0.getSelection()
  
  print "\n--- APalette Methods---"
  # set colors take as parameter a list of RGB components for colors : [r1,g1,b1,r2,g2,b2...]
  p.setColors(colors=[100,0,0]*20+[0,100,0]*20+[0,0,100]*20) 
  print "The colors of palette 'maPalette' has changed."
  
  print "\n--- ATransformation Methods---"
  t.save("/tmp/savedTransformation.trm")
  print "Save the transformation t in file /tmp/savedTransformation.trm."
  
  # return objects and windows to keep a reference on them and avoid their destroying.
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