#ifndef ANA_DIALOGS_QCLICKGRAPHICSVIEW_H
#define ANA_DIALOGS_QCLICKGRAPHICSVIEW_H

#include <QGraphicsView>


/** QGraphicsView which emits signal for mouse press, move and release
    events.

    The normal QGraphicsView captures such events and does not expose them, so
    a widget containing the graphics view cannot react to mouse events, even if
    the graphics view does nothing with them.
*/
class QClickGraphicsView: public QGraphicsView
{
  Q_OBJECT

public:
  virtual ~QClickGraphicsView();

signals:
  /// signal emitted upon mouse press event
  void mousePressed( const QMouseEvent * );
  /// signal emitted upon mouse move event
  void mouseMoved( const QMouseEvent * );
  /// signal emitted upon mouse release event
  void mouseReleased( const QMouseEvent * );

protected:
  virtual void mousePressEvent( QMouseEvent *event ) override;
  virtual void mouseMoveEvent( QMouseEvent *event ) override;
  virtual void mouseReleaseEvent( QMouseEvent *event ) override;
};

#endif
