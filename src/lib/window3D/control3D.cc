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
#include <anatomist/reference/Transformation.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/mesh/surfaceOperation.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <QDrag>
#include <QStatusBar>
#if QT_VERSION >= 0x040600
#include <QPinchGesture>
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
  return new Control3D;
}


Control3D::Control3D( int priority, const string & name )
  : Control( priority, name )
{
}


Control3D::Control3D( const Control3D & c ) : Control( c )
{
}


Control3D::~Control3D()
{
}


string Control3D::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "Control3D_description" );
}


void Control3D::eventAutoSubscription( ActionPool * actionPool )
{
  //cout << "Control3D::eventAutoSubscription\n";
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::endLink ), true );

  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ),
      "menu" );

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

  //        rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ),
                          "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ),
                          "show_rotation_center" );

  //        sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ),
                          "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ),
                          "sync_views_orientation" );
  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::beginTrackball ),
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::moveTrackball ),
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::beginTrackball ),
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::moveTrackball ),
      MouseActionLinkOf<ContinuousTrackball>
      ( actionPool->action( "ContinuousTrackball" ),
        &ContinuousTrackball::endTrackball ), true );

  keyPressEventSubscribe( Qt::Key_Space, Qt::ControlModifier,
                          KeyActionLinkOf<ContinuousTrackball>
                          ( actionPool->action( "ContinuousTrackball" ),
                            &ContinuousTrackball::startOrStop ),
                          "continuous_trackball_start_stop" );

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::toggleSelectAll ),
                          "select_all_toggle" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromWindow ),
                          "remove_from_window" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromGroup ),
                          "remove_from_group" );

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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::beginZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::moveZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
                                       &Zoom3DAction::endZoom ), true );

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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::beginTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::moveTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::endTranslate ), true );

#if QT_VERSION >= 0x040600

  // pinch
  pinchEventSubscribe(
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStart ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchMove ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStop ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStop ) );

#endif

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
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::toggleLinkedOnSlider ),
                          "linked_cursor_on_slider_change_toggle" );

  // Movie action
  keyPressEventSubscribe( Qt::Key_Space, Qt::NoModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::startOrStop ),
                          "movie_start_stop" );

  keyPressEventSubscribe( Qt::Key_S, Qt::ControlModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::sliceOn ),
                          "movie_slice_on" );

  keyPressEventSubscribe( Qt::Key_T, Qt::ControlModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::timeOn ),
                          "movie_time_on" );

  keyPressEventSubscribe( Qt::Key_M, Qt::ControlModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::nextMode ),
                          "movie_next_mode" );

  keyPressEventSubscribe( Qt::Key_Plus, Qt::NoModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::increaseSpeed ),
                          "movie_inc_speed" );
  keyPressEventSubscribe( Qt::Key_Plus, Qt::ShiftModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::increaseSpeed ),
                          "movie_inc_speed2" );

  keyPressEventSubscribe( Qt::Key_Minus, Qt::NoModifier,
                          KeyActionLinkOf<MovieAction>
                          ( actionPool->action( "MovieAction" ),
                            &MovieAction::decreaseSpeed ),
                          "movie_dec_speed" );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  // Is it MY job to maintain this map ???
  myActions[ "MovieAction" ] = actionPool->action( "MovieAction" );
  myActions[ "ContinuousTrackball" ]
    = actionPool->action( "ContinuousTrackball" );
}


void Control3D::doAlsoOnDeselect( ActionPool * )
{
  map<string, ActionPtr>::const_iterator        ia, ea=myActions.end();
  MovieAction                                        *a;
  ContinuousTrackball                                *a2;

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
  return new Select3DControl;
}

Select3DControl::Select3DControl( const string & name )
  : Control( 200, QT_TRANSLATE_NOOP( "ControlledWindow", name ) )
{
  // just for Qt translator parsing
  std::string txt __attribute__((unused)) = 
    QT_TRANSLATE_NOOP( "ControlledWindow", "Selection 3D" );
}


Select3DControl::Select3DControl( const Select3DControl & c ) : Control( c )
{
}


Select3DControl::~Select3DControl()
{
}


string Select3DControl::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "Select3DControl_description" );
}


void Select3DControl::eventAutoSubscription( ActionPool * actionPool )
{
  //cout << "Select3DControl::eventAutoSubscription\n";
  mousePressButtonEventSubscribe(
    Qt::LeftButton, Qt::NoModifier,
    MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ),
              &SelectAction::execSelect ),
    "selection" );
  mousePressButtonEventSubscribe(
    Qt::LeftButton, Qt::ShiftModifier,
    MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ),
              &SelectAction::execSelectAdding ),
    "selection_add" );
  mousePressButtonEventSubscribe(
    Qt::LeftButton, Qt::ControlModifier,
    MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ),
              &SelectAction::execSelectToggling ),
    "selection_toggle" );
  mousePressButtonEventSubscribe(
    Qt::RightButton, Qt::NoModifier,
    MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
            &MenuAction::execMenu ),
    "menu" );

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

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlModifier,
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::toggleSelectAll ),
        "select_all_toggle" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoModifier,
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::removeFromWindow ),
        "remove_from_window" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlModifier,
        KeyActionLinkOf<SelectAction>
        ( actionPool->action( "SelectAction" ),
          &SelectAction::removeFromGroup ),
        "remove_from_group" );

  //  rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
        KeyActionLinkOf<Trackball>
        ( actionPool->action( "Trackball" ),
          &Trackball::setCenter ),
        "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
        KeyActionLinkOf<Trackball>
        ( actionPool->action( "Trackball" ),
          &Trackball::showRotationCenter ),
        "show_rotation_center" );

  //  sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
        KeyActionLinkOf<Sync3DAction>
        ( actionPool->action( "Sync3DAction" ),
          &Sync3DAction::execSync ),
        "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
        KeyActionLinkOf<Sync3DAction>
        ( actionPool->action( "Sync3DAction" ),
          &Sync3DAction::execSyncOrientation ),
        "sync_views_orientation" );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
            &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
            &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
            &Trackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
               &Zoom3DAction::beginZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
               &Zoom3DAction::moveZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
               &Zoom3DAction::endZoom ), true );

  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ),
                         &Zoom3DAction::zoomWheel ) );

#if QT_VERSION >= 0x040600

  // pinch
  pinchEventSubscribe(
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStart ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchMove ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStop ),
    PinchActionLinkOf<PinchZoomAction>( actionPool->action(
      "PinchZoomAction" ), &PinchZoomAction::pinchStop ) );

#endif

  //  translation

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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
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
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::toggleLinkedOnSlider ),
                          "linked_cursor_on_slider_change_toggle" );

  // label picker / edition
  keyPressEventSubscribe( Qt::Key_Space, Qt::NoModifier,
                          KeyActionLinkOf<LabelEditAction>
                          ( actionPool->action( "LabelEditAction" ),
                            &LabelEditAction::pick ),
                          "pick_label" );
  keyPressEventSubscribe( Qt::Key_Return, Qt::ControlModifier,
                          KeyActionLinkOf<LabelEditAction>
                          ( actionPool->action( "LabelEditAction" ),
                            &LabelEditAction::edit ),
                          "paste_label" );

  // graph annotation
  keyPressEventSubscribe( Qt::Key_A, Qt::NoModifier,
                          KeyActionLinkOf<AnnotationAction>
                          ( actionPool->action( "AnnotationAction" ),
                            &AnnotationAction::switchAnnotations ),
                          "graph_annotation" );

  // show things
  keyPressEventSubscribe( Qt::Key_V, Qt::NoModifier,
                          KeyActionLinkOf<ObjectStatAction>
                          ( actionPool->action( "ObjectStatAction" ),
                            &ObjectStatAction::displayStat ),
                          "display_stat" );
}


void Select3DControl::doAlsoOnSelect( ActionPool* pool )
{
  Action *a = pool->action( "LabelEditAction" );
  if( a )
  {
    View  *v = a->view();
    if( !v )
      return;
    QAWindow  *aw = dynamic_cast<QAWindow *>( v->aWindow() );
    if( !aw )
      return;
    QToolBar *tb = aw->findChild<QToolBar *>( "select3D_toolbar" );
    if( tb )
      return; // toolbar found: it already exists.
    tb = aw->addToolBar( Qt::TopToolBarArea,
                         ControlledWindow::tr( "Selection tools" ),
                         "select3D_toolbar" );
    tb->setIconSize( QSize( 20, 20 ) );
    QLabel *l = new QLabel( ControlledWindow::tr( "Selection label" ), tb );
    l->setObjectName( "selectionLabel" );
    tb->addWidget( l );
    LabelEditAction *la = static_cast<LabelEditAction *>( a );
    la->setLabel( la->label() );
    tb->setVisible( aw->toolBarsVisible() );
  }
  // annotation
  a = pool->action( "AnnotationAction" );
  if( a )
    static_cast<AnnotationAction *>( a )->cleanAnnotations();
}


void Select3DControl::doAlsoOnDeselect( ActionPool* pool )
{
  Action *a = pool->action( "LabelEditAction" );
  if( a )
  {
    View  *v = a->view();
    if( !v )
      return;
    QAWindow  *aw = dynamic_cast<QAWindow *>( v->aWindow() );
    if( !aw )
      return;
    // the 1st time, tb is null, so is not deleted, althouth it is still
    // here. So we find/delete the label by hand (this one is found.)
    if( aw->findChild<QLabel *>( "selectionLabel" ) )
      delete aw->findChild<QLabel *>( "selectionLabel" );
    QToolBar *tb = aw->removeToolBar( "select3D_toolbar" );
    delete tb;
  }
  // annotation
  a = pool->action( "AnnotationAction" );
  if( a )
    static_cast<AnnotationAction *>( a )->cleanAnnotations();
}

// ----

Action *
LinkAction::creator()
{
  return new LinkAction;
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


QWidget* LinkAction::actionView( QWidget* )
{
  return( 0 );
}


bool LinkAction::viewableAction() const
{
  return( false );
}


void LinkAction::execLink( int x, int y, int, int )
{
  View                *v = view();
  GLWidgetManager        *w = dynamic_cast<GLWidgetManager *>( v );

  if( !w )
    {
      cerr << "LinkAction operating on wrong view type -- error\n";
      return;
    }

  AWindow        *win = v->aWindow();

  Point3df        pos;
  if( win->positionFromCursor( x, y, pos ) )
  {
    vector<float>        vp = win->getFullPosition();
    vp[0] = pos[0];
    vp[1] = pos[1];
    vp[2] = pos[2];

    cout << "Position : " << vp[0];
    unsigned i, n = vp.size();
    for( i=1; i<n; ++i )
      cout << ", " << vp[i];
    cout << endl;

    LinkedCursorCommand        *c
      = new LinkedCursorCommand( v->aWindow(), vp );
    theProcessor->execute( c );
    AWindow3D *w3 = dynamic_cast<AWindow3D *>( win );
    if( w3 )
      w3->displayInfoAtClickPosition( x, y );
  }
  else cout << "no position could be read at " << x << ", " << y << endl;
}


void LinkAction::endLink( int, int, int, int )
{
}


// --------

Action *
MenuAction::creator()
{
  return new MenuAction;
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


QWidget* MenuAction::actionView( QWidget* )
{
  return( 0 );
}


bool MenuAction::viewableAction() const
{
  return( false );
}


void MenuAction::execMenu( int x, int y, int, int )
{
  view()->aWindow()->button3clicked( x, y );
}


// -------------------

Action *
SelectAction::creator()
{
  return new SelectAction;
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


QWidget* SelectAction::actionView( QWidget* )
{
  return( 0 );
}


bool SelectAction::viewableAction() const
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
  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
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
      else if( modifier == SelectFactory::Normal )
      {
        bool unsel_bg = false;
        try
        {
          unsel_bg = bool(
            cfg->getProperty( "unselect_on_background" )->getScalar() );
        }
        catch( ... )
        {
        }
        if( unsel_bg )
        {
          SelectFactory *sf = SelectFactory::factory();
          sf->unselectAll( w3->Group() );
          sf->refresh();
        }
      }
      return;
    }
  }

  // fallback to older selection
  Point3df      pos;
  if( w->positionFromCursor( x, y, pos ) )
  {
    /*vector<float>        vp;
    vp.push_back( pos[0] );
    vp.push_back( pos[1] );
    vp.push_back( pos[2] );
    SelectionCommand        *c
      = new SelectionCommand( w->aWindow(), vp );
      theProcessor->execute( c );*/

    vector<float> posw = view()->aWindow()->getFullPosition();
    posw[0] = pos[0];
    posw[1] = pos[1];
    posw[2] = pos[2];

    view()->aWindow()->selectObject( posw,
                                     (SelectFactory::SelectMode) modifier );
  }
  else if( modifier == SelectFactory::Normal )
  {
    GlobalConfiguration   *cfg = theAnatomist->config();
    bool unsel_bg = false;
    try
    {
      unsel_bg
        = bool( cfg->getProperty( "unselect_on_background" )->getScalar() );
    }
    catch( ... )
    {
    }
    if( unsel_bg )
    {
      SelectFactory *sf = SelectFactory::factory();
      sf->unselectAll( w3->Group() );
      sf->refresh();
    }
  }
}


void SelectAction::toggleSelectAll()
{
  AWindow                *w = view()->aWindow();
  set<AObject *>        obj = w->Objects();
  SelectFactory                *sf = SelectFactory::factory();
  bool                        allsel = true;
  set<AObject *>::iterator        io, eo = obj.end();
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
  SelectFactory::factory()->removeFromThisWindow( view()->aWindow() );
}


void SelectAction::removeFromGroup()
{
  SelectFactory::factory()->remove( view()->aWindow() );
}


// --------

Action *
Zoom3DAction::creator ()
{
  return new Zoom3DAction;
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


QWidget* Zoom3DAction::actionView( QWidget* )
{
  return( 0 );
}


bool Zoom3DAction::viewableAction() const
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

  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
  if (w && w3 && w3->surfpaintIsVisible())
    w->copyBackBuffer2Texture();
}


void Zoom3DAction::moveZoom( int, int y, int, int )
{
  //cout << "Zoom3DAction::moveZoom\n";
  if( _beginpos < 0 )
    return;

  int        m = _beginpos - y;
  float        zfac = exp( 0.01 * m );
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
      const Quaternion        & q = w->quaternion();
      Point3df                p = q.transform( Point3df( 0, 0, -m ) );
      float zfac = w->invertedZ() ? -1 : 1;
      p[2] = zfac * p[2];        // invert Z axis
      //cout << "avance : " << p << endl;
      w->setRotationCenter( w->rotationCenter() + p );
      _beginpos = y;        // no memory of begin position in this mode
    }
  else
    w->setZoom( zfac * _orgzoom );
  ((AWindow3D *) w->aWindow())->refreshLightViewNow();
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
  float        zfac = exp( 0.01 * distance );
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    return;

  if( w->perspectiveEnabled() )
    {
      const Quaternion        & q = w->quaternion();
      Point3df                p = q.transform( Point3df( 0, 0, -distance ) );
      p[2] = -p[2];        // invert Z axis
      //cout << "avance : " << p << endl;
      w->setRotationCenter( w->rotationCenter() + p );
    }
  else
    w->setZoom( zfac * w->zoom() );
  ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


// --------

Action *
Translate3DAction::creator()
{
  return new Translate3DAction;
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


QWidget* Translate3DAction::actionView( QWidget* )
{
  return( 0 );
}


bool Translate3DAction::viewableAction() const
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
  /*Point2df        t = w->translation();
  _orgx = t[0];
  _orgy = t[1];*/
  _started = true;
}


void Translate3DAction::endTranslate( int, int, int, int )
{
  endTranslateKey();

  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
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
  Point3df        t;

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

  ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


// ----------

Action *
Sync3DAction::creator()
{
  return new Sync3DAction;
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
  AWindow3D        *win = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !win )
    {
      cerr << "Sync3DAction operating on wrong window type -- error\n";
      return;
    }
  win->syncViews();
}


void Sync3DAction::execSyncOrientation()
{
  AWindow3D        *win = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !win )
    {
      cerr << "Sync3DAction operating on wrong window type -- error\n";
      return;
    }
  win->syncViews( true );
}


QWidget* Sync3DAction::actionView( QWidget* )
{
  return( 0 );
}


bool Sync3DAction::viewableAction() const
{
  return( false );
}


// --------

Control *
FlightControl::creator( )
{
  return new FlightControl;
}

FlightControl::FlightControl()
  : Control( 400000,
             QT_TRANSLATE_NOOP( "ControlledWindow", "Flight control" ) )
{
  setUserLevel( 2 );
}


FlightControl::FlightControl( const FlightControl & c ) : Control( c )
{
}


FlightControl::~FlightControl()
{
}


string FlightControl::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "FlightControl_description" );
}


void FlightControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
                                     &LinkAction::execLink ),
    "linked_cursor" );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ),
    "menu" );

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

  //        rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ),
                          "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ),
                          "set_rotation_center" );

  //        sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ),
                          "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ),
                          "sync_views_orientation" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::beginTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::moveTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
        &Translate3DAction::endTranslate ), true );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  //        Flight simulator

  keyRepetitiveEventSubscribe( Qt::Key_Up, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::up ),
                               Qt::Key_Up, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Up, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::up ),
                               Qt::Key_Up, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Down, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::down ),
                               Qt::Key_Down, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Down, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::down ),
                               Qt::Key_Down, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Left, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::left ),
                               Qt::Key_Left, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Right, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::right ),
                               Qt::Key_Right, Qt::ControlModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Left, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::spinLeft ),
                               Qt::Key_Left, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Right, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::spinRight ),
                               Qt::Key_Right, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_Q, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::boost ),
                               Qt::Key_Q, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_A, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::brake ),
                               Qt::Key_A, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::release ), false, 1 );
  keyRepetitiveEventSubscribe( Qt::Key_F, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::runStep ),
                               Qt::Key_G, Qt::NoModifier,
                               KeyActionLinkOf<KeyFlightAction>
                               ( actionPool->action( "KeyFlightAction" ),
                                 &KeyFlightAction::stop ), false, 1 );
  keyPressEventSubscribe( Qt::Key_R, Qt::NoModifier,
                          KeyActionLinkOf<KeyFlightAction>
                          ( actionPool->action( "KeyFlightAction" ),
                            &KeyFlightAction::reverse ),
                          "flight_reverse" );

  // Is it MY job to maintain this map ???
  myActions[ "KeyFlightAction" ] = actionPool->action( "KeyFlightAction" );
}


void FlightControl::doAlsoOnDeselect( ActionPool * )
{
  map<string, ActionPtr>::const_iterator        ia, ea=myActions.end();
  KeyFlightAction                                *kf;

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

ObliqueControl::ObliqueControl( const string & name )
  : Control( 300, QT_TRANSLATE_NOOP( "ControlledWindow", name ) )
{
  // just for Qt translation parsing
  std::string txt __attribute__((unused)) = 
    QT_TRANSLATE_NOOP( "ControlledWindow", "ObliqueControl" );
}


ObliqueControl::ObliqueControl( const ObliqueControl & c ) : Control( c )
{
}


ObliqueControl::~ObliqueControl()
{
}


string ObliqueControl::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "ObliqueControl_description" );
}


void ObliqueControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
                                     &LinkAction::execLink ),
    "linked_cursor" );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ),
    "menu" );

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

  //        rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ),
                          "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ),
                          "show_rotation_center" );

  //        sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ),
                          "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ),
                          "sync_views_orientation" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );

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
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::toggleLinkedOnSlider ),
                          "linked_cursor_on_slider_change_toggle" );

  // wheel zoom
  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ),
                         &Zoom3DAction::zoomWheel ) );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  // oblique trackball

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ShiftModifier,
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::beginTrackball ),
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::moveTrackball ),
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::beginTrackball ),
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::moveTrackball ),
      MouseActionLinkOf<TrackOblique>( actionPool->action( "TrackOblique" ),
                                       &TrackOblique::endTrackball ), true );

  // plane inversion
  keyPressEventSubscribe( Qt::Key_I, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::invertSlice ),
                          "invert_slice" );

  // oblique slice trackball

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ControlModifier,
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ),
        &TrackObliqueSlice::beginTrackball ),
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ),
        &TrackObliqueSlice::moveTrackball ),
      MouseActionLinkOf<TrackObliqueSlice>
      ( actionPool->action( "TrackObliqueSlice" ),
        &TrackObliqueSlice::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
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
  : Control( 10000, QT_TRANSLATE_NOOP( "ControlledWindow", "TransformControl" ) )
{
}


TransformControl::TransformControl( const TransformControl & c ) : Control( c )
{
}


TransformControl::~TransformControl()
{
}


string TransformControl::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "TransformControl_description" );
}


void TransformControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
                                     &LinkAction::execLink ),
    "linked_cursor" );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ),
    "menu" );

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

  //        rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ),
                          "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ),
                          "show_rotation_center" );

  //        sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ),
                          "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ),
                          "sync_views_orientation" );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );

  // wheel zoom
  wheelEventSubscribe( WheelActionLinkOf<Zoom3DAction>
                       ( actionPool->action( "Zoom3DAction" ),
                         &Zoom3DAction::zoomWheel ) );
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
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::toggleLinkedOnSlider ),
                          "linked_cursor_on_slider_change_toggle" );

  // Transformer trackball

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ShiftModifier,
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::beginTrackball ),
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::moveTrackball ),
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::beginTrackball ),
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::moveTrackball ),
      MouseActionLinkOf<Transformer>( actionPool->action( "Transformer" ),
                                      &Transformer::endTrackball ), true );

  keyPressEventSubscribe( Qt::Key_I, Qt::NoModifier,
                          KeyActionLinkOf<Transformer>(
                            actionPool->action( "Transformer" ),
                            &Transformer::toggleDisplayInfo ),
                          "display_transform_info_toggle" );
  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ControlModifier,
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::begin ),
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::move ),
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::end ),
      true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::begin ),
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::move ),
      MouseActionLinkOf<TranslaterAction>
      ( actionPool->action( "TranslaterAction" ), &TranslaterAction::end ),
      true );
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ControlModifier,
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
    ( Qt::LeftButton, Qt::ShiftModifier,
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::begin ),
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::move ),
      MouseActionLinkOf<ResizerAction>
      ( actionPool->action( "ResizerAction" ), &ResizerAction::end ),
      true );
}


void TransformControl::doAlsoOnSelect( ActionPool * actionPool )
{
  Action *ac = actionPool->action( "Transformer" );
  if( ac )
  {
    Transformer *tac = dynamic_cast<Transformer *>( ac );
    if( tac )
      tac->showGraphicsView();
  }
}


void TransformControl::doAlsoOnDeselect( ActionPool * actionPool )
{
  Action *ac = actionPool->action( "Transformer" );
  if( ac )
  {
    Transformer *tac = dynamic_cast<Transformer *>( ac );
    if( tac )
      tac->clearGraphicsView();
  }
}


// ------------

Control * CutControl::creator()
{
  return new CutControl;
}

CutControl::CutControl()
  : Control( 3000, QT_TRANSLATE_NOOP( "ControlledWindow", "CutControl" ) )
{
}


CutControl::CutControl( const CutControl & c ) : Control( c )
{
}


CutControl::~CutControl()
{
}


string CutControl::description() const
{
  return QT_TRANSLATE_NOOP(
    "ControlledWindow",
    "CutControl_description" );
}


void CutControl::eventAutoSubscription( ActionPool * actionPool )
{
  // standard linked cursor / menu
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
                                     &LinkAction::execLink ),
    "linked_cursor" );
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ),
    "menu" );

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

  // selection shortcuts

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::toggleSelectAll ),
                          "select_all_toggle" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromWindow ),
                          "remove_from_window" );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromGroup ),
                          "remove_from_group" );

  //        rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ),
                          "set_rotation_center" );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ),
                          "show_rotation_center" );

  //        sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ),
                          "sync_views" );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ),
                          "sync_views_orientation" );

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
  keyPressEventSubscribe( Qt::Key_D, Qt::ShiftModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleSortDirection ),
                          "sort_polygons_direction" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
                                    &Trackball::endTrackball ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::AltModifier,
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
    ( Qt::MiddleButton, Qt::ShiftModifier,
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
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::ShiftModifier,
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

  keyPressEventSubscribe( Qt::Key_A, Qt::ShiftModifier,
                          KeyActionLinkOf<TrackCutAction>
                          ( actionPool->action( "TrackCutAction" ),
                            &TrackCutAction::axialSlice ),
                          "axial_slice_cut" );
  keyPressEventSubscribe( Qt::Key_C, Qt::ShiftModifier,
                          KeyActionLinkOf<TrackCutAction>
                          ( actionPool->action( "TrackCutAction" ),
                            &TrackCutAction::coronalSlice ),
                          "coronal_slice_cut" );
  keyPressEventSubscribe( Qt::Key_S, Qt::ShiftModifier,
                          KeyActionLinkOf<TrackCutAction>
                          ( actionPool->action( "TrackCutAction" ),
                            &TrackCutAction::sagittalSlice ),
                          "sagittal_slice_cut" );
  // plane inversion
  keyPressEventSubscribe( Qt::Key_I, Qt::ControlModifier,
                          KeyActionLinkOf<TrackCutAction>
                          ( actionPool->action( "TrackCutAction" ),
                            &TrackCutAction::invertSlice ),
                          "invert_slice" );

  // oblique slice trackball

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ControlModifier,
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ),
        &CutSliceAction::beginTrack ),
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ),
        &CutSliceAction::moveTrack ),
      MouseActionLinkOf<CutSliceAction>
      ( actionPool->action( "CutSliceAction" ),
        &CutSliceAction::endTrack ), true );
  // for Mac and its 2 button mouse
  mouseLongEventSubscribe
    ( Qt::RightButton, Qt::AltModifier,
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
  MovieAction * pa = new MovieAction( );

  return( pa );
}

MovieAction::MovieAction()
  : QObject(), Action(), mySliceAndNotTime(false), myIsRunning(false),
    myRunMode( Forward ), myForward( true ), myTimeInterval( 100 )
{
  myTimer = new QTimer( this );
  myTimer->setInterval( 100 );
  myTimer->stop();        // don't start right now...

  QObject::connect( myTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
}

MovieAction::~MovieAction()
{
  delete myTimer;
}

void
MovieAction::sliceOn()
{
  //cerr << "Slice On " << endl;
  mySliceAndNotTime = true;
}

void
MovieAction::timeOn()
{
  //cerr << "Time On " << endl;
  mySliceAndNotTime = false;
}


void
MovieAction::nextMode()
{
  int                rm = (int) myRunMode + 1;
  if( rm > LoopBothWays )
    myRunMode = Forward;
  else
    myRunMode = (RunMode) rm;
  static const string        modes[] = { "Forward", "Backward",
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
  //cerr << "Start or Stop" << endl;
  if( ! myIsRunning )
  {
    //cerr << "Start" << endl;
    myIsRunning = true;
    myTimer->setSingleShot( true );
    myTimer->start( myTimeInterval );
  }
  else
  {
    myIsRunning = false;
    myTimer->stop( );
    //cerr << "Stop" << endl;
  }
}


void
MovieAction::timeout()
{
  int sliderPosition, maxPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;

  if( mySliceAndNotTime || win->getTimeSliderMaxPosition() == 0 )
    {
      sliderPosition = win->getSliceSliderPosition();
      maxPosition = win->getSliceSliderMaxPosition();
    }
  else
    {
      sliderPosition = win->getTimeSliderPosition();
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

  bool        stop = false;

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
    myIsRunning = false;
    myTimer->stop( );
    return;
  }

  if( mySliceAndNotTime || win->getTimeSliderMaxPosition() == 0 )
  {
    win->setSliceSliderPosition( sliderPosition );
    myTimer->setSingleShot( true );
    myTimer->start( myTimeInterval );
  }
  else
  {
    win->setTimeSliderPosition( sliderPosition );
    myTimer->setSingleShot( true );
    myTimer->start( myTimeInterval );
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
  int sliderPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;

  sliderPosition = win->getSliceSliderPosition();

  if( sliderPosition > 0 )
    win->setSliceSliderPosition( sliderPosition - 1 );
}


void SliceAction::nextSlice()
{
  int sliderPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;

  sliderPosition = win->getSliceSliderPosition();

  if( sliderPosition < win->getSliceSliderMaxPosition() )
    win->setSliceSliderPosition( sliderPosition + 1 );
}


void SliceAction::previousTime()
{
  int sliderPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;

  sliderPosition = win->getTimeSliderPosition();

  if( sliderPosition > 0 )
    win->setTimeSliderPosition( sliderPosition - 1 );
}


void SliceAction::nextTime()
{
  int sliderPosition;
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;

  sliderPosition = win->getTimeSliderPosition();

  if( sliderPosition < win->getTimeSliderMaxPosition() )
    win->setTimeSliderPosition( sliderPosition + 1 );
}


void SliceAction::toggleLinkedOnSlider()
{
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;
  bool        onoff = !win->linkedCursorOnSliderChange();
  cout << "toggleLinkedOnSlider: " << onoff << endl;
  win->setLinkedCursorOnSliderChange( onoff );
}


void SliceAction::invertSlice()
{
  AWindow3D * win = dynamic_cast <AWindow3D *>( view()->aWindow() );
  if ( ! win )
    return;
  Quaternion q = win->sliceQuaternion();
  Quaternion r = Quaternion();
  r.fromAxis( Point3df( 1, 0, 0 ), M_PI );
  win->setSliceQuaternion( q * r );
  win->refreshNow();
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
    QAWindow        *qw = dynamic_cast<QAWindow *>( ac->view()->aWindow() );
    if( qw && !so.empty() )
      {
        QAObjectDrag *d = new QAObjectDrag( so );
        QDrag *drag = new QDrag( qw );
        drag->setMimeData( d );

        map<int, QPixmap>::const_iterator        ip
          = QObjectTree::TypeIcons.find( (*so.begin())->type() );
        if( ip != QObjectTree::TypeIcons.end() )
          drag->setPixmap( (*ip).second );
        drag->exec( Qt::CopyAction );
      }
  }

}


void DragObjectAction::dragAll( int, int, int, int )
{
  set<AObject *>        so = view()->aWindow()->Objects();
  dragObjectStart( this, so );
}


void DragObjectAction::dragSelected( int, int, int, int )
{
  AWindow                *w = view()->aWindow();
  set<AObject *>        so = w->Objects();
  // filter selected objects
  SelectFactory        *sf = SelectFactory::factory();
  set<AObject *>::iterator        i = so.begin(), e = so.end(), j;
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
  return new WindowActions;
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


QWidget* WindowActions::actionView( QWidget* )
{
  return 0;
}


bool WindowActions::viewableAction() const
{
  return false;
}


void WindowActions::close()
{
  AWindow        *w = view()->aWindow();
  w->close();
}


void WindowActions::toggleShowTools()
{
  AWindow        *w = view()->aWindow();
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( w );
  p->value<Dictionary>()[ "show_toolbars" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


void WindowActions::toggleFullScreen()
{
  AWindow        *w = view()->aWindow();
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( w );
  p->value<Dictionary>()[ "fullscreen" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


void WindowActions::focusView()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
    w->focusView();
}


void WindowActions::focusAxialView()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
  {
    Quaternion quat( 0., 0., 0., 1. );
    GLWidgetManager* v = static_cast<GLWidgetManager *>( view() );
    v->setQuaternion( quat );
    w->focusView();
  }
}


void WindowActions::focusCoronalView()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
  {
    Quaternion quat( 1. / sqrt( 2. ), 0., 0., 1. / sqrt( 2. ) );
    GLWidgetManager* v = static_cast<GLWidgetManager *>( view() );
    v->setQuaternion( quat );
    w->focusView();
  }
}


void WindowActions::focusSagittelView()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
  {
    Quaternion quat( -0.5, -0.5, -0.5, 0.5 );
    GLWidgetManager* v = static_cast<GLWidgetManager *>( view() );
    v->setQuaternion( quat );
    w->focusView();
  }
}


SortMeshesPolygonsAction::SortMeshesPolygonsAction()
  : Action()
{
}


SortMeshesPolygonsAction::~SortMeshesPolygonsAction()
{
}


Action * SortMeshesPolygonsAction::creator()
{
  return new SortMeshesPolygonsAction;
}



void SortMeshesPolygonsAction::sort()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
  {
    w->sortPolygons( true );
//     w->Refresh();
  }
}


void SortMeshesPolygonsAction::toggleAutoSort()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
    w->setPolygonsSortingEnabled( !w->polygonsSortingEnabled() );
}


void SortMeshesPolygonsAction::toggleSortDirection()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w )
    w->setPolygonsSortingDirection( !w->polygonsSortingDirection() );
}


ObjectStatAction::ObjectStatAction()
  : Action()
{
}


ObjectStatAction::~ObjectStatAction()
{
}


Action * ObjectStatAction::creator()
{
  return new ObjectStatAction;
}


namespace
{

  string objectStatus( const AObject * obj, AWindow3D *w )
  {
    string st;
    const AGraphObject *ago = dynamic_cast<const AGraphObject *>( obj );
    if( ago )
    {
      float vol = 0., surf = 0.;

      AGraphObject::const_iterator i, e = ago->end();
      for( i=ago->begin(); i!=e; ++i )
      {
        const Bucket *abk = dynamic_cast<const Bucket *>( *i );
        const ATriangulated *atr;
        if( abk )
        {
          const BucketMap<Void> & bk = abk->bucket();
          vol += bk.lower_bound( w->getTime() )->second.size()
            * bk.sizeX() * bk.sizeY() * bk.sizeZ();
        }
        else if( (atr = dynamic_cast<const ATriangulated *>( *i ) ) != 0 )
        {
          const AimsSurface<3, Void> *mesh
            = atr->surfaceOfTime( w->getTime() );
          surf += SurfaceManip::meshArea( *mesh );
        }
      }
      stringstream ss;
      if( vol != 0 )
      {
        ss << "volume: " << vol << " mm3";
        if( surf != 0 )
          ss << ", ";
      }
      if( surf != 0 )
        ss << "area: " << surf << " mm2";
      st = ss.str();
    }
    return st;
  }

}


void ObjectStatAction::displayStat()
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w )
    return;

  SelectFactory *sf = SelectFactory::factory();
  const map<unsigned, set<AObject *> > & sel = sf->selected();
  map<unsigned, set<AObject *> >::const_iterator is = sel.find( w->Group() );
  if( is == sel.end() )
    return;
  const set<AObject *> & so = is->second;
  set<AObject *>::const_iterator io, eo = so.end();
  AObject *obj = 0;
  string s;
  for( io=so.begin(); io!=eo; ++io )
    if( w->hasObject( *io ) && !( s = objectStatus( *io, w ) ).empty() )
    {
      if( obj )
      {
        obj = 0;
        break;
      }
      else
        obj = *io;
    }
  if( !obj )
    w->statusBar()->showMessage( "select one object", 1500 );
  else
    w->statusBar()->showMessage( s.c_str() );
}


// ------


#if QT_VERSION >= 0x040600

struct PinchZoomAction::Private
{
  Private() : orgzoom( 1. ), organgle( 0. ), orgscale( 1. ),
    current_zoom( 1. ),
    current_angle( 0. ), current_trans( 0., 0. ), max_scl_fac( 1.15 ),
    max_angle_diff( 20. ), max_trans( 30. ), delay( 20 ), count( 0 )
  {}

  float      orgzoom;
  Quaternion orgquaternion;
//   Point3df   startpos;
  float      organgle;
  float      orgscale;
  float current_zoom;
  float current_angle;
  QPointF current_trans;
  float max_scl_fac;
  float max_angle_diff;
  float max_trans;
  int delay;
  int count;
};


PinchZoomAction::PinchZoomAction()
  : Action(), d( new Private )
{
}


PinchZoomAction::~PinchZoomAction()
{
  delete d;
}


void PinchZoomAction::pinchStart( QPinchGesture *gesture )
{
  // cout << "PinchZoomAction start\n";

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
  {
    cerr << "Zoom3DAction operating on wrong view type -- error\n";
    return;
    d->orgzoom = 1;
  }
  d->orgzoom = w->zoom();
  d->orgquaternion = w->quaternion();
  d->orgscale = gesture->totalScaleFactor();
  d->organgle = gesture->totalRotationAngle();
  d->current_zoom = 1.;
  d->current_angle = gesture->totalRotationAngle();
  // d->current_trans = QPointF( 0., 0. );
  d->current_trans = gesture->centerPoint();
  d->count = 0;
}


void PinchZoomAction::pinchMove( QPinchGesture *gesture )
{
  /*
  cout << "PinchZoomAction move\n";
  cout << "scale: " << gesture->totalScaleFactor() << endl;
  cout << "angle: " << gesture->totalRotationAngle() << endl;
  */
//   cout << "diff: " << gesture->centerPoint() - gesture->startCenterPoint() << endl;

  // skip first events to wait stabilization
  // (some devices send hazardous values at the beginning)
  if( d->count < d->delay )
  {
    ++ d->count;
    d->organgle = gesture->totalRotationAngle();
    d->current_angle = gesture->totalRotationAngle();
    d->orgscale = gesture->totalScaleFactor();
    d->current_trans = gesture->centerPoint();
    return;
  }

  float zfac = gesture->totalScaleFactor() / d->orgscale;
  // attenuate too fast changes
  if( zfac / d->current_zoom > d->max_scl_fac )
    zfac = d->current_zoom * d->max_scl_fac;
  else if( d->current_zoom / zfac > d->max_scl_fac )
    zfac = d->current_zoom / d->max_scl_fac;
  d->current_zoom = zfac;

  float angle = gesture->totalRotationAngle() - d->organgle;
  float angle_diff = angle - d->current_angle;
  if( angle_diff > 180. )
    angle_diff -= 360.;
  if( angle_diff < -180. )
    angle_diff += 360.;
  if( angle_diff > d->max_angle_diff )
    angle = d->current_angle + d->max_angle_diff;
  else if( angle_diff < -d->max_angle_diff )
    angle = d->current_angle - d->max_angle_diff;
  d->current_angle = angle;
  angle = angle / 180. * M_PI;

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
    const Quaternion        & q = w->quaternion();
    float zfac2 = log( zfac * d->orgzoom ) * 100;
    Point3df p = q.transform( Point3df( 0, 0, -zfac2 ) );
    float fac = w->invertedZ() ? -1 : 1;
    p[2] = fac * p[2];        // invert Z axis
    //cout << "avance : " << p << endl;
    w->setRotationCenter( w->rotationCenter() + p );
  }
  else
  {
    // cout << "set zoom\n";
    w->setZoom( zfac * d->orgzoom );

    QPointF trans = gesture->centerPoint() - d->current_trans;
    float tdiff_norm = sqrt( trans.x() * trans.x() + trans.y() * trans.y() );
    if( tdiff_norm > d->max_trans )
    {
      /*
      cout << "too much translation: " << trans.x() << ", " << trans.y() << ": " << tdiff_norm << endl;
      cout << "center: " << gesture->centerPoint().x() << ", " << gesture->centerPoint().y() << ", last: " << gesture->lastCenterPoint().x() << ", " << gesture->lastCenterPoint().y() << endl;
      */
      trans = trans * d->max_trans / tdiff_norm;
      // cout << "back to: " << trans.x() << ", " << trans.y() << endl;
    }
    d->current_trans = gesture->centerPoint();

    Point3df t;
    w->translateCursorPosition( -trans.x(), trans.y(), t );
    w->setRotationCenter( w->rotationCenter() + t );

    Quaternion r = Quaternion();
    r.fromAxis( Point3df( 0, 0, 1. ), angle );
    w->setQuaternion( r * d->orgquaternion );
  }
  ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void PinchZoomAction::pinchStop( QPinchGesture *gesture )
{
  // cout << "PinchZoomAction stop\n";
}


Action* PinchZoomAction::creator()
{
  return new PinchZoomAction;
}


#endif

// ------

/* the following lines are needed in release mode (optimization -O3)
using gcc 4.2.2 of Mandriva 2008. Otherwise there are undefined symbols.
*/
template class Control::KeyActionLinkOf<WindowActions>;
template class Control::KeyActionLinkOf<SelectAction>;

