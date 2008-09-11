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

#include "anatomist/controler/controlmanager.h"
#include <iostream>

using anatomist::ControlManager ;
using namespace std ;


ControlManager* ControlManager::_instance = 0 ;

ControlManager* ControlManager::instance(){
  if( _instance == 0 )
    _instance = new ControlManager ;
  return _instance ;
}

set<string> 
ControlManager::availableControlList( const string& view, const list<string>& objects ) const 
{
  list<string>::const_iterator objectIter(objects.begin()), objectLast(objects.end()) ;
  set<string> controlNList ;
  
  ControlMapEntry entry(view, "") ;
  
  map< ControlMapEntry, set<string>, LessControlMap >::const_iterator found = myControlTable.find( entry ) ;
  set<string>::const_iterator iter( found->second.begin() ), last( found->second.end( ) ) ;
#ifdef ANADEBUG
  cout << "ControlManager : View dependant controls"
       << endl ;
#endif
  while ( iter != last ){
    controlNList.insert( *iter ) ;
#ifdef ANADEBUG
    cout << "\t" << *iter ; 
#endif
    ++iter ;
  }
  
#ifdef ANADEBUG
    cout << endl ; 
#endif
  
#ifdef ANADEBUG
  cout << "ControlManager : View and object dependant controls"
       << endl ;
#endif

  while( objectIter != objectLast ){
    entry.view = view ;
    entry.object = *objectIter ;
    found = myControlTable.find( entry ) ;
    
    if ( found != myControlTable.end( ) ){
#ifdef ANADEBUG
      cout << "ControlManager : Object " << *objectIter
	   << endl ;
#endif
      iter = found->second.begin() ;
      last = found->second.end() ;
      
      while ( iter != last ){
	controlNList.insert( *iter ) ;
	++iter ;
#ifdef ANADEBUG
	cout << "\t" << *iter ; 
#endif
      }
    }
    ++objectIter ;
#ifdef ANADEBUG
    cout << endl ; 
#endif
  }
  return controlNList ;
}

set< string > 
ControlManager::availableControlList( const string& view, const string& object ) const
{
#ifdef ANADEBUG
  print() ;
#endif
  ControlMapEntry entry(view, object) ;
#ifdef ANADEBUG
  cout << "VIEW : " << view << "\tOBJECT : " << object << endl ;
#endif

  map< ControlMapEntry, set<string>, LessControlMap >::const_iterator 
    found = myControlTable.find( entry ) ;
   
  set<string> controlList ;
  if ( found != myControlTable.end( ) ){
    set<string>::const_iterator iter( found->second.begin() ), 
      last( found->second.end( ) ) ;
        
    while ( iter != last ){
      controlList.insert( *iter ) ;
      ++iter ;
    }
    return controlList ;
  }
  
  return set< string >( ) ;
}  

set<string> 
ControlManager::activableControlList( const string& view, 
				      const list<string>& objects ) const 
{
  list<string>::const_iterator objectIter(objects.begin()), 
    objectLast(objects.end()) ;
  set<string> controlSet ;
#ifdef ANADEBUG
  print() ;
#endif
  ControlMapEntry entry(view, "") ;
  map< ControlMapEntry, set<string>, LessControlMap >::const_iterator found ;
  while( objectIter != objectLast )
    {
      entry.object = *objectIter ;
#ifdef ANADEBUG
      cout << "\tOBJECT : " <<  entry.object << endl ;
#endif
    
      found = myControlTable.find( entry ) ;
    
      if ( found != myControlTable.end( ) )
	{
#ifdef ANADEBUG
	  cout << "ControlManager : Object " << *objectIter
	       << endl ;
#endif
	  set<string>::const_iterator iter, last ;
	  if (controlSet.empty() )
	    {
	      iter = found->second.begin() ;
	      last = found->second.end()  ;
	      while ( iter != last )
		{
		  controlSet.insert( *iter ) ;
#ifdef ANADEBUG
		  cout << "\t" << *iter ; 
#endif
		  ++iter ;
		}
	    }
	  else
	    {
	      set<string>::const_iterator foundInSet ;
	      set<string>::iterator	iter = controlSet.begin(), 
		last = controlSet.end(), i2 ;

	      while( iter != last )
		{
		  foundInSet = found->second.find( *iter ) ;
		  if( foundInSet == found->second.end( ) )
		    {
		      i2 = iter;
		      ++iter;
		      controlSet.erase( i2 ) ;
		    }
		  else
		    ++iter ;
		}
	    }
	}
      ++objectIter ;
#ifdef ANADEBUG
      cout << endl ; 
#endif
    }
  
  entry.object = "" ;

  found = myControlTable.find( entry ) ;
  if ( found == myControlTable.end() )
    cerr << "No View Dependant Control" << endl ;
  else
    {
      set<string>::const_iterator iter( found->second.begin() ), 
	last( found->second.end( ) ) ;
#ifdef ANADEBUG
      cout << "ControlManager : View dependant controls"
	   << endl ;
#endif
      while ( iter != last )
	{
	  controlSet.insert( *iter ) ;
#ifdef ANADEBUG
	  cout << "\t" << *iter ;
	  cout << " Neut " ;
#endif
	  ++iter ;
	}
    
#ifdef ANADEBUG
      cout << endl ; 
#endif
    }
  return controlSet ;
}

bool 
ControlManager::insertView( const string& view, const ViewControls& viewControls ) 
{
  // if view already exists, insertView does nothing and returns a false value
  iterator controlMapIter( myControlTable.begin() ), controlMapLast( myControlTable.end() ) ;
  
  while( controlMapIter != controlMapLast ){
    if( controlMapIter->first.view == view )
      return false ;
    ++controlMapIter ;
  }

  ViewControls::const_iterator objectIter( viewControls.begin() ), objectLast( viewControls.end() ) ;
  while ( objectIter != objectLast ){
    addControl( view, objectIter->first,  objectIter->second ) ;
    ++objectIter ;
  }
  
  return true ;
}

bool 
ControlManager::removeView( const string& view ) 
{
  iterator controlMapIter( myControlTable.begin() ), controlMapLast( myControlTable.end() ) ;
  
  bool doesViewExist = false ;
  while( controlMapIter != controlMapLast ){
    if( controlMapIter->first.view == view ){
      myControlTable.erase( controlMapIter ) ;
      doesViewExist = true ;
    }
    ++controlMapIter ;
  }
  return doesViewExist ;
}


bool 
ControlManager::insertObject( const string& object, const ObjectControls& objectControls ) 
{
  // if object already exists, insertObject does nothing and returns a false value
  iterator controlMapIter( myControlTable.begin() ), controlMapLast( myControlTable.end() ) ;
  
  while( controlMapIter != controlMapLast ){
    if( controlMapIter->first.object == object )
      return false ;
    ++controlMapIter ;
  }

  ObjectControls::const_iterator viewIter( objectControls.begin() ), viewLast( objectControls.end() ) ;
  while ( viewIter != viewLast ){
    addControl( viewIter->first, object,  viewIter->second ) ;
    ++viewIter ;
  }
  
  return true ;
}

bool 
ControlManager::removeObject( const string& object ) 
{
  iterator controlMapIter( myControlTable.begin() ), controlMapLast( myControlTable.end() ) ;
  
  bool doesObjectExist = false ;
  while( controlMapIter != controlMapLast ){
    if( controlMapIter->first.object == object ){
      myControlTable.erase( controlMapIter ) ;
      doesObjectExist = true ;
    }
    ++controlMapIter ;
  }
  
  return doesObjectExist ;
}

void 
ControlManager::addControl( const string& view, const string& object, const set<string>& controls )
{
  ControlMapEntry entry( view, object ) ;

  iterator controlTableIter = myControlTable.find( entry ) ;
  if ( controlTableIter == myControlTable.end() ) {
    myControlTable.insert( pair< ControlMapEntry, set< string > > ( entry, controls ) ) ;
    return ;
  }
  set< string >::const_iterator iter( controls.begin() ), last( controls.end() ) ;
  while( iter != last ){
    controlTableIter->second.insert( *iter ) ;
    ++iter ;
  }
}

void 
ControlManager::addControl( const string& view, const string& object, const string& control )
{
  ControlMapEntry entry( view, object ) ;

  iterator controlTableIter = myControlTable.find( entry ) ;
  if ( controlTableIter == myControlTable.end() ) {
    set< string > controlList ;
    controlList.insert( control ) ;
    myControlTable.insert( pair< ControlMapEntry, set< string > >( entry, 
								       controlList) ) ;
    return ;
  }

  controlTableIter->second.insert( control ) ;
}

bool 
ControlManager::removeControl( const string& view, const string& object, const string& control )
{
  ControlMapEntry entry( view, object ) ;
  
  iterator controlTableIter = myControlTable.find( entry ) ;
  if ( controlTableIter == myControlTable.end() )
    return false ;
  
  set< string >::iterator iter( controlTableIter->second.begin() ), 
    last( controlTableIter->second.end( ) ) ;
  while( iter != last )
    if( *iter == control ){
      controlTableIter->second.erase( iter ) ;
      return true ;
    }
  return false ;
}

bool 
ControlManager::removeControlList(  const string& view, const string& object ) 
{
  ControlMapEntry entry( view, object ) ;

  iterator controlTableIter = myControlTable.find( entry ) ;
  if ( controlTableIter == myControlTable.end() )
    return false ;
  
  controlTableIter->second.clear() ;
  return true ;
}

void 
ControlManager::print() const
{
  map< ControlMapEntry, set<string>, LessControlMap >::const_iterator iter(myControlTable.begin()), last(myControlTable.end() ) ;

  cout << "ControlManager print" << endl ;
  while (iter != last){
    cout << iter->first.view << "\t" << iter->first.object << endl ;
    set<string>::const_iterator cIter(iter->second.begin()), cLast(iter->second.end()) ;
    while( cIter != cLast ){
      cout << "\t" << *cIter ;
      ++cIter ;
    }
    cout << endl ;
    ++iter ;
  }
  cout << endl << endl ;
}
