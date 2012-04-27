# -*- coding: utf-8 -*-
#  This software and supporting documentation are distributed by
#      Institut Federatif de Recherche 49
#      CEA/NeuroSpin, Batiment 145,
#      91191 Gif-sur-Yvette cedex
#      France
#
# This software is governed by the CeCILL-B license under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL-B license as circulated by CEA, CNRS
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
# knowledge of the CeCILL-B license and that you accept its terms.
from PyQt4 import QtCore, QtGui
import anatomist.direct.api as anatomist
from soma import aims
import numpy
import selection

byvertex = False

# This intermediate class is only here because I cannot (yet) make SIP
# generate a non-abstract class for TextObject binding. One day, I'll find out!
class TObj ( anatomist.cpp.TextObject ):
  def __init__( self, message='', pos=[0,0,0] ):
    anatomist.cpp.TextObject.__init__( self, message, pos )

class AnnotationProperties( object ):
  def __init__( self ):
    self.lvert = []
    self.lpoly = []
    self.usespheres = True
    self.colorlabels = True
    self.center = aims.Point3df()

def makelabel( label, gc, pos, color, props, colors, agraph ):
  objects = []
  to = TObj( label )
  to.setScale( 0.1 )
  to.setName( 'label: ' + label )
  to.setReferentialInheritance( agraph )
  if colors.has_key( label ):
    color = colors[ label ]
    if props.usespheres:
      sph = aims.SurfaceGenerator.icosphere( gc, 2, 50 )
      asph = a.toAObject( sph )
      asph.releaseAppRef()
      asph.setReferentialInheritance( agraph )
      a.unmapObject( asph )
      asph.setMaterial( diffuse=color )
      asph.setName( 'gc: ' + label )
      objects.append( asph )
    if props.colorlabels:
      to.GetMaterial().set( { 'diffuse': color } )
  texto = anatomist.cpp.TransformedObject( [ to ], False, True, pos )
  texto.setReferentialInheritance( agraph )
  texto.setDynamicOffsetFromPoint( props.center )
  texto.setName( 'annot: ' + label )
  objects.append( texto )
  props.lpoly.append( aims.AimsVector_U32_2( ( len( props.lvert ),
    len( props.lvert ) + 1 ) ) )
  props.lvert += [ gc, pos ]
  a.registerObject( texto, False )
  a.releaseObject( texto )
  return objects


class AnnotationAction( anatomist.cpp.Action ):
  def __init__( self ):
    anatomist.cpp.Action.__init__( self )
    self._built = False
    self._temp = []

  def name( self ):
    return 'AnnotationAction'

  def annotationSelected( self ):
    win = self.view().window()
    objs = win.Objects()
    graphs = []
    self._temp = []
    for o in objs:
      if o.objectTypeName( o.type() ) == 'GRAPH':
        graphs.append( o )
    for graph in graphs:
      self.buildGraphAnnotations( graph )
    if len( self._temp ) != 0:
      self._built = True

  def cleanAnnotations( self ):
    self._built = False
    a = anatomist.Anatomist()
    #a.execute( 'DeleteElement', elements=self._temp )
    self._temp = []

  def switchAnnotations( self ):
    if self._built:
      self.cleanAnnotations()
    else:
      self.annotationSelected()

  def buildGraphAnnotations( self, agraph ):
    a = anatomist.Anatomist()
    graph = agraph.graph()
    labelatt = 'name'
    if graph.has_key( 'label_property' ):
      labelatt = graph[ 'label_property' ]
    bbox = agraph.boundingbox()
    props = AnnotationProperties()
    props.center = ( bbox[0] + bbox[1] ) / 2

    size = ( bbox[1] - bbox[0] ).norm() * 0.2
    vs = graph[ 'voxel_size' ][:3]
    objects = []
    lines = aims.TimeSurface( 2 )

    elements = {}
    colors = {}

    for v in graph.vertices():
      if v.has_key( 'gravity_center' ) and v.has_key( labelatt ):
        gc = aims.Point3df( numpy.array( v['gravity_center' ] ) * vs )
        label = v[ labelatt ]
        if label != 'unknown':
          if not elements.has_key( label ):
            elem = [ aims.Point3df( 0, 0, 0 ), 0. ]
            elements[ label ] = elem
          else:
            elem = elements[ label ]
          sz = v[ 'size' ]
          elem[0] += gc * sz
          elem[1] += sz
          color = [ 0, 0, 0, 1 ]
          if v.has_key( 'ana_object' ):
            av = v[ 'ana_object' ]
            color = av.GetMaterial().genericDescription()[ 'diffuse' ]
            colors[ label ] = color
          if byvertex:
            pos = gc + ( gc - props.center ).normalize() * size
            objects += makelabel( label, gc, pos, color, props, colors,
              agraph )

    if not byvertex:
      for label, elem in elements.iteritems():
        gc = elem[0] / elem[1]
        pos = gc + ( gc - props.center ).normalize() * size
        if colors.has_key( label ):
          color = colors[ label ]
        else:
          color = [ 0, 0, 0, 1 ]
        objects += makelabel( label, gc, pos, color, props, colors, agraph )

    if len( objects ) == 0:
      return
    lines.vertex().assign( props.lvert )
    lines.polygon().assign( props.lpoly )
    alines = a.toAObject( lines )
    alines.releaseAppRef()
    alines.setMaterial( diffuse=[ 0, 0, 0, 1 ] )
    alines.setReferentialInheritance( agraph )
    objects.append( alines )
    labels = a.groupObjects( objects )
    labels.setReferentialInheritance( agraph )
    labels.releaseAppRef()
    a.unmapObject( alines )
    a.unmapObject( labels )
    self.view().window().registerObject( alines, True )
    for o in objects:
      self.view().window().registerObject( o, True )
    #a.execute( 'AddObject', windows=[self.view().window()], objects=[labels], add_children=1 )
    self._temp.append( labels )


def new_eventAutoSubscription( self, pool ):
  self._old_eventAutoSubsctiption( pool )
  self.keyPressEventSubscribe( QtCore.Qt.Key_A, QtCore.Qt.NoModifier,
                               pool.action( 'AnnotationAction' \
                               ).switchAnnotations )
  #self.keyPressEventSubscribe( QtCore.Qt.Key_Tab, QtCore.Qt.NoModifier,
                               #pool.action( 'AnnotationAction' \
                               #).switchAnnotations )

def new_doAlsoOnDeselect( self, pool ):
  self._old_doAlsoOnDeselect( pool )
  pool.action( 'AnnotationAction' ).cleanAnnotations()


a = anatomist.Anatomist()
ad = anatomist.cpp.ActionDictionary.instance()
ad.addAction( 'AnnotationAction', AnnotationAction )
cd = anatomist.cpp.ControlDictionary.instance()
selctrl = selection.SelectionControl
if not hasattr( selctrl, '_old_eventAutoSubsctiption' ):
  selctrl._old_eventAutoSubsctiption = selctrl.eventAutoSubscription
  selctrl._old_doAlsoOnDeselect = selctrl.doAlsoOnDeselect
selctrl.eventAutoSubscription = new_eventAutoSubscription
selctrl.doAlsoOnDeselect = new_doAlsoOnDeselect

