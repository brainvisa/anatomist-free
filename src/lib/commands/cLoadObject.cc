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

//--- header files ------------------------------------------------------------

#include <anatomist/commands/cLoadObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/misc/error.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <anatomist/object/Object.h>
#include <anatomist/window3D/cursor.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>


using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

LoadObjectCommand::LoadObjectCommand( const string & filename, int id, 
				      const string & objname, bool ascursor, 
				      Object options, 
                                      CommandContext* context ) 
  : WaitCommand(), SerializingCommand( context ), _filename( filename ), 
    _id( id ), _obj( 0 ), _objectname( objname ), _ascursor( ascursor ), 
    _options( options )
{
}


LoadObjectCommand::~LoadObjectCommand()
{
}


bool LoadObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "LoadObject" ];
  
  s[ "filename"    ].type = "string";
  s[ "filename"    ].needed =  true;
  s[ "res_pointer" ].type = "int";
  s[ "res_pointer" ].needed =  true;
  s[ "name"        ].type = "string";
  s[ "name"        ].needed =  false;
  s[ "as_cursor"   ] = Semantic( "int", false );
  s[ "options"     ] = Semantic( "dictionary" );
  Registry::instance()->add( "LoadObject", &read, ss );
  return( true );
}


void
LoadObjectCommand::doit()
{
  if( _ascursor )
    {
      _obj = AObject::load( _filename );
      if( _obj )
        {
          string	name = _objectname;
          if( name.empty() )
            name = FileUtil::basename( _filename );
          Cursor::addCursor( name, _obj );
        }
    }
  else
    {
      _obj = theAnatomist->loadObject( _filename, _objectname, _options );
      if( _obj )
        {
          if( context() && context()->unserial )
            context()->unserial->registerPointer( _obj, _id, "AObject" );
          // send event
          Object	ex = Object::value( Dictionary() );
          ex->setProperty( "_object", Object::value( _obj ) );
          ex->setProperty( "filename", Object::value( _filename ) );
          ex->setProperty( "type", 
                           Object::value
                           ( AObject::objectTypeName( _obj->type() ) ) );
          OutputEvent	ev( "LoadObject", ex );
          ev.send();
        }
    }
}


Command* 
LoadObjectCommand::read( const Tree & com, CommandContext* context )
{
  string	filename;
  int		id, ascurs = 0;

  if( !com.getProperty( "filename", filename ) 
      || !com.getProperty( "res_pointer", id ) )
    return( 0 );

  string	objname;
  Object	options;
  com.getProperty( "name", objname );
  com.getProperty( "as_cursor", ascurs );
  com.getProperty( "options", options );

  return( new LoadObjectCommand( filename, id, objname, (bool) ascurs, 
                                 options, context ) );
}


void LoadObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "filename", _filename );
  t->setProperty( "res_pointer", ser->serialize( _obj ) );
  if( !_objectname.empty() )
    t->setProperty( "name", _objectname );
  if( _ascursor )
    t->setProperty( "as_cursor", (int) 1 );
  if( !_options.isNull() )
    t->setProperty( "options", _options );
  com.insert( t );
}

