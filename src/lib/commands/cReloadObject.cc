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


//--- header files ------------------------------------------------------------

#include <anatomist/commands/cReloadObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

ReloadObjectCommand::ReloadObjectCommand( const set<AObject *> & objL, 
                                          bool outdated ) 
  : WaitCommand(), _objL( objL ), _onlyoutdated( outdated )
{
}


ReloadObjectCommand::~ReloadObjectCommand()
{
}


bool ReloadObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "ReloadObject" ];
  
  s[ "objects"          ] = Semantic( "int_vector", true );
  s[ "only_if_outdated" ] = Semantic( "int", false );
  Registry::instance()->add( "ReloadObject", &read, ss );
  return( true );
}


void
ReloadObjectCommand::doit()
{
  set<AObject*>::iterator obj;

  for( obj=_objL.begin(); obj!=_objL.end(); ++obj )
    {
      if( theAnatomist->hasObject( *obj ) )
	{
	  cout << "Reload object " << *obj << "\n";
	  AObject::reload( *obj, _onlyoutdated );
	  cout << "Reload object finished" << endl;
	}
    }
}


Command * ReloadObjectCommand::read( const Tree & com, 
				     CommandContext* context )
{
  set<AObject *>	objL;
  vector<int>		obj;
  unsigned		i, n;
  void			*ptr;
  int			outdated = 0;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );
  com.getProperty( "only_if_outdated", outdated );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objL.insert( (AObject *) ptr );
      else
	{
	  cerr << "object id " << obj[i] << " not found\n";
	  return( 0 );
	}
    }

  return( new ReloadObjectCommand( objL, (bool) outdated ) );
}


void ReloadObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  vector<int>				obj;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  if( _onlyoutdated )
    t->setProperty( "only_if_outdated", (int) 1 );
  com.insert( t );
}
