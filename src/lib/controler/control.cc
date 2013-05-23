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


#include "anatomist/controler/control_d.h"
#include "anatomist/controler/actionpool.h"
#include "anatomist/controler/view.h"
#include <qtimer.h>
#include <qlabel.h>
#include <qlayout.h>
#if QT_VERSION >= 0x040600
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPanGesture>
#include <math.h>
#endif
#include <qwidget.h>
#include <qapplication.h>
#include <iostream>
#include <typeinfo>

using namespace anatomist ;
using namespace std;

using anatomist::Action ;
using anatomist::ActionPool ;
using anatomist::Control ;
using anatomist::KeyAndMouseLongEvent ;
using anatomist::MouseLongEvent ;
using anatomist::LongActions ;


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
      if( wl.find( track.second ) != wl.end() )
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
//   show() ;
//   QLabel * controlDescription = new QLabel( tr( subject->name().c_str() ), 
// 					    this, "ControlDescription" ) ;
//   addTab(controlDescription, subject->name().c_str() ) ;
// }

// void
// ControlObserver::closeEvent( QCloseEvent * e )
// {
//   mySubject->detachObserver() ;
//   e->accept() ;
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
  myStartingAction = startingAction.clone() ;
  myLongAction = longAction.clone() ;
  myEndingAction = endingAction.clone() ;
}

KeyAndMouseLongEvent::KeyAndMouseLongEvent( const KeyAndMouseLongEvent&  event ) : myStartingEvent( event.startingEvent() ), myLongEvent( event.longEvent() ), myEndingEvent( event.endingEvent() ),
  myExclusiveAction( event.exclusiveAction() )
{
  myStartingAction = event.startingAction()->clone() ;
  myLongAction = event.longAction()->clone() ;
  myEndingAction = event.endingAction()->clone() ;
}


KeyAndMouseLongEvent::~KeyAndMouseLongEvent() 
{
  delete myStartingAction ;
  delete myLongAction ;
  delete myEndingAction ;
}


void
KeyAndMouseLongEvent::executeStart( ) 
{
  myStartingAction->execute( ) ;
}

void 
KeyAndMouseLongEvent::executeLong( int x, int y, int globalX, int globalY ) 
{
  myLongAction->execute( x, y, globalX, globalY ) ;
}

void 
KeyAndMouseLongEvent::executeEnd( ) 
{
  myEndingAction->execute( ) ;
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
    cerr << "Bad timer frequency, set to 100 milli seconds" << endl ;
    myTemporalStep = 100 ;
  }
  
  myStartingAction = startingAction.clone( ) ;
  myEndingAction = endingAction.clone( ) ;
  myTimer = new QTimer( 0 ) ;
  //Q_ASSERT ( myTimer != 0 ) ;
  myTimer->changeInterval( (int) temporalStep ) ;
  myTimer->stop();	// don't start right now...
  
  QObject::connect( myTimer, SIGNAL(timeout()), this, SLOT(timeout()) ) ;
}

KeyRepetitiveEvent::~KeyRepetitiveEvent() 
{
  delete myStartingAction ;
  delete myEndingAction ;
  delete myTimer ;
}

void 
KeyRepetitiveEvent::timeout()
{
  myStartingAction->execute() ;
  myTimer->start( (int) myTemporalStep, true ) ;
}

void 
KeyRepetitiveEvent::executeStart()
{
  myStartingAction->execute() ;
  myTimer->start( (int) myTemporalStep, true ) ;
}

void 
KeyRepetitiveEvent::executeEnd()
{
  myEndingAction->execute() ;
  myTimer->stop() ;
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
  myStartingAction = startingAction.clone() ;
  myLongAction = longAction.clone() ;
  myEndingAction = endingAction.clone() ;
}

MouseLongEvent::MouseLongEvent( const MouseLongEvent&  event ) : myStartingEvent( event.startingEvent() ), myLongEvent( event.longEvent() ), myEndingEvent( event.endingEvent() ),
  myExclusiveAction( event.exclusiveAction() )
{
  myStartingAction = event.startingAction()->clone() ;
  myLongAction = event.longAction()->clone() ;
  myEndingAction = event.endingAction()->clone() ;
}

MouseLongEvent::~MouseLongEvent() 
{
  delete myStartingAction ;
  delete myLongAction ;
  delete myEndingAction ;
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
//   cout << "MouseLongEvent : ExecuteStart" << endl ;
  myStartingAction->execute( x, y, globalX, globalY ) ;
}

void 
MouseLongEvent::executeLong( int x, int y, int globalX, int globalY ) 
{
//   cout << "MouseLongEvent : ExecuteLong" << endl ;
  myLongAction->execute( x, y, globalX, globalY ) ;
}

void 
MouseLongEvent::executeEnd( int x, int y, int globalX, int globalY ) 
{
//   cout << "MouseLongEvent : ExecuteEnd" << endl ;
  myEndingAction->execute ( x, y, globalX, globalY ) ;
}

//------------------------------------------------------------

ControlPtr 
Control::creator( int priority, const string& name )
{
  return new Control( priority, name ) ;
}

Control::Control( int priority, string name ) : 
  myPriority(priority), myUserLevel( 0 ), myName(name), myWheelAction(0), myFocusInAction(0), myFocusOutAction(0), myEnterAction(0), myLeaveAction(0), myPaintAction(0), myMoveAction(0), myResizeAction(0), myDragEnterAction(0), myDragLeaveAction(0), myDragMoveAction(0), myDropAction(0), myShowAction(0), myHideAction(0), mySelectionChangedAction(0)
{
//   cout << "NEW CONTROL" << endl ;
  myLongActions = new LongActions ;

}

Control::Control( const Control& control ) :
  myPriority(control.myPriority), myUserLevel( 0 ), myName(control.myName), myWheelAction(0), myFocusInAction(0), myFocusOutAction(0), myEnterAction(0), myLeaveAction(0), myPaintAction(0), myMoveAction(0), myResizeAction(0), myDragEnterAction(0), myDragLeaveAction(0), myDragMoveAction(0), myDropAction(0), myShowAction(0), myHideAction(0), mySelectionChangedAction(0)
{
//   cout << "NEW CONTROL : copy" << endl ;

  myLongActions = new LongActions ;
}

Control::~Control() 
{
  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator iter1(myKeyPressActionMap.begin()), last1(myKeyPressActionMap.end()) ;
  while( iter1 != last1 ){
    delete iter1->second ;
    ++iter1 ;
  }

  iter1 = myKeyReleaseActionMap.begin() ;
  last1 = myKeyReleaseActionMap.end() ;
  while( iter1 != last1 ){
    delete iter1->second ;
    ++iter1 ;
  }

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator iter2(myMousePressButtonActionMap.begin()), last2(myMousePressButtonActionMap.end()) ;
  
  while( iter2 != last2 ){
    delete iter2->second ;
    ++iter2 ;
  }  

  iter2 = myMouseReleaseButtonActionMap.begin() ;
  last2 = myMouseReleaseButtonActionMap.end() ;

  while( iter2 != last2 ){
    delete iter2->second ;
    ++iter2 ;
  }  

  iter2 = myMouseDoubleClickButtonActionMap.begin() ;
  last2 = myMouseDoubleClickButtonActionMap.end() ;

  while( iter2 != last2 ){
    delete iter2->second ;
    ++iter2 ;
  }  

  iter2 = myMouseMoveActionMap.begin() ;
  last2 = myMouseMoveActionMap.end() ;

  while( iter2 != last2 ){
    delete iter2->second ;
    ++iter2 ;
  }  

  
  delete myWheelAction ;
  delete myFocusInAction ;
  delete myFocusOutAction ;
  delete myEnterAction ;
  delete myLeaveAction ;
  delete myPaintAction ;
  delete myMoveAction ;
  delete myResizeAction ; 
  delete myDragEnterAction ;
  delete myDragLeaveAction ;
  delete myDragMoveAction ;
  delete myDropAction ; 
  delete myShowAction ;
  delete myHideAction ;
  
  delete myLongActions ;
  //delete mySurfpaintAction ;
}

void
Control::doOnSelect( ActionPool * pool )
{
  doAlsoOnSelect( pool ) ;
}

void
Control::doOnDeselect( ActionPool * pool )
{
  myLongActions->reset() ;

//   cout << "--------> Control Deselected" << endl ;
  doAlsoOnDeselect( pool ) ;
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
      return ;
    }

  KeyMapKey key( event->key(), event->state() ) ;
  
  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator 
    iter( myKeyPressActionMap.find( key ) ) ;
  if( iter != myKeyPressActionMap.end() )
    {
      iter->second->execute( ) ;
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
      return ;
    }
  
  KeyMapKey key( event->key(), event->state() ) ;
    
  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator 
    iter2( myKeyReleaseActionMap.find( key ) ) ;
  if( iter2 != myKeyReleaseActionMap.end() )
    {
      iter2->second->execute( ) ;
      event->accept();
    }
  else
    event->ignore();
}


void
Control::mousePressEvent ( QMouseEvent * event  ) 
{
//   cout << "MOUSEPRESSEVENT" << endl ;
//   cout << "Control :" << this << endl ;

  if( _mouseTracking().first > 0 )
  {
    pair<int, QWidget *> & track = _mouseTracking();
    --track.first;
    if( track.first == 0 )
      _removeMouseTracking();
  }

  if ( myLongActions->submitMousePressEvent( event ) )
    return ;
  
  MouseButtonMapKey k( event->button(), event->state() ) ;
  
  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator 
    iter( myMousePressButtonActionMap.find( k ) ) ;
  if( iter != myMousePressButtonActionMap.end() )
    iter->second->execute( event->x(), event->y(), 
			   event->globalX(), event->globalY() ) ;  

}

void
Control::mouseReleaseEvent ( QMouseEvent * event  ) 
{
//   cout << "MOUSERELEASEEVENT" << endl ;
//   cout << "Control :" << this << endl ;

  if( _mouseTracking().first )
    return;

  if( myLongActions->submitMouseReleaseEvent( event ) )
    return ;

  MouseButtonMapKey k( event->button(), event->state() ) ;
  
  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator 
    iter( myMouseReleaseButtonActionMap.find( k ) ) ;
  if( iter != myMouseReleaseButtonActionMap.end() )
    iter->second->execute( event->x(), event->y(), 
                          event->globalX(), event->globalY() ) ;  
}

void 
Control::mouseDoubleClickEvent ( QMouseEvent * event  ) 
{
//   cout << "MOUSEDOUBLE CLICK" << endl ;
  MouseButtonMapKey k( event->button(), event->state() ) ;

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator 
    iter( myMouseDoubleClickButtonActionMap.find( k ) ) ;
  if( iter != myMouseDoubleClickButtonActionMap.end() )
    iter->second->execute( event->x(), event->y(), 
                          event->globalX(), event->globalY() ) ;
  else
  {
    if( myLongActions->submitMousePressEvent( event ) )
      myLongActions->setMouseTracking( true );
  }
}

void 
Control::mouseMoveEvent ( QMouseEvent * event  ) 
{
//   cout << "MOUSEMOVEEVENT" << endl ;
//   cout << "Control :" << this << endl ;
  if( myLongActions->submitMouseMoveEvent( event ) ) 
    return ;

  MouseButtonMapKey k( event->button(), event->state() ) ;
  
  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator 
    iter( myMouseMoveActionMap.find( k ) ) ;
  if( iter != myMouseMoveActionMap.end() )
    iter->second->execute( event->x(), event->y(), 
                          event->globalX(), event->globalY() ) ;  
}

void 
Control::wheelEvent ( QWheelEvent * event  ) 
{
  if( myWheelAction )
    myWheelAction->execute( event->delta(), event->x(), event->y(), 
                            event->globalX(), event->globalY() ) ;
//   else cout << "no wheel action\n";
}

void 
Control::focusInEvent (   ) 
{
  if( myFocusInAction )
    myFocusInAction->execute( ) ;  
  
}

void 
Control::focusOutEvent (  ) 
{
  if( myFocusOutAction )
    myFocusOutAction->execute( ) ;    
}

void 
Control::enterEvent ( ) 
{
  if( myEnterAction )
    myEnterAction->execute( ) ;    
}

void 
Control::leaveEvent (  ) 
{
  if( myLeaveAction )
    myLeaveAction->execute( ) ;      
}

void 
Control::paintEvent ( QPaintEvent * event  ) 
{
  if( myPaintAction )
        myPaintAction->execute( event->rect().left(), event->rect().top(), 
                               event->rect().height(), event->rect().width() ) ;
}

void 
Control::moveEvent ( QMoveEvent * event  ) 
{
  if( myMoveAction )
    myMoveAction->execute( event->pos().x(), event->pos().y(), 
                          event->oldPos().x(), event->oldPos().y() ) ;
}

void 
Control::resizeEvent ( QResizeEvent * event  ) 
{
  if( myResizeAction )
    myResizeAction->execute( event->size().width(), event->size().height(), 
                            event->oldSize().width(), event->oldSize().height() ) ;
}

void 
Control::dragEnterEvent (   ) 
{
  if( myDragEnterAction )
    myDragEnterAction->execute( ) ;
}

void 
Control::dragMoveEvent (  ) 
{
  if( myDragMoveAction )
    myDragMoveAction->execute( ) ;
}

void 
Control::dragLeaveEvent ( ) 
{
  if( myDragLeaveAction )
    myDragLeaveAction->execute( ) ;
}

void 
Control::dropEvent ( QDropEvent * event  ) 
{
  if( myDropAction )
    myDropAction->execute( event->pos().x(), event->pos().y(), 
                           event->dropAction() ) ;
}

void 
Control::showEvent ( QShowEvent * event  ) 
{
  if( myShowAction )
    myShowAction->execute( event->spontaneous() ) ;
}

void 
Control::hideEvent ( QHideEvent * event  ) 
{
  if( myHideAction )
    myHideAction->execute( event->spontaneous() ) ;
}

void Control::selectionChangedEvent()
{
  if( mySelectionChangedAction )
    mySelectionChangedAction->execute();
}


#if QT_VERSION >= 0x040600
void Control::gestureEvent( QGestureEvent * event )
{
  cout << "Gesture event\n";
  if( QGesture *pinch = event->gesture(Qt::PinchGesture ) )
  {
    event->setAccepted( pinch, pinchGesture(
      static_cast<QPinchGesture *>( pinch ) ) );
  }
  if( QGesture *pan = event->gesture( Qt::PanGesture ) )
  {
    event->setAccepted( pan, panGesture( static_cast<QPanGesture *>( pan ) ) );
  }
}


bool Control::pinchGesture( QPinchGesture * gesture )
{
  if( gesture->state() == Qt::GestureStarted )
  {
    // for now, simulate corresponding mouse events
    QMouseEvent ev( QEvent::MouseButtonPress, QPoint( 0, 0 ),
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ),
                    Qt::MidButton,
                    Qt::ShiftModifier );
    mousePressEvent( &ev );
  }
  else if( gesture->state() == Qt::GestureUpdated )
  {
    // for now, simulate corresponding mouse events
    QPoint p = QPoint( 0,
                       - (int)( 100 * log( gesture->totalScaleFactor() ) ) );
    QMouseEvent ev( QEvent::MouseMove, p,
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ) + p,
                    Qt::MidButton,
                    Qt::ShiftModifier );
    mouseMoveEvent( &ev );
  }
  else if( gesture->state() == Qt::GestureCanceled
    || gesture->state() == Qt::GestureFinished )
  {
    // for now, simulate corresponding mouse events
    QPoint p = QPoint( 0,
                       - (int)( 100 * log( gesture->totalScaleFactor() ) ) );
    QMouseEvent ev( QEvent::MouseButtonRelease, p,
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ) + p,
                    Qt::MidButton,
                    Qt::ShiftModifier );
    mouseReleaseEvent( &ev );
  }
  return true;
}


bool Control::panGesture( QPanGesture * gesture )
{
  if( gesture->state() == Qt::GestureStarted )
  {
    // for now, simulate corresponding mouse events
    QMouseEvent ev( QEvent::MouseButtonPress, QPoint( 0, 0 ),
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ),
                    Qt::MidButton,
                    0 /*TODO: get actual current modifiers */ );
    mousePressEvent( &ev );
  }
  else if( gesture->state() == Qt::GestureUpdated )
  {
    // for now, simulate corresponding mouse events
    QMouseEvent ev( QEvent::MouseMove,
                    QPoint( (int) gesture->offset().rx(),
                            (int) gesture->offset().ry() ),
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ),
                    Qt::MidButton,
                    0 /*TODO: get actual current modifiers */ );
    mouseMoveEvent( &ev );
  }
  else if( gesture->state() == Qt::GestureCanceled
    || gesture->state() == Qt::GestureFinished )
  {
    // for now, simulate corresponding mouse events
    QMouseEvent ev( QEvent::MouseButtonRelease,
                    QPoint( (int) gesture->offset().rx(),
                            (int) gesture->offset().ry() ),
                    QPoint( (int) gesture->hotSpot().rx(),
                            (int) gesture->hotSpot().rx() ),
                    Qt::MidButton,
                    0 /*TODO: get actual current modifiers */ );
    mouseReleaseEvent( &ev );
  }
  return true;
}
#endif // Qt >= 4.6


bool 
Control::keyPressEventSubscribe( int key, 
                                 Qt::KeyboardModifiers buttonState,
                                 const KeyActionLink& actionMethod)
{
  KeyMapKey k(key, buttonState) ;  

  if ( myKeyPressActionMap.find(k) != myKeyPressActionMap.end() ){
    return false ;
  }
  
  myKeyPressActionMap.insert( pair<KeyMapKey,KeyActionLink* >( k, actionMethod.clone() ) ) ;

  return true ;
}

bool 
Control::keyReleaseEventSubscribe( int key, 
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethod)
{
  KeyMapKey k(key, buttonState) ;  

  if ( myKeyReleaseActionMap.find(k) != myKeyReleaseActionMap.end() ){
    return false ;
  }
  
  myKeyReleaseActionMap.insert( pair<KeyMapKey,KeyActionLink* >( k, actionMethod.clone() ) ) ;

  return true ;
}


bool 
Control::mousePressButtonEventSubscribe( Qt::MouseButtons button,
                                         Qt::KeyboardModifiers state,
                                         const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state) ;  

  if ( myMousePressButtonActionMap.find(k) != myMousePressButtonActionMap.end() ){
    return false ;
  }
  
  myMousePressButtonActionMap.insert( pair<MouseButtonMapKey,MouseActionLink* >( k, actionMethod.clone() ) ) ;

  return true ;
}

bool 
Control::mouseReleaseButtonEventSubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state) ;  

  if ( myMouseReleaseButtonActionMap.find(k) != myMouseReleaseButtonActionMap.end() ){
    return false ;
  }
  
  myMouseReleaseButtonActionMap.insert( pair<MouseButtonMapKey,MouseActionLink* >( k, actionMethod.clone() ) ) ;
  
  return true ;
}

bool 
Control::mouseDoubleClickEventSubscribe( Qt::MouseButtons button,
                                        Qt::KeyboardModifiers state,
                                        const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state) ;  

  if ( myMouseDoubleClickButtonActionMap.find(k) != myMouseDoubleClickButtonActionMap.end() ){
    return false ;
  }
  
  myMouseDoubleClickButtonActionMap.insert( pair<MouseButtonMapKey,MouseActionLink* >( k, actionMethod.clone() ) ) ;
  
  return true ;
}

bool 
Control::mouseMoveEventSubscribe( Qt::MouseButtons button,
                                  Qt::KeyboardModifiers state,
                                  const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state) ;  

  if ( myMouseMoveActionMap.find(k) != myMouseMoveActionMap.end() ){
    return false ;
  }
  
  myMouseMoveActionMap.insert( pair<MouseButtonMapKey,MouseActionLink* >( k, actionMethod.clone() ) ) ;
  
  return true ;
}

bool 
Control::keyAndMouseLongEventSubscribe( int startingKey, 
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
						       exclusiveAction ) ;
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
						 exclusiveAction ) ;
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
						     temporalStep ) ;
}


bool 
Control::wheelEventSubscribe( const WheelActionLink& actionMethod )
{
  if( myWheelAction != 0 )
    return false ;
  myWheelAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::focusInEventSubscribe( const FocusActionLink& actionMethod )
{
  if( myFocusInAction != 0 )
    return false ;
  myFocusInAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::focusOutEventSubscribe( const FocusActionLink& actionMethod )
{
  if( myFocusOutAction != 0 )
    return false ;
  myFocusOutAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::enterEventSubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( myEnterAction != 0 )
    return false ;
  myEnterAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::leaveEventSubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( myLeaveAction != 0 )
    return false ;
  myLeaveAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::paintEventSubscribe( const PaintActionLink& actionMethod )
{
  if( myPaintAction != 0 )
    return false ;
  myPaintAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::moveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myMoveAction != 0 )
    return false ;
  myMoveAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::resizeEventSubscribe( const ResizeActionLink& actionMethod )
{
  if( myResizeAction != 0 )
    return false ;
  myResizeAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::dragEnterEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragEnterAction != 0 )
       return false ;
  myDragEnterAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::dragLeaveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragLeaveAction != 0 )
       return false ;
  myDragLeaveAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::dragMoveEventSubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( myDragMoveAction != 0 )
       return false ;
  myDragMoveAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::dropEventSubscribe( const DropActionLink& actionMethod )
{
  if( myDropAction != 0 )
       return false ;
  myDropAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::showEventSubscribe( const ShowHideActionLink& actionMethod )
{
  if( myShowAction != 0 )
       return false ;
  myShowAction = actionMethod.clone() ;
  return true ;
}

bool 
Control::hideEventSubscribe( const ShowHideActionLink& actionMethod )
{
  if( myHideAction != 0 )
       return false ;
  myHideAction = actionMethod.clone() ;
  return true ;
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


bool
Control::keyPressEventUnsubscribe( int key, 
                                   Qt::KeyboardModifiers buttonState,
                                   const KeyActionLink& actionMethod )
{
  KeyMapKey k(key, buttonState ) ;

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator found( myKeyPressActionMap.find(k) ) ;
  
  if ( found != myKeyPressActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myKeyPressActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}

bool
Control::keyReleaseEventUnsubscribe( int key, 
                                     Qt::KeyboardModifiers buttonState,
                                     const KeyActionLink& actionMethod )
{
  KeyMapKey k(key, buttonState ) ;

  map<KeyMapKey, KeyActionLink*, LessKeyMap>::iterator found( myKeyPressActionMap.find(k) ) ;
  
  if ( found != myKeyReleaseActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myKeyReleaseActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}
  
bool  
Control::mousePressButtonEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state ) ;

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator found( myMousePressButtonActionMap.find(k) ) ;
  
  if ( found != myMousePressButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myMousePressButtonActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}

bool  
Control::mouseReleaseButtonEventUnsubscribe( Qt::MouseButtons button,
                                             Qt::KeyboardModifiers state,
                                             const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state ) ;

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator found( myMouseReleaseButtonActionMap.find(k) ) ;
  
  if ( found != myMouseReleaseButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myMouseReleaseButtonActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}
    
bool  
Control::mouseDoubleClickEventUnsubscribe( Qt::MouseButtons button,
                                           Qt::KeyboardModifiers state,
                                           const MouseActionLink
                                           & actionMethod )
{
  MouseButtonMapKey k(button, state ) ;

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator found( myMouseDoubleClickButtonActionMap.find(k) ) ;
  
  if ( found != myMouseDoubleClickButtonActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myMouseDoubleClickButtonActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}

bool  
Control::mouseMoveEventUnsubscribe( Qt::MouseButtons button,
                                    Qt::KeyboardModifiers state,
                                    const MouseActionLink& actionMethod )
{
  MouseButtonMapKey k(button, state ) ;

  map<MouseButtonMapKey, MouseActionLink*, LessMouseMap>::iterator found( myMouseMoveActionMap.find(k) ) ;
  
  if ( found != myMouseMoveActionMap.end() )
    if ( typeid(found->second) == typeid( actionMethod ) ) {
      delete found->second ;
      myMouseMoveActionMap.erase( found ) ;
      return true ;
    } 
  return false ;  
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
							 endingButtonState ) ;
}

bool 
Control::mouseLongEventUnsubscribe( Qt::MouseButtons startingButton, 
				    Qt::KeyboardModifiers startingButtonState )
{
  return myLongActions->mouseLongEventUnsubscribe( startingButton, 
						   startingButtonState ) ;
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
						       endingButtonState ) ;
}

bool  
Control::wheelEventUnsubscribe( const WheelActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myWheelAction ) )
    return false ;
  else {
    delete myWheelAction ;
    myWheelAction = 0 ;
  }
  return true ;
}

bool  
Control::wheelEventUnsubscribeAll( )
{
  if( myWheelAction == 0 )
    return false ;
  else {
    delete myWheelAction ;
    myWheelAction = 0 ;
  }
  return true ;
}

bool  
Control::focusInEventUnsubscribe( const FocusActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myFocusInAction ) )
    return false ;
  else {
    delete myFocusInAction ;
    myFocusInAction = 0 ;
  }
  return true ;
}

bool  
Control::focusOutEventUnsubscribe( const FocusActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myFocusOutAction ) )
    return false ;
  else {
    delete myFocusOutAction ;
    myFocusOutAction = 0 ;
  }
  return true ;
}

bool  
Control::enterEventUnsubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myEnterAction ) )
    return false ;
  else {
    delete myEnterAction ;
    myEnterAction = 0 ;
  }
  return true ;
}

bool  
Control::leaveEventUnsubscribe( const EnterLeaveActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myLeaveAction ) )
    return false ;
  else {
    delete myLeaveAction ;
    myLeaveAction = 0 ;
  }
  return true ;
}

bool  
Control::paintEventUnsubscribe( const PaintActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myPaintAction ) )
    return false ;
  else {
    delete myPaintAction ;
    myPaintAction = 0 ;
  }
  return true ;
}

bool  
Control::moveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myMoveAction ) )
    return false ;
  else {
    delete myMoveAction ;
    myMoveAction = 0 ;
  }
  return true ;
}

bool  
Control::resizeEventUnsubscribe( const ResizeActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myResizeAction ) )
    return false ;
  else {
    delete myResizeAction ;
    myResizeAction = 0 ;
  }
  return true ;
}

bool  
Control::dragEnterEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragEnterAction ) )
    return false ;
  else {
    delete myDragEnterAction ;
    myDragEnterAction = 0 ;
  }
  return true ;
}

bool  
Control::dragLeaveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragLeaveAction ) )
    return false ;
  else {
    delete myDragLeaveAction ;
    myDragLeaveAction = 0 ;
  }
  return true ;
}

bool  
Control::dragMoveEventUnsubscribe( const MoveOrDragActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDragMoveAction ) )
    return false ;
  else {
    delete myDragMoveAction ;
    myDragMoveAction = 0 ;
  }
  return true ;
}

bool  
Control::dropEventUnsubscribe( const DropActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myDropAction ) )
    return false ;
  else {
    delete myDropAction ;
    myDropAction = 0 ;
  }
  return true ;
}

bool  
Control::showEventUnsubscribe( const ShowHideActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myShowAction ) )
    return false ;
  else {
    delete myShowAction ;
    myShowAction = 0 ;
  }
  return true ;
}

bool  
Control::hideEventUnsubscribe( const ShowHideActionLink& actionMethod )
{
  if( typeid( actionMethod ) != typeid( myHideAction ) )
    return false ;
  else {
    delete myHideAction ;
    myHideAction = 0 ;
  }
  return true ;
}

LongActions::LongActions() : myActiveKeyAndMouseLongEvent(0), myActiveMouseLongEvent(0)
{
//   cout << "NEW LONGACTION" << endl ;

}

LongActions::~LongActions()
{
  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator iter3(myKeyAndMouseLongEventMap.begin()), last3(myKeyAndMouseLongEventMap.end()) ;

  while( iter3 != last3 ){
    delete iter3->second ;
    ++iter3 ;
  }  

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator iter4(myKeyRepetitiveEventMap.begin()), last4(myKeyRepetitiveEventMap.end()) ;

  while( iter4 != last4 ){
    delete iter4->second ;
    ++iter4 ;
  }  
  
  map<Control::MouseButtonMapKey, MouseLongEvent*, 
    Control::LessMouseMap>::iterator iter5(myMouseLongEventMap.begin()), 
    last5(myMouseLongEventMap.end()) ;

  while( iter5 != last5 ){
    delete iter5->second ;
    ++iter5 ;
  }    
}

void
LongActions::reset()
{
  myActiveKeyRepetitiveEvents.clear() ;
  myActiveKeyAndMouseLongEvent = 0 ;
  myActiveMouseLongEvent = 0 ;

//   cout << "myActiveMouseLongEvent set to 0 : reset" << endl ;

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
  Control::KeyMapKey startKey( startingKey, startingButtonState ) ;
  Control::MouseButtonMapKey longKey( longButton,longState ) ;
  Control::KeyMapKey endKey( endingKey, endingButtonState ) ;

  if ( myKeyAndMouseLongEventMap.find(startKey) != myKeyAndMouseLongEventMap.end() ) 
    return false ;

  KeyAndMouseLongEvent * longEvent = 
    new KeyAndMouseLongEvent( startKey, 
                              startingActionMethod,
                              longKey,
                              longActionMethod,
                              endKey,
                              endingActionMethod,
                              exclusiveAction ) ;
  
  myKeyAndMouseLongEventMap.insert( pair< Control::KeyMapKey, KeyAndMouseLongEvent* >( startKey, longEvent ) ) ;
  
  return true ;  
}

bool
LongActions::mouseLongEventSubscribe( Qt::MouseButtons startingButton, 
				     Qt::KeyboardModifiers startingButtonState, 
				     const Control::MouseActionLink& startingActionMethod,
				     const Control::MouseActionLink& longActionMethod,
				     const Control::MouseActionLink& endingActionMethod,
				     bool exclusiveAction )
{
  Control::MouseButtonMapKey startKey( startingButton, startingButtonState ) ;

  if ( myMouseLongEventMap.find(startKey) != myMouseLongEventMap.end() ) 
    return false ;

  MouseLongEvent * longEvent = 
    new MouseLongEvent( startKey, 
			startingActionMethod,
			startKey,
			longActionMethod,
			startKey,
			endingActionMethod,
			exclusiveAction ) ;
  
  myMouseLongEventMap.insert( pair< Control::MouseButtonMapKey, MouseLongEvent* >( startKey, 
										 longEvent ) ) ;
  
  return true ;
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
  Control::KeyMapKey startKey( startingKey, startingButtonState ) ;
  Control::KeyMapKey endKey( endingKey, endingButtonState ) ;

  if ( myKeyRepetitiveEventMap.find(startKey) != myKeyRepetitiveEventMap.end() ) 
    return false ;

  KeyRepetitiveEvent * repetitiveEvent = 
    new KeyRepetitiveEvent( startKey, 
                            startingActionMethod,
                            endKey,
                            endingActionMethod,
                            exclusiveAction,
                            temporalStep ) ;
  
  myKeyRepetitiveEventMap.insert( pair< Control::KeyMapKey, KeyRepetitiveEvent* >( startKey, repetitiveEvent ) ) ;
  
  return true ;    
}

bool 
LongActions::keyAndMouseLongEventUnsubscribe( int startingKey, 
                                          Qt::KeyboardModifiers startingButtonState, 
                                          Qt::MouseButtons longButton,
                                          Qt::KeyboardModifiers longState,
                                          int endingKey, 
                                          Qt::KeyboardModifiers endingButtonState ) 
{
  Control::KeyMapKey startKey( startingKey, startingButtonState ) ;
  Control::MouseButtonMapKey longKey( longButton,longState ) ;
  Control::KeyMapKey endKey( endingKey, endingButtonState ) ;

  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator found( myKeyAndMouseLongEventMap.find( startKey ) ) ;

  if ( found != myKeyAndMouseLongEventMap.end() ) 
    if ( startKey == found->second->startingEvent() &&
         longKey == found->second->longEvent() &&
         endKey == found->second->endingEvent() ) {
      delete found->second ;
      myKeyAndMouseLongEventMap.erase( found ) ;
      myActiveKeyAndMouseLongEvent = 0 ;
      return true ;
    } 
  return false ;
}

bool 
LongActions::mouseLongEventUnsubscribe( Qt::MouseButtons startingButton, 
				    Qt::KeyboardModifiers startingButtonState )
{
  Control::MouseButtonMapKey startKey( startingButton, startingButtonState ) ;

  map<Control::MouseButtonMapKey, MouseLongEvent*, Control::LessMouseMap>::iterator found( myMouseLongEventMap.find( startKey ) ) ;

  if ( found != myMouseLongEventMap.end() ) 
    if ( startKey == found->second->startingEvent() ) {
      delete found->second ;
      myMouseLongEventMap.erase( found ) ;
      myActiveMouseLongEvent = 0 ;
//       cout << "myActiveMouseLongEvent set to 0 : mouse long event unsubscbribe" << endl ;

      return true ;
    } 
  return false ;
}


bool 
LongActions::keyRepetitiveEventUnsubscribe( int startingKey, 
					    Qt::KeyboardModifiers startingButtonState, 
					    int endingKey, 
					    Qt::KeyboardModifiers endingButtonState ) 
{
  Control::KeyMapKey startKey( startingKey, startingButtonState ) ;
  Control::KeyMapKey endKey( endingKey, endingButtonState ) ;

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator found( myKeyRepetitiveEventMap.find( startKey ) ) ;

  if ( found != myKeyRepetitiveEventMap.end() ) 
    if ( startKey == found->second->startingEvent() &&
         endKey == found->second->endingEvent() ) {
      map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator foundActive( myActiveKeyRepetitiveEvents.find( startKey));
      if( found != myActiveKeyRepetitiveEvents.end() )
        myActiveKeyRepetitiveEvents.erase( foundActive ) ;
      delete found->second ;
      myKeyRepetitiveEventMap.erase( found ) ;
      return true ;
    } 
  return false ;  
}

bool 
LongActions::submitKeyPressEvent( QKeyEvent * event ) 
{
//   cout << "LongActions : submitKeyPressEvent" << endl ;
  if( event->state() != Qt::NoButton ){
    if( myActiveKeyAndMouseLongEvent != 0 ){
      myActiveKeyAndMouseLongEvent->executeEnd() ;
      myActiveKeyAndMouseLongEvent = 0 ;
    }
    if( myActiveMouseLongEvent != 0 ){
      myActiveMouseLongEvent->executeEnd( currentMouseX, currentMouseY, currentMouseGlobalX, currentMouseGlobalY ) ;
      myActiveMouseLongEvent = 0 ;
//       cout << "myActiveMouseLongEvent set to 0 : key press event" << endl ;
    }
    
    map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator
      iter;

    while( !myActiveKeyRepetitiveEvents.empty() )
      {
// 	cout << "keyRep size : " << myActiveKeyRepetitiveEvents.size() << endl;
        iter = myActiveKeyRepetitiveEvents.begin();
        iter->second->executeEnd() ;
        myActiveKeyRepetitiveEvents.erase( iter ) ;
      }
  }
  
  Control::KeyMapKey key( event->key(), event->state() ) ;
  
  map<Control::KeyMapKey, KeyAndMouseLongEvent*, Control::LessKeyMap>::iterator 
    foundLongKeyAndMouseEvent( myKeyAndMouseLongEventMap.find( key ) ) ;
  if ( foundLongKeyAndMouseEvent != myKeyAndMouseLongEventMap.end( ) ){ 
    if( myActiveKeyAndMouseLongEvent == 0 ){
      myActiveKeyAndMouseLongEvent = foundLongKeyAndMouseEvent->second ;
      myActiveKeyAndMouseLongEvent->executeStart() ;
    }

    return true ;
  }  
  
  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator found = myKeyRepetitiveEventMap.find( key ) ;
  if ( found != myKeyRepetitiveEventMap.end() ){
    myActiveKeyRepetitiveEvents[key] = found->second ;
    found->second->executeStart() ;
    return true ;
  }
  
  return false ;
}

bool 
LongActions::submitKeyReleaseEvent( QKeyEvent * event ) 
{
  // cout << "LongActions : submitKeyReleaseEvent" << event->key() << endl ;
  if( event->state() != Qt::NoButton ){
    if( myActiveKeyAndMouseLongEvent != 0 ){
      myActiveKeyAndMouseLongEvent->executeEnd() ;
      myActiveKeyAndMouseLongEvent = 0 ;
    }

    if( myActiveMouseLongEvent != 0 ){
      myActiveMouseLongEvent->executeEnd( currentMouseX, currentMouseY, 
                                          currentMouseGlobalX, 
                                          currentMouseGlobalY ) ;
      myActiveMouseLongEvent = 0 ;
      // cout << "myActiveMouseLongEvent set to 0 : key release" << endl ;
	  
    }
  
    map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator
      iter;

    while( !myActiveKeyRepetitiveEvents.empty() )
      {
        iter = myActiveKeyRepetitiveEvents.begin();
        iter->second->executeEnd() ;
        myActiveKeyRepetitiveEvents.erase( iter ) ;
      }
  }

  Control::KeyMapKey key( event->key(), event->state() ) ;

  map<Control::KeyMapKey, KeyRepetitiveEvent*, Control::LessKeyMap>::iterator 
    iter2( myActiveKeyRepetitiveEvents.begin() ), 
    last2( myActiveKeyRepetitiveEvents.end() ) ;
  
  while( iter2 != last2 && iter2->second->endingEvent() != key ){
    ++iter2 ;
  }
  
  if (  iter2 != last2 )
    {
      iter2->second->executeEnd() ;
      myActiveKeyRepetitiveEvents.erase( iter2 ) ;
    }

  if( myActiveKeyAndMouseLongEvent != 0 )
    if( myActiveKeyAndMouseLongEvent->endingEvent() == key ){
      myActiveKeyAndMouseLongEvent->executeEnd() ;
      myActiveKeyAndMouseLongEvent = 0 ;
      return true ;
    } 
  
  return false ;
}

bool 
LongActions::submitMousePressEvent( QMouseEvent * event ) 
{
//   cout << "LongActions : submitMousePressEvent" << endl ;

//   cout << "Long Action :" << this << endl ;

  Control::MouseButtonMapKey k( event->button(), event->state() ) ;
  currentMouseX = event->x() ;
  currentMouseY = event->y() ;
  currentMouseGlobalX = event->globalX() ;
  currentMouseGlobalY = event->globalY() ;

  if ( myActiveKeyAndMouseLongEvent != 0 ){
    myActiveKeyAndMouseLongEvent->executeEnd() ;
    myActiveKeyAndMouseLongEvent = 0 ;
  }

  if ( myActiveMouseLongEvent != 0 ){
//     cout << "Desactivating Mouse Long Event" << endl ;
    myActiveMouseLongEvent->executeEnd( event->x(), event->y(), 
					event->globalX(), event->globalY() ) ;
    myActiveMouseLongEvent = 0 ;
//     cout << "myActiveMouseLongEvent set to 0 : mouse press" << endl ;
  }

  map<Control::MouseButtonMapKey, MouseLongEvent*, Control::LessMouseMap>::iterator found( myMouseLongEventMap.find( k ) ) ;
  if ( found != myMouseLongEventMap.end() ) {
    myActiveMouseLongEvent = found->second ;
//     cout << " Mouse Press : myActiveMouseLongEvent =" << myActiveMouseLongEvent << endl ;
    myActiveMouseLongEvent->executeStart( event->x(), event->y(), 
					  event->globalX(), event->globalY() ) ;
    return true ;
  }else{
//     cout << "No such long mouse event in map" << endl ;
  }
  return false ;
}

bool 
LongActions::submitMouseReleaseEvent( QMouseEvent * event ) 
{
//  cout << "Long Action :" << this << endl ;

//    cout << "LongActions : submitMouseReleaseEvent" << endl ;
  currentMouseX = event->x() ;
  currentMouseY = event->y() ;
  currentMouseGlobalX = event->globalX() ;
  currentMouseGlobalY = event->globalY() ;

  if( myActiveKeyAndMouseLongEvent != 0 ){
    myActiveKeyAndMouseLongEvent->executeEnd() ;
    myActiveKeyAndMouseLongEvent = 0 ;
  }

//   cout << "myActiveMouseLongEvent = " << myActiveMouseLongEvent << endl ;

  if ( myActiveMouseLongEvent != 0 ) {
    myActiveMouseLongEvent->executeEnd( event->x(), event->y(), 
					event->globalX(), event->globalY() ) ;
    myActiveMouseLongEvent = 0 ;
//     cout << "myActiveMouseLongEvent set to 0 : mouse release" << endl ;

    return true ;
  }

  return false ;
}

bool 
LongActions::submitMouseMoveEvent( QMouseEvent * event ) 
{
//  cout << "Long Action :" << this << endl ;

//    cout << "LongActions : submitMouseMoveEvent" << endl ;
  currentMouseX = event->x() ;
  currentMouseY = event->y() ;
  currentMouseGlobalX = event->globalX() ;
  currentMouseGlobalY = event->globalY() ;
  
  if( myActiveKeyAndMouseLongEvent != 0 ){
      myActiveKeyAndMouseLongEvent->executeLong( event->x(), event->y(), 
                                                 event->globalX(), event->globalY() ) ;
      return true ;
    }   

//   cout << "myActiveMouseLongEvent = " << myActiveMouseLongEvent << endl ;
  
  if( myActiveMouseLongEvent != 0 ){
    myActiveMouseLongEvent->executeLong( event->x(), event->y(), 
					 event->globalX(), event->globalY() ) ;
    return true ;
  }
  
  return false ;
}
