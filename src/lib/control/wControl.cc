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
#include <anatomist/control/wControl.h>
#include <anatomist/application/fileDialog.h>
#include <anatomist/color/wMaterial.h>
#include <anatomist/selection/qSelectFactory.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/control/qWinTree.h>
#include <anatomist/control/controlMenuHandler.h>
#include <anatomist/control/wcontrolevents.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <anatomist/control/wPreferences.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/reference/wReferential.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/constrainteditor/wConstraintEditor.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cCreateWindowsBlock.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cLoadObjects.h>
#include <anatomist/commands/cCloseWindow.h>
#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/commands/cReloadObject.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/commands/cGroupObjects.h>
#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cDeleteAll.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/control/winconfigio.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/pipeReader.h>
#include <anatomist/browser/qwObjectBrowser.h>
#include <anatomist/control/qAbout.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/control/qImageLabel.h>
#include <anatomist/window/qWinFactory.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/application/module.h>
#include <anatomist/control/objectDrag.h>
#include <anatomist/selection/qSelMenu.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/filedialogextension.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>

#include <aims/qtmisc/qtoolbutton.h>
#include <qfiledialog.h>
#include <cartobase/config/info.h>
#include <qtoolbar.h>
#include <qlabel.h>
#include <qlist.h>
#include <qmenubar.h>
#include <qtranslator.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qtextedit.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qtreewidget.h>
#include <stdio.h>
#include <algorithm>

#ifdef ANA_WEBENGINE
#include <QWebEngineView>
#endif

#ifndef _WIN32
#include <sys/types.h>	// for fork()
#include <unistd.h>	// for fork() and exec()
#else
#include <stdio.h>
#include <process.h>
#endif

using namespace anatomist;
using namespace carto;
using namespace std;


//	Static data

string		ControlWindow::_baseTitle = "control";
ControlWindow	*ControlWindow::_theControlWindow = 0;


//	Private data structure

struct ControlWindow::Private
{
  Private() 
    : fusionbtn(0), referencebtn(0), displayLogo( true ), winList( 0 ),
      objList( 0 ), defobjref( 0 ), defwinref( 0 ), updatemenutimer( 0 ),
      closeEnabled( true ), addAction( 0 ), deleting( false )
#ifdef ANA_WEBENGINE
      , helpWebWidget( 0 )
#endif
      {}

  QAction		*fusionbtn;
  QAction		*referencebtn;
  bool			displayLogo;
  QImageLabel		*logo;
  QWindowTree		*winList;
  QObjectTree		*objList;
  mutable anatomist::Referential *defobjref;
  mutable anatomist::Referential *defwinref;
  QTimer		*updatemenutimer;
  QToolBar		*toolbar;
  bool                  closeEnabled;
  QAction               *addAction;
  bool                  deleting;
#ifdef ANA_WEBENGINE
  QWebEngineView        *helpWebWidget;
#endif
};


//	Constructors

ControlWindow::ControlWindow() 
  : QMainWindow(), 
  d( new Private )
{
  setObjectName("Control Window");
  setAttribute(Qt::WA_DeleteOnClose);
  if( _theControlWindow )
    cerr << "Error: ControlWindow instantiated several times\n";
  _theControlWindow = this;

  if( parent() == 0 )
  {
    QPixmap	anaicon( Settings::findResourceFile(
                        "icons/icon.xpm" ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  _menu = new AControlMenuHandler( menuBar(), this );
  
  drawContents();
  menuBar()->show();

  QToolBar	*tb = d->toolbar;
  int	w = tb->sizeHint().height();
  if( w < 450 )
    w = 450;

  // enlarge the window so the additional menus "object-specific" and "python" 
  // can fit
  resize( sizeHint().width() + 100, w + menuBar()->height() + 2 );
}


//	Destructor


ControlWindow::~ControlWindow()
{
  d->deleting = true;
  bool quitapp = d->closeEnabled;
  if( _theControlWindow == this )
    _theControlWindow = 0;

#ifdef ANA_WEBENGINE
  delete d->helpWebWidget;
  d->helpWebWidget = 0;
#endif

  delete d->objList;
  delete d->winList;

  delete _menu;

  // quit the QApplication only if Anatomist is authorized to do so. It may not be the case if Anatomist is embedded in another QApplication (Axon).
  if (quitapp)
    this->quit();

  delete d;
}



void ControlWindow::createMenu()
{
  _menu->create();
}


void ControlWindow::drawContents()
{
  setWindowTitle( ( string( "Anatomist " ) + theAnatomist->versionString() 
                  + " - CEA/NeuroSpin/SHFJ" ).c_str() );
  QPixmap anaicon( Settings::findResourceFile( "icons/anaIcon.xpm" ).c_str() );
  if( !anaicon.isNull() )
    setWindowIcon( anaicon );

  createMenu();
  createIcons();

  QWidget *main = new QWidget( this );
  setCentralWidget( main );
  QVBoxLayout *vlay = new QVBoxLayout( main );
  main->setLayout( vlay );
  vlay->setContentsMargins( 0, 0, 0, 0 );
  vlay->setSpacing( 5 );

  d->logo = new QImageLabel( main );
  vlay->addWidget( d->logo );

  int	dl = 1;
  theAnatomist->config()->getProperty( "controlWindowLogo", dl );
  d->displayLogo = dl;
  if( !d->displayLogo )
    d->logo->hide();

  QSplitter	*panels = new QSplitter( main );
  vlay->addWidget( panels );
  panels->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                      QSizePolicy::Expanding ) );

  //	Objects list
  d->objList = new QObjectTree( panels, "objectList" );

  //	Windows list
  d->winList = new QWindowTree( panels, "windowList" );

  panels->setStretchFactor( 0, 1 );
  panels->setStretchFactor( 1, 1 );
  QList<int> szs;
  szs.push_back( 1 );
  szs.push_back( 1 );
  panels->setSizes( szs );

  // Objects selection
  connect( d->objList, SIGNAL( selectionChanged() ), this, 
           SLOT( objectTreeClick() ) );
  connect( d->objList, SIGNAL( rightButtonPressed( anatomist::AObject*, 
                                                   const QPoint & ) ), 
           this, SLOT( objectTreeRightClick( anatomist::AObject*, 
                                             const QPoint & ) ) );

  // Windows selection
  connect( d->winList, SIGNAL( selectionChanged() ), this,
            SLOT( windowTreeClick() ) );
  connect( d->winList, SIGNAL( doubleClicked( anatomist::AWindow * ) ), this,
            SLOT( windowTreeDblClick( anatomist::AWindow * ) ) );

  UpdateMenus();
}


void ControlWindow::createIcons()
{
  //	Icon bar

  QToolBar	*iconbar = new QToolBar( tr( "Icons" ), this );
  addToolBar( Qt::LeftToolBarArea, iconbar );
  d->toolbar = iconbar;
  iconbar->setIconSize( QSize( 32, 32 ) );

  QFont	fnt = iconbar->font();
  fnt.setPixelSize( 9 );
  iconbar->setFont( fnt );

  QIcon	is( Settings::findResourceFile( "icons/open.xpm" ).c_str() );
  is.addFile( Settings::findResourceFile( "icons/open-active.xpm" ).c_str(),
              QSize(), QIcon::Active );
  iconbar->addAction( is, tr( "Load object" ), this, SLOT( loadObject() ) );


  //	windows shortcuts
  map<int, string>		wtypes = AWindowFactory::typeNames();
  map<int, string>::iterator	it, ft=wtypes.end();
  int				wtype;

  for( it=wtypes.begin(); it!=ft; ++it )
  {
    wtype = (*it).first;
    if( AWindowFactory::hasControlWindowButton( wtype ) )
    {
      QAWindowFactory::loadDefaultPixmaps( wtype );
      const QAWindowFactory::PixList & pix 
        = QAWindowFactory::pixmaps( wtype );
      const QAWindowFactory::Descrip & des
        = QAWindowFactory::description( wtype );
      QToolButtonInt        *tb = new QToolButtonInt( wtype, 0 );
      QAction *act = new QAction( tb );
      act->setText( des.brief );
      if( !pix.plarge.isNull() )
      {
        QIcon	is( pix.plarge );
        if( !pix.pactive.isNull() )
        {
          is.addPixmap( pix.pactive, QIcon::Active );
          act->setIcon( is );
        }
      }
      tb->setDefaultAction( act );
      act = iconbar->addWidget( tb );
      act->setText( des.brief );
      connect( tb, SIGNAL( activated( int ) ), this,
               SLOT( openWindow( int ) ) );
      // connect drag & drop signals to drag objects on windows icons
      tb->setAcceptDrops( true );
      connect( tb, SIGNAL( dropped( int, QDropEvent* ) ), this,
                SLOT( dropOnWindowIcon( int, QDropEvent* ) ) );
      connect( tb, SIGNAL( dragEntered( int, QDragEnterEvent* ) ), this,
                SLOT( dragEnterOnWindowIcon( int, QDragEnterEvent* ) ) );
      connect( tb, SIGNAL( dragMoved( int, QDragMoveEvent* ) ), this,
                SLOT( dragMoveOnWindowIcon( int, QDragMoveEvent* ) ) );
    }
  }

  {
    QPixmap addpix( Settings::findResourceFile( "icons/add.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile( "icons/add-active.xpm" ).c_str() );
    QIcon       is( addpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    d->addAction = iconbar->addAction( is, tr( "Add objects in windows" ),
                        this, SLOT( addObjectsInWindows() ) );
  }

  {
    QPixmap rmvpix( Settings::findResourceFile( "icons/remove.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile(
      "icons/remove-active.xpm" ).c_str() );
    QIcon       is( rmvpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "Removes objects from windows" ), this,
                        SLOT( removeObjects() ) );
  }

  {
    QPixmap fusionpix( Settings::findResourceFile(
      "icons/fusion.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile(
      "icons/fusion-active.xpm" ).c_str() );
    QIcon	is( fusionpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    d->fusionbtn = iconbar->addAction( is, tr( "Fusion objects" ), this,
      SLOT( fusionObjects() ) );
  }

  {
    QPixmap refpix( Settings::findResourceFile(
      "icons/reference.xpm" ).c_str() );
    QPixmap	p( Settings::findResourceFile(
      "icons/reference-active.xpm" ).c_str() );
    QIcon	is( refpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    d->referencebtn = iconbar->addAction( is, tr( "Assign referential" ), this,
      SLOT( chooseReferential() ) );
  }

  {
    QPixmap conEdpix( Settings::findResourceFile(
      "icons/meshPaint/sulci.png" ).c_str() );
    QPixmap p( Settings::findResourceFile(
      "icons/meshPaint/gyri.png" ).c_str() );
    QIcon is( conEdpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "ConstraintEditor" ), this, 
                        SLOT( openConstraintEditor() ) );
  }
}


//	Registration

void ControlWindow::registerWindow( AWindow* win )
{
  // make a valid title for the window
  win->createTitle();
  d->winList->registerWindow( win );
}


void ControlWindow::unregisterWindow( AWindow* win )
{
  d->winList->unregisterWindow( win );

  UpdateToggleMenus();
}


void ControlWindow::registerObject( AObject *obj )
{
  d->objList->RegisterObject( obj );
  obj->SetVisibility( true );
}


void ControlWindow::unregisterObject( AObject *obj )
{
  d->objList->UnregisterObject( obj );
  obj->SetVisibility( false );

  AObject::ParentList::const_iterator	ip, fp=obj->Parents().end();

  for( ip=obj->Parents().begin(); ip!=fp; ++ip )
    d->objList->UnregisterSubObject( *ip, obj );

  if( !theAnatomist->destroying() )
  {
    if( !d->updatemenutimer )
    {
      d->updatemenutimer = new QTimer( this );
      connect( d->updatemenutimer, SIGNAL( timeout() ), this, 
               SLOT( UpdateMenus() ) );
    }
    d->updatemenutimer->setSingleShot( true );
    d->updatemenutimer->start( 1 );
  }
}


//	Functions


set<AObject *> ControlWindow::selectedObjects()
{
  set<AObject *>	*so = d->objList->SelectedObjects();
  set<AObject *>	obj = *so;
  delete so;
  return( obj );
}


set<AWindow*> ControlWindow::selectedWindows()
{
  set<AWindow *>	*sw = d->winList->SelectedWindows();
  set<AWindow *>	win = *sw;
  delete sw;
  return( win );
}


bool ControlWindow::hasObject( AObject *obj ) const
{
  return d->objList->hasObject( obj );
}


set<int> ControlWindow::SelectedWinGroups() const
{
  return( d->winList->SelectedGroups() );
}


void ControlWindow::ResizeWindow( AWindow * )
{
}


void ControlWindow::UpdateToggleMenus()
{
  UpdateObjectMenu();
  UpdateWindowMenu();
}


void ControlWindow::UpdateObjectMenu()
{
  set<AObject*>	objL = selectedObjects();
  set<AWindow*>	winL = selectedWindows();
  AObject		*obj;

  if( objL.empty() )
    {
      enableFusionMenu( false );
      enableGroupMenu( false );
      return;
    }

  // update fusion toggle
  if( FusionFactory::canFusion( objL ) )
    enableFusionMenu( true );
  else enableFusionMenu( false );

  if( objL.size() > 1 )
    {
      // grouping possible
      _menu->setGroupMenuText( tr( "Group objects" ) );
      enableGroupMenu( true );
    }
  else	// 1 object
    {
      obj = *objL.begin();
      if( !obj->isMultiObject() )
	{
	  enableGroupMenu( false );
	  return;
	}
      // ungrouping possible
      _menu->setGroupMenuText( tr( "Ungroup objects" ) );
      enableGroupMenu( true );
    }

  if( objL.empty() && winL.empty() )
    {
      enableLoadRefMenu( false );
    }
  else
    {
      enableLoadRefMenu( true );
    }
}


void ControlWindow::UpdateWindowMenu()
{
  //	windows grouping possibility check
  set<AWindow *>	winL = selectedWindows();
  set<int>		sgr = SelectedWinGroups();

  if( sgr.size() != 0 && winL.empty() )
    {	// link group(s) selected alone
      _menu->setLinkMenuText( tr( "Unlink windows" ) );
      _menu->enableLinkMenu( true );
    }
  else if( sgr.size() == 0 && !winL.empty() )
    {	// windows selected alone
      set<AWindow *>::iterator	iw, fw=winL.end();
      bool			linkable = true;

      for( iw=winL.begin()++; iw!=fw; ++iw )
	if( (*iw)->Group() != 0 )
	  {
	    linkable = false;
	    break;
	  }

      if( linkable )	//	Windows not already linked
	{
	  _menu->setLinkMenuText( tr( "Link windows" ) );
	  _menu->enableLinkMenu( true );
	}
      else
	_menu->enableLinkMenu( false );
    }
  else
    {
	_menu->enableLinkMenu( false );
    }

  if( winL.empty() && selectedObjects().empty() )
    {
      enableLoadRefMenu( false );
    }
  else
    {
      enableLoadRefMenu( true );
    }
}


void ControlWindow::BuildObjectMenu()
{
  set<AObject *>	ol = selectedObjects();
  Tree			otr;

  OptionMatcher::commonOptions( ol, otr );

  _menu->makeObjectManipMenus( ol, otr );
}


void ControlWindow::objectTreeClick()
{
  UpdateMenus();
}


void ControlWindow::objectTreeDblClick()
{
}


void ControlWindow::objectTreeRightClick( AObject*, const QPoint & p )
{
  set<AObject *>	ol = selectedObjects();
  Tree			tr;

  OptionMatcher::commonOptions( ol, tr );

  QSelectMenu	*pop = OptionMatcher::popupMenu( ol, tr );
  pop->exec( p );
  delete pop;
}


void ControlWindow::windowTreeClick()
{
  UpdateToggleMenus();
}


void ControlWindow::windowTreeDblClick( AWindow* w )
{
  if(w)
    {
      w->show();
    }
}


void ControlWindow::SelectObject( AObject *obj )
{
  d->objList->SelectObject( obj );
  UpdateMenus();
}


void ControlWindow::UnselectAllObjects()
{
  d->objList->UnselectAll();
  UpdateMenus();
}


void ControlWindow::SelectWindow( AWindow *win )
{
  d->winList->SelectWindow( win );
}


void ControlWindow::UnselectAllWindows()
{
  d->winList->UnselectAll();
  UpdateMenus();
}


void ControlWindow::createTitle()
{
  /*----- sauf exception, le titre d'une fenetre est positionne dans les 
    ressources X
  setTitle( _baseTitle );
  -----*/
}


//	Menu callbacks


void ControlWindow::loadObject()
{
  loadObject( "", "" );
}


void ControlWindow::loadObject( const string& filter, const string& caption )
{
  QString filt = filter.c_str() ;
  QString capt = caption.c_str() ;
  
  if ( filt == "" )
    filt = theAnatomist->objectsFileFilter().c_str();
  
  if ( capt == "" )
    capt = tr( "Load Anatomist objects" );
  
  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filt );
  fd.setWindowTitle( capt );
  fd.setFileMode( QFileDialog::ExistingFiles );
  fd.setAcceptMode( QFileDialog::AcceptOpen );
  if( !fd.exec() )
  {
    cout << "Load cancelled\n";
    return;
  }

  QStringList		filenames = fd.selectedFiles();
  vector<string>        sfilenames;
  list<QString>		scenars;
  set<AObject *>	loaded;

  AimsFileDialog *afd = dynamic_cast<AimsFileDialog *>( &fd );
  string data_type;
  Object options;

  bool threaded = false;
  bool async = false;
  try
  {
    threaded = bool(
      theAnatomist->config()->getProperty( "parallel_load" )->getScalar() );
  }
  catch( ... )
  {
  }
  try
  {
    async = bool(
      theAnatomist->config()->getProperty( "async_load" )->getScalar() );
  }
  catch( ... )
  {
  }

  if( afd && afd->optionsValid() )
  {
    data_type = afd->selectedType().second;
    options = afd->selectedOptions();
    if( !options )
      options = Object::value( carto::Dictionary() );
    string url_opt = afd->selectedUrlOptions();
    if( !url_opt.empty() )
      options->setProperty( "url_options", url_opt );
    if( !data_type.empty() )
      options->setProperty( "preferred_data_type", data_type );
    try
    {
      threaded = options->getProperty( "threaded" )->getScalar();
    }
    catch( ... )
    {
    }
  }
  if( !options || !options->hasProperty( "asynchronous" ) )
  {
    if( !options )
      options = Object::value( carto::Dictionary() );
    options->setProperty( "asynchronous", async );
  }

  sfilenames.reserve( filenames.count() );

  for ( QStringList::Iterator it = filenames.begin(); it != filenames.end(); 
        ++it )
    {
      QString	& s = *it;
      if( s.lastIndexOf( ".ana" ) == int( s.length() ) - 4 )
        scenars.push_back( s );	// script file
      else
        sfilenames.push_back( it->toLocal8Bit().data() );
    }

  if( !sfilenames.empty() )
  {
    LoadObjectsCommand *command = new LoadObjectsCommand(
      sfilenames, vector<int>(), vector<string>(), options,
      &CommandContext::defaultContext(), threaded );

    theProcessor->execute( command );
    vector<AObject *> objs = command->loadedObjects();
    loaded.insert( objs.begin(), objs.end() );
  }

  // set default ref on loaded objects
  anatomist::Referential	*r = defaultObjectsReferential();
  if( r && r != theAnatomist->centralReferential() )
    {
      Command *command = new AssignReferentialCommand( r, loaded, 
                                                       set<AWindow *>() );
      theProcessor->execute( command );
    }

  theAnatomist->getControlWindow()->UnselectAllObjects();

  // play scenarios (if any)
  list<QString>::iterator	is, es = scenars.end();
  for( is=scenars.begin(); is!=es; ++is )
    new APipeReader( is->toLocal8Bit().data() );
}


void ControlWindow::replayScenario()
{
  QFileDialog	& fd = fileDialog();
  QString filter = tr( "Scenario" ) + " (*.ana)";
  QString caption = tr( "Open scenario" );
  /*QString filename = QFileDialog::getOpenFileName( QString(),
    filter, 0, 0, caption );*/
  fd.setNameFilter( filter );
  fd.setWindowTitle( caption );
  fd.setFileMode( QFileDialog::ExistingFile );
  fd.setAcceptMode( QFileDialog::AcceptOpen );
  if( !fd.exec() )
    return;

  QStringList filenames = fd.selectedFiles();

  if( !filenames.isEmpty() )
    new APipeReader( filenames[0].toLocal8Bit().data() );
}


void ControlWindow::openWindow( QAction *ac )
{
  // callback for menus
  int wtype = ac->data().toInt();
  openWindow( wtype );
}


void ControlWindow::openWindow( int type )
{
  string	typestr = AWindowFactory::typeString( type );
  CreateWindowCommand	*command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );
  // set default ref on loaded objects
  anatomist::Referential	*r = defaultWindowsReferential();
  if( r && r != theAnatomist->centralReferential() )
    {
      set<AWindow *>	w;
      w.insert( command->createdWindow() );
      Command *c = new AssignReferentialCommand( r, set<AObject *>(), w );
      theProcessor->execute( c );
    }
}


void ControlWindow::dragEnterOnWindowIcon( int, QDragEnterEvent* event )
{
  event->setAccepted( QAObjectDrag::canDecode( event->mimeData() )
      || QAObjectDrag::canDecodeURI( event->mimeData() ) );
}


void ControlWindow::dragMoveOnWindowIcon( int, QDragMoveEvent* event )
{
  event->accept();
}


void ControlWindow::dropOnWindowIcon( int type, QDropEvent* event )
{
  event->accept();
  set<AObject *>	o;
  list<QString> objects;
  list<QString> scenars;

  if( !QAObjectDrag::decode( event->mimeData(), o )
       && QAObjectDrag::decodeURI( event->mimeData(), objects, scenars ) )
  {
    list<QString>::iterator       is, es = objects.end();
    for( is=objects.begin(); is!=es; ++is )
    {
      LoadObjectCommand *command = new LoadObjectCommand( 
        is->toLocal8Bit().data() );
      theProcessor->execute( command );
      list<AObject *> objs = command->loadedObjects();
      o.insert( objs.begin(), objs.end() );
    }
  }
  if( o.empty() )
    return;

  //cout << "object decoded, " << o.size() << " objects\n";

  string			typestr = AWindowFactory::typeString( type );
  CreateWindowCommand	*command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );

  if( command->createdWindow() )
  {
    set<AWindow *>	sw;

    sw.insert( command->createdWindow() );
    Command	*c = new AddObjectCommand( o, sw );
    theProcessor->execute( c );
  }
}


void ControlWindow::openAxial()
{
  Command* command = new CreateWindowCommand( "Axial" );
  theProcessor->execute(command);
}


void ControlWindow::openSagittal()
{
  Command* command = new CreateWindowCommand( "Sagittal" );
  theProcessor->execute(command);
}


void ControlWindow::openCoronal()
{
  Command* command = new CreateWindowCommand( "Coronal" );
  theProcessor->execute(command);
}


void ControlWindow::openOblique()
{
  cerr << "openOblique : no oblique window for now...." << endl;
}


void ControlWindow::open3D()
{
  Command* command = new CreateWindowCommand( "3D" );
  theProcessor->execute(command);
}


void ControlWindow::openBrowser()
{
  Command* command = new CreateWindowCommand( "Browser" );
  theProcessor->execute(command);
}


void ControlWindow::openRefWin()
{
  theAnatomist->createReferentialWindow();
  theAnatomist->getReferentialWindow()->refresh();
  theAnatomist->getControlWindow()->enableRefWinMenu( false );
}


void ControlWindow::iconifyWindows()
{
  set<AWindow*>	wl = selectedWindows();
  set<AWindow*>::iterator i;

  for( i=wl.begin(); i!=wl.end(); ++i )
    {
      (*i)->iconify();
      ResizeWindow( *i );
    }
}


void ControlWindow::restoreWindows()
{
  set<AWindow*>::iterator i;

  set<AWindow*> wl = selectedWindows();
  for( i=wl.begin(); i!=wl.end(); ++i )
    {
      (*i)->unIconify();
      (*i)->show();
      ResizeWindow( *i );
    }
}


void ControlWindow::closeWindows()
{
  set<AWindow*> winL = selectedWindows();
  if( !winL.empty() )
    {
      Command *cmd = new CloseWindowCommand( winL );
      theProcessor->execute( cmd );
    }
}


void ControlWindow::linkWindows()
{
  set<AWindow*>		winL = selectedWindows();
  set<int>		groups = SelectedWinGroups();
  set<int>::iterator	is, fs;

  if( groups.size() == 0 )	// link case
    {
      //	command removed
      set<AWindow*>::iterator iw, j;

      // clean up list to remove windows which could have been destroyed
      for( iw=winL.begin(); iw!=winL.end(); ++iw )
	if( !theAnatomist->hasWindow( *iw ) )
	  {
	    j = iw;
	    --iw;
	    winL.erase( j );
	  }

      if( winL.empty() )
	{
	  cout << "Link - Nothing to do : not enough windows" << endl;
	  return;
	}

      theAnatomist->groupWindows( winL );

      theAnatomist->UpdateInterface();
      theAnatomist->Refresh();
    }
  else				// unlink case
    {
      for( is=groups.begin(), fs=groups.end(); is!=fs; ++is )
	{
	  //	command removed
	  theAnatomist->ungroupWindows( *is );
	}
    }
}


void ControlWindow::addObjectsInWindows()
{
  set<AWindow*>				winL = selectedWindows();
  set<int>				wg = SelectedWinGroups();
  set<AObject*>				objL = selectedObjects();
  set<int>::const_iterator		ig, fg=wg.end();
  set<AWindow *>::const_iterator	iw, fw;

  for( ig = wg.begin(); ig!=fg; ++ig )
    {
      set<AWindow *>	wl = theAnatomist->getWindowsInGroup( *ig );
      for( iw=wl.begin(), fw=wl.end(); iw!=fw; ++iw )
	winL.insert( *iw );
    }

  if( !winL.empty() && !objL.empty() )
    {
      Command *cmd = new AddObjectCommand( objL, winL );
      theProcessor->execute( cmd );
    }
}


void ControlWindow::removeObjects()
{
  set<AWindow*>				winL = selectedWindows();
  set<int>				wg = SelectedWinGroups();
  set<AObject*>				objL = selectedObjects();
  set<int>::const_iterator		ig, fg=wg.end();
  set<AWindow *>::const_iterator	iw, fw;

  for( ig = wg.begin(); ig!=fg; ++ig )
    {
      set<AWindow *>	wl = theAnatomist->getWindowsInGroup( *ig );
      for( iw=wl.begin(), fw=wl.end(); iw!=fw; ++iw )
	winL.insert( *iw );
    }

  if( !winL.empty() && !objL.empty() )
    {
      Command *cmd = new RemoveObjectCommand( objL, winL );
      theProcessor->execute( cmd );
    }
}


void ControlWindow::deleteObjects()
{
  set<AObject*>		objL = selectedObjects();
  if( !objL.empty() )
    {
      vector<AObject*>		objv;
      objv.reserve( objL.size() );
      set<AObject*>::iterator	io, eo=objL.end();
      for( io=objL.begin(); io!=eo; ++io )
	objv.push_back( *io );
      // I remove this because it causes a warning with egcs-1.1.2
      //objv.insert( objv.end(), objL.begin(), objL.end() );
      Command	*cmd = new DeleteObjectCommand( objv );
      theProcessor->execute( cmd );
    }
}


void ControlWindow::reload()
{
  set<AObject*> objL = selectedObjects();
  if( !objL.empty() )
    ObjectActions::fileReload( objL );
}


void ControlWindow::groupObjects()
{
  set<AObject*> objL = selectedObjects();
  Command	*cmd = new GroupObjectsCommand( objL );
  theProcessor->execute( cmd );
  UnselectAllObjects();
}

void ControlWindow::fusionObjects()
{
  set<AObject*>			objL = selectedObjects();
  set<AObject*>::iterator	io, eo = objL.end();
  vector<AObject *>		obj;

  obj.reserve( objL.size() );
  for( io=objL.begin(); io!=eo; ++io )
    obj.push_back( *io );

  Command	*cmd = new FusionObjectsCommand( obj );

  theProcessor->execute( cmd );
  UnselectAllObjects();
}

void ControlWindow::chooseWinReferential()
{
  ChooseReferentialWindow *w 
    = new ChooseReferentialWindow( selectedWindows(), 
                                   "Choose Referential Window" );
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->show();
}

void ControlWindow::chooseReferential()
{
  ChooseReferentialWindow *w 
    = new ChooseReferentialWindow( selectedWindows(), selectedObjects(),
                                   "Choose Referential Window" );
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->show();
}

void ControlWindow::openPreferencesWin()
{
  //	command removed
  (new PreferencesWindow)->show();
  enablePreferencesMenu( false );
}

void ControlWindow::openConstraintEditor()
{
  string title = tr( "ConstraintEditor" ).toStdString();
  ( new ConstraintEditorWindow( selectedObjects(),title.c_str() ) )->show();
}

void ControlWindow::viewRefColors()
{
  d->objList->ToggleRefColorsView();
  d->winList->ToggleRefColorsView();
}


void ControlWindow::NotifyObjectChange( AObject* obj )
{
  d->objList->NotifyObjectChange( obj );
}


void ControlWindow::NotifyWindowChange( AWindow* win )
{
  d->winList->NotifyWindowChange( win );
}


bool ControlWindow::ViewingRefColors() const
{
  return( d->objList->ViewingRefColors() );
}


void ControlWindow::ToggleRefColorsView()
{
  d->objList->ToggleRefColorsView();
}


void ControlWindow::registerSubObject( MObject* parent, AObject* obj )
{
  d->objList->RegisterSubObject( parent, obj );
}


void ControlWindow::saveWindowsConfig()
{
  QString filter = tr( "Anatomist scripts" ) + " (*.ana)";
  QString caption = tr( "Save windows configuration" );
  QString filename = QFileDialog::getSaveFileName( this, caption, 
    QString(), filter, 0, QFileDialog::Options() );
  if ( !filename.isEmpty() )
  {
    AWinConfigIO cio;
    cio.saveConfig( filename.toLocal8Bit().data() );
  }
}


void ControlWindow::enableRefWinMenu( bool state )
{
  _menu->enableRefWinMenu( state );
}


void ControlWindow::enableLoadRefMenu( bool state )
{
  _menu->enableLoadRefMenu( state );
  if( d->referencebtn )
    d->referencebtn->setEnabled( state );
}


void ControlWindow::enablePreferencesMenu( bool state )
{
  _menu->enablePreferencesMenu( state );
}


void ControlWindow::enableGroupMenu( bool state )
{
  _menu->enableGroupMenu( state );
}


void ControlWindow::enableFusionMenu( bool state )
{
  _menu->enableFusionMenu( state );
  if( d->fusionbtn )
    d->fusionbtn->setEnabled( state );
}


void ControlWindow::about()
{
  // QAbout abw( theAnatomist->getQWidgetAncestor(), "About Anatomist" );
  QAbout abw( this, "About Anatomist" );
  abw.exec();
}


namespace
{
  /* This function finds the end of a "word" in a string, taking care of 
     possible escaped characters and quotes. This may be a useful function, 
     It can be a good idea to put it somewhere in a visible place 
     (cartobase?). */
  unsigned endOfWord( const string & str, unsigned spos )
  {
    unsigned	pos = spos, epos = str.length();
    bool	esc = false, inquote = false;
    char	quote = '\0', c;

    while( pos < epos )
      {
        c = str[pos];
        if( esc )
          esc = false;
        else
          {
            if( c == '\\' )
              esc = true;
            else
              {
                if( inquote )
                  {
                    if( c == quote )
                      inquote = false;
                  }
                else
                  {
                    if( c == '"' || c == '\'' )
                      {
                        inquote = true;
                        quote = c;
                      }
                    else if( c == ' ' || c == '\t' )
                      return pos;
                  }
              }
          }
        ++pos;
      }

    return pos;
  }
}


void ControlWindow::help()
{
  char	s = FileUtil::separator();
  string url = Settings::docPath() + s + "user_doc" + s + "index.html";

#ifdef ANA_WEBENGINE
  if( !d->helpWebWidget )
    d->helpWebWidget = new QWebEngineView;

  d->helpWebWidget->setUrl( QUrl( ( string("file://" ) + url ).c_str() ) );

  d->helpWebWidget->show();
#else

#ifdef __APPLE__
  string cmd = "/Applications/Safari.app/Contents/MacOS/" 
    "Safari %1";
#else
#ifdef _WIN32
  string cmd = "explorer.exe %1";
#else
  string cmd = "mozilla %1";
#endif
#endif

  theAnatomist->config()->getProperty( "html_browser", cmd );

  /*
  string	lang;
  theAnatomist->config()->getProperty( "language", lang );
  if( !lang.empty() )
    {
      Directory	d( url + s + lang );
      if( d.isValid() )
        url = url + s + lang;
    }
  else
    url = url + s + "en";
  url = url + s + "index.html";
  */

  string::size_type i = cmd.find( "%1" );
  if( i != string::npos )
    {
      while( i != string::npos )
	{
	  cmd.replace( i, 2, url );
	  i = cmd.find( "%1", i );
	}
    }
  else
    {
       cmd += " ";
       cmd += url;
    }

  cout << "HTML browser command : " << cmd << endl;

#ifndef _WIN32

  pid_t	pid = fork();
  if( pid < 0 )
    {
      cerr << "forking error while opening HTML browser" << endl;
    }
  else if( pid > 0 )
    {
      // parent
      // (nothing to do)
    }
  else
    {

#endif

      // child
      vector<string>		params;
      vector<const char *>	parc;
      string			cmdbase, arg;
      unsigned			pos, spos = 0, epos = cmd.length(), j = 1;
      // split command into an arguments array
      pos = endOfWord( cmd, 0 );
      cmdbase = cmd.substr( 0, pos );

      params.push_back( cmdbase );
      parc.push_back( cmdbase.c_str() );

      while( pos < epos && cmd[pos] == ' ' )
        ++pos;

      while( pos < epos )
        {
          spos = pos;
          pos = endOfWord( cmd, spos );
          params.push_back( cmd.substr( spos, pos - spos ) );
          while( pos < epos && cmd[pos] == ' ' )
            ++pos;
          parc.push_back( params[j].c_str() );
          ++j;
        }

      --j;
      if( j > 0 && params[j] == "&" )
        {
          parc[j] = 0;
          params.erase( params.begin() + j );
        }
      else
        {
          if( j > 0 && params[j][ params[j].length() - 1 ] == '&' )
            params[j].erase( params[j].length() - 1, 1 );
          parc.push_back( 0 );
        }
      // debug
      /* for( j=0; parc[j]; ++j )
         cout << "param " << j << ": >" << parc[j] << "<" << endl; */
#ifndef _WIN32
      execvp( cmdbase.c_str(), (char* const*) &parc[0] );
#else
    if( _spawnvp( _P_NOWAIT, cmdbase.c_str(), (char* const*) &parc[0] ) <= 1 )
#endif

      cerr << "Couldn't launch HTML browser properly - set the correct "
	   << "command in preferences" << endl;

#ifndef _WIN32
      /* we must exit abruptly here otherwise some class destructors (Qt) will 
         be called and confuse the X server (because the X client has only been
         registered once by the parent process) */
      ::_exit( EXIT_FAILURE );
    }
#endif
#endif
}


void ControlWindow::graphParams()
{
  QGraphParam	*gpw = QGraphParam::theGP();

  if( !gpw )
    gpw = new QGraphParam;
  gpw->show();
}


void ControlWindow::quit()
{   
  cout << "Exiting QApplication"<<endl;
  clearAll();
  if( !d->deleting )
    delete this;
  if( !QApplication::closingDown() )
    qApp->quit();
//   if( theAnatomist->exitOnQuit() )
    exit( EXIT_SUCCESS );
}

void ControlWindow::languageEnglish()
{
  setLanguage( "english" );
}


void ControlWindow::languageFrench()
{
  setLanguage( "french" );
}


void ControlWindow::setLanguage( const string & filename )
{
  string path = Settings::findResourceFile( "po" );
  QTranslator	*tr = new QTranslator( qApp );

  if( tr->load( filename.c_str(), path.c_str() ) )
    qApp->installTranslator( tr );
  else
  {
    cerr << "warning: translation file not found\npath: "
          << path << "\nfile: " << filename << endl;
    delete tr;
  }
}


void ControlWindow::saveSettings()
{
  theAnatomist->config()->save();
}


void ControlWindow::enableLogo( bool state )
{
  if( state )
    d->logo->show();
  else
    d->logo->hide();
  d->displayLogo = state;
}


bool ControlWindow::logoEnabled() const
{
  return( d->displayLogo );
}


void ControlWindow::reloadPalettes()
{
  theAnatomist->palettes().load( Settings::localPath() + "/rgb" );
}


void ControlWindow::modulesList()
{
  ModuleManager			*mm = ModuleManager::instance();
  ModuleManager::const_iterator	im, em=mm->end();
  unsigned			i = 0;

  QWidget		*mv = new QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window );
  mv->setObjectName("Modules list");
  mv->setAttribute(Qt::WA_DeleteOnClose);
  QVBoxLayout* layout = new QVBoxLayout;
  QTreeWidget	*lv = new QTreeWidget( mv );
  lv->setObjectName("Modules listview");
  lv->setColumnCount(3);
  QStringList labels;
  labels << tr( "Index :" ) << tr( "Name :" )<< tr( "Description :" );
  lv->setHeaderLabels(labels);
  layout->addWidget(lv);
  mv->setWindowTitle( tr( "Modules list" ) );

  cout << "Modules list :\n";
  QTreeWidgetItem *item;
  for( im=mm->begin(); im!=em; ++im, ++i )
    {
      cout << i << " : " << (*im)->name() << endl;
      cout << "   " << (*im)->description() << endl;
      QStringList text;
      text << QString::number( i+1 ) << QString( "  " ) + tr( (*im)->name().c_str() ) << QString( "  " ) + tr( (*im)->description().c_str() ) ;
      item = new QTreeWidgetItem( lv, text );
      item->setTextAlignment(0, Qt::AlignTop);
      item->setTextAlignment(1, Qt::AlignTop);
      item->setTextAlignment(2, Qt::AlignTop);
    }
  lv->resizeColumnToContents(1);
  lv->resizeColumnToContents(2);
  mv->setLayout(layout);
  mv->resize(800, 400);
  mv->show();
}


void ControlWindow::aimsInfo()
{
  ostringstream	txt;
  Info::print( txt );
  QWidget		*w = new QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window );
  w->setAttribute(Qt::WA_DeleteOnClose);
  QVBoxLayout *layout = new QVBoxLayout;
  w->setLayout(layout);
  layout->setContentsMargins( 5, 5, 5, 5 );
  QTextEdit     *info = new QTextEdit( w );
  layout->addWidget(info);
  w->setWindowTitle( tr( "Anatomist / AIMS libraries information" ) );
  w->resize( 600, 400 );
  info->setReadOnly( true );
  info->setPlainText( txt.str().c_str() );
  w->show();
}


void ControlWindow::openBlockView()
{
  string	typestr 
    = AWindowFactory::typeString( AWindow::CORONAL_WINDOW );
  set<AWindow *>	w;
  CreateWindowCommand	*command 
    = new CreateWindowCommand( typestr, -1, 0, (vector<int>) 0, true );
  theProcessor->execute( command );
  QWidget *block = command->block();
  w.insert( command->createdWindow() );

  typestr = AWindowFactory::typeString( AWindow::AXIAL_WINDOW );
  command = new CreateWindowCommand( typestr, -1, 0, (vector<int>) 0, true, 
                                     block );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );

  typestr = AWindowFactory::typeString( AWindow::SAGITTAL_WINDOW );
  command = new CreateWindowCommand( typestr, -1, 0, (vector<int>) 0, true,
                                     block );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );

  typestr = AWindowFactory::typeString( AWindow::WINDOW_3D );
  command = new CreateWindowCommand( typestr, -1, 0, (vector<int>) 0, true, 
                                     block );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );

  set<AObject *>	o = selectedObjects();
  if( !o.empty() )
    theProcessor->execute( new AddObjectCommand( o, w ) );
}


void ControlWindow::openThreeViews()
{
  string	typestr = AWindowFactory::typeString( AWindow::AXIAL_WINDOW );
  set<AWindow *>	w;
  CreateWindowCommand	*command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );
  typestr = AWindowFactory::typeString( AWindow::CORONAL_WINDOW );
  command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );
  typestr = AWindowFactory::typeString( AWindow::SAGITTAL_WINDOW );
  command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );

  set<AObject *>	o = selectedObjects();
  if( !o.empty() )
    theProcessor->execute( new AddObjectCommand( o, w ) );
}


void ControlWindow::openEmptyBlockView()
{
  CreateWindowsBlockCommand	*command
    = new CreateWindowsBlockCommand( -1, 0, (vector<int>) 0, 0, 2 );
  theProcessor->execute( command );
  // QWidget *block = command->block();
}


anatomist::Referential* ControlWindow::defaultObjectsReferential() const
{
  set<anatomist::Referential *>	refs = theAnatomist->getReferentials();
  if( d->defobjref && refs.find( d->defobjref ) == refs.end() )
    d->defobjref = 0;
  return d->defobjref;
}


anatomist::Referential* ControlWindow::defaultWindowsReferential() const
{
  set<anatomist::Referential *>	refs = theAnatomist->getReferentials();
  if( d->defwinref && refs.find( d->defwinref ) == refs.end() )
    d->defwinref = 0;
  return d->defwinref;
}


void ControlWindow::setDefaultObjectsReferential( anatomist::Referential* ref )
{
  d->defobjref = ref;
}


void ControlWindow::setDefaultWindowsReferential( anatomist::Referential* ref )
{
  d->defwinref = ref;
}


bool ControlWindow::closeEnabled() const
{
  return d->closeEnabled;
}


void ControlWindow::enableClose( bool x )
{
  d->closeEnabled = x;
}

void ControlWindow::closeEvent(QCloseEvent *event)
{
  bool ask = true, doclose = true;
  try
  {
    Object oask = theAnatomist->config()->getProperty( "confirmBeforeQuit" );
    if( !oask.isNull() )
      ask = (bool) oask->getScalar();
  }
  catch( ... )
  {
  }
  if( ask )
  {
    int x = QMessageBox::warning( this, tr( "Quit" ), tr( "Do you really want to quit Anatomist ?" ), QMessageBox::Ok,
      QMessageBox::Cancel );
    if( x != QMessageBox::Ok )
      doclose = false;
  }
  if( doclose )
  {
    DeleteAllCommand *dc = new DeleteAllCommand;
    theProcessor->execute( dc );
    event->accept();
  }
  else event->ignore();
}

void ControlWindow::clearAll()
{
  /// warn / confirm
  int x = QMessageBox::warning( this, tr( "Close all objects / windows "
    "/referentials ?" ), tr( "All objects, windows, referentials and "
    "transformations will be closed" ), QMessageBox::Ok,
    QMessageBox::Cancel );
  if( x != QMessageBox::Ok )
    return;

  DeleteAllCommand *dc = new DeleteAllCommand;
  theProcessor->execute( dc );
}


void ControlWindow::setWindowTypeVisible( int wtype, bool visible )
{
  const QAWindowFactory::Descrip & des
    = QAWindowFactory::description( wtype );
  if( des.brief.isEmpty() )
    QAWindowFactory::loadDefaultPixmaps( wtype );
  QList<QAction *> ac = d->toolbar->actions();
  QList<QAction *>::iterator ia, ea = ac.end();
  QAction *action = 0;
  for( ia=ac.begin(); ia!=ea; ++ia )
    if( (*ia)->text() == des.brief )
    {
      action = *ia;
      break;
    }
  if( action )
  {
    action->setVisible( visible );
    return;
  }

  if( visible )
  {
    const QAWindowFactory::PixList & pix
      = QAWindowFactory::pixmaps( wtype );
    QToolButtonInt        *tb = new QToolButtonInt( wtype, 0 );
    QAction *act = new QAction( tb );
    act->setText( des.brief );
    if( !pix.plarge.isNull() )
    {
      QIcon   is( pix.plarge );
      if( !pix.pactive.isNull() )
        is.addPixmap( pix.pactive, QIcon::Active );
      act->setIcon( is );
    }
    tb->setIconSize( d->toolbar->iconSize() );
    tb->setToolButtonStyle( d->toolbar->toolButtonStyle() );
    connect( d->toolbar, SIGNAL( iconSizeChanged( QSize ) ),
            tb, SLOT( setIconSize( QSize ) ) );
    connect( d->toolbar,
             SIGNAL( toolButtonStyleChanged( Qt::ToolButtonStyle ) ),
             tb, SLOT( setToolButtonStyle( Qt::ToolButtonStyle ) ) );
    tb->setDefaultAction( act );
    QAction *act2 = d->toolbar->insertWidget( d->addAction, tb );
    act2->setText( des.brief );
    connect( tb, SIGNAL( activated( int ) ), this,
              SLOT( openWindow( int ) ) );
    // connect drag & drop signals to drag objects on windows icons
    tb->setAcceptDrops( true );
    connect( tb, SIGNAL( dropped( int, QDropEvent* ) ), this,
              SLOT( dropOnWindowIcon( int, QDropEvent* ) ) );
    connect( tb, SIGNAL( dragEntered( int, QDragEnterEvent* ) ), this,
              SLOT( dragEnterOnWindowIcon( int, QDragEnterEvent* ) ) );
    connect( tb, SIGNAL( dragMoved( int, QDragMoveEvent* ) ), this,
              SLOT( dragMoveOnWindowIcon( int, QDragMoveEvent* ) ) );
  }
}


bool ControlWindow::event( QEvent *e )
{
  // intercept control window events
  if( e->type() == MapObjectEvent::eventType() )
  {
    e->accept();
    MapObjectEvent *ev = static_cast<MapObjectEvent *>( e );
    AObject *object = ev->object();
    if( object )
      registerObject( object );
    return true;
  }
  else if( e->type() == UnmapObjectEvent::eventType() )
  {
    e->accept();
    UnmapObjectEvent *ev = static_cast<UnmapObjectEvent *>( e );
    AObject *object = ev->object();
    if( object )
      unregisterObject( object );
    return true;
  }
  else if( e->type() == UpdateControlWindowEvent::eventType() )
  {
    e->accept();
    if( !theAnatomist->destroying() )
      UpdateMenus();
    return true;
  }

  return QMainWindow::event( e );
}

