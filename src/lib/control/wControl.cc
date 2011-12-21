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
#include <anatomist/application/fileDialog.h>
#include <anatomist/color/wMaterial.h>
#include <anatomist/selection/qSelectFactory.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/control/qWinTree.h>
#include <anatomist/control/wControl.h>
#include <anatomist/control/controlMenuHandler.h>
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
#include <anatomist/commands/cLoadObject.h>
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
#include <graph/tree/tree.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>

#include <aims/qtcompat/qtoolbutton.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qfiledialog.h>
#include <cartobase/config/info.h>
#include <qtoolbar.h>
#include <qlabel.h>
#if QT_VERSION >= 0x040000
#include <qlist.h>
#else
#include <qvaluelist.h>
#endif
#include <qmenubar.h>
#include <qtranslator.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qtextedit.h>
#include <qaction.h>
#include <qmessagebox.h>

#include <stdio.h>
#include <algorithm>

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
    : fusionbtn(0), referencebtn(0), constrainteditorbtn(0), displayLogo( true ), winList( 0 ),
      objList( 0 ), defobjref( 0 ), defwinref( 0 ), updatemenutimer( 0 ),
      closeEnabled( true )
      {}

  QToolButton		*fusionbtn;
  QToolButton		*referencebtn;
  QToolButton    *constrainteditorbtn;
  bool			displayLogo;
  QImageLabel		*logo;
  QWindowTree		*winList;
  QObjectTree		*objList;
  mutable Referential	*defobjref;
  mutable Referential	*defwinref;
  QTimer		*updatemenutimer;
#if QT_VERSION >= 0x040000
  QToolBar		*toolbar;
#endif
  bool                  closeEnabled;
};


//	Constructors

ControlWindow::ControlWindow() 
  : QMainWindow( 0, "Control Window", 
                 Qt::WType_TopLevel | Qt::WDestructiveClose ), 
  d( new Private )
{
  if( _theControlWindow )
    {
      cerr << "Error: ControlWindow instantiated several times\n";
    }
  _theControlWindow = this;

  if( parent() == 0 )
    {
      QPixmap	anaicon( Settings::findResourceFile(
                         "icons/icon.xpm" ).c_str() );
      if( !anaicon.isNull() )
        setIcon( anaicon );
    }

  _menu = new AControlMenuHandler( menuBar(), this );

  drawContents();

#if QT_VERSION >= 0x040000
  QToolBar	*tb = d->toolbar;
#else
  QToolBar	*tb = toolBars( Qt::Left ).take();
#endif
  int	w = tb->sizeHint().height();
#if QT_VERSION >= 300
  if( w < 450 )
    w = 450;
#endif

  //cout << "toolbar height : " << tb->sizeHint().height() << endl;
  //cout << "toolbar width : " << tb->sizeHint().width() << endl;
  //cout << "menubar height : " << menuBar()->height() << endl;
  // enlarge the window so the additional menus "object-specific" and "python" 
  // can fit
  resize( sizeHint().width() + 100, w + menuBar()->height() + 2 );
}


//	Destructor


ControlWindow::~ControlWindow()
{
  if( _theControlWindow == this )
    _theControlWindow = 0;

  delete d->objList;
  delete d->winList;

  delete _menu;
  delete d;

  this->quit();
}



void ControlWindow::createMenu()
{
  _menu->create();
}


void ControlWindow::drawContents()
{
  setCaption( ( string( "Anatomist " ) + theAnatomist->versionString() 
                + " - CEA/NeuroSpin/SHFJ" ).c_str() );
  QPixmap anaicon( Settings::findResourceFile( "icons/anaIcon.xpm" ).c_str() );
  if( !anaicon.isNull() )
    setIcon( anaicon );

  createMenu();
  createIcons();

  QVBox	*main = new QVBox( this );
  setCentralWidget( main );

  d->logo = new QImageLabel( main );

  int	dl = 1;
  theAnatomist->config()->getProperty( "controlWindowLogo", dl );
  d->displayLogo = dl;
  if( !d->displayLogo )
    d->logo->hide();

  QSplitter	*panels = new QSplitter( main, "CW_panels" );
  // panels->setSpacing( 5 );
#if QT_VERSION < 0x040000
  panels->setMargin( 5 );
#endif
  panels->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
                                      QSizePolicy::Expanding ) );

  //	Objects list
  d->objList = new QObjectTree( panels, "objectList" );

  //	Windows list
  d->winList = new QWindowTree( panels, "windowList" );

  panels->setResizeMode( d->objList, QSplitter::Stretch );
  panels->setResizeMode( d->winList, QSplitter::Stretch );
#if QT_VERSION >= 0x040000
  QList<int>		szs;
#else
  QValueList<int>	szs;
#endif
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
#if QT_VERSION < 0x040000
  setUsesBigPixmaps( true );
#endif

  //	Icon bar

#if QT_VERSION >= 0x040000
  QToolBar	*iconbar = new QToolBar( tr( "Icons" ), this );
  addToolBar( Qt::LeftToolBarArea, iconbar );
  d->toolbar = iconbar;
  iconbar->setIconSize( QSize( 32, 32 ) );
#else
  QToolBar	*iconbar = new QToolBar( tr( "Icons" ), this, Left, "icons" );
  QToolButton	*openbtn = 0;
  QToolButton	*addbtn = 0, *rmvbtn = 0;
#endif

  QFont	fnt = iconbar->font();
  fnt.setPixelSize( 9 );
  iconbar->setFont( fnt );

#if QT_VERSION >= 0x040000
  QIcon	is( Settings::findResourceFile( "icons/open.xpm" ).c_str() );
  is.addFile( Settings::findResourceFile( "icons/open-active.xpm" ).c_str(),
              QSize(), QIcon::Active );
  iconbar->addAction( is, tr( "Load object" ), this, SLOT( loadObject() ) );

#else
  QPixmap openpix( Settings::findResourceFile( "icons/open.xpm" ).c_str() );
  if( !openpix.isNull() )
    {
      QPixmap	p( Settings::findResourceFile(
        "icons/open-active.xpm" ).c_str() );
      QIconSet	is( openpix, QIconSet::Large );
      if( !p.isNull() )
	is.setPixmap( p, QIconSet::Large, QIconSet::Active );
      openbtn = new QToolButton( is, tr( "Load object" ), 
				 tr( "Reads a new object from a file" ), this, 
				 SLOT( loadObject() ), iconbar, "openBtn" );
      //openbtn->setUsesTextLabel( true );
    }
#endif

  //	windows shortcuts
  map<int, string>		wtypes = AWindowFactory::typeNames();
  map<int, string>::iterator	it, ft=wtypes.end();
  int				wtype;

  for( it=wtypes.begin(); it!=ft; ++it )
    {
      wtype = (*it).first;
      QAWindowFactory::loadDefaultPixmaps( wtype );
      const QAWindowFactory::PixList & pix 
        = QAWindowFactory::pixmaps( wtype );
#if QT_VERSION >= 0x040000
      const QAWindowFactory::Descrip & des
        = QAWindowFactory::description( wtype );
      QToolButtonInt        *tb = new QToolButtonInt( wtype, iconbar );
      QAction *act;
      if( !pix.plarge.isNull() )
      {
        QIcon	is( pix.plarge );
        if( !pix.pactive.isNull() )
          is.addPixmap( pix.pactive, QIcon::Active );
        act = new QAction( is, des.brief, iconbar );
      }
      else
        act = new QAction( des.brief, iconbar );
      tb->setDefaultAction( act );
      connect( tb, SIGNAL( activated( int ) ), this,
               SLOT( openWindow( int ) ) );
#else
      QToolButtonInt    *tb;
      QIconSet        is( pix.plarge, QIconSet::Large );
      if( !pix.pactive.isNull() )
        is.setPixmap( pix.pactive, QIconSet::Large, QIconSet::Active );
      const QAWindowFactory::Descrip & des
        = QAWindowFactory::description( wtype );
      tb = new QToolButtonInt( wtype, is, des.brief, des.longer, this,
                                SLOT( openWindow( int ) ), iconbar );
#endif
      // connect drag & drop signals to drag objects on windows icons
      tb->setAcceptDrops( true );
      connect( tb, SIGNAL( dropped( int, QDropEvent* ) ), this,
                SLOT( dropOnWindowIcon( int, QDropEvent* ) ) );
      connect( tb, SIGNAL( dragEntered( int, QDragEnterEvent* ) ), this,
                SLOT( dragEnterOnWindowIcon( int, QDragEnterEvent* ) ) );
      connect( tb, SIGNAL( dragMoved( int, QDragMoveEvent* ) ), this,
                SLOT( dragMoveOnWindowIcon( int, QDragMoveEvent* ) ) );
    }

  {
    QPixmap addpix( Settings::findResourceFile( "icons/add.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile( "icons/add-active.xpm" ).c_str() );
#if QT_VERSION >= 0x040000
    QIcon       is( addpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "Add objects in windows" ), this,
                        SLOT( addObjectsInWindows() ) );
#else
    QIconSet	is( addpix, QIconSet::Large );
    if( !p.isNull() )
      is.setPixmap( p, QIconSet::Large, QIconSet::Active );
    addbtn = new QToolButton( is, tr( "Add objects in windows" ),
                              tr( "Puts selected objects in selected windows" ),
                              this, SLOT( addObjectsInWindows() ), iconbar,
                              "addBtn" );
    //openbtn->setUsesTextLabel( true );
#endif
  }

  {
    QPixmap rmvpix( Settings::findResourceFile( "icons/remove.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile(
      "icons/remove-active.xpm" ).c_str() );
#if QT_VERSION >= 0x040000
    QIcon       is( rmvpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "Removes objects from windows" ), this,
                        SLOT( removeObjects() ) );
#else
    QIconSet	is( rmvpix, QIconSet::Large );
    if( !p.isNull() )
      is.setPixmap( p, QIconSet::Large, QIconSet::Active );
    rmvbtn =
      new QToolButton( is, tr( "Removes objects from windows" ),
                      tr( "Removes selected objects from selected windows" ),
                        this, SLOT( removeObjects() ),
                        iconbar, "removeBtn" );
    //openbtn->setUsesTextLabel( true );
#endif
  }

  {
    QPixmap fusionpix( Settings::findResourceFile(
      "icons/fusion.xpm" ).c_str() );
    QPixmap   p( Settings::findResourceFile(
      "icons/fusion-active.xpm" ).c_str() );
#if QT_VERSION >= 0x040000
    QIcon	is( fusionpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "Fusion objects" ), this,
                        SLOT( fusionObjects() ) );
#else
    QIconSet	is( fusionpix, QIconSet::Large );
    if( !p.isNull() )
      is.setPixmap( p, QIconSet::Large, QIconSet::Active );
    d->fusionbtn
      = new QToolButton( is, tr( "Fusion objects" ),
                  tr( "Creates a new object combining the selected ones" ),
                        this, SLOT( fusionObjects() ), iconbar, "fusionBtn" );
    //openbtn->setUsesTextLabel( true );
#endif
  }

  {
    QPixmap refpix( Settings::findResourceFile(
      "icons/reference.xpm" ).c_str() );
    QPixmap	p( Settings::findResourceFile(
      "icons/reference-active.xpm" ).c_str() );
#if QT_VERSION >= 0x040000
    QIcon	is( refpix );
    if( !p.isNull() )
      is.addPixmap( p, QIcon::Active );
    iconbar->addAction( is, tr( "Assign referential" ), this,
                        SLOT( chooseReferential() ) );
#else
    QIconSet	is( refpix, QIconSet::Large );
    is.setPixmap( refpix, QIconSet::Small, QIconSet::Active );
    if( !p.isNull() )
      is.setPixmap( p, QIconSet::Large, QIconSet::Active );
    d->referencebtn
      = new QToolButton( is, tr( "Assign referential" ),
                          tr( "Choose a referential on objects or windows" ),
                          this, SLOT( chooseReferential() ), iconbar,
                          "referenceBtn" );
    //openbtn->setUsesTextLabel( true );
#endif
  }

  //ARN BEGIN
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
  //ARN END
}


//	Registration

void ControlWindow::registerWindow( AWindow* win )
{
  // make a valid title for the window
  win->CreateTitle();
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

  if( !d->updatemenutimer )
    {
      d->updatemenutimer = new QTimer( this );
      connect( d->updatemenutimer, SIGNAL( timeout() ), this, 
               SLOT( UpdateMenus() ) );
    }
  d->updatemenutimer->start( 1, true );
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
      _menu->setGroupMenuText( tr( "Group objects" ).utf8().data() );
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
      _menu->setGroupMenuText( tr( "Ungroup objects" ).utf8().data() );
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
      _menu->setLinkMenuText( tr( "Unlink windows" ).utf8().data() );
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
	  _menu->setLinkMenuText( tr( "Link windows" ).utf8().data() );
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

  QSelectMenu	pop;
  Tree::const_iterator	it, ft = tr.end();
  const Tree		*t;

  pop.setObjects( ol );

  for( it=tr.begin(); it!=ft; ++it )
    {
      t = (const Tree *) *it;
      pop.addOptionMenus( &pop, t );
    }
  pop.exec( p );
}


void ControlWindow::windowTreeClick()
{
  UpdateToggleMenus();
}


void ControlWindow::windowTreeDblClick( AWindow* w )
{
  w->show();
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


void ControlWindow::CreateTitle()
{
  /*----- sauf exception, le titre d'une fenetre est positionne dans les 
    ressources X
  SetTitle( _baseTitle );
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
  fd.setFilters( filt );
  fd.setCaption( capt );
  fd.setMode( QFileDialog::ExistingFiles );
  if( !fd.exec() )
    return;

  QStringList		filenames = fd.selectedFiles();
  list<QString>		scenars;
  set<AObject *>	loaded;

  for ( QStringList::Iterator it = filenames.begin(); it != filenames.end(); 
	++it )
    {
      QString	& s = *it;
      if( s.findRev( ".ana" ) == int( s.length() ) - 4 )
	scenars.push_back( s );	// script file
      else
	{
	  LoadObjectCommand *command = new LoadObjectCommand( (*it).utf8().data() );
	  theProcessor->execute( command );
          loaded.insert( command->loadedObject() );
	}
    }

  // set default ref on loaded objects
  Referential	*r = defaultObjectsReferential();
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
    new APipeReader( (*is).utf8().data() );
}


void ControlWindow::replayScenario()
{
  QFileDialog	& fd = fileDialog();
  QString filter = tr( "Scenario" ) + " (*.ana)";
  QString caption = tr( "Open scenario" );
  /*QString filename = QFileDialog::getOpenFileName( QString::null,
    filter, 0, 0, caption );*/
  fd.setFilters( filter );
  fd.setCaption( caption );
  fd.setMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;

  QString	filename = fd.selectedFile();

  if ( !filename.isEmpty() )
    new APipeReader( filename.utf8().data() );
}


void ControlWindow::openWindow( int type )
{
  string	typestr = AWindowFactory::typeString( type );
  CreateWindowCommand	*command = new CreateWindowCommand( typestr );
  theProcessor->execute( command );
  // set default ref on loaded objects
  Referential	*r = defaultWindowsReferential();
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
  event->accept( QAObjectDrag::canDecode( event )
      || QAObjectDrag::canDecodeURI( event ) );
}


void ControlWindow::dragMoveOnWindowIcon( int, QDragMoveEvent* event )
{
  event->accept( true );
}


void ControlWindow::dropOnWindowIcon( int type, QDropEvent* event )
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
    {
      Command	*cmd = new ReloadObjectCommand( objL );
      theProcessor->execute( cmd );
    }
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
  (new ChooseReferentialWindow( selectedWindows(), 
				"Choose Referential Window" ))->show();
}

void ControlWindow::chooseReferential()
{
  (new ChooseReferentialWindow( selectedWindows(), selectedObjects(), 
				"Choose Referential Window" ))->show();
}

void ControlWindow::openPreferencesWin()
{
  //	command removed
  (new PreferencesWindow)->show();
  enablePreferencesMenu( false );
}

void ControlWindow::openConstraintEditor()
{
  string title = string(tr( "ConstraintEditor" ));
  (new ConstraintEditorWindow( selectedObjects(),title.c_str()))->show();
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
  QString filename = QFileDialog::getSaveFileName( QString::null,
    filter, 0, 0, caption );
  if ( !filename.isEmpty() )
    {
      AWinConfigIO cio;
      cio.saveConfig( filename.utf8().data() );
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
  QAbout abw( 0, "About Anatomist" );
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

  char	s = FileUtil::separator();
  string url = Settings::docPath() + s + "html";
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
  qApp->quit();
  //exit( EXIT_SUCCESS );
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
  QTranslator	*tr = new QTranslator( qApp, "Translator" );

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

  QVBox		*mv = new QVBox( 0, "Modules list", Qt::WDestructiveClose );
  mv->setMargin( 10 );
  Q3ListView	*lv = new Q3ListView( mv, "Modules listview" );
  lv->addColumn( tr( "Index :" ) );
  lv->addColumn( tr( "Name :" ) );
  lv->addColumn( tr( "Description :" ) );
  mv->setCaption( tr( "Modules list" ) );

  cout << "Modules list :\n";
  for( im=mm->begin(); im!=em; ++im, ++i )
    {
      cout << i << " : " << (*im)->name() << endl;
      cout << "   " << (*im)->description() << endl;
      new Q3ListViewItem( lv, QString::number( i+1 ), 
			  QString( "  " ) + tr( (*im)->name().c_str() ), 
			  QString( "  " ) 
			  + tr( (*im)->description().c_str() ) );
    }

  mv->show();
}


void ControlWindow::aimsInfo()
{
  ostringstream	txt;
  Info::print( txt );
  QVBox		*w = new QVBox( 0, 0, Qt::WDestructiveClose );
#if QT_VERSION >= 0x040000
  QTextEdit     *info = new QTextEdit( w );
#else
  QTextEdit	*info = new QTextEdit( txt.str().c_str(), QString::null, w );
#endif
  w->setCaption( tr( "Anatomist / AIMS libraries information" ) );
  w->resize( 600, 400 );
  info->setReadOnly( true );
  info->setTextFormat( Qt::LogText );
#if QT_VERSION >= 0x040000
  info->setPlainText( txt.str().c_str() );
#endif
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

  typestr = AWindowFactory::typeString( AWindow::SAGITTAL_WINDOW );
  command = new CreateWindowCommand( typestr, -1, 0, (vector<int>) 0, true, 
                                     block );
  theProcessor->execute( command );
  w.insert( command->createdWindow() );

  typestr = AWindowFactory::typeString( AWindow::AXIAL_WINDOW );
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


Referential* ControlWindow::defaultObjectsReferential() const
{
  set<Referential *>	refs = theAnatomist->getReferentials();
  if( d->defobjref && refs.find( d->defobjref ) == refs.end() )
    d->defobjref = 0;
  return d->defobjref;
}


Referential* ControlWindow::defaultWindowsReferential() const
{
  set<Referential *>	refs = theAnatomist->getReferentials();
  if( d->defwinref && refs.find( d->defwinref ) == refs.end() )
    d->defwinref = 0;
  return d->defwinref;
}


void ControlWindow::setDefaultObjectsReferential( Referential* ref )
{
  d->defobjref = ref;
}


void ControlWindow::setDefaultWindowsReferential( Referential* ref )
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
  if ( d->closeEnabled ) {
      event->accept();
  } else {
      QMessageBox::warning( this, tr( "Closing forbidden" ),
                                  tr( "Anatomist is controlled by another application "
                                    "which does not allow closing the main window" ) );
      event->ignore();
  }
}

void ControlWindow::clearAll()
{
  /// warn / confirm
  int x = QMessageBox::critical( this, tr( "Delete all objects / windows "
    "/referentials ?" ), tr( "All objects, windows, referentials and "
    "transformations will be deleted" ), QMessageBox::Ok,
    QMessageBox::Cancel );
  if( x != QMessageBox::Ok )
    return;

  DeleteAllCommand *dc = new DeleteAllCommand;
  theProcessor->execute( dc );
}

