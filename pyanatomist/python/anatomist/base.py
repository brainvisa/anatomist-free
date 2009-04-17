# Copyright CEA and IFR 49 (2000-2005)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ and IFR 49
#      4 place du General Leclerc
#      91401 Orsay cedex
#      France
#
# This software is governed by the CeCILL license version 2 under 
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the 
# terms of the CeCILL license version 2 as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license version 2 and that you accept its terms.

"""
General interface of pyanatomist API. It describes classes and methods that are shared by all implementations : the set of Anatomist features available throught this API.
Several implementations exist depending on the mean of driving Anatomist (Sip bindings C++/Python or commands via socket).
"""
from soma.notification import ObservableNotifier
from soma.wip.singleton import ObservableSingleton
from soma.functiontools import partial
import operator
import string
import threading


class Anatomist(ObservableSingleton, object):
  """
  Interface to communicate with an Anatomist Application. This class is virtual, some methods are not implemented. It is the base class of Anatomist classes in each implementation.
  
  This class is a Singleton, so there is only one global instance of this class. The first time the constructor is called, an instance is created. Each next time, the same instance is returned.
  Anatomist inherits from a particular Singleton : ObservableSingleton. So it is possible to listen for anatomist instance creation event. It is also possible to ask for anatomist instance without creating an instance if it does not exit. To do this, use the constructor with create=False as parameter : 
  
  C{a=anatomist.Anatomist(create=False)} will return the current instance or None if no instance exists.
  
  This class can notify anatomist events. To call a function when an event occurs, add a listener to one of Anatomist's notifiers. 
  
  For example:
  
  C{anatomist.onLoadNotifier.add(listener)}
  
  listener must be a callback function that accepts two parameters : the event name (string) and a dictionary of parameters describing the event.
  
  @type onLoadNotifier: ObservableNotifier
  @ivar onLoadNotifier: notifies object loading. Event parameters : { 'filename' : string, 'object' : AObject, 'type' : string }
  @type onDeleteNotifier: ObservableNotifier
  @ivar onDeleteNotifier: notifies object deletion. Event parameters : {'object' : AObject }
  @type onFusionNotifier : ObservableNotifier
  @ivar onFusionNotifier : notifies objects fusion. Event parameters : {'children' : list of AObject, 'method' : string, 'object' : AObject, 'type' : string }
  @type onCreateWindowNotifier: ObservableNotifier
  @ivar onCreateWindowNotifier: notifies window creation. Event parameters : {'type' : string, 'window' : AWindow }
  @type onCloseWindowNotifier: ObservableNotifier
  @ivar onCloseWindowNotifier: notifies window closing. Event parameters : {'window' : AWindow }
  @type onAddObjectNotifier: ObservableNotifier
  @ivar onAddObjectNotifier: notifies object adding in window. Event parameters : {'object' : AObject, 'window' : AWindow }
  @type onRemoveObjectNotifier: ObservableNotifier
  @ivar onRemoveObjectNotifier: notifies object removing from window. Event parameters : {'object' : AObject, 'window' : AWindow }
  @type onCursorNotifier: ObservableNotifier
  @ivar onCursorNotifier: notifies cursor position change. Event parameters : {'position' : float vector size 4, 'window' : AWindow }
  @type onExitNotifier: ObservableNotifier
  @ivar onExitNotifier: notifies that Anatomist application exits.
  
  @type centralRef: Referential
  @ivar centralRef: anatomist's central referential (talairach acpc ref)
  @type mniTemplateRef : Referential
  @ivar mniTempateRef: template mni referential (used by spm)
  These two referentials and transformation between them are always loaded in anatomist.
  
  @type defaultRefType: string
  @cvar defaultRefType: ref type taken by default on anatomist objects. Strong means that objects or windows cannot be deleted while a reference exist on it.
  @type lock: threading.RLock
  @cvar lock: enable to take a lock on anatomist singleton instance
  """
  defaultRefType="Strong"
  lock = threading.RLock()
  def __singleton_init__(self, *args, **kwargs):
    object.__init__(self, *args, **kwargs)
    
    self.onLoadNotifier=ObservableNotifier()
    # enable listening of event  only when the notifier has at least one listener.
    self.onLoadNotifier.onAddFirstListener.add( partial( self.enableListening, "LoadObject", self.onLoadNotifier ) )
    self.onLoadNotifier.onRemoveLastListener.add( partial( self.disableListening, "LoadObject" ) )
    
    self.onDeleteNotifier=ObservableNotifier()
    self.onDeleteNotifier.onAddFirstListener.add( partial( self.enableListening, "DeleteObject", self.onDeleteNotifier ) )
    self.onDeleteNotifier.onRemoveLastListener.add( partial( self.disableListening, "DeleteObject" ) )
    
    self.onFusionNotifier=ObservableNotifier()
    self.onFusionNotifier.onAddFirstListener.add( partial( self.enableListening, "FusionObjects", self.onFusionNotifier ) )
    self.onFusionNotifier.onRemoveLastListener.add( partial( self.disableListening, "FusionObjects" ) ) 
    
    self.onCreateWindowNotifier=ObservableNotifier()
    self.onCreateWindowNotifier.onAddFirstListener.add( partial( self.enableListening, "CreateWindow", self.onCreateWindowNotifier ) )
    self.onCreateWindowNotifier.onRemoveLastListener.add( partial( self.disableListening, "CreateWindow" ) )
    
    self.onCloseWindowNotifier=ObservableNotifier()
    self.onCloseWindowNotifier.onAddFirstListener.add( partial( self.enableListening, "CloseWindow", self.onCloseWindowNotifier ) )
    self.onCloseWindowNotifier.onRemoveLastListener.add( partial( self.disableListening, "CloseWindow" ) )
    
    self.onAddObjectNotifier=ObservableNotifier()
    self.onAddObjectNotifier.onAddFirstListener.add( partial( self.enableListening, "AddObject", self.onAddObjectNotifier ) )
    self.onAddObjectNotifier.onRemoveLastListener.add( partial( self.disableListening, "AddObject" ) )
    
    self.onRemoveObjectNotifier=ObservableNotifier()
    self.onRemoveObjectNotifier.onAddFirstListener.add( partial( self.enableListening, "RemoveObject", self.onRemoveObjectNotifier ) )
    self.onRemoveObjectNotifier.onRemoveLastListener.add( partial( self.disableListening, "RemoveObject" ) )

    self.onCursorNotifier=ObservableNotifier()
    self.onCursorNotifier.onAddFirstListener.add( partial( self.enableListening, "LinkedCursor", self.onCursorNotifier ) )
    self.onCursorNotifier.onRemoveLastListener.add( partial( self.disableListening, "LinkedCursor" ) )
    
    self.onExitNotifier=ObservableNotifier()
    self.onExitNotifier.onAddFirstListener.add( partial( self.enableListening, "Exit", self.onExitNotifier ) )
    self.onExitNotifier.onRemoveLastListener.add( partial( self.disableListening,"Exit" ) ) 

  def enableListening(self, event, notifier):
    """
    Set listening of this event on. So when the event occurs, the notifier's notify method is called.
    This method is automatically called when the first listener is added to a notifier. That is to say that notifiers are activated only if they have registered listeners.
    
    @type event: string
    @param event: name of the event to listen
    @type notifier: Notifier
    @param notifier: the notifier whose notify method must be called when this event occurs
    """
    pass
  
  def disableListening(self, event):
    """
    Set listening of this event off.
    
    @type event: string
    @param event: name of the event to disable.
    """
    pass
  
  # objects creation
  def createWindowsBlock(self, nbCols=None):
    """
    Creates a window containing other windows.
    
    @type nbCols: int
    @param nbCols: number of columns of the windows block

    @rtype: AWindowBlock
    @return: a window which can contain several AWindow
    """
    pass
    
  def createWindow(self, wintype, geometry=None, block=None, no_decoration=None):
    """
    Creates a new window and opens it.
    @type wintype: string
    @param wintype: type of window to open ("Axial", "Sagittal", "Coronal", "3D", "Browser", "Profile")
    @type geometry: int vector
    @param geometry: position on screen and size of the new window (x, y, w, h)
    @type block: AWindowBlock
    @param block: a block in which the new window must be added
    @type no_decoration: bool
    @param no_decoration: indicates if decorations (menus, buttons) can be painted around the view.
    
    @rtype: AWindow
    @return: the newly created window
    """
    pass
    
  def loadObject(self, filename, objectName=None, restrict_object_types=None, forceReload=True, duplicate=False, hidden=False):
    """
    Loads an object from a file (volume, mesh, graph, texture...)
    
    @type filename: string
    @param filename: the file containing object data
    @type objectName: string
    @param objectName: object's name
    @type restrict_object_types: dictionary
    @param restrict_object_types: object -> accpepted types list. Ex: {'Volume' : ['S16', 'FLOAT']}
    @type forceReload: boolean
    @param forceReload: if True the object will be loaded even if it is already loaded in Anatomist. Otherwise, the already loaded one is returned.
    @type duplicate: boolean
    @param duplicate: if the object already exists, duplicate it. The original and the copy will share the same data but not display parameters as palette. If the object is not loaded yet, load it hidden and duplicate it (unable to keep the original object with default display parameters).
    @type hidden: boolean
    @param hidden: an idden object does not appear in Anatomist main control window.
    
    @rtype: AObject
    @return: the loaded object
    """
    pass
    
  def duplicateObject(self, source, shallowCopy=True):
    """
    Creates a copy of source object.
    
    @type source: AObject
    @param source: the object to copy.
    
    @rtype: AObject
    @return: the copy
    """
    pass
  
  def createGraph(self, object, name=None, syntax=None, filename=None):
    """
    Creates a graph associated to a object (volume for example). This object initializes graph's dimensions (voxel size, extrema).
    
    @type object: AObject
    @param object: the new graph is based on this object
    @type name: string
    @param name: graph name. default is RoiArg.
    @type syntax: string
    @param syntax: graph syntax attribute. default is RoiArg.
    @type filename: string
    @param filename: filename used for saving. Default is None.

    @rtype: AGraph
    @return: the new graph object
    """
    pass

  def loadCursor(self, filename):
    """
    Loads a cursor for 3D windows from a file.
    
    @type filename: string
    @param filename: the file containing object data
    
    @rtype: AObject
    @return: the loaded object
    """
    pass
    
  def fusionObjects(self, objects, method=None, ask_order=False):
    """
    Creates a multi object that contains all given objects.
    
    @type objects: list of AObject
    @param objects: list of objects that must be fusionned
    @type method: string
    @param method: method to apply for the fusion (Fusion2DMethod...)
    @type ask_order: boolean
    @param ask_order: if True, asks user in what order the fusion must be processed.
        
    @rtype: AObject
    @return: the newly created fusion object.
    """
    pass
  
  def createReferential(self, filename=None):
    """
    This command does not exist in Anatomist because the command AssignReferential can create a new referential if needed. 
    But the way of creating a new referential depends on the connection with Anatomist, 
    so it seems to be better to encapsulate this step on another command. So referentials are treated the same as other objects.
    (LoadObject -> addAobject | createReferential -> assignReferential)
    
    @type filename: string
    @param filename: name of a file (minf file, extension .referential) containing  informations about the referential : its name and uuid
    
    @rtype: Referential
    @return: the newly created referential
    """
    pass
  
  def loadTransformation(self, filename, origin, destination):
    """
    Loads a transformation from a referential to another. The transformation informations are given in a file.
    
    @type filename: string
    @param filename: file containing transformation informations 
    @type origin: Referential
    @param origin: origin of the transformation
    @type destination: Referential
    @param destination: coordinates' referential after applying transformation
    @rtype: Transformation
    @return: transformation to apply to convert coordinates from one referent
    """
    pass
  
  def createTransformation(self, matrix, origin, destination):
    """
    Creates a transformation from a referential to another. The transformation informations are given in a matrix. 
    
    @type matrix: float vector, size 12
    @param matrix: transformation matrix (4 lines, 3 colons ; 1st line: translation, others: rotation)
    @type origin: Referential
    @param origin: origin of the transformation
    @type destination: Referential
    @param destination: coordinates' referential after applying transformation
    @rtype: Transformation
    @return: transformation to apply to convert coordinates from one referent
    """
    pass

  def createPalette(self, name):
    """
    Creates an empty palette and adds it in the palettes list. 
    
    @type name: string
    @param name: name of the new palette
    
    @rtype: APalette
    @return: the newly created palette
    """
    pass
  
  def groupObjects(self, objects):
    """
    Creates a multi object containing objects in parameters.
    
    @type objects: list of AObject
    @param objects: object to put in a group
    
    @rtype: AObject
    @return: the newly created multi object
    """
    pass
    
  def linkWindows(self, windows, group=None):
    """
    Links windows in a group. Moving cursor position in a window moves it in all linked windows. 
    By default all windows are in the same group.
    
    @type windows: list of AWindow
    @param windows : the windows to link
    @type group: AWindowsGroup
    @param group: put the windows in this group. If it is None, a new group is created.
    
    @rtype: AWindowsGroup
    @return: windows's group
    """
    if windows != []:
      windows = self.makeList( windows )
      self.execute("LinkWindows", windows = windows, group = group)
      if group is None:
        group=windows[0].group
    return group
  
  ###############################################################################
  # objects access    
  def getPalette(self, name):
    """
    @rtype: APalette
    @return: the named palette
    """
    pass
  
  # informations that can be obtained with GetInfo command
  def getObjects(self):
    """
    Gets all objects referenced in current context.
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    pass
 
  def importObjects(self, top_level_only=False):
    """
    Gets objects importing those that are not referenced in current context. 
    @type top_level_only: bool
    @param top_level_only: if True imports only top-level objects (that have no parents), else all objects are imported. 
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    pass

  def getObject(self, filename):
    """
    Get the object corresponding to this filename if it is currently loaded.
    
    @type filename: string
    @param filename: filename of the requested object
    
    @rtype: AObject
    @return: the object if it is loaded, else returns None.
    """
    objects=self.getObjects()
    loadedObject=None
    for o in objects:
      if o.filename == filename and not o.copy:
        loadedObject=o
        break
    return loadedObject

  def getWindows(self):
    """
    Gets all windows referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    pass
  
  def importWindows(self):
    """
    Gets all windows importing those that are not referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    pass

  def getReferentials(self):
    """
    Gets all referentials in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    pass
  
  def importReferentials(self):
    """
    Gets all referentials importing those that are not referenced in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    pass

  def getTransformations(self):
    """
    Gets all transformations.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    pass

  def getTransformations(self):
    """
    Gets all transformations importing those that are not referenced in current context.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    pass

  def getPalettes(self):
    """
    @rtype: list of APalette
    @return: list of palettes. 
    """
    pass
      
  def getSelection(self, group=None):
    """
    @type group: AWindowsGroup
    @param group: get the selection in this group. If None, returns the selection in default group.
    
    @rtype:  list of AObject
    @return: the list of selected objects in the group of windows
    """
    pass
  
  def getDefaultWindowsGroup(self):
    return self.AWindowsGroup(self, 0)
  
  def linkCursorLastClickedPosition(self, ref=None):
    """
    Gives the last clicked position of the cursor. 
    
    @type ref: Referential
    @param ref: if given, cursor position value will be in this referential. Else, anatomist central referential is used.

    @rtype: float vector, size 3
    @return: last position of the cursor
    """
    pass
  
  def getWindowTypes(self):
    """
    Gets all existing window types.
    @rtype: list of string
    @return: list of existing window types. For example : axial, sagittal, 3D...
    """
    pass
  
  def getFusionMethods(self):
    """
    Gets all existing fusion methods.
    @rtype: list of string
    @return: list of existing fusion methods. For example: Fusion2DMethod...
    """
    pass
  
  ###############################################################################
  # objects manipulation
  def showObject(self, object):
    self.execute("ShowObject", object=object)
  
  def addObjects(self, objects, windows, add_children=False,
    add_graph_nodes=False, add_graph_relations=False):
    """
    Adds objects in windows.
    The objects and windows must already exist.
    
    @type objects : list of AObject
    @param objects : list of objects to add
    @type windows : list of AWindow
    @param windows : list of windows in which the objects must be added
    """
    self.execute("AddObject", objects=self.makeList(objects),
      windows=self.makeList(windows), add_children=int(add_children),
      add_graph_nodes=int(add_graph_nodes),
      add_graph_relations=int(add_graph_relations))
  
  def removeObjects(self, objects, windows):
    """
    Removes objects from windows. 
    
    @type objects : list of AObject
    @param objects : list of objects to remove
    @type windows : list of AWindow
    @param windows : list of windows from which the objects must be removed
    """
    self.execute("RemoveObject", objects=self.makeList(objects),
      windows=self.makeList(windows))
    
  def deleteObjects(self, objects):
    """
    Deletes objects
    @type objects: list of AObject
    @param objects: objects to delete
    """
    objects=self.makeList(objects)
    for o in objects:
      o.releaseRef()
    #self.execute("DeleteObject", objects=objects)

  def deleteElements(self, elements):
    """
    Deletes objects, windows, referentials, anything that is referenced in anatomist application.
    @type elements: list of AItem
    @param elements: elements to delete
    """
    self.execute("DeleteElement", elements=self.makeList(elements))

  def reloadObjects(self, objects):
   """
   Reload objects already in memory reading their files.
   """
   self.execute("ReloadObject", objects=self.makeList(objects))
    
  def assignReferential(self, referential, elements):
    """
    Assign a referential to objects and/or windows.
    The referential must exist. To create a new Referential, execute createReferential, 
    to assign the central referential, first get it with Anatomist.centralRef attribute.
   
    @type referential: Referential
    @param referential: The referential to assign to objects and/or windows
    @type elements: list of AObject / AWindow
    @param elements: objects or windows which referential must be changed
    The corresponding command tree contains an attribute central_ref to indicate if the referential to assign is anatomist central ref, 
    because this referential isn't referenced by an id. In the socket implementation, Referential object must have an attribute central_ref, 
    in order to create the command message. In direct impl, it is possible to access directly to the central ref object.
    """
    objects=[]
    windows=[]
    # in anatomist command, objects and windows must be passed in two lists
    for e in self.makeList(elements):
      if issubclass(e.__class__, Anatomist.AObject):
        objects.append(e)
      elif issubclass(e.__class__, Anatomist.AWindow):
        windows.append(e)
    self.execute("AssignReferential", ref_id = referential, objects=objects,
      windows=windows, central_ref=referential.centralRef)
    
  def camera(self, windows, zoom=None, observer_position=None, view_quaternion=None, slice_quaternion=None, force_redraw=False, cursor_position=None, boundingbox_min=None, boundingbox_max=None):
    """
    Sets the point of view, zoom, cursor position for 3D windows.

    @type windows: list of AWindow
    @param windows: the windows which options must be changed
    @type zoom: float
    @param zoom: zoom factor, default is 1
    @type observer_position: float vector, size 3
    @param observer_position: camera position
    @type view_quaternion: float vector, size 4, normed
    @param view_quaternion: view rotation
    @type slice_quaternion: float vector, size 4, normed 
    @param slice_quaternion: slice plan rotation
    @type force_redraw: boolean
    @param force_redraw: if true refresh printing immediatly, default is False
    @type cursor_position: float vector
    @param cursor_position: linked cursor position
    @type boundingbox_min: float vector
    @param boundingbox_min: bounding box min values
    @type boundingbox_max: float vector
    @param boundingbox_max: bounding box max values
    """
    if force_redraw:
      force_redraw = 1
    else: force_redraw = 0
    self.execute("Camera", windows=self.makeList(windows), zoom = zoom, observer_position = observer_position, view_quaternion = view_quaternion, slice_quaternion = slice_quaternion, force_redraw = force_redraw, cursor_position = cursor_position, boundingbox_min = boundingbox_min, boundingbox_max = boundingbox_max)
 
  def setWindowsControl(self, windows, control):
    """
    Changes the selected button in windows menu. 
    Examples of controls : 'PaintControl', 'NodeSelectionControl', 'Default 3D Control', 'Selection 3D', 'Flight Control', 'ObliqueControl', 'TransformationControl', 'CutControl', 'Browser Selection', 'RoiControl'...
    """
    self.execute("SetControl", windows=self.makeList(windows), control=control)
 
  def closeWindows(self, windows):
    """
    Closes windows.
    @type windows: list of AWindow
    @param windows: windows to be closed
    """
    windows=self.makeList(windows)
    for w in windows:
      w.releaseRef()
    #self.execute("CloseWindow", windows=windows)
  
  def setMaterial(self, objects, material=None, refresh=None, ambient=None,
    diffuse=None, emission=None, specular=None, shininess=None, lighting=None,
    smooth_shading=None, polygon_filtering=None, depth_buffer=None,
    face_culling=None, polygon_mode=None, unlit_color=None, line_width=None,
    ghost=None ):
    """
    Changes objects material properties. 
    
    @type objects: list of AObject
    @param objects: objects whose material must be changed
    @type material: Material
    @param material: material characteristics, including render properties.
    The material may be specified as a Material object, or as its various
    properties (ambient, diffuse, etc.). If both a material parameter and
    other properties are specified, the material is used as a base, and
    properties are used to modify it
    @type refresh: bool
    @param refresh: if true, force windows refreshing
    """
    if material is not None:
      if ambient is None:
        ambient = material.ambient
      if diffuse is None:
        diffuse = material.diffuse
      if emission is None:
        emission = material.emission
      if specular is None:
        specular = material.specular
      if shininess is None:
        shininess = material.shininess
      if lighting is None:
        lighting = material.lighting
      if smooth_shading is None:
        smooth_shading = material.smooth_shading
      if polygon_filtering is None:
        polygon_filtering = material.polygon_filtering
      if depth_buffer is None:
        depth_buffer = material.depth_buffer
      if face_culling is None:
        face_culling = material.face_culling
      if polygon_mode is None:
        polygon_mode = material.polygon_mode
      if unlit_color is None:
        unlit_color = material.unlit_color
      if line_width is None:
        line_width = material.line_width
      if ghost is None:
        ghost = material.ghost
    self.execute("SetMaterial", objects=self.makeList(objects), ambient=ambient, diffuse=diffuse, emission=emission, specular=specular, shininess=shininess, refresh=refresh, lighting=lighting, smooth_shading=smooth_shading, polygon_filtering=polygon_filtering, depth_buffer=depth_buffer, face_culling=face_culling, polygon_mode=polygon_mode, unlit_color=unlit_color, line_width=line_width, ghost=ghost)

  def setObjectPalette(self, objects, palette, minVal=None, maxVal=None, palette2=None,  minVal2=None, maxVal2=None, mixMethod=None, linMixFactor=None, palette1Dmapping=None, absoluteMode=False):
    """
    Assign a palette to objects
    @type objects: list of AObject
    @param objects: affect palette parameters to these objects
    @type palette: APalette
    @param palette: principal palette to apply
    @type minVal: float (0 - 1)
    @param minVal: palette value to affect to objects texture min value (proportionally to palette's limits)
    @type maxVal: float (0 - 1)
    @param maxVal: palette value to affect to objects texture max value
    @type palette2: APalette
    @param palette2: second palette, for 2D textures
    @type minVal2: float (0 - 1) 
    @param minVal2: second palette value to affect to object texture second component min value
    @type maxVal2: float (0 - 1)
    @param maxVal2: second palette value to affect to object texture second component max value
    @type mixMethod: string
    @param mixMethod: method to mix two palettes in a 2D palette : linear or geometric
    @type linMixFactor: float
    @param linMixFactor: mix factor for the linear method
    @type palette1Dmapping: string
    @param palette1Dmapping: way of using 2D palette for 1D texture : FirstLine or Diagonal
    @param absoluteMode: if True, min/max values are supposed to be absolute values (in regard to objects texture) rather than proportions
    """
    self.execute('SetObjectPalette', objects = self.makeList(objects), palette = palette, palette2 = palette2, min=minVal, max=maxVal, min2=minVal2, max2=maxVal2, mixMethod=mixMethod, linMixFactor=linMixFactor, palette1Dmapping=palette1Dmapping, absoluteMode=int(absoluteMode))
  
  ###############################################################################
  # application control
  def createControlWindow(self):
    """
    Creates anatomist main window. Currently it is done automatically.
    """
    self.execute('CreateControlWindow')

  def close(self):
    """
    Exits Anatomist application.
    if anatomist is closed, the singleton instance is deleted.
    So next time the constructor is called, a new instance will be created.
    """
    try:
      delattr(self.__class__, "_singleton_instance")
      self.execute('Exit')
    except:  # may fail if it is already closed
      pass
    
  def setGraphParams(self, display_mode=None, label_attribute=None, save_only_modified=None, saving_mode=None, selection_color=None, selection_color_inverse=None, set_base_directory=None, show_tooltips=None, use_nomenclature=None):
    """
    Modifies graphs and selections options.
    
    @type display_mode: string
    @param display_mode: paint mode of objects in graph nodes : mesh, bucket, all, first
    @type label_attribute: string
    @param label_attribute: selects the attribute used as selection filter: label or name
    @type save_only_modified: int (0/1)
    @param save_only_modified: if enable, graph's save saves not all sub objects but only those that have been modified.
    @type saving_mode: string
    @param saving_mode: graph'saving mode : unchanged (keep the reading format), global (1 file for all same category sub-objects), or local (1 file per sub-object)
    @type selection_color: int vector
    @param selection_color: selected objects' color : R G B [A [NA]] (A opacity, NA: 0/1 use object's opacity parameter)
    @type selection_color_inverse: int (0/1) 
    @param selection_color_inverse: selection inverses color instead of using selection_color
    @type set_base_directory: int (0/1)
    @param set_base_directory: save subobjects in a directory <graph name>.data
    @type show_tooltips: int (0/1)
    @param show_tooltips: show graph nodes' names in tooltips
    @type use_nomenclature: int (0/1)
    @param use_nomenclature: enables graph colorisation with nomenclature
    """
    self.execute('GraphParams', display_mode=display_mode, label_attribute=label_attribute, save_only_modified=save_only_modified, saving_mode=saving_mode, selection_color=selection_color, selection_color_inverse=selection_color_inverse, set_base_directory=set_base_directory, show_tooltips=show_tooltips, use_nomenclature=use_nomenclature)
  
  ###############################################################################
  # commands sending
  def execute( self, command, **kwargs ):
    """
    Execute a command in anatomist application.
    Parameters are converted before sending the request to anatomist application.
    
    @type command: string 
    @param command: name of the command to execute. Any command that can be processed by anatomist command processor. 
    Commands list is in http://merlin/~appli/doc/anatomist-3.1/doxygen/index.html.
    @type kwargs: dictionary
    @param kwargs: parameters for the command
    """
    params=dict( (k,self.convertParamsToIDs(v)) for k, v in kwargs.iteritems() if v is not None )
    self.logCommand(command, **params )
    self.send(command, **params)
  
  def logCommand(self, command, **kwargs):
    pass
  
  def logEvent(self, event, params):
    pass


  def makeList( thing ):
    """
    Transforms the argument into a list: a list with one element if it is
    not a sequence, or return the input sequence if it is already one
    """
    if operator.isSequenceType( thing ):
      try:
        if thing.__module__.startswith( 'anatomist.' ):
          return [ thing ]
      except:
        pass
      return thing
    return [ thing ]
  makeList = staticmethod( makeList )

  def convertSingleObjectParamsToIDs( self, item ):
    """
    Converts current api object to corresponding anatomist object representation.

    @type params: Anatomist.AItem instance
    @param params: element to convert

    @rtype: dictionary or list
    @return: converted elements
    """
    if isinstance( item, Anatomist.AItem ) :
      return item.getInternalRep()
    elif isinstance( item, ( basestring, int, float, dict ) ):
      return item
    raise TypeError( 'Expecting an Anatomist object but got one of type %s' % repr( type( item ) )  )


  def convertParamsToIDs( self, params ):
    """
    Converts current api objects to corresponding anatomist object representation.
    This method must be called before sending a command to anatomist application on command parameters.

    @type params: dictionary or list
    @param params: elements to convert

    @rtype: dictionary or list
    @return: converted elements
    """
    if not isinstance( params, basestring ) \
      and operator.isSequenceType( params ):
      return [self.convertSingleObjectParamsToIDs(i) for i in params]
    else:
      return self.convertSingleObjectParamsToIDs( params )

  def send( self, command, **kwargs ):
      """
      Sends a command to anatomist application. Call this method if there's no answer to get.
      This method depends on the mean of communication with anatomist. Must be redefined in implementation api.
      
      @type command: string 
      @param command: name of the command to execute. Any command that can be processed by anatomist command processor. 
      Commands list is in http://merlin/~appli/doc/anatomist-3.1/doxygen/index.html.
      @type kwargs: dictionary
      @param kwargs: parameters for the command
      """
      pass
      
  def newItemRep(self):
      """
      Creates a new item representation.
      This method depends on the mean of communication with anatomist. Must be redefined in implementation api.
      """
      pass

  def waitEndProcessing(self):
    """
    Wait for anatomist finishing current processing.
    """
    pass
  ############################################################################### 
  # logs

  def  log( self, message ):
    """
    Use this method to print a log message. 
    This method prints on standard output. To be redefined for another type of log.
    """
    print message
    
  ############################################################################### 
  class AItem(object):
    """
    Base class for representing an object in Anatomist application. 
    
    @type anatomistinstance: Anatomist
    @ivar anatomistinstance: reference to Anatomist object which created this object.
    Usefull because some methods defined in AItem objects will need to send a command to Anatomist application.
    @type internalRep: object
    @ivar internalRep: representation of this object in anatomist application.
    @type ref: bool
    @ivar ref: indicates if a reference has been taken on the corresponding anatomist object. If True, the reference is released on deleting this item.
    @type refType: string
    @ivar refType: type of reference taken on the object : Weak (reference counter not incremented), WeakShared (reference counter incremented but the object can be deleted even if it remains references) or Strong (reference counter is incremented, the object cannot be deleted since there is references on it)
    """
    def __init__( self, anatomistinstance, internalRep=None, refType=None, *args, **kwargs ):
      """
      @type anatomistinstance: Anatomist
      @param anatomistinstance: reference to Anatomist object which created this object.
      @type internalRep: object
      @param internalRep: representation of this object in anatomist application. 
      @type refType: string
      @param refType: type of reference taken on the object : Weak (reference counter not incremented), WeakShared (reference counter incrementerd but the object can be deleted even if it remains references) or Strong (reference counter is incremented, the object cannot be deleted since there is references on it). If it is not specified, Anatomist.defaultRefType is used.
      """
      super(Anatomist.AItem, self).__init__(*args, **kwargs)
      self.anatomistinstance = anatomistinstance
      self.refType=refType
      if internalRep is None:
        internalRep=anatomistinstance.newItemRep()
      self.internalRep = internalRep
      self.ref=False
          
    def __repr__(self):
      """
      String representation of the object.
      """
      return str(self.internalRep)
    
    def __cmp__(self, other):
      """
      Called on comparison operations between self and other.
      Their internalRep is compared. 
      @rtype: int
      @return: -1 if self < other, 0 if self == other, 1 if self > other
      """
      if not isinstance(other, Anatomist.AItem):
        return 1 
      if self.internalRep == other.internalRep:
        return 0
      elif self.internalRep < other.internalRep:
        return -1
      else:
        return 1
      
    def getInfos(self):
      """
      Gets informations about this object.
      
      @rtype: dictionary
      @return: informations about the object (property -> value)
      """
      pass
    
    def takeRef(self):
      """
      Take a reference on this object.
      """
      #print "take ref ", self.refType, self, self.__class__
      self.ref=True
    
    def releaseRef(self):
      """
      Release a reference on this object.
      """
      #print "release ref", self, self.__class__
      self.ref=False
      
    def releaseAppRef(self):
      """
      Release anatomist application reference on this object : so object life is controled by references on it. If there is no more references on the object, it is deleted. 
      Used when an object is created by python api. It is not owned by anatomist application.
      """
      pass
    
    def getRef(self, refType):
      """
      Get a reference of type refType on this object. 
      @rtype: AItem
      @return: a copy of current object with a reference of type refType on anatomist object.
      """
      #print "get ref ", self, self.__class__
      return self.__class__(self.anatomistinstance, self.getInternalRep(), refType)
    
    def __del__(self):
      """
      Called when current object is deleted (when it is no more referenced). If a reference had been taken on anatomist corresponding object, it is released.
      """
      #print "del ", self, self.__class__
      if self.ref:
        try: # can fail if Anatomist is already closed
          self.releaseRef()
        except:
          pass
    
    def getInternalRep(self):
      """
      Returns internal representation of the object (implementation dependant).
      """
      return self.internalRep

    def makeList( self, objects ):
      return self.anatomistinstance.makeList( objects )
    
  ###############################################################################
  class AObject(AItem):
    """
    Represents an object in Anatomist application.
    
    Following informations can be obtained using ObjectInfo command :
    @type objectType: string
    @ivar objectType: object type. For example : volume, bucket, graph, texture...
    @type children: list of AObject
    @ivar children: list of objects which are children of current object (for example: nodes in a graph). Can be empty.
    @type filename: string
    @ivar filename: name of the file from which the object has been loaded. May be None.
    @type name: string 
    @ivar name: name of the object presented in Anatomist window.
    @type copy: boolean
    @param copy: True indicates that this object is a copy of another object, else it is the original object.
    @type material: Material
    @ivar material: object's material parameters
    @type referential: Referential
    @ivar referential: referential assigned to this object.
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      """
      If internal rep is given in parameter, the corresponding anatomist object already exists : take a reference on it (to prevent its deletion). 
      """
      super(Anatomist.AObject, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      if internalRep is not None:
        self.takeRef()
    
    def getWindows(self):
      """
      Gets windows that contain this object.
      
      @rtype: list of AWindow
      @return: opened windows that contain this object.
      """
      allWindows=self.anatomistinstance.importWindows()
      windows=[]
      for w in allWindows:
        objs=w.objects
        if self in objs:
          windows.append(w)
      return windows
      
    # object manipulation
    def addInWindows(self, windows):
      """
      Adds the object in windows.
      Windows must already exist.
    
      @type windows : list of AWindow
      @param windows : list of windows in which the object must be added
      """
      self.anatomistinstance.addObjects([self], windows)
    
    def removeFromWindows(self, windows):
      """
      Removes object from windows. 
      
      @type windows : list of AWindow
      @param windows : list of windows from which the object must be removed
      """
      self.anatomistinstance.removeObjects([self], windows)

    def delete(self):
      """
      Deletes object
      """
      self.anatomistinstance.deleteObjects([self])

    def assignReferential(self, referential):
      """
      Assign a referential to object.
      The referential must exist. To create a new Referential, execute createReferential, 
      to assign the central referential, first get it with Anatomist.centralRef attribute.
  
      @type referential: Referential
      @param referential: The referential to assign to object
      """
      self.anatomistinstance.assignReferential(referential, [self])
  
    def setMaterial(self, material=None, refresh=None, ambient=None,
      diffuse=None, emission=None, specular=None, shininess=None,
      lighting=None, smooth_shading=None, polygon_filtering=None,
      depth_buffer=None, face_culling=None, polygon_mode=None,
      unlit_color=None, line_width=None, ghost=None ):
      """
      Changes object material properties. 
      
      @type material: Material
      @param material: material characteristics, including render properties
      @type refresh: bool
      @param refresh: if true, force windows refreshing
      """
      self.anatomistinstance.setMaterial([self], material, refresh,
      ambient, diffuse, emission, specular, shininess, lighting,
      smooth_shading, polygon_filtering, depth_buffer, face_culling,
      polygon_mode, unlit_color, line_width, ghost)
      
    def setPalette(self, palette, minVal=None, maxVal=None, palette2=None,  minVal2=None, maxVal2=None, mixMethod=None, linMixFactor=None, palette1Dmapping=None, absoluteMode=False):
      """
      Assign a palette to object
      
      @type palette: APalette
      @param palette: principal palette to apply
      @type minVal: float
      @param minVal: palette value to affect to objects texture min value 
      @type maxVal: float
      @param maxVal: palette value to affect to objects texture max value
      @type palette2: APalette
      @param palette2: second palette, for 2D textures
      @type minVal2: float
      @param minVal2: second palette value to affect to object texture second component min value
      @type maxVal2: float
      @param maxVal2: second palette value to affect to object texture second component max value
      @type mixMethod: string
      @param mixMethod: method to mix two palettes in a 2D palette : linear or geometric
      @type linMixFactor: float
      @param linMixFactor: mix factor for the linear method
      @type palette1Dmapping: string
      @param palette1Dmapping: way of using 2D palette for 1D texture : FirstLine or Diagonal
      """
      self.anatomistinstance.setObjectPalette([self], palette, minVal, maxVal, palette2,  minVal2, maxVal2, mixMethod, linMixFactor, palette1Dmapping,
      absoluteMode=absoluteMode)
    
    def extractTexture(self, time=None):
      """
      Extract object's texture to create a new texture object.
      
      @type time: float
      @param time: for temporal objects, if this parameter is mentionned the texture will be extracted at this time. if not mentionned, 
      all times will be extracted and the texture will be a temporal object.
      In socket implementation, it is necessary to get a new id for the texture object and to pass it to the command.
      
      @rtype: AObject
      @return: the newly created texture object
      """
      pass
    
    def generateTexture(self, dimension=1):
      """
      Generates an empty texture (value 0 everywhere) for a mesh object. 
      
      @type dimension: int 
      @param dimension: texture's dimension (1 or 2)
      
      @rtype: AObject
      @return: the newly created texture object
      """
      pass
    
    def exportTexture(self, filename, time=None):
      """
      @type filename: string
      @param filename: file in which the texture must be written
      @type time: float
      @param time: for temporal objects, if this parameter is mentionned the texture will be extracted at this time. if not mentionned, 
      all times will be extracted and the texture will be a temporal object.
      """
      self.anatomistinstance.execute("ExportTexture", filename=filename, object=self, time=time)

    def save(self, filename=None):
      """
      Saves object in file.
      
      @type filename: string
      @param filename: file in which the object will be written. If not mentionned, the object is saved in the file from which it has been loaded.
      """
      self.anatomistinstance.execute("SaveObject", object=self, filename=filename)
      
    def reload(self):
      """
      Reload this object already in memory reading its file.
      """
      self.anatomistinstance.reloadObjects([self])
  
  ###############################################################################
  class AGraph(AObject):
    """
    Represents a graph.
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.AGraph, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      
    def createNode(self, name=None, syntax=None, with_bucket=None, duplicate=True ):
      """
      Creates a new node with optionally an empty bucket inside and adds it in the graph.
      
      @type name: string
      @param name: node's name. default is RoiArg.
      @type syntax: string
      @param syntax: node's syntax attribute. default is roi.
      @type with_bucket: bool
      @param with_bucket: if True, creates an empty bucket in the node and returns it with the node. default is None, so the bucket is created but not returned
      @type duplicate: bool
      @param duplicate: enables duplication of nodes with the same name attribute.
      
      @rtype: (AObject, AObject)
      @return: (the created node, the created bucket) or only the created node if with_bucket is False
      """
      pass
    
  ###############################################################################
  class AWindow(AItem):
    """
    Represents an anatomist window.
    
    @type windowType: string
    @ivar windowType: windows's type (axial, sagittal, ...)
    @type group: AWindowsGroup
    @ivar group: the group which this window belongs to.
    @type objects: list of AObject
    @ivar objects: the window contains these objects.
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      """
      If internal rep is given in parameter, the corresponding anatomist window already exists : take a reference on it (to prevent its deletion). 
      """
      super(Anatomist.AWindow, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      if internalRep is not None:
        self.takeRef()
    
    def addObjects(self, objects, add_children=False, add_graph_nodes=False,
      add_graph_relations=False):
      """
      Adds objects in window. 
      
      @type objects : list of AObject
      @param objects : list of objects to add
      """
      self.anatomistinstance.addObjects(objects, [self], add_children,
        add_graph_nodes, add_graph_relations)
      
    def removeObjects(self, objects):
      """
      Removes objects from window. 
      
      @type objects : list of AObject
      @param objects : list of objects to remove
      """
      self.anatomistinstance.removeObjects(objects, [self])

    def camera(self, zoom=None, observer_position=None, view_quaternion=None, slice_quaternion=None, force_redraw=None, cursor_position=None):
      """
      Sets the point of view, zoom, cursor position for a 3D window.
  
      @type zoom: float
      @param zoom: zoom factor, default is 1
      @type observer_position: float vector, size 3
      @param observer_position: camera position
      @type view_quaternion: float vector, size 4, normed
      @param view_quaternion: view rotation
      @type slice_quaternion: float vector, size 4, normed 
      @param slice_quaternion: slice plan rotation
      @type force_redraw: boolean
      @param force_redraw: if true refresh printing immediatly, default is 0
      @type cursor_position: float vector
      @param cursor_position: linked cursor position
      """
      self.anatomistinstance.camera([self], zoom, observer_position, view_quaternion, slice_quaternion, force_redraw, cursor_position)

    def assignReferential(self, referential):
      """
      Assign a referential to window.
      The referential must exist. To create a new Referential, execute createReferential, 
      to assign the central referential, first get it with Anatomist.centralRef attribute.
  
      @type referential: Referential
      @param referential: The referential to assign to objects and/or windows
      """
      self.anatomistinstance.assignReferential(referential, [self])

    def getReferential( self ):
      """
      Get the referential attached to the window (the coordinates system used
      for 3D positions in this window)
      """
      pass


    def moveLinkedCursor(self, position):
      """
      Changes cursor position in this window and all linked windows (same group).
      
      @type position: float vector, size 3
      @param position: cursor's new position 
      """
      self.anatomistinstance.execute("LinkedCursor", window=self, position=position)

    def showToolbox(self, show=True):
      """
      Shows or hides the toolbox frame of a window.  
      
      @type show: boolean
      @param show: if true, the window's toolbox frame is shown, else it is hidden.
      """
      if show:
        show=1
      else: show=0
      self.anatomistinstance.execute("ControlsParams", window=self, show=show)

    def setControl(self, control):
      """
      Changes the selected button in windows menu. 
      Examples of controls : 'PaintControl', 'NodeSelectionControl', 'Default 3D Control', 'Selection 3D', 'Flight Control', 'ObliqueControl', 'TransformationControl', 'CutControl', 'Browser Selection', 'RoiControl'...
      """
      self.anatomistinstance.setWindowsControl(windows=[self], control=control)

    def close(self):
      """
      Closes window.
      """
      self.anatomistinstance.closeWindows([self])

  ###############################################################################
  class AWindowsBlock(AItem):
    """
    A window containing other windows.
    
    @type nbCols: int
    @ivar nbCols: number of columns of the windows block
    """
    def __init__(self, anatomistinstance, internalRep=None, nbCols=2, *args, **kwargs):
      super(Anatomist.AWindowsBlock, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      self.nbCols=nbCols
  
  ###############################################################################
  class AWindowsGroup(AItem):
    """
    A group containing several windows which are linked. Moving cursor in one window moves it in all linked windows.
    Its internalRep is the group id (int).
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.AWindowsGroup, self).__init__(anatomistinstance, internalRep, *args, **kwargs)

    def getSelection(self):
      """
      @rtype: list of AObject
      @return: objects that are selected in this windows group
      """
      return self.anatomistinstance.getSelection(self)
    
    def isSelected(self, object):
      """
      @type object: AObject
      @param object: an object in this windows group
      
      @rtype: bool
      @return: True if the object is selected in this windows group
      """
      selectedObjects=self.getSelection()
      return (selectedObjects is not None) and (object in selectedObjects)

    def setSelection(self, objects):
      """
      Initializes selection with given objects for this windows group.
      
      @type objects: list of AObject
      @param objects: objects to select      
      """
      self.anatomistinstance.execute("Select", objects=self.makeList(objects), group=self, modifiers="set")
    
    def addToSelection(self, objects):
      """
      Adds objects to this windows group's current selection.
      
      @type objects: list of AObject
      @param objects: objects to add to selection
      """
      self.anatomistinstance.execute("Select", objects=self.makeList(objects), group=self, modifiers="add")
    
    def unSelect(self, objects):
      """
      Removes objects from this windows group's selection.
       
      @type objects: list of AObject
      @param objects: objects to unselect
      """
      self.anatomistinstance.execute("Select", unselect_objects=self.makeList(objects), group=self, modifiers="add")
      
    def toggleSelection(self):
      """
      Inverses selection in this windows group. Selected objects becomes unselected, unselected objects become selected.
      """
      self.anatomistinstance.execute("Select", objects=self.makeList(objects), group=self, modifiers="toggle")
    
    def setSelectionByNomenclature(self, nomenclature, names):
      """
      Selects objects giving their name in a nomenclature. In anatomist graphical interface, it is done by clicking on items of a nomenclature opened in a browser. 
      
      @type nomenclature: AObject
      @param nomenclature: tree with names and labels associated to nodes.
      @type names: list of string
      @param names: names of elements to select.
      """
      if names is not None and names != []: # executing the command with names = [] make errors
        snames=string.join(names)
        self.anatomistinstance.execute("SelectByNomenclature", nomenclature=nomenclature, names=snames, group=self, modifiers="set")
    
    def addToSelectionByNomenclature(self, nomenclature, names):
      """
      Adds objects to this windows group's current selection, given their name in a nomenclature.
      
      @type nomenclature: AObject
      @param nomenclature: tree with names and labels associated to nodes.
      @type names: list of string
      @param names: names of elements to add to selection.
      """
      if names is not None and names != []:
        snames=string.join(names)
        self.anatomistinstance.execute("SelectByNomenclature", nomenclature=nomenclature, names=snames, group=self, modifiers="add")

    
    def toggleSelectionByNomenclature(self, nomenclature, names):
      """
      Removes objects from this windows group's selection, given their name in a nomenclature.
       
      @type nomenclature: AObject
      @param nomenclature: tree with names and labels associated to nodes.
      @type names: list of string
      @param names: names of elements to unselect.
      """
      if names is not None and names != []:
        snames=string.join(names)
        self.anatomistinstance.execute("SelectByNomenclature", nomenclature=nomenclature, names=snames, group=self, modifiers="toggle")
      
  ###############################################################################
  class Referential(AItem):
    """
    @type refUuid: string
    @ivar refUuid: a unique id representing this referential
    Two referential are equal if they have the same uuid.
    """
    
    def __init__(self, anatomistinstance, internalRep=None, uuid=None, *args, **kwargs):
      super(Anatomist.Referential, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      if uuid is not None:
        self.refUuid=uuid
    
    def __cmp__(self, other):
      """
      Called on comparison operations between self and other.
      Their uuid is compared. 
      @rtype: int
      @return: -1 if self < other, 0 if self == other, 1 if self > other
      """
      if self.refUuid == other.refUuid:
        return 0
      elif self.refUuid < other.refUuid:
        return -1
      else:
        return 1
          
  ###############################################################################
  class APalette(AItem):
    """
    @type name: string
    @ivar name: palette's name. Must be unique, it is the palette identifier.
    """
    def __init__(self, name, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.APalette, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
      self.name=name
    
    def setColors(self, colors, color_mode="RGB"):
      """
      Modifies a palette (colors).
      
      @type palette: APalette
      @param palette: the palette which colors must be changed
      @type colors: int vector
      @param colors: color vectors
      @type colors_mode: string
      @param colors_mode: RGB or RGBA
      """
      self.anatomistinstance.execute("ChangePalette", name=self.name, colors=colors, color_mode=color_mode)
  
  ###############################################################################
  class Transformation(AItem):
    """
    This objects contains informations to convert coordinates from one referential to another. 
    rotation_matrix
    translation
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.Transformation, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
    
    def save(self, filename):
      """
      Saves transformation in file.
      
      @type filename: string
      @param filename: file in which the transformation will be written.
      """
      self.anatomistinstance.execute("SaveTransformation", filename=filename, transformation=self)

  ###############################################################################
  class Material(object):
    """
    @type ambient: float vector (0-1), size 4
    @ivar ambient: ambient light rgba color
    @type diffuse: float vector (0-1), size 4
    @ivar diffuse: diffuse light rgba color (object color)
    @type emission: float vector (0-1), size 4
    @ivar emission: rgba color of light issued from object
    @type specular: float vector (0-1), size 4
    @ivar specular: reflect light rgba color in front of the object
    @type shininess: float (0-124)
    @ivar shininess: reflect spreading value
    Render properties :
    @type lighting: int
    @ivar lighting: activates or deactivates object lighting. 
      - 0 : deactivates lighting
      - 1 : activate lighting
      - -1 : keep default parameter of the view
    @type smooth_shading: int
    @ivar smooth_shading: (0/1/-1) activates or not facets smoothing. if true polygons are not visibles on the surface, it seems to be smooth.
    @type polygon_filtering: int
    @ivar polygon_filtering: (0/1/-1) antialiasing
    @type depth_buffer: int 
    @ivar depth_buffer: (0/1/-1) put or not object in z-buffer. if not, it is possible to click through the object.
    @type face_culling: int
    @ivar face_culling: (0/1/-1) do not show facets oriented at the opposite of the camera
    @type polygon_mode: string
    @ivar polygon_mode: polygon view mode
      - normal
      - wireframe
      - outline (normal + wireframe)
      - hiddenface_wireframe (wireframe only for hidden faces)
      - ext_outlined (normal + extenal outline)
      - default (default view parameter)
    @type unlit_color: float vector (0-1), size 4
    @ivar unlit_color: color of mesh when lighting is disabled
    @type line_width: float
    @ivar line_width: line width for segments mesh
    @type ghost: int
    @ivar ghost: in ghost mode, objects are not drawn in the depth buffer
    """
   
    def __init__(self, ambient=None, diffuse=None, emission=None, shininess=None, specular=None, lighting=None, smooth_shading=None, polygon_filtering=None, depth_buffer=None, face_culling=None, polygon_mode=None, unlit_color=None, line_width=None, ghost=None):
      self.ambient=ambient
      self.diffuse=diffuse
      self.emission=emission
      self.shininess=shininess
      self.specular=specular
      # render properties
      self.lighting=lighting
      self.smooth_shading=smooth_shading
      self.polygon_filtering=polygon_filtering
      self.depth_buffer=depth_buffer
      self.face_culling=face_culling
      self.polygon_mode=polygon_mode
      self.unlit_color=unlit_color
      self.line_width=line_width
      self.ghost=ghost

    def __repr__(self):
      return "{ambient :"+ str(self.ambient)+ ", diffuse :"+ str(self.diffuse)+ ", emission :"+ str(self.emission)+ ", shininess :"+ str(self.shininess)+ ", specular :"+ str(self.specular)+ ", lighting :"+str(self.lighting)+", smooth_shading:"+str(self.smooth_shading)+", polygon_filtering :"+str(self.polygon_filtering)+", depth_buffer :"+str(self.depth_buffer)+", face_culling :"+str(self.face_culling)+", polygon_mode :"+str(self.polygon_mode)+", unlit_color :"+str(self.unlit_color)+", line_width :"+str(self.line_width)+", ghost :"+str(self.ghost)+"}"
      
