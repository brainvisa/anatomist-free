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
#include <anatomist/control/labelnamingcontrol.h>
#include <anatomist/action/labelnaming.h>
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
RoiLabelNamingControl::creator( )
{
  RoiLabelNamingControl * pc = new RoiLabelNamingControl();

  return ( pc );
}


RoiLabelNamingControl::RoiLabelNamingControl( )
  : Control( 81023, "LabelNamingControl" )
{

}

RoiLabelNamingControl::RoiLabelNamingControl( const RoiLabelNamingControl& c)
  : Control(c)
{

}

RoiLabelNamingControl::~RoiLabelNamingControl()
{

}

std::string
RoiLabelNamingControl::name() const
{
  return QT_TRANSLATE_NOOP( "ControlledWindow", "LabelNamingControl" );
}

void
RoiLabelNamingControl::eventAutoSubscription( ActionPool * actionPool )
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

  keyPressEventSubscribe( Qt::Key_D, Qt::ControlModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleAutoSort ),
                          "auto_sort_polygons_by_depth" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ShiftModifier,
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
    ( Qt::MiddleButton, Qt::ControlModifier,
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

  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ControlModifier,
      MouseActionLinkOf<RoiLabelNamingAction>(
        actionPool->action( "LabelNamingAction" ),
        &RoiLabelNamingAction::removeConnecCompFromRegion ),
      "remove_component_from_roi" );

  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<RoiLabelNamingAction>(
        actionPool->action( "LabelNamingAction" ),
        &RoiLabelNamingAction::addConnecCompToRegion ),
      "add_component_to_roi" );
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
      MouseActionLinkOf<RoiLabelNamingAction>(
        actionPool->action( "LabelNamingAction" ),
        &RoiLabelNamingAction::removeWholeLabelFromRegion ),
      "remove_whole_label_from_roi" );

  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ShiftModifier,
      MouseActionLinkOf<RoiLabelNamingAction>(
        actionPool->action( "LabelNamingAction" ),
        &RoiLabelNamingAction::addWholeLabelToRegion ),
      "add_whole_label_to_roi" );

  keyPressEventSubscribe( Qt::Key_2, Qt::NoModifier,
                          KeyActionLinkOf<RoiLabelNamingAction>
                          ( actionPool->action( "LabelNamingAction" ),
                            &RoiLabelNamingAction::setModeTo2D ),
                          "2d_mode" );

  keyPressEventSubscribe( Qt::Key_3, Qt::NoModifier,
                          KeyActionLinkOf<RoiLabelNamingAction>
                          ( actionPool->action( "LabelNamingAction" ),
                            &RoiLabelNamingAction::setModeTo3D ),
                          "3d_mode" );

  //   mousePressButtonEventSubscribe
  //     ( Qt::LeftButton, Qt::ControlModifier,
  //       MouseActionLinkOf<RoiLabelNamingAction>( actionPool->action( "LabelNamingAction" ),
  //                                             &RoiLabelNamingAction::removeFromRegion ) );

  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>(
        actionPool->action( "MenuAction" ),
        &MenuAction::execMenu ),
      "menu" );

  // Renaud : Pas top, mais en attendant mieux...
  myAction = actionPool->action( "LabelNamingAction" );
}


void
RoiLabelNamingControl::doAlsoOnSelect( ActionPool * /*pool*/ )
{
  if(/*myAction*/0)
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

