
%#if SIP_VERSION == 0x040403%
%ModuleCode
// work around a bug in SIP 4.4.3
anatomist::Action* 
anatomist::ActionDictionary::ActionCreatorBase::operator () ()
{
  std::cout << "pure virtual method anatomist::ActionDictionary::ActionCreatorBase::operator () called\n";
  return 0;
}

anatomist::Control* 
anatomist::ControlDictionary::ControlCreatorBase::operator () ()
{
  return 0;
}
%End
%#endif%

namespace anatomist
{


class Action
{
%TypeHeaderCode
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/control3D.h>
%End

%ConvertToSubClassCode
  if( dynamic_cast<anatomist::Trackball *>( sipCpp ) )
  {
    if( dynamic_cast<anatomist::ContinuousTrackball *>( sipCpp ) )
      sipClass = sipClass_anatomist_ContinuousTrackball;
    else
      sipClass = sipClass_anatomist_Trackball;
  }
  else if( dynamic_cast<anatomist::WindowActions *>( sipCpp ) )
    sipClass = sipClass_anatomist_WindowActions;
  else if( dynamic_cast<anatomist::LinkAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_LinkAction;
  else if( dynamic_cast<anatomist::MenuAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_MenuAction;
  else if( dynamic_cast<anatomist::SelectAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_SelectAction;
  else if( dynamic_cast<anatomist::Zoom3DAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_Zoom3DAction;
  else if( dynamic_cast<anatomist::Translate3DAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_Translate3DAction;
  else if( dynamic_cast<anatomist::Sync3DAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_Sync3DAction;
  else if( dynamic_cast<anatomist::SliceAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_SliceAction;
  else if( dynamic_cast<anatomist::DragObjectAction *>( sipCpp ) )
    sipClass = sipClass_anatomist_DragObjectAction;
  else
    sipClass = 0;
%End

public:
  Action();
  virtual ~Action();
  virtual std::string name() const = 0;
  virtual bool viewableAction() const;
  virtual bool isSingleton();
  anatomist::View * view();
};


class ActionDictionary
{
%TypeHeaderCode
#include <anatomist/controler/actiondictionary.h>
%End

public:
  class ActionCreatorBase
  {
  public:
    virtual ~ActionCreatorBase();
%#if SIP_VERSION == 0x040403%
    virtual anatomist::Action* operator () () /Factory, AutoGen/;
%#else%
    virtual anatomist::Action* operator () () =0 /Factory/;
%#endif%

  protected:
    ActionCreatorBase();
  };

  ~ActionDictionary();
  static anatomist::ActionDictionary* instance();

  anatomist::Action* getActionInstance( std::string );
  void addAction( const std::string &, 
                  anatomist::ActionDictionary::ActionCreatorBase* /Transfer/ ) 
    /PyName=_addAction/;

private:
  ActionDictionary();
  ActionDictionary( const anatomist::ActionDictionary & );
};


class ActionPool
{
%TypeHeaderCode
#include <anatomist/controler/actionpool.h>
%End

public:
  ~ActionPool();
  anatomist::Action* action( std::string & );
  set_STRING actionSet() const;

private:
  ActionPool();
};


class Control
{
%TypeHeaderCode
#include <anatomist/controler/control.h>
#include <anatomist/controler/action.h>
%End

public:
  class KeyActionLink
  {
  public:
    virtual ~KeyActionLink();
    virtual void execute() = 0;
    virtual anatomist::Control::KeyActionLink* clone() const = 0 /Factory/;
  };

  class MouseActionLink
  {
  public:
    virtual ~MouseActionLink();
    virtual void execute( int, int, int, int ) = 0;
    virtual anatomist::Control::MouseActionLink* clone() const = 0 /Factory/;
  };

  class WheelActionLink {
  public:
    virtual ~WheelActionLink() ;
    virtual void execute( int, int, int, int, int ) = 0 ;
    virtual anatomist::Control::WheelActionLink* clone() const = 0 /Factory/;
  };

  class SelectionChangedActionLink
  {
  public:
    virtual ~SelectionChangedActionLink();
    virtual void execute() = 0 ;
    virtual anatomist::Control::SelectionChangedActionLink*
      clone() const = 0 /Factory/;
  };

  Control( int, std::string );
  virtual ~Control();

  std::string name() const;
  int priority();
  int userLevel() const;
  void setUserLevel( int );
  void setPriority( int );
  virtual void doAlsoOnSelect( anatomist::ActionPool * );
  virtual void doAlsoOnDeselect( anatomist::ActionPool * );

  virtual void eventAutoSubscription( anatomist::ActionPool * );

%#if QT_VERSION >= 0x040000%

  bool keyPressEventSubscribe( int, Qt::KeyboardModifiers, 
                               const anatomist::Control::KeyActionLink & )
    /PyName=_keyPressEventSubscribe/;
  bool mousePressButtonEventSubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & ) 
    /PyName=_mousePressButtonEventSubscribe/;
  bool mouseReleaseButtonEventSubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseReleaseButtonEventSubscribe/;
  bool mouseDoubleClickEventSubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseDoubleClickEventSubscribe/;
  bool mouseMoveEventSubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseMoveEventSubscribe/;
  bool mouseLongEventSubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink &, 
      const anatomist::Control::MouseActionLink &, 
      const anatomist::Control::MouseActionLink &, 
      bool )
    /PyName=_mouseLongEventSubscribe/;

  bool keyPressEventUnsubscribe( int, Qt::KeyboardModifiers, 
                                 const anatomist::Control::KeyActionLink & )
    /PyName=_keyPressEventUnsubscribe/;
  bool mousePressButtonEventUnsubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & ) 
    /PyName=_mousePressButtonEventUnsubscribe/;
  bool mouseReleaseButtonEventUnsubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseReleaseButtonEventUnsubscribe/;
  bool mouseDoubleClickEventUnsubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseDoubleClickEventUnsubscribe/;
  bool mouseMoveEventUnsubscribe
    ( Qt::MouseButton, Qt::KeyboardModifiers,
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseMoveEventUnsubscribe/;
  bool mouseLongEventUnsubscribe( Qt::MouseButton, Qt::KeyboardModifiers )
    /PyName=_mouseLongEventUnsubscribe/;

%#else%

  bool keyPressEventSubscribe( int, Qt::ButtonState, 
                               const anatomist::Control::KeyActionLink & )
    /PyName=_keyPressEventSubscribe/;
  bool mousePressButtonEventSubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & ) 
    /PyName=_mousePressButtonEventSubscribe/;
  bool mouseReleaseButtonEventSubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseReleaseButtonEventSubscribe/;
  bool mouseDoubleClickEventSubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseDoubleClickEventSubscribe/;
  bool mouseMoveEventSubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseMoveEventSubscribe/;
  bool mouseLongEventSubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink &, 
      const anatomist::Control::MouseActionLink &, 
      const anatomist::Control::MouseActionLink &, 
      bool )
    /PyName=_mouseLongEventSubscribe/;

  bool keyPressEventUnsubscribe( int, Qt::ButtonState, 
                                 const anatomist::Control::KeyActionLink & )
    /PyName=_keyPressEventUnsubscribe/;
  bool mousePressButtonEventUnsubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & ) 
    /PyName=_mousePressButtonEventUnsubscribe/;
  bool mouseReleaseButtonEventUnsubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseReleaseButtonEventUnsubscribe/;
  bool mouseDoubleClickEventUnsubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseDoubleClickEventUnsubscribe/;
  bool mouseMoveEventUnsubscribe
    ( Qt::ButtonState, Qt::ButtonState, 
      const anatomist::Control::MouseActionLink & )
    /PyName=_mouseMoveEventUnsubscribe/;
  bool mouseLongEventUnsubscribe( Qt::ButtonState, Qt::ButtonState )
    /PyName=_mouseLongEventUnsubscribe/;
%#endif%

  bool wheelEventSubscribe( const WheelActionLink& )
    /PyName=_wheelEventSubscribe/;
  bool wheelEventUnsubscribe( const WheelActionLink& )
    /PyName=_wheelEventUnsubscribe/;
  bool wheelEventUnsubscribeAll( ) ;
  bool selectionChangedEventSubscribe( const SelectionChangedActionLink& )
    /PyName=_selectionChangedEventSubscribe/;
  bool selectionChangedEventUnsubscribe( const SelectionChangedActionLink& )
    /PyName=_selectionChangedEventUnsubscribe/;
};


class ControlDictionary
{
%TypeHeaderCode
#include <anatomist/controler/controldictionary.h>
%End

public:
  class ControlCreatorBase
  {
  public:
    virtual ~ControlCreatorBase();
%#if SIP_VERSION == 0x040403%
    virtual anatomist::Control* operator () () /Factory, AutoGen/;
%#else%
    virtual anatomist::Control* operator () () =0 /Factory/;
%#endif%

  protected:
    ControlCreatorBase();
  };

  ~ControlDictionary();
  static anatomist::ControlDictionary* instance();

  anatomist::Control* getControlInstance( const std::string & );
  void addControl( const std::string &, 
                   anatomist::ControlDictionary::ControlCreatorBase * 
        /Transfer/, 
                   int ) /PyName=_addControl/;
  int testPriorityUnicity( int );

private:
  ControlDictionary();
  ControlDictionary( const anatomist::ControlDictionary & );
};


class ControlManager
{
%TypeHeaderCode
#include <anatomist/controler/controlmanager.h>
%End

public:
  static anatomist::ControlManager* instance();

  void addControl( const std::string &, const std::string &, 
                   const std::string & );
  void addControl( const std::string &, const std::string &, 
                   const set_STRING & );
  bool removeControl( const std::string &, const std::string &,
                      const std::string & );
  bool removeControlList( const std::string &, const std::string & );
  void print() const /PyName=printSelf/;
  set_STRING
    availableControlList( const std::string &, 
                          const list_STRING & ) const;
  set_STRING
    availableControlList( const std::string &, const std::string & ) const;
  set_STRING
    activableControlList( const std::string &, 
                          const list_STRING & ) const;

private:
  ControlManager();
  ControlManager( const anatomist::ControlManager & );
};


class IconDictionary
{
%TypeHeaderCode
#include <anatomist/controler/icondictionary.h>
%End

public:
  static anatomist::IconDictionary* instance();

  const QPixmap* getIconInstance( std::string ) const;
  void addIcon( std::string, const QPixmap & );

private:
  IconDictionary();
  IconDictionary( const anatomist::IconDictionary & );
};

};

