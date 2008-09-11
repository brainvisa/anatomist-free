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
# Module : parametrics.shapes
# Create date : 2006-06-23
#
# Description :
# 	This file contains Ellipsoid class
#
#########################################################################

from soma import aims
import anatomist.cpp as anatomist

from soma.signature.api import Signature, HasSignature, Unicode, Number
from soma.aimsalgo      import BucketMapSampler_FLOAT_3, Polynomial_FLOAT_3
from soma.wip.aims.signature                import Point3d
from anatomist.wip.parametrics.shapes.shape import Shape

class Ellipsoid( HasSignature, Shape ):
	signature = Signature(
	'name', Unicode(),
	'center', Point3d(),
	'axis1', Number(),
	'axis2', Number(),
	'axis3', Number(),
	'resolution', Point3d()
	)
	
	def __init__( self, center = aims.Point3df(0), axis1 = 1, axis2 = 1, axis3 = 1, resolution = aims.Point3df(1), name = "Ellipsoid"):
		""" 
		Constructor of the Ellipsoid object
                
		@type center : aims.Point3df
		@param center: the center of the ellipsoid
		
		@type axis1 : aims.Point3df
		@param axis1: the first axis of the ellipsoid
		
		@type axis2 : aims.Point3df
		@param axis2: the second axis of the ellipsoid
		
		@type axis3 : aims.Point3df
		@param axis3: the third axis of the ellipsoid
		"""
		
		# Initialize parent types
		HasSignature.__init__( self )
		Shape.__init__( self, name )

		self.bucketMap = aims.BucketMap_VOID()
		anatomistBucketMap = anatomist.AObjectConverter.anatomist( aims.BucketMap_VOID() )
		self.viewedBucketMap = anatomist.AObjectConverter.aims( anatomistBucketMap )
		self.insert( anatomistBucketMap )
		
		self.resetShape( center, axis1, axis2, axis3, resolution )
		self.resetVizualisation( )
		
		# Register events
		#self.onAttributeChange( 'name', self._nameChanged )
		#self.onAttributeChange( 'center', self._visualizationChanged )
		#self.onAttributeChange( 'axis1', self._visualizationChanged )
		#self.onAttributeChange( 'axis2', self._visualizationChanged )
		#self.onAttributeChange( 'axis3', self._visualizationChanged )
		#self.onAttributeChange( 'resolution', self._visualizationChanged )

	def resetShape( self, center, axis1, axis2, axis3, resolution = None, displayEquation = False ) :
		# Reset internal attribute values
		if ( center is not None ) :
			self.center = center
			
		if ( axis1 is not None ) :
			self.axis1 = axis1

		if ( axis2 is not None ) :
			self.axis2 = axis2

		if ( axis3 is not None ) :
			self.axis3 = axis3

		if ( resolution is not None ) :
			self.resolution = resolution

		# Transform ellipsoid parameters to polynomial
		coefficients = self.toPolynomialCoefficients( [self.axis1, self.axis2, self.axis3] )
		sizes = aims.Point3df( self.axis1, self.axis2, self.axis3)
		start = self.center - ( sizes )
		self.setCoefficients( coefficients, start, self.resolution, sizes * 2, displayEquation )
	
	def resetVizualisation( self ) :
		"""
		Reset the vizualisation for the current parametric Shape.
		"""
		internalBucket = self[0]
		internalBucket.setInternalsChanged()
		internalBucket.notifyObservers()

	def getCoefficients( self ) :
		return self.fromPolynomialCoefficients( self.polynomial.getCoefficients() )
	
	def setCoefficients( self, coefficients, start = None, resolution = None, sizes = None, displayEquation = False ) :
		# Reset internal attribute values
		if ( start is not None ) :
			self.start = start
			
		if ( resolution is not None ) :
			self.resolution = resolution

		if ( sizes is not None ) :
			self.sizes = sizes
			
		# Reset internal objects
		self.sampler = BucketMapSampler_FLOAT_3()
		self.polynomial = Polynomial_FLOAT_3( coefficients )
		sampled = self.sampler.sample(self.polynomial, self.start, self.sizes, self.resolution)
		self.bucketMap = sampled.getPython().get()
		self.viewedBucketMap[ self.viewedBucketMap.size() ] = self.bucketMap[0]

		if ( displayEquation ) :
			# Display the new equation
			self.displayEquation()
			
	def getBucketMap( self ) :
		return self.bucketMap
	
	def displayEquation( self ) :
		return self.polynomial.displayEquation()

	def translate( self, translation ) :
		pass
	
	def scale( self, scale ) :
		pass

	def toPolynomialCoefficients( self, coefficients ) :
		A = pow(coefficients[0], -2)
		B = pow(coefficients[1], -2)
		C = pow(coefficients[2], -2)
		return [-1,0,A,
				0,0,0,
				B,0,0,
					0,0,0,
					0,0,0,
					0,0,0,
				C,0,0,
				0,0,0,
				0,0,0]

	def fromPolynomialCoefficients( self, coefficients ) :
		axis1 = pow(coefficients[2], -0.5)
		axis2 = pow(coefficients[6], -0.5)
		axis3 = pow(coefficients[18], -0.5)
		return [ axis1, axis2, axis3 ]