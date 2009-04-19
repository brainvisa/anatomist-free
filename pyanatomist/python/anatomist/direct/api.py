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
This module is an implementation of general interface L{anatomist.base}.
It uses Sip bindings of C++ Anatomist api to execute and drive Anatomist application.

This is the default implementation. So, to use it you just have to import anatomist.api, it is automatically linked to this module.

  >>> import anatomist.api as anatomist 
  >>> a=anatomist.Anatomist()


Objects created via this module encapsulate sip bindings of C++ Anatomist api objects (Sip binding classes are in module anatomist.cpp). 
This implementation provides additional features  to those in the general interface because it gives access to the bound C++ objects, and potentially the whole Anatomist library is available, includind direct manipulation and modification of objects in memory using low-level operations.
But Anatomist program is loaded in current process so if it fails, the current process also fails and possibly crashes, so it is more dangerous.

This implementation needs to run qt application. 
In an interactive python shell, this can be done using ipython -qthread for example.
If the Anatomist object is created outside the main thread, you must get a thread safe version (See L{anatomist.threaded.api}). So you have to change the default implementation before importing anatomist api :

  >>> import anatomist
  >>> anatomist.setDefaultImplementation(anatomist.THREADED)
  >>> import anatomist.api as anatomist
  
"""

from anatomist import cpp
from anatomist import base
import operator
from soma import aims
import os, sys, types
if sys.modules.has_key( 'PyQt4' ):
  from PyQt4.QtCore import QString
else:
  from qt import QString

class Anatomist(base.Anatomist, cpp.Anatomist):
  """
  Interface to communicate with an Anatomist Application using direct bindings to the C++ api.
  It inherits from Anatomist class of L{anatomist.cpp} module (Sip bindings module).
  
  @type centralRef: Referential
  @ivar centralRef: anatomist's central referential (talairach acpc ref)
  @type mniTemplateRef : Referential
  @ivar mniTempateRef: template mni referential (used by spm)
  These two referentials and transformation between them are always loaded in anatomist.
  @type handlers: dictionary
  @ivar handlers: registered handlers for events. name of the event (string) -> event handler (rc_ptr_EventHandler). Handlers must be stored to enable unregistration.
  """
  
  def __singleton_init__(self, *args, **kwargs):
    super(Anatomist, self).__singleton_init__(*args, **kwargs)
    cpp.Anatomist.__init__(self, *args, **kwargs)
    self.log( "<H1>Anatomist launched</H1>" )
    self.context=cpp.CommandContext.defaultContext()
    self.handlers={}

  class AEventHandler(cpp.EventHandler):
    """
    Anatomist event handler class. When an event is received, corresponding notifier is notified and a message is logged.
    
    @type notifier: soma.notification.Notifier
    @ivar notifier: the notifier to activate when the event occurs.
    @type log: function
    @ivar log: log function to print messages.
    """
    def __init__(self, notifier, anatomistinstance):
      cpp.EventHandler.__init__(self)
      self.notifier=notifier
      self.anatomistinstance=anatomistinstance
      
    def doit(self, event):
      """
      This method is called when the event occurs.
      """
      eventName=event.eventType()
      data=event.contents() # event content is a GenericObject, it contains values associated with keys as a dictionary. But the values are GenericObject too, no python objects. So it must be converted in python.
      dataDict={}
      for k in data.keys(): # get all parameters in a dictionary, except private parameters (beginning with _)
        if k[0] != "_":
          dataDict[k]=data[k]
      params=dataDict
      # then object's or window identifiers will be replaced by corresponding objects but before log the value (objects cannot be logged, identifier can)
      self.anatomistinstance.logEvent( eventName, str(params) )
      o=params.get('object')
      if o is not None: # get the object by identifier and create a AObject representing it
        params['object']=self.anatomistinstance.AObject(self.anatomistinstance, self.anatomistinstance.context.object( o ))
      w=params.get('window')
      if w is not None:
        params['window'] = self.anatomistinstance.AWindow(self.anatomistinstance, self.anatomistinstance.context.object( w ))
      ch=params.get('children') # list of AObject
      if ch is not None:
        chObj = []
        for c in ch:
          chObj.append(self.anatomistinstance.AObject(self.anatomistinstance, self.anatomistinstance.context.object( c )))
        params['children']=chObj
      self.notifier.notify(eventName, params)
 
  def enableListening(self, event, notifier):
    """
    Set listening of this event on. So when the event occurs, the notifier's notify method is called.
    
    @type event: string
    @param event: name of the event to listen
    @type notifier: Notifier
    @param notifier: the notifier whose notify method must be called when this event occurs
    """
    self.context.evfilter.filter(event)
    handler=self.AEventHandler(notifier, self)
    self.handlers[event]=handler
    cpp.EventHandler.registerHandler(event, handler)
  
  def disableListening(self, event):
    """
    Set listening of this event off.
    
    @type event: string
    @param event: name of the event to disable.
    """
    self.context.evfilter.unfilter(event)
    cpp.EventHandler.unregisterHandler(event, self.handlers[event])
    del self.handlers[event]
    
  ###############################################################################
  # Methods inherited from base.Anatomist
   
  # objects creation
  def createWindowsBlock(self, nbCols=2):
    """
    An id is reserved for that block but the bound object isn't created. It will be created first time a window is added to the block with createWindow method.
    
    @type nbCols: int
    @param nbCols: number of columns of the windows block

    @rtype: AWindowBlock
    @return: a window which can contain several AWindow
    """
    return self.AWindowsBlock(self, nbCols)
    
  def createWindow(self, wintype, geometry=[], block=None, no_decoration=None, options=None):
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
    @type options: dictionary
    @param options: internal advanced options.
    
    @rtype: AWindow
    @return: the newly created window
    """
    if geometry is None:
      geometry=[]
    #options=None
    if no_decoration:
      if not options:
        options={'__syntax__' : 'dictionary', 'no_decoration' : 1}
      else:
        options['__syntax__'] = 'dictionary'
        options['no_decoration'] = 1

    if block is not None:
      # CreateWindowCommand(type, id, context, geometry, blockid, block, block_columns, options)
      c=cpp.CreateWindowCommand(wintype, -1, None, geometry, block.internalID, block.getInternalRep(), block.nbCols, aims.Object(options))
      self.execute(c)
      if block.internalRep is None:
        block.internalRep=c.block()
    else:
      c=cpp.CreateWindowCommand(wintype, -1, None, geometry, 0,  None, 0, aims.Object(options))
      self.execute(c)
    w=self.AWindow(self, c.createdWindow())
    w.releaseAppRef()
    return w
    
  def loadObject(self, filename, objectName="", restrict_object_types=None, forceReload=True, duplicate=False, hidden=False):
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
    
    @rtype: AObject
    @return: the loaded object
    """
    # LoadObjectCommand(filename, id, objname, ascursor, options, context)
    if not forceReload: # do not load the object if it is already loaded
      object=self.getObject(filename)
      if object is not None: # object is already loaded
        files = [ filename, filename + '.minf' ]
        for f in files:
          if os.path.exists( f ):
            if os.stat(f).st_mtime >= object.loadDate: # reload it if the file has been modified since last load
              self.reloadObjects([object])
              break
        if duplicate:
          return self.duplicateObject(object)
        if not hidden:
          # must show the object if it was hidden
          self.showObject(object)
        return object
    # if forceReload or object is not already loaded
    if duplicate:
      hidden=True
    if objectName is None: # Command constructor doesn't support None value for this parameter
      objectName=""
    options=None
    if restrict_object_types is not None or hidden:
      options={'__syntax__' : 'dictionary'}
      if restrict_object_types:
        restrict_object_types['__syntax__']='dictionary'
        options['restrict_object_types']=restrict_object_types
      if hidden:
        options['hidden']=1
    c=cpp.LoadObjectCommand(filename, -1, objectName, False, aims.Object(options))
    self.execute(c)
    o=self.AObject(self, c.loadedObject())
    o.releaseAppRef()
    if duplicate:
      # the original object has been loaded hidden, duplicate it
      copyObject=self.duplicateObject(o)
      return copyObject
    return o
  
  def duplicateObject(self, source, shallowCopy=True):
    """
    Creates a copy of source object.
    
    @type source: AObject
    @param source: the object to copy.
    
    @rtype: AObject
    @return: the copy. it has a reference to its source object, so original object will not be deleted since the copy exists.
    """
    newObjectId=self.newId()
    if shallowCopy:
      shallowCopy =1
    else: shallowCopy=0
    self.execute("DuplicateObject", source=source, res_pointer=newObjectId, shallow=shallowCopy)
    cObject= self.context.object(newObjectId)
    if cObject is not None:
      newObject=self.AObject(self, cObject)
      newObject.releaseAppRef()
      newObject.source=source
      return newObject
    return source


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
    newGraphId=self.newId()
    self.execute("CreateGraph", object=object, res_pointer=newGraphId,
      name=name, syntax=syntax, filename=filename)
    newGraph=self.AGraph(self, self.context.object(newGraphId))
    newGraph.releaseAppRef()
    return newGraph
  
  def toAObject(self, object):
    """
    Converts aims objects and numpy arrays to AObject.
    @rtype: AObject
    """
    bobject=cpp.AObjectConverter.anatomist( object )
    return self.AObject(self, bobject)
    
  def toAimsObject(self, object):
    """
    Converts AObject to aims object.
    @type object: AObject
    @param object: the object to convert
    """
    return cpp.AObjectConverter.aims(object.getInternalRep())
    
  def loadCursor(self, filename):
    """
    Loads a cursor for 3D windows from a file.
    
    @type filename: string
    @param filename: the file containing object data
    
    @rtype: AObject
    @return: the loaded object
    """
    c=cpp.LoadObjectCommand(filename, -1, "", True)
    self.execute(c)
    o=self.AObject(self, c.loadedObject())
    o.releaseAppRef()
    return o
    
  def fusionObjects(self, objects, method="", ask_order=False):
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
    if method is None:
      method=""
    bObjects=self.convertParamsToObjects(objects)
    c=cpp.FusionObjectsCommand(self.makeList(bObjects), method, -1, ask_order)
    self.execute(c)
    o=self.AObject(self, c.createdObject())
    o.releaseAppRef()
    return o
  
  def createReferential(self, filename=""):
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
    if filename is None:
      filename=""
    c=cpp.AssignReferentialCommand(None, [], [], -1, None, filename)
    self.execute(c)
    return self.Referential(self, c.ref())
  
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
    c=cpp.LoadTransformationCommand(filename, origin.getInternalRep() , destination.getInternalRep())
    self.execute(c)
    return self.Transformation(self, c.trans())
  
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
    c=cpp.LoadTransformationCommand(matrix, origin.getInternalRep(), destination.getInternalRep())
    self.execute(c)
    return self.Transformation(self, c.trans())

  def createPalette(self, name):
    """
    Creates an empty palette and adds it in the palettes list. 
    
    @type name: string
    @param name: name of the new palette
    
    @rtype: APalette
    @return: the newly created palette
    """
    c=cpp.NewPaletteCommand(name)
    self.execute(c)
    return self.getPalette(name)
  
  def groupObjects(self, objects):
    """
    Creates a multi object containing objects in parameters.

    @type objects: list of AObject
    @param objects: object to put in a group

    @rtype: AObject
    @return: the newly created multi object
    """
    bObjects=self.convertParamsToObjects(objects)
    c=cpp.GroupObjectsCommand(self.makeList(bObjects))
    self.execute(c)
    return self.AObject(self, c.groupObject())

  #############################################################################
  # objects access
  def __getattr__(self, name):
    """
    Called when trying to access to name attribute, which is not defined. 
    Used to give a value to centralRef attribute first time it is accessed.
    """
    if name == "centralRef":
      self.centralRef=self.Referential(self, self.centralReferential())
      return self.centralRef
    elif name == "mniTemplateRef":
      self.mniTemplateRef=self.Referential(self,
        cpp.Referential.mniTemplateReferential())
      return self.mniTemplateRef
    else:
      raise AttributeError

  def __getattribute__( self, name ):
    '''
    __getattribute__ is overloaded in Anatomist.direct.api:
    the aim is to intercept calls to the C++ API and convert return
    values which contain C++ instances to their corresponding wrapper in
    the Anatomist.direct implementation: AObject, AWindow, Referential,
    Transformation instances are converted.
    Drawback: all return values which are lists or dictionaries are copied.
    '''
    att = super( type(self), self ).__getattribute__( name )
    if callable( att ):
      if type( att ).__name__ == 'builtin_function_or_method':
        conv = super( type(self), self ).__getattribute__( \
          'convertParamsToAItems' )
        return lambda *args, **kwargs: conv( att( *args, **kwargs ) )
    return att

  def _getAttributeNames( self ):
    '''IPython completion feature...'''
    m = [ 'centralRef', 'mniTemplateRef' ]
    l = [ self ]
    done = set()
    while l:
      c = l.pop()
      done.add( c )
      m += filter( lambda x: not x.startswith( '_' ) and x not in m,
        c.__dict__.keys() )
      cl = getattr( c, '__bases__', None )
      if not cl:
        cl = getattr( c, '__class__', None )
        if cl is None:
          continue
        else:
          cl = [ cl ]
      l += filter( lambda x: x not in done, cl )
    return m

  def getPalette(self, name):
    """
    @rtype: APalette
    @return: the named palette
    """
    pal=self.palettes().find(name)
    if pal.isNull():
      pal=None
    else:
      pal = self.APalette(name, self, pal)
    return pal
  
  # informations that can be obtained with GetInfo command
  def getObjects(self):
    """
    Gets all objects referenced in current context.
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    boundObjects=cpp.Anatomist.getObjects(self)
    objects=[]
    for o in boundObjects:
      objects.append(self.AObject(self, o))
    return objects
 
  def importObjects(self, top_level_only=False):
    """
    Gets objects importing those that are not referenced in current context. 
    In this implementation, it is equivalent to getObject because all objects are referenced.
    
    @type top_level_only: bool
    @param top_level_only: if True imports only top-level objects (that have no parents), else all objects are imported. 
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    return self.getObjects()

  def getWindows(self):
    """
    Gets all windows referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    boundWindows=cpp.Anatomist.getWindows(self)
    windows=[]
    for w in boundWindows:
      windows.append(self.AWindow(self, w))
    return windows
  
  def importWindows(self):
    """
    Gets all windows importing those that are not referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    return self.getWindows()

  def getReferentials(self):
    """
    Gets all referentials in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    boundRefs=cpp.Anatomist.getReferentials(self)
    refs=[]
    for r in boundRefs:
      refs.append(self.Referential(self, r))
    return refs
  
  def importReferentials(self):
    """
    Gets all referentials importing those that are not referenced in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    return self.getReferentials()

  def getTransformations(self):
    """
    Gets all transformations.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    boundTrans=cpp.Anatomist.getTransformations(self)
    trans=[]
    for t in boundTrans:
      trans.append(self.Transformation(self, t))
    return trans

  def importTransformations(self):
    """
    Gets all transformations importing those that are not referenced in current context.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    return self.getTransformations()

  def getPalettes(self):
    """
    @rtype: list of APalette
    @return: list of palettes. 
    """
    paletteList = self.palettes().palettes()
    palettes=[]
    for p in paletteList:
      palettes.append(self.APalette(p.name(), self, p))
    return palettes
  
  def getSelection(self, group=None):
    """
    @type group: AWindowsGroup
    @param group: get the selection in this group. If None, returns the selection in default group.
    
    @rtype:  list of AObject
    @return: the list of selected objects in the group of windows
    """
    selections=cpp.SelectFactory.factory().selected()
    if group is None:
      group=0
    elif isinstance(group, Anatomist.AWindowsGroup):
      group=group.internalRep
    elif type(group) != int:
      raise TypeError('Incorrect parameter type : group')
    objects=selections.get(group)
    l=[]
    if objects is not None:
      for o in objects:
        l.append(self.AObject(self, o))
    return l
  
  def linkCursorLastClickedPosition(self, ref=None):
    """
    Gives the last clicked position of the cursor. 
    
    @type ref: Referential
    @param ref: if given, cursor position value will be in this referential. Else, anatomist central referential is used.

    @rtype: float vector, size 3
    @return: last position of the cursor
    """
    if ref is not None:
      return self.lastPosition(ref.internalRep)
    else: return self.lastPosition()
  
  def getWindowTypes(self):
    """
    Gets all existing window types.
    @rtype: list of string
    @return: list of existing window types. For example : axial, sagittal, 3D...
    """
    # ###? manque la methode correspondante en c++
    pass
  
  def getFusionMethods(self):
    """
    Gets all existing fusion methods.
    @rtype: list of string
    @return: list of existing fusion methods. For example: Fusion2DMethod...
    """
    # ###? manque la methode correspondante en c++
    pass
  
  ###############################################################################
  # objects manipulation
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
    bObjects=self.convertParamsToObjects(objects)
    bWindows=self.convertParamsToObjects(windows)
    c=cpp.AddObjectCommand(self.makeList(bObjects), self.makeList(bWindows),
      add_children, add_graph_nodes, add_graph_relations)
    self.execute(c)

  def removeObjects(self, objects, windows):
    """
    Removes objects from windows. 
    
    @type objects : list of AObject
    @param objects : list of objects to remove
    @type windows : list of AWindow
    @param windows : list of windows from which the objects must be removed
    """
    bObjects=self.convertParamsToObjects(objects)
    bWindows=self.convertParamsToObjects(windows)
    c=cpp.RemoveObjectCommand(self.makeList(bObjects), self.makeList(bWindows))
    self.execute(c)
      
  def assignReferential(self, referential, elements):
    """
    Assign a referential to objects and/or windows.
    The referential must exist. To create a new Referential, execute createReferential, 
    to assign the central referential, first get it with Anatomist.centralRef attribute.
   
    @type referential: Referential
    @param referential: The referential to assign to objects and/or windows
    @type elements: list of AObject / AWindow
    @param elements: objects or windows which referential must be changed
    """
    objects=[]
    windows=[]
    # in anatomist command, objects and windows must be passed in two lists
    for e in self.makeList(elements):
      if issubclass(e.__class__, self.AObject):
        objects.append(e.getInternalRep())
      elif issubclass(e.__class__, self.AWindow):
        windows.append(e.getInternalRep())
    c=cpp.AssignReferentialCommand(referential.internalRep, objects, windows)
    self.execute(c)
    
  ###############################################################################
  def execute( self, command, **kwargs ):
    """
    Sends a command to anatomist application. Params are replaced by id before sending the command.
    
    @type command: string or Command object
    @param command: name of the command to execute. Any command that can be processed by anatomist command processor. 
    Commands list is in http://merlin/~appli/doc/anatomist-3.1/doxygen/index.html.
    @type kwargs: dictionary
    @param kwargs: parameters for the command
    """

    params=dict( (k,self.convertParamsToIDs(v)) for k, v in kwargs.iteritems() if v is not None )
    self.logCommand(command, **params )
    return self.theProcessor().execute(command, **params) 
    
  def convertSingleObjectParamsToIDs( self, v ):
    """
    Converts current api object to corresponding anatomist object representation.

    @type params: Anatomist.AItem instance
    @param params: element to convert

    @rtype: dictionary or list
    @return: converted elements
    """
    if isinstance( v,  base.Anatomist.AItem ) :
      v=v.getInternalRep()
    if isinstance(v, cpp.APalette):
      return v.name()
    if isinstance(v, cpp.AObject) or isinstance(v, cpp.AWindow) or isinstance(v, cpp.Transformation) or isinstance(v, cpp.Referential):
      try:
        i = self.context.id( v )
      except:
        i = self.context.makeID( v )
      return i
    elif isinstance( v, ( basestring, int, float, dict ) ):
      return v
    raise TypeError( 'Expecting an Anatomist object but got one of type %s' % repr( type( v ) )  )


  def convertSingleObjectParamsToObjects( self, v ):
    """
    Converts current api object to corresponding anatomist C++ object representation.

    @type params: Anatomist.AItem instance
    @param params: element to convert

    @rtype: dictionary or list
    @return: converted elements
    """
    if isinstance( v,  base.Anatomist.AItem ) :
      return v.getInternalRep()
    return v


  def convertParamsToObjects( self, params ):
    """
    Converts current api objects to corresponding anatomist C++ object representation.
    This method must be called before instantiating a C++ command.

    @type params: dictionary or list
    @param params: elements to convert

    @rtype: dictionary or list
    @return: converted elements
    """
    if not isinstance( params, basestring ) \
      and operator.isSequenceType( params ):
      return [self.convertSingleObjectParamsToObjects(i) for i in params]
    else:
      return self.convertSingleObjectParamsToObjects( params )


  def getAItem( self, idorcpp, convertIDs=True, allowother=True ):
    """
    Converts a C++ API objects or context IDs to a generic API object.

    @param idorcpp: ID or C++ instance to be converted. If idorcpp is already
    an AItem, it is returned as is

    @param convertIDs: if True, int numbers are treated as item IDs and
    converted accordingly when possible.
    @type convertIDs: boolean

    @param allowother: if True, idorcpp is returned unchanged if not recognized
    @type allowother: boolean

    @rtype: AItem instance, or None (or the unchanged input if allowother is
    True)
    @return: converted element
    """
    if isinstance( idorcpp, base.Anatomist.AItem ):
      return idorcpp
    if convertIDs and type( idorcpp ) is types.IntType:
      try:
        o = self.context.object( idorcpp )
      except:
        o = None
      if o is None:
        if allowother:
          return idorcpp
        else:
          return None
      else:
        idorcpp = o
    if isinstance( idorcpp, cpp.AObject ):
      return Anatomist.AObject( self, idorcpp )
    if isinstance( idorcpp, cpp.AWindow ):
      return Anatomist.AWindow( self, idorcpp )
    if isinstance( idorcpp, cpp.Referential ):
      return Anatomist.Referential( self, idorcpp )
    if isinstance( idorcpp, cpp.Transformation ):
      return Anatomist.Transformation( self, idorcpp )
    if allowother:
      return idorcpp
    else:
      return None


  def convertParamsToAItems( self, params, convertIDs=False, changed=[] ):
    """
    Recursively converts C++ API objects or context IDs to generic API objects.

    @param params: dictionary or list or anything else

    @param convertIDs: if True, int numbers are treated as item IDs and
    converted accordingly when possible.
    @type convertIDs: boolean

    @param changed: if anything has been changed from the input params, then
    changed will be added a True value. It's actually an output parameter
    @type changed: list

    @return: converted elements
    """
    if isinstance( params, basestring ) or isinstance( params, QString ):
      return params
    elif operator.isSequenceType( params ):
      conv = super( type(self), self ).__getattribute__( \
        'convertParamsToAItems' )
      changed2 = []
      l = [ conv(i, convertIDs=convertIDs, changed=changed2) for i in params ]
      if not changed2:
        return params
      else:
        if not changed:
          changed.append( True )
        return l
    elif operator.isMappingType( params ):
      r = {}
      conv = super( type(self), self ).__getattribute__( \
        'convertParamsToAItems' )
      changed2 = []
      for k, v in params.iteritems():
        r[k] = conv( v, convertIDs=convertIDs, changed=changed2 )
      if changed2:
        if not changed:
          changed.append( True )
        return r
      else:
        return params
    else:
      try:
        conv = super( type(self), self ).__getattribute__( \
          'convertParamsToAItems' )
        changed2 = []
        l = [conv(i, convertIDs=convertIDs, changed=changed2) for i in params ]
        if changed2:
          if not changed:
            changed.append( True )
          return l
        else:
          return params
      except:
        obj = super( type(self), self ).__getattribute__( 'getAItem' )( \
          params, convertIDs=convertIDs )
        if obj is not params and not changed:
          changed.append( True )
        return obj


  def newItemRep(self):
    """
    Creates a new item representation. 
    """
    return None

  def newId( self ):
    """
    In this implementation, anatomist objects are accessibles but some commands need an id associated to the object : 
    CreateWindowCommand blockid attribute, linkWindows group attribute...
    This method generates a unique id in current context. 
    
    @rtype: int
    @return: a new unused ID.
    """
    newId=self.context.makeID(None)
    if newId == 0:
      newId=self.context.makeID(None)
    return newId
      
  ###############################################################################
  class AItem(base.Anatomist.AItem):
    """
    Base class for representing an object in Anatomist application. 
    
    @type anatomistinstance: Anatomist
    @ivar anatomistinstance: reference to Anatomist object which created this object.
    Usefull because some methods defined in AItem objects will need to send a command to Anatomist application.
    @type internalRep: object
    @ivar internalRep: representation of this object in anatomist application. 
    """
    def __init__( self, *args, **kwargs ):
      """
      @type anatomistinstance: Anatomist
      @param anatomistinstance: reference to Anatomist object which created this object.
      @type internalRep: object
      @param internalRep: representation of this object in anatomist application. 
      """
      super(Anatomist.AItem, self).__init__(*args, **kwargs)
    
    def getInfos(self):
      """
      Gets informations about this object.
      
      @rtype: dictionary
      @return: informations about the object (property -> value)
      """
      # using ObjectInfoCommand class directly doesn't work, I don't know why...
      # command=cpp.ObjectInfoCommand("", self.anatomistinstance.context, self.anatomistinstance.convertToIds([self]), True, True)
      command=self.anatomistinstance.execute("ObjectInfo", objects=[self], name_children=1, name_referentials=1)
      infosObj=command.result()
      infos=eval(str(infosObj)) # aims.Object -> python dictionary
      if infos is not None:
        infos=infos.get(self.anatomistinstance.context.id(self.internalRep))
      return infos

    def getInternalRep(self):
      ## en attendant une conversion automatique dans sip
      if (getattr(self.internalRep, "get", None)):
        return self.internalRep.get()
      return self.internalRep

    def __getattr__( self, name ):
      '''
      __getattribute__ is overloaded in Anatomist.direct.api:
      the aim is to intercept calls to the C++ API and convert return
      values which contain C++ instances to their corresponding wrapper in
      the Anatomist.direct implementation: AObject, AWindow, Referential,
      Transformation instances are converted.
      Drawback: all return values which are lists or dictionaries are copied.
      '''
      try:
        return super( base.Anatomist.AItem, self ).__getattr__( name )
      except:
        # delegate to internalRep
        gattr = super( base.Anatomist.AItem, self ).__getattribute__
        att = getattr( gattr( 'internalRep' ), name, None )
        if att is None:
          raise AttributeError( "'" + type(self).__name__ + \
            '\' object has no attribute \'' + name + "'" )
        conv = super( type(gattr( 'anatomistinstance' ) ),
          gattr( 'anatomistinstance' ) ).__getattribute__( \
            'convertParamsToAItems' )
        if callable( att ):
          return lambda *args, **kwargs: conv( att( *args, **kwargs ) )
        return conv( att )

    def _getAttributeNames( self ):
      '''IPython completion feature...'''
      m = []
      l = [ self.internalRep, self ]
      done = set()
      while l:
        c = l.pop()
        done.add( c )
        m += filter( lambda x: not x.startswith( '_' ) and x not in m,
          c.__dict__.keys() )
        cl = getattr( c, '__bases__', None )
        if not cl:
          cl = getattr( c, '__class__', None )
          if cl is None:
            continue
          else:
            cl = [ cl ]
        l += filter( lambda x: x not in done, cl )
      return m


  ###############################################################################
  class AObject(AItem, base.Anatomist.AObject):
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
    def __init__(self, *args, **kwargs):
      super(Anatomist.AObject, self).__init__(*args, **kwargs)
    
    def __getattr__(self, name):
      """
      The first time an attribute of this object is requested, it is asked to anatomist application with ObjectInfo command. It returns a dictionary containing informations about objects : 
      {objectId -> {attributeName : attributeValue, ...},
      ...
      requestId -> id}
      """
      if name == "objectType":
        self.objectType=self.internalRep.objectTypeName(self.internalRep.type())
        return self.objectType
      elif name == "children":
        objects=[]
        if issubclass(type(self.getInternalRep()), cpp.MObject): # if internalRep is a multi object, it is iterable and can have children
          for c in self.getInternalRep():
            objects.append(self.anatomistinstance.AObject(self.anatomistinstance, c))
        return objects
      elif name == "filename":
        self.filename=self.internalRep.fileName()
        return self.filename
      elif name=="name":
        return self.internalRep.name()
      elif name=="copy":
        self.copy=self.internalRep.isCopy()
        return self.copy
      elif name=="loadDate":
        return self.internalRep.loadDate()
      elif name == "material":
        matdesc=self.internalRep.GetMaterial().genericDescription()
        matParams={}
        for k, v in matdesc.items():
          matParams[k]=v
        matObj=self.anatomistinstance.Material(**matParams)
        return matObj
      elif name=="referential":
        ref=None
        iref=self.internalRep.getReferential()
        if iref is not None:
          ref=self.anatomistinstance.Referential(self.anatomistinstance, iref)
        return ref
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        #return getattr(self.internalRep, name)
        return Anatomist.AItem.__getattr__( self, name )

    def _getAttributeNames( self ):
      return [ 'objectType', 'children', 'filename', 'name', 'copy',
        'loadDate', 'material', 'referential' ] \
        + Anatomist.AItem._getAttributeNames( self )

    def __eq__( self, other ):
      return self.getInternalRep() == other.getInternalRep()

    def extractTexture(self, time=-1):
      """
      Extract object's texture to create a new texture object.
      
      @type time: float
      @param time: for temporal objects, if this parameter is mentionned the texture will be extracted at this time. if not mentionned, 
      all times will be extracted and the texture will be a temporal object.
      In socket implementation, it is necessary to get a new id for the texture object and to pass it to the command.
      
      @rtype: AObject
      @return: the newly created texture object
      """
      if time is None:
        time = -1
      c=cpp.ExtractTextureCommand( self.getInternalRep(), -1, time)
      self.anatomistinstance.execute(c)
      return self.anatomistinstance.AObject(self.anatomistinstance, c.createdObject())
    
    def generateTexture(self, dimension=1):
      """
      Generates an empty texture (value 0 everywhere) for a mesh object. 
      
      @type dimension: int 
      @param dimension: texture's dimension (1 or 2)
      
      @rtype: AObject
      @return: the newly created texture object
      """
      c=cpp.GenerateTextureCommand( self.getInternalRep(), -1, dimension)
      self.anatomistinstance.execute(c)
      return self.anatomistinstance.AObject(self.anatomistinstance, c.createdObject())
    
    def setChanged(self):
      self.internalRep.setChanged()
      
    def notifyObservers(self):
      self.internalRep.notifyObservers()
      
    def toAimsObject(self):
      """
      Converts AObject to aims object.
      """
      return cpp.AObjectConverter.aims(self.getInternalRep())

    def takeRef(self):
      if self.refType is None:
        self.refType=self.anatomistinstance.defaultRefType
      super(Anatomist.AObject, self).takeRef()
      if self.refType == "Weak":
        self.internalRep=cpp.weak_ptr_AObject(self.internalRep)
      elif self.refType == "WeakShared":
        self.internalRep=cpp.weak_shared_ptr_AObject(self.internalRep)
      #print "take ref ", self.refType, self, self.internalRep, self.internalRep.__class__
      
    def releaseRef(self):
      #print "release ref ", self
      super(Anatomist.AObject, self).releaseRef()
      del self.internalRep
        
    def releaseAppRef(self):
      #print "release app ref ", self
      self.anatomistinstance.releaseObject(self.getInternalRep())
     
    #def __del__(self):
      #print "del AObject ", self
      #super(Anatomist.AObject, self).__del__()
      
  ###############################################################################
  class AGraph(AObject, base.Anatomist.AGraph):
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
      nodeId=self.anatomistinstance.newId()
      bucketId=None
      if with_bucket is not None:
        if with_bucket:
          with_bucket=1
          bucketId=self.anatomistinstance.newId()
        else:
          with_bucket=0
      if duplicate:
        no_duplicate=0
      else:
        no_duplicate=1
      self.anatomistinstance.execute("AddNode", graph=self, res_pointer=nodeId, name=name, with_bucket=with_bucket, res_bucket=bucketId, no_duplicate=no_duplicate)
      node=self.anatomistinstance.AObject(self.anatomistinstance, self.context.object(nodeId))
      if bucketId is not None:
        bucket=self.anatomistinstance.AObject(self.anatomistinstance, self.context.object(bucketId))
        res=(node, bucket)
      else:
        res=node
      return res
  
  ###############################################################################
  class AWindow(AItem, base.Anatomist.AWindow):
    """
    Represents an anatomist window.
    
    @type windowType: string
    @ivar windowType: windows's type (axial, sagittal, ...)
    @type group: AWindowsGroup
    @ivar group: the group which this window belongs to.
    @type objects: list of AObject
    @ivar objects: the window contains these objects.
    """
    def __init__(self, *args, **kwargs):
      super(Anatomist.AWindow, self).__init__(*args, **kwargs)
     
    def __getattr__(self, name):
      """
      The first time an attribute of this window is requested, it is asked to anatomist application with ObjectInfo command. It returns a dictionary containing informations about objects : 
      {objectId -> {attributeName : attributeValue, ...},
      ...
      requestId -> id}
      """
      if name == "windowType":
        self.windowType=self.internalRep.subtype()
        return self.windowType
      elif name == "group": # window group can change so it is not saved in an attribute
        return self.anatomistinstance.AWindowsGroup(self.anatomistinstance, self.internalRep.Group())
      elif name == "objects":
        objs=self.internalRep.Objects()
        aobjs=[]
        for obj in objs:
          aobjs.append(self.anatomistinstance.AObject(self.anatomistinstance, obj))
        return aobjs
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        return Anatomist.AItem.__getattr__( self, name )

    def _getAttributeNames( self ):
      return [ 'windowType', 'group', 'objects' ] + \
        Anatomist.AItem._getAttributeNames( self )

    def takeRef(self):
      if self.refType is None:
        self.refType=self.anatomistinstance.defaultRefType
      super(Anatomist.AWindow, self).takeRef()
      if self.refType == "Weak":
        self.internalRep=cpp.weak_ptr_AWindow(self.internalRep)
      elif self.refType == "WeakShared":
        self.internalRep=cpp.weak_shared_ptr_AWindow(self.internalRep)
      #print "take ref ", self.refType, self, self.internalRep, self.internalRep.__class__

    def releaseRef(self):
      #print "release ref ", self
      super(Anatomist.AWindow, self).releaseRef()
      del self.internalRep

    def releaseAppRef(self):
      #print "release app ref ", self
      self.anatomistinstance.releaseWindow(self.getInternalRep())

    #def __del__(self):
      #print "del AWindow ", self
      #super(Anatomist.AWindow, self).__del__()

    def getReferential( self ):
      ref = self.internalRep.getReferential()
      if ref is not None:
        return self.anatomistinstance.Referential( self.anatomistinstance,
          ref )
      return ref

  ###############################################################################
  class AWindowsBlock(AItem, base.Anatomist.AWindowsBlock):
    """
    A window containing other windows.
    """
    def __init__(self, anatomistinstance=None, nbCols=2):
      super(Anatomist.AWindowsBlock, self).__init__(anatomistinstance, nbCols=nbCols)
      self.internalID=anatomistinstance.newId()
  
  ###############################################################################
  class AWindowsGroup(AItem, base.Anatomist.AWindowsGroup):
    """
    A group containing several windows which are linked. Moving cursor in one window moves it in all linked windows.
    
    @type initialGroup: bool
    @ivar initialGroup: True if current group is the initial group in which all windows are by default.
    """
    def __init__(self, anatomistinstance, groupid=None):
      if groupid is None:
        groupid=anatomistinstance.newId()
      super(Anatomist.AWindowsGroup, self).__init__(anatomistinstance, groupid)
  
  ###############################################################################
  class Referential(AItem, base.Anatomist.Referential):
    """
    @type refUuid: string
    @ivar refUuid: a unique id representing this referential
    Two referential are equal if they have the same uuid.
    """
    def __init__(self, anatomistinstance, internalRep=None, uuid=None):
      super(Anatomist.Referential, self).__init__(anatomistinstance, internalRep, uuid)
     
    def __getattr__(self, name):
      """
      """
      if name == "refUuid":
        self.refUuid=self.internalRep.uuid()
        return self.refUuid
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        return Anatomist.AItem.__getattr__( self, name )

    def _getAttributeNames( self ):
      return [ 'refUuid' ] + Anatomist.AItem._getAttributeNames( self )

  ###############################################################################
  class APalette(AItem, base.Anatomist.APalette):
    """
    @type name: string
    @ivar name: palette's name. Must be unique, it is the palette identifier.
    """
    def __init__(self, name, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.APalette, self).__init__(name, anatomistinstance, internalRep, *args, **kwargs)
  
  ###############################################################################
  class Transformation(AItem, base.Anatomist.Transformation):
    """
    This objects contains informations to convert coordinates from one referential to another. 
    rotation_matrix
    translation
    """
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs):
      super(Anatomist.Transformation, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
