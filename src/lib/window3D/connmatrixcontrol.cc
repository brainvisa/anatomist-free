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

#include <anatomist/window3D/connmatrixcontrol.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/sparsematrix/connectivitymatrix.h>
#include <anatomist/surface/texture.h>
#include <anatomist/controler/control_d.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/controler/view.h>

using namespace anatomist;
using namespace carto;


Action * ConnectivityMatrixAction::creator()
{
  return new ConnectivityMatrixAction;
}


ConnectivityMatrixAction::ConnectivityMatrixAction()
  : Action()
{
}


ConnectivityMatrixAction::ConnectivityMatrixAction( 
  const ConnectivityMatrixAction & a )
  : Action( a )
{
}


ConnectivityMatrixAction::~ConnectivityMatrixAction()
{
}


void ConnectivityMatrixAction::showConnectivityAtPoint( int x, int y, 
                                                        int, int )
{
  cout << "showConnectivityAtPoint\n";
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w )
    return;
  AObject *obj = w->objectAtCursorPosition( x, y );
  cout << "object: " << obj << endl;
  if( !obj )
    return;
  AObject::ParentList & parents = obj->parents();
  AObject::ParentList::iterator ip, ep = parents.end();
  AConnectivityMatrix *aconn = 0;
  for( ip=parents.begin(); !aconn && ip!=ep; ++ip )
    aconn = dynamic_cast<AConnectivityMatrix *>( *ip );
  cout << "conn: " << aconn << endl;
  if( !aconn )
    return;
  rc_ptr<AimsSurfaceTriangle> mesh = aconn->mesh()->surface();
  int poly = w->polygonAtCursorPosition( x, y, aconn );
  cout << "poly: " << poly << endl;
  if( poly == 0xffffff || poly < 0 || poly >= mesh->polygon().size() )
    return;
  const AimsVector<uint,3> & ppoly = mesh->polygon()[ poly ];
  const vector<Point3df> & vert = mesh->vertex();
  Point3df pos;
  if( !w->positionFromCursor( x, y, pos ) )
    return;
  cout << "pos: " << pos << endl;
  Point3df d( ( vert[ppoly[0]]-pos ).norm2(), 
              ( vert[ppoly[1]]-pos ).norm2(),
              ( vert[ppoly[2]]-pos ).norm2() );
  int imin = d[0] <= d[1] ? 0 : 1;
  imin = d[imin] <= d[2] ? imin : 2;
  uint v = ppoly[ imin ]; // nearest point
  cout << "vertex: " << v << ", " << vert[v] << endl;
  aconn->buildTexture( v );
  aconn->texture()->notifyObservers();
  aconn->marker()->notifyObservers();
}


void ConnectivityMatrixAction::showConnectivityForPatch( int x, int y, 
                                                         int, int )
{
  cout << "showConnectivityForPatch\n";
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w )
    return;
  AObject *obj = w->objectAtCursorPosition( x, y );
  if( !obj )
    return;
  AObject::ParentList & parents = obj->parents();
  AObject::ParentList::iterator ip, ep = parents.end();
  AConnectivityMatrix *aconn = 0;
  for( ip=parents.begin(); !aconn && ip!=ep; ++ip )
    aconn = dynamic_cast<AConnectivityMatrix *>( *ip );
  if( !aconn )
    return;
  rc_ptr<AimsSurfaceTriangle> mesh = aconn->mesh()->surface();
  int poly = w->polygonAtCursorPosition( x, y, aconn );
  if( poly == 0xffffff || poly < 0 || poly >= mesh->polygon().size() )
    return;
  const AimsVector<uint,3> & ppoly = mesh->polygon()[ poly ];
  const vector<Point3df> & vert = mesh->vertex();
  Point3df pos;
  if( !w->positionFromCursor( x, y, pos ) )
    return;
  Point3df d( ( vert[ppoly[0]]-pos ).norm2(),
              ( vert[ppoly[1]]-pos ).norm2(),
              ( vert[ppoly[2]]-pos ).norm2() );
  int imin = d[0] <= d[1] ? 0 : 1;
  imin = d[imin] <= d[2] ? imin : 2;
  uint v = ppoly[ imin ]; // nearest point
  cout << "vertex: " << v << ", " << vert[v] << endl;
  aconn->buildPatchTexture( v );
  aconn->texture()->notifyObservers();
  aconn->marker()->notifyObservers();
}


Control * ConnectivityMatrixControl::creator()
{
  return new ConnectivityMatrixControl;
}


ConnectivityMatrixControl::ConnectivityMatrixControl()
  : Control( 20, "ConnectivityMatrixControl" )
{
}


ConnectivityMatrixControl::~ConnectivityMatrixControl()
{
}



void ConnectivityMatrixControl::eventAutoSubscription(
  ActionPool * actionPool )
{
  mousePressButtonEventSubscribe( Qt::LeftButton, Qt::NoModifier,
    MouseActionLinkOf<ConnectivityMatrixAction>( 
      actionPool->action( "ConnectivityMatrixAction" ), 
      &ConnectivityMatrixAction::showConnectivityAtPoint ) );
  mousePressButtonEventSubscribe( Qt::LeftButton, Qt::ControlModifier,
    MouseActionLinkOf<ConnectivityMatrixAction>( 
      actionPool->action( "ConnectivityMatrixAction" ), 
      &ConnectivityMatrixAction::showConnectivityForPatch ) );

  // standard actions
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ShiftButton,
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

  //    rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlButton,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltButton,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ) );

  //    sync
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

  //    translation

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
}


