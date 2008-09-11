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

#include <anatomist/commands/cGroupObjects.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/mobject/ObjectList.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

GroupObjectsCommand::GroupObjectsCommand( const set<AObject *> & objL, int id, 
					  CommandContext* context ) 
  : RegularCommand(), SerializingCommand( context ), _objL( objL ), 
    _listObj( 0 ), _id( id )
{
}


GroupObjectsCommand::~GroupObjectsCommand()
{
}


bool GroupObjectsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GroupObjects" ];
  
  s[ "objects"     ].type = "int_vector";
  s[ "objects"     ].needed =  true;
  s[ "res_pointer" ].type = "int";
  s[ "res_pointer" ].needed =  false;
  Registry::instance()->add( "GroupObjects", &read, ss );
  return( true );
}


void
GroupObjectsCommand::doit()
{
  set<AObject*>::iterator io, j;

  // clean up list to remove objects which could have been destroyed
  for( io=_objL.begin(); io!=_objL.end(); ++io )
    if( !theAnatomist->hasObject( *io ) )
      {
	j = io;
	--io;
	_objL.erase( j );
      }

  if( _objL.size() == 0 )
    {
      return;
    }

  if( _objL.size() > 1 )
    {
      ObjectList *lobj = new ObjectList;
      lobj->setName( theAnatomist->makeObjectName( "ListGroup" ) );
      for( io=_objL.begin(); io!=_objL.end(); ++io )
	{
	  lobj->insert( *io );
	  /*if( (*io)->Visible() && (*io)->WinList().size() == 0 ) 
	    theAnatomist->unmapObject( *io );*/
	}
      theAnatomist->registerObject( lobj );
      _listObj = lobj;
      if( _id >= 0 )
	context()->unserial->registerPointer( lobj, _id, "AObject" );
    }
  else	// ungroup
    {
      AObject* obj = *_objL.begin();

      if( !obj->isMultiObject() )
	{
	  return;
	}
      theAnatomist->destroyObject( obj );
    }

  theAnatomist->UpdateInterface();
  theAnatomist->Refresh();
}


void GroupObjectsCommand::undoit()
{
}


Command * GroupObjectsCommand::read( const Tree & com, 
				     CommandContext* context )
{
  vector<int>		obj, win;
  set<AObject*>		objL;
  unsigned		i, n;
  void			*ptr;
  int			id = -1;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );
  com.getProperty( "res_pointer", id );

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

  return new GroupObjectsCommand( objL, id, context );
}


void GroupObjectsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  set<AObject *>::const_iterator	io;
  vector<int>				obj;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  t->setProperty( "res_pointer", ser->serialize( _listObj ) );
  com.insert( t );
}


ObjectList* GroupObjectsCommand::groupObject() const
{
  return _listObj;
}

