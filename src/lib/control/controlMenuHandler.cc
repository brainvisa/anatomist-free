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


#include <anatomist/control/controlMenuHandler.h>
#include <anatomist/selection/qSelMenu.h>
#include <anatomist/control/wControl.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/application/module.h>
#include <qmenubar.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace std;


namespace anatomist
{

  struct ControlMenuHandler_Private
  {
    map<string, QSelectMenu *>	popups;
    QAction *fusion;
    QAction *group;
    QAction *link;
    QAction *loadRef;
//     QAction *unloadRef;
    QAction *refWin;
    QAction *prefs;
  };
}


AControlMenuHandler::AControlMenuHandler( QMenuBar* menubar, 
                                          QObject* receiver )
  : AbstractMenuHandler(), d( new ControlMenuHandler_Private ), 
    _menubar( menubar ), _receiver( receiver )
{
}


AControlMenuHandler::~AControlMenuHandler()
{
  map<string, QSelectMenu *>::iterator ip, ep = d->popups.end();
  for( ip=d->popups.begin(); ip!=ep; ++ip )
    delete ip->second;
  delete d;
}


void AControlMenuHandler::create( const Tree & tr )
{
  Tree::const_iterator	it, ft=tr.end(), it2, et2;
  QSelectMenu		*pop;
  const Tree		*subt;

  for( it=tr.begin(); it!=ft; ++it )
  {
    subt = (const Tree *) *it;
    pop = getPopup( subt->getSyntax() );
    if( !pop )
    {
      pop = new QSelectMenu( QString::fromUtf8( subt->getSyntax().c_str() ) );
      _menubar->addMenu( pop );
    }
    for( it2=subt->begin(), et2=subt->end(); it2!=et2; ++it2 )
      pop->addOptionMenus( pop, (const Tree *) *it2 );
  }
}


void AControlMenuHandler::create()
{
  QAction *ac;

  //	File
  QSelectMenu	*file = new QSelectMenu( ControlWindow::tr( "File" ) );
  _menubar->addMenu( file );
  file->addAction( ControlWindow::tr( "Open" ), _receiver, 
                   SLOT( loadObject() ), Qt::CTRL+Qt::Key_O );
  file->addSeparator();
//   file->addAction( ControlWindow::tr( "Save global settings" ), 
//                    _receiver, SLOT( saveSettings() ), 0 );
  file->addAction( ControlWindow::tr( "Save windows configuration" ), 
                   _receiver, SLOT( saveWindowsConfig() ), 0 );
  file->addAction( ControlWindow::tr( "Replay scenario" ), _receiver, 
                   SLOT( replayScenario() ), 0 );
  file->addSeparator();
  file->addAction( ControlWindow::tr( "Clear everything" ), _receiver,
                   SLOT( clearAll() ), 0 );
  file->addSeparator();
  file->addAction( ControlWindow::tr( "Quit" ), _receiver, SLOT( close() ), 
                   Qt::CTRL+Qt::Key_Q );

  //	Objects
  QSelectMenu	*object = new QSelectMenu( ControlWindow::tr( "Objects" ) );
  _menubar->addMenu( object );
  object->addAction( ControlWindow::tr( "Add objects in windows" ), 
                     _receiver, SLOT( addObjectsInWindows() ),  Qt::Key_Plus );
  object->addAction( ControlWindow::tr( "Remove objects from windows" ), 
                     _receiver, SLOT( removeObjects() ), Qt::Key_Minus );
  object->addSeparator();
  object->addAction( ControlWindow::tr( "Delete objects" ), _receiver, 
                     SLOT( deleteObjects() ), Qt::CTRL+Qt::Key_Delete );
  object->addAction( ControlWindow::tr( "Reload objects" ), _receiver, 
                     SLOT( reload() ), Qt::CTRL+Qt::Key_R );
  object->addSeparator();
  d->group = object->addAction( ControlWindow::tr( "Group objects" ), 
                                _receiver, SLOT( groupObjects() ), 0 );
  d->fusion = object->addAction( ControlWindow::tr( "Fusion objects" ), 
                                 _receiver, SLOT( fusionObjects() ), 
                                 Qt::CTRL+Qt::Key_F );

  //	Windows
  QSelectMenu	*window = new QSelectMenu( ControlWindow::tr( "Windows" ) );
  _menubar->addMenu( window );

  map<int, string>		wtypes = AWindowFactory::typeNames();
  map<int, string>::iterator	it, ft=wtypes.end();

  QActionGroup *ag = new QActionGroup( _menubar );
  ag->setObjectName( "windows_types" );
  for( it=wtypes.begin(); it!=ft; ++it )
  {
    ac = window->addAction( ControlWindow::tr( (*it).second.c_str() ) );
    ac->setData( it->first );
    ag->addAction( ac );
  }
  ag->connect( ag, SIGNAL( triggered( QAction* ) ), 
               _receiver, SLOT( openWindow( QAction* ) ) );
  window->addAction( ControlWindow::tr( "Open 3 standard views" ), _receiver, 
                     SLOT( openThreeViews() ), Qt::CTRL + Qt::Key_T );
  window->addAction( ControlWindow::tr( "Open a 4 views block" ), _receiver, 
                     SLOT( openBlockView() ), Qt::CTRL + Qt::Key_B );

  window->addSeparator();
  window->addAction( ControlWindow::tr( "Iconify windows" ), _receiver, 
                     SLOT( iconifyWindows() ), 0 );
  window->addAction( ControlWindow::tr( "Restore windows" ), _receiver, 
                     SLOT( restoreWindows() ), 0 );
  window->addAction( ControlWindow::tr( "Close windows" ), _receiver, 
                     SLOT( closeWindows() ), Qt::CTRL+Qt::Key_W );
  d->link = window->addAction( ControlWindow::tr( "Link windows" ), _receiver, 
                     SLOT( linkWindows() ), 0 );
  window->addSeparator();
  d->loadRef = window->addAction( ControlWindow::tr( "Referential" ), 
                     _receiver, SLOT( chooseWinReferential() ), 0 );

  //	Settings
  QSelectMenu	*view = new QSelectMenu( ControlWindow::tr( "Settings" ) );
  _menubar->addMenu( view );
  d->refWin = view->addAction( ControlWindow::tr( "Referential window" ), 
                   _receiver, SLOT( openRefWin() ), 0 );
  view->addAction( ControlWindow::tr( "View Reference colors" ), _receiver, 
                   SLOT( viewRefColors() ), 0 );
  view->addSeparator();
  d->prefs = view->addAction( ControlWindow::tr( "Preferences" ), _receiver, 
                   SLOT( openPreferencesWin() ), 0 );
  view->addAction( ControlWindow::tr( "Graph parameters" ), _receiver, 
                   SLOT( graphParams() ), 0 );
  view->addAction( ControlWindow::tr( "Save preferences" ), _receiver, 
                   SLOT( saveSettings() ), 0 );
  view->addSeparator();
  view->addAction( ControlWindow::tr( "Reload palettes" ), _receiver, 
                   SLOT( reloadPalettes() ), 0 );

  //	Help
  QSelectMenu	*help = new QSelectMenu( ControlWindow::tr( "Help" ) );
  _menubar->addMenu( help );
  help->addAction( ControlWindow::tr( "Help" ), _receiver, SLOT( help() ), 0 );
  help->addSeparator();
  help->addAction( ControlWindow::tr( "About Anatomist" ), _receiver, 
                   SLOT( about() ), 0 );
  help->addSeparator();
  help->addAction( ControlWindow::tr( "Modules..." ), _receiver, 
                   SLOT( modulesList() ), 0 );
  help->addAction( ControlWindow::tr( "Anatomist/AIMS information" ), 
                   _receiver, SLOT( aimsInfo() ), 0 );

  d->popups[ "File" ] = file;
  d->popups[ "Objects" ] = object;
  d->popups[ "Windows" ] = window;
  d->popups[ "Settings" ] = view;
  d->popups[ "Help" ] = help;

  appendModulesOptions();
}


void AControlMenuHandler::addWindowType( const string & type, int id )
{
  QMenu *window = d->popups[ "Windows" ];
  QAction *ac = window->addAction( ControlWindow::tr( type.c_str() ) );
  ac->setData( id );
  QActionGroup * ag = _menubar->findChild<QActionGroup *>( "windows_types" );
  ag->addAction( ac );
}


void AControlMenuHandler::enableFusionMenu( bool state )
{
  d->fusion->setEnabled( state );
}


void AControlMenuHandler::enableGroupMenu( bool state )
{
  d->group->setEnabled( state );
}


void AControlMenuHandler::enableLinkMenu( bool state )
{
  d->link->setEnabled( state );
}


void AControlMenuHandler::enableLoadRefMenu( bool state )
{
  d->loadRef->setEnabled( state );
}


void AControlMenuHandler::enableUnloadRefMenu( bool state )
{
//   d->unloadRef->setEnabled( state );
}


void AControlMenuHandler::enableRefWinMenu( bool state )
{
  d->refWin->setEnabled( state );
}


void AControlMenuHandler::enablePreferencesMenu( bool state )
{
  d->prefs->setEnabled( state );
}


void AControlMenuHandler::setGroupMenuText( const string & text )
{
  d->group->setText( QString::fromUtf8( text.c_str() ) );
}


void AControlMenuHandler::setLinkMenuText( const string & text )
{
  d->link->setText( QString::fromUtf8( text.c_str() ) );
}


void AControlMenuHandler::makeObjectManipMenus( const set<AObject *> & obj, 
                                                const Tree & tr )
{
  QSelectMenu *pop = new QSelectMenu( ControlWindow::tr( "Object-specific" ) );
  Tree::const_iterator	it, ft = tr.end();
  const Tree		*t;

  pop->setObjects( obj );

  for( it=tr.begin(); it!=ft; ++it )
  {
    t = (const Tree *) *it;
    pop->addOptionMenus( pop, t );
  }

  map<string, QSelectMenu *>::iterator 
    item = d->popups.find( "object-specific" );
  if( item != d->popups.end() )
    delete item->second;
  d->popups[ "object-specific" ] = pop;
  _menubar->insertMenu( d->popups[ "Help" ]->menuAction(), pop );
}


QSelectMenu* AControlMenuHandler::getPopup( const string & popname )
{
  map<string, QSelectMenu*>::const_iterator	ip = d->popups.find( popname );
  if( ip == d->popups.end() )
    return( 0 );
  return( ip->second );
}


void AControlMenuHandler::appendModulesOptions()
{
  ModuleManager			*mm = ModuleManager::instance();
  ModuleManager::const_iterator	im, em = mm->end();
  Tree				*opt;

  for( im=mm->begin(); im!=em; ++im )
  {
    opt = (*im)->controlWinOptions();
    if( opt )
    {
      create( *opt );
      delete opt;
    }
  }
}
