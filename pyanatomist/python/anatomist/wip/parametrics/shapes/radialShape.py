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

#########################################################################
#
# Project : Pytoolboxes
# Module : parametrics
# Create date : 2006-06-14
#
# Description :
# 	This file contains RadialShape class
#
#########################################################################

from soma import aims
from anatomist.wip.parametrics.shapes import Shape

class RadialShape( Shape ):
	"""This class is used as base for radial parametrical shapes"""
	
	def __init__( self, center = aims.Point3df( 0 ), radius = 1, smooth = False, name = "Default"):
		""" 
		Constructor that creates RadialShape from a mesh or a mesh file

		@type center : aims.Point3df
		@param center: Point information concerning the center of the Shape
		Default is 'aims.Point3df( 0, 0, 0 )'
		
		@type radius : float
		@param radius: Radius information of the Shape
		Default is '1'
		
		@type smooth : int
		@param smooth: Smooth of the Shape
		"""
		
		# Initialize parent types
		Shape.__init__( self, name )
		
		# Set attribute values
		self.smooth = smooth
		self.radius = float(radius)
		self.center = center
	
	def getBoundingCenter( self ):
		""" 
		Get the bouding box center for the RadialShape

		@rtype: aims.Point3df
		@return: Get the bounding box center for the RadialShape
		"""
		return self.center
	
	def getBoundingRadius(self):
		""" 
		Gets the bounding box radius for the RadialShape
		"""  
		return self.radius
	
	def translate( self, translation ) :
		self.center += translation
	
	def scale( self, scale ) :
		self.radius *= scale



