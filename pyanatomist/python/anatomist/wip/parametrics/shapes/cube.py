# Copyright CEA and IFR 49 (2000-2007)
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

#########################################################################
#
# Project : Pytoolboxes
# Module : parametrics.shapes
# Create date : 2006-06-14
#
# Description :
# 	This file contains Cube class
#
#########################################################################

import operator
from soma import aims
import anatomist.cpp as anatomist

from soma.signature.api \
  import Signature, HasSignature, Unicode, Boolean, Number
from soma.wip.aims.signature                      import Point3d
from anatomist.wip.operations                     import lists
from anatomist.wip.parametrics.shapes.radialShape import RadialShape

class Cube( HasSignature, RadialShape ):
	
	signature = Signature(
	'name', Unicode(),
	'center', Point3d(),
	'radius', Number(0),
	'smooth', Boolean()
	)
	
	def __init__( self, center = aims.Point3df( 0 ), radius = 1, smooth = False, name = "Cube" ):
		""" 
		Constructor of the ACube object
		
                @type center : aims.Point3df
		@param center: Point information concerning the center of the Shape
		
		@type radius : float
		@param radius: Radius of the Shape
		
		@type smooth : float
		@param smooth: Smooth of the Shape
		
		"""
		HasSignature.__init__( self )
		RadialShape.__init__( self, center, radius, smooth, name )
		
		# Register events
		self.onAttributeChange( 'name', self._nameChanged )
		self.onAttributeChange( 'center', self._visualizationChanged )
		self.onAttributeChange( 'radius', self._visualizationChanged )
		self.onAttributeChange( 'smooth', self._visualizationChanged )
		
		self.resetVisualization( )
	
	def resetVisualization( self ) :
		"""
		Reset the mesh for the current parametric Shape.
		""" 
		self.mesh = aims.SurfaceGenerator.cube( self.center, self.radius, self.smooth )
		convertedMesh = anatomist.AObjectConverter.anatomist( self.mesh )
		self.eraseAll()
		self.insert( convertedMesh )
	
	def isContained( self, point ):
		""" 
                True if the point is contained in the object, False otherwise
		
                @type point : aims.Point3df
		@param point: Point information to test if it is contained by the object
		"""    
		
		# Process the min point and max point of the cube
		minPoint = self.center - type(self.center)(self.radius)
		maxPoint = self.center + type(self.center)(self.radius)
		
		return ( lists.compare( minPoint, point, operator.le) and lists.compare( point, maxPoint, operator.le) )
	
	def subscribeShapeEvents( self, pool ) :
		pass




