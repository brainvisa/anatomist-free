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


#include <anatomist/control/toolTips-qt.h>
#include <anatomist/window/Window.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/graph/attribAObject.h>
#include <cartobase/object/attributed.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qdesktopwidget.h>

using namespace anatomist;
using namespace carto;
using namespace std;


/*
QToolTipGroup	*QAViewToolTip::_tipGroup = 0;


QToolTipGroup* QAViewToolTip::toolTipGroup()
{
  if( !_tipGroup )
    _tipGroup = new QToolTipGroup( 0, "view tooltip group" );
  return( _tipGroup );
}
*/


struct QAViewToolTip::Private
{
  Private( AWindow* w, QWidget* wid ) 
    : window( w ), widget( wid ), sleept( 0 ), waket( 0 ), enabled( true )
  {}
  ~Private();

  AWindow	*window;
  QWidget	*widget;
  QTimer	*sleept;
  QTimer	*waket;
  bool		enabled;
};


QAViewToolTip::Private::~Private()
{
  delete sleept;
  delete waket;
}


namespace
{

  struct QAViewToolTip_static
  {
    QAViewToolTip_static() : enabled( true ), currenttip( 0 ) {}

    bool	 enabled;
    QLabel	*currenttip;
  };


  QAViewToolTip_static & _viewToolTip_static()
  {
    static QAViewToolTip_static	x;
    return x;
  }

}


QAViewToolTip::QAViewToolTip( AWindow* w, QWidget *parent )
  : QObject( parent ) /*QToolTip( parent, toolTipGroup() )*/, 
    d( new Private( w, parent ) )
{
  parent->installEventFilter( this );
  parent->setMouseTracking( true );
}


QAViewToolTip::~QAViewToolTip()
{
  delete d;
  delete _viewToolTip_static().currenttip;
  _viewToolTip_static().currenttip = 0;
}


namespace
{

  QString printobj( const AObject* obj, const vector<string> & todisp )
  {
    QString	text = ( string( "<b>" ) + obj->name() + "</b>" ).c_str();
    const AObject::ParentList	& pl = obj->parents();
    if( !pl.empty() )
      {
        text += " (";
        AObject::ParentList::const_iterator	ip, ep = pl.end();
        bool				first = true;
        for( ip=pl.begin(); ip!=ep; ++ip )
          {
            if( first )
              first = false;
            else
              text += ", ";
            text += (*ip)->name().c_str();
          }
        text += ")";
      }

    const AttributedAObject 
      *aao = dynamic_cast<const AttributedAObject *>( obj );
    string	proptxt;
    Object	prop;

    if( aao )
      {
        int				len = 0, x;
        vector<string>::const_iterator	ai, ae = todisp.end();
        for( ai=todisp.begin(); ai!=ae; ++ai )
          {
            x = ai->length();
            if( x > len )
              len = x;
          }

        for( ai=todisp.begin(); ai!=ae; ++ai )
          try
            {
              prop = aao->attributed()->getProperty( *ai );
              if( prop )
                {
                  proptxt = prop->getString();
                  text += QString( "<br>&nbsp;&nbsp;<em>" ) + ai->c_str() 
                    + "</em>";
                  for( x=0; x<len; ++x )
                    text += "&nbsp;";
                  text += ":&nbsp;";
                  text += proptxt.c_str();
                }
            }
          catch( ... )
            {
            }
      }
    return text;
  }

}


void QAViewToolTip::maybeTip( const QPoint & pos )
{
  Point3df	pos3;

  if( !d->window->positionFromCursor( pos.x(), pos.y(), pos3 ) )
    return;	// not a valid position

  //cout << "point 3D : " << pos3 << endl;

  set<AObject *>	shown, hidden;
  d->window->findObjectsAt( pos3[0], pos3[1], pos3[2], d->window->GetTime(), 
                            shown, hidden );
  set<AObject *>::iterator      io = shown.begin(), jo, eo = shown.end();
  // filter out temporary objects
  while( io != eo )
  {
    if( d->window->isTemporary( *io ) )
    {
      jo = io;
      ++io;
      shown.erase( jo );
    }
    else
      ++io;
  }
  io = hidden.begin();
  eo = hidden.end();
  while( io != eo )
  {
    if( d->window->isTemporary( *io ) )
    {
      jo = io;
      ++io;
      hidden.erase( jo );
    }
    else
      ++io;
  }

  if( shown.empty() && hidden.empty() )
    return;

  vector<string>		todisp(2);
  todisp[0] = "name";
  todisp[1] = "label";

  QString			text;
  string			label;
  set<AObject *>::iterator	i, e = shown.end();
  bool				first = true;

  for( i=shown.begin(); i!=e; ++i )
    {
      if( first )
        first = false;
      else
        text += "<br>";
      text += printobj( *i, todisp );
    }
  if( !hidden.empty() )
    {
      if( !shown.empty() )
        text += "<br><br>";
      text += "<em>objects not currently displayed:</em>";
      for( i=hidden.begin(), e=hidden.end(); i!=e; ++i )
        {
          text += "<br>";
          text += printobj( *i, todisp );
        }
    }

  tip( QRect( pos, pos ), text );
}


bool QAViewToolTip::eventFilter( QObject *, QEvent *e )
{
  switch( e->type() )
    {
      //case QEvent::FocusIn:
    case QEvent::MouseButtonRelease:
      d->enabled = true;
    case QEvent::MouseMove:
      //cout << "activate tip timer\n";
      hideTip();
      if( !d->enabled )
        break;
      if( !d->waket )
        {
          d->waket = new QTimer( 0, "wakeUp" );
          connect( d->waket, SIGNAL( timeout() ), this, SLOT( wakeUp() ) );
        }
      d->waket->start( 500, true );
      break;
    case QEvent::MouseButtonPress:
      //cout << "mouse press\n";
      hideTip();
      stopTimers();
      d->enabled = false;
      break;
    default:
      break;
    }
  return false;
}


void QAViewToolTip::setEnabled( bool x )
{
  QAViewToolTip_static	& s = _viewToolTip_static();

  if( x == s.enabled )
    return;
  s.enabled = x;

  if( !x )
    hide();
}


void QAViewToolTip::hide()
{
  QAViewToolTip_static	& s = _viewToolTip_static();

  if( s.currenttip )
    s.currenttip->hide();
}


void QAViewToolTip::tip( const QRect & pos, const QString & text )
{
  QAViewToolTip_static	& s = _viewToolTip_static();
  if( !s.enabled )
    return;

  if( !s.currenttip )
    {
      s.currenttip = new QLabel( 0, "toolTipTip", Qt::WStyle_StaysOnTop 
                                 | Qt::WStyle_Customize 
                                 | Qt::WStyle_NoBorder | Qt::WStyle_Tool 
                                 | Qt::WX11BypassWM );

#if QT_VERSION >= 0x040000

      s.currenttip->setFocusPolicy( Qt::NoFocus );

#else
      s.currenttip->setFocusPolicy( QWidget::NoFocus );

#endif
      s.currenttip->setBackgroundColor( QColor( 255, 255, 168 ) );
      s.currenttip->setFrameStyle( QFrame::Raised | QFrame::Panel );
      s.currenttip->setMargin( 3 );
    }

  s.currenttip->setText( text );
  s.currenttip->resize( s.currenttip->sizeHint() );
  QPoint	gpos = d->widget->mapToGlobal( pos.topLeft() );
  QPoint	npos( gpos.x() + 10, gpos.y() - s.currenttip->height() - 10 );
  if( npos.x() + s.currenttip->width() > QApplication::desktop()->width() )
    npos.setX( gpos.x() - s.currenttip->width() - 10 );
  if( npos.y() < 0 )
    npos.setY( gpos.y() + 20 );
  s.currenttip->move( npos );
  s.currenttip->show();
}


void QAViewToolTip::wakeUp()
{
  if( !d->sleept )
    {
      d->sleept = new QTimer( 0, "fallAsleep" );
      connect( d->sleept, SIGNAL( timeout() ), SLOT( hideTip() ) );
    }

  maybeTip( d->widget->mapFromGlobal( QCursor::pos() ) );

  d->sleept->start( 3000, true );
}


void QAViewToolTip::hideTip()
{
  hide();
}


void QAViewToolTip::stopTimers()
{
  if( d->waket && d->waket->isActive() )
    d->waket->stop();
  if( d->sleept && d->sleept->isActive() )
    d->sleept->stop();
}



