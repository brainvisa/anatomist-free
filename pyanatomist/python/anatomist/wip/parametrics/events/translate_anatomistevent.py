# Copyright CEA (2000-2007)
#
#  This software and supporting documentation were developed by
#      CEA/DSV/SHFJ
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

import qt
from soma import aims
from soma.functiontools import partial

class Translate_AnatomistEvent :
	
	def __init__( self, objectsCallable ):
		""" 
		Constructor of the Point3d_AnatomistEvent
		"""
		self._objectsCallable = objectsCallable
	
	def subscribe( self, control, view, attributeName ) :
		self._view = view
		
		callbackStart = partial( self.eventStart, attributeName )
		callbackMove = partial( self.eventMove, attributeName )
		callbackEnd = partial( self.eventEnd, attributeName )
		
		result = control.mouseLongEventSubscribe( \
      qt.Qt.LeftButton, qt.Qt.NoButton,
		callbackStart,
		callbackMove,
		callbackEnd,
		False )
	
	def unsubscribe( self, control, view, attributeName ) :
		
		callbackStart = partial( self.eventStart, attributeName )
		callbackMove = partial( self.eventMove, attributeName )
		callbackEnd = partial( self.eventEnd, attributeName )
		
		control.mouseLongEventUnsubscribe( \
      qt.Qt.LeftButton, qt.Qt.NoButton,
		callbackStart,
		callbackMove,
		callbackEnd,
		False )
	
	def eventStart( self, attributeName, x, y, globx, globy ) :
		self._startx = x
		self._starty = y
	
	def eventMove( self, attributeName, x, y, globx, globy ) :
		
		# Initialize values
		translation = aims.Point3df( 0 )
		
		# Get objects on wich events must be applied
		eventObjects = self._objectsCallable.__call__()
		
		# Process the translation to apply
		qaglWidget = self._view.QAGLWidget()
		qaglWidget.translateCursorPosition( self._startx - x, y - self._starty, translation )
		translation *= -1
		
		self._startx = x
		self._starty = y
		
		# Apply changes to objects
		for object in eventObjects :
			if ( hasattr( object, attributeName ) ) :
				# Set the value to the edited object
				value = getattr( object, attributeName, None)
				
				try :
					if not ( hasattr( value, '__call__' ) ) :
						setattr( object, attributeName, value + translation )
					else :
						value.__call__( translation )
				except :
					pass
	
	def eventEnd( self, attributeName, x, y, globx, globy ) :
		pass
	
	def __del__( self ) :
		print 'translate_anatomistevent --> __del__'



