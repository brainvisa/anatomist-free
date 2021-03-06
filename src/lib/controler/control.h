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


#ifndef ANATOMIST_CONTROLER_CONTROL_H
#define ANATOMIST_CONTROLER_CONTROL_H

#include "anatomist/controler/action.h"
#include <qevent.h>
#include <qobject.h>
#include <qtabwidget.h>
#include <qwidget.h>
class QGestureEvent;
class QPinchGesture;
class QPanGesture;
class QSwipeGesture;
class QTapGesture;
class QTapAndHoldGesture;
#include <list>
#include <map>
#include <set>
#include <string>


class QTimer;
class KeyRepetitiveEvent;

namespace anatomist {
  class Control;
  // TO be changed
  //typedef rc_ptr<Control> ControlPtr;
  typedef Control * ControlPtr;

//   class ControlObserver : public QTabWidget
//   {

//   public:

//     ControlObserver( QWidget * parent, Control * subject );

//     //void updateControlLinks();

//     void closeEvent( QCloseEvent * );

//   private:
//     Control * mySubject;
//   };


  class Action;
  typedef Action * ActionPtr;

  class ActionPool;
  class KeyAndMouseLongEvent;
  class LongActions;

  class Control
  {
  public:

    static ControlPtr creator( int priority, const std::string& name );

    // Les classes derivees de control doivent redefinir et appeler la methode eventAutoSubscription dans le constructeur;
    Control( int priority, std::string name );
    Control( const Control& control );

    virtual ~Control();

    // virtual ControlPtr  clone( ) const = 0;

    void doOnSelect( ActionPool * pool );
    void doOnDeselect( ActionPool * pool );

    // Can be redefined by inherited classes
    virtual void doAlsoOnDeselect ( ActionPool *) {}
    virtual void doAlsoOnSelect( ActionPool *) {}

    const std::string& name() const { return myName; }
    virtual std::string description() const;

    //   QPushButton * button( ) {
    //     return myButton;
    //   }

    //   bool animateClick() {
    // #ifdef ANADEBUG
    //     cerr << "Toto" << endl;
    //     cerr << "Is Button Defined" << (void *)this << endl;
    // #endif
    //     if( myButton->isEnabled() )
    //       myButton->animateClick();
    //     else return false;
    //     return true;
    //   }

    //   void enable() { myButton->setEnabled(true);}
    //   void disable() { myButton->setEnabled(false);}

    virtual void eventAutoSubscription( anatomist::ActionPool * actionPool );
    int userLevel() const { return myUserLevel; }
    void setUserLevel( int x ) { myUserLevel = x; }

    typedef Qt::DropAction DropAction;

    struct KeyMapKey
    {
      KeyMapKey( int k, int s ) : key(k), state(s) {}
      int key;
      int state;
      bool operator==( const KeyMapKey& k ) const {
          if ( k.key != key || k.state != state )
            return false;
          return true;
      }
      bool operator != ( const KeyMapKey& k ) const
        { return( !operator == ( k ) ); }
    };

    struct LessKeyMap
    {
      bool operator()( KeyMapKey entry1, KeyMapKey entry2 ) const {
          if ( entry1.key == entry2.key )
            return entry1.state < entry2.state;
          return entry1.key < entry2.key;
      }
      int i;
    };

    struct MouseButtonMapKey
    {
      MouseButtonMapKey( int b, int s ) : button(b), state(s) {}
      int button;
        int state;
      bool operator==( const MouseButtonMapKey& k ) const {
        if ( k.button != button || k.state != state )
          return false;
        return true;
      }
    };

    struct LessMouseMap
    {
      bool operator()( MouseButtonMapKey entry1, MouseButtonMapKey entry2 ) const 
      {
        if ( entry1.button == entry2.button )
          return entry1.state < entry2.state;
        return entry1.button < entry2.button;
      }
        int i;
    };

  class KeyActionLink
  {
  public:
    virtual ~KeyActionLink( ) = 0;
    virtual void execute( ) = 0;
    virtual KeyActionLink* clone() const = 0;
    };

  template<typename T>
  class KeyActionLinkOf : public KeyActionLink
  {
  public:
    typedef void (T:: * Callback)( );
    KeyActionLinkOf();
    KeyActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~KeyActionLinkOf() {}

    virtual void execute();
    virtual KeyActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class MouseActionLink
  {
  public:
    virtual ~MouseActionLink( ) = 0;
    virtual void execute( int x, int y, int globalX, int globalY ) = 0;
    virtual Action* action() = 0;
    virtual MouseActionLink* clone() const = 0;
    };

  template<typename T>
  class MouseActionLinkOf : public MouseActionLink
  {
  public:
    typedef void (T:: * Callback)( int x, int y, int globalX, int globalY );
    MouseActionLinkOf();
    MouseActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~MouseActionLinkOf() {}

    virtual void execute( int x, int y, int globalX, int globalY );

    virtual MouseActionLink* clone() const;
    virtual Action* action();

  private:
    T * actionInstance;
    Callback actionCallback;
  };


  class WheelActionLink
  {
  public:
    virtual ~WheelActionLink( ) = 0;
    virtual void execute( int delta, int x, int y, int globalX, int globalY ) = 0;
    virtual WheelActionLink* clone() const = 0;
  };

  template<typename T>
  class WheelActionLinkOf : public WheelActionLink
  {
  public:
    typedef void (T:: * Callback)( int delta, int x, int y, int globalX, int globalY );
    WheelActionLinkOf();
    WheelActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~WheelActionLinkOf() {}

    virtual void execute( int delta, int x, int y, int globalX, int globalY );

    virtual WheelActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class FocusActionLink
  {
  public:
    virtual ~FocusActionLink( ) = 0;
    virtual void execute( ) = 0;
    virtual FocusActionLink* clone() const = 0;
  };

  template<typename T>
  class FocusActionLinkOf : public FocusActionLink
  {
  public:
    typedef void (T:: * Callback)( );
    FocusActionLinkOf();
    FocusActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~FocusActionLinkOf() {}

    virtual void execute();
    virtual FocusActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class EnterLeaveActionLink
  {
  public:
    virtual ~EnterLeaveActionLink( ) = 0;
    virtual void execute( ) = 0;
    virtual EnterLeaveActionLink* clone() const = 0;
  };

  template<typename T>
  class EnterLeaveActionLinkOf : public EnterLeaveActionLink
  {
  public:
    typedef void (T:: * Callback)( );
    EnterLeaveActionLinkOf();
    EnterLeaveActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~EnterLeaveActionLinkOf() {}

    virtual void execute();
    virtual EnterLeaveActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class PaintActionLink
  {
  public:
    virtual ~PaintActionLink( ) = 0;
    virtual void execute( int xOffset, int yOffset, int height, int width ) = 0;
    virtual PaintActionLink* clone() const = 0;
  };

  template<typename T>
  class PaintActionLinkOf : public PaintActionLink
  {
    public:
    typedef void (T:: * Callback)( int xOffset, int yOffset, int height, int width );
    PaintActionLinkOf();
    PaintActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~PaintActionLinkOf();

    virtual void execute( int xOffset, int yOffset, int height, int width );
    virtual PaintActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class MoveOrDragActionLink
  {
  public:
    virtual ~MoveOrDragActionLink( ) = 0;
    virtual void execute( int posX = 0, int posY = 0, int oldPosX = 0, int oldPosY = 0 ) = 0;
    virtual MoveOrDragActionLink* clone() const = 0;
  };

  template<typename T>
  class MoveOrDragActionLinkOf : public MoveOrDragActionLink
  {
  public:
    typedef void (T:: * Callback)( int, int, int, int );
    MoveOrDragActionLinkOf();
    MoveOrDragActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~MoveOrDragActionLinkOf() {}

    virtual void execute( int posX = 0, int posY = 0, int oldPosX = 0, int oldPosY = 0 );
    virtual MoveOrDragActionLink* clone() const;

  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class DropActionLink
  {
  public:
    virtual ~DropActionLink( ) = 0;
    virtual void execute( int posX, int posY, Control::DropAction action ) = 0;
    virtual DropActionLink* clone() const = 0;
  };

  template<typename T>
  class DropActionLinkOf : public DropActionLink
  {
  public:
    typedef void (T:: * Callback)( int posX, int posY, Control::DropAction action );
    DropActionLinkOf();
    DropActionLinkOf( anatomist::Action * action, Callback actionCb);
    virtual ~DropActionLinkOf();

    virtual void execute( int posX, int posY, Control::DropAction action );
    virtual DropActionLink* clone() const;
  private:
    T * actionInstance;
    Callback actionCallback;
  };

  class ResizeActionLink
  {
  public:
    virtual ~ResizeActionLink( ) = 0;
    virtual void execute( int height, int width, int oldHeight, int oldWidth ) = 0;
    virtual ResizeActionLink* clone() const = 0;
  };

  template<typename T>
    class ResizeActionLinkOf : public ResizeActionLink
    {
    public:
      typedef void (T:: * Callback)( int height, int width, int oldHeight, 
                                     int oldWidth );
      ResizeActionLinkOf();
      ResizeActionLinkOf( anatomist::Action * action, Callback actionCb);
      virtual ~ResizeActionLinkOf() {}

      virtual void execute( int height, int width, int oldHeight, 
                            int oldWidth );
      virtual ResizeActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class ShowHideActionLink
    {
    public:
      virtual ~ShowHideActionLink( ) = 0;
      virtual void execute( bool spontaneous ) = 0;
      virtual ShowHideActionLink* clone() const = 0;
    };

    template<typename T>
    class ShowHideActionLinkOf : public ShowHideActionLink
    {
    public:
      typedef void (T:: * Callback)( bool spontaneous );
      ShowHideActionLinkOf();
      ShowHideActionLinkOf( anatomist::Action * action, Callback actionCb);
      virtual ~ShowHideActionLinkOf() {}

      virtual void execute( bool spontaneous );
      virtual ShowHideActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class SelectionChangedActionLink
    {
    public:
      virtual ~SelectionChangedActionLink( ) = 0;
      virtual void execute() = 0;
      virtual SelectionChangedActionLink* clone() const = 0;
    };

    template<typename T>
    class SelectionChangedActionLinkOf : public SelectionChangedActionLink
    {
    public:
      typedef void (T:: * Callback)( bool spontaneous );
      SelectionChangedActionLinkOf();
      SelectionChangedActionLinkOf( anatomist::Action * action,
                                    Callback actionCb );
      virtual ~SelectionChangedActionLinkOf() {}

      virtual void execute();
      virtual SelectionChangedActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class PinchActionLink
    {
    public:
      virtual ~PinchActionLink() = 0;
      virtual void execute( QPinchGesture* ) = 0;
      virtual PinchActionLink* clone() const = 0;
    };

    template <typename T>
    class PinchActionLinkOf : public PinchActionLink
    {
    public:
      typedef void (T:: * Callback)( QPinchGesture* );
      PinchActionLinkOf();
      PinchActionLinkOf( anatomist::Action* action,
                         Callback actioncb );
      virtual ~PinchActionLinkOf() {}

      virtual void execute( QPinchGesture * );
      virtual PinchActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class PanActionLink
    {
    public:
      virtual ~PanActionLink() = 0;
      virtual void execute( QPanGesture* ) = 0;
      virtual PanActionLink* clone() const = 0;
    };

    template <typename T>
    class PanActionLinkOf : public PanActionLink
    {
    public:
      typedef void (T:: * Callback)( QPanGesture* );
      PanActionLinkOf();
      PanActionLinkOf( anatomist::Action* action,
                       Callback actioncb );
      virtual ~PanActionLinkOf() {}

      virtual void execute( QPanGesture * );
      virtual PanActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class TapActionLink
    {
    public:
      virtual ~TapActionLink() = 0;
      virtual void execute( QTapGesture* ) = 0;
      virtual TapActionLink* clone() const = 0;
    };

    template <typename T>
    class TapActionLinkOf : public TapActionLink
    {
    public:
      typedef void (T:: * Callback)( QTapGesture* );
      TapActionLinkOf();
      TapActionLinkOf( anatomist::Action* action,
                       Callback actioncb );
      virtual ~TapActionLinkOf() {}

      virtual void execute( QTapGesture * );
      virtual TapActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class TapAndHoldActionLink
    {
    public:
      virtual ~TapAndHoldActionLink() = 0;
      virtual void execute( QTapAndHoldGesture* ) = 0;
      virtual TapAndHoldActionLink* clone() const = 0;
    };

    template <typename T>
    class TapAndHoldActionLinkOf : public TapAndHoldActionLink
    {
    public:
      typedef void (T:: * Callback)( QTapAndHoldGesture* );
      TapAndHoldActionLinkOf();
      TapAndHoldActionLinkOf( anatomist::Action* action,
                              Callback actioncb );
      virtual ~TapAndHoldActionLinkOf() {}

      virtual void execute( QTapAndHoldGesture * );
      virtual TapAndHoldActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    class SwipeActionLink
    {
    public:
      virtual ~SwipeActionLink() = 0;
      virtual void execute( QSwipeGesture* ) = 0;
      virtual SwipeActionLink* clone() const = 0;
    };

    template <typename T>
    class SwipeActionLinkOf : public SwipeActionLink
    {
    public:
      typedef void (T:: * Callback)( QSwipeGesture* );
      SwipeActionLinkOf();
      SwipeActionLinkOf( anatomist::Action* action,
                         Callback actioncb );
      virtual ~SwipeActionLinkOf() {}

      virtual void execute( QSwipeGesture * );
      virtual SwipeActionLink* clone() const;

    private:
      T * actionInstance;
      Callback actionCallback;
    };

    // ----

    int priority( ) { return myPriority; }
    std::map<std::string, anatomist::ActionPtr> actions( )
    { return myActions; }
    void setPriority( int priority ) { myPriority = priority; }

    virtual void keyPressEvent( QKeyEvent *);
    virtual void keyReleaseEvent( QKeyEvent *);
    virtual void mousePressEvent ( QMouseEvent * );
    virtual void mouseReleaseEvent ( QMouseEvent * );
    virtual void mouseDoubleClickEvent ( QMouseEvent * );
    virtual void mouseMoveEvent ( QMouseEvent * );
    virtual void wheelEvent ( QWheelEvent * );
    virtual void focusInEvent (  );
    virtual void focusOutEvent (  );
    virtual void enterEvent (  );
    virtual void leaveEvent (  );
    virtual void paintEvent ( QPaintEvent * );
    virtual void moveEvent ( QMoveEvent * );
    virtual void resizeEvent ( QResizeEvent * );
    virtual void dragEnterEvent ( );
    virtual void dragMoveEvent ( );
    virtual void dragLeaveEvent (  );
    virtual void dropEvent ( QDropEvent * );
    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );
    virtual void selectionChangedEvent();
    //virtual void customEvent ( QCustomEvent * );
#if QT_VERSION >= 0x040600
    virtual void gestureEvent( QGestureEvent * event );
    virtual bool pinchGesture( QPinchGesture * gesture );
    virtual bool panGesture( QPanGesture * gesture );
    virtual bool tapGesture( QTapGesture* gesture );
    virtual bool tapAndHoldGesture( QTapAndHoldGesture* gesture );
    virtual bool swipeGesture( QSwipeGesture* gesture );
#endif

    bool keyPressEventSubscribe( int key,
                                 Qt::KeyboardModifiers buttonState,
                                 const KeyActionLink& actionMethod,
                                 const std::string & name = "" );

    bool keyReleaseEventSubscribe( int key,
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethod,
                                   const std::string & name = "" );

    bool mousePressButtonEventSubscribe( Qt::MouseButtons button,
                                         Qt::KeyboardModifiers state,
                                         const MouseActionLink& actionMethod,
                                         const std::string & name = "" );

    bool mouseReleaseButtonEventSubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink& actionMethod,
                                           const std::string & name = "" );

    bool
    mouseDoubleClickEventSubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state,
                                    const MouseActionLink& actionMethod,
                                    const std::string & name = "" );

    bool mouseMoveEventSubscribe( Qt::MouseButtons button,
                                  Qt::KeyboardModifiers state,
                                  const MouseActionLink& actionMethod,
                                  const std::string & name = "" );

    bool keyAndMouseLongEventSubscribe(
      int startingKey, Qt::KeyboardModifiers startingButtonState,
      const KeyActionLink& startingActionMethod,
      Qt::MouseButtons longButton, Qt::KeyboardModifiers longState,
      const MouseActionLink& longActionMethod,
      int endingKey, Qt::KeyboardModifiers endingButtonState,
      const KeyActionLink& endingActionMethod,
      bool exclusiveAction );

    bool mouseLongEventSubscribe( Qt::MouseButtons startingButton,
                                  Qt::KeyboardModifiers startingButtonState,
                                  const MouseActionLink& startingActionMethod,
                                  const MouseActionLink& longActionMethod,
                                  const MouseActionLink& endingActionMethod,
                                  bool exclusiveAction );

    bool keyRepetitiveEventSubscribe( int startingKey,
                                      Qt::KeyboardModifiers startingButtonState,
                                      const KeyActionLink& startingActionMethod,

                                      int endingKey,
                                      Qt::KeyboardModifiers endingButtonState,
                                      const KeyActionLink& endingActionMethod,
                                      bool exclusiveAction,
                                      float temporalStep );

    bool wheelEventSubscribe( const WheelActionLink& actionMethod );
    bool wheelEventUnsubscribeAll( );
    bool focusInEventSubscribe( const FocusActionLink& actionMethod );
    bool focusOutEventSubscribe( const FocusActionLink& actionMethod );
    bool enterEventSubscribe( const EnterLeaveActionLink& actionMethod );
    bool leaveEventSubscribe( const EnterLeaveActionLink& actionMethod );
    bool paintEventSubscribe( const PaintActionLink& actionMethod );
    bool moveEventSubscribe( const MoveOrDragActionLink& actionMethod );
    bool resizeEventSubscribe( const ResizeActionLink& actionMethod );
    bool dragEnterEventSubscribe( const MoveOrDragActionLink& actionMethod );
    bool dragLeaveEventSubscribe( const MoveOrDragActionLink& actionMethod );
    bool dragMoveEventSubscribe( const MoveOrDragActionLink& actionMethod );
    bool dropEventSubscribe( const DropActionLink& actionMethod );
    bool showEventSubscribe( const ShowHideActionLink& actionMethod );
    bool hideEventSubscribe( const ShowHideActionLink& actionMethod );
    bool selectionChangedEventSubscribe
        ( const SelectionChangedActionLink& actionMethod );
    bool pinchEventSubscribe( const PinchActionLink & startMethod,
                              const PinchActionLink & moveMethod,
                              const PinchActionLink & stopMethod,
                              const PinchActionLink & cancelMethod );
    bool panEventSubscribe( const PanActionLink & startMethod,
                            const PanActionLink & moveMethod,
                            const PanActionLink & stopMethod,
                            const PanActionLink & cancelMethod );
    bool swipeEventSubscribe( const SwipeActionLink & startMethod,
                              const SwipeActionLink & moveMethod,
                              const SwipeActionLink & stopMethod,
                              const SwipeActionLink & cancelMethod );
    bool tapEventSubscribe( const TapActionLink & startMethod,
                            const TapActionLink & moveMethod,
                            const TapActionLink & stopMethod,
                            const TapActionLink & cancelMethod );
    bool tapAndHoldEventSubscribe( const TapAndHoldActionLink & startMethod,
                                   const TapAndHoldActionLink & moveMethod,
                                   const TapAndHoldActionLink & stopMethod,
                                   const TapAndHoldActionLink & cancelMethod );

    /// obsolete, use the other one
    bool keyPressEventUnsubscribe( int key,
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethods);
    bool keyPressEventUnsubscribe( int key,
                                   Qt::KeyboardModifiers buttonState);

    /// obsolete, use the other one
    bool keyReleaseEventUnsubscribe( int key,
                                     Qt::KeyboardModifiers buttonState,
                                     const KeyActionLink& actionMethods);
    bool keyReleaseEventUnsubscribe( int key,
                                     Qt::KeyboardModifiers buttonState);

    /// obsolete, use the other one
    bool mousePressButtonEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink& actionMethods );
    bool mousePressButtonEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state );


    /// obsolete, use the other one
    bool mouseReleaseButtonEventUnsubscribe( Qt::MouseButtons button,
                                             Qt::KeyboardModifiers state,
                                             const MouseActionLink& actionMethods );
    bool mouseReleaseButtonEventUnsubscribe( Qt::MouseButtons button,
                                             Qt::KeyboardModifiers state );

    /// obsolete, use the other one
    bool
    mouseDoubleClickEventUnsubscribe( Qt::MouseButtons button,
                                      Qt::KeyboardModifiers state,
                                      const MouseActionLink& actionMethods );
    bool
    mouseDoubleClickEventUnsubscribe( Qt::MouseButtons button,
                                      Qt::KeyboardModifiers state );

    /// obsolete, use the other one
    bool mouseMoveEventUnsubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state,
                                    const MouseActionLink& actionMethods );
    bool mouseMoveEventUnsubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state );

    bool keyAndMouseLongEventUnsubscribe( int startingKey,
                                          Qt::KeyboardModifiers startingButtonState,
                                          Qt::MouseButtons longButton,
                                          Qt::KeyboardModifiers longState,
                                          int endingKey,
                                          Qt::KeyboardModifiers endingButtonState );

    bool mouseLongEventUnsubscribe( Qt::MouseButtons startingButton,
                                    Qt::KeyboardModifiers startingButtonState );

    bool keyRepetitiveEventUnsubscribe( int startingKey,
                                        Qt::KeyboardModifiers startingButtonState,
                                        int endingKey,
                                        Qt::KeyboardModifiers endingButtonState );

    bool wheelEventUnsubscribe( const WheelActionLink& actionMethod );
    bool wheelEventUnsubscribe();
    bool focusInEventUnsubscribe( const FocusActionLink& actionMethod );
    bool focusInEventUnsubscribe();
    bool focusOutEventUnsubscribe( const FocusActionLink& actionMethod );
    bool focusOutEventUnsubscribe();
    bool enterEventUnsubscribe( const EnterLeaveActionLink& actionMethod );
    bool enterEventUnsubscribe();
    bool leaveEventUnsubscribe( const EnterLeaveActionLink& actionMethod );
    bool leaveEventUnsubscribe();
    bool paintEventUnsubscribe( const PaintActionLink& actionMethod );
    bool paintEventUnsubscribe();
    bool moveEventUnsubscribe( const MoveOrDragActionLink& actionMethod );
    bool moveEventUnsubscribe();
    bool resizeEventUnsubscribe( const ResizeActionLink& actionMethod );
    bool resizeEventUnsubscribe();
    bool dragEnterEventUnsubscribe( const MoveOrDragActionLink& actionMethod );
    bool dragEnterEventUnsubscribe();
    bool dragLeaveEventUnsubscribe( const MoveOrDragActionLink& actionMethod );
    bool dragLeaveEventUnsubscribe();
    bool dragMoveEventUnsubscribe( const MoveOrDragActionLink& actionMethod );
    bool dragMoveEventUnsubscribe();
    bool dropEventUnsubscribe( const DropActionLink& actionMethod );
    bool dropEventUnsubscribe();
    bool showEventUnsubscribe( const ShowHideActionLink& actionMethod );
    bool showEventUnsubscribe();
    bool hideEventUnsubscribe( const ShowHideActionLink& actionMethod );
    bool hideEventUnsubscribe();
    bool selectionChangedEventUnsubscribe
        ( const SelectionChangedActionLink& actionMethod );
    bool selectionChangedEventUnsubscribe();
    bool pinchEventUnsubscribe();
    bool panEventUnsubscribe();
    bool swipeEventUnsubscribe();
    bool tapEventUnsubscribe();
    bool tapAndHoldEventUnsubscribe();

    //   static bool controlFusion( const Control& control1, const Control& control2,
    //                           Control& controlsFusion );

    KeyActionLink* keyPressActionLinkByName( const std::string & name ) const;
    KeyActionLink* keyReleaseActionLinkByName(
      const std::string & name ) const;
    MouseActionLink* mousePressActionLinkByName(
      const std::string & name ) const;
    MouseActionLink* mouseReleaseActionLinkByName(
      const std::string & name ) const;
    MouseActionLink* mouseDoubleClickActionLinkByName(
      const std::string & name ) const;
    MouseActionLink* mouseMoveActionLinkByName(
      const std::string & name ) const;
    WheelActionLink* wheelActionLink() const;
    FocusActionLink* focusInActionLink() const;
    FocusActionLink* focusOutActionLink() const;

    std::set<std::string> keyPressActionLinkNames() const;
    std::set<std::string> keyReleaseActionLinkNames() const;
    std::set<std::string> mousePressActionLinkNames() const;
    std::set<std::string> mouseReleaseActionLinkNames() const;
    std::set<std::string> mouseDoubleClickActionLinkNames() const;
    std::set<std::string> mouseMoveActionLinkNames() const;

  protected :
    std::map<std::string, anatomist::ActionPtr> myActions;
    std::list<std::string> myControlLinksDescription;
  private:
    struct Private;

    int myPriority;
    int myUserLevel;
    std::string myName;

    //QPushButton * myButton;

    std::map<KeyMapKey, KeyActionLink*, LessKeyMap> myKeyPressActionMap;
    std::map<KeyMapKey, KeyActionLink*, LessKeyMap> myKeyReleaseActionMap;
    std::map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>
      myMousePressButtonActionMap;
    std::map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>
      myMouseReleaseButtonActionMap;
    std::map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>
      myMouseDoubleClickButtonActionMap;
    std::map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>
      myMouseMoveActionMap;

    WheelActionLink *myWheelAction;
    FocusActionLink *myFocusInAction;
    FocusActionLink *myFocusOutAction;
    EnterLeaveActionLink *myEnterAction;
    EnterLeaveActionLink *myLeaveAction;
    PaintActionLink *myPaintAction;
    MoveOrDragActionLink *myMoveAction;
    ResizeActionLink *myResizeAction;
    MoveOrDragActionLink *myDragEnterAction;
    MoveOrDragActionLink *myDragLeaveAction;
    MoveOrDragActionLink *myDragMoveAction;
    DropActionLink *myDropAction;
    ShowHideActionLink *myShowAction;
    ShowHideActionLink *myHideAction;
    SelectionChangedActionLink *mySelectionChangedAction;
    anatomist::LongActions * myLongActions;

    std::map<std::string, KeyActionLink*> _keyPressActionsByName;
    std::map<std::string, KeyActionLink*> _keyReleaseActionsByName;
    std::map<std::string, MouseActionLink*> _mousePressActionsByName;
    std::map<std::string, MouseActionLink*> _mouseReleaseActionsByName;
    std::map<std::string, MouseActionLink*> _mouseDoubleClickActionsByName;
    std::map<std::string, MouseActionLink*> _mouseMoveActionsByName;
    Private *d;
  };


  class KeyAndMouseLongEvent
  {
  public:
    KeyAndMouseLongEvent( const Control::KeyMapKey& startingEvent,
                          const Control::KeyActionLink& startingAction,
                          const Control::MouseButtonMapKey& longEvent,
                          const Control::MouseActionLink& longAction,
                          const Control::KeyMapKey& endingEvent,
                          const Control::KeyActionLink& endingAction,
                          bool exclusiveAction );
    KeyAndMouseLongEvent( const KeyAndMouseLongEvent& );

    ~KeyAndMouseLongEvent();
    const Control::KeyMapKey& startingEvent( ) const { return myStartingEvent; }
    const Control::KeyActionLink * startingAction( ) const { return myStartingAction; }
    const Control::MouseButtonMapKey& longEvent( ) const { return myLongEvent; }
    const Control::MouseActionLink * longAction( ) const { return myLongAction; }
    const Control::KeyMapKey& endingEvent( ) const { return myEndingEvent; }
    const Control::KeyActionLink * endingAction( ) const { return myEndingAction; }
    bool exclusiveAction() const { return myExclusiveAction; }

    void executeStart();
    void executeLong( int x, int y, int globalX, int globalY );
    void executeEnd();

  private:
    Control::KeyMapKey myStartingEvent;
    Control::KeyActionLink * myStartingAction;
    Control::MouseButtonMapKey myLongEvent;
    Control::MouseActionLink * myLongAction;
    Control::KeyMapKey myEndingEvent;
    Control::KeyActionLink * myEndingAction;
    bool myExclusiveAction;
  };


  class MouseLongEvent
  {
  public:
    MouseLongEvent( const Control::MouseButtonMapKey& startingEvent,
                    const Control::MouseActionLink& startingAction,
                    const Control::MouseButtonMapKey& longEvent,
                    const Control::MouseActionLink& longAction,
                    const Control::MouseButtonMapKey& endingEvent,
                    const Control::MouseActionLink& endingAction,
                    bool exclusiveAction );

    MouseLongEvent( const MouseLongEvent&  event );

    ~MouseLongEvent();

    const Control::MouseButtonMapKey& startingEvent( ) const
    { return myStartingEvent; }
    const Control::MouseActionLink * startingAction( ) const
    { return myStartingAction; }
    const Control::MouseButtonMapKey& longEvent( ) const
    { return myLongEvent; }
    const Control::MouseActionLink * longAction( ) const
    { return myLongAction; }
    const Control::MouseButtonMapKey& endingEvent( ) const
    { return myEndingEvent; }
    const Control::MouseActionLink * endingAction( ) const
    { return myEndingAction; }
    bool exclusiveAction() const { return myExclusiveAction; }

    void executeStart( int x, int y, int globalX, int globalY );
    void executeLong( int x, int y, int globalX, int globalY );
    void executeEnd( int x, int y, int globalX, int globalY );
    void setMouseTracking( bool );

  private:
    Control::MouseButtonMapKey myStartingEvent;
    Control::MouseActionLink * myStartingAction;
    Control::MouseButtonMapKey myLongEvent;
    Control::MouseActionLink * myLongAction;
    Control::MouseButtonMapKey myEndingEvent;
    Control::MouseActionLink * myEndingAction;
    bool myExclusiveAction;
  };


  class LongActions 
  {
  public:
    LongActions();
    ~LongActions();

    void reset();

    bool submitKeyPressEvent( QKeyEvent * event );
    bool submitKeyReleaseEvent( QKeyEvent * event );
    bool submitMousePressEvent( QMouseEvent * event );
    bool submitMouseReleaseEvent( QMouseEvent * event );
    bool submitMouseMoveEvent( QMouseEvent * event );

    bool keyAndMouseLongEventSubscribe(
      int startingKey,
      Qt::KeyboardModifiers startingButtonState,
      const Control::KeyActionLink& startingActionMethod,
      Qt::MouseButtons longButton,
      Qt::KeyboardModifiers longState,
      const Control::MouseActionLink& longActionMethod,
      int endingKey,
      Qt::KeyboardModifiers endingButtonState,
      const Control::KeyActionLink& endingActionMethod,
      bool exclusiveAction );

    bool mouseLongEventSubscribe(
      Qt::MouseButtons startingButton,
      Qt::KeyboardModifiers startingButtonState,
      const Control::MouseActionLink& startingActionMethod,
      const Control::MouseActionLink& longActionMethod,
      const Control::MouseActionLink& endingActionMethod,
      bool exclusiveAction );

    bool keyRepetitiveEventSubscribe(
      int startingKey,
      Qt::KeyboardModifiers startingButtonState,
      const Control::KeyActionLink& startingActionMethod,
      int endingKey,
      Qt::KeyboardModifiers endingButtonState,
      const Control::KeyActionLink& endingActionMethod,
      bool exclusiveAction,
      float temporalStep );

    bool keyAndMouseLongEventUnsubscribe(
      int startingKey,
      Qt::KeyboardModifiers startingButtonState,
      Qt::MouseButtons longButton,
      Qt::KeyboardModifiers longState,
      int endingKey,
      Qt::KeyboardModifiers endingButtonState );

    bool mouseLongEventUnsubscribe( 
      Qt::MouseButtons startingButton,
      Qt::KeyboardModifiers startingButtonState );

    bool keyRepetitiveEventUnsubscribe(
      int startingKey,
      Qt::KeyboardModifiers startingButtonState,
      int endingKey,
      Qt::KeyboardModifiers endingButtonState );

    void setMouseTracking( bool );

  private:
    std::map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>
      myKeyAndMouseLongEventMap;
    std::map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>
      myKeyRepetitiveEventMap;
    std::map<Control::MouseButtonMapKey, MouseLongEvent*,
             Control::LessMouseMap> myMouseLongEventMap;
    std::map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>
      myActiveKeyRepetitiveEvents;
    KeyAndMouseLongEvent* myActiveKeyAndMouseLongEvent;
    MouseLongEvent* myActiveMouseLongEvent;

    int currentMouseX;
    int currentMouseY;
    int currentMouseGlobalX;
    int currentMouseGlobalY;

  };
}

class KeyRepetitiveEvent : public QObject
{
  Q_OBJECT

public:
  KeyRepetitiveEvent( const anatomist::Control::KeyMapKey& startingEvent,
                      const anatomist::Control::KeyActionLink& startingAction,
                      const anatomist::Control::KeyMapKey& endingEvent,
                      const anatomist::Control::KeyActionLink& endingAction,
                      bool exclusiveAction,
                      float temporalStep );
  ~KeyRepetitiveEvent();

  const anatomist::Control::KeyMapKey& startingEvent( ) const
    { return myStartingEvent; }
  const anatomist::Control::KeyActionLink * startingAction( ) const
    { return myStartingAction; }
  void executeStart( );
  void executeEnd( );
  const anatomist::Control::KeyMapKey& endingEvent( ) const
    { return myEndingEvent; }
  const anatomist::Control::KeyActionLink * endingAction( ) const
    { return myEndingAction; }
  bool exclusiveAction( ) const { return myExclusiveAction; }
  float temporalStep( ) const { return myTemporalStep; }

private slots:
  void timeout();

private:
  anatomist::Control::KeyMapKey myStartingEvent;
  anatomist::Control::KeyActionLink * myStartingAction;
  anatomist::Control::KeyMapKey myEndingEvent;
  anatomist::Control::KeyActionLink * myEndingAction;

  bool myExclusiveAction;
  float myTemporalStep;
  QTimer * myTimer;
};

//   class MouseRepetitiveEvent{
//   public:
//     MouseRepetitiveEvent( Control::MouseButtonMapKey startingEvent, Control::MouseActionLink * startingAction,
//                        Control::MouseButtonMapKey endingEvent, Control::MouseActionLink * endingAction,
//                        bool exclusiveAction );
//   private:
//     void timeOut();
//     Control::MouseButtonMapKey myStartingEvent;
//     Control::MouseActionLink * myStartingAction;
//     Control::MouseButtonMapKey myEndingEvent;
//     Control::MouseActionLink * myEndingAction;
//     bool myExclusiveActionnnn;
//     Qtimer * myTimer;
//   };

#endif
