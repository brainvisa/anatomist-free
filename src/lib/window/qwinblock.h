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

#ifndef ANATOMIST_WINDOW_QWINBLOCK_H
#define ANATOMIST_WINDOW_QWINBLOCK_H

#include <anatomist/observer/Observer.h>
#include <qmainwindow.h>


class QGridLayout;
class QLabel;
class QTimer;


namespace anatomist
{
  class AWindow;
}


class QAWindowBlock : public QMainWindow, public anatomist::Observer
{
  Q_OBJECT

public:
  /** colsrows is either the number of columns (if inrows is true), or the
      number of rows (if inrows is false).
   */
  QAWindowBlock( QWidget* parent = 0, const char* name = 0, 
                 Qt::WindowFlags f = Qt::Window,
                 int colsrows = 2, bool inrows = false, bool withmenu = true );
  virtual ~QAWindowBlock();
  void addWindowToBlock( QWidget* item, bool withBorders = true );
  void setColsOrRows( bool inrows, int colsrows );
  void arrangeInRect( float widthHeightRatio = 1. );
  bool inRows() const;
  int columnCount() const;
  int rowCount() const;
  float widthHeightRatio() const;
  virtual void update( const anatomist::Observable*, void* );
  virtual void unregisterObservable( anatomist::Observable* );
  void cleanupLayout();
  void reorderViews( const std::list<QWidget *> & wins );
  void dropRowCol( int x, int y, int & row, int & col,
                   bool switching = false ) const;
  void dropRowColDirection( int x, int y, int & row, int & col,
                            int & dir, bool switching = false ) const;
  virtual QSize sizeHint() const;
  void setDefaultBlockGui( bool );

public slots:
  void layInColumns();
  void layInRows();
  void layInRectangle();
  void setColumnsNumber();
  void setRowsNumber();
  void setRectangularRatio();
  void setDefaultBlock( bool );

protected slots:
  void windowsDropped();

protected:
  virtual void dragEnterEvent( QDragEnterEvent* event );
  virtual void dragMoveEvent( QDragMoveEvent* event );
  virtual void dragLeaveEvent( QDragLeaveEvent* event );
  virtual void dropEvent( QDropEvent* );
  virtual void closeEvent( QCloseEvent * event );

private:
  struct Private;
  Private	*d;
};


namespace anatomist
{

  class BlockBorderWidget : public QWidget
  {
  public:
    BlockBorderWidget( int sides );
    virtual ~BlockBorderWidget();
    void getRowCol( int & row, int & col, int & dirx, int & diry ) const;
    /** get the layout where the parent of this (which should be a
        DraggableWrapper) is: that is, the QAWindowBlock grid layout.
        If not found, return 0.
    */
    QGridLayout* parentGridLayout() const;

  protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseDoubleClickEvent( QMouseEvent *event );

  private:
    int _sides;
    bool _pressed;
    int _last_x;
    int _last_y;
  };


  /** Taken from:
      https://forum.qt.io/topic/151855/grid-layout-widget-and-splitter-like-resizing/9
  */
  class DraggableWrapper : public QWidget
  {
  public:
    DraggableWrapper( QWidget *widget, bool withDragGrip = true );
    virtual ~DraggableWrapper();

  protected:
    virtual void mouseMoveEvent( QMouseEvent *event );

  private:
    bool _enableDragGrip;
    QLabel *_dragWidget;
    QTimer *_timer;
  };

}

#endif

