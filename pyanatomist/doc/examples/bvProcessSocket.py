from neuroProcesses import *
import shfjGlobals
import string

name = 'Test anatomist API socket implementation'
userLevel = 0

# by default, socket implementation is loaded in brainvisa
from brainvisa import anatomist as pyanatomist
import anatomist.api

signature = Signature(
  'object_to_load', ReadDiskItem( "3D Volume", shfjGlobals.anatomistVolumeFormats ),
  )

def initialization( self ):
  pass

def execution( self, context ):
  # register a function that will be called when Anatomist application starts
  pyanatomist.Anatomist.addCreateListener(afficherCreation)
  # with attribute create is False, the constructor returns the existing instance of anatomist or None if there isn't one.
  a=pyanatomist.Anatomist(create=False)
  print "anatomist instance:", a
  a=pyanatomist.Anatomist(create=True)
  print "anatomist instance:", a
  # register a function to be called when an object is loaded in anatomist
  a.onLoadNotifier.add(afficher)
  #a.onLoadNotifier.remove(afficher)
  #a.onCreateWindowNotifier.add(afficher)
  #a.onDeleteNotifier.add(afficher)
  #a.onFusionNotifier.add(afficher)
  #a.onCloseWindowNotifier.add(afficher)
  #a.onAddObjectNotifier.add(afficher)
  #a.onRemoveObjectNotifier.add(afficher)
  #a.onCursorNotifier.add(afficher)
  obj=a.loadObject(self.object_to_load)
  w=a.createWindow('Axial')
  a.addObjects([obj], [w])
  ref=obj.referential
  context.write( "referentiel de l'objet :", ref.refUuid)
  return [obj, w, ref]
  
def afficher(event, params):
  print "** Event ** ",event, params
  
def afficherCreation(instance):
  print "** Creation d'une instance d'anatomist", instance