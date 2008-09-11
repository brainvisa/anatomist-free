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


#include <anatomist/commands/cCreateGraph.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/graph/Graph.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


bool CreateGraphCommand::_helper( initSyntax() );


CreateGraphCommand::CreateGraphCommand( AObject* model, const string & name, 
					const string & syntax, int rid, 
					CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _model( model ), 
    _name( name ), _syntax( syntax ), _rid( rid ), _newobj( 0 )
{
}


CreateGraphCommand::~CreateGraphCommand()
{
}


bool CreateGraphCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "CreateGraph" ];
  
  s[ "object"      ].type = "int";
  s[ "object"      ].needed = true;
  s[ "res_pointer" ].type = "int";
  s[ "res_pointer" ].needed = true;
  s[ "name"        ].type = "string";
  s[ "name"        ].needed = false;
  s[ "syntax"      ].type = "string";
  s[ "syntax"      ].needed = false;
  Registry::instance()->add( "CreateGraph", &read, ss );
  return( true );
}


void
CreateGraphCommand::doit()
{
  string	name = _name;
  if( name.empty() )
    name = "RoiArg";
  string	syntax = _syntax;
  if( syntax.empty() )
    syntax = "RoiArg";

  AGraph	*ag = RoiBaseModule::newGraph( _model, name, syntax );

  _newobj = ag;
  if( !context() )
    _context = &CommandContext::defaultContext();
  if( context()->unserial && _rid >= 0 )
    context()->unserial->registerPointer( _newobj, _rid, "AObject" );
}


Command* CreateGraphCommand::read( const Tree & com, CommandContext* context )
{
  int		iobj, rid;
  AObject	*obj;
  void		*ptr;
  string	name, syntax;

  if( !com.getProperty( "object", iobj ) 
      || !com.getProperty( "res_pointer", rid ) )
    return( 0 );

  ptr = context->unserial->pointer( iobj, "AObject" );
  if( ptr )
    obj = (AObject *) ptr;
  else
    {
      cerr << "object id " << iobj << " not found\n";
      return( 0 );
    }
  com.getProperty( "name", name );
  com.getProperty( "syntax", syntax );

  return( new CreateGraphCommand( obj, name, syntax, rid, context ) );
}


void CreateGraphCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	obj;

  obj = ser->serialize( _model );

  t->setProperty( "object", obj );
  t->setProperty( "name", _name );
  t->setProperty( "syntax", _syntax );
  t->setProperty( "res_pointer", ser->serialize( _newobj ) );
  com.insert( t );
}
