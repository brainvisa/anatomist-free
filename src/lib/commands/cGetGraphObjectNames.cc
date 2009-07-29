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

#include <anatomist/commands/cGetGraphObjectNames.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

GetGraphObjectNamesCommand::GetGraphObjectNamesCommand( AGraph* graph ) 
  : RegularCommand(), _graph(graph)
{
}


GetGraphObjectNamesCommand::~GetGraphObjectNamesCommand()
{
}


bool GetGraphObjectNamesCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GetGraphObjectNames" ];
  
  s[ "graph" ].type = "int_vector";
  s[ "graph" ].needed = true;
  s[ "res_names" ].type = "int";
  s[ "res_names" ].needed = true;
  Registry::instance()->add( "GetGraphObjectNames", &read, ss );
  return( true );
}


void
GetGraphObjectNamesCommand::doit()
{
  if( !theAnatomist->hasObject( _graph ) )
    {
      cerr << "No such graph in anatomist" << endl ;
      return ;
    }
  
  AGraph::const_iterator iter( _graph->begin() ), last( _graph->end() ) ;
  _names.clear() ;
  while( iter != last ){
    _names.insert( (*iter)->name() ) ;
    
    ++iter ;
  }
}


void
GetGraphObjectNamesCommand::undoit()
{
}


Command* GetGraphObjectNamesCommand::read( const Tree & com, 
					   CommandContext* context )
{
  int			graphId;
  AGraph *		graph;
  void			*ptr;

  if( !com.getProperty( "graph", graphId ) )
    return( 0 );

  ptr = context->unserial->pointer( graphId, "AGraph" );
  if( ptr )
    {
      graph = (AGraph *) ptr ;
      return( new GetGraphObjectNamesCommand( graph ) );
    }
  
  cerr << "graph id " << graphId << " not found\n";
  return ( 0 ) ;
}


void GetGraphObjectNamesCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  int				graphId ;

  graphId = ser->serialize( _graph ) ;

  t->setProperty( "graph", graphId );
  com.insert( t );
}
