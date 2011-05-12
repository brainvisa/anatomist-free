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
import sys
qt4 = False
if sys.modules.has_key( 'PyQt4' ):
  qt4 = True
  from PyQt4 import QtCore, QtGui
  qt = QtGui
  from PyQt4.uic import loadUiType
  UIFormClass = None
  findChild = lambda x, y: QtCore.QObject.findChild( x, QtCore.QObject, y )
  #raise RuntimeError( 'The Selection module has not been ported to ' \
    #'Qt4 yet.' )
else:
  import qt, qtui
  findChild = qt.QObject.child

import anatomist.direct.api as anatomist
from soma import aims
import os, sip

#disable_me

class SelectionActionView( qt.QWidget ):
  def __init__( self, action, parent, name=None, flags=0 ):
    if qt4:
      qt.QWidget.__init__( self, parent )
      self.setObjectName( name )
    else:
      qt.QWidget.__init__( self, parent, name, flags )
      layout = qt.QVBoxLayout( self )
    self._action = action
    window = self._action.view().window()
    objs = window.Objects()
    vertex_it = 0
    edge_it = 0
    nodes_opacity = 100
    edges_opacity = 100
    for obj in objs:
      if isinstance(obj, anatomist.cpp.AGraph):
        for v in obj:
          go = v.attributed()
          if isinstance( go, aims.Vertex ):
            mat = v.GetMaterial()
            nodes_opacity = int(round(mat.genericDescription()["diffuse"][3]*100))
            vertex_it = 1
          if isinstance( go, aims.Edge ):
            mat = v.GetMaterial()
            edges_opacity = int(round(mat.genericDescription()["diffuse"][3]*100))
            edge_it = 1
          if edge_it==1 and vertex_it==1:
            break
      if edge_it==1 and vertex_it==1:
        break
    name = __file__
    if name.endswith( '.pyc' ) or name.endswith( '.pyo' ):
      name = name[:-1]
    if qt4:
      name = os.path.join(os.path.dirname(os.path.realpath(name)),
      'selection-qt4.ui')
      global UIFormClass
      if UIFormClass is None:
        UIFormClass, baseclass = loadUiType( name )
      qWidget = self
      UIFormClass().setupUi( qWidget )
      #layout.addWidget( qWidget )
    else:
      name = os.path.join(os.path.dirname(os.path.realpath(name)),
        'selection.ui')
      qWidget = qtui.QWidgetFactory.create(name, self, self,
        "Selection Widget UI" )
      layout.addWidget( qWidget )
    findChild( qWidget, 'nodesSlider' ).setValue(nodes_opacity)
    findChild( qWidget, 'nodesLabel' ).setText(str(nodes_opacity))
    findChild( qWidget, 'edgesSlider' ).setValue(edges_opacity)
    findChild( qWidget, 'edgesLabel' ).setText(str(edges_opacity))
    smc = findChild( qWidget, 'selectionModeCombo' )
    for mode in self._action.modes_list:
      if qt4:
        smc.insertItem(smc.count(), mode)
      else:
        smc.insertItem(mode)
    if qt4:
      smc.setCurrentIndex(self._action.mode)
    else:
      smc.setCurrentItem(self._action.mode)
    self.connect( findChild( qWidget, 'nodesSlider' ),
      qt.SIGNAL( 'valueChanged(int)' ), self.nodesOpacityChanged )
    self.connect( findChild( qWidget, 'edgesSlider' ),
      qt.SIGNAL( 'valueChanged(int)' ), self.edgesOpacityChanged )
    self.connect( smc, qt.SIGNAL( 'activated(int)' ),
      self.selectionModeChanged )

  def nodesOpacityChanged( self, value ):
    findChild( self, 'nodesLabel' ).setText(str(value))
    self._action.updateNodesOpacity(value)

  def edgesOpacityChanged( self, value ):
    findChild( self, 'edgesLabel' ).setText(str(value))
    self._action.updateEdgesOpacity(value)

  def selectionModeChanged( self, value ):
    self._action.setMode(value)

class SelectionAction( anatomist.cpp.Action ):
  modes_list = ["Basic", "Intersection", "Union"]
  mode_basic = 0
  mode_intersection = 1
  mode_union = 2

  def __init__(self):
    anatomist.cpp.Action.__init__(self)
    self.mode = self.mode_basic

  def setMode(self,mode):
    if type(mode) is type('') or type(mode) is type(u''):
      mode = self.modes_list.index( mode )
    self.mode = mode
    self.cleanup()
    self.edgeSelection()

  def name( self ):
    return 'SelectionAction'

  def viewableAction( self ):
    return True

  def actionView(self, parent):
    qWidget = SelectionActionView( self, parent,
      "Selection Widget" )
    return qWidget

  def updateNodesOpacity(self, value):
    window = self.view().window()
    objs = window.Objects()
    for obj in objs:
      if isinstance(obj, anatomist.cpp.AGraph):
        aimsObj = anatomist.cpp.AObjectConverter.aims(obj)
        vertices = aimsObj.vertices()
        for vertex in vertices:
          if vertex.has_key('ana_object'):
            v0 = vertex['ana_object']
            mat = v0.GetMaterial()
            mat_desc = mat.genericDescription()
            mat_desc = { 'diffuse': [mat_desc['diffuse'][0],
              mat_desc['diffuse'][1], mat_desc['diffuse'][2], value/100. ] }
            mat.set(mat_desc)
            v0.SetMaterial(mat)
            v0.notifyObservers()

  def updateEdgesOpacity(self, value):
    window = self.view().window()
    objs = window.Objects()
    import time
    t = time.time()
    for obj in objs:
      if isinstance(obj, anatomist.cpp.AGraph):
        aimsObj = anatomist.cpp.AObjectConverter.aims(obj)
        edges = aimsObj.edges()
        for edge in edges:
          if edge.has_key('ana_object'):
            v0 =edge['ana_object']
            mat = v0.GetMaterial()
            mat_desc = mat.genericDescription()
            mat_desc = { 'diffuse': [mat_desc['diffuse'][0],
              mat_desc['diffuse'][1], mat_desc['diffuse'][2], value/100. ] }
            mat.set(mat_desc)
            v0.SetMaterial(mat)
            v0.notifyObservers()
    #print 'time:', time.time() - t

  def changeColorByTargets(self, vertex_source, edge):
    vertices = edge.vertices()
    vertex_target = [ x for x in vertices if x != vertex_source ][0]
    if vertex_target.has_key("ana_object"):
      mat = vertex_target["ana_object"].GetMaterial()
      edgeAna_object = edge['ana_object']
      mat_desc = mat.genericDescription()
      mat_edge = edgeAna_object.GetMaterial()
      matEdge_desc = mat_edge.genericDescription()
      matEdge_desc = { 'diffuse': [mat_desc['diffuse'][0],
        mat_desc['diffuse'][1], mat_desc['diffuse'][2],
        matEdge_desc['diffuse'][3] ] }
      mat_edge.set(matEdge_desc)
      edgeAna_object.SetMaterial(mat_edge)
      edgeAna_object.notifyObservers()

  def changeColorBySources(self, edge):
    if edge.has_key( 'ana_object' ):
      vertices = edge.vertices()
      sf = anatomist.cpp.SelectFactory.factory()
      window = self.view().window()
      group = window.Group()
      for vertex in vertices:
        if vertex.has_key('ana_object') \
          and sf.isSelected(group, vertex['ana_object']):
          mat = vertex["ana_object"].GetMaterial()
          edgeAna_object = edge['ana_object']
          mat_desc = mat.genericDescription()
          mat_edge = edgeAna_object.GetMaterial()
          matEdge_desc = mat_edge.genericDescription()
          matEdge_desc = { 'diffuse': [mat_desc['diffuse'][0],  mat_desc['diffuse'][1], mat_desc['diffuse'][2], matEdge_desc['diffuse'][3] ] }
          mat_edge.set(matEdge_desc)
          edgeAna_object.SetMaterial(mat_edge)
          edgeAna_object.notifyObservers()
          break

  def edgeSelection( self ):
    try:
      recursion = getattr( self, '_recursion' )
      if recursion:
        return
    except:
      pass
    self._recursion = True
    #print 'edgeSelection'
    sf = anatomist.cpp.SelectFactory.factory()
    sel = sf.selected()
    window = self.view().window()
    group = window.Group()
    gsel = sel.get( group )
    vertexlist = set()
    edgeslist = set()
    selectedEdges_set = set()
    if gsel is not None:
      for obj in gsel:
        if obj.type() == anatomist.cpp.AObject.GRAPHOBJECT:
          go = obj.attributed()
          if isinstance( go, aims.Vertex ):
            vertexlist.add( go )
            for edge in go.edges():
              edgeslist.add(edge)
    #colorchanged = False
    if len(vertexlist)==1:
      for edge in edgeslist:
        if edge.has_key( 'ana_object' ):
          if self.mode == self.mode_intersection:
            self.changeColorByTargets(list(vertexlist)[0], edge)
            #colorchanged = True
            aobj = edge['ana_object']
            window.registerObject( aobj )
            selectedEdges_set.add( anatomist.cpp.weak_ptr_AObject( aobj ) )
          elif self.mode == self.mode_union:
            self.changeColorBySources(edge)
            #colorchanged = True
            aobj = edge['ana_object']
            window.registerObject( aobj )
            selectedEdges_set.add( anatomist.cpp.weak_ptr_AObject( aobj ) )
    else:
      if self.mode == self.mode_intersection:
        for edge in edgeslist:
          edge_validity = True
          for vertex in edge.vertices():
            if vertex not in vertexlist:
              edge_validity = False
              break
          if edge_validity:
            if edge.has_key( 'ana_object' ):
              aobj = edge['ana_object']
              selectedEdges_set.add( anatomist.cpp.weak_ptr_AObject( aobj ) )
              window.registerObject( aobj )
      elif self.mode == self.mode_union:
        for edge in edgeslist:
          self.changeColorBySources(edge)
          #colorchanged = True
          aobj = edge['ana_object']
          window.registerObject( aobj )
          selectedEdges_set.add( anatomist.cpp.weak_ptr_AObject( aobj ) )

    #if colorchanged:
      #graphs = set()
      #for edge in selectedEdges_set:
        #pl = edge['ana_object'].parents()
        #for p in pl:
          #if p.type() == anatomist.cpp.AObject.GRAPH:
            #graphs.add( p )
      #for graph in graphs:
        #graph.setChanged()
        #graph.notifyObservers()
    if self.mode != self.mode_basic:
      objlist = window.Objects()
      for obj in objlist:
        if obj.type() == anatomist.cpp.AObject.GRAPHOBJECT:
          go = obj.attributed()
          if isinstance( go, aims.Edge ):
            if anatomist.cpp.weak_ptr_AObject( obj ) not in selectedEdges_set:
              window.unregisterObject(obj)
    self._tempedges = selectedEdges_set

    del self._recursion

  def cleanup( self ):
    if hasattr( self, '_recursion' ):
      return
    if hasattr( self, '_tempedges' ):
      self._recursion = True
      window = self.view().window()
      for obj in self._tempedges:
        window.unregisterObject(obj.get())
      del self._tempedges
      del self._recursion


class SelectionControl( anatomist.cpp.Select3DControl ):
  def __init__( self ):
    anatomist.cpp.Select3DControl.__init__( self, 'SelectionControl' )

  def eventAutoSubscription( self, pool ):
    anatomist.cpp.Select3DControl.eventAutoSubscription( self, pool )
    self.selectionChangedEventSubscribe( pool.action( \
      'SelectionAction' ).edgeSelection )

  def doAlsoOnSelect( self, actionpool ):
    anatomist.cpp.Select3DControl.doAlsoOnSelect( self, actionpool )
    actionpool.action("SelectionAction").edgeSelection()

  def doAlsoOnDeselect( self, actionpool ):
    anatomist.cpp.Select3DControl.doAlsoOnDeselect( self, actionpool )
    actionpool.action("SelectionAction").cleanup()

a = anatomist.Anatomist()

icon = anatomist.cpp.IconDictionary.instance().getIconInstance( \
  'Selection 3D' )
anatomist.cpp.IconDictionary.instance().addIcon( 'SelectionControl',
  qt.QPixmap( icon ) )
ad = anatomist.cpp.ActionDictionary.instance()
ad.addAction( 'SelectionAction', SelectionAction )
cd = anatomist.cpp.ControlDictionary.instance()
cd.addControl( 'SelectionControl', SelectionControl, 2 )
cm = anatomist.cpp.ControlManager.instance()
cm.removeControl( 'QAGLWidget3D', '', 'Selection 3D' )
cm.addControl( 'QAGLWidget3D', '', 'SelectionControl' )

