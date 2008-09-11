/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#include <anatomist/application/roialgomodule.h>
#include <anatomist/action/morphomath.h>
#include <anatomist/control/dynsegmentcontrol.h>
#include <anatomist/action/dynsegmentaction.h>
#include <anatomist/action/paintaction.h>
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

using namespace anatomist ;
using namespace std ;

Control *
RoiDynSegmentControl::creator( ) 
{
  RoiDynSegmentControl * pc = new RoiDynSegmentControl() ;
  
  return ( pc ) ;
}


RoiDynSegmentControl::RoiDynSegmentControl( ) : Control( 106, "DynSegmentControl" )
{
  
}

RoiDynSegmentControl::RoiDynSegmentControl( const RoiDynSegmentControl& c) : Control(c)
{
  
}

RoiDynSegmentControl::~RoiDynSegmentControl() 
{

}

std::string 
RoiDynSegmentControl::name() const 
{ 
  return QT_TRANSLATE_NOOP( "ControlledWindow", "DynSegmentControl" ) ; 
}

void 
RoiDynSegmentControl::eventAutoSubscription( ActionPool * actionPool )
{
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

  // Level Set
  
  keyPressEventSubscribe( Qt::Key_2, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::dimensionModeTo2D ) ) ;

  keyPressEventSubscribe( Qt::Key_3, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::dimensionModeTo3D ) ) ;

  keyPressEventSubscribe( Qt::Key_O, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::increaseOrder ) ) ;

  keyPressEventSubscribe( Qt::Key_O, Qt::ControlButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::decreaseOrder ) ) ;
  
  keyPressEventSubscribe( Qt::Key_F, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::increaseFaithInterval ) ) ;

  keyPressEventSubscribe( Qt::Key_F, Qt::ControlButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::decreaseFaithInterval ) ) ;
  
  keyPressEventSubscribe( Qt::Key_R, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::refineModeOn ) ) ;

  keyPressEventSubscribe( Qt::Key_R, Qt::ControlButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::refineModeOff ) ) ;

  keyPressEventSubscribe( Qt::Key_M, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::findNearestMinimumModeOn ) ) ;

  keyPressEventSubscribe( Qt::Key_M, Qt::ControlButton, 
			  KeyActionLinkOf<RoiDynSegmentAction>
			  ( actionPool->action( "DynSegmentAction" ), 
			    &RoiDynSegmentAction::findNearestMinimumModeOff ) ) ;

  keyPressEventSubscribe( Qt::Key_U, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::undo ) ) ;
  
  keyPressEventSubscribe( Qt::Key_R, Qt::NoButton, 
			  KeyActionLinkOf<PaintAction>
			  ( actionPool->action( "PaintAction" ), 
			    &PaintAction::redo ) ) ;
  
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftButton, 
			  KeyActionLinkOf<RoiMorphoMathAction>
			  ( actionPool->action( "MorphoMathAction" ), 
			    &RoiMorphoMathAction::setDistanceToMm ) ) ;
  
  keyPressEventSubscribe( Qt::Key_D, Qt::ControlButton, 
			  KeyActionLinkOf<RoiMorphoMathAction>
			  ( actionPool->action( "MorphoMathAction" ), 
			    &RoiMorphoMathAction::setDistanceToVoxel ) ) ;
  
  
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoButton, 
      MouseActionLinkOf<RoiDynSegmentAction>( actionPool->action( "DynSegmentAction" ), 
					      &RoiDynSegmentAction::setPointToSegmentByDiscriminatingAnalyse ) ) ;
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ControlButton, 
      MouseActionLinkOf<RoiDynSegmentAction>( actionPool->action( "DynSegmentAction" ), 
					      &RoiDynSegmentAction::replaceRegion ) ) ;
  
  //   mousePressButtonEventSubscribe
  //     ( Qt::LeftButton, Qt::ControlButton, 
  //       MouseActionLinkOf<RoiDynSegmentAction>( actionPool->action( "DynSegmentAction" ), 
  // 					    &RoiDynSegmentAction::removeFromRegion ) );
  
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoButton, 
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ), 
				     &MenuAction::execMenu ) );
  
  // Renaud : Pas top, mais en attendant mieux...
  myAction = actionPool->action( "DynSegmentAction" ) ;
}

void 
RoiDynSegmentControl::doAlsoOnSelect( ActionPool * /*pool*/ )
{
  if(myAction)
    {
      ControlSwitch	*cs = myAction->view()->controlSwitch();
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
      if( cs->toolBox() )
        cs->toolBox()->showPage( myAction->name() );
    }
}

