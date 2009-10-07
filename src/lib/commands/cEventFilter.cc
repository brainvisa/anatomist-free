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

#include <anatomist/commands/cEventFilter.h>
#include <anatomist/processor/event.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/Registry.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/sstream.h>

using namespace anatomist;
using namespace carto;
using namespace std;


EventFilterCommand::EventFilterCommand( bool deffilter, 
					const set<string> & tofilt, 
					const set<string> & tounfilt, 
					CommandContext * context ) 
  : RegularCommand(), 
    SerializingCommand( context ? 
			context : &CommandContext::defaultContext() ), 
    _deffilter( (int) deffilter ), _filter( tofilt ), _unfilter( tounfilt )
{
}


EventFilterCommand::EventFilterCommand( const set<string> & tofilt, 
					const set<string> & tounfilt, 
					CommandContext * context ) 
  : RegularCommand(), 
    SerializingCommand( context ? 
			context : &CommandContext::defaultContext() ), 
    _deffilter( -1 ), _filter( tofilt ), _unfilter( tounfilt )
{
}


EventFilterCommand::~EventFilterCommand()
{
}


bool EventFilterCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "EventFilter" ];

  s[ "default_filtering" ].type = "int";
  s[ "filter"            ].type = "string";
  s[ "unfilter"          ].type = "string";

  Registry::instance()->add( "EventFilter", &read, ss );
  return( true );
}


void EventFilterCommand::doit()
{
  CommandContext	*c = context();
  if( !c )
    return;
  OutputEventFilter	*filter = c->evfilter;
  if( !filter )
    return;

  if( _deffilter >= 0 )
    filter->setDefaultIsFiltering( (bool) _deffilter );
  filter->filter( _filter );
  filter->unfilter( _unfilter );
}


Command* EventFilterCommand::read( const Tree & com, CommandContext* context )
{
  int			deffilt = 0;
  string		filter, unfilter, f;
  set<string>		filters, unfilters;

  if( com.getProperty( "filter", filter ) )
    {
      istringstream	ssf( filter.c_str() );
      while( !ssf.eof() )
	{
	  ssf >> f;
	  filters.insert( f );
	}
    }

  if( com.getProperty( "unfilter", unfilter ) )
    {
      istringstream	ssu( unfilter.c_str() );
      while( !ssu.eof() )
	{
	  ssu >> f;
	  unfilters.insert( f );
	}
    }
  
  if( com.getProperty( "default_filtering", deffilt ) )
    return new EventFilterCommand( (bool) deffilt, filters, unfilters, 
				   context );
  return new EventFilterCommand( filters, unfilters, context );
}


void EventFilterCommand::write( Tree & com, Serializer* ) const
{
  Tree		*t = new Tree( true, name() );

  if( _deffilter >= 0 )
    t->setProperty( "default_filtering", _deffilter );
  if( !_filter.empty() )
    {
      string filters;
      set<string>::const_iterator	is = _filter.begin(), 
	es = _filter.end();
      filters = *is;
      for( ++is; is!=es; ++is )
	filters += string( " " ) + *is;
      t->setProperty( "filter", filters );
    }
  if( !_unfilter.empty() )
    {
      string unfilters;
      set<string>::const_iterator	is = _unfilter.begin(), 
	es = _unfilter.end();
      unfilters = *is;
      for( ++is; is!=es; ++is )
	unfilters += string( " " ) + *is;
      t->setProperty( "unfilter", unfilters );
    }

  com.insert( t );
}


