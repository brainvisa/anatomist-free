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


#include <anatomist/commands/cAddNode.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/application/roibasemodule.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


bool AddNodeCommand::_helper( initSyntax() );


AddNodeCommand::AddNodeCommand( AGraph* graph, const string & name, 
				const string & syntax, bool withbucket, 
				bool nodup, int rid, int bkid, 
				CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _graph( graph ), 
    _name( name ), _syntax( syntax ), _withbck( withbucket ), _nodup( nodup ), 
    _rid( rid ), _bkid( bkid ), _newobj( 0 ), _newbck( 0 )
{
}


AddNodeCommand::~AddNodeCommand()
{
}


bool AddNodeCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "AddNode" ];
  
  s[ "graph"        ].type = "int";
  s[ "graph"        ].needed = true;
  s[ "res_pointer"  ].type = "int";
  s[ "res_pointer"  ].needed = true;
  s[ "name"         ].type = "string";
  s[ "name"         ].needed = false;
  s[ "syntax"       ].type = "string";
  s[ "syntax"       ].needed = false;
  s[ "with_bucket"  ].type = "int";
  s[ "with_bucket"  ].needed = false;
  s[ "res_bucket"   ].type = "int";
  s[ "res_bucket"   ].needed = false;
  s[ "no_duplicate" ].type = "int";
  s[ "no_duplicate" ].needed = false;
  Registry::instance()->add( "AddNode", &read, ss );
  return( true );
}


void
AddNodeCommand::doit()
{
  string	name = _name;
  if( name.empty() )
    name = "Roi";
  string	syntax = _syntax;
  if( syntax.empty() )
    syntax = "roi";

  _newobj = RoiBaseModule::newRegion( _graph, name, syntax, _withbck, _newbck, 
				  _nodup );
  if( !context() )
    _context = &CommandContext::defaultContext();
  if( context()->unserial && _newobj && _rid >= 0 )
    {
      context()->unserial->registerPointer( _newobj, _rid, "AObject" );
      if( _bkid && _newbck )
	context()->unserial->registerPointer( _newbck, _bkid, "AObject" );
    }
}


Command* AddNodeCommand::read( const Tree & com, CommandContext* context )
{
  int		iobj, rid, bkid, wb = 1, nodup = false;
  AGraph	*gr;
  void		*ptr;
  string	name, syntax;

  if( !com.getProperty( "graph", iobj ) 
      || !com.getProperty( "res_pointer", rid ) )
    return( 0 );

  ptr = context->unserial->pointer( iobj, "AObject" );
  if( ptr )
    gr = (AGraph *) ptr;
  else
    {
      cerr << "object id " << iobj << " not found\n";
      return( 0 );
    }
  com.getProperty( "with_bucket", wb );
  com.getProperty( "res_bucket", bkid );
  com.getProperty( "name", name );
  com.getProperty( "syntax", syntax );
  com.getProperty( "no_duplicate", nodup );

  return new AddNodeCommand( gr, name, syntax, wb, nodup, rid, bkid, context );
}


void AddNodeCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	obj;

  obj = ser->serialize( _graph );

  t->setProperty( "graph", obj );
  if( !_name.empty() )
    t->setProperty( "name", _name );
  if( !_syntax.empty() )
    t->setProperty( "syntax", _syntax );
  if( !_withbck )
    t->setProperty( "with_bucket", (int) 0 );
  if( _nodup )
    t->setProperty( "no_duplicate", (int) 1 );
  if( _bkid != -1 )
    t->setProperty( "res_bucket", ser->serialize( _newbck ) );
  t->setProperty( "res_pointer", ser->serialize( _newobj ) );
  com.insert( t );
}
