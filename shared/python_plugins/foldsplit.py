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

from __future__ import print_function

import anatomist.cpp as anatomist
from soma import aims, aimsalgo
import numpy
import os, sys
import soma.qt_gui.qt_backend.QtCore as qt
import soma.qt_gui.qt_backend.QtGui as qtgui


class SplitFoldModule( anatomist.Module ):
  def name( self ):
    return 'SplitFold'

  def description(self):
    return 'Split a fold graph vertex in two on a click'


class SplitFoldAction( anatomist.Action ):
  def name( self ):
    return 'SplitFoldAction'

  def split( self, x, y, globx, globy ):
    self.cleanup()
    a = anatomist.Anatomist()
    v = self.view()
    w = v.aWindow()
    pos = aims.Point3df()
    ok = v.positionFromCursor( x, y, pos )
    if not ok:
      return
    vertex = self.findVertex( pos, w )
    if vertex is None:
      return
    ag = None
    try:
      ag = vertex[ 'agraph' ]
    except:
      obj = vertex[ 'ana_object' ]
      ps = obj.parents()
      for p in ps:
        if p.type() == anatomist.AObject.GRAPH:
          ag = p
          break
    if ag is None:
      print('Vertex has no parent graph, strange. Aborting.')
      return
    g = ag.attributed()
    ag.loadSubObjects( 3 )
    fov = aims.FoldArgOverSegment( g )
    vs = g[ 'voxel_size' ]
    ro = ag.getReferential()
    rw = w.getReferential()
    tr = a.getTransformation( rw, ro )
    if tr is not None:
      pos = tr.transform( pos )
    posi = aims.Point3d( round( pos[0] / vs[0] ), round( pos[1] / vs[1] ),
      round( pos[2] / vs[2] ) )
    splitline = fov.findSplitLine( vertex, posi )
    if splitline:
      # show split line
      asplit = anatomist.AObjectConverter.anatomist( splitline )
      a.unmapObject( asplit )
      a.releaseObject( asplit )
      mat = asplit.GetMaterial()
      mat.set( { 'diffuse' : [ 0.6, 0., 0.8, 1. ] } )
      asplit.SetMaterial( mat )
      w.registerObject( asplit, True )

      self.temporary = { 'splitline' : splitline,
        'graph' : g,
        'vertex' : vertex,
        'tempobjects' : [ asplit ] }

  def subdivize( self, x, y, globx, globy ):
    self.cleanup()
    a = anatomist.Anatomist()
    v = self.view()
    w = v.aWindow()
    pos = aims.Point3df()
    ok = v.positionFromCursor( x, y, pos )
    if not ok:
      return
    vertex = self.findVertex( pos, w )
    if vertex is None:
      return
    ag = None
    try:
      ag = vertex[ 'agraph' ]
    except:
      obj = vertex[ 'ana_object' ]
      ps = obj.parents()
      for p in ps:
        if p.type() == anatomist.AObject.GRAPH:
          ag = p
          break
    if ag is None:
      print('Vertex has no parent graph, strange. Aborting.')
      return
    g = ag.attributed()
    ag.loadSubObjects( 3 )
    fov = aims.FoldArgOverSegment( g )
    newvert = aims.set_VertexPtr()
    nv = fov.subdivizeVertex( vertex, 20, 50, newvert )
    for vertex in newvert:
      try:
        bk = vertex[ 'aims_ss_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_bottom_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_other_ana' ]
        bk.setInternalsChanged()
      except:
        pass
    ag.updateAfterAimsChange()
    for vertex in newvert:
      av = vertex[ 'ana_object' ]
      w.registerObject( av )
    ag.notifyObservers()

  def subdivizeGraph( self, x, y, globx, globy ):
    self.cleanup()
    a = anatomist.Anatomist()
    v = self.view()
    w = v.aWindow()
    obj = w.Objects()
    graphs = []
    ag = None
    for o in obj:
      if o.type() == anatomist.AObject.GRAPH:
        graphs.append( o )
    if len( graphs ) == 1:
      ag = graphs[0]
    elif len( graphs ) >= 2:
      sf = anatomist.SelectFactory.factory()
      gr2 = []
      for o in graphs:
        if sf.isSelected( w.Group(), o ):
          gr2.append( o )
      if len( gr2 ) == 1:
        ag = gr2[0]
      del gr2
    del obj
    if ag is None:
      pos = aims.Point3df()
      ok = v.positionFromCursor( x, y, pos )
      if not ok:
        return
      vertex = self.findVertex( pos, w )
      if vertex is None:
        return
      try:
        ag = vertex[ 'agraph' ]
      except:
        obj = vertex[ 'ana_object' ]
        ps = obj.parents()
        for p in ps:
          if p.type() == anatomist.AObject.GRAPH:
            ag = p
            break
      if ag is None:
        print('Vertex has no parent graph, strange. Aborting.')
        return
    g = ag.attributed()
    ag.loadSubObjects( 3 )
    fov = aims.FoldArgOverSegment( g )
    newvert = aims.set_VertexPtr()
    nv = fov.subdivizeGraph( 20, 50, newvert )
    for vertex in newvert:
      try:
        bk = vertex[ 'aims_ss_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_bottom_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_other_ana' ]
        bk.setInternalsChanged()
      except:
        pass
    ag.updateAfterAimsChange()
    for vertex in newvert:
      av = vertex[ 'ana_object' ]
      w.registerObject( av )
    ag.notifyObservers()

  def cleanup( self ):
    if hasattr( self, 'temporary' ):
      del self.temporary

  def distanceToVertex( self, pos, vertex ):
    bck = []
    if vertex.has_key( 'aims_ss' ):
      bck.append( vertex[ 'aims_ss' ] )
    if vertex.has_key( 'aims_bottom' ):
      bck.append( vertex[ 'aims_bottom' ] )
    if vertex.has_key( 'aims_other' ):
      bck.append( vertex[ 'aims_other' ] )
    dmin = 1.e38
    pmin = None
    if len( bck ) != 0:
      vs = aims.Point3df( bck[0].sizeX(), bck[0].sizeY(), bck[0].sizeZ() )
      for bk in bck:
        for pt in bk[0].keys():
          d = ( aims.Point3df( pt[0] * vs[0], pt[1] * vs[1], pt[2] * vs[2] )\
                - pos ).norm2()
          if d < dmin:
            dmin = d
            pmin = pt
    return numpy.sqrt( dmin ), pmin

  def findVertex( self, pos, win, tolerance=4. ):
    existingvertex = None
    opos = None
    if hasattr( self, 'temporary' ):
      data = self.temporary
      if data.has_key( 'vertex' ):
        existingvertex = data[ 'vertex' ]
    if existingvertex:
      ro = data[ 'graph' ][ 'ana_object' ].getReferential()
      rw = win.getReferential()
      tr = anatomist.Anatomist().getTransformation( rw, ro )
      if tr is not None:
        opos = tr.transform( pos )
      else:
        opos = pos
      dmin, pmin = self.distanceToVertex( opos, existingvertex )
      if dmin < tolerance:
        return existingvertex
    obj = win.objectAt([pos[0], pos[1], pos[2], win.getTime()])
    if not obj:
      print('No object at that position')
      return
    if obj.type() != anatomist.AObject.GRAPHOBJECT:
      print('Not a Vertex:', obj)
      return
    vertex = obj.attributed()
    if vertex.getSyntax() != 'fold':
      if vertex.getSyntax() in \
        [ 'cortical', 'hull_junction', 'junction', 'plidepassage' ]:
        edge = vertex
        dmin = 1.e38
        vertex = None
        if opos is None:
          ro = obj.getReferential()
          rw = win.getReferential()
          tr = anatomist.Anatomist().getTransformation( rw, ro )
          if tr is not None:
            opos = tr.transform( pos )
          else:
            opos = pos
        for v in edge.vertices():
          d, pmin = self.distanceToVertex( opos, v )
          if d < dmin:
            dmin = d
            vertex = v
        if dmin < tolerance:
          return vertex
      print('Not a cortical fold:', obj, vertex.getSyntax())
      return
    return vertex

  def splitDotted( self, x, y, globx, globy ):
    a = anatomist.Anatomist()
    v = self.view()
    w = v.aWindow()
    pos = aims.Point3df()
    ok = v.positionFromCursor( x, y, pos )
    if not ok:
      return
    vertex = self.findVertex( pos, w )
    if vertex is None:
      return
    ag = None
    try:
      ag = vertex[ 'agraph' ]
    except:
      obj = vertex[ 'ana_object' ]
      ps = obj.parents()
      for p in ps:
        if p.type() == anatomist.AObject.GRAPH:
          ag = p
          break
    if ag is None:
      print('Vertex has no parent graph, strange. Aborting.')
      return
    g = ag.attributed()
    data = getattr( self, 'temporary', {} )
    if data.has_key( 'splitline' ):
      self.cleanup()
      data = {}
    elif data.has_key( 'graph' ):
      oldg = data[ 'graph' ]
      if oldg != g:
        print('different graph.')
        self.cleanup()
        data = {}
      else:
        if data.has_key( 'vertex' ):
          oldv = data[ 'vertex' ]
          if oldv != vertex:
            print('different vertex')
            self.cleanup()
            data = {}
    self.temporary = data
    data[ 'graph' ] = g
    data[ 'vertex' ] = vertex
    points = data.get( 'points', [] )
    vs = g[ 'voxel_size' ]
    ro = ag.getReferential()
    rw = w.getReferential()
    tr = a.getTransformation( rw, ro )
    if tr is not None:
      pos = tr.transform( pos )
    posi = aims.Point3d( round( pos[0] / vs[0] ), round( pos[1] / vs[1] ),
      round( pos[2] / vs[2] ) )
    points.append( posi )
    data[ 'points' ] = points
    sph = aims.SurfaceGenerator.icosahedron( pos, min( vs ) * 0.6 )
    asph = anatomist.AObjectConverter.anatomist( sph )
    asph.setReferentialInheritance( ag )
    a.unmapObject( asph )
    a.releaseObject( asph )
    mat = asph.GetMaterial()
    mat.set( { 'diffuse' : [ 0.8, 0.3, 0., 1. ] } )
    asph.SetMaterial( mat )
    w.registerObject( asph, True )
    spheres = data.get( 'tempobjects', [] )
    spheres.append( asph )
    data[ 'tempobjects' ] = spheres

  def doSplit( self ):
    if not hasattr( self, 'temporary' ):
      print('nothing to split yet. Use ctrl+left button to set split points.')
      return
    data = self.temporary
    if data.has_key( 'splitline' ): # split confirmed
      self.cleanup()
      splitline = data[ 'splitline' ]
      graph = data[ 'graph' ]
      vertex = data[ 'vertex' ]
      ag = graph[ 'ana_object' ]
      ag.loadSubObjects( 3 )
      fos = aims.FoldArgOverSegment( graph )
      newvertex = fos.splitVertex( vertex, splitline, 15 )
      if newvertex is None:
        print('Split failed.')
        return
      try:
        bk = vertex[ 'aims_ss_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_bottom_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      try:
        bk = vertex[ 'aims_other_ana' ]
        bk.setInternalsChanged()
      except:
        pass
      ag.updateAfterAimsChange()
      av = newvertex[ 'ana_object' ]
      w = self.view().aWindow()
      w.registerObject( av )
      sf = anatomist.SelectFactory.factory()
      sf.unselectAll( w.Group() )
      sf.select( w.Group(), [ av ] )
      ag.notifyObservers()
      sf.refresh()
    else: #  make split line from points
      if not data.has_key( 'graph' ) or not data.has_key( 'vertex' ) \
        or not data.has_key( 'points' ):
        print('missing split information. Use ctrl+left button to set split '
              'points.')
        return
      graph = data[ 'graph' ]
      vertex = data[ 'vertex' ]
      points = data[ 'points' ]
      if len( points ) == 0:
        print('no split points set. Use ctrl+left button to set split points.')
        return
      fos = aims.FoldArgOverSegment( graph )
      splitline = fos.findSplitLine( vertex, points )
      if splitline:
        data[ 'splitline' ] = splitline
        # show split line
        asplit = anatomist.AObjectConverter.anatomist( splitline )
        a.unmapObject( asplit )
        a.releaseObject( asplit )
        mat = asplit.GetMaterial()
        mat.set( { 'diffuse' : [ 0.6, 0., 0.8, 1. ] } )
        asplit.SetMaterial( mat )
        w = self.view().aWindow()
        w.registerObject( asplit, True )
        data[ 'tempobjects' ].append( asplit )


class SplitFoldControl( anatomist.Control ):
  def __init__( self, prio = 150 ):
    anatomist.Control.__init__( self, prio,
      qt.QT_TRANSLATE_NOOP( 'ControlledWindow', 'SplitFoldControl' ) )
    self.setUserLevel( 2 )

  def eventAutoSubscription( self, pool ):
    self.mousePressButtonEventSubscribe( \
      qt.Qt.LeftButton, qt.Qt.NoModifier,
      pool.action( 'SplitFoldAction' ).split )
    self.mousePressButtonEventSubscribe( \
      qt.Qt.RightButton, qt.Qt.ControlModifier,
      pool.action( 'SplitFoldAction' ).subdivize )
    self.mousePressButtonEventSubscribe( \
      qt.Qt.RightButton, qt.Qt.ShiftModifier,
      pool.action( 'SplitFoldAction' ).subdivizeGraph )
    self.mousePressButtonEventSubscribe( \
      qt.Qt.LeftButton, qt.Qt.ControlModifier,
      pool.action( 'SplitFoldAction' ).splitDotted )
    self.keyPressEventSubscribe( qt.Qt.Key_S, qt.Qt.NoModifier,
      pool.action( 'SplitFoldAction' ).doSplit )
    self.keyPressEventSubscribe( qt.Qt.Key_Escape, qt.Qt.NoModifier,
      pool.action( 'SplitFoldAction' ).cleanup )
    # std actions
    self.mousePressButtonEventSubscribe( qt.Qt.RightButton, qt.Qt.NoModifier,
      pool.action( "MenuAction" ).execMenu )
    self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.NoModifier,
      pool.action( "Trackball" ).beginTrackball,
      pool.action( "Trackball" ).moveTrackball,
      pool.action( "Trackball" ).endTrackball, True )
    self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.ShiftModifier,
      pool.action( "Zoom3DAction" ).beginZoom,
      pool.action( "Zoom3DAction" ).moveZoom,
      pool.action( "Zoom3DAction" ).endZoom, True )
    self.wheelEventSubscribe( pool.action( "Zoom3DAction" ).zoomWheel )
    self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.ControlModifier,
      pool.action( "Translate3DAction" ).beginTranslate,
      pool.action( "Translate3DAction" ).moveTranslate,
      pool.action( "Translate3DAction" ).endTranslate, True )

  def doAlsoOnSelect( self, actionpool ):
    anatomist.Control.doAlsoOnSelect( self, actionpool )
    actionpool.action( 'SplitFoldAction' ).cleanup()

  def doAlsoOnDeselect( self, actionpool ):
    anatomist.Control.doAlsoOnDeselect( self, actionpool )
    actionpool.action( 'SplitFoldAction' ).cleanup()

sf = SplitFoldModule()

a = anatomist.Anatomist()
pix = qtgui.QPixmap( a.anatomistSharedPath() + os.sep + 'icons' \
  + os.sep + 'control-foldsplit.png' )
anatomist.IconDictionary.instance().addIcon( 'SplitFoldControl', pix )
ad = anatomist.ActionDictionary.instance()
ad.addAction( 'SplitFoldAction', lambda: SplitFoldAction() )
cd = anatomist.ControlDictionary.instance()
cd.addControl( 'SplitFoldControl', lambda: SplitFoldControl(), 150 )
cm = anatomist.ControlManager.instance()
cm.addControl( 'QAGLWidget3D', '', 'SplitFoldControl' )

