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
This module makes anatomist module given implementation thread safe.

The function C{getThreadSafeClass} enables to create a thread safe class based on a given Anatomist implementation class. It replaces all methods by a call in main thread of the same method. 
"""

import sys, new, types
if sys.modules.has_key( 'PyQt4' ):
  from soma.qt4gui.api import QtThreadCall
else:
  from soma.qt3gui.api import QtThreadCall
from soma.singleton import Singleton

def threadedModule(anatomistModule, mainThread=None): 
  """
  Adds to current module a thread safe version of given anatomist module, replacing Anatomist class by thread safe Anatomist class.
  
  @type anatomistModule: module
  @param anatomistModule: a module containing an implementation of Anatomist class
  @type mainThread: MainThreadActions
  @param mainThread: an object that enables to send tasks to the mainThread. If it is not given in parameters, an instance will be created in this function. So it must be called by the mainThread.
  """
  moduleName=anatomistModule.__name__+"_threaded"
  anatomistThreadedModule=sys.modules.get(moduleName)
  if anatomistThreadedModule is None:
    if mainThread is None:
      mainThread=QtThreadCall()
    ThreadedAnatomist=getThreadSafeClass(classObj=anatomistModule.Anatomist, mainThread=mainThread)
    anatomistThreadedModule=new.module(moduleName)
    anatomistThreadedModule.__dict__['Anatomist']=ThreadedAnatomist
    sys.modules[moduleName]=anatomistThreadedModule
  return anatomistThreadedModule
  
def getThreadSafeClass(classObj, mainThread):
  """
  Generates a thread safe class which inherits from the class given in parameters. 
  Methods are executed in the main thread to be thread safe. 
  
  @type classObj: Class
  @param classObj: the class which needs to be thread safe
  @type mainThread: QtThreadCall
  @param mainThread: an object that enables to send tasks to the main thread.
  
  @rtype: Class
  @return: The generated thread safe class
  """
  # create a new class that inherits from classObj
  threadSafeClass=new.classobj(classObj.__name__, (classObj,), {})
  # replace all methods (not builtin) by a thread safe call to the same method
  # and replace all inner class by a thread safe class
  for attName in dir(classObj): # for all attributes of this instance
      if attName[0:2] != "__" or attName == "__singleton_init__":
        # builtin methods begin with __
        # but __singleton_init__ must be called from the main thread
        att=getattr(classObj, attName)
        #att=self.__getattribute__(attName) # getattribute and __setattr__ doesn't exist in classes not derived from object
        if (type(att) == types.MethodType) and att.im_self is None: # attribute is a method and not a class method
          # replace this method by a thread safe call to this method
          newAtt=threadSafeCall(mainThread, att)
          #print "-- methode", attName, "->", newAtt
          setattr(threadSafeClass, attName, newAtt)
          #self.__setattr__(attName, newAtt)
        elif type(att) == types.TypeType: # innner class derived from object
          # replace this class with a thread safe class
          newAtt=getThreadSafeClass(att, mainThread)
          #print "** classe", attName, "->", newAtt
          #self.__setattr__(attName, newAtt)
          setattr(threadSafeClass, attName, newAtt)
  return threadSafeClass

          
def threadSafeCall(mainThread, func):
  """
  @rtype: function
  @returns: a function that sends the given function's call to the main thread
  """
  return lambda *args, **kwargs: mainThread.call(func, *args, **kwargs)
