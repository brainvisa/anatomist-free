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


#include <anatomist/control/toolTips.h>
#include <anatomist/control/toolTips-qt.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/window2D/window2D.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window3D/glwidget3D.h>
#include <anatomist/graph/attribAObject.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qcursor.h>


QAToolTips	*QAToolTips::theToolTip = 0;


QAToolTips::QAToolTips() : QObject(), _wakeupID( 0 ), _sleepID( 0 ), 
  _currentTip( 0 )
{
  wakeUp = new QTimer( 0, "wakeUp" );
  fallAsleep = new QTimer( 0, "fallAsleep" );
  _window = 0;
  connect( wakeUp, SIGNAL( timeout() ), SLOT( showTip() ) );
  connect( fallAsleep, SIGNAL( timeout() ), SLOT( hideTip() ) );
}


QAToolTips::~QAToolTips()
{
  removeTip();
  theToolTip = 0;
  if( _wakeupID )
    XtRemoveTimeOut( _wakeupID );
  if( _sleepID )
    XtRemoveTimeOut( _sleepID );
  delete fallAsleep;
  delete wakeUp;
  delete _currentTip;
}


void QAToolTips::installToolTips( bool onoff )
{
  /*set<AWindow *>	win = theAnatomist->getWindows();
  set<AWindow *>::const_iterator	iw, fw=win.end();
  AWindow2D				*w2;
  EventMask	eventMask = EnterWindowMask | LeaveWindowMask 
    | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    Widget	wid;

  for( iw=win.begin(); iw!=fw; ++iw )
    {
      wid = 0;
      w2 = dynamic_cast<AWindow2D *>( *iw );
      if( w2 )
	{
	  wid = w2->DrawArea()->BaseWidget();
	  if( onoff )
	    XtAddEventHandler( wid, eventMask, False, xtEventHandler, *iw );
	  else
	    XtRemoveEventHandler( wid, eventMask, False, xtEventHandler, *iw );
	}
	}*/

  QAViewToolTip::toolTipGroup()->setEnabled( onoff );
}


void QAToolTips::xtEventHandler( Widget, XtPointer clientData, 
				 XEvent *event, Boolean * )
{
  if( !theToolTip )
    {
      theToolTip = new QAToolTips;
    }

  switch( event->type )
    {
    case EnterNotify:
    case ButtonRelease:
      //cout << "enter notify\n";
      theToolTip->_window 
	= dynamic_cast<AWindow2D *>( (AWindow *) clientData );
      break;
    case ButtonPress:
      theToolTip->removeTip();
    case LeaveNotify:
      //cout << "leave notify\n";
      theToolTip->_window = 0;
      break;
    case MotionNotify:
      //cout << "motion notify\n";
      theToolTip->removeTip();
      theToolTip->xtWakeUpStart( 500 );
      break;
    }
}


void QAToolTips::xtWakeUp( XtPointer, XtIntervalId * )
{
  //cout << "QAToolTips::xtWakeUp\n";
  theToolTip->_wakeupID = 0;
  if( theToolTip->drawTip() )
    theToolTip->xtSleepStart( 5000 );
}


void QAToolTips::xtFallAsleep( XtPointer, XtIntervalId * )
{
  //cout << "QAToolTips::xtFallAsleep\n";
  theToolTip->_sleepID = 0;
  theToolTip->removeTip();
}


void QAToolTips::hideTip()
{
  if( wakeUp->isActive() )
    wakeUp->stop();
  if( fallAsleep->isActive() )
  fallAsleep->stop();
  //cout << "QAToolTip::hideTip\n";
  removeTip();
}


void QAToolTips::showTip()
{
  //cout << "QAToolTip::showTip\n";
  fallAsleep->start( 2000, true );
  drawTip();
}


void QAToolTips::xtWakeUpStart( int timeout )
{
  if( _wakeupID )
    XtRemoveTimeOut( _wakeupID );
  _wakeupID = XtAppAddTimeOut( theAnatomist->AppContext(), timeout, xtWakeUp, 
			       0 );
}


void QAToolTips::xtSleepStart( int timeout )
{
  if( _sleepID )
    XtRemoveTimeOut( _sleepID );
  _sleepID = XtAppAddTimeOut( theAnatomist->AppContext(), timeout, 
			      xtFallAsleep, 0 );
}


bool QAToolTips::drawTip()
{
  /*if( !_window )
    return( false );

  QPoint	pos = QCursor::pos();
  int		x, y;
  Window	dummyw;
  Point3df	pos3;

  XTranslateCoordinates( theAnatomist->GetDisplay()->Dpy(), 
			 theAnatomist->GetDisplay()->RootW(), 
			 XtWindow( _window->DrawArea()->BaseWidget() ), 
			 pos.x(), pos.y(), &x, &y, &dummyw );
  //cout << "pos : " << (int) x << ", " << (int) y << endl;

  if( !_window->positionFromCursor( x, y, pos3 ) )
    return( false );	// not a valid position

  //cout << "point 3D : " << pos3 << endl;

  vector<float> posw = _window->getFullPosition();
  posw[0] = pos3[0];
  posw[1] = pos3[1];
  posw[2] = pos3[2];
  AObject	*obj = _window->objectAt( posw );

  if( !obj )
    return( false );	// no object there

  //cout << "obj : " << obj->name() << endl;

  if( !_currentTip )
    {
      _currentTip = new QLabel( 0, "toolTipTip", WStyle_Customize 
				| WStyle_NoBorder | WStyle_Tool );
      _currentTip->setBackgroundColor( QColor( 255, 255, 128 ) );
      _currentTip->setFrameStyle( QFrame::Raised | QFrame::Panel );
      _currentTip->setMargin( 3 );
    }

  string		text = obj->name();
  AttributedAObject	*aao = dynamic_cast<AttributedAObject *>( obj );

  if( aao )
    {
      string	label;
      if( aao->attributed()->getProperty( "label", label ) )
	text += "\nlabel : " + label;
    }

  _currentTip->setText( text.c_str() );
  _currentTip->resize( _currentTip->sizeHint() );
  QPoint	npos( pos.x() + 10, pos.y() - _currentTip->height() - 10 );
  if( npos.x() + _currentTip->width() > QApplication::desktop()->width() )
    npos.setX( pos.x() - _currentTip->width() - 10 );
  if( npos.y() < 0 )
    npos.setY( pos.y() + 20 );
  _currentTip->move( npos );
  _currentTip->show();*/
  return( true );
}


void QAToolTips::removeTip()
{
  if( _currentTip )
    _currentTip->hide();
}
