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

//only for debug
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/surface.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/glcomponent.h>

#include <anatomist/window3D/control3D.h>
#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/actionpool.h>
#include <anatomist/controler/control_d.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/commands/cWindowConfig.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/window3D/trackObliqueSlice.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window3D/transformer.h>
#include <anatomist/window3D/trackcut.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/window3D/labeleditaction.h>
#include <anatomist/window3D/annotedgraph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/application/globalConfig.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <aims/qtcompat/qtoolbutton.h>
#if QT_VERSION < 0x040000
#include <qtooltip.h>
#endif
#include <stdlib.h>

#include <anatomist/window/glwidget.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


Control * 
Control3D::creator( ) 
{
  return new Control3D ;
}


Control3D::Control3D() 
  : Control( 1, QT_TRANSLATE_NOOP( "ControlledWindow", "Default 3D control" ) )
{
}


Control3D::Control3D( const Control3D & c ) : Control( c )
{
}


Control3D::~Control3D()
{
}


void Control3D::eventAutoSubscription( ActionPool * actionPool )
{
  //cout << "Control3D::eventAutoSubscription\n";
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
	&LinkAction::execLink ), 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
	&LinkAction::execLink ), 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
	&LinkAction::endLink ), true );

  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

  //	rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::showRotationCenter ) );

  //	sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSync ) );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSyncOrientation ) );
  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ), 
        &ContinuousTrackball::beginTrackball ), 
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ), 
        &ContinuousTrackball::moveTrackball ), 
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ), 
        &ContinuousTrackball::endTrackball ), true );

  keyPressEventSubscribe( Qt::Key_Space, Qt::ControlButton, 
			  KeyActionLinkOf<ContinuousTrackball>
			  ( actionPool->action( "ContinuousTrackball" ), 
			    &ContinuousTrackball::startOrStop ) ) ;

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::toggleSelectAll ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::removeFromWindow ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::removeFromGroup ) );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::beginZoom ), 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::moveZoom ), 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::endZoom ), true );
  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ), 
                         &Zoom3DAction::zoomWheel ) );

  //	translation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::beginTranslate ), 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::moveTranslate ), 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::endTranslate ), true );

  // Slice action
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousSlice ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextSlice ) );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousTime ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextTime ) );
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::toggleLinkedOnSlider ) );

  // Movie action
  keyPressEventSubscribe( Qt::Key_Space, Qt::NoButton, 
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::startOrStop ) ) ;

  keyPressEventSubscribe( Qt::Key_S, Qt::ControlButton, 
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::sliceOn ) ) ;

  keyPressEventSubscribe( Qt::Key_T, Qt::ControlButton,
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::timeOn ) ) ;

  keyPressEventSubscribe( Qt::Key_M, Qt::ControlButton,
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::nextMode ) ) ;

  keyPressEventSubscribe( Qt::Key_Plus, Qt::NoButton,
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::increaseSpeed ) ) ;
  keyPressEventSubscribe( Qt::Key_Plus, Qt::ShiftButton,
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::increaseSpeed ) ) ;

  keyPressEventSubscribe( Qt::Key_Minus, Qt::NoButton,
			  KeyActionLinkOf<MovieAction>
			  ( actionPool->action( "MovieAction" ), 
			    &MovieAction::decreaseSpeed ) ) ;

  // Is it MY job to maintain this map ???
  myActions[ "MovieAction" ] = actionPool->action( "MovieAction" );
  myActions[ "ContinuousTrackball" ] 
    = actionPool->action( "ContinuousTrackball" );
}


void Control3D::doAlsoOnDeselect( ActionPool * )
{
  map<string, ActionPtr>::const_iterator	ia, ea=myActions.end();
  MovieAction					*a;
  ContinuousTrackball				*a2;

  for( ia=myActions.begin(); ia!=ea; ++ia )
    {
      a = dynamic_cast<MovieAction *>( ia->second );
      if( a && a->isRunning() )
	a->startOrStop();
      a2 = dynamic_cast<ContinuousTrackball *>( ia->second );
      if( a2 )
	a2->stop();
    }
}

// ----

Control * 
Select3DControl::creator( ) 
{
  return new Select3DControl ;
}

Select3DControl::Select3DControl( const string & name ) 
  : Control( 2, QT_TRANSLATE_NOOP( "ControlledWindow", name ) )
{
}


Select3DControl::Select3DControl( const Select3DControl & c ) : Control( c )
{
}


Select3DControl::~Select3DControl()
{
}


void Select3DControl::eventAutoSubscription( ActionPool * actionPool )
{
  //cout << "Select3DControl::eventAutoSubscription\n";
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ), 
               &SelectAction::execSelect ) );
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ShiftButton, 
      MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ), 
               &SelectAction::execSelectAdding ) );
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ControlButton, 
      MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ), 
               &SelectAction::execSelectToggling ) );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
             &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
        KeyActionLinkOf<WindowActions>
        ( actionPool->action( "WindowActions" ),
          &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
        KeyActionLinkOf<WindowActions>
        ( actionPool->action( "WindowActions" ),
          &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
        KeyActionLinkOf<WindowActions>
        ( actionPool->action( "WindowActions" ),
          &WindowActions::toggleShowTools ) );

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlButton, 
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::toggleSelectAll ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoButton, 
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::removeFromWindow ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlButton, 
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::removeFromGroup ) );

  //  rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
        KeyActionLinkOf<Trackball>
        ( actionPool->action( "Trackball" ),
          &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
        KeyActionLinkOf<Trackball>
        ( actionPool->action( "Trackball" ),
          &Trackball::showRotationCenter ) );

  //  sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
        KeyActionLinkOf<Sync3DAction>
        ( actionPool->action( "Sync3DAction" ),
          &Sync3DAction::execSync ) );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
            &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
            &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
            &Trackball::endTrackball ), true );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
               &Zoom3DAction::beginZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
               &Zoom3DAction::moveZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
               &Zoom3DAction::endZoom ), true );

  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ), 
                         &Zoom3DAction::zoomWheel ) );

  //  translation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
  &Translate3DAction::beginTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
  &Translate3DAction::moveTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
  &Translate3DAction::endTranslate ), true );

  // Slice action
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousSlice ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextSlice ) );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousTime ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextTime ) );
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::toggleLinkedOnSlider ) );

  // label picker / edition
  keyPressEventSubscribe( Qt::Key_Space, Qt::NoButton,
                          KeyActionLinkOf<LabelEditAction>
                          ( actionPool->action( "LabelEditAction" ),
                            &LabelEditAction::pick ) );
  keyPressEventSubscribe( Qt::Key_Return, Qt::ControlButton,
                          KeyActionLinkOf<LabelEditAction>
                              ( actionPool->action( "LabelEditAction" ),
                                &LabelEditAction::edit ) );

  // graph annotation
  keyPressEventSubscribe( Qt::Key_A, Qt::NoButton,
                          KeyActionLinkOf<AnnotationAction>
                          ( actionPool->action( "AnnotationAction" ),
                            &AnnotationAction::switchAnnotations ) );
}


void Select3DControl::doAlsoOnSelect( ActionPool* pool )
{
  Action *a = pool->action( "LabelEditAction" );
  if( !a )
    return;
  View  *v = a->view();
  if( !v )
    return;
  QAWindow  *aw = dynamic_cast<QAWindow *>( v->window() );
  if( !aw )
    return;
  QToolBar *tb = dynamic_cast<QToolBar *>( aw->child( "select3D_toolbar" ) );
  if( tb )
    return; // toolbar found: it already exists.
#if QT_VERSION >= 0x040000
  tb = aw->addToolBar( ControlledWindow::tr( "Selection tools" ),
                       "select3D_toolbar" );
  //d->toolbars.push_back( d->mute );
  tb->setIconSize( QSize( 20, 20 ) );
#else
  tb = new QToolBar( aw, "select3D_toolbar" );
  tb->setLabel( ControlledWindow::tr( "Selection tools" ) );
#endif
  new QLabel( ControlledWindow::tr( "Selection label" ), tb,
              "selectionLabel" );
  LabelEditAction *la = static_cast<LabelEditAction *>( a );
  la->setLabel( la->label() );
  tb->show();
}


void Select3DControl::doAlsoOnDeselect( ActionPool* pool )
{
  Action *a = pool->action( "LabelEditAction" );
  if( !a )
    return;
  View  *v = a->view();
  if( !v )
    return;
  QAWindow  *aw = dynamic_cast<QAWindow *>( v->window() );
  if( !aw )
    return;
#if QT_VERSION >= 0x040000
  QToolBar *tb = aw->removeToolBar( "select3D_toolbar" );
  delete tb;
#else
  QToolBar *tb = dynamic_cast<QToolBar *>( aw->child( "select3D_toolbar" ) );
  if( tb )
  {
    delete tb;
    return;
  }
  // selection toolbar not found: strange...
  cerr << "bug: selection toolbar not found\n";
#endif
}

// ----

Action * 
LinkAction::creator() 
{
  return new LinkAction ;
}

LinkAction::LinkAction() : Action()
{
}


LinkAction::LinkAction( const LinkAction & a ) : Action( a )
{
}


LinkAction::~LinkAction()
{
}


string LinkAction::name() const
{
  return( "LinkAction" );
}


QWidget* LinkAction::actionView()
{
  return( 0 );
}


bool LinkAction::viewableAction()
{
  return( false );
}


void LinkAction::execLink( int x, int y, int, int )
{
  View		*v = view();
  GLWidgetManager	*w = dynamic_cast<GLWidgetManager *>( v );

  if( !w )
    {
      cerr << "LinkAction operating on wrong view type -- error\n";
      return;
    }

  AWindow	*win = v->window();

  Point3df	pos;
  if( win->positionFromCursor( x, y, pos ) )
    {
      //cout << "Position : " << pos << endl;

      vector<float>	vp;
      vp.push_back( pos[0] );
      vp.push_back( pos[1] );
      vp.push_back( pos[2] );
      vp.push_back( win->GetTime() );
      LinkedCursorCommand	*c 
	= new LinkedCursorCommand( v->window(), vp );
      theProcessor->execute( c );
    }
}


void LinkAction::endLink( int, int, int, int )
{
}


// --------

Action * 
MenuAction::creator() 
{
  return new MenuAction ;
}


MenuAction::MenuAction() : Action()
{
}


MenuAction::MenuAction( const MenuAction & a ) : Action( a )
{
}


MenuAction::~MenuAction()
{
}


string MenuAction::name() const
{
  return( "MenuAction" );
}


QWidget* MenuAction::actionView()
{
  return( 0 );
}


bool MenuAction::viewableAction()
{
  return( false );
}


void MenuAction::execMenu( int x, int y, int, int )
{
  view()->window()->button3clicked( x, y );
}


// -------------------

Action * 
SelectAction::creator()
{
  return new SelectAction ;
}

SelectAction::SelectAction() : Action()
{
}


SelectAction::SelectAction( const SelectAction & a ) : Action( a )
{
}


SelectAction::~SelectAction()
{
}

string SelectAction::name() const
{
  return( "SelectAction" );
}


QWidget* SelectAction::actionView()
{
  return( 0 );
}


bool SelectAction::viewableAction()
{
  return( false );
}


void SelectAction::execSelect( int x, int y, int, int )
{
  select( x, y, SelectFactory::Normal );
}


void SelectAction::execSelectAdding( int x, int y, int, int )
{
  select( x, y, SelectFactory::Add );
}


void SelectAction::execSelectToggling( int x, int y, int, int )
{
  select( x, y, SelectFactory::Toggle );
}


void SelectAction::select( int x, int y, int modifier )
{
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "SelectAction operating on wrong view type -- error\n";
      return;
    }

  // new OpenGL-based selection (2010)
  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );
  if( w3 )
  {
    GlobalConfiguration   *cfg = theAnatomist->config();
    int glxsel = 0;
    try
    {
      Object  x = cfg->getProperty( "disableOpenGLSelection" );
      if( !x.isNull() )
        glxsel = (int) x->getScalar();
    }
    catch( ... )
    {
    }
    if( !glxsel )
    {
      AObject *obj = w3->objectAtCursorPosition( x, y );
      if( obj )
      {
        if( !w3->hasObject( obj ) )
        {
          // see if the objects belongs to a graph vertex/edge
          AObject::ParentList pl = obj->parents();
          AObject::ParentList::iterator ip, ep = pl.end();
          while( !pl.empty() )
          {
            ip = pl.begin();
            pl.erase( ip );
            if( ( dynamic_cast<AGraphObject *>( *ip )
              || (*ip)->parents().empty() ) && w3->hasObject( *ip ) )
            {
              obj = *ip;
              break;
            }
            pl.insert( (*ip)->parents().begin(), (*ip)->parents().end() );
          }
        }
        SelectFactory *sf = SelectFactory::factory();
        set<AObject *> so;
        so.insert( obj );
        sf->select( (SelectFactory::SelectMode) modifier, w3->Group(), so );
        sf->refresh();
        return;
      }
    }
  }

  // fallback to older selection
  Point3df      pos;
  if( w->positionFromCursor( x, y, pos ) )
  {
    /*vector<float>	vp;
    vp.push_back( pos[0] );
    vp.push_back( pos[1] );
    vp.push_back( pos[2] );
    SelectionCommand	*c
      = new SelectionCommand( w->window(), vp );
      theProcessor->execute( c );*/

    view()->window()->selectObject( pos[0], pos[1], pos[2],
                                    view()->window()->GetTime(),
                                    (SelectFactory::SelectMode) modifier );
  }
}


void SelectAction::toggleSelectAll()
{
  AWindow		*w = view()->window();
  set<AObject *>	obj = w->Objects();
  SelectFactory		*sf = SelectFactory::factory();
  bool			allsel = true;
  set<AObject *>::iterator	io, eo = obj.end();
  for( io=obj.begin(); allsel && io!=eo; ++io )
    if( !sf->isSelected( w->Group(), *io ) )
      allsel = false;

  if( allsel )
    sf->unselectAll( w->Group() );
  else
    sf->selectAll( w );
  SelectFactory::factory()->refresh();
}


void SelectAction::removeFromWindow()
{
  SelectFactory::factory()->removeFromThisWindow( view()->window() );
}


void SelectAction::removeFromGroup()
{
  SelectFactory::factory()->remove( view()->window() );
}


// --------

Action * 
Zoom3DAction::creator ()
{
  return new Zoom3DAction ;
}

Zoom3DAction::Zoom3DAction() : Action(), _beginpos( -1 ), _orgzoom( 1 )
{
}


Zoom3DAction::Zoom3DAction( const Zoom3DAction & a ) 
  : Action( a ), _beginpos( a._beginpos ), _orgzoom( a._orgzoom )
{
}


Zoom3DAction::~Zoom3DAction()
{
}


string Zoom3DAction::name() const
{
  return( "Zoom3DAction" );
}


QWidget* Zoom3DAction::actionView()
{
  return( 0 );
}


bool Zoom3DAction::viewableAction()
{
  return( false );
}


void Zoom3DAction::beginZoom( int, int y, int, int )
{
  //cout << "Zoom3DAction::beginZoom\n";
  _beginpos = y;

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Zoom3DAction operating on wrong view type -- error\n";
      return;
      _orgzoom = 1;
    }
  _orgzoom = w->zoom();
}


void Zoom3DAction::endZoom( int, int, int, int )
{
  endZoomKey();
}


void Zoom3DAction::endZoomKey()
{
  _beginpos = -1;

  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
  if (w && w3 && w3->surfpaintIsVisible())
    w->copyBackBuffer2Texture();
}


void Zoom3DAction::moveZoom( int, int y, int, int )
{
  //cout << "Zoom3DAction::moveZoom\n";
  if( _beginpos < 0 )
    return;

  int	m = _beginpos - y;
  float	zfac = exp( 0.01 * m );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  /*if( zfac < 1e-6 )
    zfac = 1e-6;*/
  //cout << "zoom factor : " << zfac << endl;

  if( !w )
    {
      cerr << "Zoom3DAction operating on wrong view type -- error\n";
      return;
    }

  if( w->perspectiveEnabled() )
    {
      const Quaternion	& q = w->quaternion().inverse();
      Point3df		p = q.apply( Point3df( 0, 0, -m ) );
      float zfac = w->invertedZ() ? -1 : 1;
      p[2] = zfac * p[2];	// invert Z axis
      //cout << "avance : " << p << endl;
      w->setRotationCenter( w->rotationCenter() + p );
      _beginpos = y;	// no memory of begin position in this mode
    }
  else
    w->setZoom( zfac * _orgzoom );
  ((AWindow3D *) w->window())->refreshLightViewNow();
}


void Zoom3DAction::zoomInOnce()
{
  zoom( 20 );
}


void Zoom3DAction::zoomOutOnce()
{
  zoom( -20 );
}


void Zoom3DAction::zoomWheel( int delta, int, int, int, int )
{
  zoom( delta/6 );
}


void Zoom3DAction::zoom( int distance )
{
  float	zfac = exp( 0.01 * distance );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    return;

  if( w->perspectiveEnabled() )
    {
      const Quaternion	& q = w->quaternion().inverse();
      Point3df		p = q.apply( Point3df( 0, 0, -distance ) );
      p[2] = -p[2];	// invert Z axis
      //cout << "avance : " << p << endl;
      w->setRotationCenter( w->rotationCenter() + p );
    }
  else
    w->setZoom( zfac * w->zoom() );
  ((AWindow3D *) w->window())->refreshLightViewNow();
}


// --------

Action * 
Translate3DAction::creator()
{
  return new Translate3DAction ;
}

Translate3DAction::Translate3DAction()
  : Action(), _started( false ), _beginx( 0 ), _beginy( 0 )
    
{
}


Translate3DAction::Translate3DAction( const Translate3DAction & a ) 
  : Action( a ), _started( a._started ), _beginx( a._beginx ), 
    _beginy( a._beginy )
{
}


Translate3DAction::~Translate3DAction()
{
}


string Translate3DAction::name() const
{
  return( "Translate3DAction" );
}


QWidget* Translate3DAction::actionView()
{
  return( 0 );
}


bool Translate3DAction::viewableAction()
{
  return( false );
}


void Translate3DAction::beginTranslate( int x, int y, int, int )
{
  //cout << "Translate3DAction::beginTranslate\n";
  _beginx = x;
  _beginy = y;

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Translate3DAction operating on wrong view type -- error\n";
      return;
      /*_orgx = 0;
	_orgy = 0;*/
    }
  /*Point2df	t = w->translation();
  _orgx = t[0];
  _orgy = t[1];*/
  _started = true;
}


void Translate3DAction::endTranslate( int, int, int, int )
{
  endTranslateKey();

  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
  if (w && w3 && w3->surfpaintIsVisible() )
    w->copyBackBuffer2Texture();
}


void Translate3DAction::endTranslateKey()
{
  //cout << "Translate3DAction::endTranslate\n";
  _started = false;
}

void Translate3DAction::moveTranslate( int x, int y, int, int )
{
  Point3df	t;
  
  //cout << "Translate3DAction::moveTranslate\n";
  if( !_started )
    return;

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Translate3DAction operating on wrong view type -- error\n";
      return;
    }

  w->translateCursorPosition( _beginx - x, y - _beginy, t );
  
  //cout << "transl : " << t << endl;
  w->setRotationCenter( w->rotationCenter() + t );
  _beginx = x;
  _beginy = y;

  ((AWindow3D *) w->window())->refreshLightViewNow();
}


// ----------

Action * 
Sync3DAction::creator() 
{
  return new Sync3DAction ;
}


Sync3DAction::Sync3DAction() : Action()
{
}


Sync3DAction::Sync3DAction( const Sync3DAction & a ) : Action( a )
{
}


Sync3DAction::~Sync3DAction()
{
}


string Sync3DAction::name() const
{
  return( "Sync3DAction" );
}


void Sync3DAction::execSync()
{
  AWindow3D	*win = dynamic_cast<AWindow3D *>( view()->window() );
  if( !win )
    {
      cerr << "Sync3DAction operating on wrong window type -- error\n";
      return;
    }
  win->syncViews();
}


void Sync3DAction::execSyncOrientation()
{
  AWindow3D	*win = dynamic_cast<AWindow3D *>( view()->window() );
  if( !win )
    {
      cerr << "Sync3DAction operating on wrong window type -- error\n";
      return;
    }
  win->syncViews( true );
}


QWidget* Sync3DAction::actionView()
{
  return( 0 );
}


bool Sync3DAction::viewableAction()
{
  return( false );
}


// --------

Control * 
FlightControl::creator( ) 
{
  return new FlightControl ;
}

FlightControl::FlightControl() 
  : Control( 4, QT_TRANSLATE_NOOP( "ControlledWindow", "Flight control" ) )
{
  setUserLevel( 2 );
}


FlightControl::FlightControl( const FlightControl & c ) : Control( c )
{
}


FlightControl::~FlightControl()
{
}



void FlightControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
				     &LinkAction::execLink ) );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

  //	rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::showRotationCenter ) );

  //	sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSync ) );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::beginTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::moveTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::endTrackball ), true );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::beginZoom ), 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::moveZoom ), 
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ), 
				       &Zoom3DAction::endZoom ), true );

  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ), 
                         &Zoom3DAction::zoomWheel ) );

  //	translation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::beginTranslate ), 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::moveTranslate ), 
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ), 
	&Translate3DAction::endTranslate ), true );

  //	Flight simulator

  keyRepetitiveEventSubscribe( Qt::Key_Up, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::up ), 
			       Qt::Key_Up, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Up, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::up ), 
			       Qt::Key_Up, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Down, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::down ), 
			       Qt::Key_Down, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Down, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::down ), 
			       Qt::Key_Down, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Left, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::left ), 
			       Qt::Key_Left, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Right, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::right ), 
			       Qt::Key_Right, Qt::ControlButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Left, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::spinLeft ), 
			       Qt::Key_Left, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Right, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::spinRight ), 
			       Qt::Key_Right, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Q, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::boost ), 
			       Qt::Key_Q, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_A, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::brake ), 
			       Qt::Key_A, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_F, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::runStep ), 
			       Qt::Key_G, Qt::NoButton, 
			       KeyActionLinkOf<KeyFlightAction>
			       ( actionPool->action( "KeyFlightAction" ), 
				 &KeyFlightAction::stop ), false, 1 );
  keyPressEventSubscribe( Qt::Key_R, Qt::NoButton, 
			  KeyActionLinkOf<KeyFlightAction>
			  ( actionPool->action( "KeyFlightAction" ), 
			    &KeyFlightAction::reverse ) );

  // Is it MY job to maintain this map ???
  myActions[ "KeyFlightAction" ] = actionPool->action( "KeyFlightAction" );
}


void FlightControl::doAlsoOnDeselect( ActionPool * )
{
  map<string, ActionPtr>::const_iterator	ia, ea=myActions.end();
  KeyFlightAction				*kf;

  for( ia=myActions.begin(); ia!=ea; ++ia )
    {
      kf = dynamic_cast<KeyFlightAction *>( (*ia).second );
      if( kf )
	kf->stop();
    }
}


// ------------

Control * ObliqueControl::creator()
{
  return new ObliqueControl;
}

ObliqueControl::ObliqueControl() 
  : Control( 3, QT_TRANSLATE_NOOP( "ControlledWindow", "ObliqueControl" ) )
{
}


ObliqueControl::ObliqueControl( const ObliqueControl & c ) : Control( c )
{
}


ObliqueControl::~ObliqueControl()
{
}



void ObliqueControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
				     &LinkAction::execLink ) );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

  //	rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::showRotationCenter ) );

  //	sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSync ) );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::beginTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::moveTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::endTrackball ), true );

  // Slice action
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousSlice ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextSlice ) );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousTime ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextTime ) );
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::toggleLinkedOnSlider ) );

  // oblique trackball

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ), 
				       &TrackOblique::beginTrackball ), 
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ), 
				       &TrackOblique::moveTrackball ), 
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ), 
				       &TrackOblique::endTrackball ), true );

  // oblique slice trackball

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ), 
	&TrackObliqueSlice::beginTrackball ), 
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ), 
	&TrackObliqueSlice::moveTrackball ), 
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ), 
	&TrackObliqueSlice::endTrackball ), true );
}


// -------------

Control * TransformControl::creator()
{
  return new TransformControl;
}

TransformControl::TransformControl() 
  : Control( 10, QT_TRANSLATE_NOOP( "ControlledWindow", "TransformControl" ) )
{
}


TransformControl::TransformControl( const TransformControl & c ) : Control( c )
{
}


TransformControl::~TransformControl()
{
}



void TransformControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
				     &LinkAction::execLink ) );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

  //	rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::showRotationCenter ) );

  //	sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSync ) );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::beginTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::moveTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::endTrackball ), true );

  // Slice action
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousSlice ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextSlice ) );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::previousTime ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::nextTime ) );
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlButton, 
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ), 
                            &SliceAction::toggleLinkedOnSlider ) );

  // Transformer trackball

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ), 
				      &Transformer::beginTrackball ), 
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ), 
				      &Transformer::moveTrackball ), 
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ), 
				      &Transformer::endTrackball ), true );
  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::begin ), 
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::move ), 
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::end ), 
      true );
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ControlButton, 
      MouseActionLinkOf<PlanarTransformer>
      ( actionPool->action( "PlanarTransformer" ), 
        &PlanarTransformer::beginTrackball ), 
      MouseActionLinkOf<PlanarTransformer>
      ( actionPool->action( "PlanarTransformer" ), 
        &PlanarTransformer::moveTrackball ), 
      MouseActionLinkOf<PlanarTransformer>
      ( actionPool->action( "PlanarTransformer" ), 
        &PlanarTransformer::endTrackball ), true );
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ShiftButton, 
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::begin ), 
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::move ), 
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::end ), 
      true );
}


// ------------

Control * CutControl::creator()
{
  return new CutControl;
}

CutControl::CutControl() 
  : Control( 30, QT_TRANSLATE_NOOP( "ControlledWindow", "CutControl" ) )
{
}


CutControl::CutControl( const CutControl & c ) : Control( c )
{
}


CutControl::~CutControl()
{
}



void CutControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ), 
				     &LinkAction::execLink ) );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::toggleSelectAll ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::removeFromWindow ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlButton, 
			  KeyActionLinkOf<SelectAction>
			  ( actionPool->action( "SelectAction" ), 
			    &SelectAction::removeFromGroup ) );

  //	rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton, 
			  KeyActionLinkOf<Trackball>
			  ( actionPool->action( "Trackball" ), 
			    &Trackball::showRotationCenter ) );

  //	sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSync ) );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltButton, 
			  KeyActionLinkOf<Sync3DAction>
			  ( actionPool->action( "Sync3DAction" ), 
			    &Sync3DAction::execSyncOrientation ) );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoButton, 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::beginTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::moveTrackball ), 
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ), 
				    &Trackball::endTrackball ), true );

  // zoom

  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ), 
                         &Zoom3DAction::zoomWheel ) );

  // oblique trackball

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftButton, 
      MouseActionLinkOf<TrackCutAction>
      ( actionPool->action( "TrackCutAction" ), 
	&TrackCutAction::beginTrackball ), 
      MouseActionLinkOf<TrackCutAction>
      ( actionPool->action( "TrackCutAction" ), 
	&TrackCutAction::moveTrackball ), 
      MouseActionLinkOf<TrackCutAction>
      ( actionPool->action( "TrackCutAction" ), 
	&TrackCutAction::endTrackball ), 
      true );
  keyPressEventSubscribe( Qt::Key_A, Qt::ShiftButton, 
			  KeyActionLinkOf<TrackCutAction>
			  ( actionPool->action( "TrackCutAction" ), 
			    &TrackCutAction::axialSlice ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::ShiftButton, 
			  KeyActionLinkOf<TrackCutAction>
			  ( actionPool->action( "TrackCutAction" ), 
			    &TrackCutAction::coronalSlice ) );
  keyPressEventSubscribe( Qt::Key_S, Qt::ShiftButton, 
			  KeyActionLinkOf<TrackCutAction>
			  ( actionPool->action( "TrackCutAction" ), 
			    &TrackCutAction::sagittalSlice ) );

  // oblique slice trackball

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlButton, 
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ), 
	&CutSliceAction::beginTrack ), 
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ), 
	&CutSliceAction::moveTrack ), 
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ), 
	&CutSliceAction::endTrack ), true );
}

//------------------

Action* 
MovieAction::creator()
{
  MovieAction * pa = new MovieAction( ) ;

  return( pa );
}

MovieAction::MovieAction()
  : QObject(), Action(), mySliceAndNotTime(false), myIsRunning(false), 
    myRunMode( Forward ), myForward( true ), myTimeInterval( 100 )
{
  myTimer = new QTimer( this ) ;
  myTimer->changeInterval( 100 ) ;
  myTimer->stop();	// don't start right now...
  
  QObject::connect( myTimer, SIGNAL(timeout()), this, SLOT(timeout()) ) ;  
} 

MovieAction::~MovieAction() 
{
  delete myTimer ;
}

void 
MovieAction::sliceOn()
{
  //cerr << "Slice On " << endl ;
  mySliceAndNotTime = true ;
}

void 
MovieAction::timeOn()
{
  //cerr << "Time On " << endl ;
  mySliceAndNotTime = false ;
}


void
MovieAction::nextMode()
{
  int		rm = (int) myRunMode + 1;
  if( rm > LoopBothWays )
    myRunMode = Forward;
  else
    myRunMode = (RunMode) rm;
  static const string	modes[] = { "Forward", "Backward", 
                                    "Loop forward", "Loop backward", 
                                    "Loop both ways" };
  cout << "Movie mode: " << modes[ myRunMode ] << endl << std::flush;
}


void
MovieAction::increaseSpeed()
{
  myTimeInterval /= 2;
  if( myTimeInterval == 0 )
    myTimeInterval = 1;
  cout << "speed: " << 1000. / myTimeInterval << " frames/sec\n";
}


void
MovieAction::decreaseSpeed()
{
  myTimeInterval *= 2;
  cout << "speed: " << 1000. / myTimeInterval << " frames/sec\n";
}


void 
MovieAction::startOrStop()
{
  //cerr << "Start or Stop" << endl ;
  if( ! myIsRunning )
    {
      //cerr << "Start" << endl ;
      myIsRunning = true ;
      myTimer->start( myTimeInterval, true ) ;
    }
  else
    {
      myIsRunning = false ;
      myTimer->stop( ) ;    
      //cerr << "Stop" << endl ;      
    }
}


void 
MovieAction::timeout()
{
  int sliderPosition, maxPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() ) ;
  if ( ! win )
    return ;
  
  if( mySliceAndNotTime || win->getTimeSliderMaxPosition() == 0 )
    {
      sliderPosition = win->getSliceSliderPosition() ;
      maxPosition = win->getSliceSliderMaxPosition();
    }
  else
    {
      sliderPosition = win->getTimeSliderPosition() ;
      maxPosition = win->getTimeSliderMaxPosition();
    }

  switch( myRunMode )
    {
    case Forward:
    case LoopForward:
      ++sliderPosition;
      break;
    case Backward:
    case LoopBackward:
      --sliderPosition;
      break;
    case LoopBothWays:
      if( myForward )
        ++sliderPosition;
      else
        --sliderPosition;
    }

  bool	stop = false;

  if( sliderPosition < 0 )
    if( maxPosition == 0 )
      stop = true;
    else
      switch( myRunMode )
        {
        case LoopBackward:
          sliderPosition = maxPosition;
          break;
        case LoopBothWays:
          sliderPosition = 1;
          myForward = true;
          break;
        default:
          stop = true;
        }
  else if( sliderPosition > maxPosition )
  {
    if( maxPosition == 0 )
      stop = true;
    else
      switch( myRunMode )
        {
        case LoopForward:
          sliderPosition = 0;
          break;
        case LoopBothWays:
          sliderPosition = maxPosition - 1;
          myForward = false;
          break;
        default:
          stop = true;
        }
  }

  if( stop )
    {
      myIsRunning = false ;
      myTimer->stop( ) ;
      return;
    }

  if( mySliceAndNotTime || win->getTimeSliderMaxPosition() == 0 )
    {
      win->setSliceSliderPosition( sliderPosition ) ;
      myTimer->start( myTimeInterval, true ) ;
    }
  else
    {
      win->setTimeSliderPosition( sliderPosition ) ;
      myTimer->start( myTimeInterval, true ) ;
    }
}


//----------------------------

SliceAction::SliceAction() : Action()
{
}


SliceAction::~SliceAction()
{
}


Action* SliceAction::creator()
{
  return new SliceAction;
}


void SliceAction::previousSlice()
{
  int sliderPosition ;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() );
  if ( ! win )
    return ;
  
  sliderPosition = win->getSliceSliderPosition();

  if( sliderPosition > 0 )
    win->setSliceSliderPosition( sliderPosition - 1 );
}


void SliceAction::nextSlice()
{
  int sliderPosition ;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() );
  if ( ! win )
    return ;
  
  sliderPosition = win->getSliceSliderPosition();

  if( sliderPosition < win->getSliceSliderMaxPosition() )
    win->setSliceSliderPosition( sliderPosition + 1 );
}


void SliceAction::previousTime()
{
  int sliderPosition ;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() );
  if ( ! win )
    return ;
  
  sliderPosition = win->getTimeSliderPosition();

  if( sliderPosition > 0 )
    win->setTimeSliderPosition( sliderPosition - 1 );
}


void SliceAction::nextTime()
{
  int sliderPosition ;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() );
  if ( ! win )
    return ;
  
  sliderPosition = win->getTimeSliderPosition();

  if( sliderPosition < win->getTimeSliderMaxPosition() )
    win->setTimeSliderPosition( sliderPosition + 1 );
}


void SliceAction::toggleLinkedOnSlider()
{
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->window() );
  if ( ! win )
    return ;
  bool	onoff = !win->linkedCursorOnSliderChange();
  cout << "toggleLinkedOnSlider: " << onoff << endl;
  win->setLinkedCursorOnSliderChange( onoff );
}


//----------------------------


DragObjectAction::DragObjectAction() : Action()
{
}


DragObjectAction::~DragObjectAction()
{
}


Action* DragObjectAction::creator()
{
  return new DragObjectAction;
}


namespace
{

  void dragObjectStart( DragObjectAction *ac, const set<AObject *> & so )
  {
    QAWindow	*qw = dynamic_cast<QAWindow *>( ac->view()->window() );
    if( qw && !so.empty() )
      {
        QDragObject *d = new QAObjectDrag( so, qw, "dragObject" );

        map<int, QPixmap>::const_iterator	ip
          = QObjectTree::TypeIcons.find( (*so.begin())->type() );
        if( ip != QObjectTree::TypeIcons.end() )
          d->setPixmap( (*ip).second );
        d->dragCopy();
      }
  }

}


void DragObjectAction::dragAll( int, int, int, int )
{
  set<AObject *>	so = view()->window()->Objects();
  dragObjectStart( this, so );
}


void DragObjectAction::dragSelected( int, int, int, int )
{
  AWindow		*w = view()->window();
  set<AObject *>	so = w->Objects();
  // filter selected objects
  SelectFactory	*sf = SelectFactory::factory();
  set<AObject *>::iterator	i = so.begin(), e = so.end(), j;
  while( i != e )
    if( !sf->isSelected( w->Group(), *i ) )
      {
        j = i;
        ++i;
        so.erase( j );
      }
    else
      ++i;
  dragObjectStart( this, so );
}


// ---------------

Action * 
WindowActions::creator() 
{
  return new WindowActions ;
}

WindowActions::WindowActions() : Action()
{
}


WindowActions::WindowActions( const WindowActions & a ) 
  : Action( a )
{
}


WindowActions::~WindowActions()
{
}


string WindowActions::name() const
{
  return "WindowActions";
}


QWidget* WindowActions::actionView()
{
  return 0;
}


bool WindowActions::viewableAction()
{
  return false;
}


void WindowActions::close()
{
  AWindow	*w = view()->window();
  w->close();
}


void WindowActions::toggleShowTools()
{
  AWindow	*w = view()->window();
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( w );
  p->value<Dictionary>()[ "show_toolbars" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


void WindowActions::toggleFullScreen()
{
  AWindow	*w = view()->window();
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( w );
  p->value<Dictionary>()[ "fullscreen" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


/* the following lines are needed in release mode (optimization -O3)
using gcc 4.2.2 of Mandriva 2008. Otherwise there are undefined symbols.
*/
template class Control::KeyActionLinkOf<WindowActions>;
template class Control::KeyActionLinkOf<SelectAction>;

