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


#include <anatomist/window/glwidget.h>
#include <QGestureEvent>
#include <QApplication>
#include <QPaintEvent>
#include <QTouchEvent>


using namespace anatomist;
using namespace carto;
using namespace std;


#ifdef ANA_USE_QOPENGLWIDGET
QAGLWidget::QAGLWidget( anatomist::AWindow* win, QWidget* parent,
                        const char* name, const QOpenGLWidget * /*shareWidget*/,
                        Qt::WindowFlags f )
  : QOpenGLWidget( parent, f ),
    GLWidgetManager( win, this ),
    QOpenGLFunctions(),
    _paintDone( false )
#else
QAGLWidget::QAGLWidget( anatomist::AWindow* win, QWidget* parent,
                        const char* name, const QGLWidget * shareWidget,
                        Qt::WindowFlags f )
  : GLWidget( parent, name, shareWidget, f ),
    GLWidgetManager( win, this ),
    _paintDone( false )
#endif
{
  setAttribute( Qt::WA_AcceptTouchEvents );
  setAttribute( Qt::WA_TouchPadAcceptSingleTouchEvents );
  grabGesture( Qt::PinchGesture );
  grabGesture( Qt::PanGesture );
  grabGesture( Qt::SwipeGesture );
  grabGesture( Qt::TapGesture );
  grabGesture( Qt::TapAndHoldGesture );
  setObjectName( name );

//   cout << "UpdateBehavior: " << updateBehavior() << endl;
//   setUpdateBehavior( QOpenGLWidget::PartialUpdate );
//   connect( this, SIGNAL( frameSwapped() ), this, SLOT( debugPrint() ) );
}


QAGLWidget::~QAGLWidget()
{
}

void QAGLWidget::debugPrint()
{
  cout << "FRAME SWAPPED\n";
  if( !_paintDone )
  {
    makeCurrent();
    paintGL();
//     context()->swapBuffers( context()->surface() );
//     QPaintEvent *pe = new QPaintEvent( QRect( 0, 0, width(), height() ) );
//     qApp->postEvent( this, pe );
  }
  else
    _paintDone = false;
}


QSize QAGLWidget::sizeHint() const
{
  return GLWidgetManager::sizeHint();
}


QSize QAGLWidget::minimumSizeHint() const
{
  return GLWidgetManager::minimumSizeHint();
}


void QAGLWidget::updateGL()
{
  GLWidgetManager::updateGL();
  emit viewRendered();
}

void QAGLWidget::initializeGL()
{
#ifdef ANA_USE_QOPENGLWIDGET
  initializeOpenGLFunctions();
#endif
  GLWidgetManager::initializeGL();
}


void QAGLWidget::resizeGL( int w, int h )
{
  GLWidgetManager::resizeGL( w, h );
}


void QAGLWidget::paintGL()
{
  GLWidgetManager::paintGL();
  _paintDone = true;
}


bool QAGLWidget::event( QEvent * event )
{
  /* QOpenGLWidget produces an uncontrolled swapBuffers() on the followning
     events:
     - Enter
     - Leave
     - WindowActivate
     - WindowDeactivate
  */
  // cout << "EVENT: " << event->type() << endl;
  if( event->type() == QEvent::Enter
      || event->type() == QEvent::Leave
      || event->type() == QEvent::WindowActivate
      || event->type() == QEvent::WindowDeactivate )
  {
//     makeCurrent();
//     paintGL();
//     event->accept();
//     return true;
  }

  if( event->type() == QEvent::Gesture )
  {
    // cout << "-- gesture --\n";
    gestureEvent( static_cast<QGestureEvent*>( event ) );
    if( event->isAccepted() )
      return true;
  }
  if( event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate
      || event->type() == QEvent::TouchEnd )
  {
    // cout << "GLW TOUCH\n";
    touchEvent( static_cast<QTouchEvent*>( event ) );
    if( event->isAccepted() )
      return true;
  }

#ifdef ANA_USE_QOPENGLWIDGET
  return QOpenGLWidget::event(event);
#else
  return QGLWidget::event(event);
#endif
}


void QAGLWidget::mousePressEvent( QMouseEvent* me )
{
  GLWidgetManager::mousePressEvent( me );
}


void QAGLWidget::mouseReleaseEvent( QMouseEvent* me )
{
  GLWidgetManager::mouseReleaseEvent( me );
}


void QAGLWidget::mouseMoveEvent( QMouseEvent* me )
{
  GLWidgetManager::mouseMoveEvent( me );
}


void QAGLWidget::mouseDoubleClickEvent( QMouseEvent* me )
{
  GLWidgetManager::mouseDoubleClickEvent( me );
}


void QAGLWidget::keyPressEvent( QKeyEvent* ev )
{
  GLWidgetManager::keyPressEvent( ev );
}


void QAGLWidget::keyReleaseEvent( QKeyEvent* ev )
{
  GLWidgetManager::keyReleaseEvent( ev );
}


void QAGLWidget::focusInEvent( QFocusEvent * ev )
{
  GLWidgetManager::focusInEvent( ev );
#ifdef ANA_USE_QOPENGLWIDGET
  QOpenGLWidget::focusInEvent( ev );
#endif
}


void QAGLWidget::focusOutEvent( QFocusEvent * ev )
{
  GLWidgetManager::focusOutEvent( ev );
#ifdef ANA_USE_QOPENGLWIDGET
  QOpenGLWidget::focusOutEvent( ev );
#endif
}


void QAGLWidget::wheelEvent( QWheelEvent * ev )
{
  GLWidgetManager::wheelEvent( ev );
}


string QAGLWidget::name() const
{
  return GLWidgetManager::name();
}

