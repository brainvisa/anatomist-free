/* Copyright (c) 1995-2006 CEA
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


#include "anatomist/controler/controldictionary.h"
#include "anatomist/misc/error.h"
#include <iostream>

using namespace anatomist;
using namespace std;


ControlDictionary* ControlDictionary::_instance = 0 ;

ControlDictionary* ControlDictionary::instance(){
  if( _instance == 0 )
    _instance = new ControlDictionary ;
  return _instance ;
}


ControlDictionary::ControlCreatorBase::~ControlCreatorBase()
{
}

ControlDictionary::ControlCreatorFunc::ControlCreatorFunc
( ControlCreator func )
  : ControlCreatorBase(), _function( func )
{
}

ControlDictionary::ControlCreatorFunc::~ControlCreatorFunc()
{
}

ControlPtr 
ControlDictionary::ControlCreatorFunc::operator () ()
{
  return _function();
}

// ----------------

ControlDictionary::ControlDictionary() 
{}


ControlDictionary::~ControlDictionary() 
{}

ControlPtr 
ControlDictionary::getControlInstance( const string& name )
{
  map<string, ControlDictionaryElement>::iterator 
    found( myControls.find( name ) ) ;
  if ( found != myControls.end() )
    return (*found->second.creator)( ) ;
  string	msg 
    = string( "Such control name ( " ) + name + " ) correspond to no control";
  AWarning( msg.c_str() );

  return 0 ;
}


int ControlDictionary::controlPriority( const string & name ) const
{
  map<string, ControlDictionaryElement>::const_iterator
    found( myControls.find( name ) );
  if( found != myControls.end() )
    return found->second.priority;
  return 1000;
}


int 
ControlDictionary::testPriorityUnicity( int priority )
{
  map<string, ControlDictionaryElement>::const_iterator 
    iter(myControls.begin()), last(myControls.end()) ;

  bool found = false ;
  while( iter != last  && !found){
    found = ( priority == iter->second.priority ) ;
    ++iter ;
  }
  if (found ) return testPriorityUnicity( priority + 1 ) ;

  return priority ;
}
  
void 
ControlDictionary::addControl( const string& name, 
                               ControlCreatorBase *control, 
			       int priority )
{
  map<string, ControlDictionaryElement>::const_iterator 
    found( myControls.find( name ) ) ;
  if( found != myControls.end() )
    {
      string	msg = string( "Control " ) + name 
	+ " can not be added to dictionary, its name is already registered";
      AWarning( msg.c_str() ) ;
      return ;
    }
  if( control == 0 )
    {
      cerr << "Control pointer is null" << endl ;
      return ;
    }
  ControlDictionaryElement el ;
  el.priority = priority ;
  el.creator.reset( control );
  
#ifdef ANADEBUG
  cerr << "Test priority unicity" << endl ;
#endif
  int validPriority = testPriorityUnicity( el.priority ) ;
  el.priority = validPriority ;
  myControls[name] = el ;
}


void 
ControlDictionary::addControl( const string& name, ControlCreator control, 
			       int priority )
{
  addControl( name, new ControlCreatorFunc( control ), priority );
}
