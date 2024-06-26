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


#ifndef ANATOMIST_WINDOW3D_AGRAPHICSVIEW_H
#define ANATOMIST_WINDOW3D_AGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>

namespace anatomist
{

  namespace internal
  {

    /// see http://doc.qt.digia.com/qq/qq26-openglcanvas.html
    class AGraphicsView : public QGraphicsView
    {
    public:
      AGraphicsView( QWidget* parent = 0 );
      virtual ~AGraphicsView();

      void setSizeHint( const QSize & );
      virtual QSize sizeHint() const;

    protected:
      virtual bool event( QEvent *ev );
      virtual void resizeEvent( QResizeEvent* event );
      virtual void mousePressEvent( QMouseEvent* me );
      virtual void mouseReleaseEvent( QMouseEvent* me );
      virtual void mouseMoveEvent( QMouseEvent* me );
      virtual void mouseDoubleClickEvent( QMouseEvent * );
      virtual void keyPressEvent( QKeyEvent* ev );
      virtual void keyReleaseEvent( QKeyEvent* ev );
      virtual void focusInEvent( QFocusEvent * );
      virtual void focusOutEvent( QFocusEvent * );
      virtual void wheelEvent( QWheelEvent * );
      virtual void dragEnterEvent( QDragEnterEvent* );
      virtual void dragMoveEvent( QDragMoveEvent* );
      virtual void dropEvent( QDropEvent* );

    private:
      QSize _sizehint;
    };


    class AGraphicsScene : public QGraphicsScene
    {
    public:
      AGraphicsScene( QObject* parent = 0 );
      virtual ~AGraphicsScene();

    protected:
      virtual void drawBackground( QPainter* painter, const QRectF & );
    };

  }

}

#endif

