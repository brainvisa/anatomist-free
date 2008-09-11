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
# 	This file contains Cone class
#
#########################################################################

from soma import aims
import anatomist.cpp as anatomist

from soma.signature.api \
  import Signature, HasSignature, Unicode, Sequence, Number
from soma.aimsalgo import BucketMapSampler_FLOAT_3, Polynomial_FLOAT_3
from soma.wip.aims.signature                import Point3d
from anatomist.wip.parametrics.shapes.shape import Shape

class Polynomial( HasSignature, Shape ):
	signature = Signature(
	'name', Unicode(),
	'coefficients', Sequence(Number),
	'start', Point3d(),
	'resolution', Point3d(),
	'sizes', Point3d(),
	)
	
	def __init__( self, coefficients = [0], start = aims.Point3df(-10), resolution = aims.Point3df(1), sizes = aims.Point3df(20), name = "Polynomial"):
		""" 
		Parameters
		----------
		coefficients : array
		Polynomial coefficients

		Description
		-----------
		Constructor of the Polynomial object
		"""
		
		# Initialize parent types
		HasSignature.__init__( self )
		Shape.__init__( self, name )

		self.bucketMap = aims.BucketMap_VOID()
		anatomistBucketMap = anatomist.AObjectConverter.anatomist( aims.BucketMap_VOID() )
		self.viewedBucketMap = anatomist.AObjectConverter.aims( anatomistBucketMap )
		self.insert( anatomistBucketMap )
		
		self.resetShape( coefficients, start, resolution, sizes )
		self.resetVizualisation( )
		
		# Register events
		self.onAttributeChange( 'name', self._nameChanged )
		self.onAttributeChange( 'start', self._visualizationChanged )
		self.onAttributeChange( 'resolution', self._visualizationChanged )
		self.onAttributeChange( 'sizes', self._visualizationChanged )
		self.onAttributeChange( 'coefficients', self._visualizationChanged )

	def resetShape( self, coefficients, start = None, resolution = None, sizes = None, displayEquation = False ) :
		self.setCoefficients( coefficients, start, resolution, sizes, displayEquation )
	
	def resetVizualisation( self ) :
		"""
		Description
		-----------
		Reset the vizualisation for the current parametric Shape.
		"""
		internalBucket = self[0]
		internalBucket.setInternalsChanged()
		internalBucket.notifyObservers()

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
			self.polynomial.displayEquation()
			
	def getCoefficients( self ) :
		return self.polynomial.getCoefficients()
		
	def getBucketMap( self ) :
		return self.bucketMap
	
	def displayEquation( self ) :
		return self.polynomial.displayEquation()

	def translate( self, translation ) :
		pass
	
	def scale( self, scale ) :
		pass

	#def isContained( self, point ):
		#""" 
		#Parameters
		#----------       
		#point : aims.Point3df
		#Point information to test if it is contained by the object
		
		#Description
		#-----------
		#True if the point is contained in the object, False otherwise
		#"""
		#x = point[0]
		#y = point[1]
		#z = point[2]
		
		#result = (self.A * x**2) + (self.B * y**2) + (self.C * z**2) + (self.D * x * y) + (self.E * x * z) + (self.F * y * z) + (self.G * x) + (self.H * y) + (self.I * z) + self.J
		
		#return (result <= 0)

		# In coefficients, column indice are x orders, row indice are y orders and Matrix indice are z orders

		# Ellipsoid equation
		#self.coefficients = [-1,0,0.33,
								#0,0,0,
								#0.5,0,0,
									#0,0,0,
									#0,0,0,
									#0,0,0,
								#1,0,0,
								#0,0,0,
								#0,0,0]

		# Rounded cube equation
		#self.coefficients = [-1,0,0,0,1,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#1,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
									#0,0,0,0,0,
								#1,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0,
								#0,0,0,0,0]