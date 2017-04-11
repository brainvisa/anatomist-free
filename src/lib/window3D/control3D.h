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


#ifndef ANATOMIST_WINDOW3D_CONTROL3D_H
#define ANATOMIST_WINDOW3D_CONTROL3D_H


#include <anatomist/controler/control.h>
#include <anatomist/controler/action.h>
#include <qglobal.h>

class AWindow3D;

namespace anatomist
{

  class Control3D : public Control
  {
  public:
    static Control * creator( ) ;
    
    Control3D( int priority = 1, 
      const std::string & name = QT_TRANSLATE_NOOP( "ControlledWindow", 
        "Default 3D control" ) );
    Control3D( const Control3D & c );
    virtual ~Control3D();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
    virtual void doAlsoOnDeselect( ActionPool * actionPool );

  protected:

  private:
  };

  class Select3DControl : public Control
  {
  public:
    static Control * creator( ) ;
    Select3DControl( const std::string & name = "Selection 3D" );
    Select3DControl( const Select3DControl & c );
    virtual ~Select3DControl();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
    virtual void doAlsoOnSelect( ActionPool * actionPool );
    virtual void doAlsoOnDeselect( ActionPool * actionPool );
  };


  class FlightControl : public Control
  {
  public:
    static Control * creator( ) ;
    FlightControl();
    FlightControl( const FlightControl & c );
    virtual ~FlightControl();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
    virtual void doAlsoOnDeselect( ActionPool * actionPool );
  };


  class ObliqueControl : public Control
  {
  public:
    static Control * creator();
    ObliqueControl( const std::string & name = "ObliqueControl" );
    ObliqueControl( const ObliqueControl & c );
    virtual ~ObliqueControl();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
  };


  class TransformControl : public Control
  {
  public:
    static Control * creator();
    TransformControl();
    TransformControl( const TransformControl & c );
    virtual ~TransformControl();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
    virtual void doAlsoOnSelect( ActionPool * actionPool );
    virtual void doAlsoOnDeselect( ActionPool * actionPool );
  };


  class CutControl : public Control
  {
  public:
    static Control * creator();
    CutControl();
    CutControl( const CutControl & c );
    virtual ~CutControl();

    virtual std::string description() const;

    virtual void eventAutoSubscription( ActionPool * actionPool );
  };


  class WindowActions : public Action
  {
  public:
    static Action * creator() ;
    WindowActions();
    WindowActions( const WindowActions & a );
    virtual ~WindowActions();

    virtual std::string name() const;

    void close();
    void toggleShowTools();
    void toggleFullScreen();
    void focusView();
    void focusAxialView();
    void focusCoronalView();
    void focusSagittelView();

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;
  };


  class LinkAction : public Action
  {
  public:
    static Action * creator() ;
    LinkAction();
    LinkAction( const LinkAction & a );
    virtual ~LinkAction();

    virtual std::string name() const;

    void execLink( int x, int y, int globalX, int globalY );
    void endLink( int x, int y, int globalX, int globalY );

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;

  protected:
  private:
  };


  class MenuAction : public Action
  {
  public:
    static Action * creator() ;

    MenuAction();
    MenuAction( const MenuAction & a );
    virtual ~MenuAction();

    virtual std::string name() const;

    void execMenu( int x, int y, int globalX, int globalY );

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;
  };


  class SelectAction : public Action
  {
  public:
    static Action * creator() ;

    SelectAction();
    SelectAction( const SelectAction & a );
    virtual ~SelectAction();

    virtual std::string name() const;

    void select( int modifier, int x, int y );
    void execSelect( int x, int y, int globalX, int globalY );
    void execSelectAdding( int x, int y, int globalX, int globalY );
    void execSelectToggling( int x, int y, int globalX, int globalY );

    /// key action for select all / unselect all objects
    void toggleSelectAll();
    void removeFromWindow();
    void removeFromGroup();

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;
  };


  class Zoom3DAction : public Action
  {
  public:
    static Action * creator() ;

    Zoom3DAction();
    Zoom3DAction( const Zoom3DAction & a );
    virtual ~Zoom3DAction();

    virtual std::string name() const;

    void beginZoom( int x, int y, int globalX, int globalY );
    void moveZoom( int x, int y, int globalX, int globalY );
    void endZoom( int x, int y, int globalX, int globalY );
    void endZoomKey();
    /// zomms a fixed amount (ie 10%) on a key event
    void zoomInOnce();
    /// zomms a fixed amount (ie 10%) on a key event
    void zoomOutOnce();
    /// zoom for a wheel event
    void zoomWheel( int, int, int, int, int );
    void zoom( int distance );

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;

  private:
    int		_beginpos;
    float	_orgzoom;
  };


  class Translate3DAction : public Action
  {
  public:
    static Action * creator() ;

    Translate3DAction();
    Translate3DAction( const Translate3DAction & a );
    virtual ~Translate3DAction();

    virtual std::string name() const;

    void beginTranslate( int x, int y, int globalX, int globalY );
    void moveTranslate( int x, int y, int globalX, int globalY );
    void endTranslate( int x, int y, int globalX, int globalY );
    void endTranslateKey();

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;

  private:
    bool	_started;
    int		_beginx;
    int		_beginy;
  };


  class Sync3DAction : public Action
  {
  public:
    static Action * creator() ;

    Sync3DAction();
    Sync3DAction( const Sync3DAction & a );
    virtual ~Sync3DAction();

    virtual std::string name() const;

    void execSync();
    void execSyncOrientation();

    QWidget* actionView( QWidget* parent );
    bool viewableAction() const;
  };


  class MovieAction : public QObject, public Action
  {
    Q_OBJECT

  public:
    enum RunMode
    {
      Forward, 
      Backward, 
      LoopForward, 
      LoopBackward, 
      LoopBothWays, 
    };

    MovieAction() ;
    virtual ~MovieAction() ;

    virtual std::string name() const { return "MovieAction" ; } 

    static Action * creator() ;

    void sliceOn() ;
    void timeOn() ;
    void nextMode();
    void increaseSpeed();
    void decreaseSpeed();

    void startOrStop() ;
    bool isRunning() const { return myIsRunning; }

  private slots:
    void timeout() ;

  private:
    bool mySliceAndNotTime;
    bool myIsRunning;
    RunMode myRunMode;
    bool myForward;
    QTimer * myTimer;
    int myTimeInterval;
  } ;


  class SliceAction : public Action
  {
  public:
    SliceAction();
    virtual ~SliceAction();

    virtual std::string name() const { return "SliceAction" ; } 
    static Action * creator() ;

    void nextSlice();
    void previousSlice();
    void nextTime();
    void previousTime();
    void toggleLinkedOnSlider();
    void invertSlice();
  };


  class DragObjectAction : public Action
  {
  public:
    DragObjectAction();
    virtual ~DragObjectAction();

    virtual std::string name() const { return "DragObjectAction" ; } 
    static Action * creator() ;

    void dragAll( int x, int y, int globalX, int globalY );
    void dragSelected( int x, int y, int globalX, int globalY );
  };


  class SortMeshesPolygonsAction : public Action
  {
  public:
    SortMeshesPolygonsAction();
    virtual ~SortMeshesPolygonsAction();

    virtual std::string name() const { return "SortMeshesPolygonsAction"; }
    static Action * creator();

    void sort();
    void toggleAutoSort();
    void toggleSortDirection();
  };


  class ObjectStatAction : public Action
  {
  public:
    ObjectStatAction();
    virtual ~ObjectStatAction();

    virtual std::string name() const { return "ObjectStatAction"; }
    static Action * creator();

    void displayStat();
  };

}


#endif
