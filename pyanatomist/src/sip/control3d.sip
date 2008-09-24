
namespace anatomist
{

class Trackball : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/trackball.h>
%End

public:
  Trackball();
  virtual ~Trackball();

  virtual void beginTrackball( int x, int y, int globalX, int globalY );
  virtual void moveTrackball( int x, int y, int globalX, int globalY );
  virtual void endTrackball( int x, int y, int globalX, int globalY );
  void setCenter();
  void showRotationCenter();
};


class ContinuousTrackball : QObject, anatomist::Trackball
{
%TypeHeaderCode
#include <anatomist/window3D/trackball.h>
%End

  public:
    ContinuousTrackball();
    virtual ~ContinuousTrackball();

    virtual void beginTrackball( int x, int y, int globalX, int globalY );
    virtual void moveTrackball( int x, int y, int globalX, int globalY );
    void endTrackball( int x, int y, int globalX, int globalY );
    void startOrStop();
    void stop();
};


class WindowActions : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    WindowActions();
    virtual ~WindowActions();

    void close();
    void toggleShowTools();
    void toggleFullScreen();
};


class LinkAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    LinkAction();
    virtual ~LinkAction();

    void execLink( int x, int y, int globalX, int globalY );
    void endLink( int x, int y, int globalX, int globalY );
};


class MenuAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    MenuAction();
    virtual ~MenuAction();

    void execMenu( int x, int y, int globalX, int globalY );
};


class SelectAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    SelectAction();
    virtual ~SelectAction();

    void select( int modifier, int x, int y );
    void execSelect( int x, int y, int globalX, int globalY );
    void execSelectAdding( int x, int y, int globalX, int globalY );
    void execSelectToggling( int x, int y, int globalX, int globalY );

    void toggleSelectAll();
    void removeFromWindow();
    void removeFromGroup();
};


class Zoom3DAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    Zoom3DAction();
    virtual ~Zoom3DAction();

    void beginZoom( int x, int y, int globalX, int globalY );
    void moveZoom( int x, int y, int globalX, int globalY );
    void endZoom( int x, int y, int globalX, int globalY );
    void endZoomKey();
    void zoomInOnce();
    void zoomOutOnce();
    void zoomWheel( int, int, int, int, int );
    void zoom( int distance );
};


class Translate3DAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    Translate3DAction();
    virtual ~Translate3DAction();

    void beginTranslate( int x, int y, int globalX, int globalY );
    void moveTranslate( int x, int y, int globalX, int globalY );
    void endTranslate( int x, int y, int globalX, int globalY );
    void endTranslateKey();
};


class Sync3DAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    Sync3DAction();
    virtual ~Sync3DAction();

    void execSync();
    void execSyncOrientation();
};


class MovieAction : QObject, anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    MovieAction() ;
    virtual ~MovieAction() ;

    void sliceOn() ;
    void timeOn() ;
    void nextMode();
    void increaseSpeed();
    void decreaseSpeed();

    void startOrStop() ;
    bool isRunning() const;
};


class SliceAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    SliceAction();
    virtual ~SliceAction();

    void nextSlice();
    void previousSlice();
    void nextTime();
    void previousTime();
    void toggleLinkedOnSlider();
};


class DragObjectAction : anatomist::Action
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    DragObjectAction();
    virtual ~DragObjectAction();

    void dragAll( int x, int y, int globalX, int globalY );
    void dragSelected( int x, int y, int globalX, int globalY );
};


class Select3DControl : anatomist::Control
{
%TypeHeaderCode
#include <anatomist/window3D/control3D.h>
%End

  public:
    Select3DControl( const std::string & name = "Selection 3D" );
    Select3DControl( const anatomist::Select3DControl & c );
    virtual ~Select3DControl();

    virtual void eventAutoSubscription( anatomist::ActionPool * actionPool );
    virtual void doAlsoOnSelect( anatomist::ActionPool * actionPool );
    virtual void doAlsoOnDeselect( anatomist::ActionPool * actionPool );
};


};

