#  This software and supporting documentation are distributed by
#      Institut Federatif de Recherche 49
#      CEA/NeuroSpin, Batiment 145,
#      91191 Gif-sur-Yvette cedex
#      France
#
# This software is governed by the CeCILL-B license under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the 
# terms of the CeCILL-B license as circulated by CEA, CNRS
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
# knowledge of the CeCILL-B license and that you accept its terms.

# a test example for python plugin in Anatomist (Pyanatomist module)

"""Python menu in Anatomist control window
This module adds a menu "Python" in Anatomist control window, with a few
options dealing with python modules:
* list currnently imported python modules
* open a python shell in a threaded window
* run a python program file in Anatomist
"""

import sys, os, string, glob
if sys.modules.has_key( 'PyQt4' ):
  from PyQt4.QtCore import *
  from PyQt4.QtGui import *
  qt4 = 1
else:
  from qt import *
  qt4 = 0

import anatomist.cpp as anatomist

consoleShellRunning = False


class PyAnatomistModule( anatomist.Module ):
  def name( self ):
    return 'Python menu'
  def description( self ):
    return __doc__

a = anatomist.Anatomist()

def pyCuteShell():
  # mostly copied from PyCute executable from PyQwt
  try:
    if qVersion()[0] == '3':
      from qwt.PyCute3 import PyCute
    elif qVersion()[0] == '2':
      from qwt.PyCute2 import PyCute
    else: # Qt >= 4 (or <= 1)
      return None
  except:
    return None

  class AnaPyCuteShell( PyCute ):
    def keyPressEvent( self, e ):
      if e.state() & Qt.ControlButton and e.key() == self.eofKey:
        # don't exit the app but close the shell window
        self.close()
        del sys.modules['__main__'].__w__
      else:
        PyCute.keyPressEvent( self, e )

  # locals = .. -- make all names in __main__ visible to the PyCute shell
  # log    = .. -- save session in 'log'
  locals=sys.modules['__main__']
  locals.___w___ = AnaPyCuteShell(locals.__dict__) #, log='log')
  locals.___w___.show()
  return 1

def fixMatplotlib():
  # fix matplotlib if if is already loaded
  # print 'ipythonShell, test matplotlib'
  if 'matplotlib' in sys.modules:
    # print 'fixing matplotlib'
    try:
      from soma.gui.api import chooseMatplotlibBackend
      chooseMatplotlibBackend()
      import matplotlib.backends
      matplotlib.backends.new_figure_manager, \
      matplotlib.backends.draw_if_interactive, \
      matplotlib.backends.show = matplotlib.backends.pylab_setup()
    except Exception, e:
      print 'exception:', e
      pass

def ipythonShell():
  global consoleShellRunning
  if consoleShellRunning:
    print 'console shell is already running.'
    return 1
  try:
    import IPython
    fixMatplotlib()
    # run interpreter
    consoleShellRunning = True
    if qt4:
      ipshell = IPython.Shell.IPShell()
    else:
      ipshell = IPython.Shell.IPShellQt( [ '-qthread' ] )
    ipshell.mainloop()
    consoleShellRunning = False
    print 'shell terminated'
    return 1
  except:
    return 0

def pythonShell():
  global consoleShellRunning
  if consoleShellRunning:
    print 'console shell is already running.'
    return 1
  try:
    import code
    # try Qwt hook into Qt loop
    # (not needed with Qt 4)
    if qt4:
      iqt = None
    else:
      try:
        import Qwt5._iqt as iqt
      except:
        try:
          import Qwt4._iqt as iqt
        except:
          iqt = None
      # try completion setup
      if iqt:
        iqt.hook(True)
      else:
        print 'Warning Qwt not here, the event loop is stopped while in ' \
        'the python shell.'
    try:
      # then readline
      import readline, rlcompleter
      readline.parse_and_bind('tab: complete')
    except:
      pass
    # run interpreter
    print 'running interactive interpreter.'
    consoleShellRunning = True
    code.interact()
    consoleShellRunning = False
    print 'shell terminated'
    return 1
  except:
    return 0

def pyShell():
  #import threading
  try:
    import Tkinter
  except:
    return 0 # no Tkinter: fails
  try:
    from idlelib.PyShell import main
  except:
    try:
      from idle.PyShell import main
    except:
      return 0
  print 'PyShell imported'
  sys.argv = [ 'anatomist' ]
  #shellThread = threading.Thread( target=main )
  #shellThread.start()
  main()
  return 1

def openshell():
  if pyCuteShell():
    return # OK
  print 'PyCute not found, trying a console shell'
  global consoleShellRunning
  if consoleShellRunning:
    print 'console shell is already running.'
    return
  if ipythonShell():
    return
  if pythonShell():
    return
  if pyShell():
    return
  print 'No shell available. Sorry.'

def listmods():
  print 'python modules:'
  print
  sz = 0
  for x in anatomist.loaded_modules:
    try:
      exec( 'import ' + x )
      sz = max( sz, len( x ) )
    except:
      pass
  for x in anatomist.loaded_modules:
    try:
      s = ''
      for i in xrange( sz - len(x) ):
        s += ' '
      print ' ', x, s + ':', eval( x + '.__file__' )
      descr = eval( x + '.__doc__' )
      if descr is not None:
        print descr
      print
    except:
      pass

def loadpython():
  print 'load python file'
  file = QFileDialog.getOpenFileName( None, '*.py' )
  if file is not None:
    if qt4:
      execfile( file.toLocal8Bit().data() )
    else:
      execfile( file.latin1() )


class PythonScriptRun( anatomist.ObjectReader.LoadFunctionClass ):
  def run( filename, options ):
    print 'run:', filename, 'with options:', options
    try:
      a.theProcessor().allowExecWhileIdle( True )
      execfile( filename )
      return list(a.getObjects())[0]
    except Exception, e:
      import traceback, sys
      sys.stdout.flush()
      sys.stderr.flush()
      print >> sys.stderr, e
      traceback.print_stack()
      sys.stderr.flush()
    a.theProcessor().allowExecWhileIdle( False )
  run = staticmethod( run )
  def load( self, filename, options ):
    if not qt4:
      from soma.qt3gui import qt3thread
      res = qt3thread.QtThreadCall().push( PythonScriptRun.run, filename,
        options )
    else:
      print 'warning, running python script in an arbitrary thread'
      res = self.run( filename, options )
    return res


#class ExecutePythonCommand( anatomist.WaitCommand ):
  #def __init__( self, filename ):
    #self._filename = filename
  #def name( self ):
    #return 'ExecutePython'
  #def write( self, context ):
    #from soma import aims
    #obj = aims.Object( { '__syntax__' : 'ExecutePythonCommand' } )
    #obj[ 'filename' ] = self._filename
    #return obj
  #def doit( self ):
    #try:
      #execfile( self._filename )
    #except Exception, e:
      #import traceback, sys
      #sys.stdout.flush()
      #sys.stderr.flush()
      #print >> sys.stderr, e
      #traceback.print_stack()
      #sys.stderr.flush()


cw = a.getControlWindow()
if cw is not None:
  menu = cw.menuBar()
  if qt4:
    p = menu.addMenu( 'Python' )
    p.addAction( 'Open python shell', openshell )
    pop = p.addMenu( 'Specific python shells' )
    pcshell = pop.addAction( 'Pycute shell', pyCuteShell )
    ipshell = pop.addAction( 'Console IPython shell', ipythonShell )
    pshell = pop.addAction( 'Console standard python shell', pythonShell )
    pyshell = pop.addAction( 'PyShell', pyShell )
    p.addSeparator()
    p.addAction( 'list loaded python modules', listmods )
    p.addAction( 'run python script file...', loadpython )
    try:
      import Pycute
    except:
      pcshell.setEnabled( False )
    try:
      import IPython
      fixMatplotlib()
    except:
      ipshell.setEnabled( False )
    try:
      import Tkinter
      try:
        import idlelib.PyShell
      except:
        try:
          import idle.PyShell
        except:
          pyshell.setEnabled( False )
    except:
      pyshell.setEnabled( False )
  else:
    p = QPopupMenu()
    p.insertItem( 'Open python shell', openshell )
    pop = QPopupMenu()
    pop.insertItem( 'Pycute shell', pyCuteShell, 0, 1000 )
    pop.insertItem( 'Console IPython shell', ipythonShell, 0, 1001 )
    pop.insertItem( 'Console standard python shell', pythonShell, 0, 1002 )
    pop.insertItem( 'PyShell', pyShell, 0, 1003 )
    p.insertItem( 'Specific python shells', pop )
    p.insertSeparator()
    p.insertItem( 'list loaded python modules', listmods )
    p.insertItem( 'run python script file...', loadpython )
    menu.insertItem( 'Python', p, -1, menu.count() - 1 )
    try:
      import Pycute
    except:
      pop.setItemEnabled( 1000, False )
    try:
      import IPython
      fixMatplotlib()
    except:
      pop.setItemEnabled( 1001, False )
    try:
      import Tkinter
      try:
        import idlelib.PyShell
      except:
        try:
          import idle.PyShell
        except:
          pop.setItemEnabled( 1003, False )
    except:
      pop.setItemEnabled( 1003, False )

pm = PyAnatomistModule()
if not qt4:
  from soma.qt3gui import qt3thread
  tc = qt3thread.QtThreadCall()
  del tc
pythonscriptloader = PythonScriptRun()
anatomist.ObjectReader.registerLoader( 'py', pythonscriptloader )

