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

#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <stdio.h>

using namespace anatomist;
using namespace carto;
using namespace std;


FusionObjectsCommand::FusionObjectsCommand( const vector<AObject *> & obj, 
                                            const string & method,
                                            int id, bool askorder,
                                            CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _obj( obj ), _id( id ),
    _newobj( 0 ), _method( method ), _askorder( askorder )
{
}


FusionObjectsCommand::~FusionObjectsCommand()
{
}


bool FusionObjectsCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "FusionObjects" ];

  s[ "objects"     ].type = "int_vector";
  s[ "objects"     ].needed = true;
  s[ "res_pointer" ].type = "int";
  s[ "res_pointer" ].needed = true;
  s[ "method"      ].type = "string";
  s[ "method"      ].needed = false;
  s[ "ask_order"   ].type = "int";

  Registry::instance()->add( "FusionObjects", &read, ss );
  return( true );
}


void FusionObjectsCommand::doit()
{
  FusionMethod	*fm;
  if( _method.empty() || _askorder )
    fm = FusionFactory::factory()->chooseMethod( _obj );
  else
    fm = FusionFactory::factory()->method( _method );

  if( fm )
  {
    _method = fm->ID();
    set<AObject *> so( _obj.begin(), _obj.end() );
    if( !fm->canFusion( so ) )
    {
      _newobj = 0;
      cout << "cannot make a fusion of type " << fm->ID()
        << " on the given objects\n";
        return;
    }
    _newobj = fm->fusion( _obj );

    if( _newobj )
    {
      if( _newobj->name().empty() )
      {
        string name = AObject::objectTypeName( _newobj->type() ) + ": ";
        vector<AObject *>::const_iterator i, e = _obj.end();
        bool first = true, trunc = false;
        string::size_type maxlen = 30;
        for( i=_obj.begin(); i!=e; ++i )
        {
          if( first )
            first = false;
          else
            name += ", ";
          name += (*i)->name();
          if( name.length() > maxlen )
          {
            trunc = true;
            break;
          }
        }
        if( trunc )
          name = name.substr( 0, maxlen ) + "...";
        _newobj->setName( theAnatomist->makeObjectName( name ) );
      }
      theAnatomist->registerObject( _newobj );
      if( context() && context()->unserial && _id >= 0 )
        context()->unserial->registerPointer( _newobj, _id, "AObject" );

      // send event
      Object	ex( (GenericObject *)
                        new ValueObject<Dictionary> );
      ex->setProperty( "_object", Object::value( _newobj ) );
      ex->setProperty( "method", Object::value( _method ) );
      ex->setProperty( "type",
                        Object::value
                        ( AObject::objectTypeName( _newobj->type() ) ) );
      // should we output the children ?
      // well, yes, just to test!
      ex->setProperty( "_children", Object::value( _obj ) );
      OutputEvent	ev( "FusionObjects", ex );
      ev.send();
    }
  }
  else
  {
    cerr << "No fusion method for objects ( ";

    vector<AObject *>::const_iterator	io, fo=_obj.end();

    for( io=_obj.begin(); io!=fo; ++io )
      cerr << (*io)->name() << " ";
    cerr << ")\n";
  }
}


Command* FusionObjectsCommand::read( const Tree & com, 
				     CommandContext* context )
{
  vector<int>		ids;
  void			*ptr;
  unsigned		i, n;
  vector<AObject *>	obj;
  int			rid, ask = 0;
  string		method;

  com.getProperty( "objects", ids );
  com.getProperty( "res_pointer", rid );
  com.getProperty( "method", method );
  com.getProperty( "ask_order", ask );

  obj.reserve( ids.size() );
  for( i=0, n=ids.size(); i<n; ++i )
  {
    ptr = context->unserial->pointer( ids[i], "AObject" );
    if( !ptr )
    {
      cerr << "object id " << ids[i] << " not found\n";
      return( 0 );
    }
    obj.push_back( (AObject *) ptr );
  }

  return( new FusionObjectsCommand( obj, method, rid, (bool) ask, context ) );
}


void FusionObjectsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );
  vector<int>	ids;
  vector<AObject *>::const_iterator	io, fo = _obj.end();

  for( io=_obj.begin(); io!=fo; ++io )
    ids.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", ids );
  t->setProperty( "res_pointer", ser->serialize( _newobj ) );
  if( !_method.empty() )
    t->setProperty( "method", _method );

  com.insert( t );
}
