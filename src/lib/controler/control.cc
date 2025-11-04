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


#include <cstdlib>
#include <cmath>
#include "anatomist/controler/control_d.h"
#include "anatomist/controler/actionpool.h"
#include "anatomist/controler/view.h"
#include <anatomist/window/glwidgetmanager.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPanGesture>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <math.h>
#include <qwidget.h>
#include <qapplication.h>
#include <iostream>
#include <typeinfo>

using namespace anatomist;
using namespace std;

using anatomist::Action;
using anatomist::ActionPool;
using anatomist::Control;
using anatomist::KeyAndMouseLongEvent;
using anatomist::MouseLongEvent;
using anatomist::LongActions;


namespace
{
  pair<int, QWidget *> & _mouseTracking()
  {
    static pair<int, QWidget *> _tracking( 0, 0 );
    return _tracking;
  }

  void _removeMouseTracking()
  {
    pair<int, QWidget *> & track = _mouseTracking();
    if( track.second )
    {
      QWidgetList wl = qApp->allWidgets();
      if( wl.contains( track.second ) )
        track.second->setMouseTracking( false );
      track.first = 0;
      track.second = 0;
    }
  }

  void _setMouseTracking( int n, QWidget * w )
  {
    _removeMouseTracking();
    if( n == 0 )
      return;
    pair<int, QWidget *> & track = _mouseTracking();
    track.first = n;
    track.second = w;
    w->setMouseTracking( true );
  }
}


// ControlObserver::ControlObserver( QWidget * parent, Control * subject )
//   : QTabWidget( parent, "ControlObserver", Qt::WDestructiveClose ),
//     mySubject(subject)
// {
//   setMargin( 5 );
//   show();
//   QLabel * controlDescription = new QLabel( tr( subject->name().c_str() ),
//                                             this, "ControlDescription" );
//   addTab(controlDescription, subject->name().c_str() );
// }

// void
// ControlObserver::closeEvent( QCloseEvent * e )
// {
//   mySubject->detachObserver();
//   e->accept();
// }


Control::KeyActionLink::~KeyActionLink( ){}

Control::MouseActionLink::~MouseActionLink( ){}

Control::WheelActionLink::~WheelActionLink( ){}

Control::FocusActionLink::~FocusActionLink( ){}

Control::EnterLeaveActionLink::~EnterLeaveActionLink( ){}

Control::PaintActionLink::~PaintActionLink( ){}

Control::MoveOrDragActionLink::~MoveOrDragActionLink( ){}

Control::DropActionLink::~DropActionLink( ){}

Control::ResizeActionLink::~ResizeActionLink( ){}

Control::ShowHideActionLink::~ShowHideActionLink( ){}

Control::SelectionChangedActionLink::~SelectionChangedActionLink( ){}

Control::PinchActionLink::~PinchActionLink() {}

Control::PanActionLink::~PanActionLink() {}

Control::SwipeActionLink::~SwipeActionLink() {}

Control::TapActionLink::~TapActionLink() {}

Control::TapAndHoldActionLink::~TapAndHoldActionLink() {}


KeyAndMouseLongEvent::KeyAndMouseLongEvent
( const Control::KeyMapKey& startingEvent,
  const Control::KeyActionLink& startingAction,
  const Control::MouseButtonMapKey& longEvent,
  const Control::MouseActionLink& longAction,
  const Control::KeyMapKey& endingEvent,
  const Control::KeyActionLink& endingAction,
  bool exclusiveAction ) :
  myStartingEvent( startingEvent ), myLongEvent( longEvent ),
  myEndingEvent(endingEvent),
  myExclusiveAction( exclusiveAction )
{
  myStartingAction = startingAction.clone();
  myLongAction = longAction.clone();
  myEndingAction = endingAction.clone();
}

KeyAndMouseLongEvent::KeyAndMouseLongEvent( const KeyAndMouseLongEvent&  event ) : myStartingEvent( event.startingEvent() ), myLongEvent( event.longEvent() ), myEndingEvent( event.endingEvent() ),
  myExclusiveAction( event.exclusiveAction() )
{
  myStartingAction = event.startingAction()->clone();
  myLongAction = event.longAction()->clone();
  myEndingAction = event.endingAction()->clone();
}


KeyAndMouseLongEvent::~KeyAndMouseLongEvent()
{
  delete myStartingAction;
  delete myLongAction;
  delete myEndingAction;
}


void
KeyAndMouseLongEvent::executeStart( )
{
  myStartingAction->execute( );
}

void
KeyAndMouseLongEvent::executeLong( int x, int y, int globalX, int globalY )
{
  myLongAction->execute( x, y, globalX, globalY );
}

void
KeyAndMouseLongEvent::executeEnd( )
{
  myEndingAction->execute( );
}

KeyRepetitiveEvent::KeyRepetitiveEvent(  const Control::KeyMapKey& startingEvent,
                                         const Control::KeyActionLink& startingAction,
                                         const Control::KeyMapKey& endingEvent,
                                         const Control::KeyActionLink& endingAction,
                                         bool exclusiveAction,
                                         float temporalStep ) :
  myStartingEvent(startingEvent), myEndingEvent(endingEvent),
  myExclusiveAction(exclusiveAction), myTemporalStep(temporalStep)
{
  if ( myTemporalStep <= 0. ){
    cerr << "Bad timer frequency, set to 100 milli seconds" << endl;
    myTemporalStep = 100;
  }

  myStartingAction = startingAction.clone( );
  myEndingAction = endingAction.clone( );
  myTimer = new QTimer( 0 );
  myTimer->setInterval( (int) temporalStep );
  myTimer->stop();        // don't start right now...

  QObject::connect( myTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
}

KeyRepetitiveEvent::~KeyRepetitiveEvent()
{
  delete myStartingAction;
  delete myEndingAction;
  delete myTimer;
}

void
KeyRepetitiveEvent::timeout()
{
  myStartingAction->execute();
  myTimer->setSingleShot( true );
  myTimer->start( (int) myTemporalStep );
}

void
KeyRepetitiveEvent::executeStart()
{
  myStartingAction->execute();
  myTimer->setSingleShot( true );
  myTimer->start( (int) myTemporalStep );
}

void
KeyRepetitiveEvent::executeEnd()
{
  myEndingAction->execute();
  myTimer->stop();
}

MouseLongEvent::MouseLongEvent
(  const Control::MouseButtonMapKey& startingEvent,
   const Control::MouseActionLink& startingAction,
   const Control::MouseButtonMapKey& longEvent,
   const Control::MouseActionLink& longAction,
   const Control::MouseButtonMapKey& endingEvent,
   const Control::MouseActionLink& endingAction,
   bool exclusiveAction ) :
  myStartingEvent( startingEvent ), myLongEvent( longEvent ),
  myEndingEvent(endingEvent),
  myExclusiveAction( exclusiveAction )
{
  myStartingAction = startingAction.clone();
  myLongAction = longAction.clone();
  myEndingAction = endingAction.clone();
}

MouseLongEvent::MouseLongEvent( const MouseLongEvent&  event ) : myStartingEvent( event.startingEvent() ), myLongEvent( event.longEvent() ), myEndingEvent( event.endingEvent() ),
  myExclusiveAction( event.exclusiveAction() )
{
  myStartingAction = event.startingAction()->clone();
  myLongAction = event.longAction()->clone();
  myEndingAction = event.endingAction()->clone();
}

MouseLongEvent::~MouseLongEvent()
{
  delete myStartingAction;
  delete myLongAction;
  delete myEndingAction;
}


void MouseLongEvent::setMouseTracking( bool t )
{
  QWidget *w = dynamic_cast< QWidget * >( myLongAction->action()->view() );
  if( w )
  {
    if( t )
      _setMouseTracking( 2, w );
    else
      _removeMouseTracking();
  }
}


void
MouseLongEvent::executeStart( int x, int y, int globalX, int globalY )
{
//   cout << "MouseLongEvent : ExecuteStart" << endl;
  myStartingAction->execute( x, y, globalX, globalY );
}

void
MouseLongEvent::executeLong( int x, int y, int globalX, int globalY )
{
//   cout << "MouseLongEvent : ExecuteLong" << endl;
  myLongAction->execute( x, y, globalX, globalY );
}

void
MouseLongEvent::executeEnd( int x, int y, int globalX, int globalY )
{
//   cout << "MouseLongEvent : ExecuteEnd" << endl;
  myEndingAction->execute ( x, y, globalX, globalY );
}

//------------------------------------------------------------

struct Control::Private
{
  Private()
    : myPinchStartAction(0), myPinchMoveAction(0), myPinchStopAction(0),
      myPinchCancelAction(0), myPanStartAction(0), myPanMoveAction(0),
      myPanStopAction(0), myPanCancelAction(0), mySwipeStartAction(0),
      mySwipeMoveAction(0), mySwipeStopAction(0), mySwipeCancelAction(0),
      myTapStartAction(0), myTapMoveAction(0), myTapStopAction(0),
      myTapCancelAction(0), myTapAndHoldStartAction(0),
      myTapAndHoldMoveAction(0), myTapAndHoldStopAction(0),
      myTapAndHoldCancelAction(0),
      doing_pan( false ), doing_pinch( false ), pinch_scale( 1. ) {}
  ~Private()
  {
    delete myPinchStartAction;
    delete myPinchMoveAction;
    delete myPinchStopAction;
    delete myPinchCancelAction;
    delete myPanStartAction;
    delete myPanMoveAction;
    delete myPanStopAction;
    delete myPanCancelAction;
    delete mySwipeStartAction;
    delete mySwipeMoveAction;
    delete mySwipeStopAction;
    delete mySwipeCancelAction;
    delete myTapStartAction;
    delete myTapMoveAction;
    delete myTapStopAction;
    delete myTapCancelAction;
    delete myTapAndHoldStartAction;
    delete myTapAndHoldMoveAction;
    delete myTapAndHoldStopAction;
    delete myTapAndHoldCancelAction;
  }

  Control::PinchActionLink* myPinchStartAction;
  Control::PinchActionLink* myPinchMoveAction;
  Control::PinchActionLink* myPinchStopAction;
  Control::PinchActionLink* myPinchCancelAction;
  Control::PanActionLink* myPanStartAction;
  Control::PanActionLink* myPanMoveAction;
  Control::PanActionLink* myPanStopAction;
  Control::PanActionLink* myPanCancelAction;
  Control::SwipeActionLink* mySwipeStartAction;
  Control::SwipeActionLink* mySwipeMoveAction;
  Control::SwipeActionLink* mySwipeStopAction;
  Control::SwipeActionLink* mySwipeCancelAction;
  Control::TapActionLink* myTapStartAction;
  Control::TapActionLink* myTapMoveAction;
  Control::TapActionLink* myTapStopAction;
  Control::TapActionLink* myTapCancelAction;
  Control::TapAndHoldActionLink* myTapAndHoldStartAction;
  Control::TapAndHoldActionLink* myTapAndHoldMoveAction;
  Control::TapAndHoldActionLink* myTapAndHoldStopAction;
  Control::TapAndHoldActionLink* myTapAndHoldCancelAction;

  bool doing_pan;
  bool doing_pinch;
  float pinch_scale;
  // some devices / drivers report a strange initial offset,
  // so we record it to substract to later ones
  QPoint last_pan_pos;
  QPoint last_pan_gpos;

  set<string> inhibitedActions;
};

//------------------------------------------------------------

ControlPtr
Control::creator( int priority, const string& name )
{
  return new Control( priority, name );
}

Control::Control( int priority, string name ) :
  myPriority(priority), myUserLevel( 0 ), myName(name), myWheelAction(0),
  myFocusInAction(0), myFocusOutAction(0), myEnterAction(0), myLeaveAction(0),
  myPaintAction(0), myMoveAction(0), myResizeAction(0), myDragEnterAction(0),
  myDragLeaveAction(0), myDragMoveAction(0), myDropAction(0), myShowAction(0),
  myHideAction(0), mySelectionChangedAction(0),
  d( new Private )
{
//   cout << "NEW CONTROL" << endl;
  myLongActions = new LongActions;

}

Control::Control( const Control& control ) :
  myPriority(control.myPriority), myUserLevel( 0 ), myName(control.myName),
  myWheelAction(0), myFocusInAction(0), myFocusOutAction(0), myEnterAction(0),
  myLeaveAction(0), myPaintAction(0), myMoveAction(0), myResizeAction(0),
  myDragEnterAction(0), myDragLeaveAction(0), myDragMoveAction(0),
  myDropAction(0), myShowAction(0), myHideAction(0),
  mySelectionChangedAction(0),
  d( new Private )
{
//   cout << "NEW CONTROL : copy" << endl;

  myLongActions = new LongActions;
}

Control::~Control()
{
  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator iter1(myKeyPressActionMap.begin()), last1(myKeyPressActionMap.end());
  while( iter1 != last1 ){
    delete iter1->second;
    ++iter1;
  }

  iter1 = myKeyReleaseActionMap.begin();
  last1 = myKeyReleaseActionMap.end();
  while( iter1 != last1 ){
    delete iter1->second;
    ++iter1;
  }

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator iter2(myMousePressButtonActionMap.begin()), last2(myMousePressButtonActionMap.end());

  while( iter2 != last2 ){
    delete iter2->second;
    ++iter2;
  }

  iter2 = myMouseReleaseButtonActionMap.begin();
  last2 = myMouseReleaseButtonActionMap.end();

  while( iter2 != last2 ){
    delete iter2->second;
    ++iter2;
  }

  iter2 = myMouseDoubleClickButtonActionMap.begin();
  last2 = myMouseDoubleClickButtonActionMap.end();

  while( iter2 != last2 ){
    delete iter2->second;
    ++iter2;
  }

  iter2 = myMouseMoveActionMap.begin();
  last2 = myMouseMoveActionMap.end();

  while( iter2 != last2 ){
    delete iter2->second;
    ++iter2;
  }


  delete myWheelAction;
  delete myFocusInAction;
  delete myFocusOutAction;
  delete myEnterAction;
  delete myLeaveAction;
  delete myPaintAction;
  delete myMoveAction;
  delete myResizeAction;
  delete myDragEnterAction;
  delete myDragLeaveAction;
  delete myDragMoveAction;
  delete myDropAction;
  delete myShowAction;
  delete myHideAction;

  delete myLongActions;
  //delete mySurfpaintAction;

  delete d;
}


string Control::description() const
{
  return name();
}

void
Control::doOnSelect( ActionPool * pool )
{
  doAlsoOnSelect( pool );
}

void
Control::doOnDeselect( ActionPool * pool )
{
  myLongActions->reset();

  // cout << "--------> Control Deselected: " << name() << endl;
  doAlsoOnDeselect( pool );
}

void
Control::eventAutoSubscription( ActionPool * )
{}


void
Control::keyPressEvent( QKeyEvent * event )
{
  if( event->isAutoRepeat() )
    {
      event->ignore();
      return;
    }

  if ( myLongActions->submitKeyPressEvent( event ) )
    {
      event->accept();
      return;
    }

  KeyMapKey key( event->key(), event->modifiers() );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator
    iter( myKeyPressActionMap.find( key ) );
  if( iter != myKeyPressActionMap.end() )
    {
      iter->second->execute( );
      event->accept();
    }
  else
    event->ignore();
}

void
Control::keyReleaseEvent( QKeyEvent * event )
{
  if( event->isAutoRepeat() )
    {
      event->ignore();
      return;
    }

  if ( myLongActions->submitKeyReleaseEvent( event ) )
    {
      event->accept();
      return;
    }

  KeyMapKey key( event->key(), event->modifiers() );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator
    iter2( myKeyReleaseActionMap.find( key ) );
  if( iter2 != myKeyReleaseActionMap.end() )
    {
      iter2->second->execute( );
      event->accept();
    }
  else
    event->ignore();
}


void
Control::mousePressEvent ( QMouseEvent * event  )
{
//   cout << "MOUSEPRESSEVENT" << endl;
//   cout << "Control :" << this << ", " << name() << endl;

  if( _mouseTracking().first > 0 )
  {
    pair<int, QWidget *> & track = _mouseTracking();
    --track.first;
    if( track.first == 0 )
      _removeMouseTracking();
  }

  if ( myLongActions->submitMousePressEvent( event ) )
    return;

  MouseButtonMapKey k( event->button(), event->modifiers() );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    iter( myMousePressButtonActionMap.find( k ) );
  if( iter != myMousePressButtonActionMap.end() )
  {
    iter->second->execute( event->x(), event->y(),
                           event->globalX(), event->globalY() );
    event->accept();
  }
  else
    event->ignore();

}

void
Control::mouseReleaseEvent ( QMouseEvent * event  )
{
//   cout << "MOUSERELEASEEVENT" << endl;
//   cout << "Control :" << this << endl;

  if( _mouseTracking().first )
    return;

  if( myLongActions->submitMouseReleaseEvent( event ) )
    return;

  MouseButtonMapKey k( event->button(), event->modifiers() );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    iter( myMouseReleaseButtonActionMap.find( k ) );
  if( iter != myMouseReleaseButtonActionMap.end() )
    iter->second->execute( event->x(), event->y(),
                          event->globalX(), event->globalY() );
}

void
Control::mouseDoubleClickEvent ( QMouseEvent * event  )
{
//   cout << "MOUSEDOUBLE CLICK" << endl;
  MouseButtonMapKey k( event->button(), event->modifiers() );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    iter( myMouseDoubleClickButtonActionMap.find( k ) );
  if( iter != myMouseDoubleClickButtonActionMap.end() )
    iter->second->execute( event->x(), event->y(),
                          event->globalX(), event->globalY() );
  else
  {
    if( myLongActions->submitMousePressEvent( event ) )
      myLongActions->setMouseTracking( true );
  }
}

void
Control::mouseMoveEvent ( QMouseEvent * event  )
{
//   cout << "MOUSEMOVEEVENT " << event->button() << ", " << event->modifiers() << ", " << event->x() << ", " << event->y() << endl;
//   cout << "Control :" << this << endl;
  if( myLongActions->submitMouseMoveEvent( event ) )
    return;

  MouseButtonMapKey k( event->button(), event->modifiers() );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    iter( myMouseMoveActionMap.find( k ) );
  if( iter != myMouseMoveActionMap.end() )
    iter->second->execute( event->x(), event->y(),
                          event->globalX(), event->globalY() );
}

void
Control::wheelEvent ( QWheelEvent * event  )
{
  if( myWheelAction )
#if QT_VERSION >= 0x050e00
    myWheelAction->execute( event->angleDelta().y(), event->position().x(),
                            event->position().y(),
                            event->globalPosition().x(),
                            event->globalPosition().y() );
#else
    myWheelAction->execute( event->angleDelta().y(), event->pos().x(),
                            event->pos().y(),
                            event->globalPos().x(),
                            event->globalPos().y() );
#endif
//   else cout << "no wheel action\n";
}

void
Control::focusInEvent (   )
{
  if( myFocusInAction )
    myFocusInAction->execute( );

}

void
Control::focusOutEvent (  )
{
  if( myFocusOutAction )
    myFocusOutAction->execute( );
}

void
Control::enterEvent ( )
{
  if( myEnterAction )
    myEnterAction->execute( );
}

void
Control::leaveEvent (  )
{
  if( myLeaveAction )
    myLeaveAction->execute( );
}

void
Control::paintEvent ( QPaintEvent * event  )
{
  if( myPaintAction )
        myPaintAction->execute( event->rect().left(), event->rect().top(),
                               event->rect().height(), event->rect().width() );
}

void
Control::moveEvent ( QMoveEvent * event  )
{
  if( myMoveAction )
    myMoveAction->execute( event->pos().x(), event->pos().y(),
                          event->oldPos().x(), event->oldPos().y() );
}

void
Control::resizeEvent ( QResizeEvent * event  )
{
  if( myResizeAction )
    myResizeAction->execute( event->size().width(), event->size().height(),
                            event->oldSize().width(), event->oldSize().height() );
}

void
Control::dragEnterEvent (   )
{
  if( myDragEnterAction )
    myDragEnterAction->execute( );
}

void
Control::dragMoveEvent (  )
{
  if( myDragMoveAction )
    myDragMoveAction->execute( );
}

void
Control::dragLeaveEvent ( )
{
  if( myDragLeaveAction )
    myDragLeaveAction->execute( );
}

void
Control::dropEvent ( QDropEvent * event  )
{
  if( myDropAction )
    myDropAction->execute( event->pos().x(), event->pos().y(),
                           event->dropAction() );
}

void
Control::showEvent ( QShowEvent * event  )
{
  if( myShowAction )
    myShowAction->execute( event->spontaneous() );
}

void
Control::hideEvent ( QHideEvent * event  )
{
  if( myHideAction )
    myHideAction->execute( event->spontaneous() );
}

void Control::selectionChangedEvent()
{
  if( mySelectionChangedAction )
    mySelectionChangedAction->execute();
}


void Control::gestureEvent( QGestureEvent * event )
{
  // cout << "Gesture event\n";
  if( QGesture *swipe = event->gesture( Qt::SwipeGesture ) )
  {
    event->setAccepted( swipe,
                        swipeGesture(
                          static_cast<QSwipeGesture *>( swipe ) ) );
  }
  else if( QGesture *pan = event->gesture( Qt::PanGesture ) )
  {
    event->setAccepted( pan, panGesture( static_cast<QPanGesture *>( pan ) ) );
  }
  if( QGesture *pinch = event->gesture(Qt::PinchGesture ) )
  {
    event->setAccepted( pinch, pinchGesture(
      static_cast<QPinchGesture *>( pinch ) ) );
  }
  if( QGesture *taphold = event->gesture( Qt::TapAndHoldGesture ) )
  {
    event->setAccepted( taphold,
                        tapAndHoldGesture(
                          static_cast<QTapAndHoldGesture *>( taphold ) ) );
  }
  else if( QGesture *tap = event->gesture( Qt::TapGesture ) )
  {
    event->setAccepted( tap, tapGesture( static_cast<QTapGesture *>( tap ) ) );
  }
}


bool Control::pinchGesture( QPinchGesture * gesture )
{
  // cout << "Gesture event: pinch\n";
//   cout << "hotspot:" << gesture->hotSpot().rx() << ", " << gesture->hotSpot().ry() << ", " << gesture->totalScaleFactor() << ", " << gesture->scaleFactor() << ", " << gesture->totalRotationAngle() << ", " << gesture->rotationAngle() << endl;

  if( d->doing_pan )
  {
    // cout << "abort pan from pinch\n";
    if( d->myPanCancelAction )
    {
      // ? generate a QPanGesture
    }
    else
    {
      QMouseEvent ev( QEvent::MouseButtonRelease,
                      d->last_pan_pos, d->last_pan_gpos,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
      mouseReleaseEvent( &ev );
    }
    d->doing_pan = false;
  }

  d->doing_pinch = true;

  if( gesture->state() == Qt::GestureStarted )
  {
    // cout << "start\n";
    d->pinch_scale = 1.;
    if( d->myPinchStartAction )
      d->myPinchStartAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QMouseEvent ev( QEvent::MouseButtonPress, QPoint( 0, 0 ),
                      QPoint( (int) gesture->hotSpot().rx(),
                              (int) gesture->hotSpot().rx() ),
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::ShiftModifier );
      mousePressEvent( &ev );
    }
    return true;
  }

  float scl = gesture->totalScaleFactor();
  if( std::isnan( scl ) )
  {
    // some buggy devices / drivers (such as wacom intuos on ubuntu 16.04)
    // report nan as totalScaleFactor, and also sometimes as scaleFactor.
    // Plus, they tend to send (several times) anormally large or small
    // values at the end of the gesture (when the user releases fingers)
    // so we have to filter a little bit.
    if( !std::isnan( gesture->scaleFactor() )
        && gesture->state() == Qt::GestureUpdated
        && gesture->scaleFactor() > 0.9 && gesture->scaleFactor() < 1.1 )
      d->pinch_scale *= gesture->scaleFactor();
    scl = d->pinch_scale;
    gesture->setTotalScaleFactor( scl );
//     scl = gesture->scaleFactor();
//     if( std::isnan( scl ) )
//       scl = 1.F;
  }
  if( gesture->state() == Qt::GestureUpdated )
  {
    if( d->myPinchMoveAction )
      d->myPinchMoveAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QPoint p = QPoint( 0, - (int)( 100 * log( scl ) ) );
      // cout << "update, scl: " << scl << ", p: " << p.y() << endl;
      QMouseEvent ev( QEvent::MouseMove, p,
                      QPoint( (int) gesture->hotSpot().rx(),
                              (int) gesture->hotSpot().rx() ) + p,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::ShiftModifier );
      mouseMoveEvent( &ev );
    }
  }
  else if( gesture->state() == Qt::GestureFinished )
  {
    // cout << "finish, scl: " << scl << endl;
    if( d->myPinchStopAction )
      d->myPinchStopAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QPoint p = QPoint( 0, - (int)( 100 * log( scl ) ) );
      QMouseEvent ev( QEvent::MouseButtonRelease, p,
                      QPoint( (int) gesture->hotSpot().rx(),
                              (int) gesture->hotSpot().rx() ) + p,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::ShiftModifier );
      mouseReleaseEvent( &ev );
    }
    d->doing_pinch = false;
  }
  else if( gesture->state() == Qt::GestureCanceled )
  {
    // cout << "cancel, scl: " << scl << endl;
    if( d->myPinchCancelAction )
      d->myPinchCancelAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QPoint p = QPoint( 0, - (int)( 100 * log( scl ) ) );
      QMouseEvent ev( QEvent::MouseButtonRelease, p,
                      QPoint( (int) gesture->hotSpot().rx(),
                              (int) gesture->hotSpot().rx() ) + p,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::ShiftModifier );
      mouseReleaseEvent( &ev );
    }
    d->doing_pinch = false;
  }
  return true;
}


bool Control::panGesture( QPanGesture * gesture )
{
  // cout << "Gesture event: pan\n";

  // try to find associated widget, through actions
  QWidget *w = 0;
  if( !myActions.empty() )
  {
    GLWidgetManager *glw = dynamic_cast<GLWidgetManager *>(
        myActions.begin()->second->view() );
    if( glw )
      w = glw->qglWidget();
  }
  // cout << "offset: " << gesture->offset().rx() << ", " << gesture->offset().ry() << ", delta: " << gesture->delta().x() << ", " << gesture->delta().y() << endl;

  if( d->doing_pinch )
  {
    if( d->doing_pan )
    {
      // pinch in progress: abort pan
      // cout << "abort pan\n";
      if( d->myPanCancelAction )
      {
        // ?
      }
      else
      {
        QMouseEvent ev( QEvent::MouseButtonRelease,
                        d->last_pan_pos, d->last_pan_gpos,
                        Qt::MiddleButton, Qt::MiddleButton,
                        Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
        mouseReleaseEvent( &ev );
      }
      d->doing_pan = false;
    }
    // cout << "pinch in progress.\n";
    return false;
  }

  d->doing_pan = true;
//   if( gesture->state() == Qt::GestureStarted )
//     d->fix_pan_offset = gesture->offset().toPoint();
//   d->fix_pan_offset = QPoint( 0, 0 );

  QPoint gpos = QPoint( (int) ( gesture->offset().rx()
                                + gesture->hotSpot().rx() ),
                        (int) ( gesture->offset().ry()
                                + gesture->hotSpot().ry() ) );
//   gpos -= d->fix_pan_offset;
  QPoint pos = gpos;
  if( w )
    pos = w->mapFromGlobal( gpos );
  d->last_pan_gpos = gpos;
  d->last_pan_pos = pos;

  if( gesture->state() == Qt::GestureStarted )
  {
    // cout << "start " << pos.x() << ", " << pos.y() << " / " << gpos.x() << ", " << gpos.y() << endl;
    if( d->myPanStartAction )
      d->myPanStartAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QMouseEvent ev( QEvent::MouseButtonPress,
                      pos, gpos,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
      mousePressEvent( &ev );
    }
  }
  else if( gesture->state() == Qt::GestureUpdated )
  {
    // cout << "update " << pos.x() << ", " << pos.y() << " / " << gpos.x() << ", " << gpos.y() << endl;
    if( d->myPanMoveAction )
      d->myPanMoveAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QMouseEvent ev( QEvent::MouseMove,
                      pos, gpos,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
      mouseMoveEvent( &ev );
    }
  }
  else if( gesture->state() == Qt::GestureFinished )
  {
    // cout << "finish\n";
    if( d->myPanStopAction )
      d->myPanStopAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QMouseEvent ev( QEvent::MouseButtonRelease,
                      pos, gpos,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
      mouseReleaseEvent( &ev );
    }
    d->doing_pan = false;
  }
  else if( gesture->state() == Qt::GestureCanceled )
  {
    // cout << "cancel\n";
    if( d->myPanCancelAction )
      d->myPanCancelAction->execute( gesture );
    else
    {
      // for now, simulate corresponding mouse events
      QMouseEvent ev( QEvent::MouseButtonRelease,
                      pos, gpos,
                      Qt::MiddleButton, Qt::MiddleButton,
                      Qt::KeyboardModifier() /*TODO: get actual current modifiers */ );
      mouseReleaseEvent( &ev );
    }
    d->doing_pan = false;
  }
  return true;
}


bool Control::swipeGesture( QSwipeGesture *gesture )
{
  // cout << "Swipe\n";
  if( gesture->state() == Qt::GestureStarted )
  {
    if( d->mySwipeStartAction )
    {
      d->mySwipeStartAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureUpdated )
  {
    if( d->mySwipeMoveAction )
    {
      d->mySwipeMoveAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureFinished )
  {
    if( d->mySwipeStopAction )
    {
      d->mySwipeStopAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureCanceled )
  {
    if( d->mySwipeCancelAction )
    {
      d->mySwipeCancelAction->execute( gesture );
      return true;
    }
    return false;
  }

  return false;
}


bool Control::tapGesture( QTapGesture *gesture )
{
  // cout << "Tap\n";
  if( gesture->state() == Qt::GestureStarted )
  {
    if( d->myTapStartAction )
    {
      d->myTapStartAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureUpdated )
  {
    if( d->myTapMoveAction )
    {
      d->myTapMoveAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureFinished )
  {
    if( d->myTapStopAction )
    {
      d->myTapStopAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureCanceled )
  {
    if( d->myTapCancelAction )
    {
      d->myTapCancelAction->execute( gesture );
      return true;
    }
    return false;
  }

  return false;
}


bool Control::tapAndHoldGesture( QTapAndHoldGesture *gesture )
{
  // cout << "Tap And Hold\n";
  if( gesture->state() == Qt::GestureStarted )
  {
    if( d->myTapAndHoldStartAction )
    {
      d->myTapAndHoldStartAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureUpdated )
  {
    if( d->myTapAndHoldMoveAction )
    {
      d->myTapAndHoldMoveAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureFinished )
  {
    if( d->myTapAndHoldStopAction )
    {
      d->myTapAndHoldStopAction->execute( gesture );
      return true;
    }
    return false;
  }
  if( gesture->state() == Qt::GestureCanceled )
  {
    if( d->myTapAndHoldCancelAction )
    {
      d->myTapAndHoldCancelAction->execute( gesture );
      return true;
    }
    return false;
  }

  return false;
}


namespace
{

  template <typename T>
  std::string actionLinkDefaultName( T* actionlink )
  {
    return typeid( *actionlink ).name();
  }

}


bool
Control::keyPressEventSubscribe( int key,
                                 Qt::KeyboardModifiers buttonState,
                                 const KeyActionLink& actionMethod,
                                 const string & name )
{
  KeyMapKey k(key, buttonState);

  if( myKeyPressActionMap.find(k) != myKeyPressActionMap.end() )
    return false;

  KeyActionLink *actionlink = actionMethod.clone();
  myKeyPressActionMap.insert( pair<KeyMapKey, KeyActionLink* >(
    k, actionlink ) );
  _keyPressActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}

bool
Control::keyReleaseEventSubscribe( int key,
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethod,
                                   const string & name )
{
  KeyMapKey k(key, buttonState);

  if( myKeyReleaseActionMap.find(k) != myKeyReleaseActionMap.end() )
    return false;

  KeyActionLink *actionlink = actionMethod.clone();
  myKeyReleaseActionMap.insert( pair<KeyMapKey, KeyActionLink* >( 
    k, actionlink ) );
  _keyReleaseActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}


bool
Control::mousePressButtonEventSubscribe( Qt::MouseButtons button,
                                         Qt::KeyboardModifiers state,
                                         const MouseActionLink& actionMethod,
                                         const string & name )
{
  MouseButtonMapKey k(button, state);

  if( myMousePressButtonActionMap.find(k) 
      != myMousePressButtonActionMap.end() )
    return false;

  MouseActionLink *actionlink = actionMethod.clone();
  myMousePressButtonActionMap.insert(
    pair<MouseButtonMapKey, MouseActionLink* >( k, actionlink ) );
  _mousePressActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}

bool
Control::mouseReleaseButtonEventSubscribe(
  Qt::MouseButtons button,
  Qt::KeyboardModifiers state,
  const MouseActionLink& actionMethod, const string & name )
{
  MouseButtonMapKey k(button, state);

  if( myMouseReleaseButtonActionMap.find(k) 
      != myMouseReleaseButtonActionMap.end() )
    return false;

  MouseActionLink *actionlink = actionMethod.clone();
  myMouseReleaseButtonActionMap.insert(
    pair<MouseButtonMapKey, MouseActionLink* >( k, actionlink ) );
  _mouseReleaseActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}

bool
Control::mouseDoubleClickEventSubscribe( Qt::MouseButtons button,
                                         Qt::KeyboardModifiers state,
                                         const MouseActionLink& actionMethod,
                                         const string & name )
{
  MouseButtonMapKey k(button, state);

  if( myMouseDoubleClickButtonActionMap.find(k) 
      != myMouseDoubleClickButtonActionMap.end() )
    return false;

  MouseActionLink *actionlink = actionMethod.clone();
  myMouseDoubleClickButtonActionMap.insert(
    pair<MouseButtonMapKey, MouseActionLink* >( k, actionlink ) );
  _mouseDoubleClickActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}

bool
Control::mouseMoveEventSubscribe( Qt::MouseButtons button,
                                  Qt::KeyboardModifiers state,
                                  const MouseActionLink& actionMethod,
                                  const string & name )
{
  MouseButtonMapKey k(button, state);

  if ( myMouseMoveActionMap.find(k) != myMouseMoveActionMap.end() )
    return false;

  MouseActionLink *actionlink = actionMethod.clone();
  myMouseMoveActionMap.insert(
    pair<MouseButtonMapKey, MouseActionLink* >( k, actionlink ) );
  _mouseMoveActionsByName[
    name.empty() ? actionLinkDefaultName( actionlink ) : name ] = actionlink;

  return true;
}

bool
Control::keyAndMouseLongEventSubscribe(
  int startingKey,
  Qt::KeyboardModifiers startingButtonState,
  const KeyActionLink & startingActionMethod,
  Qt::MouseButtons longButton,
  Qt::KeyboardModifiers longState,
  const MouseActionLink & longActionMethod,
  int endingKey,
  Qt::KeyboardModifiers endingButtonState,
  const KeyActionLink & endingActionMethod,
  bool exclusiveAction )
{
  return myLongActions->keyAndMouseLongEventSubscribe( startingKey,
                                                       startingButtonState,
                                                       startingActionMethod,
                                                       longButton,
                                                       longState,
                                                       longActionMethod,
                                                       endingKey,
                                                       endingButtonState,
                                                       endingActionMethod,
                                                       exclusiveAction );
}

bool
Control::mouseLongEventSubscribe( Qt::MouseButtons startingButton,
                                  Qt::KeyboardModifiers startingButtonState,
                                  const MouseActionLink& startingActionMethod,
                                  const MouseActionLink& longActionMethod,
                                  const MouseActionLink& endingActionMethod,
                                  bool exclusiveAction )
{
  return myLongActions->mouseLongEventSubscribe( startingButton,
                                                 startingButtonState,
                                                 startingActionMethod,
                                                 longActionMethod,
                                                 endingActionMethod,
                                                 exclusiveAction );
}


bool
Control::keyRepetitiveEventSubscribe( int startingKey,
                                      Qt::KeyboardModifiers startingButtonState,
                                      const KeyActionLink& startingActionMethod,

                                      int endingKey,
                                      Qt::KeyboardModifiers endingButtonState,
                                      const KeyActionLink & endingActionMethod,
                                      bool exclusiveAction,
                                      float temporalStep )
{
  return myLongActions->keyRepetitiveEventSubscribe( startingKey,
                                                     startingButtonState,
                                                     startingActionMethod,
                                                     endingKey,
                                                     endingButtonState,
                                                     endingActionMethod,
                                                     exclusiveAction,
                                                     temporalStep );
}


bool
Control::wheelEventSubscribe( const WheelActionLink& actionMethod )
{
  if( myWheelAction != 0 )
    return false;
  myWheelAction = actionMethod.clone();
  return true;
}

bool
Control::focusInEventSubscribe( const FocusActionLink& actionMethod )
{
  if( myFocusInAction != 0 )
    return false;
  myFocusInAction = actionMethod.clone();
  return true;
}

bool
Control::focusOutEventSubscribe( const FocusActionLink& actionMethod )
{
  if( myFocusOutAction != 0 )
    return false;
  myFocusOutAction = actionMethod.clone();
  return true;
}

bool
Control::enterEventSubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( myEnterAction != 0 )
    return false;
  myEnterAction = actionMethod.clone();
  return true;
}

bool
Control::leaveEventSubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( myLeaveAction != 0 )
    return false;
  myLeaveAction = actionMethod.clone();
  return true;
}

bool
Control::paintEventSubscribe( const PaintActionLink& actionMethod )
{
  if( myPaintAction != 0 )
    return false;
  myPaintAction = actionMethod.clone();
  return true;
}

bool
Control::moveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myMoveAction != 0 )
    return false;
  myMoveAction = actionMethod.clone();
  return true;
}

bool
Control::resizeEventSubscribe( const ResizeActionLink& actionMethod )
{
  if( myResizeAction != 0 )
    return false;
  myResizeAction = actionMethod.clone();
  return true;
}

bool
Control::dragEnterEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragEnterAction != 0 )
       return false;
  myDragEnterAction = actionMethod.clone();
  return true;
}

bool
Control::dragLeaveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragLeaveAction != 0 )
       return false;
  myDragLeaveAction = actionMethod.clone();
  return true;
}

bool
Control::dragMoveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragMoveAction != 0 )
       return false;
  myDragMoveAction = actionMethod.clone();
  return true;
}

bool
Control::dropEventSubscribe( const DropActionLink& actionMethod )
{
  if( myDropAction != 0 )
       return false;
  myDropAction = actionMethod.clone();
  return true;
}

bool
Control::showEventSubscribe( const ShowHideActionLink& actionMethod )
{
  if( myShowAction != 0 )
       return false;
  myShowAction = actionMethod.clone();
  return true;
}

bool
Control::hideEventSubscribe( const ShowHideActionLink& actionMethod )
{
  if( myHideAction != 0 )
       return false;
  myHideAction = actionMethod.clone();
  return true;
}


bool Control::selectionChangedEventSubscribe
    ( const SelectionChangedActionLink& actionMethod )
{
  if( mySelectionChangedAction != 0 )
    return false;
  mySelectionChangedAction = actionMethod.clone();
  return true;
}


bool Control::selectionChangedEventUnsubscribe
    ( const SelectionChangedActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( mySelectionChangedAction ) )
    return false;
  else
  {
    delete mySelectionChangedAction;
    mySelectionChangedAction = 0;
  }
  return true;
}


bool Control::selectionChangedEventUnsubscribe()
{
  delete mySelectionChangedAction;
  mySelectionChangedAction = 0;
  return true;
}


bool Control::pinchEventSubscribe(
  const PinchActionLink & startMethod,
  const PinchActionLink & moveMethod,
  const PinchActionLink & stopMethod,
  const PinchActionLink & cancelMethod )
{
  if( d->myPinchStartAction != 0 )
    return false;
  d->myPinchStartAction  = startMethod.clone();
  d->myPinchMoveAction   = moveMethod.clone();
  d->myPinchStopAction   = stopMethod.clone();
  d->myPinchCancelAction = cancelMethod.clone();
  return true;
}

bool Control::pinchEventUnsubscribe()
{
  delete d->myPinchStartAction;
  d->myPinchStartAction = 0;
  delete d->myPinchMoveAction;
  d->myPinchMoveAction = 0;
  delete d->myPinchStopAction;
  d->myPinchStopAction = 0;
  delete d->myPinchCancelAction;
  d->myPinchCancelAction = 0;
  return true;
}


bool Control::panEventSubscribe(
  const PanActionLink & startMethod,
  const PanActionLink & moveMethod,
  const PanActionLink & stopMethod,
  const PanActionLink & cancelMethod )
{
  if( d->myPanStartAction != 0 )
    return false;
  d->myPanStartAction  = startMethod.clone();
  d->myPanMoveAction   = moveMethod.clone();
  d->myPanStopAction   = stopMethod.clone();
  d->myPanCancelAction = cancelMethod.clone();
  return true;
}

bool Control::panEventUnsubscribe()
{
  delete d->myPanStartAction;
  d->myPanStartAction = 0;
  delete d->myPanMoveAction;
  d->myPanMoveAction = 0;
  delete d->myPanStopAction;
  d->myPanStopAction = 0;
  delete d->myPanCancelAction;
  d->myPanCancelAction = 0;
  return true;
}


bool Control::swipeEventSubscribe(
  const SwipeActionLink & startMethod,
  const SwipeActionLink & moveMethod,
  const SwipeActionLink & stopMethod,
  const SwipeActionLink & cancelMethod )
{
  if( d->mySwipeStartAction != 0 )
    return false;
  d->mySwipeStartAction  = startMethod.clone();
  d->mySwipeMoveAction   = moveMethod.clone();
  d->mySwipeStopAction   = stopMethod.clone();
  d->mySwipeCancelAction = cancelMethod.clone();
  return true;
}

bool Control::swipeEventUnsubscribe()
{
  delete d->mySwipeStartAction;
  d->mySwipeStartAction = 0;
  delete d->mySwipeMoveAction;
  d->mySwipeMoveAction = 0;
  delete d->mySwipeStopAction;
  d->mySwipeStopAction = 0;
  delete d->mySwipeCancelAction;
  d->mySwipeCancelAction = 0;
  return true;
}


bool Control::tapEventSubscribe(
  const TapActionLink & startMethod,
  const TapActionLink & moveMethod,
  const TapActionLink & stopMethod,
  const TapActionLink & cancelMethod )
{
  if( d->myTapStartAction != 0 )
    return false;
  d->myTapStartAction  = startMethod.clone();
  d->myTapMoveAction   = moveMethod.clone();
  d->myTapStopAction   = stopMethod.clone();
  d->myTapCancelAction = cancelMethod.clone();
  return true;
}

bool Control::tapEventUnsubscribe()
{
  delete d->myTapStartAction;
  d->myTapStartAction = 0;
  delete d->myTapMoveAction;
  d->myTapMoveAction = 0;
  delete d->myTapStopAction;
  d->myTapStopAction = 0;
  delete d->myTapCancelAction;
  d->myTapCancelAction = 0;
  return true;
}



bool
Control::keyPressEventUnsubscribe( int key,
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethod )
{
  KeyMapKey k(key, buttonState );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator 
    found( myKeyPressActionMap.find(k) );

  if ( found != myKeyPressActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, KeyActionLink*>::iterator
            in=_keyPressActionsByName.begin(),
            en=_keyPressActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _keyPressActionsByName.erase( in );
          break;
        }
      myKeyPressActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::keyPressEventUnsubscribe( int key,
                                   Qt::KeyboardModifiers buttonState )
{
  KeyMapKey k(key, buttonState );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator
    found( myKeyPressActionMap.find(k) );

  if ( found != myKeyPressActionMap.end() )
  {
    for( map<string, KeyActionLink*>::iterator
          in=_keyPressActionsByName.begin(),
          en=_keyPressActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _keyPressActionsByName.erase( in );
        break;
      }
    myKeyPressActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::keyReleaseEventUnsubscribe( int key,
                                     Qt::KeyboardModifiers buttonState,
                                     const KeyActionLink& actionMethod )
{
  KeyMapKey k(key, buttonState );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator
    found( myKeyPressActionMap.find(k) );

  if ( found != myKeyReleaseActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, KeyActionLink*>::iterator
            in=_keyReleaseActionsByName.begin(),
            en=_keyReleaseActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _keyReleaseActionsByName.erase( in );
          break;
        }
      myKeyReleaseActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::keyReleaseEventUnsubscribe( int key,
                                     Qt::KeyboardModifiers buttonState )
{
  KeyMapKey k(key, buttonState );

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator
    found( myKeyPressActionMap.find(k) );

  if ( found != myKeyReleaseActionMap.end() )
  {
    for( map<string, KeyActionLink*>::iterator
          in=_keyReleaseActionsByName.begin(),
          en=_keyReleaseActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _keyReleaseActionsByName.erase( in );
        break;
      }
    myKeyReleaseActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::mousePressButtonEventUnsubscribe( 
  Qt::MouseButtons button,
  Qt::KeyboardModifiers state,
  const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMousePressButtonActionMap.find(k) );

  if ( found != myMousePressButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, MouseActionLink*>::iterator
            in=_mousePressActionsByName.begin(),
            en=_mousePressActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _mousePressActionsByName.erase( in );
          break;
        }
      myMousePressButtonActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::mousePressButtonEventUnsubscribe(
  Qt::MouseButtons button,
  Qt::KeyboardModifiers state )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMousePressButtonActionMap.find(k) );

  if ( found != myMousePressButtonActionMap.end() )
  {
    for( map<string, MouseActionLink*>::iterator
          in=_mousePressActionsByName.begin(),
          en=_mousePressActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _mousePressActionsByName.erase( in );
        break;
      }
    myMousePressButtonActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::mouseReleaseButtonEventUnsubscribe(
  Qt::MouseButtons button,
  Qt::KeyboardModifiers state,
  const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseReleaseButtonActionMap.find(k) );

  if ( found != myMouseReleaseButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, MouseActionLink*>::iterator
            in=_mouseReleaseActionsByName.begin(),
            en=_mouseReleaseActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _mouseReleaseActionsByName.erase( in );
          break;
        }
      myMouseReleaseButtonActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::mouseReleaseButtonEventUnsubscribe(
  Qt::MouseButtons button,
  Qt::KeyboardModifiers state )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseReleaseButtonActionMap.find(k) );

  if ( found != myMouseReleaseButtonActionMap.end() )
  {
    for( map<string, MouseActionLink*>::iterator
          in=_mouseReleaseActionsByName.begin(),
          en=_mouseReleaseActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _mouseReleaseActionsByName.erase( in );
        break;
      }
    myMouseReleaseButtonActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::mouseDoubleClickEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink
                                           & actionMethod )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseDoubleClickButtonActionMap.find(k) );

  if ( found != myMouseDoubleClickButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, MouseActionLink*>::iterator
            in=_mouseDoubleClickActionsByName.begin(),
            en=_mouseDoubleClickActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _mouseDoubleClickActionsByName.erase( in );
          break;
        }
      myMouseDoubleClickButtonActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::mouseDoubleClickEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseDoubleClickButtonActionMap.find(k) );

  if ( found != myMouseDoubleClickButtonActionMap.end() )
  {
    for( map<string, MouseActionLink*>::iterator
          in=_mouseDoubleClickActionsByName.begin(),
          en=_mouseDoubleClickActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _mouseDoubleClickActionsByName.erase( in );
        break;
      }
    myMouseDoubleClickButtonActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::mouseMoveEventUnsubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state,
                                    const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseMoveActionMap.find(k) );

  if ( found != myMouseMoveActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) )
    {
      for( map<string, MouseActionLink*>::iterator
            in=_mouseMoveActionsByName.begin(),
            en=_mouseMoveActionsByName.end();
          in!=en; ++in )
        if( in->second == found->second )
        {
          _mouseMoveActionsByName.erase( in );
          break;
        }
      myMouseMoveActionMap.erase( found );
      delete found->second;
      return true;
    }
  return false;
}


bool
Control::mouseMoveEventUnsubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state )
{
  MouseButtonMapKey k(button, state );

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator
    found( myMouseMoveActionMap.find(k) );

  if ( found != myMouseMoveActionMap.end() )
  {
    for( map<string, MouseActionLink*>::iterator
          in=_mouseMoveActionsByName.begin(),
          en=_mouseMoveActionsByName.end();
        in!=en; ++in )
      if( in->second == found->second )
      {
        _mouseMoveActionsByName.erase( in );
        break;
      }
    myMouseMoveActionMap.erase( found );
    delete found->second;
    return true;
  }
  return false;
}

bool
Control::keyAndMouseLongEventUnsubscribe( int startingKey,
                                          Qt::KeyboardModifiers startingButtonState,
                                          Qt::MouseButtons longButton,
                                          Qt::KeyboardModifiers longState,
                                          int endingKey,
                                          Qt::KeyboardModifiers endingButtonState )
{
  return myLongActions->keyAndMouseLongEventUnsubscribe( startingKey,
                                                         startingButtonState,
                                                         longButton,
                                                         longState,
                                                         endingKey,
                                                         endingButtonState );
}

bool
Control::mouseLongEventUnsubscribe( Qt::MouseButtons startingButton,
                                    Qt::KeyboardModifiers startingButtonState )
{
  return myLongActions->mouseLongEventUnsubscribe( startingButton,
                                                   startingButtonState );
}

bool
Control::keyRepetitiveEventUnsubscribe( int startingKey,
                                        Qt::KeyboardModifiers startingButtonState,
                                        int endingKey,
                                        Qt::KeyboardModifiers endingButtonState )
{
  return myLongActions->keyRepetitiveEventUnsubscribe( startingKey,
                                                       startingButtonState,
                                                       endingKey,
                                                       endingButtonState );
}

bool
Control::wheelEventUnsubscribe( const WheelActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myWheelAction ) )
    return false;
  else {
    delete myWheelAction;
    myWheelAction = 0;
  }
  return true;
}


bool
Control::wheelEventUnsubscribe()
{
  delete myWheelAction;
  myWheelAction = 0;
  return true;
}

bool
Control::wheelEventUnsubscribeAll( )
{
  if( myWheelAction == 0 )
    return false;
  else {
    delete myWheelAction;
    myWheelAction = 0;
  }
  return true;
}

bool
Control::focusInEventUnsubscribe( const FocusActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myFocusInAction ) )
    return false;
  else {
    delete myFocusInAction;
    myFocusInAction = 0;
  }
  return true;
}


bool
Control::focusInEventUnsubscribe()
{
  delete myFocusInAction;
  myFocusInAction = 0;
  return true;
}

bool
Control::focusOutEventUnsubscribe( const FocusActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myFocusOutAction ) )
    return false;
  else {
    delete myFocusOutAction;
    myFocusOutAction = 0;
  }
  return true;
}


bool
Control::focusOutEventUnsubscribe()
{
  delete myFocusOutAction;
  myFocusOutAction = 0;
  return true;
}

bool
Control::enterEventUnsubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myEnterAction ) )
    return false;
  else {
    delete myEnterAction;
    myEnterAction = 0;
  }
  return true;
}


bool
Control::enterEventUnsubscribe()
{
  delete myEnterAction;
  myEnterAction = 0;
  return true;
}

bool
Control::leaveEventUnsubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myLeaveAction ) )
    return false;
  else {
    delete myLeaveAction;
    myLeaveAction = 0;
  }
  return true;
}


bool
Control::leaveEventUnsubscribe()
{
  delete myLeaveAction;
  myLeaveAction = 0;
  return true;
}

bool
Control::paintEventUnsubscribe( const PaintActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myPaintAction ) )
    return false;
  else {
    delete myPaintAction;
    myPaintAction = 0;
  }
  return true;
}


bool
Control::paintEventUnsubscribe()
{
  delete myPaintAction;
  myPaintAction = 0;
  return true;
}

bool
Control::moveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myMoveAction ) )
    return false;
  else {
    delete myMoveAction;
    myMoveAction = 0;
  }
  return true;
}


bool
Control::moveEventUnsubscribe()
{
  delete myMoveAction;
  myMoveAction = 0;
  return true;
}

bool
Control::resizeEventUnsubscribe( const ResizeActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myResizeAction ) )
    return false;
  else {
    delete myResizeAction;
    myResizeAction = 0;
  }
  return true;
}


bool
Control::resizeEventUnsubscribe()
{
  delete myResizeAction;
  myResizeAction = 0;
  return true;
}

bool
Control::dragEnterEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragEnterAction ) )
    return false;
  else {
    delete myDragEnterAction;
    myDragEnterAction = 0;
  }
  return true;
}


bool
Control::dragEnterEventUnsubscribe()
{
  delete myDragEnterAction;
  myDragEnterAction = 0;
  return true;
}

bool
Control::dragLeaveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragLeaveAction ) )
    return false;
  else {
    delete myDragLeaveAction;
    myDragLeaveAction = 0;
  }
  return true;
}


bool
Control::dragLeaveEventUnsubscribe()
{
  delete myDragLeaveAction;
  myDragLeaveAction = 0;
  return true;
}

bool
Control::dragMoveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragMoveAction ) )
    return false;
  else {
    delete myDragMoveAction;
    myDragMoveAction = 0;
  }
  return true;
}


bool
Control::dragMoveEventUnsubscribe()
{
  delete myDragMoveAction;
  myDragMoveAction = 0;
  return true;
}

bool
Control::dropEventUnsubscribe( const DropActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDropAction ) )
    return false;
  else {
    delete myDropAction;
    myDropAction = 0;
  }
  return true;
}


bool
Control::dropEventUnsubscribe()
{
  delete myDropAction;
  myDropAction = 0;
  return true;
}

bool
Control::showEventUnsubscribe( const ShowHideActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myShowAction ) )
    return false;
  else {
    delete myShowAction;
    myShowAction = 0;
  }
  return true;
}


bool
Control::showEventUnsubscribe()
{
  delete myShowAction;
  myShowAction = 0;
  return true;
}

bool
Control::hideEventUnsubscribe( const ShowHideActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myHideAction ) )
    return false;
  else {
    delete myHideAction;
    myHideAction = 0;
  }
  return true;
}

bool
Control::hideEventUnsubscribe()
{
  delete myHideAction;
  myHideAction = 0;
  return true;
}


Control::KeyActionLink*
Control::keyPressActionLinkByName( const string & name ) const
{
  map<string, KeyActionLink*>::const_iterator
    i = _keyPressActionsByName.find( name );
  if( i == _keyPressActionsByName.end() )
    return 0;
  return i->second;
}


Control::KeyActionLink*
Control::keyReleaseActionLinkByName( const string & name ) const
{
  map<string, KeyActionLink*>::const_iterator
    i = _keyReleaseActionsByName.find( name );
  if( i == _keyReleaseActionsByName.end() )
    return 0;
  return i->second;
}


Control::MouseActionLink*
Control::mousePressActionLinkByName( const string & name ) const
{
  map<string, MouseActionLink*>::const_iterator
    i = _mousePressActionsByName.find( name );
  if( i == _mousePressActionsByName.end() )
    return 0;
  return i->second;
}


Control::MouseActionLink*
Control::mouseReleaseActionLinkByName( const string & name ) const
{
  map<string, MouseActionLink*>::const_iterator
    i = _mouseReleaseActionsByName.find( name );
  if( i == _mouseReleaseActionsByName.end() )
    return 0;
  return i->second;
}


Control::MouseActionLink*
Control::mouseDoubleClickActionLinkByName( const string & name ) const
{
  map<string, MouseActionLink*>::const_iterator
    i = _mouseDoubleClickActionsByName.find( name );
  if( i == _mouseDoubleClickActionsByName.end() )
    return 0;
  return i->second;
}


Control::MouseActionLink*
Control::mouseMoveActionLinkByName( const string & name ) const
{
  map<string, MouseActionLink*>::const_iterator
    i = _mouseMoveActionsByName.find( name );
  if( i == _mouseMoveActionsByName.end() )
    return 0;
  return i->second;
}


Control::WheelActionLink* Control::wheelActionLink() const
{
  return myWheelAction;
}


Control::FocusActionLink* Control::focusInActionLink() const
{
  return myFocusInAction;
}


Control::FocusActionLink* Control::focusOutActionLink() const
{
  return myFocusOutAction;
}


set<string> Control::keyPressActionLinkNames() const
{
  cout << "Control::keyPressActionLinkNames\n";
  set<string> names;
  map<string, KeyActionLink*>::const_iterator
    in, en = _keyPressActionsByName.end();

  for( in=_keyPressActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  cout << "num: " << names.size() << endl;
  return names;
}


set<string> Control::keyReleaseActionLinkNames() const
{
  set<string> names;
  map<string, KeyActionLink*>::const_iterator
    in, en = _keyReleaseActionsByName.end();

  for( in=_keyReleaseActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  return names;
}


set<string> Control::mousePressActionLinkNames() const
{
  set<string> names;
  map<string, MouseActionLink*>::const_iterator
    in, en = _mousePressActionsByName.end();

  for( in=_mousePressActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  return names;
}


set<string> Control::mouseReleaseActionLinkNames() const
{
  set<string> names;
  map<string, MouseActionLink*>::const_iterator
    in, en = _mouseReleaseActionsByName.end();

  for( in=_mouseReleaseActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  return names;
}


set<string> Control::mouseDoubleClickActionLinkNames() const
{
  set<string> names;
  map<string, MouseActionLink*>::const_iterator
    in, en = _mouseDoubleClickActionsByName.end();

  for( in=_mouseDoubleClickActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  return names;
}


set<string> Control::mouseMoveActionLinkNames() const
{
  set<string> names;
  map<string, MouseActionLink*>::const_iterator
    in, en = _mouseMoveActionsByName.end();

  for( in=_mouseMoveActionsByName.begin(); in!=en; ++in )
    names.insert( in->first );

  return names;
}


const set<string> & Control::inhibitedActions() const
{
  return d->inhibitedActions;
}


void Control::inhibitAction( const string & action, bool inhibit )
{
  if( inhibit )
    d->inhibitedActions.insert( action );
  else
    d->inhibitedActions.erase( action );
}


// ---

LongActions::LongActions() : myActiveKeyAndMouseLongEvent(0), myActiveMouseLongEvent(0)
{
//   cout << "NEW LONGACTION" << endl;

}

LongActions::~LongActions()
{
  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator iter3(myKeyAndMouseLongEventMap.begin()), last3(myKeyAndMouseLongEventMap.end());

  while( iter3 != last3 ){
    delete iter3->second;
    ++iter3;
  }

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator iter4(myKeyRepetitiveEventMap.begin()), last4(myKeyRepetitiveEventMap.end());

  while( iter4 != last4 ){
    delete iter4->second;
    ++iter4;
  }

  map<Control::MouseButtonMapKey, MouseLongEvent*,
    Control::LessMouseMap>::iterator iter5(myMouseLongEventMap.begin()),
    last5(myMouseLongEventMap.end());

  while( iter5 != last5 ){
    delete iter5->second;
    ++iter5;
  }
}

void
LongActions::reset()
{
  myActiveKeyRepetitiveEvents.clear();
  myActiveKeyAndMouseLongEvent = 0;
  myActiveMouseLongEvent = 0;

//   cout << "myActiveMouseLongEvent set to 0 : reset" << endl;

}


void
LongActions::setMouseTracking( bool t )
{
  myActiveMouseLongEvent->setMouseTracking( t );
}

bool
LongActions::keyAndMouseLongEventSubscribe( int startingKey,
                                            Qt::KeyboardModifiers startingButtonState,
                                            const Control::KeyActionLink & startingActionMethod,
                                            Qt::MouseButtons longButton,
                                            Qt::KeyboardModifiers longState,
                                            const Control::MouseActionLink & longActionMethod,
                                            int endingKey,
                                            Qt::KeyboardModifiers endingButtonState,
                                            const Control::KeyActionLink & endingActionMethod,
                                            bool exclusiveAction )
{
  Control::KeyMapKey startKey( startingKey, startingButtonState );
  Control::MouseButtonMapKey longKey( longButton,longState );
  Control::KeyMapKey endKey( endingKey, endingButtonState );

  if ( myKeyAndMouseLongEventMap.find(startKey) != myKeyAndMouseLongEventMap.end() )
    return false;

  KeyAndMouseLongEvent * longEvent =
    new KeyAndMouseLongEvent( startKey,
                              startingActionMethod,
                              longKey,
                              longActionMethod,
                              endKey,
                              endingActionMethod,
                              exclusiveAction );

  myKeyAndMouseLongEventMap.insert( pair< Control::KeyMapKey, KeyAndMouseLongEvent* >( startKey, longEvent ) );

  return true;
}

bool
LongActions::mouseLongEventSubscribe( Qt::MouseButtons startingButton,
                                     Qt::KeyboardModifiers startingButtonState,
                                     const Control::MouseActionLink& startingActionMethod,
                                     const Control::MouseActionLink& longActionMethod,
                                     const Control::MouseActionLink& endingActionMethod,
                                     bool exclusiveAction )
{
  Control::MouseButtonMapKey startKey( startingButton, startingButtonState );

  if ( myMouseLongEventMap.find(startKey) != myMouseLongEventMap.end() )
    return false;

  MouseLongEvent * longEvent =
    new MouseLongEvent( startKey,
                        startingActionMethod,
                        startKey,
                        longActionMethod,
                        startKey,
                        endingActionMethod,
                        exclusiveAction );

  myMouseLongEventMap.insert( pair< Control::MouseButtonMapKey, MouseLongEvent* >( startKey,
                                                                                 longEvent ) );

  return true;
}


bool
LongActions::keyRepetitiveEventSubscribe( int startingKey,
                                      Qt::KeyboardModifiers startingButtonState,
                                      const Control::KeyActionLink& startingActionMethod,

                                      int endingKey,
                                      Qt::KeyboardModifiers endingButtonState,
                                      const Control::KeyActionLink& endingActionMethod,
                                      bool exclusiveAction,
                                      float temporalStep )
{
  Control::KeyMapKey startKey( startingKey, startingButtonState );
  Control::KeyMapKey endKey( endingKey, endingButtonState );

  if ( myKeyRepetitiveEventMap.find(startKey) != myKeyRepetitiveEventMap.end() )
    return false;

  KeyRepetitiveEvent * repetitiveEvent =
    new KeyRepetitiveEvent( startKey,
                            startingActionMethod,
                            endKey,
                            endingActionMethod,
                            exclusiveAction,
                            temporalStep );

  myKeyRepetitiveEventMap.insert( pair< Control::KeyMapKey, KeyRepetitiveEvent* >( startKey, repetitiveEvent ) );

  return true;
}

bool
LongActions::keyAndMouseLongEventUnsubscribe( int startingKey,
                                          Qt::KeyboardModifiers startingButtonState,
                                          Qt::MouseButtons longButton,
                                          Qt::KeyboardModifiers longState,
                                          int endingKey,
                                          Qt::KeyboardModifiers endingButtonState )
{
  Control::KeyMapKey startKey( startingKey, startingButtonState );
  Control::MouseButtonMapKey longKey( longButton,longState );
  Control::KeyMapKey endKey( endingKey, endingButtonState );

  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator found( myKeyAndMouseLongEventMap.find( startKey ) );

  if ( found != myKeyAndMouseLongEventMap.end() )
    if ( startKey == found->second->startingEvent() &&
         longKey == found->second->longEvent() &&
         endKey == found->second->endingEvent() ) {
      delete found->second;
      myKeyAndMouseLongEventMap.erase( found );
      myActiveKeyAndMouseLongEvent = 0;
      return true;
    }
  return false;
}

bool
LongActions::mouseLongEventUnsubscribe( Qt::MouseButtons startingButton,
                                    Qt::KeyboardModifiers startingButtonState )
{
  Control::MouseButtonMapKey startKey( startingButton, startingButtonState );

  map<Control::MouseButtonMapKey, MouseLongEvent*, Control::LessMouseMap>::iterator found( myMouseLongEventMap.find( startKey ) );

  if ( found != myMouseLongEventMap.end() )
    if ( startKey == found->second->startingEvent() ) {
      delete found->second;
      myMouseLongEventMap.erase( found );
      myActiveMouseLongEvent = 0;
//       cout << "myActiveMouseLongEvent set to 0 : mouse long event unsubscbribe" << endl;

      return true;
    }
  return false;
}


bool
LongActions::keyRepetitiveEventUnsubscribe( int startingKey,
                                            Qt::KeyboardModifiers startingButtonState,
                                            int endingKey,
                                            Qt::KeyboardModifiers endingButtonState )
{
  Control::KeyMapKey startKey( startingKey, startingButtonState );
  Control::KeyMapKey endKey( endingKey, endingButtonState );

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator found( myKeyRepetitiveEventMap.find( startKey ) );

  if ( found != myKeyRepetitiveEventMap.end() )
    if ( startKey == found->second->startingEvent() &&
         endKey == found->second->endingEvent() ) {
      map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator foundActive( myActiveKeyRepetitiveEvents.find( startKey));
      if( found != myActiveKeyRepetitiveEvents.end() )
        myActiveKeyRepetitiveEvents.erase( foundActive );
      delete found->second;
      myKeyRepetitiveEventMap.erase( found );
      return true;
    }
  return false;
}

bool
LongActions::submitKeyPressEvent( QKeyEvent * event )
{
//   cout << "LongActions : submitKeyPressEvent" << endl;
  if( event->modifiers() != Qt::NoModifier )
  {
    if( myActiveKeyAndMouseLongEvent != 0 )
    {
      myActiveKeyAndMouseLongEvent->executeEnd();
      myActiveKeyAndMouseLongEvent = 0;
    }
    if( myActiveMouseLongEvent != 0 )
    {
      myActiveMouseLongEvent->executeEnd( currentMouseX, currentMouseY, currentMouseGlobalX, currentMouseGlobalY );
      myActiveMouseLongEvent = 0;
//       cout << "myActiveMouseLongEvent set to 0 : key press event" << endl;
    }

    map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator
      iter;

    while( !myActiveKeyRepetitiveEvents.empty() )
    {
//         cout << "keyRep size : " << myActiveKeyRepetitiveEvents.size() << endl;
      iter = myActiveKeyRepetitiveEvents.begin();
      iter->second->executeEnd();
      myActiveKeyRepetitiveEvents.erase( iter );
    }
  }

  Control::KeyMapKey key( event->key(), event->modifiers() );

  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator
    foundLongKeyAndMouseEvent( myKeyAndMouseLongEventMap.find( key ) );
  if ( foundLongKeyAndMouseEvent != myKeyAndMouseLongEventMap.end( ) )
  {
    if( myActiveKeyAndMouseLongEvent == 0 )
    {
      myActiveKeyAndMouseLongEvent = foundLongKeyAndMouseEvent->second;
      myActiveKeyAndMouseLongEvent->executeStart();
    }

    return true;
  }

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator found = myKeyRepetitiveEventMap.find( key );
  if ( found != myKeyRepetitiveEventMap.end() )
  {
    myActiveKeyRepetitiveEvents[key] = found->second;
    found->second->executeStart();
    return true;
  }

  return false;
}

bool
LongActions::submitKeyReleaseEvent( QKeyEvent * event )
{
  // cout << "LongActions : submitKeyReleaseEvent" << event->key() << endl;
  if( event->modifiers() != Qt::NoModifier )
  {
    if( myActiveKeyAndMouseLongEvent != 0 )
    {
      myActiveKeyAndMouseLongEvent->executeEnd();
      myActiveKeyAndMouseLongEvent = 0;
    }

    if( myActiveMouseLongEvent != 0 )
    {
      myActiveMouseLongEvent->executeEnd( currentMouseX, currentMouseY,
                                          currentMouseGlobalX,
                                          currentMouseGlobalY );
      myActiveMouseLongEvent = 0;
      // cout << "myActiveMouseLongEvent set to 0 : key release" << endl;
    }

    map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator
      iter;

    while( !myActiveKeyRepetitiveEvents.empty() )
    {
      iter = myActiveKeyRepetitiveEvents.begin();
      iter->second->executeEnd();
      myActiveKeyRepetitiveEvents.erase( iter );
    }
  }

  Control::KeyMapKey key( event->key(), event->modifiers() );

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator
    iter2( myActiveKeyRepetitiveEvents.begin() ),
    last2( myActiveKeyRepetitiveEvents.end() );

  while( iter2 != last2 && iter2->second->endingEvent() != key )
  {
    ++iter2;
  }

  if (  iter2 != last2 )
  {
    iter2->second->executeEnd();
    myActiveKeyRepetitiveEvents.erase( iter2 );
  }

  if( myActiveKeyAndMouseLongEvent != 0 )
    if( myActiveKeyAndMouseLongEvent->endingEvent() == key )
    {
      myActiveKeyAndMouseLongEvent->executeEnd();
      myActiveKeyAndMouseLongEvent = 0;
      return true;
    }

  return false;
}

bool
LongActions::submitMousePressEvent( QMouseEvent * event )
{
//   cout << "LongActions : submitMousePressEvent" << endl;
//   cout << "Long Action :" << this << endl;

  Control::MouseButtonMapKey k( event->button(), event->modifiers() );
  currentMouseX = event->x();
  currentMouseY = event->y();
  currentMouseGlobalX = event->globalX();
  currentMouseGlobalY = event->globalY();

  if ( myActiveKeyAndMouseLongEvent != 0 )
  {
    myActiveKeyAndMouseLongEvent->executeEnd();
    myActiveKeyAndMouseLongEvent = 0;
  }

  if ( myActiveMouseLongEvent != 0 )
  {
//     cout << "Desactivating Mouse Long Event" << endl;
    myActiveMouseLongEvent->executeEnd( event->x(), event->y(),
                                        event->globalX(), event->globalY() );
    myActiveMouseLongEvent = 0;
//     cout << "myActiveMouseLongEvent set to 0 : mouse press" << endl;
  }

  map<Control::MouseButtonMapKey, MouseLongEvent*, Control::LessMouseMap>::iterator found( myMouseLongEventMap.find( k ) );
  if ( found != myMouseLongEventMap.end() )
  {
    myActiveMouseLongEvent = found->second;
//     cout << " Mouse Press : myActiveMouseLongEvent =" << myActiveMouseLongEvent << endl;
    myActiveMouseLongEvent->executeStart( event->x(), event->y(),
                                          event->globalX(), event->globalY() );
    event->accept();
    return true;
  }
  else
  {
//     cout << "No such long mouse event in map" << endl;
    event->ignore();
  }
  return false;
}

bool
LongActions::submitMouseReleaseEvent( QMouseEvent * event )
{
//  cout << "Long Action :" << this << endl;

//    cout << "LongActions : submitMouseReleaseEvent" << endl;
  currentMouseX = event->x();
  currentMouseY = event->y();
  currentMouseGlobalX = event->globalX();
  currentMouseGlobalY = event->globalY();

  if( myActiveKeyAndMouseLongEvent != 0 ){
    myActiveKeyAndMouseLongEvent->executeEnd();
    myActiveKeyAndMouseLongEvent = 0;
  }

//   cout << "myActiveMouseLongEvent = " << myActiveMouseLongEvent << endl;

  if ( myActiveMouseLongEvent != 0 ) {
    myActiveMouseLongEvent->executeEnd( event->x(), event->y(),
                                        event->globalX(), event->globalY() );
    myActiveMouseLongEvent = 0;
//     cout << "myActiveMouseLongEvent set to 0 : mouse release" << endl;

    return true;
  }

  return false;
}

bool
LongActions::submitMouseMoveEvent( QMouseEvent * event )
{
//  cout << "Long Action :" << this << endl;

//    cout << "LongActions : submitMouseMoveEvent" << endl;
  currentMouseX = event->x();
  currentMouseY = event->y();
  currentMouseGlobalX = event->globalX();
  currentMouseGlobalY = event->globalY();

  if( myActiveKeyAndMouseLongEvent != 0 ){
      myActiveKeyAndMouseLongEvent->executeLong( event->x(), event->y(),
                                                 event->globalX(), event->globalY() );
      return true;
    }

//   cout << "myActiveMouseLongEvent = " << myActiveMouseLongEvent << endl;

  if( myActiveMouseLongEvent != 0 ){
    myActiveMouseLongEvent->executeLong( event->x(), event->y(),
                                         event->globalX(), event->globalY() );
    return true;
  }

  return false;
}
