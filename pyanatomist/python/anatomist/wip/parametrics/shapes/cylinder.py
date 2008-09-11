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
# Create date : 2006-06-23
#
# Description :
# 	This file contains Cylinder class
#
#########################################################################

import math
from soma import aims
import anatomist.cpp as anatomist

from soma.signature.api \
  import Signature, HasSignature, Unicode, Boolean, Number, Integer
from soma.wip.aims.signature                import Point3d
from anatomist.wip.operations               import aobjects
from anatomist.wip.parametrics.shapes.shape import Shape

class Cylinder( HasSignature, Shape ):
	
	signature = Signature(
	'name', Unicode(),
	'point1', Point3d(),
	'point2', Point3d(),
	'radius1', Number(0),
	'radius2', Number(0),
	'facets', Integer(),
	'closed', Boolean(),
	'smooth', Boolean()
	)
	
	def __init__( self, point1 = aims.Point3df( 0 ), point2 = aims.Point3df( 1 ), radius1 = 1, radius2 = 1, facets = 20, closed = False, smooth = False, name = "Cylinder"):
		""" 
		"""
		
		# Initialize parent types
		HasSignature.__init__( self )
		Shape.__init__( self, name )
		
		# Set attribute values
		self.point1 = point1
		self.point2 = point2
		self.radius1 = radius1
		self.radius2 = radius2
		self.facets = facets
		self.closed = closed
		self.smooth = smooth
		
		# Register events
		self.onAttributeChange( 'name', self._nameChanged )
		self.onAttributeChange( 'point1', self._visualizationChanged )
		self.onAttributeChange( 'point2', self._visualizationChanged )
		self.onAttributeChange( 'radius1', self._visualizationChanged )
		self.onAttributeChange( 'radius2', self._visualizationChanged )
		self.onAttributeChange( 'facets', self._visualizationChanged )
		self.onAttributeChange( 'closed', self._visualizationChanged )
		
		self.resetVisualization( )
	
	def resetVisualization( self ) :
		"""
		Reset the mesh for the current parametric Shape.
		""" 
		self.mesh = aims.SurfaceGenerator.cylinder( self.point1, self.point2, self.radius1, self.radius2, self.facets, self.closed, self.smooth )
		convertedMesh = anatomist.AObjectConverter.anatomist( self.mesh )
		self.eraseAll()
		self.insert( convertedMesh )
	
	def translate( self, translation ) :
		self.point1 += translation
		self.point2 += translation
	
	def scale( self, scale ) :
		self.radius1 *= scale
		self.radius2 *= scale
		
		growth = (self.point2 - self.point1) * ( scale - 1 ) / 2
		self.point1 -= growth
		self.point2 += growth
	
	def isContained( self, point ):
		""" 
                True if the point is contained in the object, False otherwise
                                
		@type point : aims.Point3df
		@param point: Point information to test if it is contained by the object
		"""
		return aobjects.isPointContainedInCylinder( point, self.point1, self.point2, self.radius1, self.radius2 )
	
	def getBoundingCenter( self ):
		""" 
		Get the bouding box center for the RadialShape
                		
                @rtype: aims.Point3df
		@return: Get the bounding box center for the RadialShape
		"""
		return self.point1 + ( self.point2 - self.point1 ) / 2
	
	def getBoundingRadius( self ):
		""" 
		Gets the bounding box radius for the RadialShape
		"""
		P1C = ( self.point2 - self.point1 ) / 2
		
		radius1 = math.sqrt( self.radius1 ** 2 + ( P1C.norm() ) ** 2 )
		radius2 = math.sqrt( self.radius2 ** 2 + ( P1C.norm() ) ** 2 )
		return max( radius1, radius2 )



