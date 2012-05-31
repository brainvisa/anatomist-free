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


#include <anatomist/control/qWinTree.h>
#include <aims/listview/qalistview.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <anatomist/window/qwindow.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/refpixmap.h>
#include <anatomist/window/qWinFactory.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/control/windowdrag.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/control/backPixmap_P.h>
#include <anatomist/commands/cLoadObject.h>
#include <q3header.h>
#include <stdio.h>


using namespace anatomist;
using namespace aims::gui;
using namespace std;


unsigned		QWindowTree::RefPixSize = 10;
QPixmap			*QWindowTree::LinkIcon = 0;


QWindowTree::QWindowTree( QWidget *parent, const char *name )
  : QWidget( parent ), _viewRefCol( true ), _highlightedWindow( 0 )
{
  setObjectName(name);
  if( !LinkIcon )
  {
    string s = Settings::findResourceFile( "icons/list_link.xpm" );
    LinkIcon = new QPixmap( s.c_str() );
  }

  QVBoxLayout	*lay1 = new QVBoxLayout( this, 0, -1, "OTlayout1" );
  QFrame	*fr = new QFrame( this, "OTframe" );
  int		margin = 0;
  QVBoxLayout	*lay2 = new QVBoxLayout( fr, margin, -1, "OTlayout2" );

  fr->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  _lview = new QAListView( fr, "qWinList" );
  _lview->addColumn( "" );
  _lview->addColumn( tr( "Ref" ) );
  _lview->addColumn( tr( "Windows" ) );
  _lview->addColumn( tr( "Type" ) );
  _lview->addColumn( tr( "Nb. obj." ) );
  _lview->setMultiSelection( true );
  _lview->setSelectionMode( Q3ListView::Extended );
  _lview->setRootIsDecorated( true );
  _lview->setAllColumnsShowFocus( true );
  _lview->setItemMargin( 2 );
  _lview->setSorting( 10 );	// disable sorting by default

  installBackgroundPixmap( _lview );

  lay1->addWidget( fr );
  lay2->addWidget( _lview );

  fr->resize( 300, 300 );	// default size
  resize( 300, 300 );

  setAcceptDrops(TRUE);
  setMouseTracking( true );

  _lview->connect( _lview, SIGNAL( selectionChanged() ), this, 
                   SLOT( unselectInvisibleItems() ) );
#if QT_VERSION >= 0x040000
  connect( _lview, SIGNAL( doubleClicked( Q3ListViewItem * ) ), this,
	   SLOT( doubleClickedSlot( Q3ListViewItem * ) ) );
#else
  connect( _lview, SIGNAL( doubleClicked( QListViewItem * ) ), this,
           SLOT( doubleClickedSlot( QListViewItem * ) ) );
#endif
  connect( _lview, SIGNAL( dragStart( Q3ListViewItem*, Qt::ButtonState ) ),
           this,
           SLOT( startDragging( Q3ListViewItem*, Qt::ButtonState ) ) );
}


QWindowTree::~QWindowTree()
{
}


void QWindowTree::registerWindow( AWindow* win )
{
  map<AWindow *, Q3ListViewItem *>::iterator	iw 
    = _windows.find( win );

  if( iw == _windows.end() )	// not already there
    {
      if( win->Group() == 0 )
	{
	  insertWindow( _lview, win );
	}
      else
	{
	  map<int, Q3ListViewItem *>::iterator 
	    ig = _groups.find( win->Group() );
	  Q3ListViewItem	*parent;

	  if( ig == _groups.end() )
	    {
	      char	id[10];

	      sprintf( id, "Group %d", win->Group() );
	      parent = new Q3ListViewItem( _lview, "", "", id );
	      if( !LinkIcon->isNull() )
		parent->setPixmap( 0, *LinkIcon );
	      _groups[ win->Group() ] = parent;
	      _groupItems[ parent ] = win->Group();
	    }
	  else
	    parent = (*ig).second;
	  insertWindow( parent, win );
	}
      _lview->triggerUpdate();
    }
}


Q3ListViewItem* QWindowTree::insertWindow( Q3ListViewItem* item, AWindow*win )
{
  Q3ListViewItem	*ni = new Q3ListViewItem( item );

  _windows[ win ] = ni;
  _items[ ni ] = win;
  decorateItem( ni, win );

  return( ni );
}


void QWindowTree::decorateItem( Q3ListViewItem* item, AWindow*win )
{
  const unsigned	iconCol = 0, nameCol = 2, refCol = 1, 
    typeCol = 3, szCol = 4;
  int		type;

  item->setText( nameCol, win->Title().c_str() );
  type = win->type();
  if( type == AWindow::WINDOW_2D )
    type = win->subtype();

  const QAWindowFactory::PixList	& pixl 
    = QAWindowFactory::pixmaps( type );

  if( !pixl.psmall.isNull() )
    item->setPixmap( iconCol, pixl.psmall );
  else
    {
      static QPixmap	pix;
      if( pix.isNull() )
        {
          static QPixmap  pix;
          QBitmap	bmp;
          pix.resize( 1, 1 );
          pix.fill( Qt::color0 );
          bmp.resize( 1, 1 );
          bmp.fill( Qt::color0 );
          pix.setMask( bmp );
        }
      item->setPixmap( nameCol, pix );
    }

  item->setPixmap( refCol,
                   ReferencePixmap::referencePixmap
                   ( win->getReferential(), true, RefPixSize ) );

  string	tname = AWindowFactory::typeString( type );

  if( tname.empty() )
    item->setText( typeCol, 0 );
  else
    item->setText( typeCol, tname.c_str() );

  char	no[ 10 ];

  sprintf( no, "%u", (unsigned) win->Objects().size() );
  item->setText( szCol, no );
}


Q3ListViewItem* QWindowTree::insertWindow( Q3ListView* lview, AWindow*win )
{
  Q3ListViewItem	*ni = new Q3ListViewItem( lview );

  _windows[ win ] = ni;
  _items[ ni ] = win;
  decorateItem( ni, win );

  return( ni );
}


void QWindowTree::unregisterWindow( AWindow* win )
{
  map<AWindow *, Q3ListViewItem *>::iterator
    iw = _windows.find( win );

  if( iw != _windows.end() )
  {
    Q3ListViewItem    *item = iw->second;
    Q3ListViewItem	*parent = item->parent();
    _items.erase( item );
    _windows.erase( iw );
    delete item;
    if( parent && parent->childCount() == 0 )
    {
      _groups.erase( win->Group() );
      _groupItems.erase( parent );
      delete parent;
    }
    _lview->triggerUpdate();
  }
}


void QWindowTree::NotifyWindowChange( AWindow* win )
{
  map<AWindow *, Q3ListViewItem *>::iterator
    iw = _windows.find( win );

  if( iw != _windows.end() )
    {
      decorateItem( (*iw).second, win );
      (*iw).second->widthChanged( 0 );
    }
  _lview->triggerUpdate();
}


set<AWindow *> *QWindowTree::SelectedWindows() const
{
  map<AWindow *, Q3ListViewItem *>::const_iterator	iw, fw=_windows.end();
  set<AWindow *>	*lo = new set<AWindow *>;

  for( iw=_windows.begin(); iw!=fw; ++iw )
    if( (*iw).second->isSelected() 
	&& lo->find( (*iw).first ) == lo->end() )
      lo->insert( (*iw).first );

  return( lo );
}


set<int> QWindowTree::SelectedGroups() const
{
  map<int, Q3ListViewItem *>::const_iterator	ig, fg=_groups.end();
  set<int>					sg;

  for( ig=_groups.begin(); ig!=fg; ++ig )
    if( (*ig).second->isSelected() )
      sg.insert( (*ig).first );

  return( sg );
}


void QWindowTree::SelectWindow( AWindow *win )
{
  map<AWindow *, Q3ListViewItem *>::iterator	iw = _windows.find( win );

  if( iw == _windows.end() )
    {
      cerr << "QWindowTree::SelectWindow : " << win->Title() 
	   << " was not in list\n";
    }
  else
    (*iw).second->setSelected( true );
}


void QWindowTree::SelectGroup( int group )
{
  map<int, Q3ListViewItem *>::iterator	ig = _groups.find( group );

  if( ig == _groups.end() )
    {
      cerr << "QWindowTree::SelectGroup : " << group 
	   << " was not in list\n";
    }
  else
    (*ig).second->setSelected( true );
}


bool QWindowTree::isWindowSelected( AWindow* win ) const
{
  map<AWindow *, Q3ListViewItem *>::const_iterator
    iw = _windows.find( win );

  if( iw == _windows.end() )
    {
      cerr << "QWindowTree::isWindowSelected : " << win->Title() 
	   << " was not in list\n";
      return( false );
    }

  if( (*iw).second->isSelected() )
    return( true );
  return( false );
}


void QWindowTree::UnselectAll()
{
  map<AWindow *, Q3ListViewItem *>::const_iterator	iw, fw=_windows.end();

  for( iw=_windows.begin(); iw!=fw; ++iw )
    (*iw).second->setSelected( false );
}


bool QWindowTree::ViewingRefColors() const
{
  return( _viewRefCol );
}


void QWindowTree::ToggleRefColorsView()
{
  if( _viewRefCol )
    UndisplayRefColors();
  else
    DisplayRefColors();
}


void QWindowTree::DisplayRefColors()
{
  _lview->setColumnWidthMode( 1, Q3ListView::Maximum );
  _lview->setColumnWidth( 1, RefPixSize + 20 );
  _viewRefCol = true;
  _lview->triggerUpdate();
}


void QWindowTree::UndisplayRefColors()
{
  _lview->setColumnWidthMode( 1, Q3ListView::Manual );
  _lview->setColumnWidth( 1, 0 );
  _viewRefCol = false;
  _lview->triggerUpdate();
}


void QWindowTree::dragEnterEvent( QDragEnterEvent* event )
{
  event->accept( QAObjectDrag::canDecode( event )
      || QAObjectDrag::canDecodeURI( event ) );
}


void QWindowTree::dragMoveEvent( QDragMoveEvent* event )
{
  Q3ListViewItem	*item
    = _lview->itemAt( _lview->viewport()->mapFromParent( event->pos() ) );
  if( item )
    {
      //cout << "QWindowTree::dragMoveEvent\n";
      map<Q3ListViewItem *, anatomist::AWindow *>::const_iterator	iw
	= _items.find( item );
      event->accept( iw != _items.end()
          && ( QAObjectDrag::canDecode( event )
            || QAObjectDrag::canDecodeURI( event ) ) );
    }
  else
    event->accept( false );
}


void QWindowTree::dropEvent( QDropEvent* event )
{
  //cout << "QWindowTree::dropEvent\n";
  Q3ListViewItem	*item 
    = _lview->itemAt( _lview->viewport()->mapFromParent( event->pos() ) );
  if( item )
  {
    map<Q3ListViewItem *, anatomist::AWindow *>::const_iterator	iw
      = _items.find( item );
    if( iw != _items.end() )
    {
      set<AObject *>	o;
      list<QString> objects;
      list<QString> scenars;

      if( !QAObjectDrag::decode( event, o )
           && QAObjectDrag::decodeURI( event, objects, scenars ) )
      {
        list<QString>::iterator       is, es = objects.end();
        for( is=objects.begin(); is!=es; ++is )
        {
          LoadObjectCommand *command = new LoadObjectCommand( is->latin1() );
          theProcessor->execute( command );
          o.insert( command->loadedObject() );
        }
      }
      if( o.empty() )
        return;

      //cout << "object decoded, " << o.size() << " objects\n";
      set<AWindow *>	*sw;

      if( item->isSelected() )
      {
        sw = SelectedWindows();
      }
      else
      {
        sw = new set<AWindow *>;
        sw->insert( (*iw).second );
      }
      Command	*c = new AddObjectCommand( o, *sw );
      theProcessor->execute( c );
      delete sw;
    }
  }
}


void QWindowTree::startDragging( Q3ListViewItem* item, Qt::ButtonState )
{
  //cout << "QWindowTree::startDragging\n";
  if( !item )
    return;

  if( !item->isSelected() )
    {
      map<Q3ListViewItem *, AWindow *>::iterator	io = _items.find( item );
      if( io != _items.end() )
        SelectWindow( io->second );
    }

  set<AWindow *>	*so = SelectedWindows();
  if( !so->empty() )
    {
      QDragObject *d = new QAWindowDrag( *so, this, "dragObject" );

      const QAWindowFactory::PixList	& pixl 
        = QAWindowFactory::pixmaps( (*so->begin())->type() );

      if( !pixl.psmall.isNull() )
        d->setPixmap( pixl.psmall );

      d->dragCopy();
      //cout << "dragCopy done\n";
    }
  delete so;
}


void QWindowTree::doubleClickedSlot( Q3ListViewItem * item )
{
  map<Q3ListViewItem *, anatomist::AWindow *>::const_iterator
    i = _items.find( item );
  if( i != _items.end() )
    emit doubleClicked( i->second );
}


void QWindowTree::unselectInvisibleItems()
{
  _lview->unselectInvisibleItems();
  emit selectionChanged();
}


void QWindowTree::mouseMoveEvent( QMouseEvent* ev )
{
  cout << "mouseMoveEvent\n";
//   QPoint p( _lview->contentsToViewport( _lview->mapFrom( this, ev->pos() ) ) );
  QPoint p( _lview->mapFrom( this, ev->pos() ) - QPoint( 0, _lview->header()->height() ) );
//   cout << "ev pos: " << ev->pos().y() << ", p: " << p.y() << endl;
  Q3ListViewItem *item = _lview->itemAt( p );
  if( item )
  {
    AWindow *win = _items[ item ];
    if( win == _highlightedWindow )
      return;
    highlightWindow( _highlightedWindow, false );
    highlightWindow( win, true );
  }
  else
  {
    highlightWindow( _highlightedWindow, false );
  }
  ev->ignore();
}


void QWindowTree::highlightWindow( AWindow *win, bool state )
{
  if( win && theAnatomist->hasWindow( win ) )
  {
    QAWindow *qwin = dynamic_cast<QAWindow *>( win );
    if( qwin )
    {
      if( state )
      {
        cout << "highlight win: " << qwin << endl;
        qwin->setPalette( QColor( 255, 160, 160 ) );
        _highlightedWindow = win;
        return;
      }
      else
      {
        cout << "unhighlight win: " << qwin << endl;
        qwin->setPalette( QPalette() );
      }
    }
  }
  _highlightedWindow = 0;
}


