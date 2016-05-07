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
#include <aims/listview/qatreewidget.h>
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
#include <QHeaderView>
#include <QDrag>
#include <qtimer.h>
#include <stdio.h>


using namespace anatomist;
using namespace aims::gui;
using namespace std;


unsigned		QWindowTree::RefPixSize = 10;
QPixmap			*QWindowTree::LinkIcon = 0;

struct QWindowTree::Private
{
  Private( QWindowTree* parent );
  ~Private();

  QTimer &timer();

  QTimer *_timer;
  QWindowTree* parent;
  set<QAWindow *> highlighted;
  std::map<anatomist::AWindow *, QTreeWidgetItem *>      windows;
  std::map<int, QTreeWidgetItem *>                      groups;
  std::map<QTreeWidgetItem *, anatomist::AWindow *>     items;
  std::map<QTreeWidgetItem *, int>                      groupItems;
  aims::gui::QATreeWidget                               *lview;
  bool                                                  viewRefCol;
  anatomist::AWindow *highlightedWindow;
  unsigned count;
};


QWindowTree::Private::Private( QWindowTree* parent ) : _timer( 0 ),
  parent( parent ), lview( 0 ), viewRefCol( true ), highlightedWindow( 0 ),
  count( 0 )
{
}


QWindowTree::Private::~Private()
{
  delete _timer;
}


QTimer & QWindowTree::Private::timer()
{
  if( !_timer )
  {
    _timer = new QTimer( parent );
    _timer->setSingleShot( true );
    QObject::connect( _timer, SIGNAL( timeout() ), parent,
                      SLOT( raiseDropWindows() ) );
  }
  return *_timer;
}


QWindowTree::QWindowTree( QWidget *parent, const char *name )
  : QWidget( parent ), d( new Private( this ) )
{
  setObjectName(name);
  if( !LinkIcon )
  {
    string s = Settings::findResourceFile( "icons/list_link.xpm" );
    LinkIcon = new QPixmap( s.c_str() );
  }

  QVBoxLayout	*lay1 = new QVBoxLayout( this );
  lay1->setObjectName( "OTlayout1" );
  lay1->setMargin( 0 );
  QFrame	*fr = new QFrame( this );
  fr->setObjectName( "OTframe" );
  int		margin = 0;
  QVBoxLayout	*lay2 = new QVBoxLayout( fr );
  lay2->setObjectName( "OTlayout2" );
  lay2->setMargin( margin );

  fr->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  d->lview = new QATreeWidget( fr );
  d->lview->setObjectName( "qWinList" );
  d->lview->setColumnCount( 6 );
  QTreeWidgetItem* hdri = new QTreeWidgetItem;
  d->lview->setHeaderItem( hdri );
  hdri->setText( 0, "" );
  hdri->setText( 1, tr( "Ref" ) );
  hdri->setText( 2, tr( "Windows" ) );
  hdri->setText( 3, tr( "Type" ) );
  hdri->setText( 4, tr( "Nb. obj." ) );
  hdri->setText( 5, tr( "cnt" ) );
  d->lview->setSelectionMode( QTreeWidget::ExtendedSelection );
  d->lview->setItemsExpandable( true );
  d->lview->setRootIsDecorated( true );
  d->lview->setAllColumnsShowFocus( true );
  d->lview->setSelectionBehavior( QAbstractItemView::SelectRows );
  d->lview->setDragEnabled( true );
  // disable "natural" treewidget drag&drop: we overload it.
  d->lview->setDragDropMode( QAbstractItemView::NoDragDrop );
  d->lview->setIconSize( QSize( 32, 32 ) );
  QHeaderView *hdr = d->lview->header();
#if QT_VERSION >= 0x050000
  hdr->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  hdr->setSectionResizeMode( 1, QHeaderView::Fixed );
  hdr->resizeSection( 1, 26 );
  hdr->setSectionResizeMode( 2, QHeaderView::Interactive );
  hdr->resizeSection( 2, 130 );
  hdr->setStretchLastSection( false );
  hdr->setSectionResizeMode( 3, QHeaderView::Interactive );
  hdr->resizeSection( 3, 60 );
  hdr->setSectionResizeMode( 4, QHeaderView::Interactive );
#else
  hdr->setResizeMode( 0, QHeaderView::ResizeToContents );
  hdr->setResizeMode( 1, QHeaderView::Fixed );
  hdr->resizeSection( 1, 26 );
  hdr->setResizeMode( 2, QHeaderView::Interactive );
  hdr->resizeSection( 2, 130 );
  hdr->setStretchLastSection( false );
  hdr->setResizeMode( 3, QHeaderView::Interactive );
  hdr->resizeSection( 3, 60 );
  hdr->setResizeMode( 4, QHeaderView::Interactive );
#endif
  hdr->resizeSection( 4, 45 );
  d->lview->header()->hideSection( 5 );
  hdr->setSortIndicator( -1, Qt::AscendingOrder );
  hdr->setSortIndicatorShown( -1 );
  d->lview->setSortingEnabled( true );

  installBackgroundPixmap( d->lview );

  lay1->addWidget( fr );
  lay2->addWidget( d->lview );

  fr->resize( 300, 300 );	// default size
  resize( 300, 300 );

  setAcceptDrops( true );
  d->lview->setMouseTracking( true );

  d->lview->connect( d->lview, SIGNAL( itemSelectionChanged() ), this, 
                   SLOT( unselectInvisibleItems() ) );
  connect( d->lview, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), 
           this,
           SLOT( doubleClickedSlot( QTreeWidgetItem *, int ) ) );
  connect( d->lview, SIGNAL( dragStart( QTreeWidgetItem*, Qt::MouseButtons,
             Qt::KeyboardModifiers ) ),
           this,
           SLOT( startDragging( QTreeWidgetItem*, Qt::MouseButtons,
             Qt::KeyboardModifiers ) ) );
  connect( d->lview, SIGNAL( cursorMoved( QTreeWidgetItem*, int ) ),
           this, SLOT( itemChanged( QTreeWidgetItem*, int ) ) );
  connect( d->lview->header(), 
           SIGNAL( sortIndicatorChanged( int, Qt::SortOrder  ) ),
           this, SLOT( sortIndicatorChanged( int, Qt::SortOrder  ) ) );
}


QWindowTree::~QWindowTree()
{
  delete d;
}


void QWindowTree::registerWindow( AWindow* win )
{
  map<AWindow *, QTreeWidgetItem *>::iterator	iw 
    = d->windows.find( win );

  if( iw == d->windows.end() )	// not already there
  {
    if( win->Group() == 0 )
    {
      insertWindow( d->lview, win );
    }
    else
    {
      map<int, QTreeWidgetItem *>::iterator 
        ig = d->groups.find( win->Group() );
      QTreeWidgetItem	*parent;

      if( ig == d->groups.end() )
        {
          char	id[10];

          sprintf( id, "Group %d", win->Group() );
          parent = new QTreeWidgetItem;
          d->lview->insertTopLevelItem( 0, parent );
          parent->setText( 2, id );
          stringstream s;
          s.width( 5 );
          s.fill( '0' );
          s << d->count;
          parent->setText( 5, QString::fromStdString( s.str() ) );
          ++d->count;

          if( !LinkIcon->isNull() )
            parent->setIcon( 0, *LinkIcon );
          d->groups[ win->Group() ] = parent;
          d->groupItems[ parent ] = win->Group();
        }
      else
        parent = (*ig).second;
      insertWindow( parent, win );
    }
    d->lview->resizeColumnToContents( 2 );
  }
}


QTreeWidgetItem* QWindowTree::insertWindow( QTreeWidgetItem* item, 
                                            AWindow*win )
{
  QTreeWidgetItem	*ni = new QTreeWidgetItem;

  item->addChild( ni );
  d->windows[ win ] = ni;
  d->items[ ni ] = win;
  decorateItem( ni, win );

  return( ni );
}


void QWindowTree::decorateItem( QTreeWidgetItem* item, AWindow*win )
{
  const unsigned	iconCol = 0, nameCol = 2, refCol = 1, 
    typeCol = 3, szCol = 4, countCol = 5;
  int		type;

  item->setText( nameCol, win->Title().c_str() );
  type = win->type();
  if( type == AWindow::WINDOW_2D )
    type = win->subtype();

  const QAWindowFactory::PixList	& pixl 
    = QAWindowFactory::pixmaps( type );

  if( !pixl.psmall.isNull() )
    item->setIcon( iconCol, pixl.psmall );
  else
    {
      static QPixmap	pix;
      if( pix.isNull() )
        {
          static QPixmap  pix;
          QBitmap	bmp( 1, 1 );
          pix.fill( Qt::color0 );
          pix.setMask( bmp );
        }
      item->setIcon( nameCol, pix );
    }

  item->setIcon( refCol,
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

  if( item->text( countCol ) == "" )
  {
    stringstream s;
    s.width( 5 );
    s.fill( '0' );
    s << d->count;
    item->setText( countCol, QString::fromStdString( s.str() ) );
    ++d->count;
  }
}


QTreeWidgetItem* QWindowTree::insertWindow( QTreeWidget* lview, AWindow*win )
{
  QTreeWidgetItem	*ni = new QTreeWidgetItem;

  lview->insertTopLevelItem( 0, ni );
  d->windows[ win ] = ni;
  d->items[ ni ] = win;
  decorateItem( ni, win );

  return( ni );
}


void QWindowTree::unregisterWindow( AWindow* win )
{
  map<AWindow *, QTreeWidgetItem *>::iterator
    iw = d->windows.find( win );

  if( iw != d->windows.end() )
  {
    QTreeWidgetItem    *item = iw->second;
    QTreeWidgetItem	*parent = item->parent();
    d->items.erase( item );
    d->windows.erase( iw );
    delete item;
    if( parent && parent->childCount() == 0 )
    {
      d->groups.erase( win->Group() );
      d->groupItems.erase( parent );
      delete parent;
    }
  }
}


void QWindowTree::NotifyWindowChange( AWindow* win )
{
  map<AWindow *, QTreeWidgetItem *>::iterator
    iw = d->windows.find( win );

  if( iw != d->windows.end() )
    decorateItem( (*iw).second, win );
}


set<AWindow *> *QWindowTree::SelectedWindows() const
{
  map<AWindow *, QTreeWidgetItem *>::const_iterator
    iw, fw=d->windows.end();
  set<AWindow *>	*lo = new set<AWindow *>;

  for( iw=d->windows.begin(); iw!=fw; ++iw )
    if( (*iw).second->isSelected() 
        && lo->find( (*iw).first ) == lo->end() )
      lo->insert( (*iw).first );

  return( lo );
}


set<int> QWindowTree::SelectedGroups() const
{
  map<int, QTreeWidgetItem *>::const_iterator	ig, fg=d->groups.end();
  set<int>					sg;

  for( ig=d->groups.begin(); ig!=fg; ++ig )
    if( (*ig).second->isSelected() )
      sg.insert( (*ig).first );

  return( sg );
}


void QWindowTree::SelectWindow( AWindow *win )
{
  map<AWindow *, QTreeWidgetItem *>::iterator	iw = d->windows.find( win );

  if( iw == d->windows.end() )
    {
      cerr << "QWindowTree::SelectWindow : " << win->Title() 
	   << " was not in list\n";
    }
  else
    (*iw).second->setSelected( true );
}


void QWindowTree::SelectGroup( int group )
{
  map<int, QTreeWidgetItem *>::iterator	ig = d->groups.find( group );

  if( ig == d->groups.end() )
    {
      cerr << "QWindowTree::SelectGroup : " << group 
	   << " was not in list\n";
    }
  else
    (*ig).second->setSelected( true );
}


bool QWindowTree::isWindowSelected( AWindow* win ) const
{
  map<AWindow *, QTreeWidgetItem *>::const_iterator
    iw = d->windows.find( win );

  if( iw == d->windows.end() )
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
  map<AWindow *, QTreeWidgetItem *>::const_iterator	iw, fw=d->windows.end();

  for( iw=d->windows.begin(); iw!=fw; ++iw )
    (*iw).second->setSelected( false );
}


bool QWindowTree::ViewingRefColors() const
{
  return( d->viewRefCol );
}


void QWindowTree::ToggleRefColorsView()
{
  if( d->viewRefCol )
    UndisplayRefColors();
  else
    DisplayRefColors();
}


void QWindowTree::DisplayRefColors()
{
  d->lview->header()->showSection( 1 );
  d->viewRefCol = true;
}


void QWindowTree::UndisplayRefColors()
{
  d->lview->header()->hideSection( 1 );
  d->viewRefCol = false;
}


void QWindowTree::dragEnterEvent( QDragEnterEvent* event )
{
  event->setAccepted( QAObjectDrag::canDecode( event->mimeData() )
      || QAObjectDrag::canDecodeURI( event->mimeData() ) );
}


void QWindowTree::dragMoveEvent( QDragMoveEvent* event )
{
  QTreeWidgetItem	*item
    = d->lview->itemAt( d->lview->viewport()->mapFromParent( event->pos() ) );
  if( item )
  {
    //cout << "QWindowTree::dragMoveEvent\n";
    map<QTreeWidgetItem *, anatomist::AWindow *>::const_iterator	iw
      = d->items.find( item );
    if( iw != d->items.end() )
    {
      if( iw->second != d->highlightedWindow )
      {
        highlightWindow( d->highlightedWindow, false );
        highlightWindow( iw->second, true );
      }
      bool ok = ( QAObjectDrag::canDecode( event->mimeData() )
            || QAObjectDrag::canDecodeURI( event->mimeData() ) );
      if( ok )
      {
        set<AWindow *>    *sw;
        if( item->isSelected() )
          sw = SelectedWindows();
        else
        {
          sw = new set<AWindow *>;
          sw->insert( (*iw).second );
        }
        set<QAWindow *>::iterator iw, ew = d->highlighted.end();
        for( iw=d->highlighted.begin(); iw!=ew; ++iw )
        {
          if( sw->find( *iw ) == sw->end() )
            highlightWindow( *iw, false );
        }
        set<AWindow *>::iterator iw2, ew2 = sw->end();
        d->highlighted.clear();
        QAWindow *qwin;
        for( iw2=sw->begin(); iw2!=ew2; ++iw2 )
        {
          qwin = dynamic_cast<QAWindow *>( *iw2 );
          if( qwin )
          {
            d->highlighted.insert( qwin );
            highlightWindow( qwin, true );
          }
        }
        delete sw;
        d->timer().start( 800 );
        event->accept();
        return;
      }
    }
  }
  highlightWindow( d->highlightedWindow, false );
  d->timer().stop();
  set<QAWindow *>::iterator iw, ew = d->highlighted.end();
  for( iw=d->highlighted.begin(); iw!=ew; ++iw )
    highlightWindow( *iw, false );
  d->highlighted.clear();
  event->ignore();
}


void QWindowTree::dropEvent( QDropEvent* event )
{
  //cout << "QWindowTree::dropEvent\n";
  clearWindowsHighlights();
  d->timer().stop();
  QTreeWidgetItem	*item 
    = d->lview->itemAt( d->lview->viewport()->mapFromParent( event->pos() ) );
  if( item )
  {
    map<QTreeWidgetItem *, anatomist::AWindow *>::const_iterator	iw
      = d->items.find( item );
    if( iw != d->items.end() )
    {
      set<AObject *>	o;
      list<QString> objects;
      list<QString> scenars;

      if( !QAObjectDrag::decode( event->mimeData(), o )
           && QAObjectDrag::decodeURI( event->mimeData(), objects, scenars ) )
      {
        list<QString>::iterator       is, es = objects.end();
        for( is=objects.begin(); is!=es; ++is )
        {
          LoadObjectCommand *command 
            = new LoadObjectCommand( is->toStdString() );
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


void QWindowTree::dragLeaveEvent( QDragLeaveEvent* )
{
  clearWindowsHighlights();
}


void QWindowTree::startDragging( QTreeWidgetItem* item, Qt::MouseButtons,
  Qt::KeyboardModifiers )
{
  //cout << "QWindowTree::startDragging\n";
  if( !item )
    return;

  if( !item->isSelected() )
    {
      map<QTreeWidgetItem *, AWindow *>::iterator	io = d->items.find( item );
      if( io != d->items.end() )
        SelectWindow( io->second );
    }

  set<AWindow *>	*so = SelectedWindows();
  if( !so->empty() )
    {
      QAWindowDrag *d = new QAWindowDrag( *so );
      QDrag *drag = new QDrag( this );
      drag->setMimeData( d );

      const QAWindowFactory::PixList	& pixl 
        = QAWindowFactory::pixmaps( (*so->begin())->type() );

      if( !pixl.psmall.isNull() )
        drag->setPixmap( pixl.psmall );

      drag->exec( Qt::CopyAction );
      //cout << "dragCopy done\n";
    }
  delete so;
}


void QWindowTree::doubleClickedSlot( QTreeWidgetItem * item, int )
{
  map<QTreeWidgetItem *, anatomist::AWindow *>::const_iterator
    i = d->items.find( item );
  if( i != d->items.end() )
    emit doubleClicked( i->second );
}


void QWindowTree::unselectInvisibleItems()
{
  emit selectionChanged();
}


void QWindowTree::itemChanged( QTreeWidgetItem *item, int )
{
  if( item )
  {
    AWindow *win = d->items[ item ];
    map<QTreeWidgetItem *, anatomist::AWindow *>::const_iterator       iw
      = d->items.find( item );
    if( iw != d->items.end() )
    {
      if( iw->second != d->highlightedWindow )
      {
        highlightWindow( d->highlightedWindow, false );
        highlightWindow( iw->second, true );
      }
    }
    else
    highlightWindow( d->highlightedWindow, false );
  }
  else
    highlightWindow( d->highlightedWindow, false );
}


namespace
{

  QColor modifiedColor( const QColor & icol, int r, int g, int b )
  {
    int rr = icol.red() + r;
    if( rr < 0 )
      rr = 0;
    else if( rr > 255 )
      rr = 255;
    int gg = icol.green() + g;
    if( gg < 0 )
      gg = 0;
    else if( gg > 255 )
      gg = 255;
    int bb = icol.blue() + b;
    if( bb < 0 )
      bb = 0;
    else if( bb > 255 )
      bb = 255;
    return QColor( rr, gg, bb );
  }

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
        int modr = -5, modg = -5, modb = 25;
        if( !qwin->isWindow() )
          qwin->setAutoFillBackground( true );
        qwin->setPalette( modifiedColor( QPalette().color(
          QPalette::Button ), modr, modg, modb ) );
        d->highlightedWindow = win;
        return;
      }
      else
      {
        qwin->setPalette( QPalette() );
        qwin->setAutoFillBackground( false );
      }
    }
  }
  d->highlightedWindow = 0;
}


void QWindowTree::raiseDropWindows()
{
  set<QAWindow *>::iterator iw, ew = d->highlighted.end();
  for( iw=d->highlighted.begin(); iw!=ew; ++iw )
    (*iw)->show();
}


void QWindowTree::clearWindowsHighlights()
{
  highlightWindow( d->highlightedWindow, false );
  set<QAWindow *>::iterator iw, ew = d->highlighted.end();
  for( iw=d->highlighted.begin(); iw!=ew; ++iw )
    highlightWindow( *iw, false );
  d->highlighted.clear();
}


void QWindowTree::sortIndicatorChanged( int col, Qt::SortOrder )
{
  if( col == 0 && d->lview->header()->sortIndicatorSection() != -1 )
    d->lview->header()->setSortIndicator( 5, Qt::DescendingOrder );
}


