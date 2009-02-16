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
Anatomist is runned in another process in server mode, it listens for requests on a socket. So application is driven sending commands on the socket. 

This is not the default implementation (except in Brainvisa). So, to use it you have to change default implementation before importing anatomist.api. 

  >>> import anatomist
  >>> anatomist.setDefaultImplementation(anatomist.SOCKET)
  >>> import anatomist.api as anatomist 
  >>> a=anatomist.Anatomist()

It is also possible to directely load socket implementation without changing default implementation, for example if you want to use the 2 implementations in the same application :
  >>> import anatomist.socket.api as anatomist

This implementation is thread safe so it can be used in multi-threaded applications.

Anatomist class is a Singleton as it inherits from base.Anatomist. But it redefines C{__new__} method to enable to force the creation of another instance of Anatomist. Indeed it is possible with socket implementation to start two anatomist processes that listen on two different socket ports.
To do this, use the constructor with forceNewInstance parameter :
  >>> a=anatomist.Anatomist(forceNewInstance=True)
  
"""
from anatomist import base
from soma.wip.io import Socket
from soma.qt3gui.qt3thread import QtThreadCall
from soma.html import htmlEscape

import threading, thread
import types
import string
import time
import sys, os
import distutils.spawn
import copy

from qt import QProcess, SIGNAL, qApp

class Anatomist(base.Anatomist):
  """
  Interface to communicate with an Anatomist Application sending commands on a socket.
  Anatomist is launched in server mode and listens for commands arriving on a socket.
  
  @type comm: ASocket
  @ivar comm: socket used to communicate with anatomist application.
  @type launched: bool
  @ivar launched: indicates if anatomist application has been correclty launched in server mode
  @type _requestID: int
  @ivar _requestID: counter to generates unique id for requests to anatomist application. It is used to match request and answer. 
  @type anaServerProcess: QProcess
  @ivar anaServerProcess: anatomist application process
  @type centralRef: Referential
  @ivar centralRef: anatomist's central referential. First time it is accessed, an id is affected to the central referential.
  @type mniTemplateRef : Referential
  @ivar mniTempateRef: template mni referential (used by spm)
  These two referentials and transformation between them are always loaded in anatomist.

  @type anatomistExecutable: string
  @cvar anatomistExecutable: path to anatomist executable
  @type lock: RLock
  @cvar lock : lock for thread safety when access to singleton instance
  """
  talairachMNIReferentialId = '803552a6-ac4d-491d-99f5-b938392b674b'
  anatomistExecutable = distutils.spawn.find_executable('anatomist')
  mainThread=None
  
  def __new__(cls, *args, **kwargs):
    if kwargs.get("forceNewInstance", False):
      self=object.__new__(cls)
      self.__singleton_init__(*args, **kwargs)
    else:
      self=super(Anatomist, cls).__new__(cls, *args, **kwargs)
    return self
  
  def __singleton_init__( self, *args, **kwargs):
    super(Anatomist, self).__singleton_init__(*args, **kwargs)
    self.comm = ASocket( self, "localhost", ASocket.defaultPort )
    self.launched = 0
    self._requestID = 0
    # must run Qt methods in qt thread
    if self.mainThread is None:
      self.mainThread=QtThreadCall()
    self.mainThread.call(self.runAnatomistServer, *args)
    # see all events:
    #self.execute('EventFilter', filter=None, unfilter=None, default_filtering=0)
      
  def enableListening(self, event, notifier):
    """
    Set listening of this event on. So when the event occurs, the notifier's notify method is called.
    
    @type event: string
    @param event: name of the event to listen
    @type notifier: Notifier
    @param notifier: the notifier whose notify method must be called when this event occurs
    """
    self.execute('EventFilter', filter=[event])
    # if the event occurs, notify method will be called with two parameters : the event name and a dictionary describing the event
    self.comm.addEventHandler(event, lambda data: notifier.notify(event, data))
    
  def disableListening(self, event):
    """
    Set listening of this event off.
    
    @type event: string
    @param event: name of the event to disable.
    """
    self.execute('EventFilter', unfilter=[event])
    self.comm.removeEventHandler(event)

  ###############################################################################
  # Methods inherited from base.Anatomist
    
  # objects creation
  def createWindowsBlock(self, nbCols=2):
    """
    @rtype: AWindowsBlock
    @return: a window which can contain several AWindow
    """
    return self.AWindowsBlock(anatomistinstance=self, nbCols=nbCols)
  
  def createWindow(self, wintype, geometry=None, block=None, no_decoration=False):
    """
    Creates a new window and opens it.
    @type wintype: string
    @param wintype: type of window to open ("Axial", "Sagittal", "Coronal", "3D", "Browser", "Profile")
    @type geometry: int vector
    @param geometry: position on screen and size of the new window (x, y, w, h)
    @type block: AWindowsBlock
    @param block: a block in which the new window must be added
    @type no_decoration: bool
    @param no_decoration: indicates if decorations (menus, buttons) can be painted around the view.
    
    @rtype: AWindow
    @return: the newly created window
    """
    # Create a new window
    newWindow = self.AWindow( self )
    options=None
    if no_decoration:
      options="{'__syntax__' : 'dictionary', 'no_decoration' : 1}"
    block_columns=None
    if block is not None:
      block_columns=block.nbCols
    # execute method automatically replace AItem parameters by their id 
    self.execute( "CreateWindow", type = wintype,
                  geometry = geometry,
                  res_pointer = newWindow,
                  block = block, block_columns=block_columns, 
                  options = options )
    newWindow.takeRef()
    newWindow.releaseAppRef()
    return newWindow
        
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
    newObject=self.AObject(self)
    options=None
    option_types=None
    option_hidden=None
    if restrict_object_types is not None:
      option_types="'restrict_object_types' : {'__syntax__' : 'dictionary'"
      for k, v in restrict_object_types.items():
        option_types+=","+repr(k)+" : "+repr(v)
      option_types+="}"
    if hidden:
      option_hidden="'hidden' : 1"
    if option_types or option_hidden:
      options="{'__syntax__' : 'dictionary' "
      if option_types:
        options+=", "+option_types
      if option_hidden:
        options+=", "+option_hidden
      options+="}"
    #print 'options:', options
    self.execute("LoadObject", filename = filename, name=objectName, res_pointer = newObject, options = options)
    newObject.takeRef()
    newObject.releaseAppRef()
    if duplicate:
      # the original object has been loaded hidden, duplicate it
      copyObject=self.duplicateObject(newObject)
      return copyObject
    return newObject
  
  def duplicateObject(self, source, shallowCopy=True):
    """
    Creates a copy of source object.
    
    @type source: AObject
    @param source: the object to copy.
    
    @rtype: AObject
    @return: the copy. it has a reference to its source object, so original object will not be deleted since the copy exists.
    """
    newObject=self.AObject(self)
    self.execute("DuplicateObject", source=source, res_pointer=newObject, shallow=shallowCopy)
    if newObject.loadDate is not None: # check if the duplication succeeded
      newObject.takeRef()
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
    newGraph=self.AGraph(self)
    self.execute("CreateGraph", object=object, res_pointer=newGraph, name=name,
      syntax=syntax, filename=filename)
    newGraph.takeRef()
    newGraph.releaseAppRef()
    return newGraph
  
  def loadCursor(self, filename):
    """
    Loads a cursor for 3D windows from a file. Any mesh can be loaded as cursor. 
    The loaded file is added to cursor choice list in anatomist parameters.
    
    @type filename: string
    @param filename: the file containing object data
    
    @rtype: AObject
    @return: the loaded object
    """
    newObject=self.AObject(self)
    self.execute("LoadObject", filename = filename, res_pointer = newObject, as_cursor = 1)
    newObject.takeRef()
    newObject.releaseAppRef()
    return newObject
    
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
    newObject=self.AObject(self)
    if ask_order:
      ask_order=1
    else: ask_order=0
    self.execute("FusionObjects", objects = self.makeList(objects), res_pointer = newObject, method = method, ask_order = ask_order)
    newObject.takeRef()
    newObject.releaseAppRef()
    return newObject
  
  def createReferential(self, filename=None):
    """
    This command does not exist in Anatomist because the command AssignReferential can create a new referential if needed. 
    But the way of creating a new referential depends on the connection with Anatomist, 
    so it seems to be better to encapsulate this step on another command. So referentials are treated the same as other objects.
    (LoadObject -> addAobject | createReferential -> assignReferential)
    In this implementation, creating a new referential is just reserving an id for it. The corresponding object will only be created
    when the referential is assigned to an object or a window.
    
    @type filename: string
    @param filename: name of a file (minf file, extension .referential) containing  informations about the referential : its name and uuid
    
    @rtype: Referential
    @return: the newly created referential
    """
    newRef=self.Referential(self)
    self.execute("AssignReferential", ref_id = newRef, filename=filename, central_ref=0)
    return newRef
  
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
    newTrans=self.Transformation(self)
    self.execute("LoadTransformation", origin = origin, destination = destination, filename = filename, res_pointer = newTrans)
    return newTrans
  
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
    newTrans=self.Transformation(self)
    self.execute("LoadTransformation", origin = origin, destination = destination, matrix = matrix, res_pointer = newTrans)
    return newTrans

  def createPalette(self, name):
    """
    Creates an empty palette and adds it in the palettes list. 
    
    @type name: string
    @param name: name of the new palette
    
    @rtype: APalette
    @return: the newly created palette
    """
    newPalette=self.APalette(name, self, name)
    self.execute("NewPalette", name = name)
    return newPalette
  
  def groupObjects(self, objects):
    """
    Creates a multi object containing objects in parameters.
    
    @type objects: list of AObject
    @param objects: object to put in a group
    
    @rtype: AObject
    @return: the newly created multi object
    """
    newObject=self.AObject(self)
    self.execute("GroupObjects", objects = self.makeList(objects), res_pointer = newObject)
    return newObject
    
  ###############################################################################
  # objects access

  def __getattr__(self, name):
    """
    Called when trying to access to name attribute, which is not defined. 
    Used to give a value to centralRef attribute first time it is accessed.
    """
    
    if name == "centralRef":
      self.lock.acquire()
      self.centralRef=self.Referential(anatomistinstance=self, centralRef=1)
      self.lock.release()
      self.execute("AssignReferential", ref_id = self.centralRef, central_ref=1)
      return self.centralRef
    elif name == "mniTemplateRef":
      self.lock.acquire()
      self.mniTemplateRef=self.Referential(anatomistinstance=self)
      self.lock.release()
      # ### for the moment I put this referential uuid in a constant in this class
      # ### to be replaced by a parameter mnitemplateref in the command or access to this constant from another lib
      #print "get mniTemplateRef", Anatomist.talairachMNIReferentialId, self.mniTemplateRef
      self.execute("AssignReferential", ref_id=self.mniTemplateRef, ref_uuid=Anatomist.talairachMNIReferentialId)
      return self.mniTemplateRef
    else:
      raise AttributeError
  
  def getPalette(self, name):
    """
    Returns a new APalette with name attribute = name.
    Returns None if the palette doesn't exist in Anatomist.
    @rtype: APalette
    @return: the named palette
    """
    result=self.executeAndWaitAnswer("GetInfo", palettes = 1)
    names=result.get("palettes")
    palette=None
    if name in names:
      palette=self.APalette(name, self, name)
    return palette
  
  # informations that can be obtained with GetInfo command
  def getObjects(self):
    """
    Gets all objects referenced in current context.
    Sends getInfo command.
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    result=self.executeAndWaitAnswer("GetInfo", objects = 1)
    objectsId=result.get("objects")
    objects=[]
    for oid in objectsId:
      objects.append(self.AObject(self, oid))
    return objects
 
  def importObjects(self, top_level_only=False):
    """
    Gets objects importing those that are not referenced in current context. 
    @type top_level_only: bool
    @param top_level_only: if True imports only top-level objects (that have no parents), else all objects are imported. 
    
    @rtype:  list of AObject
    @return: list of existing objects
    """
    name_objects=1
    if top_level_only:
      name_objects="top"
    else: name_objects="all"
    result=self.executeAndWaitAnswer("GetInfo", objects = 1, name_objects = name_objects)
    objectsId=result.get("objects")
    objects=[]
    for oid in objectsId:
      objects.append(self.AObject(self, oid))
    return objects
  
  def getObject(self, filename):
    """
    Get the object corresponding to this filename if it is currently loaded.
    
    @type filename: string
    @param filename: filename of the requested object
    
    @rtype: AObject
    @return: the object if it is loaded, else returns None.
    """
    infos = self.executeAndWaitAnswer("ObjectInfo", objects_filenames='"' + filename + '"', name_children=1, name_referentials=1)
    loadedObject=None
    #print "infos recues:", infos, len(infos)
    if (infos is not None) and (len(infos) == 1):
      orig = False
      for oid, oInfos in infos.items():
        if not oInfos.get( 'copy', 0 ):
          orig = True
          break
      if not orig:
        oid, oInfos = infos.items()[0]
      loadedObject=self.AObject(self, oid)
      loadedObject.objectType=oInfos.get('objectType')
      ids=oInfos.get('children')
      objects=[]
      if ids is not None:
        for i in ids:
          objects.append(self.AObject(self, i))
      loadedObject.children=objects
      loadedObject.filename=oInfos.get('filename')
    return loadedObject

  def getWindows(self):
    """
    Gets all windows referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    result=self.executeAndWaitAnswer("GetInfo", windows = 1)
    windowsId=result.get("windows")
    windows=[]
    for wid in windowsId:
      windows.append(self.AWindow(self, wid))
    return windows
 
  def importWindows(self):
    """
    Gets all windows importing those that are not referenced in current context.
    
    @rtype: list of AWindow
    @return: list of opened windows
    """
    result=self.executeAndWaitAnswer("GetInfo", windows = 1, name_windows=1)
    windowsId=result.get("windows")
    windows=[]
    for wid in windowsId:
      windows.append(self.AWindow(self, wid))
    return windows

  def getReferentials(self):
    """
    Gets all referentials in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    result=self.executeAndWaitAnswer("GetInfo", referentials = 1)
    ids=result.get("referentials")
    l=[]
    for oid in ids:
      l.append(self.Referential(self, oid))
    return l

  def importReferentials(self):
    """
    Gets all referentials importing those that are not referenced in current context.
    
    @rtype: list of Referential
    @return: list of referentials
    """
    # ###? recupere-t-on le referentiel central et si oui comment le reconnaitre
    result=self.executeAndWaitAnswer("GetInfo", referentials = 1, name_referentials = 1)
    ids=result.get("referentials")
    l=[]
    for oid in ids:
      l.append(self.Referential(self, oid))
    return l

  def getTransformations(self):
    """
    Gets all transformations.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    result=self.executeAndWaitAnswer("GetInfo", transformations = 1)
    ids=result.get("transformations")
    l=[]
    for oid in ids:
      l.append(self.Transformation(self, oid))
    return l

  def importTransformations(self):
    """
    Gets all transformations importing those that are not referenced in current context.
    
    @rtype: list of Transformation
    @return: list of transformations
    """
    result=self.executeAndWaitAnswer("GetInfo", transformations = 1, name_transformations = 1)
    ids=result.get("transformations")
    l=[]
    for oid in ids:
      l.append(self.Transformation(self, oid))
    return l

  def getPalettes(self):
    """
    @rtype: list of APalette
    @return: list of palettes. 
    """
    result=self.executeAndWaitAnswer("GetInfo", palettes = 1)
    names=result.get("palettes")
    l=[]
    for n in names:
      l.append(self.APalette(n, self, n))
    return l
 
  def getSelection(self, group=None):
    """
    @type group: AWindowsGroup
    @param group: get the selection in this group. If None, returns the selection in default group.
    
    @rtype:  list of AObject
    @return: the list of selected objects in the group of windows
    """
    result=self.executeAndWaitAnswer("GetInfo", name_objects=1, selections = 1)
    selections=result.get("selections")
    if group is None:
      group=0
    elif isinstance(group, self.AWindowsGroup):
      group=group.internalRep
    elif type(group) != int:
      raise TypeError('Incorrect parameter type : group')
    objects=selections.get(group)
    l=[]
    if objects is not None:
      for oid in objects:
        l.append(self.AObject(self, oid))
    return l
  
  def linkCursorLastClickedPosition(self, ref=None):
    """
    Gives the last clicked position of the cursor. 
    
    @type ref: Referential
    @param ref: if given, cursor position value will be in this referential. Else, anatomist central referential is used.

    @rtype: float vector, size 3
    @return: last position of the cursor
    """
    result=self.executeAndWaitAnswer("GetInfo", linkcursor_lastpos = 1, linkcursor_referential = ref)
    return result.get('linkcursor_position')
  
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
  #implementation dependant methods
  
  def newItemRep(self):
    """
    Creates a new item representation. In this implementation, generates a new non zero Id.
    """
    internalID =self.newId()
    # the id 0 seems to be reserved... So if it is the id given by anatomist (first request), 
    # it is asked a new number. 
    if internalID == 0:
      internalID = self.newId()
    return internalID

  def newId( self ):
    """
    In this implementation, anatomist objects are not accessibles. In the commands send to Anatomist,
    objects are referenced by unique identifier. Objects defined in this module encapsulate the id of the
    corresponding Anatomist object.
    
    @rtype: int
    @return: a new unused ID.
    """
    ids = self.executeAndWaitAnswer( 'NewId' ).get( 'ids' )
    if ids and len( ids ) >= 1:
      return ids[0]
    raise RuntimeError( 'Cannot generate new id for Anatomist object'  )

  def runAnatomistServer( self, *args ):
      """
      Executes Anatomist in server mode. 
      Parameters in args will be passed as anatomist command parameters.
      """
      if Anatomist.anatomistExecutable is not None:
        port = self.comm.findFreePort()
        self.anaServerProcess = QProcess()
        command = [ Anatomist.anatomistExecutable, '-s', str( port ) ]
        command += args
        for a in command:
          self.anaServerProcess.addArgument( a )
        self.anaServerProcess.connect( self.anaServerProcess, SIGNAL( 'processExited()' ), self.anaServerProcessExited )
        if not self.anaServerProcess.launch( '' ):
          raise RuntimeError(  'Anatomist could not run'  )
        self.log( "<H1>Anatomist launched</H1>")
        self.log("Command : " +htmlEscape( string.join( command ) ) )
        self.comm.initialize( )#port = port)
        self.log( "Successfull connection to Anatomist on PORT: " +str( port ) )
        self.launched = 1
    
  def anaServerProcessExited( self ):
      """
      This method is called when anatomist process exited.
      """
      logtxt = '<b>Anatomist process exited: '
      if self.anaServerProcess.normalExit():
        logtxt += '(normal exit)'
      else:
        logtxt += 'abnormal exit, code:'+ str( self.anaServerProcess.exitStatus() )
      logtxt += '</b>'
      self.log( logtxt )
      self.comm.close()
      self._requestID = 0
      self.launched=False
      try:
        delattr(self.__class__, "_singleton_instance")
      except: # may fail if it is already closed 
        pass
        
  def close( self ):
      """
      Kill current session of Anatomist.
      """
      if not self.launched:
          return
      super(Anatomist, self).close()
      if self.anaServerProcess.isRunning( ):
        self.log( 'Killing Anatomist' )
        self.anaServerProcess.kill( )
      sys.stdout.flush()
      self.comm.close()
      self._requestID = 0
      self.launched=False
  
  def send( self, command, **kwargs ):
      """
      Sends a command to anatomist application. Call this method if there's no answer to get.
      
      @type command: string 
      @param command: name of the command to execute. Any command that can be processed by anatomist command processor. 
      Commands list is in http://merlin/~appli/doc/anatomist-3.1/doxygen/index.html.
      @type kwargs: dictionary
      @param kwargs: parameters for the command
      """
      if not self.launched:
        raise RuntimeError(  'Anatomist is not running.'  )
      #print "send", command, kwargs
      cmd=self.createCommandMessage(command, **kwargs)
      #print "-- send", cmd
      self.comm.send( cmd )

  def createCommandMessage( self, command, **kwargs ):
      """
      Writes a command in the format requested by anatomist processor. 
      
      @type command: string 
      @param command: name of the command to execute. 
      @type kwargs: dictionary
      @param kwargs: parameters for the command
      
      @rtype: string 
      @return: a tree representing the command
      """
      cmd = "\n*BEGIN TREE EXECUTE\n*BEGIN TREE " + command + "\n"
      for ( name, value ) in kwargs.items():
          if value is not None:
              if type( value ) in ( types.TupleType, types.ListType ):
                  value = string.join( map( str, value ) )
              else:
                  value = str( value )
              cmd += name + " " + value + "\n"
      cmd +=  "*END\n*END\n"
      return cmd
              
  def executeAndWaitAnswer( self, command, timeout=100, **kwargs ):
    '''
    Sends a command to anatomist and returns the result. 
    
    @type command: string 
    @param command: name of the command to execute. 
    @type kwargs: dictionary
    @param kwargs: parameters for the command

    @rtype: dictionary
    @return: a dictionary describing the result of the command.
    '''
    if not self.launched:
      raise RuntimeError(  'Anatomist is not running.'  )
    args=self.convertParams(kwargs)
    requestID = self.newRequestID()
    # an id is added to the request in order to retrieve the corresponding answer among messages read on the socket
    args['request_id']=requestID
    msg=self.createCommandMessage(command, **args)
    #print "send and wait answer", command, args
    self.logCommand(command, **kwargs)
    result=self.comm.sendAndWaitAnswer(command, msg, requestID, timeout)
    if result is not None:
      del result['request_id']
    return result
  
  def newRequestID( self ):
    """
    Generates a new unique id for a request to send to anatomist.
    
    @rtype: int
    @return: a unique id
    """
    self.lock.acquire()
    self._requestID += 1
    self.lock.release()
    return str( self._requestID )
   
  def waitEndProcessing(self):
    """
    Wait for anatomist finishing current processing.
    Some commands gives no  answer so we don't know when anatomist has finished to process them. Use this method to make sure anatomist has finished all current processing. 
    It sends a simple request to anatomist and wait for the answer.
    """
    self.executeAndWaitAnswer("GetInfo")

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
    def __init__( self, anatomistinstance, internalRep=None, *args, **kwargs ):
      """
      @type anatomistinstance: Anatomist
      @param anatomistinstance: reference to Anatomist object which created this object.
      @type internalRep: object
      @param internalRep: representation of this object in anatomist application. 
      """
      super(Anatomist.AItem, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
#      self.weakRef=True
      #if internalRep is not None: # if internalRep is None, the item doesn't exist yet in anatomist, so we can't put a reference on it
        #self.takeRef()
  
    #def takeRef(self, releaseAppRef=False):
      ## add a reference on this item
      ## take a weak shared reference on the item
##      self.weakRef=False
      #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="TakeWeakSharedRef")
      #if releaseAppRef: 
        ## release application reference to enable this item to be deleted when there is no later references in python
        #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="ReleaseApplication")
        ## must release the reference on item deletion
  
    #def getWeakRef(self):
      ## copy self content but take no reference on the item
      #item=copy.copy(self)
      #item.weakRef=True
      #return item

    #def takeRef(self):
      #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="TakeWeakSharedRef")
      #super(Anatomist.AItem, self).takeRef()
    
    #def releaseRef(self):
      #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="ReleaseWeakSharedRef")
      #super(Anatomist.AItem, self).releaseRef()

    #def releaseAppRef(self):
      #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="ReleaseApplication")

    def getInfos(self, name_children=0):
      """
      Gets informations about this object.
      
      @rtype: dictionary
      @return: informations about the object (property -> value)
      """
      infos=self.anatomistinstance.executeAndWaitAnswer("ObjectInfo", objects=[self], name_children=name_children, name_referentials=1)
      if infos is not None:
        infos=infos.get(self.internalRep)
      return infos
     
    #def __del__(self):
      #if not self.weakRef:
        ## release the reference on item deletion
        #self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="ReleaseWeakSharedRef")
    def takeRef(self):
      if self.refType is None:
        self.refType=self.anatomistinstance.defaultRefType
      super(Anatomist.AItem, self).takeRef()
      if self.refType:
        self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="Take"+self.refType+"Ref")
      #print "take ref ", self.refType, self, self.internalRep, self.internalRep.__class__
      
    def releaseRef(self):
      #print "release ref ", self
      super(Anatomist.AItem, self).releaseRef()
      if self.refType:
        self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="Release"+self.refType+"Ref")
        
    def releaseAppRef(self):
      #print "release app ref ", self
      self.anatomistinstance.execute("ExternalReference", elements=[self], action_type="ReleaseApplication")
        
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
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs ):
      super(Anatomist.AObject, self).__init__(anatomistinstance, internalRep, *args, **kwargs)

    def __getattr__(self, name):
      """
      The first time an attribute of this object is requested, it is asked to anatomist application with ObjectInfo command. It returns a dictionary containing informations about objects : 
      {objectId -> {attributeName : attributeValue, ...},
      ...
      requestId -> id}
      """
      if name == "objectType":
        infos=self.getInfos()
        self.setAttributes(infos)
        return self.objectType
      elif name == "children":
        infos=self.getInfos(name_children=1)
        return self.getChildren(infos)
      elif name == "filename":
        infos=self.getInfos()
        self.setAttributes(infos)
        return self.filename
      elif name=="name":
        infos=self.getInfos()
        n=None
        if infos is not None:
          n=infos.get("name")
        return n
      elif name=="copy":
        infos=self.getInfos()
        self.setAttributes(infos)
        return self.copy
      elif name == "loadDate":
        # send objectinfo request because loadDate can change if the object is reloaded
        date=None
        infos=self.getInfos()
        if infos is not None:
            date=infos.get("loadDate")
        return date
      elif name == "material":
        mat=None
        infos=self.getInfos()
        if infos is not None:
            matParams=infos.get("material")
            mat= self.anatomistinstance.Material(**matParams)
        return mat
      elif name == "referential":
        ref=None
        infos=self.getInfos()
        if infos is not None:
            refId=infos.get("referential")
            ref= self.anatomistinstance.Referential(self.anatomistinstance, refId)
        return ref
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        raise AttributeError
    
    def setAttributes(self, infos):
      self.objectType=None
      self.filename=None
      self.name=None
      self.copy=False
      if infos is not None:
        self.objectType=infos.get('objectType')
        self.filename=infos.get('filename')
        self.name=infos.get('name')
        if infos.get('copy') == 1:
          self.copy=True
    
    def getChildren(self, infos):
      objects=[]
      if infos is not None:
        ids=infos.get('children')
        if ids is not None:
          for i in ids:
            objects.append(self.anatomistinstance.AObject(self.anatomistinstance, i))
      return objects
    
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
      tex=self.anatomistinstance.AObject(self.anatomistinstance)
      self.anatomistinstance.execute("ExtractTexture", object=self, time=time, res_pointer=tex)
      return tex
    
    def generateTexture(self, dimension=1):
      """
      Generates an empty texture (value 0 everywhere) for a mesh object. 
      
      @type dimension: int 
      @param dimension: texture's dimension (1 or 2)
      
      @rtype: AObject
      @return: the newly created texture object
      """
      tex=self.anatomistinstance.AObject(self.anatomistinstance)
      self.anatomistinstance.execute("GenerateTexture", object=self, dimension=dimension, res_pointer=tex)
      return tex
    
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
      node=self.anatomistinstance.AObject(self.anatomistinstance)
      bucket=None
      res=node
      if with_bucket is not None:
        if with_bucket:
          with_bucket=1
          bucket=self.anatomistinstance.AObject(self.anatomistinstance)
          res=(node, bucket)
        else:
          with_bucket=0
      if duplicate:
        no_duplicate=0
      else:
        no_duplicate=1
      self.anatomistinstance.execute("AddNode", graph=self, res_pointer=node, name=name, with_bucket=with_bucket, res_bucket=bucket, no_duplicate=no_duplicate)
      return res
    
  ###############################################################################
  #  base api classes implementation
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
    def __init__(self, anatomistinstance, internalRep=None, *args, **kwargs ):
      super(Anatomist.AWindow, self).__init__(anatomistinstance, internalRep, *args, **kwargs)
    
    def __getattr__(self, name):
      """
      The first time an attribute of this window is requested, it is asked to anatomist application with ObjectInfo command. It returns a dictionary containing informations about objects : 
      {objectId -> {attributeName : attributeValue, ...},
      ...
      requestId -> id}
      """
      if name == "windowType":
        self.windowType=None
        infos=self.getInfos()
        if infos is not None:
            self.windowType=infos.get("windowType")
        return self.windowType
      elif name == "group":
        group=None
        infos=self.getInfos()
        if infos is not None:
            group=self.anatomistinstance.AWindowsGroup(self.anatomistinstance, infos.get("group"))
        return group
      elif name == "objects":
        objects=[]
        infos=self.getInfos()
        if infos is not None:
          ids=infos.get("objects")
          if ids is not None:
            for i in ids:
              o=self.anatomistinstance.AObject(self.anatomistinstance, i)
              objects.append(o)
        return objects
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        raise AttributeError
    
  ###############################################################################
  class Referential(AItem, base.Anatomist.Referential):
    """
    @type refUuid: string
    @ivar refUuid: a unique id representing this referential
    Two referential are equal if they have the same uuid.
    @type centralRef: bool
    @ivar centralRef: indicates if this referential represents anatomist's central referential
    """
    def __init__(self, anatomistinstance, internalRep=None, uuid=None, centralRef=False, *args, **kwargs ):
      super(Anatomist.Referential, self).__init__(anatomistinstance, internalRep, uuid, *args, **kwargs)
      self.centralRef=centralRef
     
    def __getattr__(self, name):
      """
      """
      if name == "refUuid":
        self.refUuid=None
        infos=self.getInfos()
        if infos is not None:
            self.refUuid=infos.get("uuid")
        return self.refUuid
      else: # must raise AttributeError if it is not an existing attribute. else, error can occur on printing the object
        raise AttributeError

###############################################################################
class ASocket(Socket):
    """
    Specialized Socket to communicate with anatomist application. 
    It redefines readMessage and processMessage Socket's methods. 
    It provides a method to send a command and wait the answer. 
    
    @type eventCallbacks: dictionary
    @ivar eventCallbacks: registers methods to call on messages received on the socket (message -> callback)
    """
    
    def __init__(self, anatomistinstance, host, port=None):
      """
      @type host: string
      @param host: socket server machine (localhost if it is current machine)
      @type port: int
      @param port: port that the socket server listens
      """
      super(ASocket, self).__init__(host, port)
      self.eventCallbacks = {}
      self.anatomistinstance=anatomistinstance
  
    def readMessage(self, timeout=30 ):
      """
      Reads a message from the socket. 
      Reads two lines : header and data parts of the message.
      the header is the command name, data is the string representation of a dictionary containing the results of that command.
      
      @type timeout: int
      @param timeout: max time to wait before reading the message.
      @rtype: tuple (string, string)
      @return: the message received from the socket (header, data).
      """
      self.readLock.acquire()
      try:
        header = self.readLine( timeout )
        data = self.readLine( timeout )
      finally:
        self.readLock.release()
      return (header, data)
      
    def sendAndWaitAnswer( self, command, msg, requestID, timeout=100):
      """
      Sends a command to Anatomist application and wait for the answer (message to read on the socket).
      A request id is affected to the command and a callback function is associated to the command with request id.
      So when a message with correponding header arrives on the socket, the callback function is called and gets the results.
      
      @type command: string
      @param command: name of the command to send
      @type msg: string
      @param msg: message to send (command + parameters)
      @type requestID: int
      @param requestID: an id associated to the event to recognize a specific command
      @type timeout: int
      @param timeout: max time to wait before receiving the answer.
      """
      def _executeMe( self, lock, data, excep ):
        """
        Dynamic callback function for the request whose answer is waited.
        """
        #print "_executeMe", threading.currentThread(), lock, data, excep
        if lock.threaded: # if reading message thread is used
          lock.condition.acquire()
          try:
            lock.result = data
            lock.exception = excep
          finally:
            lock.condition.notify()
            lock.condition.release()
        else: #if QSocketNotifier is used
          try:
            lock.result = data
            lock.exception = excep
          finally:
            lock.locked = False
        #print "end _executeMe"
      class _MyLock( object ):
        """
        Dynamic object to store the result of the sended command
        """
        pass
      lock = _MyLock()
      if not self.usethread :
        # here socket reading ocurs in the current (main) thread: don't block
        lock.threaded = False
        lock.locked = True
      else:
        # here we are in a different thread from the socket reading thread
        lock.threaded = True
        lock.condition = threading.Condition()
        lock.condition.acquire()
      lock.result = None
      lock.exception = None
      # register a callback to be aware when the answer will arrive
      self.addEventHandler( command, lambda x,e: _executeMe( self, lock, x, e ), requestID=requestID )
      # send command
      #print "sendAndWaitAnswer", command, threading.currentThread()
      self.send( msg )
      if lock.threaded:
        # block current thread waiting a notify from the reading messages thread
        try:
          lock.condition.wait( timeout )
        finally:
          lock.condition.release()
          try: # if timeout occures, delete the callback
            self.delCallbacks((command, requestID))
            raise RuntimeError( 'ASocket communications timeout' )
          except:
            pass
      else:
        # wait for the answer message arriving, when the callback will be called, lock.locked will become false
        t = time.time()
        while lock.locked and time.time() - t < timeout:
          #time.sleep(1)
          #pass
          qApp.processEvents()
        if lock.locked:
            try: # if timeout occures, delete the callback, else it is deleted when the message is processed
              self.delCallbacks((command, requestID))
            except:
              pass
            raise RuntimeError( 'ASocket communications timeout' )
      if lock.exception is not None:
        raise lock.exception
      if lock.result is None:
        raise RuntimeError( 'ASocket communications timeout' )
      return lock.result
      
    def processMessage( self, msg, excep = None ):
      """
      This method is an handler to process recieved messages sent
      by Anatomist For example when a window is closed into
      Anatomist, it send a message to tell this window was closed.
      So, Anatomist can send various messages at anytime and this
      method knows what to do with thoses messages.
      """
      if msg is not None:
        header, data = msg
        if excep is not None:
          # there has been an exception: call every waiting handler
          # (those with a request_id) to unlock them
          self.lock.acquire()
          try:
            callbacks = self.eventCallbacks.items()
          finally:
            self.lock.release()
          for x,y in callbacks:
            if type( x ) is types.TupleType:
              for function in y:
                function( data, excep )
                self.delCallbacks(x)
          return
        try:
          requestID = eval( data ).get( 'request_id' )
        except:
          requestID = None
        if requestID is not None:
          header = ( header, requestID )
        callbacks=self.getCallbacks(header)
        # the callbacks are executed in another thread to prevent from blocking event processing
        thread.start_new_thread(self.executeCallbacks, (header, eval(data), callbacks, requestID))

    def executeCallbacks(self, event, params, callbacks, requestID):
      """
      When an event is received, corresponding callbacks must be called. Event content is converted in order to contain Anatomist objects instead of simple identifiers.
      """
      paramsString=str(params)
      if callbacks is not None:
        # convert params
        o=params.get('object')
        if o is not None: # get the object by identifier and create a AObject representing it
          params['object']=self.anatomistinstance.AObject(self.anatomistinstance, o)
        w=params.get('window')
        if w is not None:
          params['window'] = self.anatomistinstance.AWindow(self.anatomistinstance, w )
        ch=params.get('children') # list of AObject
        if ch is not None:
          chObj = []
          for c in ch:
            chObj.append(self.anatomistinstance.AObject(self.anatomistinstance, c ))
          params['children']=chObj
        # execute callbacks
        if requestID is None:
          for function in callbacks:
            function( params )#self.convertEventParams(data) )
        else:
          for function in callbacks:
            function( params, None )
            # if requestID is specified, the handler is temporary
            self.delCallbacks(event)
      # log received event. Done at the end because execution of Anatomist method can be done in the main thread if it is the thread safe implementation and it may block if main thread is currently waiting for an answer from anatomist. So it is done after callbacks execution which may send the waited answer.
      self.anatomistinstance.logEvent( str(event), paramsString )
  
    def addEventHandler( self, eventName, handlerFunction, requestID=None ):
      """
      Adds a callback function to handle an event. 
      (eventName, requestID) -> [handlerFunction] in eventCallbacks
      @type eventName: string
      @param eventName: In Anatomist it is generally the name of the command which causes message sending
      @type handlerFunction: function
      @param handlerFunction: the callback function to call on receiving that event
      @type requestID: int
      @param requestID: an id associated to the event to recognize a specific command
      """
      if eventName == 'Close': eventName = ''
      elif eventName: eventName =  "'" + eventName +"'"
      if requestID is not None:
        eventName = ( eventName, requestID )
      self.lock.acquire()
      try:
        callbacks = self.eventCallbacks.setdefault( eventName, [] )
        if handlerFunction not in callbacks:
          callbacks.append( handlerFunction )
      finally:
        self.lock.release()
    
    def getCallbacks(self, header):
        """
        Thread safe acces to callbacks methods registered for that header.
        @type header: string or tuple
        @param header: command name or (command name, requestID)
        """
        self.lock.acquire()
        try:
          callbacks = self.eventCallbacks.get( header, None )
        finally:
          self.lock.release()
        return callbacks 

    def delCallbacks(self, header):
        """
        Thread safe delete of callbacks for that header.
        @type header: string or tuple
        @param header: command name or (command name, requestID)
        """
        self.lock.acquire()
        try:
          del self.eventCallbacks[ header ]
        finally:
          self.lock.release()
    
    def removeEventHandler( self, eventName, handlerFunction=None, requestID=None ):
      """
      Removes handler function for a specfic event.
      @type eventName: string
      @param eventName: In Anatomist it is generally the name of the command which causes message sending
      @type handlerFunction: function
      @param handlerFunction: the callback function to call on receiving that event
      @type requestID: int
      @param requestID: an id associated to the event to recognize a specific command
      """
      if eventName == 'Close': eventName = ''
      elif eventName: eventName =  "'" + eventName +"'"
      if requestID is not None:
        eventName = ( eventName, requestID )
      self.lock.acquire()
      try:
        callbacks = self.eventCallbacks.get( eventName )
        if callbacks is not None:
          if handlerFunction is not None:
            try:
              callbacks.remove( handlerFunction )
            except ValueError:
              pass
          if not callbacks or not handlerFunction:
            del self.eventCallbacks[ eventName ]
      finally:
        self.lock.release()

    def close( self ):
      # flush callbacks and send them all an exception before closing
      excep = RuntimeError( 'Connection closed' )
      callbacks = self.eventCallbacks.items()
      for x,y in callbacks:
        if type( x ) is types.TupleType:
          for function in y:
            function( None, excep )
            self.delCallbacks(x)
      # now the regular close
      super( ASocket, self ).close()
