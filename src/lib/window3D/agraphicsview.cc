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

#include <anatomist/window3D/agraphicsview_p.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/controler/controlswitch.h>
#include <QPainter>
#include <QPaintEngine>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>


using namespace anatomist;
using namespace anatomist::internal;
using namespace std;


// AGraphicsView

AGraphicsView::AGraphicsView( QWidget* parent )
  : QGraphicsView( parent ), _sizehint( QSize( 0, 0 ) )
{
  setAttribute( Qt::WA_AcceptTouchEvents );
  setAttribute( Qt::WA_TouchPadAcceptSingleTouchEvents );
  grabGesture( Qt::SwipeGesture );
  grabGesture( Qt::PanGesture );
  grabGesture( Qt::PinchGesture );
  grabGesture( Qt::TapGesture );
  grabGesture( Qt::TapAndHoldGesture );
}


AGraphicsView::~AGraphicsView()
{
}


bool AGraphicsView::event( QEvent* ev )
{
  // cout << "GraphicsView event: " << ev->type() << endl;
  bool res = QGraphicsView::event( ev );
  // cout << "accepted: " << ev->isAccepted() << endl;
  if( /* !ev->isAccepted() && */ ( ev->type() == QEvent::Gesture || ev->type() == QEvent::GestureOverride ) )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      glm->gestureEvent( static_cast<QGestureEvent*>( ev ) );
      // ev->ignore();
      if( ev->isAccepted() )
        return true;
    }
  }
  return res;
}


void AGraphicsView::setSizeHint( const QSize & sz )
{
  _sizehint = sz;
}


QSize AGraphicsView::sizeHint() const
{
  if( _sizehint != QSize( 0, 0 ) )
    return _sizehint;
  return QGraphicsView::sizeHint();
}


void AGraphicsView::resizeEvent( QResizeEvent* event )
{
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  // non-visible widgets (like viewport()) don't receive resize events...
  if( glm )
    glm->resizeGL( event->size().width(), event->size().height() );

  if( scene() )
    scene()->setSceneRect(
      0, 0, event->size().width(), event->size().height() );
  QGraphicsView::resizeEvent( event );
}


void AGraphicsView::mousePressEvent( QMouseEvent* event )
{
  QGraphicsView::mousePressEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
      glm->mousePressEvent( event );
  }
}


void AGraphicsView::mouseReleaseEvent( QMouseEvent* event )
{
  QGraphicsView::mouseReleaseEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
    glm->mouseReleaseEvent( event );
}


void AGraphicsView::mouseMoveEvent( QMouseEvent* event )
{
  QGraphicsView::mouseMoveEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
    glm->mouseMoveEvent( event );
}


void AGraphicsView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QGraphicsView::mouseDoubleClickEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
      glm->mouseDoubleClickEvent( event );
  }
}


void AGraphicsView::keyPressEvent( QKeyEvent* event )
{
  /* process via QGraphicsView first, but with a few exceptions:
     Key_Up and Key_Down seem to be systematically "eaten" by the
     QGraphicsView, and we still need them for the controls system.
  */
  set<int> reservedKeys;
  reservedKeys.insert( Qt::Key_PageUp );
  reservedKeys.insert( Qt::Key_PageDown );
  reservedKeys.insert( Qt::Key_Up );
  reservedKeys.insert( Qt::Key_Down );
  reservedKeys.insert( Qt::Key_Left );
  reservedKeys.insert( Qt::Key_Right );
  if( reservedKeys.find( event->key() ) == reservedKeys.end() )
    QGraphicsView::keyPressEvent( event );
  else
    event->setAccepted( false );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
      glm->keyPressEvent( event );
  }
}


void AGraphicsView::keyReleaseEvent( QKeyEvent* event )
{
  QGraphicsView::keyReleaseEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
    glm->keyReleaseEvent( event );
}


void AGraphicsView::focusInEvent( QFocusEvent * event )
{
  QGraphicsView::focusInEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
    glm->focusInEvent( event );
}


void AGraphicsView::focusOutEvent( QFocusEvent * event )
{
  QGraphicsView::focusOutEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
    glm->focusOutEvent( event );
}


void AGraphicsView::wheelEvent( QWheelEvent * event )
{
  QGraphicsView::wheelEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
      glm->wheelEvent( event );
  }
}


void AGraphicsView::dragEnterEvent( QDragEnterEvent* event )
{
  event->ignore();
  QGraphicsView::dragEnterEvent( event );
  // Problem: all events are accepted in QGraphicsView::dragEnterEvent(),
  // which forbids filtering
  event->ignore();
//   if( !event->isAccepted() )
//   {
//     GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
//     if( glm )
//     {
//       AWindow3D *w = dynamic_cast<AWindow3D *>( glm->aWindow() );
//       if( w )
//         w->dragEnterEvent( event );
//     }
//   }
//   cout << "accepted (2): " << event->isAccepted() << endl;
}


void AGraphicsView::dragMoveEvent( QDragMoveEvent* event )
{
  QGraphicsView::dragMoveEvent( event );
//   if( !event->isAccepted() )
//   {
//     GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
//     if( glm )
//     {
//       AWindow3D *w = dynamic_cast<AWindow3D *>( glm->aWindow() );
//       if( w )
//         w->dragMoveEvent( event );
//     }
//   }
}


void AGraphicsView::dropEvent( QDropEvent* event )
{
  /* We should try the QGraphicsView first to enable dropping on graphics items
     or widgets, but unfortunately, QGraphicsView:dropEvent always accepts the
     event, so we don't know if drop has actually taken place.
  */
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    AWindow3D *w = dynamic_cast<AWindow3D *>( glm->aWindow() );
    if( w )
      w->dropEvent( event );
  }
  if( !event->isAccepted() )
    QGraphicsView::dropEvent( event );
}


// AGraphicsScene

AGraphicsScene::AGraphicsScene( QObject* parent )
  : QGraphicsScene( parent )
{
}


AGraphicsScene::~AGraphicsScene()
{
}


void AGraphicsScene::drawBackground( QPainter* painter, const QRectF & )
{
  // cout << "drawBackground\n";
  if( painter->paintEngine()->type() != QPaintEngine::OpenGL
    && painter->paintEngine()->type() != QPaintEngine::OpenGL2 )
  {
    cerr << "AGraphicsScene: drawBackground needs a QGLWidget to be set as"
      "viewport on the graphics view" << endl;
    cerr << "paintEngine type: " << painter->paintEngine()->type() << endl;
    return;
  }
  QList<QGraphicsView *> gviews = views();
  QList<QGraphicsView *>::iterator ig, eg = gviews.end();
  for( ig=gviews.begin(); ig!=eg; ++ig )
  {
    GLWidgetManager* glm = dynamic_cast<GLWidgetManager *>( 
      (*ig)->viewport() );
    if( glm )
      glm->paintScene();
  }
//   QTimer::singleShot( 20, this, SLOT( update() ) );
}


