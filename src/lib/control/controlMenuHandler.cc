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


#include <anatomist/control/controlMenuHandler.h>
#include <anatomist/selection/qSelMenu.h>
#include <anatomist/control/wControl.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/application/module.h>
#include <qmenubar.h>
#include <aims/qtcompat/qaccel.h>
#include <graph/tree/tree.h>
#if QT_VERSION >= 0x040000
#include <QMenuItem>
#endif


using namespace anatomist;
using namespace std;


namespace anatomist
{

  struct ControlMenuHandler_Private
  {
    map<string, QSelectMenu *>	popups;
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
	  pop = new QSelectMenu;
	  _menubar->insertItem( subt->getSyntax().c_str(), pop );
	}
      for( it2=subt->begin(), et2=subt->end(); it2!=et2; ++it2 )
	pop->addOptionMenus( pop, (const Tree *) *it2 );
    }
}


void AControlMenuHandler::create()
{
  //	File
  QSelectMenu	*file = new QSelectMenu;
  _menubar->insertItem( ControlWindow::tr( "File" ), file, 0 );
  file->insertItem( ControlWindow::tr( "Open" ), _receiver, 
		    SLOT( loadObject() ), Qt::CTRL+Qt::Key_O, 1 );
  file->insertSeparator();
  file->insertItem( ControlWindow::tr( "Save global settings" ), 
		    _receiver, SLOT( saveSettings() ), 0, 5 );
  file->insertItem( ControlWindow::tr( "Save windows configuration" ), 
		    _receiver, SLOT( saveWindowsConfig() ), 0, 2 );
  file->insertItem( ControlWindow::tr( "Replay scenario" ), _receiver, 
		    SLOT( replayScenario() ), 0, 3 );
  file->insertSeparator();
  file->insertItem( ControlWindow::tr( "Clear everything" ), _receiver,
                    SLOT( clearAll() ), 0, 6 );
  file->insertSeparator();
  file->insertItem( ControlWindow::tr( "Quit" ), _receiver, SLOT( close() ), 
		    Qt::CTRL+Qt::Key_Q, 4 );

  //	Objects
  QSelectMenu	*object = new QSelectMenu;
  _menubar->insertItem( ControlWindow::tr( "Objects" ), object, 1 );
  object->insertItem( ControlWindow::tr( "Add objects in windows" ), 
		      _receiver, SLOT( addObjectsInWindows() ), 
                      Qt::Key_Plus, 101 );
  object->insertItem( ControlWindow::tr( "Remove objects from windows" ), 
		      _receiver, SLOT( removeObjects() ), 
                      Qt::Key_Minus, 102 );
  object->insertSeparator();
  object->insertItem( ControlWindow::tr( "Delete objects" ), _receiver, 
		      SLOT( deleteObjects() ), Qt::CTRL+Qt::Key_Delete, 103 );
  object->insertItem( ControlWindow::tr( "Reload objects" ), _receiver, 
		      SLOT( reload() ), Qt::CTRL+Qt::Key_R, 104 );
  object->insertSeparator();
  object->insertItem( ControlWindow::tr( "Group objects" ), _receiver, 
		      SLOT( groupObjects() ), 0, 105 );
  object->insertItem( ControlWindow::tr( "Fusion objects" ), _receiver, 
		      SLOT( fusionObjects() ), Qt::CTRL+Qt::Key_F, 106 );

  //	Windows
  QSelectMenu	*window = new QSelectMenu;
  _menubar->insertItem( ControlWindow::tr( "Windows" ), window, 2 );

  map<int, string>		wtypes = AWindowFactory::typeNames();
  map<int, string>::iterator	it, ft=wtypes.end();

  for( it=wtypes.begin(); it!=ft; ++it )
    {
      window->insertItem( ControlWindow::tr( (*it).second.c_str() ), 
			  _receiver, SLOT( openWindow( int ) ), 0, 
			  1000+(*it).first );
      window->setItemParameter( 1000+(*it).first, (*it).first );
    }
  window->insertItem( ControlWindow::tr( "Open 3 standard views" ), _receiver, 
		      SLOT( openThreeViews() ), Qt::CTRL + Qt::Key_T, 205 );
  window->insertItem( ControlWindow::tr( "Open a 4 views block" ), _receiver, 
		      SLOT( openBlockView() ), Qt::CTRL + Qt::Key_B );

  window->insertSeparator();
  window->insertItem( ControlWindow::tr( "Iconify windows" ), _receiver, 
		      SLOT( iconifyWindows() ), 0, 207 );
  window->insertItem( ControlWindow::tr( "Restore windows" ), _receiver, 
		      SLOT( restoreWindows() ), 0, 208 );
  window->insertItem( ControlWindow::tr( "Close windows" ), _receiver, 
		      SLOT( closeWindows() ), Qt::CTRL+Qt::Key_W, 209 );
  window->insertItem( ControlWindow::tr( "Link windows" ), _receiver, 
		      SLOT( linkWindows() ), 0, 210 );
  window->insertSeparator();
  window->insertItem( ControlWindow::tr( "Referential" ), _receiver, 
		      SLOT( chooseWinReferential() ), 0, 211 );

  //	Settings
  QSelectMenu	*view = new QSelectMenu;
  _menubar->insertItem( ControlWindow::tr( "Settings" ), view, 3 );
  view->insertItem( ControlWindow::tr( "Referential window" ), _receiver, 
		    SLOT( openRefWin() ), 0, 301 );
  view->insertItem( ControlWindow::tr( "View Reference colors" ), _receiver, 
		    SLOT( viewRefColors() ), 0, 302 );
  view->insertSeparator();
  view->insertItem( ControlWindow::tr( "Preferences" ), _receiver, 
		    SLOT( openPreferencesWin() ), 0, 305 );
  view->insertItem( ControlWindow::tr( "Graph parameters" ), _receiver, 
		    SLOT( graphParams() ), 0, 303 );
  view->insertItem( ControlWindow::tr( "Save preferences" ), _receiver, 
		    SLOT( saveSettings() ), 0, 306 );
  view->insertSeparator();
  view->insertItem( ControlWindow::tr( "Reload palettes" ), _receiver, 
		    SLOT( reloadPalettes() ), 0, 307 );

  //	Help
  QSelectMenu	*help = new QSelectMenu;
  _menubar->insertItem( ControlWindow::tr( "Help" ), help, 5 );
  help->insertItem( ControlWindow::tr( "Help" ), _receiver, SLOT( help() ), 0, 
		    501 );
  help->insertSeparator();
  help->insertItem( ControlWindow::tr( "About Anatomist" ), _receiver, 
		    SLOT( about() ), 0, 502 );
  help->insertSeparator();
  help->insertItem( ControlWindow::tr( "Modules..." ), _receiver, 
		    SLOT( modulesList() ), 0, 503 );
  help->insertItem( ControlWindow::tr( "Anatomist/AIMS information" ), 
                    _receiver, SLOT( aimsInfo() ), 0, 504 );

  d->popups[ "File" ] = file;
  d->popups[ "Objects" ] = object;
  d->popups[ "Windows" ] = window;
  d->popups[ "Settings" ] = view;
  d->popups[ "Help" ] = help;

  appendModulesOptions();
}


void AControlMenuHandler::enableFusionMenu( bool state )
{
  _menubar->setItemEnabled( 106, state );
}


void AControlMenuHandler::enableGroupMenu( bool state )
{
  _menubar->setItemEnabled( 105, state );
}


void AControlMenuHandler::enableLinkMenu( bool state )
{
  _menubar->setItemEnabled( 210, state );
}


void AControlMenuHandler::enableLoadRefMenu( bool state )
{
  _menubar->setItemEnabled( 211, state );
}


void AControlMenuHandler::enableUnloadRefMenu( bool state )
{
  _menubar->setItemEnabled( 212, state );
}


void AControlMenuHandler::enableRefWinMenu( bool state )
{
  _menubar->setItemEnabled( 301, state );
}


void AControlMenuHandler::enablePreferencesMenu( bool state )
{
  _menubar->setItemEnabled( 305, state );
}


void AControlMenuHandler::setGroupMenuText( const string & text )
{
  _menubar->changeItem( 105, text.c_str() );
}


void AControlMenuHandler::setLinkMenuText( const string & text )
{
  _menubar->changeItem( 210, text.c_str() );
}


//	obsolete function ? never used (no callback)
QSelectMenu* AControlMenuHandler::makePopup( const Tree & tr )
{
  QSelectMenu			*pop = new QSelectMenu;
  Tree::const_iterator		it, ft = tr.end();
  const Tree			*subt;

  for( it=tr.begin(); it!=ft; ++it )
    {
      subt = (const Tree *) *it;
      if( subt->size() == 0 )	// terminal item
	{
	  pop->insertItem( subt->getSyntax().c_str() );
	}
      else			// sub tree: new submenu popup
	{
	  QSelectMenu	*subp = makePopup( *subt );
	  pop->insertItem( subt->getSyntax().c_str(), subp );
	}
    }

  return( pop );
}


void AControlMenuHandler::makeObjectManipMenus( const set<AObject *> & obj, 
						const Tree & tr )
{
  QSelectMenu		*pop = new QSelectMenu;
  Tree::const_iterator	it, ft = tr.end();
  const Tree		*t;

  pop->setObjects( obj );

  for( it=tr.begin(); it!=ft; ++it )
    {
      t = (const Tree *) *it;
      pop->addOptionMenus( pop, t );
    }

  QMenuItem	*item = 0;
#if QT_VERSION >= 0x040000
  QObject	*parent;
  item = _menubar->findItem( 4 );
  if( item )
    delete item;
#else
  QMenuData	*parent;
  item = _menubar->findItem( 4, &parent );
  if( item && parent == _menubar )
    _menubar->removeItem( 4 );
#endif
  if( pop->count() > 0 )
    _menubar->insertItem( ControlWindow::tr( "Object-specific" ), pop, 4, 4 );
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
