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


#include <anatomist/commands/cDuplicateObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <anatomist/object/Object.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

DuplicateObjectCommand::DuplicateObjectCommand( AObject * source,
    bool shallow, int id, const string & objname, bool hidden, 
    CommandContext* context )
  : RegularCommand(), SerializingCommand( context ), _source( source ),
    _shallow( shallow ), _id( id ), _obj( 0 ), _objectname( objname ),
    _hidden( hidden )
{
}


DuplicateObjectCommand::~DuplicateObjectCommand()
{
}


bool DuplicateObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "DuplicateObject" ];

  s[ "source"      ] = Semantic( "int", true );
  s[ "res_pointer" ] = Semantic( "int", true );
  s[ "name"        ] = Semantic( "string", false );
  s[ "hidden"      ] = Semantic( "bool", false );
  Registry::instance()->add( "DuplicateObject", &read, ss );
  return true;
}


void
DuplicateObjectCommand::doit()
{
  if( !_source )
    return;
  _obj = _source->clone( _shallow );
  if( !_obj )
  {
    cerr << "DuplicateObject: object could not be duplicated" << endl;
    return;
  }

  _obj->setName( theAnatomist->makeObjectName( _source->name() ) );
  theAnatomist->registerObject( _obj, !_hidden );
  if( _source->referentialInheritance() )
    _obj->setReferentialInheritance( _source->referentialInheritance() );
  else
    _obj->setReferential( _source->getReferential() );
  _obj->setCopyFlag( true );

  if( context() && context()->unserial )
    context()->unserial->registerPointer( _obj, _id, "AObject" );
  // send event
  Object	ex = Object::value( Dictionary() );
  ex->setProperty( "_object", Object::value( _obj ) );
  ex->setProperty( "_source", Object::value( _source ) );
  ex->setProperty( "type",
                   Object::value( AObject::objectTypeName( _obj->type() ) ) );
  OutputEvent	ev( "DuplicateObject", ex );
  ev.send();
}


Command* 
DuplicateObjectCommand::read( const Tree & com, CommandContext* context )
{
  int source, id;

  if( !com.getProperty( "source", source )
      || !com.getProperty( "res_pointer", id ) )
    return 0;

  AObject *sobj = reinterpret_cast<AObject *>( context->unserial->pointer(
                                               source, "AObject" ) );
  if( !sobj )
  {
    cerr << "object id " << source << " not found\n";
    return 0;
  }

  string	objname;
  int           shallow = 1, hidden = 0;
  com.getProperty( "name", objname );
  com.getProperty( "shallow", shallow );
  com.getProperty( "hidden", hidden );

  return new DuplicateObjectCommand( sobj, (bool) shallow, id, objname,
                                     (bool) hidden, context );
}


void DuplicateObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "source", ser->serialize( _source ) );
  t->setProperty( "res_pointer", ser->serialize( _obj ) );
  if( !_objectname.empty() )
    t->setProperty( "name", _objectname );
  if( !_shallow )
    t->setProperty( "shallow", (int) 0 );
  if( _hidden )
    t->setProperty( "hidden", (int) 1 );
  com.insert( t );
}

