# Copyright CEA (2000-2008)
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

import anatomist.cpp as anatomist
from soma import aims
import numpy

class ObjectFollowerCube( anatomist.ASurface_2 ):
  def __init__( self, obj ):
    anatomist.ASurface_2.__init__( self )
    self.GetMaterial().set( { 'ghost': 1 } )
    self._objects = []
    self.setObserved( obj )

  def observed( self ):
    return self._objects

  def setObserved( self, obj ):
    oldobj = [ i for i in self._objects ]
    for i in oldobj:
      self.unregisterObservable( i )
    del oldobj
    self._objects = [ anatomist.weak_ptr_AObject( i ) for i in obj ]
    ref = None
    for i in obj:
      self.registerObservable( i )
      if ref is None:
        self.setReferentialInheritance( i )
    self.redraw()

  def update( self, obs, param ):
    if obs in self._objects:
      self.redraw()
      self.notifyObservers( obs )

  def unregisterObserver( self, obs ):
    if obs in self._objects:
      self._objects = [ i for i in self._objects if i != obs ]
      self.redraw()

  def boundingbox( self ):
    bbox = []
    for obj in self._objects:
      bbox2 = obj.boundingbox()
      a = anatomist.Anatomist()
      tr = a.getTransformation( obj.getReferential(),
        self.getReferential() )
      if tr is not None:
        bbox2 = tr.transformBoundingBox( bbox2[0], bbox2[1] )
      if not bbox:
        bbox = bbox2
      else:
        bbox = [ aims.Point3df( numpy.min( [ bbox[0], bbox2[0] ],
          axis=0 ) ),
          aims.Point3df( numpy.max( [ bbox[1], bbox2[1] ], axis=0 ) ) ]
    if not bbox:
      return ()
    return ( bbox[0], bbox[1] )

  def redraw( self ):
    mesh = self.surface()
    if mesh.isNull():
      self.setSurface( aims.AimsTimeSurface_2() )
      mesh = self.surface()
    if not self._objects:
      mesh.vertex().clear()
      mesh.normal().clear()
      mesh.polygon().clear()
    else:
      bbox = self.boundingbox()
      vert = [ aims.Point3df( bbox[0] ),
        aims.Point3df( bbox[1][0], bbox[0][1], bbox[0][2] ),
        aims.Point3df( bbox[0][0], bbox[1][1], bbox[0][2] ),
        aims.Point3df( bbox[0][0], bbox[0][1], bbox[1][2] ),
        aims.Point3df( bbox[1][0], bbox[1][1], bbox[0][2] ),
        aims.Point3df( bbox[1][0], bbox[0][1], bbox[1][2] ),
        aims.Point3df( bbox[0][0], bbox[1][1], bbox[1][2] ),
        aims.Point3df( bbox[1] )
      ]
      pol = [ [0,1], [0,2], [1,4], [2,4], [0,3], [1,5], [2,6], [4,7], [3,5],
        [3,6], [5,7], [6,7] ]
      mesh.vertex().assign( vert )
      mesh.polygon().assign( [ aims.AimsVector_U32_2(i) for i in pol ] )
    self.setChanged()
