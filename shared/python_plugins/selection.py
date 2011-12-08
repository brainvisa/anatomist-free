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
from PyQt4 import QtCore, QtGui
qt = QtGui
from PyQt4.uic import loadUiType
UIFormClass = None
findChild = lambda x, y: QtCore.QObject.findChild( x, QtCore.QObject, y )

import anatomist.direct.api as anatomist
import anatomist.cpp.followerobject as followerobject
from soma import aims
import os, sip

#disable_me

class SelectionActionView( qt.QWidget ):
  def __init__( self, action, parent, name=None, flags=0 ):
    qt.QWidget.__init__( self, parent )
    self.setObjectName( name )
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
    name = os.path.join(os.path.dirname(os.path.realpath(name)),
    'selection-qt4.ui')
    global UIFormClass
    if UIFormClass is None:
      UIFormClass, baseclass = loadUiType( name )
    qWidget = self
    UIFormClass().setupUi( qWidget )
    #layout.addWidget( qWidget )
    findChild( qWidget, 'nodesSlider' ).setValue(nodes_opacity)
    findChild( qWidget, 'nodesLabel' ).setText(str(nodes_opacity))
    findChild( qWidget, 'edgesSlider' ).setValue(edges_opacity)
    findChild( qWidget, 'edgesLabel' ).setText(str(edges_opacity))
    smc = findChild( qWidget, 'selectionModeCombo' )
    for mode in self._action.modes_list:
      smc.insertItem(smc.count(), mode)
    smc.setCurrentIndex(self._action.mode)
    boxHighlight = findChild( qWidget, 'boxHighlight' )
    boxHighlightIndividual = findChild( qWidget, 'boxHighlightIndividual' )
    boxHighlight.setChecked( self._action.useBoxHighlight )
    boxHighlightIndividual.setChecked( self._action.useBoxHighlightIndividual )
    boxColorMode = findChild( qWidget, 'boxColorMode' )
    boxCustomColor = findChild( qWidget, 'boxCustomColor' )
    boxColorMode.setCurrentIndex( self._action.boxSelectionColorMode )
    col = QtGui.QColor( self._action.boxSelectionCustomColor.r * 255.99,
      self._action.boxSelectionCustomColor.g * 255.99,
      self._action.boxSelectionCustomColor.b * 255.99 )
    boxCustomColor.setPalette( QtGui.QPalette( col ) )

    self.connect( findChild( qWidget, 'nodesSlider' ),
      qt.SIGNAL( 'valueChanged(int)' ), self.nodesOpacityChanged )
    self.connect( findChild( qWidget, 'edgesSlider' ),
      qt.SIGNAL( 'valueChanged(int)' ), self.edgesOpacityChanged )
    self.connect( smc, qt.SIGNAL( 'activated(int)' ),
      self.selectionModeChanged )
    self.connect( boxHighlight, qt.SIGNAL( 'stateChanged(int)' ),
      self.switchBoxHighligting )
    self.connect( boxHighlightIndividual, qt.SIGNAL( 'stateChanged(int)' ),
      self.switchBoxHighligtingIndividual )
    self.connect( boxColorMode, qt.SIGNAL( 'activated(int)' ),
      self.switchBoxColorMode )
    self.connect( boxCustomColor, qt.SIGNAL( 'clicked(bool)' ),
      self.selectCustomBoxColor )

  def nodesOpacityChanged( self, value ):
    findChild( self, 'nodesLabel' ).setText(str(value))
    self._action.updateNodesOpacity(value)

  def edgesOpacityChanged( self, value ):
    findChild( self, 'edgesLabel' ).setText(str(value))
    self._action.updateEdgesOpacity(value)

  def selectionModeChanged( self, value ):
    self._action.setMode(value)

  def switchBoxHighligting( self, value ):
    self._action.switchBoxHighligting(value)

  def switchBoxHighligtingIndividual( self, value ):
    self._action.switchBoxHighligtingIndividual(value)

  def switchBoxColorMode( self, value ):
    self._action.switchBoxColorMode( value )

  def selectCustomBoxColor( self, value ):
    self._action.selectCustomBoxColor()
    col = QtGui.QColor( self._action.boxSelectionCustomColor.r * 255.99,
      self._action.boxSelectionCustomColor.g * 255.99,
      self._action.boxSelectionCustomColor.b * 255.99 )
    findChild( self, 'boxCustomColor' ).setPalette( QtGui.QPalette( col ) )


class SelectionAction( anatomist.cpp.Action ):
  modes_list = ["Basic", "Intersection", "Union"]
  mode_basic = 0
  mode_intersection = 1
  mode_union = 2
  BoxColor_Gray = 0
  BoxColor_AsSelection = 1
  BoxColor_Custom = 2
  BoxColor_Modes = ( 'gray', 'as_selection', 'custom' )

  def __init__(self):
    anatomist.cpp.Action.__init__(self)
    self.mode = self.mode_basic
    gconf = a.config()
    self.useBoxHighlight = True
    self.useBoxHighlightIndividual = False
    self.boxSelectionColorMode = self.BoxColor_Gray
    self.boxSelectionCustomColor = anatomist.cpp.SelectFactory.HColor()
    self.boxSelectionCustomColor.r = 0.7
    self.boxSelectionCustomColor.g = 0.7
    self.boxSelectionCustomColor.b = 0.7
    self.boxSelectionCustomColor.a = 1.
    self.boxSelectionCustomColor.na = False
    if gconf.has_key( 'boxSelectionHighlight' ):
      self.useBoxHighlight = gconf[ 'boxSelectionHighlight' ]
    if gconf.has_key( 'boxSelectionIndividual' ):
      self.useBoxHighlightIndividual = gconf[ 'boxSelectionIndividual' ]
    if gconf.has_key( 'boxSelectionColorMode' ):
      self.boxSelectionColorMode = self.BoxColor_Modes.index( \
        gconf[ 'boxSelectionColorMode' ] )
    if gconf.has_key( 'boxSelectionCustomColor' ):
      col = gconf[ 'boxSelectionCustomColor' ]
      self.boxSelectionCustomColor.r = col[0]
      self.boxSelectionCustomColor.g = col[1]
      self.boxSelectionCustomColor.b = col[2]
      if len( col ) >= 4:
        self.boxSelectionCustomColor.a = col[3]
        self.boxSelectionCustomColor.na = False
      else:
        self.boxSelectionCustomColor.a = 1.
        self.boxSelectionCustomColor.na = True

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


  def boxSelectionColor( self, objs=None ):
    if self.boxSelectionColorMode == self.BoxColor_Gray:
      col = anatomist.cpp.SelectFactory.HColor()
      col.r = 0.7
      col.g = 0.7
      col.b = 0.7
      col.a = 1.
      col.na = False
      return col
    elif self.boxSelectionColorMode == self.BoxColor_Custom:
      return self.boxSelectionCustomColor
    else:
      sf = anatomist.cpp.SelectFactory.factory()
      for obj in objs:
        break
      col = sf.highlightColor( obj )
    return col


  def boxSelection( self ):
    if hasattr( self, '_recursing' ):
      return
    self._recursing = True
    a = anatomist.cpp.Anatomist()
    v = self.view()
    w = v.window()
    if self.useBoxHighlight:
      sf = anatomist.cpp.SelectFactory.factory()
      sel = sf.selected().get( w.Group() )
    else:
      sel = set()
    if not hasattr( self, '_selectboxes' ):
      self._selectboxes = {}
    if not sel:
      sel = []
    selectboxes = {}
    bboxglob = None
    globalbb = not self.useBoxHighlightIndividual
    def makefollower( self, objs, w, selectboxes, key ):
      if key in self._selectboxes:
        f = self._selectboxes[ key ]
        f.setObserved( objs )
      else:
        f = followerobject.ObjectFollowerCube( objs )
        f.setName( 'follower' )
        a.releaseObject( f )
      selectboxes[ key ] = f
      col = self.boxSelectionColor( objs )
      mat = f.GetMaterial()
      mat.set( { 'diffuse' : [ col.r, col.g, col.b, col.a ] } )
      f.SetMaterial( mat )
      w.registerObject( f, True )
      w.refreshTemp()
    tosel = []
    for obj in sel:
      if not w.hasObject( obj ) or w.isTemporary( obj ):
        continue
      tosel.append( obj )
      if not globalbb:
        if self._selectboxes.has_key( anatomist.cpp.weak_ptr_AObject( obj ) ):
          selectboxes[ anatomist.cpp.weak_ptr_AObject( obj ) ] \
            = self._selectboxes[ anatomist.cpp.weak_ptr_AObject( obj ) ]
          continue
        makefollower( self, [ obj ], w, selectboxes,
          anatomist.cpp.weak_ptr_AObject( obj ) )
    if globalbb:
      if tosel:
        makefollower( self, tosel, w, selectboxes, 'global' )
    self._selectboxes = selectboxes
    del self._recursing

  def switchBoxHighligting( self, value ):
    self.useBoxHighlight = value
    gconf = a.config()
    if value == 0:
      gconf[ 'boxSelectionHighlight' ] = 0
    elif gconf.has_key( 'boxSelectionHighlight' ):
      del gconf[ 'boxSelectionHighlight' ]
    self.boxSelection()

  def switchBoxHighligtingIndividual( self, value ):
    self.useBoxHighlightIndividual = value
    gconf = a.config()
    if value != 0:
      gconf[ 'boxSelectionIndividual' ] = 1
    elif gconf.has_key( 'boxSelectionIndividual' ):
      del gconf[ 'boxSelectionIndividual' ]
    self.boxSelection()

  def switchBoxColorMode( self, value ):
    self.boxSelectionColorMode = value
    gconf = a.config()
    if value != 0:
      gconf[ 'boxSelectionColorMode' ] = self.BoxColor_Modes[ value ]
    elif gconf.has_key( 'boxSelectionColorMode' ):
      del gconf[ 'boxSelectionColorMode' ]
    if hasattr( self, '_selectboxes' ):
      del self._selectboxes
    self.boxSelection()

  def selectCustomBoxColor( self ):
    sc = self.boxSelectionCustomColor
    alpha = int( sc.a * 255.99 )
    nalpha = sc.na
    col, alpha, nalpha = anatomist.cpp.QAColorDialog.getColor( \
      QtGui.QColor( int( sc.r * 255.99 ),
                    int( sc.g * 255.99 ),
                    int( sc.b * 255.99 ) ), None,
      'Selection color', alpha, nalpha )
    if col.isValid():
      hcol = anatomist.cpp.SelectFactory.HColor()
      hcol.r = float( col.red() ) / 255.99
      hcol.g = float( col.green() ) / 255.99
      hcol.b = float( col.blue() ) / 255.99
      hcol.a = float( alpha ) / 255.99
      hcol.na = nalpha
      self.boxSelectionCustomColor = hcol
      gconf = a.config()
      cval = [ hcol.r, hcol.g, hcol.b ]
      if not hcol.na:
        cval.append( hcol.a )
      gconf[ 'boxSelectionCustomColor' ] = cval
      if cval != [ 0.7, 0.7, 0.7, 1. ]:
        gconf[ 'boxSelectionCustomColor' ] = cval
      elif gconf.has_key( 'boxSelectionCustomColor' ):
        del gconf[ 'boxSelectionCustomColor' ]
      if hasattr( self, '_selectboxes' ):
        del self._selectboxes
      self.boxSelection()

  def selectionChanged( self ):
    self.edgeSelection()
    self.boxSelection()

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
    if hasattr( self, '_selectboxes' ):
      del self._selectboxes


class SelectionControl( anatomist.cpp.Select3DControl ):
  def __init__( self ):
    anatomist.cpp.Select3DControl.__init__( self, 'SelectionControl' )

  def eventAutoSubscription( self, pool ):
    anatomist.cpp.Select3DControl.eventAutoSubscription( self, pool )
    self.selectionChangedEventSubscribe( pool.action( \
      'SelectionAction' ).selectionChanged )

  def doAlsoOnSelect( self, actionpool ):
    anatomist.cpp.Select3DControl.doAlsoOnSelect( self, actionpool )
    actionpool.action("SelectionAction").edgeSelection()
    actionpool.action("SelectionAction").boxSelection()

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

