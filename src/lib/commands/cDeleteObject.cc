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

#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

DeleteObjectCommand::DeleteObjectCommand( const vector<AObject *> & objL ) 
  : RegularCommand(), _objL( objL )
{
}


DeleteObjectCommand::~DeleteObjectCommand()
{
}


bool DeleteObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "DeleteObject" ];

  s[ "objects" ].type = "int_vector";
  s[ "objects" ].needed = true;
  Registry::instance()->add( "DeleteObject", &read, ss );
  return( true );
}


void
DeleteObjectCommand::doit()
{
  set<AObject *> objs( _objL.begin(), _objL.end() );
  theAnatomist->destroyObjects( objs );
  _objL.clear();
  theAnatomist->UpdateInterface();
  theAnatomist->Refresh();
}


Command * DeleteObjectCommand::read( const Tree & com, 
				     CommandContext* context )
{
  vector<int>		obj;
  vector<AObject *>	objL;
  unsigned		i, n;
  void			*ptr;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objL.push_back( (AObject *) ptr );
      else
	cerr << "object id " << obj[i] << " not found\n";
    }

  return( new DeleteObjectCommand( objL ) );
}


void DeleteObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  vector<AObject *>::const_iterator	io;
  vector<int>				obj;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  com.insert( t );
}
