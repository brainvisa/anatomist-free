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
#include <anatomist/window/glwidgetmanager.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QProgressBar>
#include <QGraphicsProxyWidget>

using namespace anatomist;
using namespace carto;
using namespace std;


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
  : QObject(),
    Action( a )
{
}


ConnectivityMatrixAction::~ConnectivityMatrixAction()
{
}


void ConnectivityMatrixAction::showConnectivityAtPoint( int x, int y, 
                                                        int, int )
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w )
    return;
  AObject *obj = w->objectAtCursorPosition( x, y );
  // obj should be a textured surface
  AConnectivityMatrix *aconn = connectivityMatrix( obj );
  if( !aconn )
    return;
  int poly = w->polygonAtCursorPosition( x, y, aconn );
  if( poly == 0xffffff || poly < 0 )
    return;
  Point3df pos;
  if( !w->positionFromCursor( x, y, pos ) )
    return;

  aconn->cancelThread();

  vector<rc_ptr<ATriangulated > > meshes = aconn->meshes();
  vector<rc_ptr<ATriangulated > >::iterator im, em = meshes.end();
  int index = 0;
  MObject *mobj = dynamic_cast<MObject *>( obj );

  for( im=meshes.begin(); im!=em; ++im, ++index )
  {
    if( obj == im->get() )
      break;
    if( mobj )
    {
      AObject::ParentList & mparents = (*im)->parents();
      if( mparents.find( mobj ) != mparents.end() )
        break;
    }
  }
  if( im == em )
    // mesh not found
    return;
  const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( w->getTime() );
  if( static_cast<size_t>(poly) >= surf->polygon().size() )
    return;
  const AimsVector<uint,3> & ppoly = surf->polygon()[ poly ];
  const vector<Point3df> & vert = surf->vertex();
  Point3df d( ( vert[ppoly[0]]-pos ).norm2(),
              ( vert[ppoly[1]]-pos ).norm2(),
              ( vert[ppoly[2]]-pos ).norm2() );
  int imin = d[0] <= d[1] ? 0 : 1;
  imin = d[imin] <= d[2] ? imin : 2;
  uint v = ppoly[ imin ]; // nearest point
  aconn->buildTexture( index, v, w->getTime() );
  vector<rc_ptr<ATexture> > textures = aconn->textures();
  vector<rc_ptr<ATexture> >::iterator it, et = textures.end();
  for( it=textures.begin(); it!=et; ++it )
    (*it)->notifyObservers();
  aconn->marker()->notifyObservers();
}


AConnectivityMatrix* ConnectivityMatrixAction::connectivityMatrix(
  AObject * obj ) const
{
  if( !obj )
    return 0;
  AObject::ParentList & parents = obj->parents();
  AObject::ParentList::iterator ip, ep = parents.end();
  AConnectivityMatrix *aconn = 0;
  for( ip=parents.begin(); !aconn && ip!=ep; ++ip )
    aconn = dynamic_cast<AConnectivityMatrix *>( *ip );
  return aconn;
}


void ConnectivityMatrixAction::cancelProcessings()
{
  cout << "cancelProcessings\n";
  set<AObject *> objs = view()->aWindow()->Objects();
  set<AObject *>::iterator io, eo = objs.end();
  for( io=objs.begin(); io!=eo; ++io )
  {
    AConnectivityMatrix* aconn = dynamic_cast<AConnectivityMatrix *>( *io );
    if( aconn )
    {
      aconn->cancelThread();
      clearConnectivityProgress( aconn );
    }
  }
}


void ConnectivityMatrixAction::showConnectivityForPatch( int x, int y, 
                                                         int, int )
{
  AWindow3D *w = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w )
    return;
  AObject *obj = w->objectAtCursorPosition( x, y );
  AConnectivityMatrix *aconn = connectivityMatrix( obj );
  if( !aconn )
    return;
  int poly = w->polygonAtCursorPosition( x, y, aconn );
  if( poly == 0xffffff || poly < 0 )
    return;
  Point3df pos;
  if( !w->positionFromCursor( x, y, pos ) )
    return;

  aconn->cancelThread();

  vector<rc_ptr<ATriangulated > > meshes = aconn->meshes();
  vector<rc_ptr<ATriangulated > >::iterator im, em = meshes.end();
  int index = 0;
  MObject *mobj = dynamic_cast<MObject *>( obj );

  for( im=meshes.begin(); im!=em; ++im, ++index )
  {
    if( obj == im->get() )
      break;
    if( mobj )
    {
      AObject::ParentList & mparents = (*im)->parents();
      if( mparents.find( mobj ) != mparents.end() )
        break;
    }
  }
  if( im == em )
    // mesh not found
    return;
  const AimsSurface<3, Void> *surf = (*im)->surfaceOfTime( w->getTime() );
  if( static_cast<size_t>(poly) >= surf->polygon().size() )
    return;
  const AimsVector<uint,3> & ppoly = surf->polygon()[ poly ];
  const vector<Point3df> & vert = surf->vertex();
  Point3df d( ( vert[ppoly[0]]-pos ).norm2(),
              ( vert[ppoly[1]]-pos ).norm2(),
              ( vert[ppoly[2]]-pos ).norm2() );
  int imin = d[0] <= d[1] ? 0 : 1;
  imin = d[imin] <= d[2] ? imin : 2;
  uint v = ppoly[ imin ]; // nearest point

  // create progress bar
  GLWidgetManager* glw = static_cast<GLWidgetManager *>( view() );
  QWidget* parent = glw->qglWidget()->parentWidget();
  QGraphicsView* gview = dynamic_cast<QGraphicsView *>( parent );
  if( gview )
  {
    QGraphicsScene *scene = gview->scene();
    QList<QGraphicsItem *> items = scene->items();
    QProgressBar* progress = 0;
    QList<QGraphicsItem *>::iterator it, et = items.end();
    for( it=items.begin(); it!=et; ++it )
    {
      QGraphicsProxyWidget *item = dynamic_cast<QGraphicsProxyWidget *>( *it );
      if( item && item->objectName() == "progressbaritem" )
      {
        progress = dynamic_cast<QProgressBar *>( item->widget() );
        if( progress )
          break;
      }
    }
    if( !progress )
    {
      progress = new QProgressBar;
      progress->setObjectName( "progressbar" );
      QGraphicsProxyWidget *item = scene->addWidget( progress );
      item->setObjectName( "progressbaritem" );
      item->setOpacity( 0.5 );
    }
  }

  aconn->connect( aconn, SIGNAL( processingProgress( AConnectivityMatrix *,
                                                     int, int ) ),
                  this, SLOT( updateConnectivityProgress(
                                AConnectivityMatrix *, int, int ) ) );
  aconn->connect( aconn, SIGNAL( texturesUpdated( AConnectivityMatrix * ) ),
                  this,
                  SLOT( updateConnectivityObject( AConnectivityMatrix * ) ) );
  aconn->buildPatchTexture( index, v, w->getTime() );
}


void ConnectivityMatrixAction::updateConnectivityObject(
  AConnectivityMatrix *aconn )
{
  clearConnectivityProgress( aconn );
  aconn->disconnect( aconn, SIGNAL( texturesUpdated( AConnectivityMatrix * ) ),
                     this, SLOT(
                      updateConnectivityObject( AConnectivityMatrix * ) ) );
  aconn->disconnect( aconn,
                     SIGNAL( processingProgress( AConnectivityMatrix *, int,
                                                 int ) ),
                     this, SLOT( updateConnectivityProgress(
                       AConnectivityMatrix *, int, int ) ) );
  vector<rc_ptr<ATexture> > textures = aconn->textures();
  vector<rc_ptr<ATexture> >::iterator it, et = textures.end();
  for( it=textures.begin(); it!=et; ++it )
    (*it)->notifyObservers();
  aconn->marker()->notifyObservers();
}


void ConnectivityMatrixAction::updateConnectivityProgress(
  AConnectivityMatrix* /*aconn*/, int current, int count )
{
  GLWidgetManager* glw = static_cast<GLWidgetManager *>( view() );
  QWidget* parent = glw->qglWidget()->parentWidget();
  QGraphicsView* gview = dynamic_cast<QGraphicsView *>( parent );
  if( !gview )
    return;
  QGraphicsScene *scene = gview->scene();
  QList<QGraphicsItem *> items = scene->items();
  QProgressBar* progress = 0;
  QList<QGraphicsItem *>::iterator it, et = items.end();
  for( it=items.begin(); it!=et; ++it )
  {
    QGraphicsProxyWidget *item = dynamic_cast<QGraphicsProxyWidget *>( *it );
    if( item && item->objectName() == "progressbaritem" )
    {
      progress = dynamic_cast<QProgressBar *>( item->widget() );
      if( progress )
        break;
    }
  }

  if( progress )
  {
    progress->setRange( 0, count );
    progress->setValue( current );
  }
  else
    cout << "progress: " << current << " / " << count << endl;
}


void ConnectivityMatrixAction::clearConnectivityProgress(
  AConnectivityMatrix* aconn )
{
  aconn->disconnect( aconn, SIGNAL( texturesUpdated( AConnectivityMatrix * ) ),
                     this, SLOT(
                      updateConnectivityObject( AConnectivityMatrix * ) ) );
  aconn->disconnect( aconn,
                     SIGNAL( processingProgress( AConnectivityMatrix *, int,
                                                 int ) ),
                     this, SLOT( updateConnectivityProgress(
                       AConnectivityMatrix *, int, int ) ) );
  GLWidgetManager* glw = static_cast<GLWidgetManager *>( view() );
  QWidget* parent = glw->qglWidget()->parentWidget();
  QGraphicsView* gview = dynamic_cast<QGraphicsView *>( parent );
  if( !gview )
    return;
  QGraphicsScene *scene = gview->scene();
  QList<QGraphicsItem *> items = scene->items();
  QList<QGraphicsItem *>::iterator it, et = items.end();
  for( it=items.begin(); it!=et; ++it )
  {
    scene->removeItem( *it );
    delete *it;
  }
}


Control * ConnectivityMatrixControl::creator()
{
  return new ConnectivityMatrixControl;
}


ConnectivityMatrixControl::ConnectivityMatrixControl()
  : Control( 5000, "ConnectivityMatrixControl" )
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
  keyPressEventSubscribe( Qt::Key_Escape, Qt::NoModifier,
                          KeyActionLinkOf<ConnectivityMatrixAction>
                          ( actionPool->action( "ConnectivityMatrixAction" ),
                            &ConnectivityMatrixAction::cancelProcessings ) );

  // standard actions
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::ShiftModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
        &LinkAction::endLink ), true );

  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
                                     &MenuAction::execMenu ) );

  // general window shortcuts

  keyPressEventSubscribe( Qt::Key_W, Qt::ControlModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::close ) );
  keyPressEventSubscribe( Qt::Key_F9, Qt::NoModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::toggleFullScreen ) );
  keyPressEventSubscribe( Qt::Key_F10, Qt::NoModifier,
                          KeyActionLinkOf<WindowActions>
                          ( actionPool->action( "WindowActions" ),
                            &WindowActions::toggleShowTools ) );

  //    rotation center
  keyPressEventSubscribe( Qt::Key_C, Qt::ControlModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::setCenter ) );
  keyPressEventSubscribe( Qt::Key_C, Qt::AltModifier,
                          KeyActionLinkOf<Trackball>
                          ( actionPool->action( "Trackball" ),
                            &Trackball::showRotationCenter ) );

  //    sync
  keyPressEventSubscribe( Qt::Key_S, Qt::NoModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSync ) );
  keyPressEventSubscribe( Qt::Key_S, Qt::AltModifier,
                          KeyActionLinkOf<Sync3DAction>
                          ( actionPool->action( "Sync3DAction" ),
                            &Sync3DAction::execSyncOrientation ) );
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

  keyPressEventSubscribe( Qt::Key_A, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::toggleSelectAll ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::NoModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromWindow ) );
  keyPressEventSubscribe( Qt::Key_Delete, Qt::ControlModifier,
                          KeyActionLinkOf<SelectAction>
                          ( actionPool->action( "SelectAction" ),
                            &SelectAction::removeFromGroup ) );

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

  //    translation

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
                            &SliceAction::previousSlice ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::NoModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::nextSlice ) );
  keyPressEventSubscribe( Qt::Key_PageUp, Qt::ShiftModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::previousTime ) );
  keyPressEventSubscribe( Qt::Key_PageDown, Qt::ShiftModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::nextTime ) );
  keyPressEventSubscribe( Qt::Key_L, Qt::ControlModifier,
                          KeyActionLinkOf<SliceAction>
                          ( actionPool->action( "SliceAction" ),
                            &SliceAction::toggleLinkedOnSlider ) );
}


void ConnectivityMatrixControl::doAlsoOnDeselect( ActionPool * actionPool )
{
  KeyActionLinkOf<ConnectivityMatrixAction>(
    actionPool->action( "ConnectivityMatrixAction" ), &ConnectivityMatrixAction::cancelProcessings ).execute();
}

