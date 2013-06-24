/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


#include <anatomist/window3D/wTools3D.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <iostream>

using namespace anatomist;
using namespace std;

struct Tools3DWindow::Private
{
  Private() : destroying( false ) {}

  QGroupBox	*renderfr;
  QButtonGroup  *renderbg;
  QCheckBox	*pmode;
  QCheckBox	*transmode;
  QCheckBox	*cull;
  QCheckBox	*flatpoly;
  QCheckBox	*smooth;
  QCheckBox	*fog;
  QGroupBox	*clip;
  QButtonGroup  *clipbg;
  QSpinBox	*cld;
  bool		destroying;
  QCheckBox	*cursor;
};


namespace
{

  void setButtonState( QAbstractButton* b, int x )
  {
    QCheckBox	*cb = dynamic_cast<QCheckBox *>( b );
    if( !cb )
      return;
    switch( x )
    {
    case 0:
      cb->setCheckState( Qt::Unchecked );
      break;
    case 1:
      cb->setCheckState( Qt::Checked );
      break;
    default:
      cb->setCheckState( Qt::PartiallyChecked );
    }
  }

}


Tools3DWindow::Tools3DWindow( AWindow3D *win )
  : QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ), Observer(),
    _window( win ), 
    d( new Private )
{
  setObjectName("tools3D");
  setAttribute(Qt::WA_DeleteOnClose);
  win->addObserver( this );

  setWindowTitle( tr( "3D settings : " ) + _window->Title().c_str() );
  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setMargin( 10 );
  mainlay->setSpacing( 5 );

  QGroupBox	*cubefr = new QGroupBox( tr( "Helper gadgets :" ), this );
  QVBoxLayout *vlay = new QVBoxLayout( cubefr );
  d->cursor = new QCheckBox( tr( "Display cursor" ), cubefr );
  vlay->addWidget( d->cursor );
  d->cursor->setTristate( true );
  setButtonState( d->cursor, win->hasSelfCursor() );
  d->cursor->setToolTip(
    tr( "<p align=\"justify\">Show/hide the linked cursor on the individual " 
        "associated window. You can override the global default "
        "(in the settings)</p>" ) );
  QCheckBox	*cubeEn = new QCheckBox( tr( "Display orientation cube" ), 
                                         cubefr );
  vlay->addWidget( cubeEn );
  cubeEn->setChecked( _window->hasOrientationCube() );
  QCheckBox	*boundEn = new QCheckBox( tr( "Display bounding frame" ), 
                                          cubefr );
  vlay->addWidget( boundEn );
  boundEn->setChecked( _window->hasBoundingFrame() );
  cubeEn->setEnabled( false );	// currently disabled (until we program it)
  boundEn->setEnabled( false );
  vlay->addStretch( 1 );

  QGroupBox *renderfr = new QGroupBox( tr( "Rendering mode :" ), this );
  vlay = new QVBoxLayout( renderfr );
  QButtonGroup *renderbg = new QButtonGroup( renderfr );
  d->renderbg = renderbg;
  d->renderfr = renderfr;
  QRadioButton	*r = new QRadioButton( tr( "Normal" ), renderfr );
  vlay->addWidget( r );
  renderbg->addButton( r, 0 );
  r->setToolTip( 
    tr( "<p align=\"justify\">Normal rendering mode - you should almost "
        "always use this one. It uses filled polygons</p>") );
  r = new QRadioButton( tr( "Wireframe" ), renderfr );
  vlay->addWidget( r );
  renderbg->addButton( r, 1 );
  r->setToolTip( 
    tr( "<p align=\"justify\">Only polygon edges are drawn. Hidden faces are "
        "also drawn, except for polygons facing back if culling " 
        "is enabled</p>" ) );
  r = new QRadioButton( tr( "Outlined (filled faces + wireframes)" ), 
                        renderfr );
  vlay->addWidget( r );
  renderbg->addButton( r, 2 );
  r->setToolTip( 
    tr( "<p align=\"justify\">Polygons are filled and polygon edges are "
        "re-drawn using a different color (black). This mode is quite "
        "slow as two renderings are performed</p>" ) );
  r = new QRadioButton( tr( "Wireframe with hidden faces" ), renderfr );
  vlay->addWidget( r );
  renderbg->addButton( r, 3 );
  r->setToolTip( 
    tr( "<p align=\"justify\">Only polygon edges are drawn, and hidden faces "
        "are actually hidden. To do so two renderings must be " 
        "performed like in outline mode (slow)</p>" ) );
  QRadioButton	*rb = new QRadioButton( tr( "Fast rendering" ), renderfr );
  vlay->addWidget( rb );
  vlay->addStretch( 1 );
  renderbg->addButton( rb, 4 );
  rb->setToolTip( 
    tr( "<p align=\"justify\">Well, this mode doesn't exist in fact... "
        "Maybe one day it will perform real-time polygon " 
        "pruning to speed-up huge mesh rendering, " 
        "but up to now..." ) );
  renderbg->button( _window->renderingMode() )->setChecked( true );
  rb->setEnabled( false );

  QCheckBox	*pmode = new QCheckBox( tr( "Use perspective" ), this );
  d->pmode = pmode;
  pmode->setChecked( _window->perspectiveEnabled() );
  pmode->setToolTip(
    tr( "<p align=\"justify\">Normal projection mode is orthogonal. In "
        "perspective mode, you can get a distance effect between the " 
        "observer and objects. It is useful for the " 
        "'flight simulator' control.</p>" ) );
  QCheckBox	*transmode 
    = new QCheckBox( tr( "Transparent objects are drawn in depth buffer" ),
                     this );
  d->transmode = transmode;
  transmode->setToolTip(
    tr( "<p align=\"justify\">Enable this if you want to click on opaque "
        "objects through transparent ones, or if some superimposed " 
        "transparent objects can't be seen. As a counterpart " 
        "objects superimposition will not be handled correctly</p>" 
        ) );
  transmode->setChecked( _window->transparentZEnabled() );
  QCheckBox	*cull 
    = new QCheckBox( tr( "Cull polygon faces" ), this );
  d->cull = cull;
  cull->setToolTip(
    tr( "<p align=\"justify\">Culling avoids drawing polygons facing back. " 
        "Without this options, transparent objects will look " 
        "bad, but only the external side of open (or clipped) " 
        "meshes will be seen, so it depends on what you're " 
        "looking at</p>" ) );
  cull->setChecked( _window->cullingEnabled() );
  QCheckBox	*flatpoly 
    = new QCheckBox( tr( "Flat-shaded polygons" ), this );
  d->flatpoly = flatpoly;
  flatpoly->setToolTip(
    tr( "<p align=\"justify\">Normal rendering uses smoothed colors "
        "polygons, so you don't see polygon boundaries. It's nicer but if "
        "you are looking at exact details of where polygons are, " 
        "you'll have to use either flat-shading, or a rendering "
        "mode showing polygon edges (wireframe, outline, ...)</p>" 
        ) );
  flatpoly->setChecked( _window->flatShading() );
  QCheckBox	*smooth 
    = new QCheckBox( tr( "Smooth polygons / lines" ), this );
  d->smooth = smooth;
  smooth->setToolTip(
    tr( "<p align=\"justify\">This enables smooth filtering (anti-aliasing) "
        "of lines and polygons edges, but doesn't work well in all " 
        "cases (polygons should be sorted by depth), and it can "
        "be very slow</p>" ) );
  smooth->setChecked( _window->smoothing() );
  QCheckBox	*fog = new QCheckBox( tr( "Fog" ), this );
  d->fog = fog;
  fog->setToolTip( tr( "Use fog (useful only in perspective mode" ) );
  d->fog->setChecked( _window->fog() );

  QGroupBox *clip = new QGroupBox( tr( "Clipping plane :" ), this );
  vlay = new QVBoxLayout( clip );
  QButtonGroup *clipbg = new QButtonGroup( clip );
  d->clipbg = clipbg;
  d->clip = clip;
  r = new QRadioButton( tr( "No clipping" ), clip );
  vlay->addWidget( r );
  clipbg->addButton( r, 0 );
  r = new QRadioButton( tr( "Single plane" ), clip );
  vlay->addWidget( r );
  clipbg->addButton( r, 1 );
  r->setToolTip( 
    tr( "<p align=\"justify\">You can clip the whole scene to cut meshes on "
        "one or both sides of the current plane of the 2D window mode</p>"
      ) );
  r = new QRadioButton( tr( "Clip both sides" ), clip );
  vlay->addWidget( r );
  clipbg->addButton( r, 2 );
  clipbg->button( (int) _window->clipMode() )->setChecked( true );
  QWidget *clb = new QWidget( clip );
  vlay->addWidget( clb );
  QHBoxLayout *hlay = new QHBoxLayout( clb );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );
  QLabel *l = new QLabel( tr( "Distance to current plane (mm/100) :" ), 
                          clb );
  hlay->addWidget( l );
  l->setToolTip( 
    tr( "<p align=\"justify\">This is the thickness of the slice between the "
        "current 2D window plane and the clipping plane(s). If you are " 
        "using a volume (slice), don't set the distance to zero "
        "because the slice will be just on the clipping plane, " 
        "and you don't know what will be drawn and what won't be</p>" 
      ) );
  QSpinBox	*cld = new QSpinBox( clb );
  hlay->addWidget( cld );
  d->cld = cld;
  cld->setRange( 0, 100000 );
  cld->setSingleStep( 10 );
  cld->setValue( (int) ( _window->clipDistance() * 100 ) );
  vlay->addStretch( 1 );

  // depth peeling tag
  QGroupBox *dpg = new QGroupBox( tr( "Transparency improvement:" ), this );
  vlay = new QVBoxLayout( dpg );
  QCheckBox *dpeel = new QCheckBox( tr( "Use slower-but-better "
                                        "transparency rendering algorithm" 
                                      ), dpg );
  vlay->addWidget( dpeel );
  QWidget *dppb = new QWidget( dpg );
  vlay->addWidget( dppb );
  hlay = new QHBoxLayout( dppb );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( 
    "Number of rendering passes (0=as necessary)" ), dppb ) );
  QSpinBox	*dppass = new QSpinBox( dppb );
  hlay->addWidget( dppass );

  GLWidgetManager	*da = static_cast<GLWidgetManager *>( win->view() );
  if( !da->depthPeelingAllowed() )
    dpg->setEnabled( false );
  dpeel->setChecked( da->depthPeelingEnabled() );
  dppass->setValue( (int) da->depthPeelingPasses() );
  vlay->addStretch( 1 );

  // window saving modes
  QGroupBox	*wsg = new QGroupBox( tr( "Window saving modes:" ), this );
  vlay = new QVBoxLayout( wsg );
  QButtonGroup *wsgg = new QButtonGroup( wsg );
  wsgg->setExclusive( false );
  QCheckBox	*wsrgb = new QCheckBox( tr( "RGB buffer" ), wsg );
  vlay->addWidget( wsrgb );
  wsgg->addButton( wsrgb, 0 );
  QCheckBox	*wsa = new QCheckBox( tr( "Alpha buffer" ), wsg );
  vlay->addWidget( wsa );
  wsgg->addButton( wsa, 1 );
  QCheckBox	*wsrgba = new QCheckBox( tr( "RGBA buffer" ), wsg );
  vlay->addWidget( wsrgba );
  wsgg->addButton( wsrgba, 2 );
  QCheckBox	*wsd = new QCheckBox( tr( "Depth buffer" ), wsg );
  vlay->addWidget( wsd );
  wsgg->addButton( wsd, 3 );
  QCheckBox	*wsl = new QCheckBox( tr( "Luminance buffer" ), wsg );
  vlay->addWidget( wsl );
  wsgg->addButton( wsl, 4 );
  vlay->addStretch( 1 );

  int	savem = da->otherBuffersSaveMode();
  wsrgb->setChecked( savem & 1 );
  wsa->setChecked( savem & 2 );
  wsrgba->setChecked( savem & 4 );
  wsd->setChecked( savem & 8 );
  wsl->setChecked( savem & 16 );

  //

  mainlay->addWidget( cubefr );
  mainlay->addWidget( renderfr );
  mainlay->addWidget( pmode );
  mainlay->addWidget( transmode );
  mainlay->addWidget( cull );
  mainlay->addWidget( flatpoly );
  mainlay->addWidget( smooth );
  mainlay->addWidget( fog );
  mainlay->addWidget( clip );
  mainlay->addWidget( dpg );
  mainlay->addWidget( wsg );

  connect( d->cursor, SIGNAL( stateChanged( int ) ), this, 
	   SLOT( setCursorVisibility( int ) ) );
  connect( cubeEn, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableCube( bool ) ) );
  connect( boundEn, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableBoundingFrame( bool ) ) );
  connect( renderbg, SIGNAL( buttonClicked( int ) ), this, 
	   SLOT( setRenderMode( int ) ) );
  connect( pmode, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enablePerspective( bool ) ) );
  connect( clipbg, SIGNAL( buttonClicked( int ) ), 
           this, SLOT( setClipMode( int ) ) );
  connect( cld, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( setClipDistance( int ) ) );
  connect( transmode, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableTransparentZ( bool ) ) );
  connect( cull, SIGNAL( toggled( bool ) ), this, SLOT( setCulling( bool ) ) );
  connect( flatpoly, SIGNAL( toggled( bool ) ), this, 
	   SLOT( setFlatShading( bool ) ) );
  connect( smooth, SIGNAL( toggled( bool ) ), this, 
	   SLOT( setSmoothing( bool ) ) );
  connect( fog, SIGNAL( toggled( bool ) ), this, SLOT( setFog( bool ) ) );
  connect( dpeel, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enableDepthPeeling( bool ) ) );
  connect( dppass, SIGNAL( valueChanged( int ) ), this, 
	   SLOT( setDepthPeelingPasses( int ) ) );
  connect( wsgg, SIGNAL( buttonClicked( int ) ), this, 
	   SLOT( toggleSavingMode( int ) ) );
}


Tools3DWindow::~Tools3DWindow()
{
  d->destroying = true;
  _window->deleteObserver( this );
  _window->toolsWinDestroyed();
  cleanupObserver();
  //_window->deleteObserver( this );
  delete d;
}


void Tools3DWindow::update( const Observable*, void* arg )
{
  // cout << "Tools3DWindow::update\n";

  if( arg == 0 )
    {
      cout << "called obsolete Tools3DWindow::update( obs, NULL )\n";
      delete this;
      return;
    }

  blockSignals( true );

  d->renderbg->button( _window->renderingMode() )->setChecked( true );
  d->pmode->setChecked( _window->perspectiveEnabled() );
  d->transmode->setChecked( _window->transparentZEnabled() );
  d->cull->setChecked( _window->cullingEnabled() );
  d->flatpoly->setChecked( _window->flatShading() );
  d->smooth->setChecked( _window->smoothing() );
  d->clipbg->button( (int) _window->clipMode() )->setChecked( true );
  d->cld->setValue( (int) ( _window->clipDistance() * 100 ) );
  d->smooth->setChecked( _window->smoothing() );
  setButtonState( d->cursor, _window->hasSelfCursor() );

  blockSignals( false );
}


void Tools3DWindow::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  if( !d->destroying )
    delete this;
}


void Tools3DWindow::enableCube( bool state )
{
  _window->setOrientationCube( state );
  _window->setChanged();
  _window->notifyObservers( this );
  _window->Refresh();
}


void Tools3DWindow::enableBoundingFrame( bool state )
{
  _window->setBoundingFrame( state );
  _window->setChanged();
  _window->notifyObservers( this );
  _window->Refresh();
}


void Tools3DWindow::setRenderMode( int mode )
{
  _window->setRenderingMode( (AWindow3D::RenderingMode) mode );
  _window->setChanged();
  _window->notifyObservers( this );
  _window->Refresh();
}


void Tools3DWindow::enablePerspective( bool state )
{
  _window->enablePerspective( state );
  _window->Refresh();
}


void Tools3DWindow::setClipMode( int m )
{
  _window->setClipMode( (AWindow3D::ClipMode) m );
  _window->Refresh();
  ( (GLWidgetManager *) _window->view() )->copyBackBuffer2Texture();
}


void Tools3DWindow::setClipDistance( int d )
{
  _window->setClipDistance( (float) d * 0.01 );
  _window->Refresh();
  ( (GLWidgetManager *) _window->view() )->copyBackBuffer2Texture();
}


void Tools3DWindow::enableTransparentZ( bool x )
{
  _window->enableTransparentZ( x );
  _window->Refresh();
}


void Tools3DWindow::setCulling( bool x )
{
  _window->setCulling( x );
  _window->Refresh();
}


void Tools3DWindow::setFlatShading( bool x )
{
  _window->setFlatShading( x );
  _window->Refresh();
}


void Tools3DWindow::setSmoothing( bool x )
{
  _window->setSmoothing( x );
  _window->Refresh();
}


void Tools3DWindow::setFog( bool x )
{
  _window->setFog( x );
  _window->Refresh();
}


void Tools3DWindow::enableDepthPeeling( bool x )
{
  GLWidgetManager *da = static_cast<GLWidgetManager *>( _window->view() );
  da->enableDepthPeeling( x );
}


void Tools3DWindow::setDepthPeelingPasses( int n )
{
  GLWidgetManager *da = static_cast<GLWidgetManager *>( _window->view() );
  da->setDepthPeelingPasses( (unsigned) n );
}


void Tools3DWindow::toggleSavingMode( int x )
{
  GLWidgetManager *da = static_cast<GLWidgetManager *>( _window->view() );
  int	savem = da->otherBuffersSaveMode();
  savem ^= ( 1 << x );
  da->setOtherBuffersSaveMode( savem );
}


void Tools3DWindow::setCursorVisibility( int x )
{
  switch( x )
    {
    case Qt::Checked:
      x = 1;
      break;
    case Qt::Unchecked:
      x = 0;
      break;
    default:
      x = -1;
    }

  _window->setHasCursor( x );
  _window->Refresh();
}



