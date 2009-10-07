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

#include <anatomist/commands/cSaveObject.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/errormessage.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <qapplication.h>
#include <time.h>

using namespace anatomist;
using namespace carto;
using namespace std;


SaveObjectCommand::SaveObjectCommand( AObject* obj, const string & filename )
  : WaitCommand(), _object( obj ), _filename( filename )
{
}


SaveObjectCommand::~SaveObjectCommand()
{
}


bool SaveObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SaveObject" ];
  
  s[ "object" ].type = "int";
  s[ "object" ].needed = true;
  s[ "filename" ].type = "string";
  s[ "filename" ].needed = false;
  Registry::instance()->add( "SaveObject", &read, ss );
  return( true );
}


void
SaveObjectCommand::doit()
{
  if( theAnatomist->hasObject( _object ) )
    {
      bool	res = false;
      string	fname = _filename;
      if( _filename.empty() )
        fname = _object->fileName();
      res = _object->save( fname );
      if( !res )
        ErrorMessage::message( string( qApp->translate
                                       ( "ErrorMessage", 
                                         "Save failed: object " ).utf8().data() ) 
                               + _object->name() 
                               + qApp->translate
                               ( "ErrorMessage", 
                                 " could not be saved to " ).utf8().data() + fname, 
                               ErrorMessage::Error );
      else
        {
          _object->setFileName( fname );
          _object->setLoadDate( time( 0 ) );
        }
    }
}


Command* SaveObjectCommand::read( const Tree & com, CommandContext* context )
{
  int		obj;
  string	fname;
  AObject	*ao;
  void		*ptr;

  if( !com.getProperty( "object", obj ) )
    return( 0 );
  com.getProperty( "filename", fname );

  ptr = context->unserial->pointer( obj, "AObject" );
  if( ptr )
    ao = (AObject *) ptr;
  else
    {
      cerr << "object id " << obj << " not found\n";
      return( 0 );
    }

  return( new SaveObjectCommand( ao, fname ) );
}


void SaveObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );
  int	obj = ser->serialize( _object );

  t->setProperty( "object", obj );
  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  com.insert( t );
}
