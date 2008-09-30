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


#include <anatomist/window/controlledWindow.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/view.h>
#include <anatomist/object/Object.h>
#include <anatomist/selection/selectFactory.h>
#include <aims/qtcompat/qtoolbutton.h>
#include <qtoolbar.h>
#include <qtimer.h>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;


namespace anatomist
{

  struct ControlledWindow_PrivateData
  {
    ControlledWindow_PrivateData();
    ~ControlledWindow_PrivateData();

    QToolBar			*controls;
    map<string, QToolButton *>	ctlbts;
    QTimer			*trigger;
  };

}


ControlledWindow_PrivateData::ControlledWindow_PrivateData()
  : controls( 0 ), trigger( 0 )
{
}


ControlledWindow_PrivateData::~ControlledWindow_PrivateData()
{
}


// ----

ControlledWindow::ControlledWindow( QWidget* parent, const char* name, 
				    Object options, Qt::WFlags f )
  : QAWindow( parent, name, options, f ), d( new ControlledWindow_PrivateData )
{
}


ControlledWindow::~ControlledWindow()
{
  delete d;
}


void ControlledWindow::registerObject( AObject* o, bool temporaryObject )
{
  QAWindow::registerObject( o, temporaryObject );
  
  if(!temporaryObject )
    {
      SelectFactory *sf = SelectFactory::factory();
      if( sf->isSelected( Group(), o ) )
        view()->controlSwitch()->selectionChangedEvent();
      if( !d->trigger )
	{
	  d->trigger = new QTimer( this, "ControlledWindow_timer" );
	  connect( d->trigger, SIGNAL( timeout() ), this, 
		   SLOT( updateControls() ) );
	}
      d->trigger->start( 10, true );
    }
}


void ControlledWindow::updateControls()
{
  list<string>				obj;
  list<shared_ptr<AObject> >::const_iterator	io, eo=_objects.end();

  for( io=_objects.begin(); io!=eo; ++io )
    obj.push_back( AObject::objectTypeName( (*io)->type() ) );
  view()->controlSwitch()->setAvailableControls( obj );
  view()->controlSwitch()->setActivableControls();
  view()->controlSwitch()->notifyActionChange();
  // view()->controlSwitch()->selectionChangedEvent();
}


void ControlledWindow::unregisterObject( AObject* o, bool temporaryObject )
{
  QAWindow::unregisterObject( o, temporaryObject );

  if(!temporaryObject )
    {
/*      SelectFactory *sf = SelectFactory::factory();
      if( sf->isSelected( Group(), o ) )*/
      view()->controlSwitch()->selectionChangedEvent();
      if( !d->trigger )
	{
	  d->trigger = new QTimer( this, "ControlledWindow_timer" );
	  connect( d->trigger, SIGNAL( timeout() ), this, 
		   SLOT( updateControls() ) );
	}
      d->trigger->start( 10, true );
    }
}


void ControlledWindow::updateAvailableControls()
{
  //cout << "ControlledWindow::updateAvailableControls\n";

  const map<int, string> & ctl = view()->controlSwitch()->availableControls();
  map<int, string>::const_iterator	ic, ec=ctl.end();
  IconDictionary			*icons = IconDictionary::instance();
  int userLevel = theAnatomist->userLevel();
  ControlDictionary *cd = ControlDictionary::instance();

  bool	deco = toolBarsVisible();
#if QT_VERSION >= 0x040000
  if( !d->controls && deco )
  {
    d->controls = addToolBar( tr( "controls" ), "controls" );
    addToolBar( Qt::LeftToolBarArea, d->controls, "controls" );
    d->controls->setIconSize( QSize( 20, 20 ) );
  }
#else
  if( !d->controls && deco )
    d->controls = new QToolBar( this );
#endif

  QToolBar					*tb = d->controls;
  const QPixmap					*p;
  map<string, QToolButton *>::const_iterator	ib, eb=d->ctlbts.end();

  //	delete unused controls buttons
  set<string>				ac, todel;
  set<string>::iterator			eac = ac.end(), id, ed = todel.end();

  for( ic=ctl.begin(); ic!=ec; ++ic )
    ac.insert( (*ic).second );
  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( ac.find( (*ib).first ) == eac )
    {
      todel.insert( (*ib).first );
      delete (*ib).second;
    }
  for( id=todel.begin(); id!=ed; ++id )
    d->ctlbts.erase( *id );

  if( deco )
  {
    //	create new buttons
    QToolButton	*b;

    for( ic=ctl.begin(); ic!=ec; ++ic )
    {
      const string	& txt = (*ic).second;
      int ul = cd->getControlInstance( txt )->userLevel();
      ib = d->ctlbts.find( txt );
      if( ib == eb )
      {
        p = icons->getIconInstance( txt.c_str() );
        if( p )
        {
          b = new Q34ToolButton( *p, tr( txt.c_str() ), "", this,
                                  SLOT( activeControlChanged() ), tb );
        }
        else	// no icon
        {
          cout << "No icon for control " << txt << endl;
          b = new Q34ToolButton( tb );
          b->setTextLabel( tr( txt.c_str() ) );
          connect( b, SIGNAL( clicked() ), this,
                    SLOT( activeControlChanged() ) );
        }
        b->setToggleButton( true );
        d->ctlbts[ txt ] = b;
        if( ul <= userLevel )
          b->show();
        else
          b->hide();
      }
      else
        if( ul <= userLevel )
          ib->second->show();
        else
          ib->second->hide();
    }
  }
}


void ControlledWindow::updateActivableControls()
{
  //cout << "ControlledWindow::updateActivableControls\n";

  const map<int, string> & ctl = view()->controlSwitch()->activableControls();
  map<int, string>::const_iterator	ic, ec=ctl.end();
  set<string>				ac;
  set<string>::iterator			eac = ac.end();
  map<string, QToolButton *>::const_iterator	ib, eb=d->ctlbts.end();

  for( ic=ctl.begin(); ic!=ec; ++ic )
    {
      ac.insert( (*ic).second );
      //cout << "active : " << (*ic).second << endl;
    }

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    (*ib).second->setEnabled( ac.find( (*ib).first ) != eac );
}


void ControlledWindow::activeControlChanged()
{
  //cout << "ControlledWindow::activeControlChanged\n";
  //updateActiveControl();

  const string		ac = view()->controlSwitch()->activeControl();
  //cout << "(old) active : " << ac << endl;
  map<string, QToolButton *>::const_iterator	ib, eb=d->ctlbts.end();
  QToolButton					*b = 0;

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( (*ib).second->isOn() )
      {
	if( (*ib).first != ac  )
	  {
	    b = (*ib).second;
	    view()->controlSwitch()->setActiveControl( (*ib).first );
	    //cout << "activating " << (*ib).first << endl;
	  }
	else
	  {
	    //cout << "de-activating " << (*ib).first << endl;
	    (*ib).second->setOn( false );
	  }
      }
  if( !b && !ac.empty() && toolBarsVisible() )	// none activated
    {
      //cout << "re-activating " << ac << endl;
      d->ctlbts[ ac ]->setOn( true );
    }
}


void ControlledWindow::updateActiveControl()
{
  //cout << "ControlledWindow::updateActiveControl\n";
  const string		ac = view()->controlSwitch()->activeControl();
  //cout << "active : " << ac << endl;
  map<string, QToolButton *>::const_iterator	ib, eb=d->ctlbts.end();

  for( ib=d->ctlbts.begin(); ib!=eb; ++ib )
    if( ib->first == ac )
      {
	//cout << "setting ON " << ac << endl;
	ib->second->setOn( true );
      }
    else
      ib->second->setOn( false );
  activeControlChanged();
}


void ControlledWindow::updateActions()
{
  //cout << "ControlledWindow::updateActions\n";
  const set<string>		& ac = view()->controlSwitch()->actions();
  set<string>::const_iterator	ia, ea=ac.end();

  //  cout << ac.size() << " actions\n";
  for( ia=ac.begin(); ia!=ea; ++ia )
    {
      //cout << "action : " << (*ia) << endl;
    }
}
