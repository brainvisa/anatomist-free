
#include <anatomist/dialogs/qclickgraphicsview.h>


QClickGraphicsView::~QClickGraphicsView()
{
}


void QClickGraphicsView::mousePressEvent( QMouseEvent *event )
{
  QGraphicsView::mousePressEvent( event );
  emit mousePressed( event );
}


void QClickGraphicsView::mouseMoveEvent( QMouseEvent *event )
{
  QGraphicsView::mouseMoveEvent( event );
  emit mouseMoved( event );
}


void QClickGraphicsView::mouseReleaseEvent( QMouseEvent *event )
{
  QGraphicsView::mouseReleaseEvent( event );
  emit mouseReleased( event );
}
