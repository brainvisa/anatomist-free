/* Copyright (c) 2006 CEA
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

#include <anatomist/commands/cGraphDisplayProperties.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/context.h>
#include <anatomist/selection/selectFactory.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


GraphDisplayPropertiesCommand::GraphDisplayPropertiesCommand
( const set<AObject *> & graphs, const string & dispmode, 
  const string & property )
  : RegularCommand(), _graphs( graphs ), _displaymode( dispmode ), 
    _property( property )
{
}


GraphDisplayPropertiesCommand::~GraphDisplayPropertiesCommand()
{
}


bool GraphDisplayPropertiesCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "GraphDisplayProperties" ];
  
  s[ "objects"          ] = Semantic( "int_vector", true );
  s[ "display_mode"     ].type = "string";
  s[ "display_property" ].type = "string";
  Registry::instance()->add( "GraphDisplayProperties", &read, ss );
  return( true );
}


void GraphDisplayPropertiesCommand::doit()
{
  set<AObject *>::iterator	i, e = _graphs.end();
  bool				setmode = !_displaymode.empty();
  bool				setprop = !_property.empty();
  AGraph::ColorMode		dm = AGraph::Normal;
  AGraph			*g;

  if( setmode )
  {
    if( _displaymode == "PropertyMap" )
      dm = AGraph::PropertyMap;
    else if( _displaymode != "Normal" )
      {
        cerr << "Unknown display mode " << _displaymode << endl;
        setmode = false;
      }
  }

  for( i=_graphs.begin(); i!=e; ++i )
    if( ( g = dynamic_cast<AGraph *>( *i ) ) )
      {
        if( setmode )
          g->setColorMode( dm );
        if( setprop )
          g->setColorProperty( _property );
        g->notifyObservers( this );
      }
}


Command* GraphDisplayPropertiesCommand::read( const Tree & com, 
                                              CommandContext* context )
{
  string		dispmode, property;
  vector<int>		obj;
  set<AObject *>	aobj;
  unsigned		i, n;
  void			*ptr;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );
  com.getProperty( "display_mode", dispmode );
  com.getProperty( "display_property", property );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	aobj.insert( (AObject *) ptr );
      else
	cerr << "object id " << obj[i] << " not found\n";
    }

  return new GraphDisplayPropertiesCommand( aobj, dispmode, property );
}


void GraphDisplayPropertiesCommand::write( Tree & com, Serializer* ser ) const
{
  Tree				*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  vector<int>				obj;
  for( io=_graphs.begin(); io!=_graphs.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  if( !_displaymode.empty() )
    t->setProperty( "display_mode", _displaymode );
  if( !_property.empty() )
    t->setProperty( "display_property", _property );

  com.insert( t );
}


