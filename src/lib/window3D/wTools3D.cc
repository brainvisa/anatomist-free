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
#include <aims/qtcompat/qvgroupbox.h>
#include <qcheckbox.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qradiobutton.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qbuttongroup.h>
#include <aims/qtcompat/qbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <iostream>

using namespace anatomist;
using namespace std;

struct Tools3DWindow::Private
{
  Private() : destroying( false ) {}

  QVButtonGroup	*renderfr;
  QCheckBox	*pmode;
  QCheckBox	*transmode;
  QCheckBox	*cull;
  QCheckBox	*flatpoly;
  QCheckBox	*smooth;
  QCheckBox	*fog;
  QVButtonGroup	*clip;
  QSpinBox	*cld;
  bool		destroying;
  QCheckBox	*cursor;
};


namespace
{

  void setButtonState( QButton* b, int x )
  {
    QCheckBox	*cb = dynamic_cast<QCheckBox *>( b );
    if( !cb )
      return;
#if QT_VERSION >= 0x040000
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
#else
    switch( x )
      {
      case 0:
        cb->setChecked( false );
        break;
      case 1:
        cb->setChecked( true );
        break;
      default:
        cb->setNoChange();
      }
#endif
  }

}


Tools3DWindow::Tools3DWindow( AWindow3D *win )
  : QWidget( 0, "tools3D", Qt::WDestructiveClose ), Observer(), 
    _window( win ), 
    d( new Private )
{
  win->addObserver( this );

  setCaption( tr( "3D settings : " ) + _window->Title().c_str() );
  QVBoxLayout	*mainlay = new QVBoxLayout( this, 10, 5 );

  QVGroupBox	*cubefr = new QVGroupBox( tr( "Helper gadgets :" ), this );
  d->cursor = new QCheckBox( tr( "Display cursor" ), cubefr );
  d->cursor->setTristate( true );
  setButtonState( d->cursor, win->hasSelfCursor() );
  QToolTip::add( d->cursor, 
                 tr( "Show/hide the linked cursor on the individual " 
                     "associated window. You can override the global default "
                     "(in the settings)" ) );
  QCheckBox	*cubeEn = new QCheckBox( tr( "Display orientation cube" ), 
					 cubefr );
  cubeEn->setChecked( _window->hasOrientationCube() );
  QCheckBox	*boundEn = new QCheckBox( tr( "Display bounding frame" ), 
					  cubefr );
  boundEn->setChecked( _window->hasBoundingFrame() );
  cubeEn->setEnabled( false );	// currently disabled (until we program it)
  boundEn->setEnabled( false );

  QVButtonGroup	*renderfr = new QVButtonGroup( tr( "Rendering mode :" ), 
					       this );
  d->renderfr = renderfr;
  QRadioButton	*r = new QRadioButton( tr( "Normal" ), renderfr );
  QToolTip::add( r, tr( "Normal rendering mode - you should almost always\n" 
			"use this one. It uses filled polygons") );
  r = new QRadioButton( tr( "Wireframe" ), renderfr );
  QToolTip::add( r, 
		 tr( "Only polygon edges are drawn. Hidden faces are also\n" 
		     "drawn, except for polygons facing back if culling\n" 
		     "is enabled" ) );
  r = new QRadioButton( tr( "Outlined (filled faces + wireframes)" ), 
			renderfr );
  QToolTip::add( r, tr( "Polygons are filled and polygon edges are re-drawn\n" 
			"using a different color (black). This mode is quite\n"
			"slow as two renderings are performed" ) );
  r = new QRadioButton( tr( "Wireframe with hidden faces" ), renderfr );
  QToolTip::add( r, tr( "Only polygon edges are drawn, and hidden faces are\n" 
			"actually hidden. To do so two renderings must be\n" 
			"performed like in outline mode (slow)" ) );
  QRadioButton	*rb = new QRadioButton( tr( "Fast rendering" ), renderfr );
  QToolTip::add( rb, tr( "Well, this mode doesn't exist in fact...\n"
			 "Maybe one day it will perform real-time polygon\n" 
			 "pruning to speed-up huge mesh rendering,\n" 
			 "but up to now..." ) );
  renderfr->setButton( _window->renderingMode() );
  rb->setEnabled( false );

  QCheckBox	*pmode = new QCheckBox( tr( "Use perspective" ), this );
  d->pmode = pmode;
  pmode->setChecked( _window->perspectiveEnabled() );
  QToolTip::add( pmode, 
		 tr( "Normal projection mode is orthogonal. In perspective\n" 
		     "mode, you can get a distance effect between the\n" 
		     "observer and objects. It is useful for the\n" 
		     "'flight simulator' control." ) );
  QCheckBox	*transmode 
    = new QCheckBox( tr( "Transparent objects are drawn in depth buffer" ), 
		     this );
  d->transmode = transmode;
  QToolTip::add( transmode, 
		 tr( "Enable this if you want to click on opaque objects\n" 
		     "through transparent ones, or if some superimposed\n" 
		     "transparent objects can't be seen. As a counterpart\n" 
		     "objects superimposition will not be handled correctly" 
		     ) );
  transmode->setChecked( _window->transparentZEnabled() );
  QCheckBox	*cull 
    = new QCheckBox( tr( "Cull polygon faces" ), this );
  d->cull = cull;
  QToolTip::add( cull, 
		 tr( "Culling avoids drawing polygons facing back.\n" 
		     "Without this options, transparent objects will look\n" 
		     "bad, but only the external side of open (or clipped)\n" 
		     "meshes will be seen, so it depends on what you're\n" 
		     "looking at" ) );
  cull->setChecked( _window->cullingEnabled() );
  QCheckBox	*flatpoly 
    = new QCheckBox( tr( "Flat-shaded polygons" ), this );
  d->flatpoly = flatpoly;
  QToolTip::add( flatpoly, 
		 tr( "Normal rendering uses smoothed colors polygons, so you\n"
		     "don't see polygon boundaries. It's nicer but if you\n" 
		     "are looking at exact details of where polygons are,\n" 
		     "you'll have to use either flat-shading, or a rendering\n"
		     "mode showing polygon edges (wireframe, outline, ...)" 
		     ) );
  flatpoly->setChecked( _window->flatShading() );
  QCheckBox	*smooth 
    = new QCheckBox( tr( "Smooth polygons / lines" ), this );
  d->smooth = smooth;
  QToolTip::add( smooth, 
		 tr( "This enables smooth filtering (anti-aliasing) of lines\n"
		     "and polygons edges, but doesn't work well in all\n" 
		     "cases (polygons should be sorted by depth), and it can\n"
		     "be very slow" ) );
  smooth->setChecked( _window->smoothing() );
  QCheckBox	*fog = new QCheckBox( tr( "Fog" ), this );
  d->fog = fog;
  QToolTip::add( fog, tr( "Use fog (useful only in perspective mode" ) );
  d->fog->setChecked( _window->fog() );

  QVButtonGroup	*clip = new QVButtonGroup( tr( "Clipping plane :" ), this );
  d->clip = clip;
  new QRadioButton( tr( "No clipping" ), clip );
  r = new QRadioButton( tr( "Single plane" ), clip );
  QToolTip::add( r, 
		 tr( "You can clip the whole scene to cut meshes on one or\n" 
		     "both sides of the current plane of the 2D window mode" 
		     ) );
  new QRadioButton( tr( "Clip both sides" ), clip );
  clip->setButton( (int) _window->clipMode() );
  QHBox		*clb = new QHBox( clip );
  clb->setSpacing( 10 );
  QLabel	*l 
    = new QLabel( tr( "Distance to current plane (mm) :" ), clb );
  QToolTip::add( l, 
		 tr( "This is the thickness of the slice between the current\n"
		     "2D window plane and the clipping plane(s). If you are\n" 
		     "using a volume (slice), don't set the distance to zero\n"
		     "because the slice will be just on the clipping plane,\n" 
		     "and you don't know what will be drawn and what won't be" 
		     ) );
  QSpinBox	*cld = new QSpinBox( clb );
  d->cld = cld;
  cld->setValue( (int) _window->clipDistance() );

  // depth peeling tag
  QVButtonGroup	*dpg = new QVButtonGroup( tr( "Transparency improvement:" ), 
					  this );
  QCheckBox	*dpeel = new QCheckBox( tr( "Use slower-but-better "
					    "transparency rendering algorithm" 
					    ), dpg );
  QHBox		*dppb = new QHBox( dpg );
  dppb->setSpacing( 10 );
  new QLabel( tr( "Number of rendering passes (0=as necessary)" ), dppb );
  QSpinBox	*dppass = new QSpinBox( dppb );

  GLWidgetManager	*da = static_cast<GLWidgetManager *>( win->view() );
  if( !da->depthPeelingAllowed() )
    dpg->setEnabled( false );
  dpeel->setChecked( da->depthPeelingEnabled() );
  dppass->setValue( (int) da->depthPeelingPasses() );

  // window saving modes
  QVButtonGroup	*wsg = new QVButtonGroup( tr( "Window saving modes:" ), this );
  QCheckBox	*wsrgb = new QCheckBox( tr( "RGB buffer" ), wsg );
  QCheckBox	*wsa = new QCheckBox( tr( "Alpha buffer" ), wsg );
  QCheckBox	*wsrgba = new QCheckBox( tr( "RGBA buffer" ), wsg );
  QCheckBox	*wsd = new QCheckBox( tr( "Depth buffer" ), wsg );
  QCheckBox	*wsl = new QCheckBox( tr( "Luminance buffer" ), wsg );

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
  connect( renderfr, SIGNAL( clicked( int ) ), this, 
	   SLOT( setRenderMode( int ) ) );
  connect( pmode, SIGNAL( toggled( bool ) ), this, 
	   SLOT( enablePerspective( bool ) ) );
  connect( clip, SIGNAL( clicked( int ) ), this, SLOT( setClipMode( int ) ) );
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
  connect( wsg, SIGNAL( clicked( int ) ), this, 
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
  cout << "Tools3DWindow::update\n";

  if( arg == 0 )
    {
      cout << "called obsolete Tools3DWindow::update( obs, NULL )\n";
      delete this;
      return;
    }

  blockSignals( true );

  d->renderfr->setButton( _window->renderingMode() );
  d->pmode->setChecked( _window->perspectiveEnabled() );
  d->transmode->setChecked( _window->transparentZEnabled() );
  d->cull->setChecked( _window->cullingEnabled() );
  d->flatpoly->setChecked( _window->flatShading() );
  d->smooth->setChecked( _window->smoothing() );
  d->clip->setButton( (int) _window->clipMode() );
  d->cld->setValue( (int) _window->clipDistance() );
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
  _window->setClipDistance( (float) d );
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
  cout << "toggleSavingMode " << x << endl;
  GLWidgetManager *da = static_cast<GLWidgetManager *>( _window->view() );
  int	savem = da->otherBuffersSaveMode();
  savem ^= ( 1 << x );
  da->setOtherBuffersSaveMode( savem );
  cout << "new saving mode: " << savem << endl;
}


void Tools3DWindow::setCursorVisibility( int x )
{
  switch( x )
    {
    case QCheckBox::On:
      x = 1;
      break;
    case QCheckBox::Off:
      x = 0;
      break;
    default:
      x = -1;
    }

  _window->setHasCursor( x );
  _window->Refresh();
}



