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


#include <anatomist/window/qwindow.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <qpixmap.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cWindowConfig.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/object/Object.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/commands/cLoadObject.h>
#include <qtimer.h>
#include <qmenubar.h>
#include <QDrag>
#include <qtoolbar.h>
#include <stdio.h>


using namespace anatomist;
using namespace carto;
using namespace std;


struct QAWindow::Private
{
  Private();

  QTimer		*refreshtimer;
  bool			refreshneeded;
  set<QToolBar *>	toolbars;
  QAction               *detachmenuaction;
};


QAWindow::Private::Private() 
  : refreshtimer( 0 ), refreshneeded( false ), detachmenuaction( 0 )
{
}


// ---------


QAWindow::QAWindow( QWidget* parent, const char* name, Object params, 
                    Qt::WindowFlags f )
  : QMainWindow( parent ), AWindow(), ControlSwitchObserver(), 
    d( new Private )
{
  setObjectName( name );
  setWindowFlags( f );

  bool nodeco = false;
  if( params.get() )
    try
    {
      nodeco = (bool) params->getProperty( "no_decoration" )->getScalar();
      if( nodeco )
        AWindow::showToolBars( false );
    }
    catch( ... )
    {
    }

  if( isWindow() )
  {
    QPixmap	anaicon( Settings::findResourceFile(
                        "icons/icon.xpm" ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  setAcceptDrops( true );
}


QAWindow::~QAWindow()
{
  // cout << "~QAWindow\n";
  delete d;
}


void QAWindow::show()
{
  QMainWindow::show();
  window()->raise();
  if( isMinimized() )
    showNormal();
  AWindow::show();
}


void QAWindow::hide()
{
  QMainWindow::hide();
  AWindow::hide();
}


void QAWindow::iconify()
{
  AWindow::iconify();
  showMinimized();
}


void QAWindow::unIconify()
{
  if( isMinimized() )
    showNormal();
}


void QAWindow::closeEvent( QCloseEvent * event )
{
  if( testDeletable() )
  {
    event->accept();
    if( parent() )
    {
      setParent( 0 );
      setAttribute( Qt::WA_DeleteOnClose );
    }
  }
  else
  {
    cout << "can't delete window - just hiding it." << endl;
    event->ignore();
    hide();
  }
}


bool QAWindow::close()
{
  if( testDeletable() )
  {
    if( parent() )
    {
      setParent( 0 );
      setAttribute( Qt::WA_DeleteOnClose );
    }
    return QMainWindow::close();
  }
  else
  {
    cout << "can't delete window - just hiding it." << endl;
    hide();
    return false;
  }
}


void QAWindow::showToolBars( int state )
{
  if( state < 0 || state >= 2 )
    state = 1 - toolBarsVisible();
  switch( state )
    {
    case 0:
      {
        const QObjectList		& ch = children();
        QObjectList::const_iterator	ic, ec = ch.end();
        for( ic=ch.begin(); ic!=ec; ++ic )
          {
            if( *ic != centralWidget() && (*ic)->isWidgetType() )
              ((QWidget *) *ic)->hide();
          }
      }
      break;
    default:
      {
        const QObjectList		& ch = children();
        QObjectList::const_iterator	ic, ec = ch.end();
        for( ic=ch.begin(); ic!=ec; ++ic )
          {
            if( *ic != centralWidget() && (*ic)->isWidgetType() )
              ((QWidget *) *ic)->show();
          }
      }
    }
  AWindow::showToolBars( state );
}


void QAWindow::toggleToolBars()
{
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( this );
  p->value<Dictionary>()[ "show_toolbars" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


void QAWindow::setFullScreen( int x )
{
  switch( x )
    {
    case 0:
      setWindowState( windowState() & ~Qt::WindowFullScreen );
      break;
    case 1:
      setWindowState( windowState() | Qt::WindowFullScreen );
      break;
    default:
      setWindowState( windowState() ^ Qt::WindowFullScreen );
    }
}


bool QAWindow::isFullScreen() const
{
  return QMainWindow::isFullScreen();
}


void QAWindow::toggleFullScreen()
{
  Object  p = Object::value( Dictionary() );
  set<AWindow *>  sw;
  sw.insert( this );
  p->value<Dictionary>()[ "fullscreen" ] = Object::value( 2 );
  WindowConfigCommand *c = new WindowConfigCommand( sw, *p );
  theProcessor->execute( c );
}


void QAWindow::setGeometry( int x, int y, unsigned w, unsigned h )
{
  if( w > 0 && h > 0 )
    QMainWindow::setGeometry( x, y, w, h );
  else
    QMainWindow::move( x, y );
}


void QAWindow::geometry( int *px, int *py, unsigned *w, unsigned *h )
{
  *px = x();
  *py = y();
  *w = width();
  *h = height();
}


void QAWindow::createTitle()
{
  if( _instNumber >= 0 ) return;	// already done

  set<unsigned>::iterator	si;
  unsigned			n;
  char				nstr[20];

  for( si=typeCount().begin(), n=0; si!=typeCount().end() && *si==n; 
       ++si, ++n ) {}

  _instNumber = n;
  typeCount().insert( n );
  sprintf( nstr, "%d", n+1 );
  if ( n == 0 )
    setTitle( baseTitle() );
  else
    setTitle( baseTitle() + " (" + nstr + ")" );
}


void QAWindow::setTitle( const string & title )
{
  AWindow::setTitle( title );
  QMainWindow::setWindowTitle( title.c_str() );
}


void QAWindow::Refresh()
{
  //cout << "QAWindow::Refresh\n";
  if( !d->refreshtimer )
    {
      d->refreshtimer = new QTimer( this );
      d->refreshtimer->setObjectName( "QAWindow_refreshtimer" );
      connect( d->refreshtimer, SIGNAL( timeout() ), this, 
	       SLOT( triggeredRefresh() ) );
    }
  if( !d->refreshneeded )
    {
      d->refreshneeded = true;
      d->refreshtimer->setSingleShot( true );
      d->refreshtimer->start( 30 );
    }
}


void QAWindow::refreshNow()
{
  // empty - to be overloaded...
}


void QAWindow::triggeredRefresh()
{
  if( d->refreshneeded )
    {
      //cout << "QAWindow::refreshNow\n";
      AWindow::Refresh();
      refreshNow();
      d->refreshneeded = false;
    }
}


bool QAWindow::needsRedraw() const
{
  return( d->refreshneeded );
}


void QAWindow::dragEnterEvent( QDragEnterEvent* event )
{
  //cout << "QAWindow::dragEnterEvent\n";
  event->setAccepted( QAObjectDrag::canDecode( event->mimeData() )
      || QAObjectDrag::canDecodeURI( event->mimeData() ) );
}


void QAWindow::dragMoveEvent( QDragMoveEvent* event )
{
  //cout << "QAWindow::dragMoveEvent\n";
  event->setAccepted( QAObjectDrag::canDecode( event->mimeData() )
      || QAObjectDrag::canDecodeURI( event->mimeData() ) );
}


void QAWindow::dropEvent( QDropEvent* event )
{
  // cout << "QAWindow::dropEvent\n";
  if( event->source() == this )
    return;

  event->setAccepted( QAObjectDrag::canDecode( event->mimeData() )
      || QAObjectDrag::canDecodeURI( event->mimeData() ) );
  if( !event->isAccepted() )
    return;

  set<AObject *>	o;
  list<QString> objects;
  list<QString> scenars;

  if( !QAObjectDrag::decode( event->mimeData(), o )
       && QAObjectDrag::decodeURI( event->mimeData(), objects, scenars ) )
  {
    list<QString>::iterator       is, es = objects.end();
    for( is=objects.begin(); is!=es; ++is )
    {
      LoadObjectCommand *command = new LoadObjectCommand( is->toStdString() );
      theProcessor->execute( command );
      o.insert( command->loadedObject() );
    }
  }
  if( o.empty() )
    return;
  //cout << "object decoded, " << o.size() << " objects\n";
  set<AWindow *>	sw;
  sw.insert( this );
  Command	*c = new AddObjectCommand( o, sw );
  theProcessor->execute( c );
}


void QAWindow::enableDetachMenu( bool x )
{
  if( d->detachmenuaction )
    d->detachmenuaction->setEnabled( x );
}


void QAWindow::detach()
{
  if( parent() )
  {
    setParent( 0 );
    setAttribute( Qt::WA_DeleteOnClose );
    if( d->detachmenuaction )
      d->detachmenuaction->setEnabled( false );
    show();
  }
}


void QAWindow::setDetachMenuAction( QAction* a )
{
  d->detachmenuaction = a;
}


void QAWindow::mouseMoveEvent( QMouseEvent * ev )
{
  if( !(ev->buttons() & Qt::LeftButton) )
  {
    ev->ignore();
    return;
  }

  set<AObject *>	so = Objects();
  set<AObject *>::iterator	i = so.begin(), e = so.end(), j;
  // filter out temporary objects
  while( i != e )
    if( isTemporary( *i ) )
    {
      j = i;
      ++i;
      so.erase( j );
    }
    else
      ++i;

  if( ev->modifiers() & Qt::ControlModifier )
  {
    // filter selected objects
    SelectFactory	*sf = SelectFactory::factory();
    i = so.begin();
    while( i != e )
      if( !sf->isSelected( Group(), *i ) )
        {
          j = i;
          ++i;
          so.erase( j );
        }
      else
        ++i;
  }
  if( !so.empty() )
  {
    QAObjectDrag *d = new QAObjectDrag( so );
    QDrag *drag = new QDrag( this );
    drag->setMimeData( d );

    map<int, QPixmap>::const_iterator	ip
      = QObjectTree::TypeIcons.find( (*so.begin())->type() );
    if( ip != QObjectTree::TypeIcons.end() )
      drag->setPixmap( (*ip).second );
    drag->exec( Qt::CopyAction );
    ev->accept();
  }
  else
    ev->ignore();
}


QToolBar* QAWindow::addToolBar( const QString & title, const QString & name )
{
  QToolBar* tb = QMainWindow::addToolBar( title );
  tb->setObjectName( name );
  d->toolbars.insert( tb );
  return tb;
}


void QAWindow::addToolBar( QToolBar* toolbar, const QString & name )
{
  toolbar->setObjectName( name );
  QMainWindow::addToolBar( toolbar );
  d->toolbars.insert( toolbar );
}


void QAWindow::addToolBar( Qt::ToolBarArea area, QToolBar* toolbar,
                           const QString & name )
{
  toolbar->setObjectName( name );
  QMainWindow::addToolBar( area, toolbar );
  d->toolbars.insert( toolbar );
}


QToolBar* QAWindow::toolBar( const QString & name )
{
  set<QToolBar *>::const_iterator i, e = d->toolbars.end();
  for( i=d->toolbars.begin(); i!=e && (*i)->objectName()!=name; ++i );
  if( i == e )
    return 0;
  return *i;
}


void QAWindow::removeToolBar( QToolBar * toolbar )
{
  QMainWindow::removeToolBar( toolbar );
  d->toolbars.erase( toolbar );
}


QToolBar* QAWindow::removeToolBar( const QString & name )
{
  QToolBar *tb = toolBar( name );
  if( tb )
    removeToolBar( tb );
  return tb;
}


