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


#include <anatomist/window/controlledWindow.h>
#include <QActionGroup>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/view.h>
#include <anatomist/object/Object.h>
#include <anatomist/selection/selectFactory.h>
#include <qtoolbar.h>
#include <qtimer.h>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;


struct ControlledWindow::Private
{
  Private();
  ~Private();

  QToolBar			*controls;
  map<string, QAction *>	ctlbts;
  QTimer			*trigger;
  QActionGroup *actions;
};


ControlledWindow::Private::Private()
  : controls( 0 ), trigger( 0 ), actions( 0 )
{
}


ControlledWindow::Private::~Private()
{
}


// ----

ControlledWindow::ControlledWindow( QWidget* parent, const char* name, 
				    Object options, Qt::WFlags f )
  : QAWindow( parent, name, options, f ), d( new Private )
{
}


ControlledWindow::~ControlledWindow()
{
  delete d;
}

//ARN
map<string, QAction *> ControlledWindow::getControlButtonObjects( void )
{
  return d->ctlbts;
}

void ControlledWindow::registerObject( AObject* o, bool temporaryObject,
                                       int pos )
{
  QAWindow::registerObject( o, temporaryObject, pos );

  if(!temporaryObject )
  {
    SelectFactory *sf = SelectFactory::factory();
    if( sf->isSelected( Group(), o ) )
      view()->controlSwitch()->selectionChangedEvent();
    if( !d->trigger )
    {
      d->trigger = new QTimer( this );
      d->trigger->setObjectName( "ControlledWindow_timer" );
      connect( d->trigger, SIGNAL( timeout() ), this,
                SLOT( updateControls() ) );
    }
    d->trigger->setSingleShot( true );
    d->trigger->start( 10 );
  }
}


void ControlledWindow::updateControls()
{
  list<string>				obj;
  list<shared_ptr<AObject> >::const_iterator	io, eo=_objects.end();

  for( io=_objects.begin(); io!=eo; ++io )
    if( !isTemporary( io->get() ) )
      obj.push_back( AObject::objectTypeName( (*io)->type() ) );
  view()->controlSwitch()->setAvailableControls( obj );
  view()->controlSwitch()->setActivableControls();
  view()->controlSwitch()->notifyActionChange();
  // view()->controlSwitch()->selectionChangedEvent();
}


void ControlledWindow::unregisterObject( AObject* o )
{
  bool temporaryObject = isTemporary( o );
  QAWindow::unregisterObject( o );

  if(!temporaryObject )
  {
/*      SelectFactory *sf = SelectFactory::factory();
    if( sf->isSelected( Group(), o ) )*/
    view()->controlSwitch()->selectionChangedEvent();
    if( !d->trigger )
    {
      d->trigger = new QTimer( this );
      d->trigger->setObjectName( "ControlledWindow_timer" );
      connect( d->trigger, SIGNAL( timeout() ), this, 
                SLOT( updateControls() ) );
    }
    d->trigger->setSingleShot( true );
    d->trigger->start( 10 );
  }
}


void ControlledWindow::updateAvailableControls()
{
  // cout << "ControlledWindow::updateAvailableControls\n";

  const map<int, string> & ctl = view()->controlSwitch()->availableControls();
  map<int, string>::const_iterator	ic, ec=ctl.end();
  IconDictionary			*icons = IconDictionary::instance();
  int userLevel = theAnatomist->userLevel();
  ControlDictionary *cd = ControlDictionary::instance();

  bool	deco = toolBarsVisible();
  if( !d->controls && deco )
  {
    d->controls = addToolBar( tr( "controls" ), "controls" );
    addToolBar( Qt::LeftToolBarArea, d->controls, "controls" );
    d->controls->setIconSize( QSize( 20, 20 ) );
    d->actions = new QActionGroup( d->controls );
    d->actions->setExclusive( true );
  }

  QToolBar					*tb = d->controls;
  const QPixmap					*p;
  map<string, QAction *>::const_iterator	ib, eb=d->ctlbts.end();

  //	delete unused controls buttons
  set<string>				ac, todel;
  set<string>::iterator			eac = ac.end(), id, ed = todel.end();

  for( ic=ctl.begin(); ic!=ec; ++ic )
    ac.insert( (*ic).second );
  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( ac.find( (*ib).first ) == eac )
    {
      todel.insert( (*ib).first );
      d->actions->removeAction( ib->second );
      d->controls->removeAction( ib->second );
      delete (*ib).second;
    }
  for( id=todel.begin(); id!=ed; ++id )
    d->ctlbts.erase( *id );

  if( deco )
  {
    //	create new buttons
    QAction *ac;

    for( ic=ctl.begin(); ic!=ec; ++ic )
    {
      const string	& txt = (*ic).second;
      int ul = cd->getControlInstance( txt )->userLevel();
      ib = d->ctlbts.find( txt );
      if( ib == eb )
      {
        p = icons->getIconInstance( txt.c_str() );
        if( p )
          ac = d->actions->addAction( *p, tr( txt.c_str() ) );
        else	// no icon
        {
          cout << "No icon for control " << txt << endl;
          ac = d->actions->addAction( tr( txt.c_str() ) );
        }
        ac->setCheckable( true );
        d->controls->addAction( ac );
        d->ctlbts[ txt ] = ac;
        if( ul <= userLevel )
          d->controls->widgetForAction( ac )->show();
        else
          d->controls->widgetForAction( ac )->hide();
      }
      else
        if( ul <= userLevel )
          d->controls->widgetForAction( ib->second )->show();
        else
          d->controls->widgetForAction( ib->second )->hide();
    }
    connect( d->actions, SIGNAL( triggered( QAction* ) ),
             this, SLOT( activeControlChanged( QAction* ) ) );
  }
}


void ControlledWindow::updateActivableControls()
{
  // cout << "ControlledWindow::updateActivableControls\n";

  const map<int, string> & ctl = view()->controlSwitch()->activableControls();
  map<int, string>::const_iterator	ic, ec=ctl.end();
  set<string>				ac;
  set<string>::iterator			eac = ac.end();
  map<string, QAction *>::const_iterator ib, eb=d->ctlbts.end();

  for( ic=ctl.begin(); ic!=ec; ++ic )
    {
      ac.insert( (*ic).second );
      //cout << "active : " << (*ic).second << endl;
    }

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    (*ib).second->setEnabled( ac.find( (*ib).first ) != eac );
}


void ControlledWindow::activeControlChanged( QAction* act )
{
  // cout << "ControlledWindow::activeControlChanged\n";

  const string		ac = view()->controlSwitch()->activeControl();
  // cout << "(old) active : " << ac << endl;
  map<string, QAction *>::const_iterator	ib, eb=d->ctlbts.end();
  QAction *b = 0;

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( ib->second == act )
    {
      if( ib->first != ac )
      {
        b = (*ib).second;
        view()->controlSwitch()->setActiveControl( ib->first );
        // cout << "activating " << (*ib).first << endl;
      }
    }

  if( !b && !ac.empty() && toolBarsVisible() )	// none activated
  {
    // cout << "re-activating " << ac << endl;
    d->ctlbts[ ac ]->setChecked( true );
  }
}


void ControlledWindow::updateActiveControl()
{
  // cout << "ControlledWindow::updateActiveControl\n";
  const string		ac = view()->controlSwitch()->activeControl();
  //cout << "active : " << ac << endl;
  map<string, QAction *>::const_iterator	ib, eb=d->ctlbts.end();
  QAction *act = 0;

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( ib->first == ac )
      {
	//cout << "setting ON " << ac << endl;
        act = ib->second;
        act->setChecked( true );
      }
    else
      ib->second->setChecked( false );
  if( act )
    activeControlChanged( act );
}


void ControlledWindow::updateActions()
{
  // cout << "ControlledWindow::updateActions\n";
//   const set<string>		& ac = view()->controlSwitch()->actions();
//   set<string>::const_iterator	ia, ea=ac.end();
// 
//   //  cout << ac.size() << " actions\n";
//   for( ia=ac.begin(); ia!=ea; ++ia )
//     {
//       //cout << "action : " << (*ia) << endl;
//     }
}

