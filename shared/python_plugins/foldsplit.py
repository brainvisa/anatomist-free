# Copyright IFR 49 (1995-2009)
#
#  This software and supporting documentation were developed by
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
import anatomist.cpp as anatomist
from soma import aims, aimsalgo
import numpy
import os, sys
if sys.modules.has_key( 'PyQt4' ):
  import PyQt4.QtCore as qt
  import PyQt4.QtGui as qtgui
else:
  import qt
  qtgui = qt


class SplitFoldModule( anatomist.Module ):
  def name( self ):
    return 'SplitFold'

  def description(self):
    return 'Split a fold graph vertex in two on a click'


class SplitFoldAction( anatomist.Action ):
  def name( self ):
    return 'SplitFoldAction'

  def split( self, x, y, globx, globy ):
    a = anatomist.Anatomist()
    v = self.view()
    w = v.window()
    pos = aims.Point3df()
    ok = v.positionFromCursor( x, y, pos )
    if not ok:
      return
    obj = w.objectAt( pos[0], pos[1], pos[2], w.GetTime() )
    if not obj:
      print 'No object at that position'
      return
    if obj.type() != anatomist.AObject.GRAPHOBJECT:
      print 'Not a Vertex:', obj
      return
    print 'pos:', pos
    vertex = obj.attributed()
    if vertex.getSyntax() != 'fold':
      print 'Not a cortical fold:', obj
      return
    ag = None
    try:
      ag = vertex[ 'agraph' ]
    except:
      ps = obj.parents()
      for p in ps:
        if p.type() == anatomist.AObject.GRAPH:
          ag = p
          break
    if ag is None:
      print 'Vertex has no parent graph, strange. Aborting.'
      return
    g = ag.attributed()
    ag.loadSubObjects( 3 )
    fov = aims.FoldArgOverSegment( g )
    vs = g[ 'voxel_size' ]
    posi = aims.Point3d( round( pos[0] / vs[0] ), round( pos[1] / vs[1] ),
      round( pos[2] / vs[2] ) )
    newvertex = fov.splitVertex( vertex, posi )
    if newvertex is None:
      print 'Split failed.'
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
    w.registerObject( av )
    sf = anatomist.SelectFactory.factory()
    sf.unselectAll( w.Group() )
    sf.select( w.Group(), [ av ] )
    ag.notifyObservers()
    sf.refresh()

  def subdivize( self, x, y, globx, globy ):
    a = anatomist.Anatomist()
    v = self.view()
    w = v.window()
    pos = aims.Point3df()
    ok = v.positionFromCursor( x, y, pos )
    if not ok:
      return
    obj = w.objectAt( pos[0], pos[1], pos[2], w.GetTime() )
    if not obj:
      print 'No object at that position'
      return
    if obj.type() != anatomist.AObject.GRAPHOBJECT:
      print 'Not a Vertex:', obj
      return
    print 'pos:', pos
    vertex = obj.attributed()
    if vertex.getSyntax() != 'fold':
      print 'Not a cortical fold:', obj
      return
    ag = None
    try:
      ag = vertex[ 'agraph' ]
    except:
      ps = obj.parents()
      for p in ps:
        if p.type() == anatomist.AObject.GRAPH:
          ag = p
          break
    if ag is None:
      print 'Vertex has no parent graph, strange. Aborting.'
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
    a = anatomist.Anatomist()
    v = self.view()
    w = v.window()
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
      obj = w.objectAt( pos[0], pos[1], pos[2], w.GetTime() )
      if not obj:
        print 'No object at that position'
        return
      if obj.type() != anatomist.AObject.GRAPHOBJECT:
        print 'Not a Vertex:', obj
        return
      print 'pos:', pos
      vertex = obj.attributed()
      if vertex.getSyntax() != 'fold':
        print 'Not a cortical fold:', obj
        return
      try:
        ag = vertex[ 'agraph' ]
      except:
        ps = obj.parents()
        for p in ps:
          if p.type() == anatomist.AObject.GRAPH:
            ag = p
            break
      if ag is None:
        print 'Vertex has no parent graph, strange. Aborting.'
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


class SplitFoldControl( anatomist.Control ):
  def __init__( self, prio = 150 ):
    anatomist.Control.__init__( self, prio, 'SplitFoldControl' )
    self.setUserLevel( 2 )

  def eventAutoSubscription( self, pool ):
    if sys.modules.has_key( 'PyQt4' ):
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.NoModifier,
        pool.action( 'SplitFoldAction' ).split )
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.ControlModifier,
        pool.action( 'SplitFoldAction' ).subdivize )
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.ShiftModifier,
        pool.action( 'SplitFoldAction' ).subdivizeGraph )
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
    else:
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.NoButton,
        pool.action( 'SplitFoldAction' ).split )
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.ControlButton,
        pool.action( 'SplitFoldAction' ).subdivize )
      self.mousePressButtonEventSubscribe( \
        qt.Qt.LeftButton, qt.Qt.ShiftButton,
        pool.action( 'SplitFoldAction' ).subdivizeGraph )
      # std actions
      self.mousePressButtonEventSubscribe( qt.Qt.RightButton, qt.Qt.NoButton,
        pool.action( "MenuAction" ).execMenu )
      self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.NoButton,
        pool.action( "Trackball" ).beginTrackball,
        pool.action( "Trackball" ).moveTrackball,
        pool.action( "Trackball" ).endTrackball, True )
      self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.ShiftButton,
        pool.action( "Zoom3DAction" ).beginZoom,
        pool.action( "Zoom3DAction" ).moveZoom,
        pool.action( "Zoom3DAction" ).endZoom, True )
      self.wheelEventSubscribe( pool.action( "Zoom3DAction" ).zoomWheel )
      self.mouseLongEventSubscribe( qt.Qt.MidButton, qt.Qt.ControlButton,
        pool.action( "Translate3DAction" ).beginTranslate,
        pool.action( "Translate3DAction" ).moveTranslate,
        pool.action( "Translate3DAction" ).endTranslate, True )


sf = SplitFoldModule()

a = anatomist.Anatomist()
pix = qtgui.QPixmap( a.anatomistSharedPath() + os.sep + qt.QString( 'icons' ) \
  + os.sep + 'control-foldsplit.png' )
anatomist.IconDictionary.instance().addIcon( 'SplitFoldControl', pix )
ad = anatomist.ActionDictionary.instance()
ad.addAction( 'SplitFoldAction', lambda: SplitFoldAction() )
cd = anatomist.ControlDictionary.instance()
cd.addControl( 'SplitFoldControl', lambda: SplitFoldControl(), 150 )
cm = anatomist.ControlManager.instance()
cm.addControl( 'QAGLWidget3D', '', 'SplitFoldControl' )

