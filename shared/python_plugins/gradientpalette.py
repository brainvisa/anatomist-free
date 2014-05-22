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
import sys, os
try:
  try:
    from soma.qt_gui.qt_backend import init_matplotlib_backend
    init_matplotlib_backend()
  except:
    pass
  from soma.qtgui import gradwidget
  ok = True
except:
  print 'gradient widget plugin disabled because soma.qtgui.gradwidget ' \
  'cannot be imported.'
  ok = False

import anatomist.cpp as anatomist
from soma import aims
import sip, weakref
import PyQt4.QtCore as qt
import PyQt4.QtGui as qtgui
use_qstring = False
try:
  # test SIP and QString API
  import sip
  if sip.getapi( 'QString' ) < 2:
    use_qstring = True
except:
  pass


class GWManager( qt.QObject ):
  gradwidgets = set()

if ok:
  gwManager = GWManager()


class GradientPaletteWidget( qtgui.QWidget ):
  class GradientObserver( anatomist.Observer ):
    def __init__( self, gradpal ):
      anatomist.Observer.__init__( self )
      self._gradpal = weakref.proxy( gradpal )

    def registerObservable( self, obs ):
      anatomist.Observer.registerObservable( self, obs )
      self._gradpal._objects.append( anatomist.weak_ptr_AObject(obs) )

    def unregisterObservable( self, obs ):
      try:
        self._gradpal._objects.remove( obs )
      except:
        pass
      anatomist.Observer.unregisterObservable( self, obs )

    def update( self, observable, args ):
      self._gradpal.update( observable, args )

  class GradientObjectParamSelect( anatomist.ObjectParamSelectSip ):
    def __init__( self, objects, parent ):
      anatomist.ObjectParamSelect.__init__( self, objects, parent )

    def filter( self, obj ):
      glapi = obj.glAPI()
      if not glapi:
        return False
      if glapi.glNumTextures() > 0:
        te = glapi.glTexExtrema( 0 );
        if te.minquant.size() > 0 and te.maxquant.size() > 0:
          return True
      return False

  def __init__( self, objects, parent = None, name = None, flags = 0 ):
    qtgui.QWidget.__init__( self, parent, flags )
    self.setAttribute( qt.Qt.WA_DeleteOnClose, True )
    if name:
      self.setObjectName(name)
    lay = qtgui.QVBoxLayout( self )
    lay.setMargin( 5 )
    lay.setSpacing( 5 )
    self._objsel = GradientPaletteWidget.GradientObjectParamSelect( objects,
      self )
    self._gradw = gradwidget.GradientWidget( self, 'gradientwidget', '', 0, 1 )
    lay.addWidget( self._objsel )
    lay.addWidget( self._gradw )
    hb = qtgui.QWidget( self )
    lay.addWidget( hb )
    hblay = qtgui.QGridLayout( hb )
    hblay.setSpacing( 5 )
    savebtn = qtgui.QPushButton( self.tr( 'Save palette image...' ), hb )
    editbtn = qtgui.QPushButton( self.tr( 'Edit gradient information' ), hb  )
    keepbtn = qtgui.QPushButton( self.tr( 'Keep as static palette' ), hb )
    hblay.addWidget( savebtn, 0, 0 )
    hblay.addWidget( editbtn, 0, 1 )
    hblay.addWidget( keepbtn, 1, 0 )
    modecb = qtgui.QComboBox( hb )
    modecb.insertItem( 0, self.tr( 'RGB mode' ) )
    modecb.insertItem( 1, self.tr( 'HSV mode' ) )
    modecb.setCurrentIndex( 0 )
    hblay.addWidget( modecb, 1, 1 )
    hb.setSizePolicy( qtgui.QSizePolicy.Preferred, qtgui.QSizePolicy.Fixed )
    self._objects = []
    self._observer = GradientPaletteWidget.GradientObserver( self )
    self._gradw.setHasAlpha( True )
    self._changing = False
    o0 = None
    for obj in objects:
      if o0 is None:
        o0 = obj
      obj.addObserver( self._observer )
    self._initial = [] + self._objects
    self.connect( self._gradw, qt.SIGNAL( 'gradientChanged( QString )' ),
      self.gradientChanged )
    global gwManager
    gwManager.gradwidgets.add( self )
    if o0 is not None:
      self.update( o0, None )
    self._objsel.selectionStarts.connect( self.chooseObject )
    self._objsel.objectsSelectedSip.connect( self.objectsChosen )
    self.connect( savebtn, qt.SIGNAL( 'clicked()' ), self.save )
    self.connect( keepbtn, qt.SIGNAL( 'clicked()' ), self.makeStaticPalette )
    self.connect( editbtn, qt.SIGNAL( 'clicked()' ), self.editGradient )
    self.connect( modecb, qt.SIGNAL( 'activated(int)' ), self.setMode )

  def closeEvent( self, event):
    gwManager.gradwidgets.remove( self )
    for x in [] + self._objects:
      x.deleteObserver( self._observer )
    del self._observer
    qtgui.QWidget.closeEvent(self, event)

  def gradientChanged( self, s ):
    a = anatomist.Anatomist()
    paldim = 512
    pal = anatomist.APalette( 'CustomGradient', paldim )
    pdata = pal.volume()
    rgbp = self._gradw.fillGradient( paldim, True )
    rgb = rgbp.data()
    if sys.byteorder == 'little':
      for i in xrange(paldim):
        pdata.setValue( aims.AimsRGBA( ord(rgb[i*4+2]), ord(rgb[i*4+1]),
          ord(rgb[i*4]), ord(rgb[i*4+3]) ), i )
    else:
      for i in xrange(paldim):
        pdata.setValue( aims.AimsRGBA( ord(rgb[i*4+1]), ord(rgb[i*4+2]),
          ord(rgb[i*4+3]), ord(rgb[i*4]) ), i )
    gradientString = self._gradw.getGradientString()
    if use_qstring:
      gradientString = gradientString.toLocal8Bit().data()
    pal.header()[ "palette_gradients" ] = gradientString
    if self._gradw.isHsv():
      pal.header()[ "palette_gradients_mode" ] = 'HSV'
    else:
      pal.header()[ "palette_gradients_mode" ] = 'RGB'
    pal.update()
    for obj in self._objects:
      opal = obj.getOrCreatePalette()
      opal.setRefPalette( pal )
      opal.fill()
      obj.setPalette( opal )
      self._changing = True
      obj.notifyObservers()
      self._changing = False

  def update( self, observable, args ):
    if self._changing:
      return
    if len( self._objects ) > 0:
      obj = self._objects[0]
      pal = obj.getOrCreatePalette()
      glapi = obj.glAPI()
      if glapi:
        extr = glapi.glTexExtrema()
        sz = extr.maxquant[0] - extr.minquant[0]
        pmin = extr.minquant[0] + sz * pal.min1()
        pmax = extr.minquant[0] + sz * pal.max1()
        self._gradw.setBounds( pmin, pmax )
      self._changing = True
      if pal:
        rpal = pal.refPalette()
        try:
          grads = rpal.header()[ 'palette_gradients' ]
        except:
          grads = None
        if grads:
          try:
            gradmode = rpal.header()[ 'palette_gradients_mode' ]
          except:
            gradmode = None
          if gradmode == 'HSV':
            self._gradw.newHsv()
          else:
            self._gradw.newRgb()
          self._gradw.setGradient( grads )
        else:
          self._gradw.setGradient( '0;0;1;1#0;0;1;1#0;0;1;1#0.5;1' )
      else:
        self._gradw.setGradient( '0;0;1;1#0;0;1;1#0;0;1;1#0.5;1' )
      self._gradw.update()
      self._changing = False

  def chooseObject( self ):
    self._initial = [ x for x in self._initial \
      if x.get() and not sip.isdeleted( x.get() ) ]
    self._objsel.selectObjects( [ x.get() for x in self._initial ],
        [ x.get() for x in self._objects ] );

  @qt.pyqtSlot( 'const set_AObjectPtr &' )
  def objectsChosen( self, objects ):
    objects = self._objsel.selectedObjects()
    for x in [] + self._objects:
      x.deleteObserver( self._observer )
    for x in objects:
      x.addObserver( self._observer )
    self.update( None, None )

  def save( self ):
    if len( self._objects ) > 0:
      obj = self._objects[0]
      pal = obj.getOrCreatePalette().refPalette()
      apal = anatomist.AObjectConverter.anatomist( pal.volume() )
      apal.setName( pal.name() )
      a = anatomist.Anatomist()
      hp = a.anatomistHomePath()
      if use_qstring:
        hp = hp.toLocal8Bit().data()
      apal.setFileName( os.path.join( hp, 'rgb', pal.name() ) )
      anatomist.ObjectActions.saveStatic( [ apal ] )
      a = anatomist.Anatomist()
      a.releaseObject( apal )

  def editGradient( self ):
    d = qtgui.QDialog( )
    d.setModal(True)
    d.setObjectName( 'gradient' )
    d.setWindowTitle( self.tr( 'Gradient definition:' ) )

    l = qtgui.QVBoxLayout( d )
    t = qtgui.QTextEdit( d )
    l.addWidget( t )
    t.setWordWrapMode( qtgui.QTextOption.WrapAtWordBoundaryOrAnywhere )
    t.setText( self._gradw.getGradientString() )
    hb = qtgui.QWidget( d )
    l.addWidget( hb )
    hl = qtgui.QHBoxLayout( hb )
    ok = qtgui.QPushButton( self.tr( 'OK' ), hb )
    cc = qtgui.QPushButton( self.tr( 'Cancel' ), hb )
    hl.addWidget( ok )
    hl.addWidget( cc )
    ok.pressed.connect( d.accept )
    cc.pressed.connect( d.reject )
    res = d.exec_()
    if res:
      self._gradw.setGradient( t.toPlainText() )
      self._gradw.update()
      self.gradientChanged( t.toPlainText() )

  def makeStaticPalette( self ):
    obj = self._objects[0]
    a = anatomist.Anatomist()
    pall = a.palettes()
    pali = obj.getOrCreatePalette().refPalette()
    d = qtgui.QDialog( )
    d.setModal(True)
    d.setObjectName("paletteName")
    d.setWindowTitle(self.tr( 'Palette name:' ))
    l = qtgui.QVBoxLayout( d )
    t = qtgui.QLineEdit( pali.name(), d )
    l.addWidget( t )
    d.connect( t, qt.SIGNAL( 'returnPressed()' ), d.accept )
    res = d.exec_()

    if res:
      pal = anatomist.APalette( pali.get() )
      txt = t.text()
      if use_qstring:
        txt = txt.toLocal8Bit().data()
      pal.setName( txt )
      pall.push_back( pal )

  def setMode( self, mode ):
    if mode == 0:
      if self._gradw.isHsv():
        self._gradw.newRgb()
    else:
      if not self._gradw.isHsv():
        self._gradw.newHsv()


# ---

class GradientPaletteModule( anatomist.Module ):
  def name( self ):
    return 'GradientPalette'

  def description(self):
    return 'Gradient palette'


class GradientPaletteExtensionAction( anatomist.APaletteExtensionAction ):
  def __init__( self, icon, text, parent ):
    anatomist.APaletteExtensionAction.__init__( self, icon, text, parent )

  def extensionTriggered( self, objects ):
    w = GradientPaletteWidget( 
        objects, anatomist.Anatomist().getQWidgetAncestor(), None, 
        qt.Qt.Window )
    w.setAttribute( qt.Qt.WA_DeleteOnClose, True )
    w.show()


class GradientPaletteCallback( anatomist.ObjectMenuCallback ):
  def __init__( self ):
    anatomist.ObjectMenuCallback.__init__(self)

  def doit( self, objects ):
    w = GradientPaletteWidget( objects, anatomist.Anatomist().getQWidgetAncestor(), None, qt.Qt.Window  )
    w.setAttribute( qt.Qt.WA_DeleteOnClose, True )
    w.show()

callbacks_list = []

class GradientPaletteMenuRegistrer( anatomist.ObjectMenuRegistrerClass ):
  def doit( self, otype, menu ):
    glapi = otype.glAPI()
    if glapi and glapi.glNumTextures() != 0:
      if menu is None:
        menu = anatomist.ObjectMenu()
      gradpalette = GradientPaletteCallback()
      menu.insertItem( ['Color'], 'Palette editor', gradpalette )
    return menu


def init():
  r = GradientPaletteMenuRegistrer()
  callbacks_list.append( r )
  anatomist.AObject.addObjectMenuRegistration( r )
  apath = anatomist.Anatomist().anatomistSharedPath()
  import sip
  if use_qstring:
    apath = apath.toUtf8().data()
  icon = qtgui.QIcon( os.path.join(
    apath, 'icons', 'meshPaint', 'palette.png' ) )
  ac = GradientPaletteExtensionAction( icon, 'gradient', None )
  #ac.extensionTriggered.connect( openGradientPalette )
  anatomist.QAPaletteWin.addExtensionAction( ac )


if ok:
  gp = GradientPaletteModule()
  init()

del ok

