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

#include <anatomist/browser/qwObjectBrowser.h>
#include <anatomist/browser/stringEdit.h>
#include <anatomist/browser/attributedchooser.h>
#include <anatomist/browser/labeledit.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/dialogs/colorDialog.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/commands/cSelect.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/application/syntax.h>
#include <anatomist/control/backPixmap_P.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/browser/attDescr.h>
#include <anatomist/browser/browsercontrol.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/controler/view.h>
#include <anatomist/object/actions.h>
#include <anatomist/selection/qSelMenu.h>
#include <aims/def/path.h>
#include <aims/graph/graphmanip.h>
#include <graph/graph/graph.h>
#include <graph/tree/tree.h>
#include <cartobase/exception/assert.h>
#include <cartobase/object/sreader.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qapplication.h>
#include <QList>
#include <qtimer.h>
#include <QHeaderView>
#include <QDrag>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <float.h>

namespace Qt {}
using namespace Qt;


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

// private class and def

#define Event_BrowserUpdate	( QEvent::User + 1 )


namespace
{

  class QBrowserUpdateEvent : public QEvent
  {
  public:
    typedef QEvent::Type QtEvType;
    QBrowserUpdateEvent( QtEvType type ) : QEvent( type ) {}
  };


  class BrowserView : public View
  {
  public:
    BrowserView( AWindow * win ) : View( win ) {}
    virtual string name() const { return "Browser"; }
  };

}


struct QObjectBrowser::Private
{
  Private( QObjectBrowser* br );

  ///	Left (main) tree panel (objects)
  QObjectBrowserWidget	*lview;
  ///	Right (optional) tree panel (graph relations, ...)
  QObjectBrowserWidget	*rview;
  QStatusBar		*statbar;
  QLabel		*modeWid;
  ///	Flag to avoid recursive updates
  bool			recursive;
  ///	Current edition mode
  unsigned		editMode;
  ///	Non-modal open editor
  QLabelEdit		*editor;

  ///	0: left, 1: right
  unsigned		lastActivePanel;
  QTimer        	*rviewrefreshtimer;
  bool          	rviewrefresh;

  unique_ptr<View>	view;

  QTreeWidgetItem        *lastselectednode1;
  QTreeWidgetItem        *lastselectednode2;

  // temporary elements for "add attribute" option
  set<QTreeWidgetItem *> tempAddedItems;
  bool                  tempAddedNewSyntax;
  string                tempAddedSyntax;
  string                tempAddedName;
  bool                  showDetailsUponRegister;
};


struct QObjectBrowser::Static
{
  Static();

   vector<string>                       Modes;
  int					classType;
  /// Browser windows static counter
  std::set<unsigned>			browserCount;
  std::string				baseTitle;
  ///	attribute editors for base types
  std::map<std::string, EditFunc>	typeEditors;
  /**	attribute editors for particular attributes.
	map: syntactic att |-> attribute |-> edit function
  */
  std::map<std::string, std::map<std::string, EditFunc> > attEditors;
  bool					initialized;
  ///	Browser in EDIT mode (unique)
  QObjectBrowser			*receivingBrowser;
  AttDescr	                        attDescr;
};


QObjectBrowser::Private::Private( QObjectBrowser* br )
  : lview( 0 ), rview( 0 ), statbar( 0 ), modeWid( 0 ), recursive( false ), 
    editMode( QObjectBrowser::NORMAL ), editor( 0 ), lastActivePanel( 0 ),
    rviewrefreshtimer( 0 ), rviewrefresh( false ),
    view( new BrowserView( br ) ),
    lastselectednode1( 0 ), lastselectednode2( 0 ),
    showDetailsUponRegister( false )
{
}

QObjectBrowser::Static::Static()
  : baseTitle( "Browser" ), initialized( false ), receivingBrowser( 0 )
{
  Modes.reserve( 3 );
  Modes.push_back( QObjectBrowser::tr( "Normal" ).toStdString() );
  Modes.push_back( QObjectBrowser::tr( "EDIT" ).toStdString() );
  Modes.push_back( QObjectBrowser::tr( "SEND edit" ).toStdString() );
}

QObjectBrowser::Static & QObjectBrowser::staticState()
{
  static Static  s;
  return s;
}

int QObjectBrowser::registerClass()
{
  int type = AWindowFactory::registerType( QT_TR_NOOP( "Browser" ), 
					   createBrowser, true );
  staticState().classType = type;

  return type;
}


QObjectBrowser::QObjectBrowser( QWidget * parent, const char * name, 
                                Object options, WindowFlags f )
  : ControlledWindow( parent, name, options, f ), d( new Private( this ) )
{
  setAttribute( Qt::WA_DeleteOnClose );
  createTitle();
  //setWindowTitle( tr( "AnaQt Objects Browser" ) );
  resize( 400, 400 );

  //	controls
  d->view->controlSwitch()->attach( this );
  d->view->controlSwitch()->notifyAvailableControlChange();
  d->view->controlSwitch()->notifyActivableControlChange();
  d->view->controlSwitch()->setActiveControl( "Browser Selection" );
  // should be done in ControlSwitch
  d->view->controlSwitch()->notifyActiveControlChange();
  d->view->controlSwitch()->notifyActionChange();

  Static  & sstate = staticState();
  if( !sstate.initialized )
    {
      sstate.attDescr.addSyntax( SyntaxRepository::internalSyntax() );
      sstate.initialized = true;
    }

  QSplitter	*fr = new QSplitter( this );
  fr->setObjectName( "OBrSplit" );
  int   margin = 3;
  fr->setContentsMargins( margin, margin, margin, margin );
  setCentralWidget( fr );

  fr->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  d->lview = new QObjectBrowserWidget( fr, "LBrowser" );
  d->rview = new QObjectBrowserWidget( fr, "RBrowser" );
  installBackgroundPixmap( d->lview );
  installBackgroundPixmap( d->rview );

  d->lview->setColumnCount( 6 );
  QTreeWidgetItem* hdr = new QTreeWidgetItem;
  d->lview->setHeaderItem( hdr );
  hdr->setText( 0, tr( "Object" ) );
  hdr->setText( 1, tr( "Type" ) );
  hdr->setText( 2, tr( "Value" ) );
  hdr->setText( 3, tr( "Label" ) );
  hdr->setText( 4, tr( "Sel." ) );
  hdr->setText( 5, tr( "Reg." ) );
  QHeaderView *hdri = d->lview->header();
#if QT_VERSION >= 0x050000
  hdri->setSectionResizeMode( 0, QHeaderView::Interactive );
  hdri->setSectionResizeMode( 1, QHeaderView::Interactive );
  hdri->setSectionResizeMode( 2, QHeaderView::Interactive );
  hdri->setSectionResizeMode( 3, QHeaderView::Interactive );
  hdri->setSectionResizeMode( 4, QHeaderView::Fixed );
  hdri->resizeSection( 4, 26 );
  hdri->setStretchLastSection( false );
  hdri->setSectionResizeMode( 5, QHeaderView::Fixed );
#else
  hdri->setResizeMode( 0, QHeaderView::Interactive );
  hdri->setResizeMode( 1, QHeaderView::Interactive );
  hdri->setResizeMode( 2, QHeaderView::Interactive );
  hdri->setResizeMode( 3, QHeaderView::Interactive );
  hdri->setResizeMode( 4, QHeaderView::Fixed );
  hdri->resizeSection( 4, 26 );
  hdri->setStretchLastSection( false );
  hdri->setResizeMode( 5, QHeaderView::Fixed );
#endif
  hdri->resizeSection( 5, 26 );
  d->lview->setSortingEnabled( true );

  d->rview->setColumnCount( 5 );
  hdr = new QTreeWidgetItem;
  d->rview->setHeaderItem( hdr );
  hdr->setText( 0, tr( "Name" ) );
  hdr->setText( 1, tr( "Type" ) );
  hdr->setText( 2, tr( "Value" ) );
  hdr->setText( 3, tr( "Label1" ) );
  hdr->setText( 4, tr( "Label2" ) );
  hdri = d->rview->header();
#if QT_VERSION >= 0x050000
  hdri->setSectionResizeMode( 2, QHeaderView::Interactive );
  hdri->setStretchLastSection( false );
  hdri->setSectionResizeMode( 3, QHeaderView::Interactive );
  hdri->setSectionResizeMode( 4, QHeaderView::Interactive );
#else
  hdri->setResizeMode( 2, QHeaderView::Interactive );
  hdri->setStretchLastSection( false );
  hdri->setResizeMode( 3, QHeaderView::Interactive );
  hdri->setResizeMode( 4, QHeaderView::Interactive );
#endif
  d->rview->setSortingEnabled( true );

  d->statbar = statusBar(); // new QStatusBar( this, "status" );
  d->modeWid = new QLabel( modeString().c_str(), d->statbar );
  d->modeWid->setObjectName( "modelabel" );
  d->modeWid->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  d->modeWid->setFixedHeight( d->modeWid->sizeHint().height() );
  d->statbar->addPermanentWidget( d->modeWid, 0 );
  d->statbar->setFixedHeight( d->statbar->sizeHint().height() );

  // this is a bug in Qt4...
  d->statbar->hide();
  QTimer::singleShot( 0, d->statbar, SLOT( show() ) );

  d->rview->setMinimumWidth( 0 );
  fr->setStretchFactor( 0, 1 );
  fr->setStretchFactor( 1, 0 );
  d->rview->resize( 0, d->rview->sizeHint().height() );
  QList<int>	vl;
  vl.append( fr->width() );
  vl.append( 0 );
  fr->setSizes( vl );

  showToolBars( 0 );

  connect( d->lview, SIGNAL( itemSelectionChanged() ), this, 
            SLOT( leftSelectionChangedSlot() ) );
  connect( d->lview, 
            SIGNAL( itemRightPressed( QTreeWidgetItem *, const QPoint & ) ), 
            this, 
            SLOT( rightButtonClickedSlot( QTreeWidgetItem *, 
                                          const QPoint & ) ) );
  connect( d->lview, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), 
            this, 
            SLOT( doubleClickedSlot( QTreeWidgetItem *, int ) ) );
  connect( d->rview, SIGNAL( itemRightPressed( QTreeWidgetItem *, 
                                                const QPoint & ) ), 
            this, SLOT( rightButtonRightPanel( QTreeWidgetItem *, 
                                              const QPoint & ) ) );
  connect( d->rview, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), 
            this,
            SLOT( rightPanelDoubleClicked( QTreeWidgetItem *, int ) ) );
  connect( d->rview, SIGNAL( itemSelectionChanged() ), 
           this, SLOT( rightSelectionChangedSlot() ) );
  connect( d->lview, SIGNAL( dragStart( QTreeWidgetItem *, Qt::MouseButtons, 
                                        Qt::KeyboardModifiers ) ), 
           this, SLOT( startDrag( QTreeWidgetItem *, Qt::MouseButtons, 
                                  Qt::KeyboardModifiers ) ) );
  connect( d->rview, SIGNAL( dragStart( QTreeWidgetItem *, Qt::MouseButtons, 
                                        Qt::KeyboardModifiers ) ), 
           this, SLOT( startDrag( QTreeWidgetItem *, Qt::MouseButtons, 
                                  Qt::KeyboardModifiers ) ) );

  //	Attribute editors
  if( sstate.typeEditors.size() == 0 )
    {
      sstate.typeEditors[ "string" ] = stringEditor;
      sstate.typeEditors[ "int" ] = intEditor;
      sstate.typeEditors[ "S32" ] = intEditor;
      sstate.typeEditors[ "float" ] = floatEditor;
      sstate.typeEditors[ "FLOAT" ] = floatEditor;
      sstate.typeEditors[ "double" ] = doubleEditor;
      sstate.typeEditors[ "DOUBLE" ] = doubleEditor;
      sstate.attEditors[ "fold" ][ "label" ] = labelEditor;
      sstate.attEditors[ "fold" ][ "name" ] = labelEditor;
      sstate.attEditors[ "fold_name" ][ "color" ] = colorEditor;
      sstate.attEditors[ "cluster" ][ "label" ] = labelEditor;
      sstate.attEditors[ "cluster" ][ "name" ] = labelEditor;
      sstate.attEditors[ "CorticalFoldArg" ][ "bottom_label" ] = colorEditor;
      sstate.attEditors[ "CorticalFoldArg" ][ "other_label" ] = colorEditor;
      sstate.attEditors[ "CorticalFoldArg" ][ "ss_label" ] = colorEditor;
      sstate.attEditors[ "CorticalFoldArg" ][ "Tmtktri_label" ] = colorEditor;
      sstate.attEditors[ "ClusterArg" ][ "bucket_label" ] = colorEditor;
      sstate.attEditors[ "ClusterArg" ][ "bucket_filename" ] = colorEditor;
      sstate.attEditors[ "ClusterArg" ][ "Tmtktri_label" ] = colorEditor;
      sstate.attEditors[ "ClusterArg" ][ "Tmtktri_filename" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "bucket_label" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "bucket_filename" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "Tmtktri_label" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "Tmtktri_filename" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "roi_mesh_label" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "roi_mesh_filename" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "roi_mesh_junction_label" ] = colorEditor;
      sstate.attEditors[ "RoiArg" ][ "roi_mesh_juntion_filename" ]
        = colorEditor;
      sstate.attEditors[ "roi" ][ "label" ] = labelEditor;
      sstate.attEditors[ "roi" ][ "name" ] = labelEditor;
    }
}


QObjectBrowser::~QObjectBrowser()
{
  Static  & s = staticState();
  if( _instNumber != -1 ) 
    s.browserCount.erase( s.browserCount.find( _instNumber ) );
  if( d->editor )
    {
      delete d->editor;
      editCancel();
    }
  delete d;
}


AWindow* QObjectBrowser::createBrowser( void* dock, carto::Object options )
{
  QWidget	*dk = (QWidget *) dock;
  QObjectBrowser* b = new QObjectBrowser( dk, "Browser", options );

  b->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return( b );
}


void QObjectBrowser::displayClickPoint()
{
}


void QObjectBrowser::Draw( bool )
{
}


void QObjectBrowser::registerObject( AObject* object, bool temporaryObject,
                                     int pos )
{
  if( _sobjects.find( object ) == _sobjects.end() )
  {
    QAWindow::registerObject( object, temporaryObject, pos );
    d->lview->registerObject( object, temporaryObject, pos,
                              d->showDetailsUponRegister );
  }
}


void QObjectBrowser::unregisterObject( AObject* object )
{
  d->lview->unregisterObject( object );

  AWindow::unregisterObject( object );

  //	re-insert sub-objects which could still be in window

  set<AObject *>::const_iterator	io, fo=_sobjects.end();

  for( io=_sobjects.begin(); io!=fo; ++io )
    d->lview->registerObject( *io );
  updateRightPanel();
}


void QObjectBrowser::update( const anatomist::Observable* observable, 
                             void* arg )
{
  const AObject	*obj = dynamic_cast<const AObject *>( observable );
  if( obj )
  {
    if( arg == 0 )
      unregisterObject( (AObject *) obj );
    else
      updateObject( (AObject * ) obj );
  }

  AWindow::update( observable, arg );
}


void QObjectBrowser::refreshNow()
{
  using carto::shared_ptr;

  //cout << "QObjectBrowser::refreshNow()\n";
  if( d->recursive )
    return;		// already in refresh process

  QAWindow::refreshNow();	// common parts

  const unsigned	selColumn = 4;

  map<QTreeWidgetItem *, AObject *>::const_iterator 
    ia, fa = d->lview->aObjects().end();
  SelectFactory 	*fac = SelectFactory::factory();
  QTreeWidgetItem		*cur = 0;
  QTreeWidgetItem		*it;

  d->recursive = true;

  for( ia=d->lview->aObjects().begin(); ia!=fa; ++ia )
    {
      it = (*ia).first;
      if( fac->isSelected( Group(), (*ia).second ) )
      {
        if( !it->isSelected() )
        {
          it->setText( selColumn, "*" );
          it->setSelected( true );
          if( !cur )
            cur = it;
        }
      }
      else if( it->isSelected() )
      {
        it->setText( selColumn, 0 );
        it->setSelected( false );
      }
    }

  //	now check indirect objects (in graph objects)

  map<QTreeWidgetItem *, GenericObject *>::const_iterator 
    ig, fg = d->lview->gObjects().end();
  shared_ptr<AObject>	ao;

  for( ig=d->lview->gObjects().begin(); ig!=fg; ++ig )
    if( (*ig).second->getProperty( "ana_object", ao ) )
    {
      it = (*ig).first;
      if( fac->isSelected( Group(), ao.get() ) )
      {
        if( !it->isSelected() )
        {
          it->setText( selColumn, "*" );
          it->setSelected( true );
          if( !cur )
            cur = it;
        }
      }
      else if( it->isSelected() )
      {
        it->setText( selColumn, 0 );
        it->setSelected( false );
      }
    }

  if( cur )
  {
    d->lview->setCurrentItem( cur );
    d->lview->scrollToItem( cur, QTreeWidget::EnsureVisible );
    /* apparently setting the current item sometimes deselects it. 
       So force it again. */
    cur->setSelected( true );
  }
  ResetRefreshFlag();

  d->recursive = false;
  updateRightPanel();
  QWidget::update();
}


void QObjectBrowser::updateObject( AObject* obj )
{
  d->lview->updateObject( obj );
}


void QObjectBrowser::leftSelectionChangedSlot()
{
  d->lastActivePanel = 0;	// use left LView
  normalModeSelectionChanged();
}


void QObjectBrowser::normalModeSelectionChanged()
{
  using carto::shared_ptr;

  const unsigned selColumn = 4;

  if( d->recursive )
    return;

  QObjectBrowserWidget	*view;
  if( d->lastActivePanel == 0 )
    view = d->lview;
  else
    view = d->rview;

  /* cout << "QObjectBrowser::normalModeSelectionChanged()\n";
  cout << "view: " << ( view == d->lview ? "left" : "right" ) << endl; */
  map<QTreeWidgetItem *, AObject *>::const_iterator 
    io, fo = view->aObjects().end();
  set<AObject *>	tosel, tounsel;
  SelectFactory		*fac = SelectFactory::factory();

  for( io=view->aObjects().begin(); io!=fo; ++io )
  {
    if( (*io).first->isSelected() )
    {
      if( !fac->isSelected( Group(), (*io).second ) )
        tosel.insert( (*io).second );
      (*io).first->setText( selColumn, "*" );
    }
    else if( fac->isSelected( Group(), (*io).second ) )
    {
      tounsel.insert( (*io).second );
      (*io).first->setText( selColumn, 0 );
    }
  }

  //	now check indirect objects (in graph objects)

  map<QTreeWidgetItem *, GenericObject *>::const_iterator 
    ig, fg = view->gObjects().end();
  shared_ptr<AObject>	ao;
  unsigned	ngosel = 0;
  QTreeWidgetItem	*cur = 0;
  map<Hierarchy *, list<QObjectBrowserWidget::ItemDescr> > hieelem;

  for( ig=view->gObjects().begin(); ig!=fg; ++ig )
  {
    QTreeWidgetItem* it = (*ig).first;
    if( it->isSelected() )
    {
      ++ngosel;
      cur = it;
      QObjectBrowserWidget::ItemDescr   descr;
      view->whatIs( cur, descr );
      if( descr.tobj )
      {
        Hierarchy   *h = dynamic_cast<Hierarchy *>( descr.tobj );
        if( h && mode() == NORMAL )
          hieelem[h].push_back( descr );
      }
    }
    if( (*ig).second->getProperty( "ana_object", ao ) )
    {
      if( it->isSelected() )
      {
        if( !fac->isSelected( Group(), ao.get() ) )
        {
          tosel.insert( ao.get() );
          it->setText( selColumn, "*" );
        }
      }
      else if( fac->isSelected( Group(), ao.get() ) )
      {
        tounsel.insert( ao.get() );
        it->setText( selColumn, 0 );
      }
    }
  }

  // process nomenclature selections
  map<Hierarchy *, list<QObjectBrowserWidget::ItemDescr> >::iterator
      in, en = hieelem.end();
  list<QObjectBrowserWidget::ItemDescr>::iterator inl, enl;
  for( in=hieelem.begin(); in!=en; ++in )
    for( inl=in->second.begin(), enl=in->second.end(); inl!=enl; ++inl )
      nomenclatureClick( in->first, *inl, tosel );

  // process AObject selections
  SelectCommand	*c
    = new SelectCommand( tosel, tounsel, Group() );
  theProcessor->execute( c );

  updateRightPanel();
}


void QObjectBrowser::sendModeSelection( void* parent )
{
  QObjectBrowser	*obr = (QObjectBrowser *) parent;
  QObjectBrowserWidget	*br = obr->d->lview;
  QTreeWidgetItem		*item = br->currentItem();

  if( !item )
    return;

  string val = obr->canSend( br, item );

  if( !val.empty() )
    staticState().receivingBrowser->d->editor->receiveValue( val );
}


void QObjectBrowser::setAttributeToAllSelected( void* parent )
{
  QObjectBrowser	*obr = (QObjectBrowser *) parent;
  QObjectBrowserWidget	*br = obr->d->lview;
  QTreeWidgetItem	*item = br->currentItem();

  if( !item )
    return;

  QObjectBrowserWidget::ItemDescr	descr;

  br->whatIs( item, descr );
  if( !descr.tobj )
    return;
  Hierarchy *nom = dynamic_cast<Hierarchy *>( descr.tobj );
  if( !nom )
    return;
  GenericObject *ao = descr.ao;
  string att = "name";	// hard coded up to now
  string val;
  ao->getProperty( att, val );
  string synt;
  nom->attributed()->getProperty( "graph_syntax", synt );

  if( !val.empty() )
  {
    SelectFactory* sel = SelectFactory::factory();
    map<unsigned, set<AObject *> >::const_iterator
        i = sel->selected().find( obr->Group() );
    if( i != sel->selected().end() )
    {
      set<AObject *>        aobj;
      const set<AObject *>  & so = i->second;
      set<AObject *>::iterator  io, eo = so.end();
      for( io=so.begin(); io!=eo; ++io )
      {
        AttributedAObject
            *aao = dynamic_cast<AttributedAObject *>( *io );
        if( aao )
        {
          // get parent objects: one parent must be a graph with the
          // expected syntax
          const set<MObject *> & pl = (*io)->parents();
          set<MObject *>::const_iterator  ip, ep = pl.end();
          for( ip=pl.begin(); ip!=ep; ++ip )
            if( (*ip)->type() == AObject::GRAPH )
            {
              const AGraph * g = static_cast<const AGraph *>( *ip );
              const SyntaxedInterface
                  *si = g->attributed()->getInterface<SyntaxedInterface>();
              if( si->getSyntax() == synt )
              {
                aobj.insert( *io );
                aao->attributed()->setProperty(
                                GraphParams::graphParams()->attribute, val );
                (*io)->setChanged();
                (*io)->internalUpdate();
                break;
              }
            }
        }
      }
      for( io=aobj.begin(), eo=aobj.end(); io!=eo; ++io )
        (*io)->notifyObservers( obr );
    }
  }
}


string QObjectBrowser::canSend( QObjectBrowserWidget* br,
                                QTreeWidgetItem *item )
{
  if( !staticState().receivingBrowser )
    return "";
  return canSendToAny( br, item );
}


string QObjectBrowser::canSendToAny( QObjectBrowserWidget* br,
                                     QTreeWidgetItem *item )
{
  QObjectBrowserWidget::ItemDescr	descr;

  br->whatIs( item, descr );
  if( !descr.tobj )
    return "";

  Hierarchy	*hie = dynamic_cast<Hierarchy *>( descr.tobj );
  if( !hie )
    return "";

  string		synt, att, val;
  GenericObject	*ao;

  ASSERT( hie->tree()->getProperty( "graph_syntax", synt ) );

  switch( descr.type )
  {
  case QObjectBrowserWidget::ATTRIBUTE:
    att = descr.att;
    ao = descr.pao;
    break;
  case QObjectBrowserWidget::GOBJECT:
    ao = descr.ao;
    att = "name";	// hard coded up to now
    break;
  default:
    ao = 0;
    break;
  }
  if( !ao || att != "name" )
    return "";
  ao->getProperty( att, val );
  return val;
}


void QObjectBrowser::updateRightPanel()
{
  if( !d->rviewrefreshtimer )
    {
      d->rviewrefreshtimer = new QTimer( this );
      d->rviewrefreshtimer->setObjectName( 
        "QObjectBrowser_rviewrefreshtimer" );
      connect( d->rviewrefreshtimer, SIGNAL( timeout() ), this, 
	       SLOT( updateRightPanelNow() ) );
    }
  if( !d->rviewrefresh )
    {
      d->rviewrefresh = true;
      d->rviewrefreshtimer->setSingleShot( true );
      d->rviewrefreshtimer->start( 30 );
    }
}


void QObjectBrowser::updateRightPanelNow()
{
  using carto::shared_ptr;
  using ::Edge;

  if( !d->rviewrefresh )
    return;

  // cout << "QObjectBrowser::updateRightPanel()\n";
  map<QTreeWidgetItem *, GenericObject *>::const_iterator 
    ig, fg = d->lview->gObjects().end();
  unsigned	   ngosel = 0;
  QTreeWidgetItem	*cur = 0, *it, *other = 0;
  shared_ptr<AObject>   obj;
  QTreeWidgetItem  *item;

  for( ig=d->lview->gObjects().begin(); ig!=fg; ++ig )
  {
    it = (*ig).first;
    if( it->isSelected() )
    {
      QObjectBrowserWidget::ItemType      typ = d->lview->typeOf( it );
      if( typ == QObjectBrowserWidget::GOBJECT )
      {
        ++ngosel;
        cur = it;
        if( !other )
          other = it;
      }
    }
  }

  if( ( ngosel == 1 && cur == d->lastselectednode1 && ! d->lastselectednode2 )
        || ( ngosel == 2
        && ( ( cur == d->lastselectednode1 && other == d->lastselectednode2 )
        || ( cur == d->lastselectednode2 && other == d->lastselectednode1 ) ) )
     )
  {
    // in this case, don't update right panel because it shouldn't change
    d->rviewrefresh = false;
    /* cout << "don't refresh right panel\n";
    cout << ngosel << ", " << cur << ", " << d->lastselectednode1 << ", "
        << d->lastselectednode2 << endl;
    */
    return;
  }

  d->rview->clear();

  // cout << "ngosel: " << ngosel << endl;

  SelectFactory *fac = SelectFactory::factory();

  if( ngosel == 1 )
  {
    d->lastselectednode1 = cur;
    d->lastselectednode2 = 0;

    map<QTreeWidgetItem *, GenericObject *>::const_iterator
      ig = d->lview->gObjects().find( cur );
    Vertex	*v;

    if( ig != d->lview->gObjects().end() )
      if( ( v=dynamic_cast<Vertex *>( (*ig).second ) ) )
      {
        Vertex::const_iterator	ie, fe=v->end();
        for( ie=v->begin(); ie!=fe; ++ie )
        {
          d->rview->registerObject( *ie );
          if( (*ie)->getProperty( "ana_object", obj )
                && fac->isSelected( Group(), obj.get() ) )
          {
            item = d->rview->itemFor( obj.get() );
            if( item )
              item->setSelected( true );
          }
        }
      }
  }
  else if( ngosel == 2 )
  {
    d->lastselectednode1 = cur;
    d->lastselectednode2 = other;

    map<QTreeWidgetItem *, GenericObject *>::const_iterator
        ig = d->lview->gObjects().find( cur );
    Vertex        *v1, *v2;
    Vertex::iterator  iv, ev;
    Edge::iterator  ie, ee;

    if( ig != d->lview->gObjects().end()
        && ( v1=dynamic_cast<Vertex *>( (*ig).second ) ) )
    {
      ig = d->lview->gObjects().find( other );
      if( ig != d->lview->gObjects().end()
          && ( v2=dynamic_cast<Vertex *>( (*ig).second ) ) )
      {
        set<Edge *> edgs;
        // get edges between v1 and v2
        for( iv=v1->begin(), ev=v1->end(); iv!=ev; ++iv )
          for( ie=(*iv)->begin(), ee=(*iv)->end(); ie!=ee; ++ie )
            if( *ie == v2 )
            {
              edgs.insert( *iv );
              break;
            }
        set<Edge *>::const_iterator  ie, fe=edgs.end();
        for( ie=edgs.begin(); ie!=fe; ++ie )
        {
          d->rview->registerObject( *ie );
          if( (*ie)->getProperty( "ana_object", obj )
                && fac->isSelected( Group(), obj.get() ) )
          {
            item = d->rview->itemFor( obj.get() );
            if( item )
              item->setSelected( true );
          }
        }
      }
    }
  }
  else
  {
    d->lastselectednode1 = 0;
    d->lastselectednode2 = 0;
  }

  d->rviewrefresh = false;
}


void QObjectBrowser::rightButtonClickedSlot( QTreeWidgetItem * item, 
					     const QPoint & pos )
{
  if( !item )
    return;

  d->lastActivePanel = 0;	// left

  SelectFactory	*fac = SelectFactory::factory();

  d->lview->setCurrentItem( item );
  Tree	tr( true, "menu" );
  buildSpecificMenuTree( d->lview, item, tr );
  fac->handleSelectionMenu( this, pos.x(), pos.y(), &tr );
}


void QObjectBrowser::rightButtonRightPanel( QTreeWidgetItem * item, 
					    const QPoint & pos )
{
  if( !item )
    return;

  d->lastActivePanel = 1;	// right

  SelectFactory	*fac = SelectFactory::factory();

  d->rview->setCurrentItem( item );
  Tree	tr( true, "menu" );
  buildSpecificMenuTree( d->rview, item, tr );
  fac->handleSelectionMenu( this, pos.x(), pos.y(), &tr );
}


void QObjectBrowser::doubleClickedSlot( QTreeWidgetItem* item, int )
{
  d->lastActivePanel = 0;	// left
  d->lview->setCurrentItem( item );
  Tree	tr( true, "menu" );
  Tree* def = buildSpecificMenuTree( d->lview, item, tr );
  if( def )
  {
    // avoid opening item on a double-click which causes another action
    item->setExpanded( !item->isExpanded() );
    void	(*func)( void * ) = 0;
    void	*clientdata = 0;

    def->getProperty( "callback", func );
    def->getProperty( "client_data", clientdata );
    func( clientdata );	// execute callback
  }
}


Tree* QObjectBrowser::buildSpecificMenuTree( QObjectBrowserWidget* br, 
					     QTreeWidgetItem* item, Tree & tr )
{
  QObjectBrowserWidget::ItemDescr	descr;

  br->whatIs( item, descr );
  Tree					*t2;
  Tree					*def = 0;

  switch( descr.type )
    {
    case QObjectBrowserWidget::AOBJECT:
      if( !gObject( br, item, descr.type ) )
	break;
    case QObjectBrowserWidget::GOBJECT:
      if( descr.ao && descr.ao->hasProperty( "name" ) )
	{
	  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
						  "Modify name" ) );
	  t2->setProperty( "callback", &modifNameStatic );
	  t2->setProperty( "client_data", (void *) this );
	  tr.insert( t2 );
	  def = t2;
	}
      if( descr.ao && descr.ao->hasProperty( "label" ) )
	{
	  t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
						  "Modify label" ) );
	  t2->setProperty( "callback", &modifLabelStatic );
	  t2->setProperty( "client_data", (void *) this );
	  tr.insert( t2 );
	}
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Add attribute..." ) );
      t2->setProperty( "callback", &addAttributeStatic );
      t2->setProperty( "client_data", (void *) this );
      tr.insert( t2 );
      break;
    case QObjectBrowserWidget::ATTRIBUTE:
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Modify attribute..." ) );
      t2->setProperty( "callback", &modifyAttributeStatic );
      t2->setProperty( "client_data", (void *) this );
      tr.insert( t2 );
      def = t2;
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Delete attribute" ) );
      t2->setProperty( "callback", &removePropertyStatic );
      t2->setProperty( "client_data", (void *) this );
      tr.insert( t2 );
      break;
    default:
      break;
    }

  string	val = canSendToAny( br, item );
  if( !val.empty() )
  {
    if( d->editMode & SENDEDIT )
    {
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                              "Send value " ) + val );
      t2->setProperty( "callback", &sendModeSelection );
      t2->setProperty( "client_data", (void *) this );
      tr.insert( t2 );
      def = t2;
    }
    else
    {
      QString msg = QSelectMenu::tr( "Set value " ) + val.c_str()
          + QSelectMenu::tr( " as \"" )
          + GraphParams::graphParams()->attribute.c_str()
          + QSelectMenu::tr( "\" property to any selected object" );
      t2 = new Tree( true, msg.toStdString() );
      t2->setProperty( "callback", &setAttributeToAllSelected );
      t2->setProperty( "client_data", (void *) this );
      tr.insert( t2 );
    }
  }
  if( def )
    def->setProperty( "default", true );
  return( def );
}


void QObjectBrowser::modifyAttributeStatic( void* parent )
{
  ((QObjectBrowser *) parent)->modifyAttribute();
}


void QObjectBrowser::modifyAttribute()
{
  QString		msg = tr( "Modify attribute " );

  if( d->editMode & EDIT || d->editor || staticState().receivingBrowser )
    {
      QMessageBox::warning( this, msg, 
			    tr( "An editor is already open. close it first." 
				) );
      return;
    }

  QObjectBrowserWidget	*lview;
  if( d->lastActivePanel == 0 )
    lview = d->lview;
  else
    lview = d->rview;

  QTreeWidgetItemIterator it( lview, QTreeWidgetItemIterator::Selected );
  set<QTreeWidgetItem *> items;
  set<GenericObject *>  objs;
  GenericObject         *ao;
  QTreeWidgetItem        *item;
  string                att, att2;

  while( *it )
  {
    item = *it;
    ao = attributeCaract( lview, item, att );
    if( !ao )
    {
      QMessageBox::warning( this, tr( "Modify attribute" ),
                            tr( "Cannot determine attribute characteristics"
                              ) );
      return;
    }
    if( att2.empty() )
      att2 = att;
    if( att.empty() || att2 != att )
    {
      QMessageBox::warning( this, tr( "Modify attribute" ),
                            tr( "Can only edit the same attribute on multiple "
                                "items"
                              ) );
      return;
    }
    objs.insert( ao );
    items.insert( item );
    ++it;
  }

  if( items.empty() )
  {
    QMessageBox::warning( this, tr( "Modify attribute" ),
                          tr( "Item not found" ) );
    return;
  }

  bool	edited;
  if( !editAttribute( objs, att, lview, items, edited ) )
    QMessageBox::warning( this, tr( "Modify attribute" ), 
			  tr( "I cannot edit this attribute type" ) );
  else if( edited )
  {
    //	find parent AObject
    set<QTreeWidgetItem *>::iterator ii, ei = items.end();
    for( ii=items.begin(); ii!=ei; ++ii )
    {
      AObject		*paro = aObject( lview, *ii );
      if( paro )
	{
	  paro->setChanged();
	  paro->internalUpdate();
	  paro->notifyObservers( this );
	}
      else
	cerr << "Can't find parent AObject for attribute\n";
    }
  }
}


void QObjectBrowser::modifNameStatic( void* parent )
{
  ((QObjectBrowser *) parent)->modifAttrib( "name" );
}


void QObjectBrowser::modifLabelStatic( void* parent )
{
  ((QObjectBrowser *) parent)->modifAttrib( "label" );
}


void QObjectBrowser::modifAttrib( const string & att )
{
  QString		msg = tr( "Modify attribute " ) + att.c_str();

  if( staticState().receivingBrowser )
  {
    QMessageBox::warning( this, msg,
                          tr( "An editor is already open. close it first."
                              ) );
    return;
  }

  QObjectBrowserWidget	*lview;
  if( d->lastActivePanel == 0 )
    lview = d->lview;
  else
    lview = d->rview;

  QTreeWidgetItemIterator it( lview, QTreeWidgetItemIterator::Selected );
  set<QTreeWidgetItem *> items;
  set<GenericObject *>  objs;
  GenericObject         *ao;
  QTreeWidgetItem        *item;

  while( *it )
  {
    item = *it;
    QObjectBrowserWidget::ItemDescr	descr;

    lview->whatIs( item, descr );
    ao = descr.ao;
    if( !ao )
    {
      QMessageBox::warning( this, msg,
                            tr( "Cannot determine attribute characteristics"
                              ) );
      return;
    }

    item = lview->itemFor( item, QObjectBrowserWidget::ATTRIBUTE, att );
    if( !item )
    {
      QMessageBox::warning( this, msg,
                            tr( "Can't find attribute in tree.\nTry using "
                                "\"add attribute (?)\"" ) );
      return;
    }

    objs.insert( ao );
    items.insert( item );
    ++it;
  }

  if( items.empty() )
  {
    QMessageBox::warning( this, tr( "Modify attribute" ),
                          tr( "Item not found" ) );
    return;
  }

  bool	edited;
  if( !editAttribute( objs, att, lview, items, edited ) )
    QMessageBox::warning( this, msg,
			  tr( "I cannot edit this attribute type" ) );
  else if( edited )
    {
      //	find parent AObject
      set<QTreeWidgetItem *>::iterator ii, ei = items.end();
      for( ii=items.begin(); ii!=ei; ++ii )
      {
        AObject		*paro = aObject( lview, *ii );
        if( paro )
	{
	  paro->setChanged();
	  paro->internalUpdate();
	  paro->notifyObservers( this );
	}
        else
	 cerr << "Can't find parent AObject for attribute\n";
      }
    }
}


void QObjectBrowser::addAttributeStatic( void* parent )
{
  ((QObjectBrowser *) parent)->addAttribute();
}


void QObjectBrowser::addAttribute()
{
  if( staticState().receivingBrowser )
  {
    QMessageBox::warning( this, tr( "Add attribute" ), 
                          tr( "An editor is already open. close it first." ) );
    return;
  }

  QObjectBrowserWidget	*lview;
  if( d->lastActivePanel == 0 )
    lview = d->lview;
  else
    lview = d->rview;
  QTreeWidgetItem		*item = lview->currentItem();

  if( !item )
  {
    QMessageBox::warning( this, tr( "Add attribute" ), 
                          tr( "Item not found" ) );
    return;
  }

  int			t = lview->typeOf( item );
  GenericObject	*ao = gObject( lview, item, t );

  if( !ao )
  {
    QMessageBox::warning( this, tr( "Add attribute" ), 
                          tr( "Cannot determine attributed object" ) );
    return;
  }

  // ask new attrib name & type

  SyntaxSet	sset = AttDescr::descr()->syntaxSet();

  SyntaxedInterface	*si = ao->getInterface<SyntaxedInterface>();
  string		snt;
  if( si && si->hasSyntax() )
    snt = si->getSyntax();
  else
    snt = ao->type();

  AttributedChooser ach( *ao, sset, true, theAnatomist->getQWidgetAncestor(), 
    ( tr( "New attribute in " ) + snt.c_str() ).toStdString().c_str() );
  if( ach.exec() )
  {
    string			name = ach.attName(), type = ach.attType();
    SyntaxSet::iterator	iss = sset.find( snt );
    bool			newsynt = ( iss == sset.end() );

    if( newsynt )
    {
      sset[ snt ];
      iss = sset.find( snt );
    }

    SemanticSet::iterator	is = (*iss).second.find( name );
    bool			newsem = ( is == (*iss).second.end() );

    if( newsem )
    {
      (*iss).second[ name ].type = type;
      (*iss).second[ name ].needed = false;
    }
    else if( (*is).second.type != type )
    {
      QString	msg = tr( "Conflicting type for attribute " ) 
        + name.c_str() + " : " + tr( "exists as " ) 
        + (*is).second.type.c_str() + ", " 
        + tr( "so cannot be edited as " ) + type.c_str();
      QMessageBox::warning( this, tr( "Add attribute" ), msg );
      return;
    }

    if( newsynt || newsem )
      AttDescr::descr()->setSyntax( sset );

    bool	edited, registered;
    QTreeWidgetItem	*newitem;

    if( ao->hasProperty( name ) )	// already has got it
    {
      registered = true;

      unsigned	i, n = item->childCount();

      for( i=0; i<n; ++i )
      {
        newitem = item->child( i );
        if( newitem->text( 0 ).toStdString() == name )
          break;
      }
      if( n == 0 )	// not found
      {
        QMessageBox::warning( this, tr( "Add attribute" ), 
                              tr( "Attribute exists and I can't get it!" 
                                  "\n(not direct child in tree ?)\n" 
                                  "try clicking it and selecting " 
                                  "'Modify'" ) );
        return;
      }
    }
    else
    {
      registered = false;
      newitem = new QTreeWidgetItem( item );
      newitem->setText( 0, name.c_str() );
      newitem->setText( 1, type.c_str() );
    }

    QTreeWidgetItem	*parent = newitem->parent();

    for( ; parent; parent = parent->parent() )
      if( !parent->isExpanded() )
        parent->setExpanded( true );
    lview->scrollToItem( newitem, QTreeWidget::EnsureVisible );

    if( !editAttribute( ao, name, lview, newitem, edited ) )
      QMessageBox::warning( this, tr( "Add attribute" ),
                            tr( "I cannot edit this attribute type" ) );
    if( !edited )
    {
      if( d->editor )
      {
        // this must be delayed when the editor is non-modal
        if( !registered )
          d->tempAddedItems.insert( newitem );
        d->tempAddedNewSyntax = newsynt;
        d->tempAddedSyntax = snt;
        if( newsem )
          d->tempAddedName = name;
        else
          d->tempAddedName.clear();
      }
      else
      {
        if( !registered )
          delete newitem;
        if( newsynt )
        {
          sset.erase( iss );
          AttDescr::descr()->setSyntax( sset );
        }
        else if( newsem )
        {
          (*iss).second.erase( name );
          AttDescr::descr()->setSyntax( sset );
        }
      }
    }
    else
    {
      if( !registered )
        lview->registerAttribute( newitem );

      //	find parent AObject
      AObject		*paro = aObject( lview, newitem );
      if( paro )
      {
        paro->setChanged();
        paro->notifyObservers( this );
      }
      else
        cerr << "Can't find parent AObject for attribute\n";
    }
  }
}


void QObjectBrowser::removePropertyStatic( void* parent )
{
  ((QObjectBrowser *) parent)->removeProperty();
}


void QObjectBrowser::removeProperty()
{
  QString		msg = tr( "Delete attribute " );

  if( staticState().receivingBrowser )
    {
      QMessageBox::warning( this, msg, 
			    tr( "An editor is already open. close it first." 
				) );
      return;
    }

  QObjectBrowserWidget	*lview;
  if( d->lastActivePanel == 0 )
    lview = d->lview;
  else
    lview = d->rview;
  QTreeWidgetItem		*item = lview->currentItem();
  string		att;

  if( !item )
    {
      QMessageBox::warning( this, tr( "Delete attribute" ), 
			    tr( "Item not found" ) );
      return;
    }

  GenericObject	*ao = attributeCaract( lview, item, att );

  if( !ao )
    {
      QMessageBox::warning( this, tr( "Delete attribute" ), 
			    tr( "Cannot determine attribute characteristics" 
				) );
      return;
    }

  msg += att.c_str();
  if( QMessageBox::information( this, tr( "Delete attribute" ), msg, 
				tr( "OK" ), tr( "Cancel" ) ) == 0 )
    {
      lview->unregisterItem( item );
      // *** WARNING ***
      // possible memory leak if the attribute stored a pointer 
      // use with care ! ...
      ao->removeProperty( att );

      //	find parent AObject
      AObject		*paro = aObject( lview, item );
      if( paro )
	{
	  paro->setChanged();
	  paro->notifyObservers( this );
	}
      else
	cerr << "Can't find parent AObject for attribute\n";
    }
}


GenericObject* 
QObjectBrowser::attributeCaract( const QObjectBrowserWidget* br, 
				 const QTreeWidgetItem* item, 
				 string & att )
{
  att = item->text( 0 ).toStdString();

  QTreeWidgetItem		*parent = item->parent();
  QObjectBrowserWidget::ItemType	t;

  while( parent 
	 && (t = br->typeOf( parent )) != QObjectBrowserWidget::GOBJECT 
	 && t != QObjectBrowserWidget::AOBJECT )
    parent = parent->parent();
  if( !parent )
    return( 0 );

  return( gObject( br, parent, t ) );
}


GenericObject* QObjectBrowser::gObject( const QObjectBrowserWidget* br, 
					   const QTreeWidgetItem* item, 
					   int t )
{
  GenericObject	*ao = 0;

  if( t == QObjectBrowserWidget::AOBJECT )
    {
      AObject 
	*obj = (*br->aObjects().find( (QTreeWidgetItem *) item )).second;

      AttributedAObject* ato = dynamic_cast<AttributedAObject *>( obj );
      if( ato )
	ao = ato->attributed();
    }
  else
    ao = (*br->gObjects().find( (QTreeWidgetItem *) item )).second;

  return( ao );
}


AObject* QObjectBrowser::aObject( const QObjectBrowserWidget* br, 
				  const QTreeWidgetItem* item )
{
  using carto::shared_ptr;

  int 			type;
  shared_ptr<AObject>	sobj;
  AObject               *obj = 0;
  GenericObject	*ao;

  for( type = br->typeOf( (QTreeWidgetItem *) item ); 
       item && type!=QObjectBrowserWidget::AOBJECT; 
       item = item->parent(),
       type = item ?
       br->typeOf( (QTreeWidgetItem *) item ) : QObjectBrowserWidget::UNKNOWN )
    if( type == QObjectBrowserWidget::GOBJECT )
    {
      ao = gObject( br, item, type );
      if( ao->getProperty( "ana_object", sobj ) )
      {
        obj = sobj.get();
        break;
      }
    }

  if( type== QObjectBrowserWidget::AOBJECT )
    {
      return( (AObject *) (*br->aObjects().find( (QTreeWidgetItem *) 
						 item )).second );
    }

  if( !obj && d->lastActivePanel != 0 )
    {
      //	take AObject selected in left panel
      SelectFactory	*fac = SelectFactory::factory();
      set<AObject *>::const_iterator	io, fo=_sobjects.end();

      unsigned	n = 0;

      for( io=_sobjects.begin(); io!=fo; ++io )
	if( fac->isSelected( Group(), *io ) )
	  {
	    ++n;
	    obj = *io;
	  }
      if( n > 1 )
	obj = 0;	// multi-selection, should be unique
    }
  return( obj );
}


void QObjectBrowser::registerTypeEditor( const string & type, EditFunc func )
{
  staticState().typeEditors[ type ] = func;
}


void QObjectBrowser::registerAttributeEditor( const string & syntax, 
					      const string & att, 
					      EditFunc func )
{
  staticState().attEditors[ syntax ][ att ] = func;
}


bool QObjectBrowser::stringEditor( const std::set<GenericObject*> & objs,
                                   const string & att,
				   QObjectBrowserWidget* br, 
                                   const std::set<QTreeWidgetItem*> & items )
{
  if( objs.empty() )
    return false;
  string	attval = "";
  (*objs.begin())->getProperty( att, attval );
  QTreeWidgetItem  *item = br->currentItem();
  if( !item || items.find( item ) == items.end() )
    item = *items.begin();
  QRect		pos = br->visualItemRect( item );
  if( !pos.isValid() )
    pos = br->visualItemRect( item->parent() );
  QPoint	xy = br->mapToGlobal( pos.topLeft() );

  if( pos.height() < 10 )
    pos.setHeight( 20 );

  int	dx = br->columnViewportPosition( 2 );
  int	w = br->columnWidth( 2 );

  if( dx < 0 )
    {
      w += dx;
      dx = 0;
    }

  if( w > ((int) pos.width()) - (int) dx )
    w = pos.width() - dx;
  if( w < 30 )
    w = 30;

  QStringEdit	ed( attval, xy.x() + dx, xy.y(), w, pos.height(), theAnatomist->getQWidgetAncestor(),
                    att.c_str(), Qt::WindowStaysOnTopHint );
  if( ed.exec() )
  {
    set<GenericObject *>::const_iterator  io, eo = objs.end();
    for( io=objs.begin(); io!=eo; ++io )
      (*io)->setProperty( att, ed.text() );
    set<QTreeWidgetItem *>::const_iterator il, el = items.end();
    for( il=items.begin(); il!=el; ++il )
      (*il)->setText( 2, ed.text().c_str() );
    return true;
  }
  else
    return false;
}


bool QObjectBrowser::intEditor( const std::set<GenericObject*> & objs,
                                const string & att, 
				QObjectBrowserWidget* br,
                                const std::set<QTreeWidgetItem*> & items )
{
  if( objs.empty() )
    return false;
  int		attval = 0;
  char		str[30];
  (*objs.begin())->getProperty( att, attval );
  QTreeWidgetItem  *item = br->currentItem();
  if( !item || items.find( item ) == items.end() )
    item = *items.begin();
  QRect		pos = br->visualItemRect( item );
  if( !pos.isValid() )
    pos = br->visualItemRect( item->parent() );
  QPoint	xy = br->mapToGlobal( pos.topLeft() );

  sprintf( str, "%d", attval );
  if( pos.height() < 10 )
    pos.setHeight( 20 );

  int	dx = br->columnViewportPosition( 2 );
  int	w = br->columnWidth( 2 );

  if( dx < 0 )
    {
      w += dx;
      dx = 0;
    }

  if( w > ((int) pos.width()) - (int) dx )
    w = pos.width() - dx;
  if( w < 30 )
    w = 30;

  QStringEdit	ed( str, xy.x() + dx, xy.y(), w, pos.height(), 
                    theAnatomist->getQWidgetAncestor(), att.c_str(), 
                    Qt::WindowStaysOnTopHint /*WStyle_Customize | WStyle_NoBorder
                        | WStyle_Tool*/ );
  ed.lineEdit()->setValidator( new QIntValidator( ed.lineEdit() ) );
  if( ed.exec() )
  {
    sscanf( ed.text().c_str(), "%d", &attval );
    set<GenericObject *>::const_iterator  io, eo = objs.end();
    for( io=objs.begin(); io!=eo; ++io )
      (*io)->setProperty( att, attval );
    set<QTreeWidgetItem *>::const_iterator il, el = items.end();
    for( il=items.begin(); il!=el; ++il )
      (*il)->setText( 2, ed.text().c_str() );
    return true;
  }
  else
    return false;
}


bool QObjectBrowser::floatEditor( const std::set<GenericObject*> & objs,
                                  const string & att,
				  QObjectBrowserWidget* br, 
                                  const std::set<QTreeWidgetItem*> & items )
{
  if( objs.empty() )
    return false;
  float		attval = 0;
  char		str[30];
  (*objs.begin())->getProperty( att, attval );
  QTreeWidgetItem  *item = br->currentItem();
  if( !item || items.find( item ) == items.end() )
    item = *items.begin();
  QRect		pos = br->visualItemRect( item );
  if( !pos.isValid() )
    pos = br->visualItemRect( item->parent() );
  QPoint	xy = br->mapToGlobal( pos.topLeft() );

  sprintf( str, "%g", attval );
  if( pos.height() < 10 )
    pos.setHeight( 20 );

  int	dx = br->columnViewportPosition( 2 );
  int	w = br->columnWidth( 2 );

  if( dx < 0 )
    {
      w += dx;
      dx = 0;
    }

  if( w > ((int) pos.width()) - (int) dx )
    w = pos.width() - dx;
  if( w < 30 )
    w = 30;

  QStringEdit	ed( str, xy.x() + dx, xy.y(), w, pos.height(), theAnatomist->getQWidgetAncestor(), att.c_str(), 
                    Qt::WindowStaysOnTopHint /*WStyle_Customize | WStyle_NoBorder
                        | WStyle_Tool*/ );
  ed.lineEdit()->setValidator( new QDoubleValidator( -FLT_MAX, FLT_MAX, 30, 
						     ed.lineEdit() ) );
  if( ed.exec() )
  {
    sscanf( ed.text().c_str(), "%g", &attval );
    set<GenericObject *>::const_iterator  io, eo = objs.end();
    for( io=objs.begin(); io!=eo; ++io )
      (*io)->setProperty( att, attval );
    set<QTreeWidgetItem *>::const_iterator il, el = items.end();
    for( il=items.begin(); il!=el; ++il )
      (*il)->setText( 2, ed.text().c_str() );
    return true;
  }
  else
    return false;
}


bool QObjectBrowser::doubleEditor( const std::set<GenericObject*> & objs,
                                   const string & att,
				   QObjectBrowserWidget* br, 
                                   const std::set<QTreeWidgetItem*> & items )
{
  if( objs.empty() )
    return false;
  double	attval = 0;
  float		tmp;
  char		str[30];
  (*objs.begin())->getProperty( att, attval );
  QTreeWidgetItem  *item = br->currentItem();
  if( !item || items.find( item ) == items.end() )
    item = *items.begin();
  QRect		pos = br->visualItemRect( item );
  if( !pos.isValid() )
    pos = br->visualItemRect( item->parent() );
  QPoint	xy = br->mapToGlobal( pos.topLeft() );

  sprintf( str, "%g", attval );
  if( pos.height() < 10 )
    pos.setHeight( 20 );

  int	dx = br->columnViewportPosition( 2 );
  int	w = br->columnWidth( 2 );

  if( dx < 0 )
    {
      w += dx;
      dx = 0;
    }

  if( w > ((int) pos.width()) - (int) dx )
    w = pos.width() - dx;
  if( w < 30 )
    w = 30;

  QStringEdit	ed( str, xy.x() + dx, xy.y(), w, pos.height(), theAnatomist->getQWidgetAncestor(), att.c_str(), 
                    Qt::WindowStaysOnTopHint /*WStyle_Customize | WStyle_NoBorder
                        | WStyle_Tool*/ );
  ed.lineEdit()->setValidator( new QDoubleValidator( -HUGE_VAL, HUGE_VAL, 30, 
						     ed.lineEdit() ) );
  if( ed.exec() )
  {
    sscanf( ed.text().c_str(), "%g", &tmp );
    set<GenericObject *>::const_iterator  io, eo = objs.end();
    for( io=objs.begin(); io!=eo; ++io )
      (*io)->setProperty( att, (double) tmp );
    set<QTreeWidgetItem *>::const_iterator il, el = items.end();
    for( il=items.begin(); il!=el; ++il )
      (*il)->setText( 2, ed.text().c_str() );
    return true;
  }
  else
    return false;
}


bool QObjectBrowser::editAttribute( GenericObject* ao, const string & att, 
                                    QObjectBrowserWidget* br,
                                    QTreeWidgetItem* item, bool & edited )
{
  set<GenericObject *> objs;
  objs.insert( ao );
  set<QTreeWidgetItem *> items;
  items.insert( item );
  return editAttribute( objs, att, br, items, edited );
}


bool QObjectBrowser::editAttribute( const set<GenericObject*> & objs,
                                    const string & att,
				    QObjectBrowserWidget* br, 
				    const set<QTreeWidgetItem*> & items,
                                    bool & edited )
{
  edited = false;

  Static  & s = staticState();

  set<GenericObject *>::const_iterator io, eo = objs.end();
  GenericObject *ao;
  EditFunc  func = 0, func2 = 0;
  const SyntaxSet & sset = AttDescr::descr()->syntaxSet();

  for( io=objs.begin(); io!=eo; ++io )
  {
    ao = *io;
    SyntaxedInterface	*si = ao->getInterface<SyntaxedInterface>();
    string		snt;
    if( si && si->hasSyntax() )
      snt = si->getSyntax();
    else
      snt = ao->type();

    map<string, map<string, EditFunc> >::const_iterator 
      is = s.attEditors.find( snt );

    if( is != s.attEditors.end() )	// syntax found
    {
      map<string, EditFunc>::const_iterator	ia = (*is).second.find( att );

      if( ia != (*is).second.end() )	// attribute found
      {
        func = ia->second;
        if( !func2 )
          func2 = func;
        if( func != func2 )
          return false; // several unmatching editors needed
        continue;
      }
    }

    // not found in particular attributes: look in general type editors
    SyntaxSet::const_iterator	iss = sset.find( snt );
    string			type;

    if( iss != sset.end() )
    {
      SemanticSet::const_iterator	ise = (*iss).second.find( att );

      if( ise != (*iss).second.end() )
        type = (*ise).second.type;
    }
    if( type.empty() )
    {
      try
      {
        Object	obj = ao->getProperty( att );
        if( obj.isNone() )
          return false;
        type = obj->type();
      }
    catch( ... )
      {
        return false;
      }
    }

    map<string, EditFunc>::const_iterator it = s.typeEditors.find( type );

    if( it != s.typeEditors.end() )
    {
      func = it->second;
      if( !func2 )
        func2 = func;
      if( func != func2 )
        return false; // several unmatching editors needed
    }
  }

  if( !func ) // no editor
    return false;

  if( func( objs, att, br, items ) )
    edited = true;
  return true ;
}


bool QObjectBrowser::labelEditor( const set<GenericObject*> & objs,
                                  const string & att,
				  QObjectBrowserWidget* br, 
				  const set<QTreeWidgetItem*> & items )
{
  QWidget		*pw = br->parentWidget();
  QObjectBrowser	*tbr;

  for( tbr=dynamic_cast<QObjectBrowser *>( pw ); pw && !tbr;
       pw=pw->parentWidget(), tbr=dynamic_cast<QObjectBrowser *>( pw ) ) {}
  ASSERT( tbr );

  if( staticState().receivingBrowser )	// special editor already in use
    return( stringEditor( objs, att, br, items ) );

  // verify we are not in a hierarchy and get syntax of top-level AttObject

  QTreeWidgetItem	*par = (*items.begin())->parent(), *par2 = par;

  while( par )
    {
      par2 = par;
      par = par->parent();
    }

  if( !par2 )	// parent AObject not recognized as GenericObject
    return( stringEditor( objs, att, br, items ) );

  const map<QTreeWidgetItem *, AObject *>	& aobj = br->aObjects();
  map<QTreeWidgetItem *, AObject *>::const_iterator	ia, fa = aobj.end();

  ia = aobj.find( par2 );
  if( ia == fa )	// top parent not recognized as AObject
    return( stringEditor( objs, att, br, items ) );

  AttributedAObject   *aao = dynamic_cast<AttributedAObject *>( (*ia).second );
  if( !aao )
    return( stringEditor( objs, att, br, items ) );

  GenericObject	*pao = aao->attributed();
  SyntaxedInterface	*si = pao->getInterface<SyntaxedInterface>();
  string		synt;
  if( si && si->hasSyntax() )
    synt = si->getSyntax();
  else
    synt = pao->type();

  //	now check if there is a matching Hierarchy in the appli
  set<AObject *>	obj = theAnatomist->getObjects();
  set<AObject *>::const_iterator	io, fo=obj.end();
  Hierarchy				*hie;
  GenericObject			*hao;
  string				hsynt;
  bool					hierenabled = false;
  set<AWindow *>::const_iterator	iw, fw;
  QObjectBrowser			*bro;

  for( io=obj.begin(); io!=fo; ++io )
    {
      hie = dynamic_cast<Hierarchy *>( *io );
      if( hie )
	{
	  hao = hie->attributed();
	  if( hao->getProperty( "graph_syntax", hsynt ) && hsynt == synt )
	    {
	      hierenabled = true;
	      const set<AWindow *> & win = hie->WinList();
	      for( iw=win.begin(), fw=win.end(); iw!=fw; ++iw )
		{
		  bro = dynamic_cast<QObjectBrowser *>( *iw );
		  if( bro )
		    bro->setMode( bro->mode() | SENDEDIT );
		}
	    }
	}
    }
  if( !hierenabled )	// not found
    return( stringEditor( objs, att, br, items ) );

  tbr->setMode( tbr->mode() | EDIT );

  //	same as other editors
  string	attval = "";
  (*objs.begin())->getProperty( att, attval );
  QTreeWidgetItem *item = br->currentItem();
  if( !item || items.find( item ) == items.end() )
    item = *items.begin();
  QRect		pos = br->visualItemRect( item );
  if( !pos.isValid() )
    pos = br->visualItemRect( item->parent() );
  QPoint	xy = br->viewport()->mapToGlobal( pos.topLeft() );

  if( pos.height() < 10 )
    pos.setHeight( 20 );

  int	dx = br->columnViewportPosition( 2 );
  int	w = br->columnWidth( 2 );

  if( dx < 0 )
    {
      w += dx;
      dx = 0;
    }

  if( w > ((int) pos.width()) - (int) dx )
    w = pos.width() - dx;
  if( w < 30 )
    w = 30;

  tbr->d->editor = new QLabelEdit( attval, xy.x() + dx, xy.y(), w, 
                                   pos.height(), tbr, objs, att, items, theAnatomist->getQWidgetAncestor(),
                                   att.c_str(), Qt::WindowStaysOnTopHint );
  tbr->d->editor->show();

  return false;
}


void QObjectBrowser::setMode( unsigned mode )
{
  if( mode != d->editMode )
    {
      Static  & s = staticState();
      if( d->editMode & EDIT )
	s.receivingBrowser = 0;
      d->editMode = mode;
      d->modeWid->setText( modeString().c_str() );
      d->modeWid->setBackgroundRole( QPalette::Window );
      QPalette pal = d->modeWid->palette();
      if( d->editMode != 0 )
        pal.setBrush(QPalette::Window, QColor( 255, 192, 192 ) );
      else
        pal.setBrush( QPalette::Window,
          d->statbar->palette().brush( QPalette::Window ) );
      d->modeWid->setPalette( pal );
      d->modeWid->update();
      if( mode & EDIT )
        s.receivingBrowser = this;
      if( !(mode & EDIT) && d->editor )
        {
          delete d->editor;
          d->editor = 0;
        }
    }
}


string QObjectBrowser::modeString() const
{
  bool		fst = true;
  Static        & s = staticState();
  unsigned	i, n = s.Modes.size();
  string	modestr;

  for( i=0; i<n; ++i )
    if( d->editMode & (0x1<<i) )
      {
	if( fst )
	  fst = false;
	else
	  modestr += " + ";
	modestr += s.Modes[ i+1 ];
      }
  if( fst )
    modestr = s.Modes[ 0 ];
  return( modestr );
}


void QObjectBrowser::editCancel()
{
  // delete temporary elements in case a property addition aborts
  set<QTreeWidgetItem *>::iterator i, e = d->tempAddedItems.end();
  for( i=d->tempAddedItems.begin(); i!=e; ++i )
    delete *i;
  d->tempAddedItems.clear();
  if( !d->tempAddedSyntax.empty() )
  {
    SyntaxSet ss = AttDescr::descr()->syntaxSet();
    SyntaxSet::iterator is = ss.find( d->tempAddedSyntax );
    if( is != ss.end() )
    {
      if( d->tempAddedNewSyntax )
        ss.erase( is );
      else if( !d->tempAddedName.empty() )
        is->second.erase( d->tempAddedName );
      if( d->tempAddedNewSyntax || !d->tempAddedName.empty() )
        AttDescr::descr()->setSyntax( ss );
    }
    d->tempAddedSyntax.clear();
    d->tempAddedName.clear();
    d->tempAddedNewSyntax = false;
  }

  d->editor = 0;

  set<AWindow *>			win = theAnatomist->getWindows();
  set<AWindow *>::const_iterator	iw, fw=win.end();
  QObjectBrowser			*br;

  for( iw=win.begin(); iw!=fw; ++iw )
    {
      br = dynamic_cast<QObjectBrowser *>( *iw );
      if( br )
	br->setMode( NORMAL );
    }
}


void QObjectBrowser::leftItemStartsRename( QTreeWidgetItem*, int )
{
}


void QObjectBrowser::leftItemCancelsRename( QTreeWidgetItem*, int )
{
}


void QObjectBrowser::leftItemRenamed( QTreeWidgetItem *, int, const QString & )
{
}


void QObjectBrowser::editValidate()
{
  QObjectBrowserWidget        *view;

  // take care of temporary elements in case a property addition
  set<QTreeWidgetItem *>::iterator i, e = d->tempAddedItems.end();
  for( i=d->tempAddedItems.begin(); i!=e; ++i )
  {
    view = (QObjectBrowserWidget *) (*i)->treeWidget();
    view->registerAttribute( *i );
  }
  d->tempAddedItems.clear();
  d->tempAddedSyntax.clear();
  d->tempAddedName.clear();
  d->tempAddedNewSyntax = false;

  set<GenericObject *>  objs = d->editor->attributedObjects();
  set<GenericObject *>::iterator  ig, eg=objs.end();
  for( ig=objs.begin(); ig!=eg; ++ig )
    (*ig)->setProperty( d->editor->attrib(), d->editor->text() );
  set<QTreeWidgetItem *> items = d->editor->items();
  set<QTreeWidgetItem *>::iterator il, el=items.end();
  set<AObject *>              aobj;
  /* cout << "editValidate, genobjects: " << objs.size() << ", listitems: "
      << items.size() << endl; */
  for( il=items.begin(); il!=el; ++il )
  {
    view = (QObjectBrowserWidget *) (*il)->treeWidget();
    //	find parent AObject
    AObject		*paro = aObject( view, *il );

    if( paro )
    {
      aobj.insert( paro );
      paro->setChanged();
    }
    else
      cerr << "can't find parent AObject for attribute (BUG !)\n";
  }
  editCancel();	// cleanup

  set<AObject *>::iterator ia, ea = aobj.end();
  for( ia=aobj.begin(); ia!=ea; ++ia )
    (*ia)->internalUpdate();
  // use a different loop because now the graph changed flags will be reset
  // only once
  for( ia=aobj.begin(); ia!=ea; ++ia )
    (*ia)->notifyObservers( this );
}


void 
QObjectBrowser::nomenclatureClick( Hierarchy* h,
                                   QObjectBrowserWidget::ItemDescr & descr, 
                                   set<AObject *> & tosel )
{
  using carto::shared_ptr;

  if( d->lview->buttonsAtLastEvent() != Qt::LeftButton )
    return; // if another button than the left is pressed: do nothing.

  // cout << "nomenclatureClick\n";
  string	attrib, gattrib;

  if( QGraphParam::theGP() )
    attrib = QGraphParam::theGP()->nomenclatureAttrib();
  if( attrib.empty() )
    attrib = "name";

  set<AWindow *>	win = theAnatomist->getWindowsInGroup( Group() );
  set<AWindow *>::const_iterator	iw, fw=win.end();
  string				synt;
  set<AObject *>::const_iterator	io, fo;
  AGraph				*ag;
  Graph					*g;
  set<Graph *>				gr;
  set<string>				vallist;
  MObject				*mobj;
  MObject::const_iterator		im, fm;

  if( !h->tree()->getProperty( "graph_syntax", synt ) )
    return;
  h->namesUnder( (Tree *) descr.ao, vallist );
  if( vallist.size() == 0 )
    //if( !descr.ao->getProperty( "name", val ) )
    return;

  //	find graphs in windows of the same group
  for( iw=win.begin(); iw!=fw; ++iw )
    {
      set<AObject *>	obj = (*iw)->Objects();
      for( io=obj.begin(), fo=obj.end(); io!=fo; ++io )
	{
	  if( (*io)->type() == AObject::GRAPH )
	    {
	      ag = dynamic_cast<AGraph *>( *io );
	      ASSERT( ag );
	      g = ag->graph();
	      if( g->getSyntax() == synt )	// matching syntax ?
	        gr.insert( g );
	    }
	  //	and graphs contained in objects (fusions...) in windows
	  else if( (mobj = dynamic_cast<MObject *>( *io ) ) )
	    for( im=mobj->begin(), fm=mobj->end(); im!=fm; ++im )
	      if( (*im)->type() == AObject::GRAPH )
		{
	          ag = dynamic_cast<AGraph *>( *im );
	          ASSERT( ag );
	          g = ag->graph();
	          if( g->getSyntax() == synt )	// matching syntax ?
	            gr.insert( g );
		}
	}
    }

  //	now select nodes in the graphs
  set<Graph *>::const_iterator	ig, fg=gr.end();
  set<Vertex *>			sv;
  set<Vertex *>::const_iterator	iv, fv;
  shared_ptr<AObject>		obj;
  set<string>::const_iterator	is, fs=vallist.end();
  string			ws, name;
  string::size_type		pos;
  bool				done;

  for( ig=gr.begin(); ig!=fg; ++ig )
  {
    gattrib = attrib;
    // if there a per-graph nomenclature property setting, use it
    try
    {
      gattrib = (*ig)->getProperty( "label_property" )->getString();
    }
    catch( ... )
    {
    }
    for( iv=(*ig)->begin(), fv=(*ig)->end(); iv!=fv; ++iv )
      if( (*iv)->getProperty( "ana_object", obj ) 
          && (*iv)->getProperty( gattrib, name ) )
      {
        done = false;
        while( !name.empty() && !done )
        {
          pos = name.find( '+' );
          if( pos == string::npos )
          pos = name.size();
          ws = name.substr( 0, pos );
          name.erase( 0, pos+1 );

          for( is=vallist.begin(); is!=fs; ++is )
          {
            if( ws == *is )
            {
              tosel.insert( obj.get() );
              done = true;
              break;
            }
            /*sv = (*ig)->getVerticesWith( attrib, *is );
            for( iv=sv.begin(), fv=sv.end(); iv!=fv; ++iv )
              if( (*iv)->getProperty( "ana_object", obj ) )
                tosel.insert( obj );*/
            }
        }
      }
  }
}


bool QObjectBrowser::colorEditor( const set<GenericObject*> & objs,
                                  const string & att,
                                  QObjectBrowserWidget* bw,
                                  const set<QTreeWidgetItem*> & )
{
  using carto::shared_ptr;

  if( objs.empty() )
    return 0;
  vector<int>	attval;
  GenericObject * ao = *objs.begin();
  try
  {
    Object value;
    value = ao->getProperty( att );
    if( value && value->size() >= 3 )
    {
      attval.reserve( value->size() );
      Object vit = value->objectIterator();
      for( ; vit->isValid(); vit->next() )
        attval.push_back( int( vit->currentValue()->getScalar() ) );
    }
  }
  catch( ... )
  {
  }
  if( attval.size() != 3 )
  {
    if( attval.size() > 4 )
      attval.resize( 4 );
    if( attval.size() < 3 )
      attval.resize( 4, 128 );
  }

  string	oname;
  ao->getProperty( "name", oname );
  SyntaxedInterface	*si = ao->getInterface<SyntaxedInterface>();
  string		snt;
  if( si && si->hasSyntax() )
    snt = si->getSyntax();
  else
    snt = ao->type();
  QString	name = tr( "Color for " ) + snt.c_str();
  if( !oname.empty() )
  {
    name += " ";
    name += oname.c_str();
  }

  bool neutralpha = true;
  int alpha = 255;
  if( attval.size() > 3 )
  {
    alpha = attval[3];
    neutralpha = false;
  }
  QColor col = QAColorDialog::getColor( 
    QColor( attval[0], attval[1], attval[2] ),
    theAnatomist->getQWidgetAncestor(), name.toStdString().c_str(), &alpha, &neutralpha );
  if( col.isValid() )
  {
    attval[0] = col.red();
    attval[1] = col.green();
    attval[2] = col.blue();
    if( neutralpha )
    {
      if( attval.size() > 3 )
        attval.resize( 3 );
    }
    else
    {
      if( attval.size() > 3 )
        attval[3] = alpha;
      else
        attval.push_back( alpha );
    }
    set<GenericObject *>::const_iterator  io, eo = objs.end();
    for( io=objs.begin(); io!=eo; ++io )
    {
      (*io)->setProperty( att, attval );
      Graph * graph = dynamic_cast<Graph *>( *io );
      if( graph )
      {
        GraphManip::setAttributeColor( *graph, att, attval );
        shared_ptr<AObject> ao;
        AGraph *ag;
        if( graph->getProperty( "ana_object", ao ) )
        {
          ag = dynamic_cast<AGraph *>( ao.get() );
          if( ag )
          {
            ag->SetMaterial( ag->GetMaterial() );
            ag->notifyObservers( bw );
          }
        }
      }
    }
    return true;
  }
  else
    return false;
}


void QObjectBrowser::rightSelectionChangedSlot()
{
  QBrowserUpdateEvent	*bue 
    = new QBrowserUpdateEvent( (QBrowserUpdateEvent::QtEvType) 
			       Event_BrowserUpdate );
  QApplication::postEvent( this, bue );
}


// This warning is triggered by -Wall even though the code is correct
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
bool QObjectBrowser::event( QEvent* ev )
{
  switch( ev->type() )
    {
    case Event_BrowserUpdate:
      updateRightSelectionChange( 0 );
      return true;
      break;
    default:
      break;
    }
  return( QWidget::event( ev ) );
}
#pragma GCC diagnostic pop


void QObjectBrowser::updateRightSelectionChange( int modifier )
{
  using carto::shared_ptr;
  using ::Edge;

  // cout << "QObjectBrowser::updateRightSelectionChange()\n";
  QTreeWidgetItem	*item, *cur = 0;
  unsigned              nsel = 0;
  set<AObject *>        so, unsel;
  QObjectBrowserWidget::ItemDescr       descr;
  QTreeWidgetItemIterator ilv( d->rview );
  Edge  *edg = 0;

  SelectFactory *fac = SelectFactory::factory();

  for( ; *ilv; ++ilv )
  {
    item = *ilv;
    if( item->isSelected() )
    {
      ++nsel;
      cur = item;
      d->rview->whatIs( cur, descr );
      if( descr.obj )
        so.insert( descr.obj );
    }
    else
    {
      d->rview->whatIs( item, descr );
      if( descr.obj )
      {
        if( fac->isSelected( Group(), descr.obj ) )
          unsel.insert( descr.obj );
      }
    }
    nsel += countSelected( item, cur );
  }

  fac->unselect( Group(), unsel );

  if( nsel == 0 )
  {
    fac->refresh();
    return;
  }

  if( nsel == 1 )
  {
    d->rview->whatIs( cur, descr );
    edg = 0;
    if( descr.type == QObjectBrowserWidget::GOBJECT )
      edg = dynamic_cast<Edge *>( descr.ao );
    else if( descr.type == QObjectBrowserWidget::AOBJECT )
    {
      AttributedAObject *aao = dynamic_cast<AttributedAObject *>( descr.obj );
      if( aao )
        edg = dynamic_cast<Edge *>( aao->attributed() );
    }
    else
    {
      fac->refresh();
      return;
    }

    if( !edg )
    {
      fac->refresh();
      return;
    }

    Edge::const_iterator	iv;
    Vertex		*v1, *v2;
    iv = edg->begin();
    v1 = *iv;
    ++iv;
    v2 = *iv;

    shared_ptr<AObject>	o1, o2, obj;

    if( descr.obj )
      so.insert( descr.obj );

    if( v1->getProperty( "ana_object", o1 ) )
    {
      if( modifier == 0 )
      {
        if( fac->isSelected( Group(), o1.get() ) )
          so.insert( o1.get() );
      }
      else
        so.insert( o1.get() );
    }
    if(v2->getProperty( "ana_object", o2 ) )
    {
      if( modifier == 0 )
      {
        if( fac->isSelected( Group(), o2.get() ) )
          so.insert( o2.get() );
      }
      else
        so.insert( o2.get() );
    }

    if( edg->getProperty( "ana_object", obj ) )
      so.insert( obj.get() );
  }

  if( nsel == 1 )
    fac->unselectAll( Group() );
  fac->select( Group(), so );
  fac->refresh();
  /*QTreeWidgetItem	*newitem = d->rview->itemFor( edg );
  if( newitem )
    {
      d->dontProcessNextEvents = 1;	// avoid recursion
      d->rview->setCurrentItem( newitem );
      newitem->setSelected( true );
    }
  else
  cerr << "can't find new item for edge (BUG)\n";*/
//   d->lview->triggerUpdate();
}


void QObjectBrowser::rightPanelDoubleClicked( QTreeWidgetItem*, int )
{
  updateRightSelectionChange( 1 );
}


unsigned QObjectBrowser::countSelected( QTreeWidgetItem* parent, 
                                        QTreeWidgetItem* & current )
{
  unsigned	n = 0, i, c = parent->childCount();
  QTreeWidgetItem	*item;

  for( i=0; i<c; ++i )
  {
    item = parent->child( i );
    if( item->isSelected() )
    {
      ++n;
      current = item;
    }
  }
  return n;
}


const set<unsigned> & QObjectBrowser::typeCount() const
{
  return( staticState().browserCount );
}


set<unsigned> & QObjectBrowser::typeCount()
{
  return( staticState().browserCount );
}


const string & QObjectBrowser::baseTitle() const
{
  return( staticState().baseTitle );
}


AWindow::Type QObjectBrowser::type() const
{
  return (Type) staticState().classType;
}
  
  
int QObjectBrowser::classType()
{
  return staticState().classType;
}


unsigned QObjectBrowser::mode() const
{
  return d->editMode;
}


AttDescr & QObjectBrowser::attDescr()
{
  return staticState().attDescr;
}


void QObjectBrowser::startDrag( QTreeWidgetItem*, Qt::MouseButtons button, 
                                Qt::KeyboardModifiers )
{
  set<AObject *>	so = Objects();
  if( !( button & Qt::MiddleButton ) )
    {
      // filter selected objects
      SelectFactory	*sf = SelectFactory::factory();
      set<AObject *>::iterator	i = so.begin(), e = so.end(), j;
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

//         d->setPixmap( (*ip).second );
//       d->dragCopy();

    Qt::DropAction dropaction __attribute__((unused)) = drag->exec( Qt::CopyAction );
    // we should not delete drag.
  }
}


View* QObjectBrowser::view()
{
  return d->view.get();
}


const View* QObjectBrowser::view() const
{
  return d->view.get();
}


void QObjectBrowser::keyPressEvent( QKeyEvent* ev )
{
  d->view->controlSwitch()->keyPressEvent( ev );
}


bool QObjectBrowser::showDetailsUponRegister() const
{
  return d->showDetailsUponRegister;
}


void QObjectBrowser::setShowDetailsUponRegister( bool x )
{
  d->showDetailsUponRegister = x;
}

