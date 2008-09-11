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
# Module : operations
# Create date : 2006-06-14
#
# Description :
# 	This file contains tools concerning meshes that uses python 
#       wrappings.
#
#########################################################################

from soma import aims
import anatomist.cpp as anatomist

def getCenter(object):
	""" 
	Gets the barycenter of an anatomist.ASurface_3

        @type object : anatomist.ASurface_3
	@param object: Mesh containing polygons to scale
	
	@rtype: aims.Point3df
	@return: Processed barycenter of the anatomist.ASurface_3	
	"""
	if isinstance(object, anatomist.ASurface_3):
		center = aims.Point3df( 0, 0, 0 )
		v = object.surface().vertex()
		
		for i in xrange( len(v) ):
			center += v[i]
		return center / len(v)
	else:
		return None

def isPointContainedInCylinder( point, point1, point2, radius1, radius2 ) :
	""" 
	Gets a boolean value that specify if point is contained by cylinder.

	@type point : aims.Point3df
	@param point: point to test if contained in cylinder
	
	@type point1 : aims.Point3df
	@param point1: point1 of cylinder definition
	
	@type point2 : aims.Point3df
	@param point2: point2 of cylinder definition
	
	@type radius1 : float
	@param radius1: radius1 of cylinder definition
	
	@type radius2 : float
	@param radius2: radius2 of cylinder definition
	
	@rtype: boolean
	@return: True if the point is contained in cylinder, False otherwise.
	"""
	pointC = ( point1 + point2 ) / 2
	P1C = ( point2 - point1 ) / 2
	CP1 = P1C * -1
	P1CNorm = P1C.dnorm()
	CM = ( point - pointC )
	CMp = CP1 * ( abs( CM.dot( CP1 ) ) / ( P1CNorm ** 2 ) )
	CMpNorm = CMp.dnorm()
	
	MsMpNorm = ( ( ( P1C + CMp ).dnorm() * abs( radius2 - radius1 ) ) / ( CMp - P1C ).dnorm() ) + min( radius1, radius2 )
	MMpNorm = ( CM - CMp ).dnorm()
	
	return ( ( MsMpNorm >= MMpNorm ) and ( P1CNorm >= CMpNorm ) )


def getMergedBucketMap( objects ):
	'''
	Recursively get the merged bucket map from a list of objects
	'''
	mergedBucketMap = aims.BucketMap_VOID()

	for object in objects :
		
		if hasattr( object, 'type' ) :
			if ( object.type() == anatomist.AObject.BUCKET ):
				# Merges the new bucket found with prevously merged buckets
				bucketMap = anatomist.AObjectConverter.aimsBucketMap_VOID( object )
		
		elif ( type(object) is aims.BucketMap_VOID ):
			bucketMap = object
			
		elif ( hasattr(object, '__iter__') ):
			# Iterates through all the object tree
			bucketMap = getMergedBucketMap( object )
			
		mergedBucketMap.merge( bucketMap )

	return mergedBucketMap


