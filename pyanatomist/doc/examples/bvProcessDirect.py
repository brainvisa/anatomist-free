from neuroProcesses import *
import anatomistapi.api
import shfjGlobals

# use direct (threaded) anatomist implementation (using bindings)
import neuroConfig
neuroConfig.anatomistImplementation = 'threaded'
from brainvisa import anatomist as pyanatomist

name = 'Test anatomist API direct implementation'
userLevel = 0

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
  a=pyanatomist.Anatomist()
  print "anatomist instance:", a
  # register function to be called on some anatomist events
  a.onLoadNotifier.add(afficher)
  a.onLoadNotifier.remove(afficher) # unregister a  listener
  a.onCreateWindowNotifier.add(afficher)
  a.onDeleteNotifier.add(afficher)
  a.onFusionNotifier.add(afficher)
  a.onCloseWindowNotifier.add(afficher)
  a.onAddObjectNotifier.add(afficher)
  a.onRemoveObjectNotifier.add(afficher)
  a.onCursorNotifier.add(afficher)
  
  obj=a.loadObject(self.object_to_load)
  w=a.createWindow('Axial')
  w.addObjects([obj])
  ref=obj.referential
  context.write( "referentiel de l'objet :", ref.refUuid)
  return [obj, w, ref]
  
def afficher(eventName, params):
  print "** Event ** ", eventName, params
  
def afficherCreation(instance):
  print "** Creation d'une instance d'anatomist", instance