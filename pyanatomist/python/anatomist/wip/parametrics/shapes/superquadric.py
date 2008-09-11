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
# 	This file contains SuperQuadric class
#
#########################################################################

from soma import aims
import anatomist.cpp as anatomist

from soma.signature.api \
  import Signature, HasSignature, Unicode, Sequence, Number
from soma.aimsalgo                          import BucketMapSampler_FLOAT_3
from soma.wip.aimsalgo.samplables           import SuperQuadricSamplable
from soma.wip.aims.signature                import Point3d
from anatomist.wip.parametrics.shapes.shape import Shape

class SuperQuadric( HasSignature, Shape ):
	signature = Signature(
	'name', Unicode(),
	'coefficients', Sequence(Number),
	'start', Point3d(),
	'resolution', Point3d(),
	'sizes', Point3d(),
	)
	
	def __init__( self, coefficients = [0], start = None, sizes = None, resolution = aims.Point3df(1), name = "SuperQuadric"):
		""" 
		Constructor of the Quadric object		
		"""
		
		# Initialize parent types
		HasSignature.__init__( self )
		Shape.__init__( self, name )

		self.viewedBucketMap = anatomist.AObjectConverter.anatomist( aims.BucketMap_VOID() )
		self.bucketMap = anatomist.AObjectConverter.aims( self.viewedBucketMap )
		self.insert( self.viewedBucketMap )
		self.resetShape( coefficients, start, sizes, resolution )
		self.resetVizualisation( )

		# Register events
		#self.onAttributeChange( 'name', self._nameChanged )
		#self.onAttributeChange( 'start', self._visualizationChanged )
		#self.onAttributeChange( 'resolution', self._visualizationChanged )
		#self.onAttributeChange( 'sizes', self._visualizationChanged )
		#self.onAttributeChange( 'coefficients', self._visualizationChanged )

	def resetShape( self, coefficients, start = None, sizes = None, resolution = None, displayEquation = False ) :
		self.sampler = BucketMapSampler_FLOAT_3()
		self.setCoefficients( coefficients, start, sizes, resolution, displayEquation )
	
	def resetVizualisation( self ) :
		"""
		Reset the vizualisation for the current parametric Shape.
		"""
		self.viewedBucketMap.setInternalsChanged()
		self.viewedBucketMap.notifyObservers()

	def setCoefficients( self, coefficients, start = None, sizes = None, resolution = None, displayEquation = False ) :

		# Reset internal objects
		self.samplable = SuperQuadricSamplable( coefficients )

		# Reset internal attribute values
		if ( start is not None ) :
			self.start = start
		else :
			self.start = aims.Point3df(0, 0, 0)

		if ( sizes is not None ) :
			self.sizes = sizes
		else :
			self.sizes = self.samplable.getBoundingBoxSizes()

		if ( resolution is not None ) :
			self.resolution = resolution

		if ( self.samplable.isChecked() ) :
			
			sampled = self.sampler.sample(self.samplable, self.start, self.sizes, self.resolution)
			self.lastProcessedBucketMap = sampled.getPython().get()
			
			# Update internal bucketMaps
			self.bucketMap[ self.bucketMap.size() ] = self.lastProcessedBucketMap[0]
		else :
			self.lastProcessedBucketMap = aims.BucketMap_VOID()

		if ( displayEquation ) :
			# Display the new equation
			pass
				
	def getCoefficients( self ) :
		return self.samplable._coefficients
		
	def getBucketMap( self ) :
		return self.bucketMap
	
	def displayEquation( self ) :
		pass

	def translate( self, translation ) :
		pass
	
	def scale( self, scale ) :
		pass
