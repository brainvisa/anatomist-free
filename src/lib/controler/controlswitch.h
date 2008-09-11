/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#ifndef ANATOMIST_CONTROLER_CONTROLSWITCH_H
#define ANATOMIST_CONTROLER_CONTROLSWITCH_H


#include <anatomist/controler/control.h>
#include <anatomist/controler/actionpool.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/application/Anatomist.h>

#include <qobject.h>
#include <qpushbutton.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qhbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <vector>
#include <list>

  
class ControlSwitch ;


namespace anatomist
{

  class ControlSwitchObserver
  {
  public:
    ControlSwitchObserver( ) ;
    virtual ~ControlSwitchObserver( ) {}

    virtual void updateAvailableControls( ) {}
    virtual void updateActivableControls( ) {}
    virtual void updateActiveControl() {}
    virtual void updateActions( ) {}

  protected:
  } ;

}

class ToolBox : public QWidget
{

  Q_OBJECT

public:
  ToolBox( const std::string& activeControlDescription ) ;
  virtual ~ToolBox() ;

  void resetActions( ) ;
  void updateActiveControl( const std::string& activeControlDescription ) ;

  void addTab( QWidget * child, const QString & label ) ;
  void showPage ( QWidget * w ) ;
  void showPage( const std::string & label );

private slots :
  void switchControlDescriptionActivation() ;

private:
  struct Private;

  QVBoxLayout * myLayout ;
  QTabWidget * myActionTab ;
  QPushButton * myControlDescriptionActivation ;
  std::string myControlDescription ;
  QLabel * myControlDescriptionWidget, *l1, *l2, *l3, *l4 ;

  bool myDescriptionActivated ;
  Private	*d;
};


class ControlSwitch : public QObject
{

  Q_OBJECT

public:
  virtual ~ControlSwitch() ;

  friend class anatomist::View ; 

  bool attach( anatomist::ControlSwitchObserver * window ) ;
  bool detach( anatomist::ControlSwitchObserver * window ) ;

  void notifyActivableControlChange() ;
  void notifyAvailableControlChange() ;
  void notifyActiveControlChange() ;
  void notifyActionChange() ;

  const std::map<int, std::string>& activableControls( ) const ;
  const std::map<int, std::string>& availableControls( ) const ;
  const std::set<std::string>& activableControlGroups( ) const ;
  const std::set<std::string>& availableControlGroups( ) const ;  
  const std::set<std::string>& actions() const ;

  //vector<ControlPtr>::iterator iterator ;
  typedef std::map<std::string, anatomist::ControlPtr>::const_iterator 
  const_iterator ;
  typedef std::map<std::string, anatomist::ControlPtr>::iterator 
  iterator ;

  //iterator begin() ;
  const_iterator begin() const ;
  //iterator end() ;
  const_iterator end() const ;
  void setActiveControl( const std::string& ) ;
  const std::string& activeControl() const ;

  void keyPressEvent( QKeyEvent *) ;
  void keyReleaseEvent( QKeyEvent *) ;
  void mousePressEvent ( QMouseEvent * ) ;
  void mouseReleaseEvent ( QMouseEvent * ) ;
  void mouseDoubleClickEvent ( QMouseEvent * ) ;
  void mouseMoveEvent ( QMouseEvent * ) ;
  void wheelEvent ( QWheelEvent * ) ;
  void focusInEvent ( QFocusEvent * ) ;
  void focusOutEvent ( QFocusEvent * ) ;
  void enterEvent ( QEvent * ) ;
  void leaveEvent ( QEvent * ) ;
  void paintEvent ( QPaintEvent * ) ;
  void moveEvent ( QMoveEvent * ) ;
  void resizeEvent ( QResizeEvent * ) ;
  void dragEnterEvent ( QDragEnterEvent * ) ;
  void dragMoveEvent ( QDragMoveEvent * ) ;
  void dragLeaveEvent ( QDragLeaveEvent * ) ;
  void dropEvent ( QDropEvent * ) ;
  void showEvent ( QShowEvent * ) ;
  void hideEvent ( QHideEvent * ) ;
  void selectionChangedEvent();
  //void customEvent ( QCustomEvent * ) ; 

  void setAvailableControls( const std::list<std::string>& objects ) ;
  void setActivableControls( bool init = false) ;

  //   void setControlBarButtons() ;

  void printControls() ;
  // private slots:
  //   void clicked( int id ) ;

  void updateToolBox() ;
  void updateControlDescription() ;

  bool isToolBoxVisible() const;
  void switchToolBoxVisible() ;
  ToolBox* toolBox();

  anatomist::Action* getAction( const std::string& actionName ) ;

protected:
  ControlSwitch( anatomist::View * view ) ;

private:
  void init( ) ;
  void getSelectedObjectNames() ;

  anatomist::ActionPool * myActionPool ;
  //   QHButtonGroup * myControlBar ;
  std::string myViewType ;
  std::map<std::string, anatomist::ControlPtr> myControls ;

  std::map<int, std::string> myAvailableControls ;
  std::map<int, std::string> myActivableControls ;

  std::set<std::string> myAvailableControlGroups ;
  std::set<std::string> myActivableControlGroups ;
  std::list<std::string> mySelectedObjects ;

  std::list<anatomist::ControlSwitchObserver *> myObservers ;

  bool myControlEnabled ;
  std::string myActiveControl ;

  ToolBox * myToolBox ;

private slots:
  void toolBoxDestroyed();

};

#endif
