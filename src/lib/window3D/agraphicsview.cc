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
  : QGraphicsView( parent )
{
}


AGraphicsView::~AGraphicsView()
{
}


void AGraphicsView::resizeEvent( QResizeEvent* event )
{
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
    {
      ControlSwitch *cs = glm->controlSwitch();
      cs->mousePressEvent( event );
    }
  }
}


void AGraphicsView::mouseReleaseEvent( QMouseEvent* event )
{
  QGraphicsView::mouseReleaseEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    ControlSwitch *cs = glm->controlSwitch();
    cs->mouseReleaseEvent( event );
  }
}


void AGraphicsView::mouseMoveEvent( QMouseEvent* event )
{
  QGraphicsView::mouseMoveEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    ControlSwitch *cs = glm->controlSwitch();
    cs->mouseMoveEvent( event );
  }
}


void AGraphicsView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QGraphicsView::mouseDoubleClickEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      ControlSwitch *cs = glm->controlSwitch();
      cs->mouseDoubleClickEvent( event );
    }
  }
}


void AGraphicsView::keyPressEvent( QKeyEvent* event )
{
  QGraphicsView::keyPressEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      ControlSwitch *cs = glm->controlSwitch();
      cs->keyPressEvent( event );
    }
  }
}


void AGraphicsView::keyReleaseEvent( QKeyEvent* event )
{
  QGraphicsView::keyReleaseEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    ControlSwitch *cs = glm->controlSwitch();
    cs->keyReleaseEvent( event );
  }
}


void AGraphicsView::focusInEvent( QFocusEvent * event )
{
  QGraphicsView::focusInEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    ControlSwitch *cs = glm->controlSwitch();
    cs->focusInEvent( event );
  }
}


void AGraphicsView::focusOutEvent( QFocusEvent * event )
{
  QGraphicsView::focusOutEvent( event );
  GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
  if( glm )
  {
    ControlSwitch *cs = glm->controlSwitch();
    cs->focusOutEvent( event );
  }
}


void AGraphicsView::wheelEvent( QWheelEvent * event )
{
  QGraphicsView::wheelEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      ControlSwitch *cs = glm->controlSwitch();
      cs->wheelEvent( event );
    }
  }
}


void AGraphicsView::dragEnterEvent( QDragEnterEvent* event )
{
  QGraphicsView::dragEnterEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      AWindow3D *w = dynamic_cast<AWindow3D *>( glm->aWindow() );
      if( w )
        w->dragEnterEvent( event );
    }
  }
}


void AGraphicsView::dragMoveEvent( QDragMoveEvent* event )
{
  QGraphicsView::dragMoveEvent( event );
  if( !event->isAccepted() )
  {
    GLWidgetManager *glm = dynamic_cast<GLWidgetManager *>( viewport() );
    if( glm )
    {
      AWindow3D *w = dynamic_cast<AWindow3D *>( glm->aWindow() );
      if( w )
        w->dragMoveEvent( event );
    }
  }
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


