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

#include <anatomist/processor/event.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/pythonwriter.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/Window.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>

using namespace anatomist;
using namespace carto;
using namespace std;


OutputEvent::OutputEvent() : _allocateIDs( true )
{
}


OutputEvent::OutputEvent( const string & evname, const Object contents, 
			  bool allocIDs, 
			  const std::set<std::string> & discrim )
  : _name( evname ), _allocateIDs( allocIDs ), _contents( contents ), 
    _discrim( discrim )
{
  // avoid null objects
  if( !contents )
    _contents = Object::value( Dictionary() );
}


OutputEvent::~OutputEvent()
{
}


namespace
{

  template<typename T> 
  bool trytype( Object val, const string & att, 
		Syntax & s, const string & typen )
  {
    try
      {
	val->GenericObject::value<T *>();
	s[ att ] = Semantic( typen, false, true );
	//cout << "internal attrib: " << att << ": " << typen << " *\n";
	return true;
      }
    catch( exception & )
      {
	try
	  {
	    val->GenericObject::value<vector<T *> >();
	    s[ att ] = Semantic( typen + "_vector", false, true );
	    /*cout << "internal attrib: " << att << ": vector<" << typen 
	      << " *>\n";*/
	    return true;
	  }
	catch( exception & )
	  {
	    try
	      {
		val->GenericObject::value<list<T *> >();
		s[ att ] = Semantic( typen + "_list", false, true );
		/*cout << "internal attrib: " << att << ": list<" << typen 
		  << " *>\n";*/
		return true;
	      }
	    catch( exception & )
	      {
		try
		  {
		    val->GenericObject::value<set<T *> >();
		    s[ att ] = Semantic( typen + "_set", false, true );
		    /*cout << "internal attrib: " << att << ": set<" << typen 
		      << " *>\n";*/
		    return true;
		  }
		catch( exception & )
		  {
		    //cout << "not recognized\n";
		    return false;
		  }
	      }
	  }
      }
  }


  void scanSerialize( Object contents, Syntax & s )
  {
    Object	ia;
    string	attn;

    for( ia=contents->objectIterator(); ia->isValid(); ia->next() )
      if( ia->key()[0] == '_' )
	{
	  Object	val = ia->currentValue();
          attn = ia->key();
	  trytype<AObject>( val, attn, s, "AObject" )
	    || trytype<AWindow>( val, attn, s, "AWindow" )
	    || trytype<Referential>( val, attn, s, "Referential" )
	    || trytype<Transformation>( val, attn, s, "Transformation" );
	}
  }


  int idOf( Unserializer * uns, map<void*, int> & rev, void * ptr, 
	    const string & type, bool aid )
  {
    map<void*, int>::iterator	ir = rev.find( ptr );
    if( ir == rev.end() )
      {
	if( !aid )
	  return -1;
	/*cout << "id for " << ptr << " not found (table size: " << rev.size() 
	  << ")\n";*/
	int	id = uns->makeID( ptr, type );
	rev[ ptr ] = id;
	return id;
      }
    return ir->second;
  }


  template<typename T> 
  bool serializeT2( Object contents, const Semantic & sem, 
		    Object val, Unserializer * uns, 
		    map<void*, int> & rev, const string & att, bool aid )
  {
    try
      {
	const T & v = val->GenericObject::value<T>();
	typename T::const_iterator	is, es = v.end();
	vector<int>			ids;
	int				id;
	for( is=v.begin(); is!=es; ++is )
	  {
	    id = idOf( uns, rev, *is, sem.type, aid );
	    if( id >= 0 )
	      ids.push_back( id );
	  }
	contents->setProperty( att.substr( 1, att.length() - 1 ), ids );
	return true;
      }
    catch( exception & )
      {
	return false;
      }
  }


  template<typename T> 
  bool serializeT3( Object contents, const Semantic & sem, 
		    Object val, Unserializer * uns, 
		    map<void*, int> & rev, const string & att, bool aid )
  {
    try
      {
	const T	*v = val->GenericObject::value<T *>();
	int	id = idOf( uns, rev, (void *) v, sem.type, aid );
	if( id >= 0 )
	  contents->setProperty( att.substr( 1, att.length() - 1 ), id );
	return true;
      }
    catch( exception & )
      {
	return false;
      }
  }


  template<typename T> 
  bool serializeT( Object contents, const Semantic & sem, 
		   Object val, Unserializer * uns, 
		   map<void*, int> & rev, const string & att, bool aid )
  {
    if( sem.type.substr( sem.type.length() - 4, 4 ) == "_set" )
      return serializeT2<set<T *> >( contents, sem, val, uns, rev, att, aid );
    else if( sem.type.substr( sem.type.length() - 5, 5 ) == "_list" )
      return serializeT2<list<T *> >( contents, sem, val, uns, rev, att, aid );
    else if( sem.type.substr( sem.type.length() - 7, 7 ) == "_vector" )
      return serializeT2<vector<T *> >( contents, sem, val, uns, rev, att, 
					aid );
    else
      return serializeT3<T>( contents, sem, val, uns, rev, att, aid );
  }


  void serialize( Object contents, const Syntax & s, 
		  CommandContext & context, bool aid )
  {
    context.mutex()->lock();
    Unserializer	*uns = context.unserial.get();
    if( !uns )
      return;
    // make reverse serializer map
    uns->garbageCollect();
    map<void*, int>			rev;
    const map<int, void *> 		& ids = uns->ids();
    map<int, void *>::const_iterator	ii, ei = ids.end();
    for( ii=ids.begin(); ii!=ei; ++ii )
      rev[ ii->second ] = ii->first;

    Syntax::const_iterator	is, es = s.end();
    for( is=s.begin(); is!=es; ++is )
      try
	{
	  Object	val = contents->getProperty( is->first );
	  const Semantic	& sem = is->second;
	  serializeT<AObject>( contents, sem, val, uns, rev, is->first, aid ) 
	    || serializeT<AWindow>( contents, sem, val, uns, rev, is->first, 
				    aid ) 
	    || serializeT<Referential>( contents, sem, val, uns, rev, 
					is->first, aid ) 
	    || serializeT<Transformation>( contents, sem, val, uns, rev, 
					   is->first, aid );
	}
      catch( exception & )
	{
	}
    context.mutex()->unlock();
  }

}	// empty namespace

void OutputEvent::send()
{
  const set<CommandContext *>	& contexts = CommandContext::contexts();
  set<CommandContext *>::const_iterator	ic, ec = contexts.end();
  ostream			*ostr;
  SyntaxSet			synt;
  string			satt;
  bool				neg;
  OutputEventFilter		*filt;

  SyntaxedInterface	*si = _contents->getInterface<SyntaxedInterface>();
  if( si && si->hasSyntax() )
    satt = si->getSyntax();
  else
    satt = "__generic__";

  Syntax			& s = synt[ satt ];
  set<string>::const_iterator	idi, edi = _discrim.end();
  unsigned			n;

  scanSerialize( _contents, s );

  for( ic=contexts.begin(); ic!=ec; ++ic )
    {
      ostr = (*ic)->ostr;
      //	check event filter
      filt = (*ic)->evfilter;
      if( ostr && filt )
	{
	  const set<string>	& filts = filt->filters();
	  neg = ( filts.find( eventType() ) != filts.end() );
	  if( !( neg ^ filt->isDefaultFiltering() ) )
	    {
	      serialize( _contents, s, **ic, _allocateIDs );
	      if( !_discrim.empty() )
		{
		  n = 0;
		  for( idi=_discrim.begin(); idi!=edi; ++idi )
		    if( _contents->hasProperty( *idi ) )
		      ++n;
		}
	      else
		n = 1;
	      if( n > 0 )
		{
                  if( ostr != &cout )
                    {
                      (*ostr) << "'" << _name << "'" << endl;
                      PythonWriter	pw( synt );
                      pw.setSingleLineMode( true );
                      pw.attach( *ostr );
                      pw.write( *_contents, false, false );
                      (*ostr) << endl << flush;
                    }
                  if( *ic == &CommandContext::defaultContext() )
                    {
                      map<string, set<rc_ptr<EventHandler> > >::const_iterator 
                        ih = EventHandler::handlers().find( eventType() );
                      if( ih != EventHandler::handlers().end() )
                        {
                          set<rc_ptr<EventHandler> >::const_iterator 
                            ihh, eh = ih->second.end();
                          for( ihh=ih->second.begin(); ihh!=eh; ++ihh )
                            (*ihh)->doit( *this );
                        }
                    }
		}
	    }
	}
    }
}

// ---------------

OutputEventFilter::OutputEventFilter()
  : _deffiltering( true )
{
}


OutputEventFilter::~OutputEventFilter()
{
}


void OutputEventFilter::filter( const string & evtype )
{
  _filtered.insert( evtype );
}


void OutputEventFilter::filter( const set<string> & evtype )
{
  _filtered.insert( evtype.begin(), evtype.end() );
}


void OutputEventFilter::unfilter( const string & evtype )
{
  _filtered.erase( evtype );
}


void OutputEventFilter::unfilter( const set<string> & evtype )
{
  set<string>::const_iterator	ie, ee = evtype.end();
  for( ie=evtype.begin(); ie!=ee; ++ie )
    _filtered.erase( *ie );
}


void OutputEventFilter::setDefaultIsFiltering( bool x )
{
  _deffiltering = x;
  clear();
}


void OutputEventFilter::clear()
{
  _filtered.clear();
}


// ------------------

EventHandler::EventHandler()
{
}


EventHandler::~EventHandler()
{
}


void EventHandler::registerHandler( const string & evtype, 
                                    rc_ptr<EventHandler> handler )
{
  _handlers()[ evtype ].insert( handler );
}


void EventHandler::unregisterHandler( const string & evtype, 
                                      rc_ptr<EventHandler> handler )
{
  map<string, set<rc_ptr<EventHandler> > >::iterator 
    i = _handlers().find( evtype );
  if( i == _handlers().end() )
    return;
  i->second.erase( handler );
}


map<string, set<rc_ptr<EventHandler> > > &  EventHandler::_handlers()
{
  static map<string, set<rc_ptr<EventHandler> > >	h;
  return h;
}


