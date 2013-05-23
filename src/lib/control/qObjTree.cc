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


#include <anatomist/control/qObjTree.h>
#include <aims/listview/qatreewidget.h>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <qlayout.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <QDragEnterEvent>
#include <anatomist/object/Object.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/refpixmap.h>
#include <anatomist/control/wControl.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/control/backPixmap_P.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/processor/pipeReader.h>
#include <anatomist/commands/cLoadObject.h>
#include <stdio.h>


using namespace anatomist;
using namespace aims::gui;
using namespace std;


map<int, QPixmap>	QObjectTree::TypeIcons;
map<int, string>	QObjectTree::TypeNames;
unsigned		QObjectTree::RefPixSize = 10;


/*namespace anatomist
{
  struct QObjectTree_Private
  {
    QObjectTree_Private();

    QTimer	*selchangedtimer;
    bool	selchanged;
  };
}


QObjectTree_Private::QObjectTree_Private() 
  : selchangedtimer( 0 ), selchanged( false )
{
}*/


void QObjectTree::initIcons()
{
  if( TypeIcons.size() > 0 )
    return;	// already done

  string str;
  str = Settings::findResourceFile( "icons/list_volume.xpm" );
  if( !TypeIcons[ AObject::VOLUME ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::VOLUME );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_bucket.xpm" );
  if( !TypeIcons[ AObject::BUCKET ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::BUCKET );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_surface.xpm" );
  if( !TypeIcons[ AObject::TRIANG ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::TRIANG );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_list.xpm" );
  if( !TypeIcons[ AObject::LIST ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::LIST );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_vector.xpm" );
  if( !TypeIcons[ AObject::VECTOR ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::VECTOR );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_graph.xpm" );
  if( !TypeIcons[ AObject::GRAPH ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::GRAPH );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_node.xpm" );
  if( !TypeIcons[ AObject::GRAPHOBJECT ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::GRAPHOBJECT );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_fusion2d.xpm" );
  if( !TypeIcons[ AObject::FUSION2D ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::FUSION2D );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_fusion3d.xpm" );
  if( !TypeIcons[ AObject::FUSION3D ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::FUSION3D );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_fascicle.xpm" );
  if( !TypeIcons[ AObject::FASCICLE ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::FASCICLE );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_fascgraph.xpm" );
  if( !TypeIcons[ AObject::FASCICLEGRAPH ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::FASCICLEGRAPH );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_texture.xpm" );
  if( !TypeIcons[ AObject::TEXTURE ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::TEXTURE );
      cerr << "Icon " << str.c_str() << " not found\n";
    }
  str = Settings::findResourceFile( "icons/list_fusiontexsurf.xpm" );
  if( !TypeIcons[ AObject::TEXSURFACE ].load( str.c_str() ) )
    {
      TypeIcons.erase( AObject::TEXSURFACE );
      cerr << "Icon " << str.c_str() << " not found\n";
    }

  TypeNames[ AObject::VOLUME        ] = "Volume";
  TypeNames[ AObject::TRIANG        ] = "Surface";
  TypeNames[ AObject::BUCKET        ] = "Bucket";
  TypeNames[ AObject::FUSION2D      ] = "Fusion2D";
  TypeNames[ AObject::FUSION3D      ] = "Fusion3D";
  TypeNames[ AObject::LIST          ] = "List";
  TypeNames[ AObject::GRAPH         ] = "Graph";
  TypeNames[ AObject::GRAPHOBJECT   ] = "GraphElement";
  TypeNames[ AObject::FASCICLE      ] = "Fascicle";
  TypeNames[ AObject::FASCICLEGRAPH ] = "Fasc. Gr.";
  TypeNames[ AObject::TEXTURE       ] = "Texture";
  TypeNames[ AObject::TEXSURFACE    ] = "Tex. Surf.";
}


QObjectTree::QObjectTree( QWidget *parent, const char *name )
  : QWidget( parent ), _viewRefCol( true ), _count( 0 )
{
  setObjectName(name);
  QVBoxLayout	*lay1 = new QVBoxLayout( this, 0, -1, "OTlayout1" );
  QFrame	*fr = new QFrame( this, "OTframe" );

  QVBoxLayout	*lay2 = new QVBoxLayout( fr, 0, -1, "OTlayout2" );

  fr->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  _lview = new QATreeWidget( fr );
  _lview->setObjectName( "qObjList" );
  _lview->setColumnCount( 5 );
  QTreeWidgetItem* hdr = new QTreeWidgetItem;
  _lview->setHeaderItem( hdr );
  hdr->setText( 0, tr( "" ) );
  hdr->setText( 1, tr( "Ref" ) );
  hdr->setText( 2, tr( "Objects" ) );
  hdr->setText( 3, tr( "Type" ) );
  hdr->setText( 4, tr( "cnt" ) );
  _lview->setSelectionMode( QTreeWidget::ExtendedSelection );
  _lview->setItemsExpandable( true );
  _lview->setRootIsDecorated( true );
  _lview->setAllColumnsShowFocus( true );
  _lview->setEditTriggers( QAbstractItemView::DoubleClicked 
    | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed );
  _lview->setSelectionBehavior( QAbstractItemView::SelectRows );
  _lview->setDragEnabled( true );
  // disable "natural" treewidget drag&drop: we overload it.
  _lview->setDragDropMode( QAbstractItemView::NoDragDrop );
  // _lview->setAlternatingRowColors( true );
  _lview->setIconSize( QSize( 32, 32 ) );
  _lview->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
  _lview->header()->setResizeMode( 1, QHeaderView::Fixed );
  _lview->header()->resizeSection( 1, 26 );
  _lview->header()->setResizeMode( 2, QHeaderView::Stretch );
  _lview->header()->setStretchLastSection( false );
  _lview->header()->setResizeMode( 3, QHeaderView::Interactive );
  _lview->header()->resizeSection( 3, 60 );
  _lview->header()->hideSection( 4 );
  _lview->header()->setSortIndicator( -1, Qt::Ascending );
  _lview->header()->setSortIndicatorShown( -1 );
  _lview->setSortingEnabled( true );

  installBackgroundPixmap( _lview );

  lay1->addWidget( fr );
  lay2->addWidget( _lview );

  initIcons();
  setAcceptDrops(TRUE);

  _lview->connect( _lview, SIGNAL( itemSelectionChanged() ), this, 
                   SLOT( unselectInvisibleItems() ) );
  connect( _lview, SIGNAL( dragStart( QTreeWidgetItem*, Qt::ButtonState ) ),
           this, 
           SLOT( startDragging( QTreeWidgetItem*, Qt::ButtonState ) ) );
  connect( _lview,
           SIGNAL( itemRightPressed( QTreeWidgetItem*, const QPoint & ) ), 
           this,
           SLOT( rightButtonPressed( QTreeWidgetItem *, const QPoint & ) ) );
  connect( _lview, 
           SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
           this, 
           SLOT( objectRenamed( QTreeWidgetItem*, int ) ) );
  connect( _lview->header(), 
           SIGNAL( sortIndicatorChanged( int, Qt::SortOrder  ) ),
           this, SLOT( sortIndicatorChanged( int, Qt::SortOrder  ) ) );
}


QObjectTree::~QObjectTree()
{
}


void QObjectTree::RegisterObject( AObject* obj )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator	io 
    = _objects.find( obj ), fo = _objects.end();

  if( io != fo )	// already there
    {
      for( ; io!=fo && (*io).first == obj; ++io )
	if( (*io).second->parent() == 0 )
	  return;	// top-level: no need to add it
    }

  QTreeWidgetItem*	li = insertObject( _lview, obj );

  if( obj->isMultiObject() )
    registerSubObjects( li, (MObject *) obj );
//   _lview->triggerUpdate();
}


void QObjectTree::registerSubObjects( QTreeWidgetItem* li, MObject* mobj )
{
  MObject::iterator	io, fo=mobj->end();
  QTreeWidgetItem *ni;

  for( io=mobj->begin(); io!=fo; ++io )
    {
      ni = insertObject( li, *io );
      if( (*io)->isMultiObject() )
	registerSubObjects( ni, (MObject *) *io );
    }
}


QTreeWidgetItem* QObjectTree::insertObject( QTreeWidgetItem* item, AObject*obj )
{
  QTreeWidgetItem	*ni = new QTreeWidgetItem;
  item->addChild( ni );
  ni->setFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable 
    | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled );

  _objects.insert( pair<AObject *, QTreeWidgetItem *>( obj, ni ) );
  _items[ ni ] = obj;
  decorateItem( ni, obj );

  return( ni );
}


void QObjectTree::decorateItem( QTreeWidgetItem* item, AObject*obj )
{
  map<int, QPixmap>::const_iterator	ip, fp = TypeIcons.end();
  const unsigned iconCol = 0, nameCol = 2, refCol = 1, typeCol = 3,
    countCol = 4;

  item->setText( nameCol, obj->name().c_str() );
  ip = TypeIcons.find( obj->type() );
  if( ip != fp )
    item->setIcon( iconCol, (*ip).second );
  else
    {
      static QPixmap	pix;
      if( pix.isNull() )
        {
          QBitmap	bmp;
          pix.resize( 1, 1 );
          pix.fill( Qt::color0 );
          bmp.resize( 1, 1 );
          bmp.fill( Qt::color0 );
          pix.setMask( bmp );
        }
      item->setIcon( iconCol, pix );
    }

  item->setIcon( refCol,
                 ReferencePixmap::referencePixmap
                 ( obj->getReferential(), obj->referentialInheritance() == 0,
                   RefPixSize ) );

  map<int, string>::const_iterator	in = TypeNames.find( obj->type() );

  if( in != TypeNames.end() )
    item->setText( typeCol, (*in).second.c_str() );
  else
    item->setText( typeCol, 0 );
  if( item->text( countCol ) == "" )
  {
    stringstream s;
    s.width( 5 );
    s.fill( '0' );
    s << _count;
    item->setText( countCol, QString::fromStdString( s.str() ) );
    ++_count;
  }
}


QTreeWidgetItem* QObjectTree::insertObject( QTreeWidget* lview, AObject*obj )
{
  QTreeWidgetItem	*ni = new QTreeWidgetItem;
  lview->insertTopLevelItem( 0, ni );
  ni->setFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable 
    | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled );

  _objects.insert( pair<AObject *, QTreeWidgetItem *>( obj, ni ) );
  _items[ ni ] = obj;
  decorateItem( ni, obj );

  return( ni );
}


void QObjectTree::UnregisterObject( AObject* obj )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator 
    io = _objects.find( obj ), fo = _objects.end(), jo;

  while( io!=fo && (*io).first==obj )
  {
    unregisterSubObjects( (*io).second );
    _items.erase( io->second );
    delete (*io).second;
    jo = io;
    ++io;
    _objects.erase( jo );
  }
  // _lview->triggerUpdate();
}


void QObjectTree::NotifyObjectChange( AObject* obj )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator 
    io = _objects.find( obj ), fo = _objects.end();

  for( ; io!=fo && (*io).first==obj; ++io )
    {
      decorateItem( (*io).second, obj );
      // (*io).second->widthChanged( 0 );
    }
  // _lview->triggerUpdate();
}


void QObjectTree::RegisterSubObject( MObject *mobj, AObject *obj )
{
  list<QTreeWidgetItem *>	lit;
  multimap<AObject *, QTreeWidgetItem *>::iterator 
    io = _objects.find( mobj ), fo = _objects.end();
  QTreeWidgetItem *ni;

  for( ; io!=fo && (*io).first==mobj; ++io )
    {
      ni = insertObject( (*io).second, obj );
      if( obj->isMultiObject() )
	registerSubObjects( ni, (MObject *) obj );
    }
}


void QObjectTree::UnregisterSubObject( MObject *mobj, AObject *obj )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator	
    io = _objects.find( obj ), fo = _objects.end(), io2;
  map<QTreeWidgetItem *, AObject *>::iterator	ii, fi=_items.end();
  QTreeWidgetItem *par;
  bool		incit;

  while( io!=fo && (*io).first==obj )
    {
      incit = true;
      par = (*io).second->parent();
      if( par )
	{
	  ii = _items.find( par );
	  if( ii != fi && (*ii).second == mobj )	// found good parent
	    {
	      if( obj->isMultiObject() )
		unregisterSubObjects( (*io).second );
              _items.erase( io->second );
	      delete (*io).second;
              io2 = io;
              ++io;
              _objects.erase( io2 );
              incit = false;
	    }
	}
      if( incit )
	++io;
    }
}


void QObjectTree::unregisterSubObjects( QTreeWidgetItem * it )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator	
    io, fo = _objects.end();
  unsigned					i, n = it->childCount();
  QTreeWidgetItem				*ch;
  map<QTreeWidgetItem *, AObject *>::iterator	ii, fi;
  AObject					*obj;

  for( i=0; i<n; ++i )
  {
    ch = it->child( i );
    ii = _items.find( ch );
    if( ii == fi )
    {
      cerr << "QObjectTree::unregisterSubObjects : item not found\n";
    }
    else
    {
      obj = (*ii).second;
      _items.erase( ii );
      for( io=_objects.find( obj ); io!=fo && (*io).first==obj; ++io )
        if( (*io).second == ch )
          {
            _objects.erase( io );
            break;
          }
      if( ch->childCount() > 0 )
        unregisterSubObjects( ch );
    }
  }
}


set<AObject *> *QObjectTree::SelectedObjects() const
{
  multimap<AObject *, QTreeWidgetItem*>::const_iterator	io, fo=_objects.end();
  set<AObject *>	*lo = new set<AObject *>;

  for( io=_objects.begin(); io!=fo; ++io )
    if( (*io).second->isSelected() 
	&& lo->find( (*io).first ) == lo->end() )
      lo->insert( (*io).first );

  return( lo );
}


AObject* QObjectTree::ObjectOfNumber( unsigned pos ) const
{
  multimap<AObject *, QTreeWidgetItem *>::const_iterator
    io, fo=_objects.end();
  unsigned n;

  if( pos >= _objects.size() )
    return( 0 );	// not in list

  for( n=0, io=_objects.begin(); n < pos && io!=fo; ++n, ++io ) {}

  return( (*io).first );
}


void QObjectTree::SelectObject( AObject *obj )
{
  multimap<AObject *, QTreeWidgetItem *>::iterator	io 
    = _objects.find( obj ), fo = _objects.end();

  if( io == fo )
    {
      cerr << "QObjectTree::SelectObject : " << obj->name() 
	   << " was not in list\n";
    }
  else
    for( ; io!=fo && (*io).first==obj; ++io )
      (*io).second->setSelected( true );
}


bool QObjectTree::isObjectSelected( AObject* obj ) const
{
  multimap<AObject *, QTreeWidgetItem *>::const_iterator	io 
    = _objects.find( obj ), fo = _objects.end();

  if( io == fo )
    {
      cerr << "QObjectTree::isObjectSelected : " << obj->name() 
	   << " was not in list\n";
      return( false );
    }

  for( ; io!=fo && (*io).first==obj; ++io )
    if( (*io).second->isSelected() )
      return( true );
  return( false );
}


void QObjectTree::UnselectAll()
{
  multimap<AObject *, QTreeWidgetItem *>::const_iterator
  io, fo=_objects.end();

  for( io=_objects.begin(); io!=fo; ++io )
    (*io).second->setSelected( false );
}


bool QObjectTree::ViewingRefColors() const
{
  return( _viewRefCol );
}


void QObjectTree::ToggleRefColorsView()
{
  if( _viewRefCol )
    UndisplayRefColors();
  else
    DisplayRefColors();
}


void QObjectTree::DisplayRefColors()
{
  _lview->header()->showSection( 1 );
//   _lview->header()->setResizeMode( 1, QHeaderView::Fixed );
//   _lview->header()->resizeSection( 1, 26 );
  _viewRefCol = true;
}


void QObjectTree::UndisplayRefColors()
{
  _lview->header()->hideSection( 1 );
//   _lview->header()->setResizeMode( 1, QHeaderView::Fixed );
//   _lview->header()->resizeSection( 1, 0 );
  _viewRefCol = false;
}


void QObjectTree::startDragging( QTreeWidgetItem* item, Qt::ButtonState )
{
  //cout << "QObjectTree::startDragging\n";
  if( !item )
    return;

  if( !item->isSelected() )
    {
      map<QTreeWidgetItem *, AObject *>::iterator io = _items.find( item );
      if( io != _items.end() )
        SelectObject( io->second );
    }

  set<AObject *>	*so = SelectedObjects();
  if( !so->empty() )
    {
      QDragObject *d = new QAObjectDrag( *so, this, "dragObject" );

      map<int, QPixmap>::const_iterator	ip
        = TypeIcons.find( (*so->begin())->type() );
      if( ip != TypeIcons.end() )
        d->setPixmap( (*ip).second ); // FIXME
      d->dragCopy();
      //cout << "dragCopy done\n";
    }
  delete so;
}


void QObjectTree::rightButtonPressed( QTreeWidgetItem* item, const QPoint & p )
{
  map<QTreeWidgetItem *, AObject *>::iterator
    io = _items.find( item );
  if( io != _items.end() )
  {
    if( !item->isSelected() )
      item->setSelected( true );
    emit rightButtonPressed( io->second, mapToGlobal( p ) );
  }
}

void QObjectTree::setObjectTypeName(int type, const std::string &name)
{
  TypeNames[type] = name;
}

void QObjectTree::setObjectTypeIcon(int type, const std::string &img)
{
  if (!TypeIcons[type].load(img.c_str()))
  {
    TypeIcons.erase(type);
    std::cerr << "Icon " << img << " not found" << std::endl;
  }
}


void QObjectTree::unselectInvisibleItems()
{
//   _lview->unselectInvisibleItems(); // FIXME
  emit selectionChanged();
}


void QObjectTree::dragEnterEvent( QDragEnterEvent* event )
{
  //cout << "QObjectTree::dragEnterEvent\n";
  event->accept( !QAObjectDrag::canDecode( event )
      && QAObjectDrag::canDecodeURI( event ) );
}


void QObjectTree::dragMoveEvent( QDragMoveEvent* event )
{
  event->accept( true );
}


void QObjectTree::dropEvent( QDropEvent* event )
{
  // cout << "QObjectTree::dropEvent\n";
  list<QString> objects;
  list<QString>	scenars;
  if( QAObjectDrag::canDecode( event ) )
  {
    set<AObject *> objs;
    // check more precisely
    if( QAObjectDrag::decode( event, objs ) )
      return;
  }
  QAObjectDrag::decodeURI( event, objects, scenars );

  list<QString>::iterator	is, es = objects.end();
  for( is=objects.begin(); is!=es; ++is )
  {
    LoadObjectCommand *command = new LoadObjectCommand( is->latin1() );
    theProcessor->execute( command );
  }
  // play scenarios (if any)
  for( is=scenars.begin(), es=scenars.end(); is!=es; ++is )
  {
    new APipeReader( is->latin1() );
  }
}


void QObjectTree::objectRenamed( QTreeWidgetItem* item, int col )
{
  int nameCol = 2;
  if( col != nameCol )
    return;

  map<QTreeWidgetItem *, AObject *>::const_iterator i = _items.find( item );
  if( i == _items.end() )
  {
    cout << "warning: item does not correspond to an existing object\n";
    return;
  }
  QString newname = item->text( nameCol );
  AObject * obj = i->second;
  // rename obj
  if( obj->name() != newname.toUtf8().data() )
  {
    obj->setName( newname.toUtf8().data() );
    theAnatomist->NotifyObjectChange( obj );
  }
}


void QObjectTree::sortIndicatorChanged( int col, Qt::SortOrder )
{
  if( col == 0 && _lview->header()->sortIndicatorSection() != -1 )
    _lview->header()->setSortIndicator( 4, Qt::Descending );
}

