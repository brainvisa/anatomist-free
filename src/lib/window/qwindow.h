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


#ifndef ANATOMIST_WINDOW_QWINDOW_H
#define ANATOMIST_WINDOW_QWINDOW_H

#include <qmainwindow.h>
#include <anatomist/window/Window.h>
#include <anatomist/controler/controlswitch.h>


namespace anatomist
{
  struct QAWindow_PrivateData;
}

/**	Anatomist window with Qt */
class QAWindow : public QMainWindow, public anatomist::AWindow, 
		 public anatomist::ControlSwitchObserver
{
  Q_OBJECT

public:

  QAWindow( QWidget* parent = 0, const char* name = 0, 
	    carto::Object params = carto::none(), 
            Qt::WFlags f = 0 );
  virtual ~QAWindow();

  virtual void setGeometry( int x, int y, unsigned w, unsigned h );
  virtual void geometry( int *x, int *y, unsigned *w, unsigned *h );

  virtual void show();
  virtual void hide();
  virtual void iconify();
  virtual void unIconify();
  virtual bool close();
  virtual void showToolBars( int state = 2 );
  void setFullScreen( int state = 2 );
  bool isFullScreen() const;
  virtual void SetTitle( const std::string & name );
  virtual void enableDetachMenu( bool x );

  /**	In QAWindows, Refresh() doesn't redraw right now. Instead, it triggers 
	a timer do redraw in a few mseconds, so when multiple Refresh() are 
	requested, only one redraw is performed.
	As a result, subclasses of QAWindow MUSTN'T overload Refresh(), but 
	overload refreshNow() instead, which is the effective drawing function 
	called when the timer timesout. That's why Refresh is not virtual 
	anymore. */
  void Refresh();
  virtual QToolBar* addToolBar( const QString & title, const QString & name );
  virtual void addToolBar( QToolBar*, const QString & name );
  virtual void addToolBar( Qt::ToolBarArea area, QToolBar* toolbar,
                           const QString & name );
  virtual void removeToolBar( QToolBar * toolbar );
  virtual QToolBar* removeToolBar( const QString & name );
  QToolBar* toolBar( const QString & name );
  void setDetachMenuAction( QAction* );

public slots:
  /**	Real drawing function, replacing AWindow::Refresh in subclasses of 
	QAWindow - Don't forget to call QAWindow::refreshNow from overloaded 
	functions */
  virtual void refreshNow();
  virtual bool needsRedraw() const;
  /// detach view from a block
  virtual void detach();
  virtual void toggleToolBars();
  virtual void toggleFullScreen();

protected slots:
  /// calls refreshNow() and cleanup
  void triggeredRefresh();

protected:
  virtual void CreateTitle();
  virtual void dragEnterEvent( QDragEnterEvent* );
  virtual void dropEvent( QDropEvent* );
  virtual void mouseMoveEvent( QMouseEvent * e );
  /* in qt4 we have to find a way to catch close events and prevent deletion
     if ref-counting doesn't allow it */
  virtual void closeEvent( QCloseEvent * event );

private:
  struct Private;
  Private	*d;

};


#endif
