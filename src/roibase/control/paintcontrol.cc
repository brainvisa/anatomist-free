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

#include <anatomist/application/roibasemodule.h>
#include <anatomist/control/paintcontrol.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/controler/actionpool.h>
#include <anatomist/controler/control_d.h>
#include <anatomist/controler/view.h>
#include <anatomist/controler/controlswitch.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/misc/error.h>
#include <qlabel.h>
#include <qobject.h>
#include <qcursor.h>
#include <iostream>


using namespace anatomist ;
using namespace carto;
using namespace std;

// PaintControlObserver::PaintControlObserver( QWidget * parent, 
// 					    Control * subject )
//   : ControlObserver( parent, subject )
// {}


struct PaintControl::Private
{
  static map<AWindow3D*, AWindow3D::ObjectModifier *> & modifiers()
  {
    static map<AWindow3D*, AWindow3D::ObjectModifier *>	m;
    return m;
  }
};


Control *
PaintControl::creator( ) 
{
  PaintControl * pc = new PaintControl() ;
  
  return ( pc ) ;
}


PaintControl::PaintControl( ) 
  : Control( 101, QT_TRANSLATE_NOOP( "ControlledWindow", "PaintControl" ) ), 
    d( new Private ), myPaintAction(0)
{
  
}

PaintControl::PaintControl( const PaintControl& c) 
  : Control(c), d( new Private ), myPaintAction( 0 )
{
  
}

PaintControl::~PaintControl() 
{
  if(d)
    delete d;
}

void 
PaintControl::eventAutoSubscription( ActionPool * actionPool )
{
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );
  
  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::ControlButton,  // WARNING
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoButton, 
			  KeyActionLinkOf<WindowActions>
			  ( actionPool->action( "WindowActions" ), 
			    &WindowActions::toggleShowTools ) );


  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ShiftButton, 
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::fill ) );
  
  keyPressEventSubscribe( Qt::Key_S, Qt::ControlButton, 
			  KeyActionLinkOf<RoiManagementAction>
			  ( actionPool->action( "RoiManagementAction" ), 
			    &RoiManagementAction::saveGraph ) ) ;
  
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoButton, 
			  KeyActionLinkOf<RoiManagementAction>
			  ( actionPool->action( "RoiManagementAction" ), 
			    &RoiManagementAction::reloadGraph ) ) ;

  keyPressEventSubscribe( Qt::Key_P, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::brushToPoint ) ) ;

  keyPressEventSubscribe( Qt::Key_B, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::brushToBall ) ) ;

  keyPressEventSubscribe( Qt::Key_D, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::brushToDisk ) ) ;

  keyPressEventSubscribe( Qt::Key_Plus, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::increaseBrushSize ) ) ;

  keyPressEventSubscribe( Qt::Key_Minus, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::decreaseBrushSize ) ) ;

  keyPressEventSubscribe( Qt::Key_U, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::undo ) ) ;
  
  keyPressEventSubscribe( Qt::Key_Z, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::undo ) ) ;
  
  keyPressEventSubscribe( Qt::Key_R, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::redo ) ) ;

  keyPressEventSubscribe( Qt::Key_Z, (Qt::ButtonState ) 
                          ( Qt::ShiftButton | Qt::ControlButton ), 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::redo ) ) ;

  keyPressEventSubscribe( Qt::Key_E, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::clearRegion ) ) ;
  
  keyPressEventSubscribe( Qt::Key_L, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::lineOn ) ) ;

  keyPressEventSubscribe( Qt::Key_L, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::lineOff ) ) ;
  
  keyPressEventSubscribe( Qt::Key_C, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::followingLinkedCursorOn ) ) ;

  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::followingLinkedCursorOff ) ) ;

  keyPressEventSubscribe( Qt::Key_R, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::replaceOn ) ) ;

  keyPressEventSubscribe( Qt::Key_R, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::replaceOff ) ) ;

  keyPressEventSubscribe( Qt::Key_M, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::brushToMm ) ) ;

  keyPressEventSubscribe( Qt::Key_V, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::brushToVoxel ) ) ;

  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::copyPreviousSliceCurrentRegion ) ) ;
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::copyPreviousSliceWholeSession ) ) ;
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ControlButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::copyNextSliceCurrentRegion ) ) ;
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::copyNextSliceWholeSession ) ) ;

  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::paintStart ),
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::paint ),
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::validateChange ), true ) ;
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ControlButton, 
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::eraseStart ),
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				     &PaintAction::erase ),
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ), 
				      &PaintAction::validateChange ), true ) ;

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
	&Translate3DAction::endTranslate ), true ) ;

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

  // paint cursor action
  mouseMoveEventSubscribe
    ( Qt::NoButton, Qt::NoButton,
      MouseActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ),
                                      &PaintAction::moveCursor ) );
  focusOutEventSubscribe(
    FocusActionLinkOf<PaintAction>( actionPool->action( "PaintAction" ),
                                    &PaintAction::hideCursor ) );

  // Renaud : Pas top, mais en attendant mieux...
  myPaintAction = dynamic_cast<PaintAction*>( actionPool->action( "PaintAction" ) ) ;
}


namespace
{

  class GhostSelected : public AWindow3D::ObjectModifier
  {
  public:
    GhostSelected( AWindow3D* w )
      : AWindow3D::ObjectModifier( w )
    {
    }

    virtual ~GhostSelected() {}
    virtual void modify( AObject* o, GLPrimitives & p );
  };

  void GhostSelected::modify( AObject* o, GLPrimitives & p )
  {
    bool	ghost = false;
    if( window()->isTemporary( o ) )
      ghost = true;
    else
      {
        AObject::ParentList	& pl = o->Parents();
        AGraph	*g = RoiChangeProcessor::instance()->getGraph( window() );
        if( g && pl.find( g ) != pl.end() )
          ghost = true;
      }
    if( ghost )
      {
        GLItemList	*gil = new GLItemList;
        gil->items.insert( gil->items.end(), p.begin(), p.end() );
        gil->setGhost( true );
        p.clear();
        p.push_back( RefGLItem( gil ) );
      }
  }

}


void 
PaintControl::doAlsoOnDeselect ( ActionPool * /*pool*/ )
{
  if(myPaintAction)
    {
      myPaintAction->changeCursor( false ) ;
      myPaintAction->hideCursor();
      //cout << "Cursor : Arrow" <<endl ;
      AWindow3D	*w3 
        = dynamic_cast<AWindow3D *>( myPaintAction->view()->window() );
      if( w3 )
        {
          w3->enableToolTips( true );
          map<AWindow3D *,AWindow3D::ObjectModifier *>::iterator 
            m = d->modifiers().find( w3 );
          if( m != d->modifiers().end() )
            {
              delete m->second;
              d->modifiers().erase( m );
            }
        }
    }
}



void 
PaintControl::doAlsoOnSelect( ActionPool * /*pool*/ )
{
  if(myPaintAction)
    {
      myPaintAction->changeCursor( true ) ;

      AWindow3D	*w3 
        = dynamic_cast<AWindow3D *>( myPaintAction->view()->window() );
      if( w3 )
      {
        d->modifiers()[ w3 ] = new GhostSelected( w3 );
        w3->enableToolTips( false );
      }

      ControlSwitch	*cs = myPaintAction->view()->controlSwitch();
      if( !cs->isToolBoxVisible() )
        {
          // this is not a very elegant way of doing it...
          set<AWindow *>		w = theAnatomist->getWindows();
          set<AWindow *>::iterator	iw, ew = w.end();
          AWindow3D			*w3;
          bool				visible = false;
          for( iw=w.begin(); iw!=ew; ++iw )
            {
              w3 = dynamic_cast<AWindow3D *>( *iw );
              if( w3 && w3->view()->controlSwitch()->isToolBoxVisible() )
                {
                  visible = true;
                  break;
                }
            }
          if( !visible )
            cs->switchToolBoxVisible();
        }
      RoiManagementAction	rma;
      if( cs->toolBox() )
        cs->toolBox()->showPage( rma.name() );
    }
}

