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


#include <anatomist/object/Object.h>
#include <anatomist/controler/controlswitch.h>
#include <anatomist/controler/view.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlgroupdictionary.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/window/controlledWindow.h>
#include <anatomist/selection/selectFactory.h>
#include <QApplication>
#include <qlabel.h>
#include <set>
#include <iostream>

using namespace anatomist;
using namespace std;


ControlSwitchObserver::ControlSwitchObserver( )
{}


struct ToolBox::Private
{
  Private() : tab( 0 ), controldata( 0 ) {}

  QWidget	*tab;
  QWidget	*controldata;
  set<string> actions;
};


ToolBox::ToolBox( const string& activeControlDescription ):
  QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ), myActionTab(0), 
  myControlDescriptionActivation(0), 
  myControlDescription(activeControlDescription),
  myControlDescriptionWidget(0), myDescriptionActivated(false), 
  d( new Private )
{
  setObjectName("ToolBox");
  setAttribute(Qt::WA_DeleteOnClose);
  //cout << "ToolBox::ToolBox, this=" << this << endl;
  myLayout = new QVBoxLayout( this );
  myLayout->setMargin( 10 );
  myLayout->setSpacing( 10 );
  myLayout->setObjectName( "Layout" );
  myLayout->setEnabled(true) ;

  d->tab = new QWidget( this );
  QVBoxLayout *vlay = new QVBoxLayout( d->tab );
  d->tab->setLayout( vlay );
  vlay->setMargin( 0 );
  d->controldata = new QWidget( this );
  vlay = new QVBoxLayout( d->controldata );
  d->controldata->setLayout( vlay );
  myLayout->addWidget( d->tab );
  myLayout->addWidget( d->controldata );

  myControlDescriptionActivation 
    = new QPushButton( tr("Show control description"),
                       d->controldata );
  vlay->addWidget( myControlDescriptionActivation );
  myControlDescriptionActivation->setSizePolicy( 
    QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  myControlDescriptionActivation->setCheckable( true ) ;
  connect( myControlDescriptionActivation, SIGNAL( clicked() ), 
           this, SLOT( switchControlDescriptionActivation() ) ) ;
}

ToolBox::~ToolBox( )
{
}

void 
ToolBox::resetActions()
{
  if( myActionTab )
  {
    delete myActionTab ;
    myActionTab = 0 ;
  }
  d->actions.clear();
  myActionTab = new QTabWidget( d->tab );
  myActionTab->setObjectName( "ActionTab" );
  d->tab->layout()->addWidget( myActionTab );
  myActionTab->show() ;
  setWindowTitle( tr("Tool Box") ) ;
}

void
ToolBox::updateActiveControl( const string& activeControlDescription ) 
{
  cout << "ToolBox::updateActiveControl, this=" << this << ", descr: "
       << activeControlDescription << endl;

  myControlDescription = activeControlDescription ;
  if ( myControlDescriptionWidget )
  {
    delete myControlDescriptionWidget ;
    myControlDescriptionWidget = 0;
  }

  if( myDescriptionActivated )
  {
    myControlDescriptionWidget 
      = new QLabel(
        qApp->translate( "ControlledWindow", myControlDescription.c_str() ),
        d->controldata );
    d->controldata->layout()->addWidget( myControlDescriptionWidget );
    myControlDescriptionWidget->
      setMaximumSize( myControlDescriptionWidget->sizeHint() );
    myControlDescriptionWidget->show() ;
  }
  resize( sizeHint() ) ;

  //cout << "ToolBox::updateActiveControl done\n";
}

void 
ToolBox::addTab( QWidget * child, const QString & label,
                 const string & actionid )
{
  QString lab(label) ;
  int index  = lab.indexOf("Action", 0) ;

  //cout << "Index = " << index << endl ; 
//   cout << "DEBUG : lab = " << lab << "\tindex = " << index << "\tSize = " << lab.length() 
//        << endl << ( index + 6 >= lab.length() ) << "BIG PROBLEM !!!!" << endl ;

  if( index != -1 )
    lab.remove( index, 6 ) ;
  myActionTab->addTab( child, lab ) ;
  if( actionid.empty() )
    d->actions.insert( label.toStdString() );
  else
    d->actions.insert( actionid );
}


const set<string> & ToolBox::actions() const
{
  return d->actions;
}


void 
ToolBox::showPage ( QWidget * w )
{
   myActionTab->setCurrentIndex( myActionTab->indexOf( w ) ) ;
}

void ToolBox::showPage( const string & label )
{
  int	i, n = myActionTab->count();
  QString	l = ControlSwitch::tr( label.c_str() );
  for( i=0; i<n; ++i )
    if( myActionTab->tabText( i ) == l )
      {
        myActionTab->setCurrentIndex( i );
        break;
      }
}

void 
ToolBox::switchControlDescriptionActivation()
{
  if( myDescriptionActivated ) 
    myDescriptionActivated = false ;
  else 
    myDescriptionActivated = true ;

  updateActiveControl( myControlDescription ) ;
}


ControlSwitch::ControlSwitch( View * view )
{
  myViewType = view->name() ;
#ifdef ANADEBUG
  cout << "VIEW NAME : " << myViewType << endl ;
#endif

  myActionPool = new ActionPool( view ) ;

#ifdef ANADEBUG
  cerr << "ControlSwitch::Active Control : " << myActiveControl << endl ;
  cerr << "ControlSwitch::myControlEnabled : " << myControlEnabled << endl ;
#endif

  myToolBox = 0;
}

ControlSwitch::~ControlSwitch() 
{
  if( myToolBox )
    delete myToolBox ;
  if( myActionPool )
    delete myActionPool ; 
  map<string, ControlPtr>::iterator iter(myControls.begin()), last(myControls.end()) ;
  while ( iter != last ){
    if( iter->second )
      delete iter->second ;
    ++iter ;
  }
}


void
ControlSwitch::init()
{
  setAvailableControls( list<string>() ) ;
  setActivableControls( true ) ;  
}

bool 
ControlSwitch::attach( ControlSwitchObserver * window )
{
  list<ControlSwitchObserver *>::iterator iter(myObservers.begin()), last(myObservers.end()) ;
  bool found = false ;
  while( iter!= last && !found ){
    if( *iter == window )
      found = true ;
    ++iter ;
  }
  
  if( !found )
    myObservers.push_back( window ) ;
  return found ;
}

bool 
ControlSwitch::detach( ControlSwitchObserver * window )
{
  list<ControlSwitchObserver *>::iterator iter(myObservers.begin()), last(myObservers.end()) ;
  bool found = false ;
  while( iter!= last && !found ){
    if( *iter == window )
      found = true ;
    else
      ++iter ;
  }
  
  if( found )
    myObservers.erase( iter ) ;
  return found ;
}

void 
ControlSwitch::notifyActivableControlChange()
{
  list<ControlSwitchObserver *>::const_iterator iter( myObservers.begin() ), last( myObservers.end() ) ;
  while (iter != last ){
    (*iter)->updateActivableControls( ) ;
    ++iter ;
  }
}

void 
ControlSwitch::notifyAvailableControlChange()
{
  list<ControlSwitchObserver *>::const_iterator iter( myObservers.begin() ), last( myObservers.end() ) ;
  while (iter != last ){
    (*iter)->updateAvailableControls( ) ;
    ++iter ;
  }  
}

void 
ControlSwitch::notifyActiveControlChange()
{
  list<ControlSwitchObserver *>::const_iterator 
    iter( myObservers.begin() ), last( myObservers.end() ) ;
  while (iter != last )
    {
      //cout << "update controlswitchObserver " << *iter << endl;
      (*iter)->updateActiveControl( ) ;
      ++iter ;
    }    
}

void 
ControlSwitch::notifyActionChange() 
{
  //cout << "ControlSwitch::notifyActionChange\n";
  list<ControlSwitchObserver *>::const_iterator 
    iter( myObservers.begin() ), last( myObservers.end() ) ;
  while ( iter != last )
    {
      (*iter)->updateActions( ) ;
      ++iter ;
    }
  updateToolBox() ;
}

const map<int, string>& 
ControlSwitch::activableControls( ) const
{
  return myActivableControls ;
}

const set<string>&
ControlSwitch::activableControlGroups( ) const
{
  return myActivableControlGroups ;
}

const map<int, string>&
ControlSwitch::availableControls( ) const
{
  return myAvailableControls ;
}

const set<string>&
ControlSwitch::availableControlGroups( ) const
{
  return myAvailableControlGroups ;
}



const string& 
ControlSwitch::activeControl( ) const
{
  return myActiveControl ;
}

const set<string>& 
ControlSwitch::actions() const
{
  return myActionPool->actionSet() ;
}

void 
ControlSwitch::setActiveControl( const string& control )
{
  map<string, ControlPtr>::iterator 
    found ( myControls.find(myActiveControl) ) ;
  if ( found != myControls.end() )
    found->second->doOnDeselect( myActionPool ) ;

  found = myControls.find(control) ;
  if ( found == myControls.end() )
    {
      cout << "control " << control << " not found\n";
#ifdef ANADEBUG
      map<int,string>::const_iterator	ic, ec = myAvailableControls.end();
      cout << "available controls:\n";
      for( ic=myAvailableControls.begin(); ic!=ec; ++ic )
	{
	  cout << ic->second << endl;
	  if( ic->second == control )
	    cout << "(exists, not activable)\n";
	}
      cout << "activable controls:\n";
      map<string, ControlPtr>::const_iterator	ic2, ec2 = myControls.end();
      for( ic2=myControls.begin(); ic2!=ec2; ++ic2 )
      cout << ic2->first << endl;
#endif
      myControlEnabled = false ;
      myActiveControl = "" ;
      return ;
    }
  
  myActiveControl = control ;
  found->second->doOnSelect( myActionPool ) ;
  if( myToolBox )
    myToolBox->updateActiveControl( controlDescription( myActiveControl ) );
  myControlEnabled = true ;
}

// void
// ControlSwitch::clicked( int id ) 
// {
//   map<int, string>::iterator found = myAvailableControls.find(id) ;
  
//   if( found != myAvailableControls.end() )
//     setActiveControl( found->second ) ;
//   else{ 
//     cerr << "Error : priority map corrupted" << endl ;
//     ASSERT(0) ;
//   }
// }

void
ControlSwitch::setAvailableControls( const list<string>& objects )
{
  // cout << "ControlSwitch::setAvailableControls" << endl ;

  map<string, ControlPtr>::iterator 
    eraseIter(myControls.begin()), eraseLast(myControls.end()) ;
  while( eraseIter != eraseLast )
    {
      if( eraseIter->second ) 
        delete eraseIter->second ;
      // myControls.erase(eraseIter) ;
      ++eraseIter ;
    }
  myControls.clear() ;
  myAvailableControls.clear() ;
  myAvailableControlGroups.clear() ;

  set<string> viewOnlyControls( ControlManager::instance()->
				availableControlList( myViewType, "" ) ) ;
  set<string>::const_iterator viewOnlyIter( viewOnlyControls.begin() ), 
    viewOnlyLast( viewOnlyControls.end() ) ;
  map<string, ControlPtr>::iterator found ;
  ControlPtr control ;
  set<string> controlGroup ;
  ControlDictionary *cd = ControlDictionary::instance();
  int               prio;

  bool isThereAViewOnlyControl = false ;
  while( viewOnlyIter != viewOnlyLast )
    {
      isThereAViewOnlyControl = true ;
      control = cd->getControlInstance( *viewOnlyIter );
      if( control )
      {
        found = myControls.find(*viewOnlyIter) ;
        if( found != myControls.end() )
          if( found->second )
            delete found->second ;
        myControls[*viewOnlyIter] = control ;
        control->eventAutoSubscription( myActionPool ) ;
        prio = cd->controlPriority( *viewOnlyIter );
        myAvailableControls[ prio ] = *viewOnlyIter ;
      }
      else
      {
        controlGroup = ControlGroupDictionary::instance()->
          getControlGroupInstance( *viewOnlyIter ) ;
        if( !controlGroup.empty() )
          /*cerr << "Error : control group should not be empty" << endl ;
            else*/
          myAvailableControlGroups.insert( *viewOnlyIter ) ;
      }
      ++viewOnlyIter ;
    }

  if(!isThereAViewOnlyControl)
    cerr << "No view dependant control. One must be defined " << endl ;
  list<string>::const_iterator iter( objects.begin() ), last( objects.end() ) ;
  set<string> objetControlList ;
  set<string>::iterator objetControlIter, objetControlLast ;
  while( iter != last )
    {
      objetControlList = ControlManager::instance()->
	availableControlList( myViewType, *iter )  ;
      objetControlIter = objetControlList.begin() ;
      objetControlLast = objetControlList.end() ;
    
      while( objetControlIter != objetControlLast )
	{
	  control = cd->getControlInstance( *objetControlIter );
          prio = cd->controlPriority(  *objetControlIter );
	  if( control )
	    {
              found = myControls.find(*objetControlIter) ;
              if( found != myControls.end() )
                if( found->second )
                  delete found->second ;
              myControls[*objetControlIter] = control ;
	      control->eventAutoSubscription( myActionPool ) ;
	      myAvailableControls[ prio ] = *objetControlIter ;
	    }
	  else
	    {
	      controlGroup = ControlGroupDictionary::instance()->
		getControlGroupInstance( *objetControlIter ) ;
	      if( !controlGroup.empty() )
		/*cerr << "Error : control group should not be empty" << endl ;
		  else*/
		myAvailableControlGroups.insert( *objetControlIter ) ;
	    }
	  ++objetControlIter ;
	}
      ++iter ;
    }

  notifyAvailableControlChange() ;

  found = myControls.find(myActiveControl) ;
  if ( found != myControls.end() )
  {
    myControlEnabled = true ;
    notifyActiveControlChange() ;
  }
  else
  {
#ifdef ANADEBUG
    cerr << "Active Control no more defined" << endl ;
#endif
    if(isThereAViewOnlyControl)
    {
      myControlEnabled = true ;
      found = myControls.find( myAvailableControls.begin()->second ) ;
      if ( found != myControls.end() ){
	setActiveControl( myAvailableControls.begin()->second ) ;
	notifyActiveControlChange() ;
      }else {
	cerr << "Inconsistency : there should be a view dependant control" << endl ;
	myControlEnabled = false ;
      }
    }else 
      myControlEnabled = false ;
  }
}

void
ControlSwitch::setActivableControls( bool initialization )
{
  // cout << "ControlSwitch::setActivableControls" << endl ;
  if ( !initialization )
    getSelectedObjectNames() ;
  myActivableControls.clear() ;
  set<string> activableControls ( ControlManager::instance()->
    activableControlList( myViewType, mySelectedObjects ) );
  set<string>::iterator iter( activableControls.begin() ),
    last( activableControls.end() ) ;
  ControlDictionary *cd = ControlDictionary::instance();

  map<string, ControlPtr>::iterator foundControl ;
  set<string>::iterator foundGroup ;
  while( iter != last )
  {
    foundControl = myControls.find( *iter ) ;
    if( foundControl == myControls.end() )
    {
      foundGroup = myAvailableControlGroups.find( *iter ) ;
      if( foundGroup != myAvailableControlGroups.end() )
        myActivableControlGroups.insert( *iter ) ;
    }
    else
      myActivableControls[ cd->controlPriority( *iter ) ]
        = foundControl->first ;

    ++iter ;
  }
  notifyActivableControlChange() ;
  int prio;

  if ( myControlEnabled ){
    foundControl = myControls.find( myActiveControl ) ;
    if( foundControl == myControls.end() ){
      cerr << "Error : Inconsistency between view controls and control manager" << endl ;
      ASSERT(0) ;
    }
    prio = cd->controlPriority( myActiveControl );
    if ( myActivableControls.find( prio ) == myActivableControls.end() )
      myControlEnabled = false ;
  } else {
    foundControl = myControls.find( myActiveControl );
    if ( foundControl != myControls.end() )
    {
      prio = cd->controlPriority( myActiveControl );
      if( myActivableControls.find( prio ) != myActivableControls.end() )
	myControlEnabled = true ;
    }
  }
}

void
ControlSwitch::printControls()
{
  cerr << "In control map order" << endl ;
  map<string, ControlPtr>::iterator iter( myControls.begin() ), 
    last(myControls.end()) ;
  ControlDictionary *cd = ControlDictionary::instance();
  while( iter != last )
    {
      cout << "Name : " << iter->second->name() << endl 
	   << "Priority : " << cd->controlPriority( iter->first ) << endl
	/*
	  << "Button enabled ? " << ( iter->second->button()->isEnabled() ? "Yes" : "No" ) << endl*/ ;
      ++iter ;
    }
  
  cerr << "In priority map order" << endl ;
  map<int, string>::iterator iter2( myAvailableControls.begin() ), last2(myAvailableControls.end()) ;
  while( iter2 != last2 ){
    iter = myControls.find( iter2->second ) ; 
    cout << "Name : " << iter->second->name() << endl 
	 << "Priority : " << cd->controlPriority( iter->first ) << endl;
    ++iter2 ;
  }
}


ControlSwitch::const_iterator 
ControlSwitch::begin() const 
{
  return myControls.begin() ;
}

ControlSwitch::const_iterator 
ControlSwitch::end() const 
{
  return myControls.end() ;
}

void 
ControlSwitch::keyPressEvent( QKeyEvent * event )
{
  //cout << "SWITCH : KEYPRESSEVENT" << endl ;
  
  if ( myControlEnabled )
    {
      if( event->key() ==  Qt::Key_F1 ){
	switchToolBoxVisible( ) ;
	event->ignore() ;
	return ;
      }
      map<string, ControlPtr>::iterator 
	found( myControls.find( myActiveControl ) ) ;
      
      if( found == myControls.end() )
	{
	  cerr << "Error : KeyPressEvent : bad active control" << endl ;
	  ASSERT(0) ;
	}
      found->second->keyPressEvent( event) ;
    }
  else
    event->ignore();
}

void 
ControlSwitch::keyReleaseEvent( QKeyEvent * event ){
  //cout << "SWITCH : KEYRELEASEEVENT" << endl ;
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : keyReleaseEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->keyReleaseEvent( event ) ;
  }
}

void 
ControlSwitch::mousePressEvent( QMouseEvent * event ){
  //cout << "SWITCH : MOUSEPRESSEVENT" << endl ;
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : mousePressEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->mousePressEvent( event ) ;
  }
}

void 
ControlSwitch::mouseReleaseEvent( QMouseEvent * event )
{
  //cout << "SWITCH : MOUSERELEASEEVENT" << endl ;

  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled )
    {
      if( found == myControls.end() )
	{
	  cerr << "Error : mouseReleaseEvent : bad active control" << endl ;
	  ASSERT(0) ;
	}
      found->second->mouseReleaseEvent( event ) ;
    }
}


void 
ControlSwitch::mouseDoubleClickEvent( QMouseEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : mouseDoubleClickEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->mouseDoubleClickEvent( event ) ;
  }
}

void 
ControlSwitch::mouseMoveEvent( QMouseEvent * event )
{
  //cout << "SWITCH : MOUSEMOVEEVENT" << endl ;
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled )
    {
      if( found == myControls.end() )
	{
	  cerr << "Error : keyReleaseEvent : bad active control" << endl ;
	  ASSERT(0) ;
	}
      found->second->mouseMoveEvent( event ) ;
    }
}

void 
ControlSwitch::wheelEvent( QWheelEvent * event )
{
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : wheelEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->wheelEvent( event ) ;
  }
}

void 
ControlSwitch::focusInEvent( QFocusEvent *  ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : focusInEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->focusInEvent(  ) ;
  }
}

void 
ControlSwitch::focusOutEvent( QFocusEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : focusOutEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->focusOutEvent(  ) ;
  }
}

void 
ControlSwitch::enterEvent( QEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : enterEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->enterEvent(  ) ;
  }
}

void 
ControlSwitch::leaveEvent( QEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : leaveEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->leaveEvent(  ) ;
  }
}

void 
ControlSwitch::paintEvent( QPaintEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : paintEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->paintEvent( event ) ;
  }
}


void 
ControlSwitch::moveEvent( QMoveEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : moveEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->moveEvent( event ) ;
  }
}

void 
ControlSwitch::resizeEvent( QResizeEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : resizeEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->resizeEvent( event ) ;
  }
}

void 
ControlSwitch::dragEnterEvent( QDragEnterEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : dragEnterEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->dragEnterEvent(  ) ;
  }
}

void 
ControlSwitch::dragMoveEvent( QDragMoveEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : dragMoveEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->dragMoveEvent(  ) ;
  }
}

void 
ControlSwitch::dragLeaveEvent( QDragLeaveEvent * ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : dragLeaveEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->dragLeaveEvent(  ) ;
  }
}

void 
ControlSwitch::dropEvent( QDropEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : dropEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->dropEvent( event ) ;
  }
}

void 
ControlSwitch::showEvent( QShowEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : showEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->showEvent( event ) ;
  }
}

void 
ControlSwitch::hideEvent( QHideEvent * event ){
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
  if ( myControlEnabled ) {
    if( found == myControls.end() ){
      cerr << "Error : hideEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->hideEvent( event ) ;
  }
}

// void 
// ControlSwitch::customEvent( QCustomEvent * event ){
//     map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) ) ;
//   if ( myControlEnabled ) {
//     if( found == myControls.end() ){
//       cerr << "Error :  : bad active control" << endl ;
//       ASSERT(0) ;
//     }
//     found->second->customEvent( event ) ;
// }


void 
ControlSwitch::selectionChangedEvent()
{
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) );
  if( myControlEnabled )
  {
    if( found == myControls.end() )
    {
      cerr << "Error : selectionChangedEvent : bad active control" << endl ;
      ASSERT(0) ;
    }
    found->second->selectionChangedEvent();
  }
}


#if QT_VERSION >= 0x040600
void
ControlSwitch::gestureEvent( QGestureEvent *event )
{
  map<string, ControlPtr>::iterator found( myControls.find(myActiveControl) );
  if ( myControlEnabled )
  {
    if( found == myControls.end() )
    {
      cerr << "Error : showEvent : bad active control" << endl;
      ASSERT(0);
    }
    found->second->gestureEvent( event );
  }
}
#endif


void 
ControlSwitch::getSelectedObjectNames()
{
  mySelectedObjects.clear() ;
  const map<unsigned, set<AObject *> >&
    so = SelectFactory::factory()->selected() ;

  list<ControlSwitchObserver *>::iterator
    iter( myObservers.begin() ), last( myObservers.end() ) ;

  // cout << "Observers size " << myObservers.size() << endl ;

  ControlledWindow * win = 0 ;
  while( iter != last )
  {
    win = dynamic_cast<ControlledWindow *>( *iter ) ;
    if( win != 0 )
      break ;

    ++iter ;
  }

  if( iter == last )
  {
    cerr << "Fatal Error : Control switch is associated with no window ! "
          << "win = "<< win << endl ;
    ASSERT(0) ;
  }

  set<string> otypes;
  string otype;
  size_t n = 0;

  map<unsigned, set<AObject *> >::const_iterator
    found( so.find( win->Group() ) ) ;
  if( found != so.end() )
  {
    set<AObject *>::const_iterator
      soIter( found->second.begin() ), soLast( found->second.end() ) ;

    while( soIter != soLast )
    {
      otype = AObject::objectTypeName( (*soIter)->type() );
      n = otypes.size();
      otypes.insert( otype );
      if( otypes.size() != n ) // avoid inserting several times the same type
        mySelectedObjects.push_back( otype );

      ++soIter ;
    }
  }

  // if no selection: insert all objects types that are present in a window
  if( otypes.empty() )
  {
    set<AObject *> wobj = win->Objects();
    set<AObject *>::const_iterator io, eo = wobj.end();
    for( io=wobj.begin(); io!=eo; ++io )
    {
      otype = AObject::objectTypeName( (*io)->type() );
      n = otypes.size();
      otypes.insert( otype );
      if( otypes.size() != n ) // avoid inserting several times the same type
        mySelectedObjects.push_back( otype );
    }
  }
}


bool
ControlSwitch::isToolBoxVisible() const
{
  return myToolBox && myToolBox->isVisible();
}

void
ControlSwitch::switchToolBoxVisible()
{
  if( !myToolBox )
  {
    myToolBox = new ToolBox;
    connect( myToolBox, SIGNAL( destroyed() ),
             this, SLOT( toolBoxDestroyed() ) );
    updateToolBox();
  }
  else if ( myToolBox->isVisible() )
  {
    delete myToolBox;
    myToolBox = 0;
    return;
  }
  myToolBox->show() ;
}

void 
ControlSwitch::updateToolBox()
{
  if( !myToolBox )
    return;

  const std::set<std::string>& actions = myActionPool->actionSet() ;
  set<std::string>::const_iterator
      iter, last( actions.end() ), notfound( myToolBox->actions().end() );
  bool redo = false;

  for( iter=actions.begin(); iter != last; ++iter )
    if( myToolBox->actions().find( *iter ) == notfound )
    {
      redo = true;
      break;
    }
  if( !redo && actions.size() == myToolBox->actions().size() )
    return; // identical actions: don't do it again

  // here actions have changed
  myToolBox->resetActions() ;
  myToolBox->updateActiveControl( controlDescription( myActiveControl) );

  for( iter=actions.begin(); iter != last; ++iter )
  {
    QWidget * view = myActionPool->action( *iter )->actionView(myToolBox) ;
    if( view != 0 ){
      const string& label = myActionPool->action( *iter )->name() ;
      myToolBox->addTab( view, tr(label.c_str()), label ) ;
      myToolBox->showPage(view) ;
    }
  }
}

void
ControlSwitch::updateControlDescription()
{
}

ToolBox* ControlSwitch::toolBox()
{
  return myToolBox;
}

anatomist::Action* 
ControlSwitch::getAction( const std::string& actionName )
{
  return myActionPool->action( actionName ) ;
}


void ControlSwitch::toolBoxDestroyed()
{
  myToolBox = 0;
}


void ControlSwitch::activateKeyPressAction(
  const string & methodname ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::KeyActionLink*
    actionlink = control->keyPressActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute();
}


void ControlSwitch::activateKeyReleaseAction(
  const string & methodname ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::KeyActionLink*
    actionlink = control->keyReleaseActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute();
}


void ControlSwitch::activateMousePressAction(
  const string & methodname, int x, int y ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::MouseActionLink*
    actionlink = control->mousePressActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute( x, y, x, y );
}


void ControlSwitch::activateMouseReleaseAction(
  const string & methodname, int x, int y ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::MouseActionLink*
    actionlink = control->mouseReleaseActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute( x, y, x, y );
}


void ControlSwitch::activateMouseDoubleClickAction(
  const string & methodname, int x, int y ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::MouseActionLink*
    actionlink = control->mouseDoubleClickActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute( x, y, x, y );
}


void ControlSwitch::activateMouseMoveAction(
  const string & methodname, int x, int y ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( myActiveControl );
  if( ic == myControls.end() )
    return;
  Control *control = ic->second;
  Control::MouseActionLink*
    actionlink = control->mouseMoveActionLinkByName( methodname );
  if( actionlink )
    actionlink->execute( x, y, x, y );
}


ControlPtr ControlSwitch::activeControlInstance() const
{
  return myControls.find( myActiveControl )->second;
}


string ControlSwitch::controlDescription( const string & ctrlname ) const
{
  map<string, ControlPtr>::const_iterator
    ic = myControls.find( ctrlname );
  if( ic == myControls.end() )
    return string();
  return ic->second->description();
}

