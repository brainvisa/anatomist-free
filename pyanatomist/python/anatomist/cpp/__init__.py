#
#  Copyright (C) 2004-2006 CEA
#
#  This software and supporting documentation were developed by
#       CEA/DSV/SHFJ
#       4 place du General Leclerc
#       91401 Orsay cedex
#       France


'''
Low level module containing Sip bindings of Anatomist C++ API. 

Introduction
============

This module is mostly bindings to the C++ library of Anatomist. A few classes
have been slightly modified or rewritten, either to hide garbage or to
appear more pythonic. 

For typical use, this module will not be called directly but throught general API. But it can be needed for using advanced features.

The main entry point is the L{Anatomist} class which must be instantiated before any operation can be performed.

  >>> import anatomist.cpp as anatomist
  >>> a = anatomist.Anatomist()

Note that instantiating Anatomist also instantiates the
U{Qt<http://trolltech.com>} QApplication, but does not run the Qt event loop,
so python interactive shells are still accessible after that operation, but
the GUI is frozen until the event loop is executed, using the following PyQt
code:

  >>> import qt
  >>> qt.qApp.exec_loop()

but then the loop does not return until the GUI application is over, so you
should start it at the end of your code.

Another comfortable alternative to the GUI loop problem in interactive
python shells is to use U{IPython<http://ipython.scipy.org/>} with the
C{-qthread} option: IPython is an interactive python shell with many
improvements, and which can run in a different thread than the GUI, so that
the GUI can run without hanging the shell.

Contrarily to the Qt application, Anatomist can be instantiated multiple
times (it is not a singleton, but contains a singleton).

In addition to providing bindings to the Anatomist C++ API, the anatomist
module also loads some python modules in anatomist: every python module
found in the following locations are loaded:
  - C{os.path.join( str( anatomist.Anatomist().anatomistSharedPath() ),
                    'python_plugins' )}
  - C{os.path.join( str( anatomist.Anatomist().anatomistHomePath() ),
                    'python_plugins' )}

Main concepts
=============
There are many pieces in Anatomist library, but a few are really needed to
begin with.

  - the Anatomist application, the L{Anatomist} class instance, is responsible
    for many global variables, the application state, and the startup
    procedure (including plugins loading). The application is used internally
    in many functions.
  - objects: L{AObject} class and subclasses
  - windows: L{AWindow} class and subclasses
  - the commands processor: a singleton L{Processor} instance obtained via
    the application: L{Anatomist.theProcessor}(). The processor is a commands
    interpreter for the rudimentary language of Anatomist. It is also
    responsible of saving every command executed in the history file
    (C{$HOME/.anatomist/history.ana}) so most of the session can be replayed.
    Many operations are done via commands and the processor: creating windows,
    loading objects, displaying them, etc. so this element is probably the
    most important part of Anatomist.
  - conversions between AIMS object and Anatomist objects: L{AObjectConverter}
    is here to perform this task.
'''

import os, sys, string, glob, operator
import numpy

path = os.path.dirname( __file__ )
if path not in sys.path:
  sys.path.insert( 0, path )

# add path for python modules lib
if sys.platform[:3] == 'win':
  sep = ';'
else:
  sep = ':'

# PYTHONPATH. On python 2.3, it doesn't seem to be taken into account
# when pyhton is run from a library
path = os.getenv( 'PYTHONPATH' )
if path is not None:
  for x in string.split( path, sep ):
    if x not in sys.path:
      sys.path.append( x )
  del x
del path, sep

from soma import aims
from anatomistsip import *
from soma.importer import ExtendedImporter

# cleanup namespaces in Sip-generated code
ExtendedImporter().importInModule( '', globals(), locals(), 'anatomistsip' )
ExtendedImporter().importInModule( '', globals(), locals(), 'anatomistsip',
  ['anatomistsip.anatomist'] )
del ExtendedImporter

aims.__fixsipclasses__( locals().items() )

loaded_modules = []
global _anatomist_modsloaded
_anatomist_modsloaded = 0

# update AObjectConverter class to be more flexible
def aimsFromAnatomist( ao, options={} ):
  def scaleTexture( tex, ao, options ):
    try:
      if int( options.get( 'scale', 1 ) ):
        print 'scale texture'
        txex = ao.glAPI().glTexExtrema(0)
        tmin = txex.minquant[0]
        scl = ( txex.maxquant[0] - tmin ) * ( txex.max[0] - txex.min[0] )
        tmin -= txex.min[0] * scl
        print scl, tmin
        print tex.__class__
        ntex = tex.__class__( tex )
        print ntex
        for t in xrange( ntex.size() ):
          try:
            ar = ntex[t].arraydata()
            print 'ar OK for time', t
            ar *= scl
            ar += tmin
          except:
            pass
        return ntex
      else:
        if int( options.get( 'always_copy', 0 ) ):
          return tex.__class__( tex.get().__class__( tex.get() ) )
    except:
      return tex
  tn = ao.objectTypeName( ao.type() )
  if tn == 'VOLUME':
    aim = AObjectConverter.aimsData_U8( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_S16( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_U16( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_S32( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_U32( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_FLOAT( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_DOUBLE( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_RGB( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsData_RGBA( ao )
    if aim:
      return aim
  elif tn == 'SURFACE':
    aim = AObjectConverter.aimsSurface3( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsSurface4( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsSurface2( ao )
    if aim:
      return aim
  elif tn == 'BUCKET':
    aim = AObjectConverter.aimsBucketMap_VOID( ao )
    if aim:
      return aim
  elif tn == 'TEXTURE':
    aim = AObjectConverter.aimsTexture_FLOAT( ao )
    if aim:
      return scaleTexture( aim, ao, options )
    aim = AObjectConverter.aimsTexture_POINT2DF( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsTexture_S16( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsTexture_S32( ao )
    if aim:
      return aim
    aim = AObjectConverter.aimsTexture_U32( ao )
    if aim:
      return aim
  elif tn == 'GRAPH':
    aim = AObjectConverter.aimsGraph( ao )
    if aim:
      return aim
  return None

AObjectConverter.aims = staticmethod( aimsFromAnatomist )
del aimsFromAnatomist

def anatomistFromAims( obj ):
  ot = type( obj ).__name__
  if ot.startswith( 'Volume_' ):
    dt = ot[ 7: ]
    return AObjectConverter._anatomist( eval( 'aims.AimsData_' + dt \
                                              + '( aims.rc_ptr_Volume_' + dt \
                                              + '( obj ) )' ) )
  elif ot.startswith( 'rc_ptr_Volume_' ):
    dt = ot[ 14: ]
    return AObjectConverter._anatomist( eval( 'aims.AimsData_' + dt \
                                              + '( obj )' ) )
  elif isinstance( obj, AObject ):
    return obj
  t = None
  if isinstance( obj, numpy.ndarray ):
    t = None
    if obj.dtype is numpy.dtype( numpy.int8 ):
      t = aims.Volume_S8
    elif obj.dtype is numpy.dtype( numpy.uint8 ):
      t = aims.Volume_U8
    elif obj.dtype is numpy.dtype( numpy.int16 ):
      t = aims.Volume_S16
    elif obj.dtype is numpy.dtype( numpy.uint16 ):
      t = aims.Volume_U16
    elif obj.dtype is numpy.dtype( numpy.int_ ) \
      or obj.dtype is numpy.dtype( numpy.int32 ):
      t = aims.Volume_S32
    elif obj.dtype is numpy.dtype( numpy.uint32 ):
      t = aims.Volume_U32
    elif obj.dtype is numpy.dtype( numpy.float32 ):
      t = aims.Volume_FLOAT
    elif obj.dtype is numpy.dtype( numpy.float64 ) \
      or obj.dtype is numpy.dtype( numpy.float_ ):
      t = aims.Volume_DOUBLE
  if t:
    return AObjectConverter.anatomist( t( obj ) )
  return AObjectConverter._anatomist( obj )
AObjectConverter._anatomist = AObjectConverter.anatomist
AObjectConverter.anatomist = staticmethod( anatomistFromAims )
del anatomistFromAims


# Anatomist class: entry point to anatomist

class Anatomist( AnatomistSip ):
  '''
  Anatomist class: entry point to anatomist
  '''

  def __init__( self, *args ):
    import anatomistsip, os, sys, glob, traceback
    global _anatomist_modsloaded
    modsloaded = _anatomist_modsloaded
    _anatomist_modsloaded = 1
    AnatomistSip.__init__( self, ( 'anatomist', ) + args )
    if modsloaded:
      return

    pythonmodules = os.path.join( str( self.anatomistSharedPath() ),
                                      'python_plugins' )
    homemodules = os.path.join( str( self.anatomistHomePath() ), 
                                'python_plugins' )

    print 'global modules:', pythonmodules
    print 'home   modules:', homemodules

    if sys.path[0] != homemodules:
      sys.path.insert( 0, homemodules )
    if sys.path[1] != pythonmodules:
      sys.path.insert( 1, pythonmodules )

    mods = glob.glob( os.path.join( pythonmodules, '*' ) ) \
           + glob.glob( os.path.join( homemodules, '*' ) )
    # print 'modules:', mods

    global loaded_modules

    for x in mods:
      if x[-4:] == '.pyo':
        x = x[:-4]
      elif x[-4:] == '.pyc':
        x = x[:-4]
      elif x[-3:] == '.py':
        x = x[:-3]
      elif not os.path.isdir( x ):
        continue
      x = os.path.basename( x )
      if x in loaded_modules:
        continue
      loaded_modules.append( x )
      print 'loading module', x
      try:
        exec( 'import ' + x )
      except:
        print
        print 'loading of module', x, 'failed:'
        exceptionInfo = sys.exc_info()
        e, v, t = exceptionInfo
        tb = traceback.extract_tb( t )
        try:
          name = e.__name__
        except:
          name = str( e )
        print name, ':', v
        print 'traceback:'
        for file, line ,function, text in tb:
          if text is None:
            text = '?'
          print file, '(', line, ') in', function, ':'
          print text
        print
        # must explicitely delete reference to frame objects (traceback) else it creates a reference cycle and the object cannot be deleted
        del e, v, t, tb, exceptionInfo

    print 'all python modules loaded'

# Processor.execute

def newexecute( self, *args, **kwargs ):
  '''
  Commands execution. It can be used in several different forms:
    - execute( Command )
    - execute( commandname, params = None, context = None )
    - execute( commandname, context = None, **kwargs )

  @param Command: command to be executed
  @type Command: L{Command} subclass
  @param commandname: name of the command to executed
  @type commandname: string
  @param params: optional parameters (default: None)
  @type params: dictionary or string
  @param context: command execution context (default: None)
  @type context: L{CommandContext}
  @param kwargs: keyword arguments which are passed to Anatomist directly
    in the command
  @return: the executed command (or None)
  '''
  def replace_dict( dic, cc ):
    for k,v in dic.items():
      if isinstance( v, AObject ) or isinstance( v, AWindow ) \
        or isinstance( v, Referential ) or isinstance( v, Transformation ):
        try:
          i = cc.id( v )
        except:
          i = cc.makeID( v )
        dic[k] = i
      elif operator.isMappingType( v ):
        replace_dict( v, cc )
      elif not type(v) is str and operator.isSequenceType( v ):
        replace_list( v, cc )

  def replace_list( l, cc ):
    k = 0
    for v in l:
      if isinstance( v, AObject ) or isinstance( v, AWindow )\
        or isinstance( v, Referential ) or isinstance( v, Transformation ):
        try:
          i = cc.id( v )
        except:
          i = cc.makeID( v )
        l[k] = i
      elif operator.isMappingType( v ):
        replace_dict( v, cc )
      elif not type(v) is str and operator.isSequenceType( v ):
        replace_list( v, cc )
      k += 1

  if len( args ) < 1 or len( args ) > 3:
    raise RunTimeError( 'wrong arguments number' )
  if type( args[0] ) is not str:
    if len( args ) != 1:
      raise RunTimeError( 'wrong arguments number' )
    return self._execute( args[0] )

  dic = {}
  cc = None
  if len( kwargs ) != 0:
    if len( args ) > 2:
      raise RunTimeError( 'wrong arguments number' )
    kw = 0
    if len( args ) == 2:
      cc = args[1]
    else:
      cc = kwargs.get( 'context' )
      if cc is not None:
        kw = 1
    dic = kwargs.copy()
    if kw:
      del dic[ 'context' ]
  elif len( args ) >= 2:
    dic = args[1]
    if len( args ) == 3:
      cc = args[2]
    elif len( args ) > 3:
      raise RunTimeError( 'wrong arguments number' )
  if not cc:
    cc = CommandContext.defaultContext()
  replace_dict( dic, cc )
  return self._execute( args[0], str( dic ), cc )

Processor.execute = newexecute
del newexecute

# automatically wrap creator functions in controls system
class PyKeyActionLink( Control.KeyActionLink ):
  def __init__( self, method ):
    Control.KeyActionLink.__init__( self )
    self._method = method
  def execute( self ):
    self._method()
  def clone( self ):
    return PyKeyActionLink( self._method )

class PyMouseActionLink( Control.MouseActionLink ):
  def __init__( self, method ):
    Control.MouseActionLink.__init__( self )
    self._method = method
  def execute( self, x, y, globalX, globalY ):
    self._method( x, y, globalX, globalY )
  def clone( self ):
    return PyMouseActionLink( self._method )

class PyWheelActionLink( Control.WheelActionLink ):
  def __init__( self, method ):
    Control.WheelActionLink.__init__( self )
    self._method = method
  def execute( self, delta, x, y, globalX, globalY ):
    self._method( delta, x, y, globalX, globalY )
  def clone( self ):
    return PyWheelActionLink( self._method )

class PySelectionChangedActionLink( Control.SelectionChangedActionLink ):
  def __init__( self, method ):
    Control.SelectionChangedActionLink.__init__( self )
    self._method = method
  def execute( self ):
    self._method()
  def clone( self ):
    return PySelectionChangedActionLink( self._method )

class PyActionCreator( ActionDictionary.ActionCreatorBase ):
  def __init__( self, function ):
    ActionDictionary.ActionCreatorBase.__init__( self )
    self._function = function
  def __call__( self ):
    return self._function()

class PyControlCreator( ControlDictionary.ControlCreatorBase ):
  def __init__( self, function ):
    ControlDictionary.ControlCreatorBase.__init__( self )
    self._function = function
  def __call__( self ):
    return self._function()

ControlDictionary.addControl = \
                             lambda self, name, creator, prio: \
                             self._addControl( name,
                                               PyControlCreator( creator ),
                                               prio )
ActionDictionary.addAction = \
                           lambda self, name, creator: \
                           self._addAction( name, PyActionCreator( creator ) )

# create lambda subscribe functions
Control.keyPressEventSubscribe = lambda self, key, state, func: \
                                 self._keyPressEventSubscribe( \
  key, state, PyKeyActionLink( func ) )
Control.mousePressButtonEventSubscribe = lambda self, but, state, func: \
  self._mousePressButtonEventSubscribe( but, state,
                                        PyMouseActionLink( func ) )
Control.mouseReleaseButtonEventSubscribe = lambda self, but, state, func: \
  self._mouseReleaseButtonEventSubscribe( but, state,
                                          PyMouseActionLink( func ) )
Control.mouseMoveEventSubscribe = lambda self, but, state, func: \
                                  self._mouseMoveEventSubscribe( \
  but, state, PyMouseActionLink( func ) )
Control.mouseLongEventSubscribe = lambda self, but, state, startfunc, \
                                  longfunc, endfunc, exclusive: \
                                  self._mouseLongEventSubscribe( \
  but, state, PyMouseActionLink( startfunc ),
  PyMouseActionLink( longfunc ), PyMouseActionLink( endfunc ), exclusive )
Control.wheelEventSubscribe = lambda self, func: \
                                  self._wheelEventSubscribe(
                                  PyWheelActionLink( func ) )
Control.selectionChangedEventSubscribe = lambda self, func: \
                                  self._selectionChangedEventSubscribe(
                                  PySelectionChangedActionLink( func ) )

# create lambda unsubscribe functions
Control.keyPressEventUnsubscribe = lambda self, key, state, func: \
                                 self._keyPressEventUnsubscribe( \
  key, state, PyKeyActionLink( func ) )
Control.mousePressButtonEventUnsubscribe = lambda self, but, state, func: \
  self._mousePressButtonEventUnsubscribe( but, state,
                                        PyMouseActionLink( func ) )
Control.mouseReleaseButtonEventUnsubscribe = lambda self, but, state, func: \
  self._mouseReleaseButtonEventUnsubscribe( but, state,
                                          PyMouseActionLink( func ) )
Control.mouseMoveEventUnsubscribe = lambda self, but, state, func: \
                                  self._mouseMoveEventUnsubscribe( \
  but, state, PyMouseActionLink( func ) )
Control.mouseLongEventUnsubscribe = lambda self, but, state, startfunc, \
                                  longfunc, endfunc, exclusive: \
                                  self._mouseLongEventUnsubscribe( \
  but, state )
Control.wheelEventUnsubscribe = lambda self, func: \
                                  self._wheelEventUnsubscribe(
                                  PyWheelActionLink( func ) )
Control.selectionChangedEventUnsubscribe = lambda self, func: \
                                  self._selectionChangedEventUnsubscribe(
                                  PySelectionChangedActionLink( func ) )

aims.convertersObjectToPython.update( { \
  'AObject' : AObject.fromObject,
  'PN9anatomist7AObjectE' : AObject.fromObject,
  'N5carto10shared_ptrIN9anatomist7AObjectEEE' : AObject.fromObject,
} )


# Import external python modules
import mobject
# delete things from other modules

del os, sys, string, glob
del anatomist # , aims

# -------------
# docs

Command.__doc__ = '''
Commands are the execution units of the L{Processor}.

Commands are used as subclasses of C{Command}. They can be built either on the
fly by the programmer, or using the commands factory via the
L{Processor.execute}() function.

Never call C{Command.execute}() or C{Command.doit}() directly: only the
processor will do so. Once built, a command must always be executed by the
processor:

  >>> a = anatomist.Anatomist()
  >>> c = anatomist.CreateWindowCommand( 'Axial' )
  >>> a.theProcessor().execute( c )

Comamnds can be subclassed in Python. To be valid, a new command must define
at least the L{name}() and L{doit}() methods. L{doit} will actually do the
execution and your program may make it do anything. Later, C{read}() and
C{write} should also be redefined to allow proper IO for this command (when
the commands IO and factory are exported from C++ to python).

L{Command.doit}() receives no parameters (apart from C{self}). All execution
arguments must be set in the command itself upon construction (either by the
constructor or by setting some instance variables).

One important parameter which a command would often use is the
L{CommandContext}, which specifies some IO streams for output printing and
communication with external programs, and an identifiers set used to name and
identify objects (in a general meaning: including windows etc.) through this
IO strams.
'''

AObjectConverter.__doc__ = '''
Aims / Anatomist objects converter

Only two static methods are useful in this class:
  - L{AObjectConverter.anatomist}() converts an AIMS (or possibly other) object
    into an Anatomist object (L{AObject} subclass), if possible
  - L{AObjectConverter.aims}() converts an Anatomist object to an AIMS object,
    if possible

Conversions are generally done by wrapping or embedding, or by extracting a
reference to an internal object, so objects data are genrally shared between
the AIMS and the Anatomist objects layers. This allows internal modification
of Anatomist objects data.

After a modification through the Aims (lower level) layer API, modification
notification must be issued to the Anatomist layer to update the display of
the object in Anatomist. This is generally done via the L{AObject.setChanged}()
and L{AObject.notifyObservers}() methods.
'''

private = [ 'private', 'anatomistsip', 'AnatomistSip', 'numpy', 'operator',
  'mobject' ]
__all__ = []
for x in dir():
  if not x.startswith( '_' ) and x not in private:
    __all__.append(x)
del x, private

