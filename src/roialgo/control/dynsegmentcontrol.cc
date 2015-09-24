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

using namespace anatomist;
using namespace std;

Control *
RoiDynSegmentControl::creator()
{
  RoiDynSegmentControl * pc = new RoiDynSegmentControl();

  return ( pc );
}


RoiDynSegmentControl::RoiDynSegmentControl()
  : Control( 106, "DynSegmentControl" )
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
  return QT_TRANSLATE_NOOP( "ControlledWindow", "DynSegmentControl" );
}

void
RoiDynSegmentControl::eventAutoSubscription( ActionPool * actionPool )
{
  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::close ),
                          "close_window" );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::toggleFullScreen ),
                          "full_screen_toggle" );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::toggleShowTools ),
                          "show_tools_toggle" );

  // sort triangles by depth
  keyPressEventSubscribe( Qt::Key_D, Qt::NoModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::sort ),
                          "sort_polygons_by_depth" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ShiftModifier,
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::beginZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::moveZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::endZoom ), true );
  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ),
                         &Zoom3DAction::zoomWheel ) );

  //        translation

  mouseLongEventSubscribe
    ( Qt::MidButton, Qt::ControlModifier,
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
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::NoModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::previousSlice ),
                          "previous_slice" );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::nextSlice ),
                          "next_slice" );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::previousTime ),
                          "previous_time" );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::nextTime ),
                          "next_time" );

  // Level Set

  keyPressEventSubscribe( Qt::Key_2, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::dimensionModeTo2D ),
                          "2d_mode" );

  keyPressEventSubscribe( Qt::Key_3, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::dimensionModeTo3D ),
                          "3d_mode" );

  keyPressEventSubscribe( Qt::Key_O, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::increaseOrder ),
                          "increase_order" );

  keyPressEventSubscribe( Qt::Key_O, Qt::ControlModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::decreaseOrder ),
                          "decrease_order" );

  keyPressEventSubscribe( Qt::Key_F, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::increaseFaithInterval ),
                          "increase_confidence_interval" );

  keyPressEventSubscribe( Qt::Key_F, Qt::ControlModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::decreaseFaithInterval ),
                          "decrease_confidence_interval" );

  keyPressEventSubscribe( Qt::Key_R, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::refineModeOn ),
                          "set_refine_mode" );

  keyPressEventSubscribe( Qt::Key_R, Qt::ControlModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::refineModeOff ),
                          "unset_refine_mode" );

  keyPressEventSubscribe( Qt::Key_M, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::findNearestMinimumModeOn ),
                          "set_find_nearest_min_mode" );

  keyPressEventSubscribe( Qt::Key_M, Qt::ControlModifier,
                          KeyActionLinkOf<RoiDynSegmentAction>
                          ( actionPool->action( "DynSegmentAction" ),
                            &RoiDynSegmentAction::findNearestMinimumModeOff ),
                          "unset_find_nearest_min_mode" );

  keyPressEventSubscribe( Qt::Key_U, Qt::NoModifier,
                          KeyActionLinkOf<PaintAction>
                          ( actionPool->action( "PaintAction" ),
                            &PaintAction::undo ),
                          "undo" );

  keyPressEventSubscribe( Qt::Key_Z, Qt::ControlModifier,
                          KeyActionLinkOf<PaintAction>
                          ( actionPool->action( "PaintAction" ),
                            &PaintAction::undo ),
                          "undo" );

  keyPressEventSubscribe( Qt::Key_R, Qt::NoModifier,
                          KeyActionLinkOf<PaintAction>
                          ( actionPool->action( "PaintAction" ),
                            &PaintAction::redo ),
                          "redo" );

  keyPressEventSubscribe( Qt::Key_Z, (Qt::KeyboardModifiers )
                          ( Qt::ShiftModifier | Qt::ControlModifier ),
                          KeyActionLinkOf<PaintAction>
                          ( actionPool->action( "PaintAction" ),
                            &PaintAction::redo ),
                          "redo" );

  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<RoiMorphoMathAction>
                          ( actionPool->action( "MorphoMathAction" ),
                            &RoiMorphoMathAction::setDistanceToMm ),
                          "set_distance_millimeter" );

  keyPressEventSubscribe( Qt::Key_D, Qt::ControlModifier,
                          KeyActionLinkOf<RoiMorphoMathAction>
                          ( actionPool->action( "MorphoMathAction" ),
                            &RoiMorphoMathAction::setDistanceToVoxel ),
                          "set_distance_voxel" );


  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<RoiDynSegmentAction>(
        actionPool->action( "DynSegmentAction" ),
        &RoiDynSegmentAction::setPointToSegmentByDiscriminatingAnalyse ),
      "add_dyn_segment_to_roi" );
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ControlModifier,
      MouseActionLinkOf<RoiDynSegmentAction>(
        actionPool->action( "DynSegmentAction" ),
        &RoiDynSegmentAction::replaceRegion ),
      "replace_roi_with_dyn_segment" );

  //   mousePressButtonEventSubscribe
  //     ( Qt::LeftButton, Qt::ControlModifier,
  //       MouseActionLinkOf<RoiDynSegmentAction>( actionPool->action( "DynSegmentAction" ),
  //                                             &RoiDynSegmentAction::removeFromRegion ) );

  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>(
        actionPool->action( "MenuAction" ),
        &MenuAction::execMenu ),
      "menu" );

  // Renaud : Pas top, mais en attendant mieux...
  myAction = actionPool->action( "DynSegmentAction" );
}

void
RoiDynSegmentControl::doAlsoOnSelect( ActionPool * /*pool*/ )
{
  if(myAction)
    {
      ControlSwitch        *cs = myAction->view()->controlSwitch();
      if( !cs->isToolBoxVisible() )
        {
          // this is not a very elegant way of doing it...
          set<AWindow *>                w = theAnatomist->getWindows();
          set<AWindow *>::iterator        iw, ew = w.end();
          AWindow3D                        *w3;
          bool                                visible = false;
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

