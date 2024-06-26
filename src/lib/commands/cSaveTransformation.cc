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

#include <anatomist/commands/cSaveTransformation.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/context.h>
#include <aims/io/writer.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SaveTransformationCommand::SaveTransformationCommand( const string & filename, 
						      Transformation* t )
  : RegularCommand(), _tra( t ), _filename( filename )
{
}


SaveTransformationCommand::~SaveTransformationCommand()
{
}


bool SaveTransformationCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SaveTransformation" ];

  s[ "transformation" ].type = "int";
  s[ "transformation" ].needed = true;
  s[ "filename"       ].type = "string";
  s[ "filename"       ].needed = true;

  Registry::instance()->add( "SaveTransformation", &read, ss );
  return( true );
}


void SaveTransformationCommand::doit()
{
  aims::Writer<AffineTransformation3d>  mw( _filename );
  mw.write( _tra->motion() );
}


Command* SaveTransformationCommand::read( const Tree & com, 
					  CommandContext* context )
{
  int			id;
  void			*ptr;
  string		filename;

  com.getProperty( "transformation", id );
  ptr = context->unserial->pointer( id, "Transformation" );
  if( !ptr )
    {
      cerr << "Transformation id " << id << " not found\n";
      return( 0 );
    }
  com.getProperty( "filename", filename );

  return( new SaveTransformationCommand( filename, (Transformation *) ptr ) );
}


void SaveTransformationCommand::write( Tree & com, Serializer* ser ) const
{
  Tree		*t = new Tree( true, name() );

  t->setProperty( "transformation", ser->serialize( _tra ) );
  t->setProperty( "filename", _filename );

  com.insert( t );
}


